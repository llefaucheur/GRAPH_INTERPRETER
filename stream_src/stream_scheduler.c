/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_scheduler.c
 * Description:  graph scheduler
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#ifdef __cplusplus
 extern "C" {
#endif
    

#include "stream_const.h"      /* graph list */
#include "stream_types.h"
#include "stream_extern.h"


static void read_header (arm_stream_instance_t *S);
static void reset_component (arm_stream_instance_t *S);
static uint8_t lock_this_component (arm_stream_instance_t *S);
static uint8_t unlock_this_component (arm_stream_instance_t *S);
static void set_new_parameters (arm_stream_instance_t *S, uint32_t *ptr_param32b);
static void run_node (arm_stream_instance_t *S);
static uint8_t arc_ready_for_write(arm_stream_instance_t *S, uint32_t *arc, uint32_t *frame_size);
static uint8_t arc_ready_for_read(arm_stream_instance_t *S, uint32_t *arc, uint32_t *frame_size);
static intPtr_t arc_extract_info_int (arm_stream_instance_t *S, uint32_t *arc, uint8_t tag);
static void load_memory_segments (arm_stream_instance_t *S, uint8_t pre0post1);

#define script_option (RD(S->scheduler_control, SCRIPT_SCTRL))
#define return_option (RD(S->scheduler_control, RETURN_SCTRL))


/*----------------------------------------------------------------------------
   execute a swc
   this is the only place where a swc is called for safety reason 
   swc can check the return-address is identical
 *----------------------------------------------------------------------------*/
/**
  @brief         Call a software component / Node
  @param[in]     address    address of the component
  @param[in]     instance   memory instance of the component
  @param[in/out] data       arcs of the components (list of pointer and size)
  @param[in]     parameter  returned status after execution
  @return        none

  @par           The components are called from this single place to control the 
                  PC return-address both from Stream and the Node side.
                 The Node updates the "size" field before returning.
  @remark
 */

static void stream_calls_swc (arm_stream_instance_t *S,
                    stream_handle_t instance, 
                    stream_xdmbuffer_t *data,
                    uint32_t *parameter
                   )
{
    /* node execution is starting */
    ST(S->scheduler_control, NODEEXEC_SCTRL, 1);

    /* xdm_buffer pointers are updated inside the component */
    S->address_swc (S->pack_command, instance, data, parameter);

    /* node execution is finished */
    ST(S->scheduler_control, NODEEXEC_SCTRL, 0);
}


/**
  @brief        debug script 
  @param[in]    offset     table of long offsets of idx_memory_base_offset
  @param[in]    data       packed address
  @return       inPtr_t    address in the format of the processor

  @par          execution depends on "S->script_option" scheduler configuration :
    STREAM_SCHD_SCRIPT_BEFORE_EACH_SWC  script is called before each SWC called 
    STREAM_SCHD_SCRIPT_AFTER_EACH_SWC   script is called after each SWC called 
    STREAM_SCHD_SCRIPT_END_PARSING      script is called at the end of the loop 
    STREAM_SCHD_SCRIPT_START            script is called when starting 
    STREAM_SCHD_SCRIPT_END              script is called before return 

 */

/*----------------------------------------------------------*/
static void script_processing(uint32_t script_index)
{
    // call to arm_stream_script, under definition
}


/**
  @brief        unpack a 27-bits address to physical address
  @param[in]    offset     table of long offsets of idx_memory_base_offset
  @param[in]    data       packed address
  @return       inPtr_t    address in the format of the processor

  @par          A graph gives the address of buffers as indexes ("packed address") in a 
                way independent of the memory mapping of the processor interpreting the graph.
                The scheduler of each Stream instance sends a physical address to the Nodes
                and translates here the indexes to physical address using a table of offsets.
 */

/*----------------------------------------------------------*/
static intPtr_t pack2linaddr_int(const intPtr_t *long_offset, uint32_t x, uint32_t unit)
{
    intPtr_t dbg1, dbg2, dbg3;

    dbg1 = long_offset[RD(x,DATAOFF_ARCW0)];    
    dbg2 = (intPtr_t)(unit * (intPtr_t)RD((x),BASEIDX_ARCW0));

    if (RD(x,BAS_SIGN_ARCW0)) 
        dbg3 = dbg1 + ~(dbg2) +1;   // dbg1-dbg2 using unsigned integers
     else
        dbg3 = dbg1 + dbg2;

    return dbg3; 
}


/**
  @brief         Tool box : Arc descriptor fields extraction, returns an integer
  @param[in]     instance   global data of the instance
  @param[in]     arc        pointer to the arc descriptor to read
  @param[in]     command    operation to do
  @return        int

  @par           Arcs are 4-words packed structures
                 arc_extract_info_int returns the amount of data or free area, ..
  @remark
 */

static intPtr_t arc_extract_info_int (arm_stream_instance_t *S, uint32_t *arc, uint8_t tag)
{
    uint32_t read;
    uint32_t write;
    uint32_t size;
    intPtr_t ret;
    uint32_t *all_formats;

    read =  RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);
    size =  RD(arc[1], BUFF_SIZE_ARCW1);
    all_formats =  S->all_formats;

    switch (tag)
    {
    case arc_data_amount  : ret = write - read; 
        break;
    case arc_free_area    : ret = size - write; 
        break;
    case arc_buffer_size  : ret = size; 
        break;
    default : ret = 0; 
    }
    return ret;
}

/**
  @brief         Tool box : Arc descriptor fields extraction, returns a byte pointer
  @param[in]     instance   global data of the instance
  @param[in]     arc        pointer to the arc descriptor to read
  @param[in]     command    operation to do
  @return        uint8 *    data pointer 

  @par           Arcs are 4-words packed structures
                 arc_extract_info_ptr returns the read/write addresses in the buffer
  @remark
 */

static uint8_t * arc_extract_info_pt (arm_stream_instance_t *S, uint32_t *arc, uint8_t tag)
{
    uint32_t read;
    uint32_t write;
    uint32_t base;
    uint8_t *long_base;
    uint8_t *ret;

    read =  RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);
    base =  RD(arc[0], BASEIDXOFFARCW0);
    long_base = pack2linaddr_ptr(S->long_offset, base, LINADDR_UNIT_BYTE);

    switch (tag)
    {
    case arc_read_address : ret = &(((uint8_t *)long_base)[read]); 
        break;
    case arc_write_address: ret = &(((uint8_t *)long_base)[write]); 
        break;
    default : ret = 0u; 
    }
    return ret;
}


/**
  @brief         Set the "need of data alignment bit" of the arc
  @param[in]     format     pointer to the table of formats
  @param[in/out] arc        pointer to the arc to check 
  @return        none

  @par           The arc descriptor gives, in the 1st word, the stream format used by
                 the node producing data to this arc.
                 Looking at the remaining free space in the buffer and the frame-size
                 used by the producer (the minimum amount of byte produced per call), the
                 function set the bit ALIGNBLCK_ARCW3 which will be read by the consumer
                 node to decide to realign the data to the buffer base address.
                 When a producer sets the ALIGNBLCK_ARCW3 it means "blocked by lack
                 of free space", so there is no risk of collision between different 
                 processors accessing the arc during data realignment.

  @remark
 */
static void set_alignment_bit (arm_stream_instance_t *S, uint32_t *arc)
{
    uint32_t producer_frame_size, fifosize, write;
    uint8_t i;

    fifosize =  RD(arc[1], BUFF_SIZE_ARCW1);
    write = RD(arc[3], WRITE_ARCW3);

    /* does the write index is already far, to ask for data realignment? */
    i = RD(arc[0],PRODUCFMT_ARCW0);
    i = i * STREAM_FORMAT_SIZE_W32;
    producer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

    if (fifosize - write < producer_frame_size)
    {   SET_BIT(arc[3], ALIGNBLCK_ARCW3_LSB);
    }
    else
    {   CLEAR_BIT(arc[3], ALIGNBLCK_ARCW3_LSB);
    }
}


/**
  @brief         Checks the producer node can use this arc
  @param[in]     instance   global registers of this instance
  @param[in]     arc        arc to check
  @return        none

  @par           The arc descriptor gives, in the 1st word, the stream format used by
                 the node producing data to this arc.
                 Looking at the remaining free space in the buffer and the frame-size
                 used by the producer (the minimum amount of byte produced per call), the
                 function returns a go/no-go flag.
  @remark
 */

static uint8_t arc_ready_for_write(arm_stream_instance_t *S, uint32_t *arc, uint32_t *free_for_writes)
{
    uint32_t producer_frame_size;   
    uint8_t ret;
    uint32_t fifosize, write, *all_formats;

    all_formats = S->all_formats;
    write = RD(arc[3], WRITE_ARCW3);
    fifosize = RD(arc[1], BUFF_SIZE_ARCW1);
  
    producer_frame_size = 
        RD(all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc[0],PRODUCFMT_ARCW0)], FRAMESIZE_FMT0);

    *free_for_writes =  (fifosize - write); /* memory available for writes */

    if (write > fifosize - producer_frame_size)
    {   ret = 0;
    }
    else
    {   ret = 1;
    }

    return ret;
}


/**
  @brief         Checks the consumer node can use this arc
  @param[in]     instance   global registers of this instance
  @param[in]     arc        arc to check
  @return        none

  @par           The arc descriptor gives, in the 2nd word, the stream format used by
                 the node consuming data from this arc.
                 Looking at the amount of data in the buffer and the frame-size
                 consumer by the node (the minimum amount of byte consumed per call), the
                 function returns a go/no-go flag.
  @remark
 */

static uint8_t arc_ready_for_read(arm_stream_instance_t *S, uint32_t *arc, uint32_t *frame_size)
{
    uint32_t consumer_frame_size, consumer_frame_format;   
    uint32_t fifosize, read, write, *all_formats, fifo_data_amount;
    uint8_t ret;

    all_formats = S->all_formats;
    read = RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);
    fifosize = RD(arc[1], BUFF_SIZE_ARCW1);

    consumer_frame_format = all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc[1],CONSUMFMT_ARCW1)];
    consumer_frame_size = RD(consumer_frame_format, FRAMESIZE_FMT0);
    *frame_size = consumer_frame_size;
    fifo_data_amount = write - read;

    if (fifo_data_amount >= consumer_frame_size)
    {   ret = 1;
    }
    else
    {   ret = 0;
    }

    return ret;
}



/**
  @brief         Toolbox of operations on arc
  @param[in]     instance   pointer to the static area of the current Stream instance
  @param[in/out] arc        Pointer to the arc descriptor, which can be modified 
  @param[in]     tag        Command to execute
  @param[in]     size       parameter used for data moves
  @return        none

  @par           "arc_data_operations" implements the data moves to/from arc buffers
                  and the data realignments

  @remark
 */

static void arc_data_operations (
        arm_stream_instance_t *S, 
        uint32_t *arc, 
        uint8_t tag, 
        uint8_t *buffer, 
        uint32_t datasize
        )
{
    uint32_t read;
    uint32_t write;
    uint32_t base;
    uint32_t size;
    uint8_t *long_base;
    uint8_t *src;
    uint8_t* dst;

    //xdm_data = (stream_xdmbuffer_t *)buffer;
    //fifosize =  RD(arc[1], BUFF_SIZE_ARCW1);
    base =      RD(arc[0], BASEIDXOFFARCW0);
    long_base = pack2linaddr_ptr(S->long_offset, base, LINADDR_UNIT_BYTE);

    switch (tag)
    {
    /* data is left shifted to the base address to avoid circular addressing */ 
    /*   or, buffer is empty but R/W are at the end of the buffer => reset/loop the indexes */ 

    case arc_data_realignment_to_base:
        read =  RD(arc[2], READ_ARCW2);
        if (read == 0)
        {   break;      /* buffer is full there is nothing to realign */
        }
        write = RD(arc[3], WRITE_ARCW3);
        size = U(write - read);
        src = &(long_base[read]);
        dst =  long_base;
        MEMCPY (dst, src, (uint32_t)size);

        /* update the indexes Read=0, Write=dataLength */
        ST(arc[2], READ_ARCW2, 0);
        ST(arc[3], WRITE_ARCW3, size);

        /* clear the bit if there is enough free space after this move */
        set_alignment_bit (S, arc);

        /* DMB on RD/WR to let the producers be aware */
        DATA_MEMORY_BARRIER;
    break;

    /* move in and increment write index, update filling status */
    case data_move_to_arc : 
        /* only one node can read the write-index at a time : no collision is possible */
        write = RD(arc[3], WRITE_ARCW3);
        src = buffer;
        dst = &(long_base[write]);
        MEMCPY (dst, src, (uint32_t)datasize);
        write = write + datasize;
        ST(arc[3], WRITE_ARCW3, write);
        set_alignment_bit (S, arc); /* check overflow */
        break;

    /* move out was notified in the driver, do the data move increment read index, 
        update emptiness status */
    case data_moved_from_arc  : 
        /* only one node can update the read-index at a time : no collision is possible */
        read = RD(arc[2], READ_ARCW2);
        src = &(long_base[read]);
        dst = buffer;
        MEMCPY (dst, src, (uint32_t)datasize);
        break;

    case data_swapped_with_arc:
        read = RD(arc[2], READ_ARCW2);
        src = &(long_base[read]);
        dst = buffer;
        MEMSWAP (dst, src, (uint32_t)datasize);
        break;
               
    default : 
        break; 
    }
}


/**
  @brief         Update the arc descriptor after Node processing
  @param[in]     instance   pointer to the static area of the current Stream instance
  @param[in/out] arc        Pointer to the arc descriptor, which can be modified 
  @param[in]     in0out1    0: the arc is an input of the node, 1: an output of the node
  @param[in]     xdm_data   pair of pointer + buffer size
  @return        0 if arcs are not ready for data move => RUN attempt is cancelled

  @par           xdm_data isolates the Node computations from the internal data structure
                 of Stream.

                 Multiprocess realignment of data to base addresses, algorithm :
                 - output FIFO write pointer is incremented AND a check is made for data 
                 re-alignment to base adresses (to avoid address looping)
                 then the "ALIGNBLCK_ARCW3" is set. The SWC don't wait and let the 
                 consumer manage the alignement
                 "Simple" arcs have the exact size of the frame length


  @remark
 */
static uint8_t arc_index_update (arm_stream_instance_t *S, stream_xdmbuffer_t *xdm_data, uint8_t pre0post1)
{
    uint32_t fifosize, read, write;
    uint32_t *arc;
    uint16_t iarc, arcID;
    uint8_t xdm11, ret, narc;
    uint32_t xdm11_frame_size, tmp;      // same frame size between input and output arcs "1 to 1 XDM frame size"
     
    /* all is fine by default */
    ret = 1;        

   
    /* if this is a call to a script : XDM is bytes code + Stream instance + arc */
    if (arm_stream_script_index == (uint16_t)RD(S->swc_header[0], SWC_IDX_LW0))
    {   uint32_t *byte_codes;
        uint32_t *script_offsets;
        uint32_t *arc;

        script_offsets = S->script_offsets;
        arc =  &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & (S->arcID[0]))]);
        byte_codes = &(script_offsets[RD(S->swc_header[0], SCRIPT_LW0)]);

        xdm_data[0].address = (intPtr_t)byte_codes;
        xdm_data[1].address = (intPtr_t)S;
        xdm_data[2].address = (intPtr_t)arc;
        return (ret);
    }


    xdm11 = TEST_BIT(S->swc_header[S->swc_memory_banks_offset+ADDR_LW2], XDM11_LW2_LSB);

    /*
       The arc descriptor gives, in the 2nd word, the stream format used by
       the node consuming data from this arc. On the input streams side, 
       looking at the amount of data in the buffer and the frame-size
       consumed by the node (the minimum amount of byte consumed per call), the
       function returns a go/no-go flag. And on the output streams side, 
       looking at the remaining free space in the buffer and the frame-size
       used by the producer (the minimum amount of byte produced per call), the
       function updates the go/no-go flag.
    */
    xdm11_frame_size = MAXINT32;
    narc = RD((S->swc_header)[0], NBARCW_LW0);

    for (iarc = 0; iarc < narc; iarc++)
    {
        arcID = S->arcID[iarc];
        arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);

        if (ARC_RX0TX1_TEST & arcID)
        {   /* TX arc */
            ret &= arc_ready_for_write(S, arc, &tmp);
            xdm11_frame_size = MIN(xdm11_frame_size, tmp);
        }
        else
        {   /* RX arc */
            ret &= arc_ready_for_read(S, arc, &tmp);
            xdm11_frame_size = MIN(xdm11_frame_size, tmp);
        } 
    }   

    if (0 == ret)
    {   return (ret);     /* arcs are not ready, stop execution */
    }


    for (iarc = 0; iarc < narc; iarc++)
    {   
        arcID = (S->arcID[iarc]);
        arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
        read = RD(arc[2], READ_ARCW2);
        write = RD(arc[3], WRITE_ARCW3);
        fifosize =  RD(arc[1], BUFF_SIZE_ARCW1);

        if (ARC_RX0TX1_TEST & arcID)
        {   /* TX arc case */
            if (0 == pre0post1)
            {   xdm_data[iarc].address = (intPtr_t)(arc_extract_info_pt (S, arc, arc_write_address));

                if (xdm11) /* output buffer of the SWC : size = common with RX and TX */   
                {   xdm_data[iarc].size = xdm11_frame_size;
                }
                else /* output buffer of the SWC : size = free area in the original buffer */  
                {   xdm_data[iarc].size    = arc_extract_info_int (S, arc, arc_free_area);
                }
            }
            else
            {   uint32_t increment;
                /* to save cycles and code size, the SWC can avoid incrementing the pointers */
                if (xdm11) 
                {   increment = xdm11_frame_size;
                }
                else
                {   /* the SWC put the amount of data produced in "size"
                        output buffer of the SWC : check the pointer increment */
                    increment = (uint32_t)(xdm_data->size);
                }
                write = write + increment;
                ST(arc[3], WRITE_ARCW3, write);

                /* set ALIGNBLCK_ARCW3 if (fifosize - write < producer_frame_size) */
                set_alignment_bit (S, arc);

                /* is there debug/monitoring activity to do ? */
                if (RD(arc[2], COMPUTCMD_ARCW2) != 0u)
                { /* @@@ TODO : implement the data monitoring 
                    arm_arc_monitor(*, n, RD(,COMPUTCMD_ARCW2), RD(,DEBUG_REG_ARCW1))
                    implement the pointer loop to base if there is not enough free-space
                    for the next dump 
                   */
                }

                /* check for data flush : done when the consumers are not on the same processor */
                if (U(0) != TEST_BIT(arc[1], MPFLUSH_ARCW1_LSB))
                {   DATA_MEMORY_BARRIER
                }
            }
        } 
        else
        {   /* RX arc case */
            if (0 == pre0post1)
            {   xdm_data[iarc].address = (intPtr_t)(arc_extract_info_pt (S, arc, arc_read_address));

                if (xdm11) /* input buffer of the SWC : size = common with RX and TX */   
                {   xdm_data[iarc].size = xdm11_frame_size;
                }
                else /* input buffer of the SWC : size = data amount in the buffer */
                {   xdm_data[iarc].size    = arc_extract_info_int (S, arc, arc_data_amount);
                }

                /* is there debug/monitoring activity to do ? */
                if (RD(arc[2], COMPUTCMD_ARCW2) != 0u)
                { /* @@@ TODO : implement the data monitoring 
                    arm_arc_monitor(*, n, RD(,COMPUTCMD_ARCW2), RD(,DEBUG_REG_ARCW1))
                    implement the pointer loop to base if there is not enough data
                    for the next read (continuous play of a test pattern) 
                   */
                }
            }
            else
            {   uint32_t increment, producer_frame_size, fmt;
                /* to save code and cycles, the SWC is not incrementing the pointers*/
                if (xdm11) 
                {   increment = xdm11_frame_size;
                }
                else
                {
                    /* the SWC put the amount of data consumed in "size"
                        input buffer of the SWC, update the read index*/
                    increment = (uint32_t)(xdm_data->size);
                }
                read = read + increment;
                ST(arc[2], READ_ARCW2, read);

                /* does data realignement must be done ? : realign and clear the bit */
                fmt = RD(arc,PRODUCFMT_ARCW0) * STREAM_FORMAT_SIZE_W32;
                producer_frame_size = RD(S->all_formats[fmt], FRAMESIZE_FMT0);
                if (write > fifosize - producer_frame_size)
                {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                }
            }
        }
    }

    // DTCM/FastMem in stack : AL gives the address of the the fast memory
    if (TEST_BIT(S->swc_header[S->swc_memory_banks_offset+ADDR_LW2], DTCM_LW2_LSB))
    {   /* push the DTCM address of the processor */
        const p_stream_al_services *al_func;
        uint32_t *fast_mem;
        al_func = &(S->al_services[0]);
        (*al_func)(PACK_AL_SERVICE(0,AL_SERVICE_READ_MEMORY_FAST_MEM_ADDRESS,AL_SERVICE_READ_MEMORY), (uint8_t *)&fast_mem, 0, 0, 0);
        xdm_data[narc].address = (intPtr_t)(fast_mem);
    }
    return ret;
 }

/**
  @brief        copy/swap FIFO to fast memory
  @return       none

  @par          parse memory_segment_swap, extract the arcID (SWAPBUFID_LW2S)
                use the arcID descriptor for the address and length of data to copy/swap
                parameter pre0post1 tells if we are before or after processing
                the field SWAP_LW2S tells to copy or swap the memory segment

  @remark
 */
/* --------------------------------------------------------------------------------------------------
      check input ring buffers :
      each Stream instance has a list of graph boundary to check for in/out data
 */
void load_memory_segments (arm_stream_instance_t *S, uint8_t pre0post1)
{
    uint8_t imem, mask, arcID;
    intPtr_t *memaddr;
    uint32_t *memreq, *arc, memlen;
    
    mask = S->memory_segment_swap;                          /* bit-field of segments to swap */
    memreq = &(S->swc_header[S->swc_memory_banks_offset]);  /* list of memory segments to copy */
    
    for (imem = 0; imem < MAX_NB_MEM_REQ_PER_NODE; imem++)
    {   
        if (TEST_BIT(mask, imem))
        {   memaddr = (intPtr_t *)pack2linaddr_ptr (S->long_offset, memreq[NBW32_MEMREQ_LW2 * imem + ADDR_LW2], LINADDR_UNIT_BYTE);
            arcID = RD(memaddr, SWAPBUFID_LW2S);
            arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
            memlen = RD(arc[1], BUFF_SIZE_ARCW1);

            if (LW2S_COPY == RD(memaddr, SWAP_LW2S))
            {   arc_data_operations (S, arc, data_moved_from_arc, (uint8_t *)memaddr, memlen);
            } 
            else
            {   arc_data_operations (S, arc, data_swapped_with_arc, (uint8_t *)memaddr, memlen);
            } 

            CLEAR_BIT(mask, imem);
            if (mask == 0)  /* does all memory segments are copied ? */
            {   return;
            }
        }
    }      
}


/**
  @brief         Check the Streams at the boundary of the graph
  @param[in]     instance   pointer to the static area of the current Stream instance
  @return        none

  @par           The streams at the boundary of the graph can only be managed by one Stream
                 instance. The graph section dedicated to Stream instances has 2 words/instance.
                 The first word hold a mask of the boundaries allowed to scan for data movement.
                 The second word holds the flags telling a movement request is on-going, and there
                 is no need to ask for more (example of DMA requests on the go).

  @remark
 */
/* --------------------------------------------------------------------------------------------------
      check input ring buffers :
      each Stream instance has a list of graph boundary to check for in/out data
 */
void check_graph_boundaries(arm_stream_instance_t *S)
{
    uint8_t graph_io_idx;
    uint8_t need_data_move;
    uint32_t size;
    uint8_t *buffer;
    uint32_t *arc;
    uint32_t *pio, *pio_base;
    uint32_t io_mask = S->iomask;
    uint32_t nio;
    const p_io_function_ctrl *io_func;

    pio_base = S->pio;
    nio = RD((S->graph)[1],NB_IOS_GR1);

    for (graph_io_idx = 0; graph_io_idx < nio; graph_io_idx++)
    {
        /* jump to next graph port */
        pio = &(pio_base[graph_io_idx * STREAM_IOFMT_SIZE_W32]);

        /* to change when NB_IOS_GR1 allows 32 IOs active from the 128 IOs available on the platform */ 
        if (0 == U(io_mask & (1L << graph_io_idx)))
            continue;
        
        /* a previous request is in process or if the IO is commander on the interface, then no 
            need to ask again */
        if ((0u != TEST_BIT(*pio, ONGOING_IOFMT_LSB)) ||
            (IO_IS_COMMANDER0 == TEST_BIT(*pio, SERVANT1_IOFMT_LSB)))
        {   continue;
        }

        {   uint8_t arc_idx;
            arc_idx = SIZEOF_ARCDESC_W32 * RD(*pio, IOARCID_IOFMT);
            arc = &(S->all_arcs[ARC_RX0TX1_CLEAR & arc_idx]);
        }

        size = 0;

        /* if this is an input stream : check the buffer is empty  */
        if (RX0_TO_GRAPH == TEST_BIT(*pio, RX0TX1_IOFMT_LSB))
        {   need_data_move = arc_ready_for_write(S, arc, &size);
            buffer = arc_extract_info_pt(S, arc, arc_write_address);
            if (size == 0)
            {   continue;   /* look next IO */
            }
        }

        /* if this is an output stream : check the buffer has data */
        else
        {   need_data_move = arc_ready_for_read(S, arc, &size);
            buffer = arc_extract_info_pt(S, arc, arc_read_address);
            if (size == 0)
            {   continue;       /* look next IO */
            }
        }
        
        if (0u != need_data_move)
        {   /*  TODO 
                When the IO is slave and stream_io_domain=PLATFORM_ANALOG_SENSOR_XX 
                 check the time interval from last frame (by a read of the time-stamp in 
                 the FIFO) and current time, to deliver a data rate close to :
                 "platform_io_control.stream_settings_default.SAMPLING_FMT1_"
                Trigger the data request some time ahead to let the converters
                 have the time to exchange data (image, remote temperature sensors,
                 characters on a display, ..).
                Tell data transfer on-going (REQMADE_IO_LSB)
            */
            SET_BIT(*pio, ONGOING_IOFMT_LSB);

            /* fw function index is in the control field, while platform_io[] has all the possible functions */
            io_func = &(S->platform_io[RD(*pio, FWIOIDX_IOFMT)]);

            /* the main application do not give control to data requests skip this IO */
            if (*io_func == 0)
            {   continue;
            }
            (*io_func)(STREAM_RUN, buffer, size);

            /* if this is an input stream : check the buffer needs alignment by the consumer */
            if (RX0_TO_GRAPH == TEST_BIT(*pio, RX0TX1_IOFMT_LSB))
            {   //set_alignment_bit (S, arc);

                /* does data realignement must be done ? : realign and clear the bit */
                if (0u != TEST_BIT (arc[3], ALIGNBLCK_ARCW3_LSB))
                {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                }
            }

        }
    }
}


/*----------------------------------------------------------------------------
/**
  @brief        check_hwsw_compatibility
  @param[in]    none
  @return       none

  @par          Loads the global variable of the platform holding the base addresses 
                to the physical memory banks described in the "platform manifest"

  @remark       
 */
static uint32_t check_hwsw_compatibility(arm_stream_instance_t *S) 
{
    uint8_t match = 1;
    uint32_t whoami = S->whoami_ports;
    uint32_t header = (S->swc_header)[0];

    if (RD(header, ARCHID_LW0) > 0u) /* do we care about the architecture ID ? */
    {   match = (RD(header, ARCHID_LW0) == RD(whoami, ARCHID_PARCH));
    }

    if (RD(header, PROCID_LW0) > 0u) /* do we care about the processor ID ? */
    {   match = match & (RD(header, PROCID_LW0) == RD(whoami, PROCID_PARCH));
    }

    if (RD(header, PRIORITY_LW0) > 0u) /* do we care about the priority ? */
    {   match = match & (RD(header, PRIORITY_LW0) == RD(whoami, PRIORITY_PARCH));
    }

    return match;
}


   
/*----------------------------------------------------------------------------*/
/**
  @brief        Main scheduler loop of the linked list of nodes in the graph
  @param[in]    instance       pointer to the static area of the current Stream instance
  @param[in]    command   Tells if this is a scan for reset, run or stop the graph
  @return       none

  @par          scan the graph and execute a node or all the nodes or all until no more data is available
                algorithm :
                  - scan the linked-list of node
                  - check the input ring buffers at the boundary of the graph are full
                  - check the output ring buffers at the boundary of the graph are empty
                  - search components having enough input data and free space in the ouput buffer
     
  @remark
  @remark
 */


void stream_scan_graph (arm_stream_instance_t *S, int8_t command, uint32_t *data) 
{   
    static long DEBUG_CNT;

    if (script_option & STREAM_SCHD_SCRIPT_START) { script_processing (S->main_script);}

    /* continue from the last position, index in W32 */
    S->linked_list_ptr = &((S->linked_list)[RD(S->whoami_ports, SWC_W32OFF_PARCH)]);

    /* loop until all the components are blocked by the data streams */
	do 
    {  
        /* start scanning the list assuming no data is processed */
        CLEAR_BIT(S->scheduler_control, STILDATA_SCTRL_LSB);

        /* detection of the end of the SWC linked list */
        CLEAR_BIT(S->scheduler_control, ENDLLIST_SCTRL_LSB);

        /* read the linked-list until finding the SWC index "LAST_WORD" */
	    do 
        {   DEBUG_CNT++; if (DEBUG_CNT == 4)
                DEBUG_CNT = DEBUG_CNT;
            
            /* check the boundaries of the graph, not during end/stop periods */
            if (command == STREAM_RUN) 
            {   check_graph_boundaries(S); 
            }
         
            /* read all the information about the Node and the way to set its parameters */
            read_header (S);

            /* does the SWC is executable on this processor */
            if (0U == check_hwsw_compatibility(S))
            {   continue;
            }

            /* does an other process/processor is try to execute the same Node ? */
            if (0 == lock_this_component (S))
            {   continue;
            }

            /* ---------------- parameter was changed, or reset phase ? -------------------- */
            if (command == STREAM_RESET)
            {   reset_component (S);

                /* read the parameter header -   "word 3+n" */
                set_new_parameters (S, &((S->swc_header)[S->swc_parameters_offset]));
            }

            if (command == STREAM_SET_PARAMETER)
            {   set_new_parameters (S, data);
            }

            /* end of graph processing ? */
            if (command == STREAM_STOP)
            {   uint32_t returned;
                ST(S->pack_command, COMMAND_CMD, STREAM_STOP);
                stream_calls_swc (S,
                    S->swc_instance_addr, 0u, &returned);
            }
    
            /* a script called scan_graph for a parameter change */
            if (command == STREAM_SET_PARAMETER)
            {   /* if (/* the "arm_stream_instance_t" has all the details */
                return;
            }

            /* if the SWC generating the input data was blocked (ALIGNBLCK_ARCW3=1) 
                then it is the responsibility of the consumer node (current SWC) to realign the 
                data, and clear the flag. 
            */
            {   uint8_t iarc;
                uint16_t arcID;
                uint32_t *arc;

                for (iarc = 0; iarc < RD(S->swc_header[0], NBARCW_LW0); iarc++)
                {   /* search the RX arcs being blocked in producer's side */
                    arcID = (S->arcID[iarc]);
                    if (0 == (ARC_RX0TX1_TEST & arcID))
                    {   arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
                        if (0u != TEST_BIT (arc[WRIOCOLL_ARCW3], ALIGNBLCK_ARCW3_LSB))
                        {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                        }
                    }
                }
            }

            /* check input arc has enough data and output arc is free */
            if (command == STREAM_RUN) 
            {   run_node (S);
            }
    
            unlock_this_component(S);

            if (return_option == STREAM_SCHD_RET_END_EACH_SWC)
            {   break;
            }

	    }  while (0 == TEST_BIT(S->scheduler_control, ENDLLIST_SCTRL_LSB));

        if ((return_option == STREAM_SCHD_RET_END_ALL_PARSED) || 
            (return_option == STREAM_SCHD_RET_END_EACH_SWC))
        {   CLEAR_BIT(S->scheduler_control, STILDATA_SCTRL_LSB); 
            break;
        }

        if (script_option & STREAM_SCHD_SCRIPT_END_PARSING) {script_processing (S->main_script); }

    } while ((return_option == STREAM_SCHD_RET_END_SWC_NODATA) && 
                (0u != TEST_BIT(S->scheduler_control, STILDATA_SCTRL_LSB)));
}


/**
  @brief         Read one software component description
  @param[in]     instance   pointer to the static area of the current Stream instance

  @return        none

  @par           Data is read from "linked_list_ptr". The function prepares the pointers to
                 the SWC subroutine to call, the pointer to the parameter and preset
                 Format :
                    - Header    (saved in S->swc_header)
                    - Main Instance + nb of memory pointers
                       {other memory pointers}
                    - BootParam : Proc/Arch, preset, parameter length to skip
                    - Parameters (when skip>0) : nbParams, param byte-stream
  @remark
 */


static void read_header (arm_stream_instance_t *S)
{
    uint32_t x;
    uint16_t narc, iarc;
    uint16_t idx_swc;

    S->swc_header = S->linked_list_ptr;                     // linked_list_ptr => HEADER
    narc = RD(S->swc_header[0], NBARCW_LW0);

    S->swc_memory_banks_offset = 1 + ((1 + narc)>>1);       // memreq is at +1(header) +narc/2

    S->swc_parameters_offset = S->swc_memory_banks_offset;
    x = S->swc_header[S->swc_memory_banks_offset];
    S->swc_parameters_offset  += NBW32_MEMREQ_LW2 * RD(x, NBALLOC_LW2);

    /* default parameters of the node */
    S->pack_command = PACK_COMMAND(
        0,      /* TRACEID tag */
        RD((S->swc_header)[S->swc_parameters_offset], PRESET_LW3), /* PRESET */
        narc,   /* number of arcs*/
        RD(S->scheduler_control, BOOT_SCTRL), /* cold/warm boot, SWC knows */
        0);     /* command */

    /* physical address of the instance (descriptor address for scripts) */
    idx_swc = (uint16_t)RD(S->swc_header[0], SWC_IDX_LW0);
    S->swc_instance_addr = (stream_handle_t)pack2linaddr_int
        (S->long_offset, 
         S->swc_header[S->swc_memory_banks_offset],
         LINADDR_UNIT_BYTE
        );

    /* list of memory segment to swap */
    S->memory_segment_swap = (uint8_t)RD(((uint32_t *)S->swc_instance_addr)[SIZE_LW2], TO_SWAP_LW2S);

    /* read the physical address */
    S->address_swc = S->node_entry_point_table[idx_swc];

    /* read the arc indexes */
    for (iarc = 0; iarc < MIN(narc, MAX_NB_STREAM_PER_SWC); iarc+=2)
    {   uint32_t arcs;
        arcs = (S->swc_header)[1+iarc/2];
        S->arcID[iarc]   = (uint16_t)arcs;
        S->arcID[iarc+1] = (uint16_t)(arcs>>16);
    }    

    /* SWC arcs indexes = [0..narc-1], arcs of the associated script = narc (optionaly narc+1) */
    /*S->script_arctx = (uint8_t)(ARC_RX0TX1_CLEAR & ((S->arcID)[narc]));*/

    ///* byte pointer for locking the node, can be the arc 0/1/2/3 ARCLOCK_LW0 has 2bits */
    x = RD(S->swc_header[0], ARCLOCK_LW0);
    iarc = ARC_RX0TX1_CLEAR & S->arcID[x];
    x = WRIOCOLL_ARCW3 + SIZEOF_ARCDESC_W32 * iarc;
    S->pt8b_collision_arc = (uint8_t *)&(S->all_arcs[x]);
    S->pt8b_collision_arc = &(S->pt8b_collision_arc[COLLISION_ARC_OFFSET_BYTE]);

    /* set the linkedList pointer to the next node */
    x = (uint16_t)RD((S->swc_header)[S->swc_parameters_offset], W32LENGTH_LW3) +
        S->swc_parameters_offset;
        
    S->linked_list_ptr = &(S->linked_list_ptr[x]);      // linked_list_ptr => next SWC.

    /* check for a rewind to the start of the list (end = SWC index 0b11111..111) */
    if (GRAPH_LAST_WORD == RD(*S->linked_list_ptr,SWC_IDX_LW0))
    {   /* rewind the index when the last word is detected */
        SET_BIT(S->scheduler_control, ENDLLIST_SCTRL_LSB);
        S->linked_list_ptr = S->linked_list;
    } 

    /* save the position in word32 */
    ST(S->whoami_ports, SWC_W32OFF_PARCH, (uint32_t)(S->linked_list_ptr - S->linked_list));
}


/**
  @brief         Lock this component against collisions with other process/processors
  @param[in]     instance   pointer to the static area of the current Stream instance

  @return        none

  @par           The algorithm consists in the detection of collisions. 
                 If a collision is detected escape and let it be executed by the others.
                 The reservation flag is on the production arc, word_3. The memory access
                 is made with a Byte address pointer (no mask).
  @remark
 */

static uint8_t lock_this_component (arm_stream_instance_t *S)
{
    //uint8_t tmp;
    //uint8_t check;
    const p_stream_al_services *al_func;

    al_func = &(S->al_services[0]);

    /* ---------------------SWC reservation attempt -------------------*/            
    /* if the SWC is already used skip it */
    //(*al_func)(PACK_AL_SERVICE(0,AL_SERVICE_MUTUAL_EXCLUSION_RD_BYTE_MP,AL_SERVICE_MUTUAL_EXCLUSION), S->pt8b_collision_arc, &check, &tmp, 0);
    //if (check != U8(0))
    //{   return 0;
    // }
 
    //tmp = RD(S->whoami_ports, INST_ID_PARCH);  
    //(*al_func)(PACK_AL_SERVICE(0,AL_SERVICE_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP,AL_SERVICE_MUTUAL_EXCLUSION), S->pt8b_collision_arc, &check, &tmp, 0);

    //if (0 == check)
    //{   return 0;   /* a collision occured, don't wait, jump to next nanoAppRT */
    //}
    /* ------------------SWC is now locked for me ! --------------------*/            

    return 1;
}

static uint8_t unlock_this_component (arm_stream_instance_t *S)
{
    const p_stream_al_services *al_func = &(S->al_services[0]);
    uint8_t tmp = 0;

    (*al_func)(PACK_SERVICE(0,0,AL_SERVICE_MUTUAL_EXCLUSION_WR_BYTE_MP,AL_SERVICE_MUTUAL_EXCLUSION), 
        S->pt8b_collision_arc, &tmp, 0, 0);

    return 1;
}


/**
  @brief         Prepare the instance of the component at Reset time
  @param[in]     instance   pointer to the static area of the current Stream instance

  @return        none

  @par           Nodes must not be able to read the graph. A temporary buffer is used to store
                 the pointer to memery required by the node (described in the "Node Manifest"
                 used by the compilation tool "graph2bin"). The addresses are precomputed in the 
                 graph, close to the component index. The temporary buffer holds the data format
                 of the input and output arcs

  @remark
 */

static void reset_component (arm_stream_instance_t *S)
{
    uint8_t nbmem;
    uint8_t imem, j, iformat, narc;
    uint32_t *memreq, check;
    
    #define MEMRESET (MAX_NB_MEM_REQ_PER_NODE + (STREAM_FORMAT_SIZE_W32*MAX_NB_STREAM_PER_SWC))
    intPtr_t memreq_physical[MEMRESET];
    
    memreq = &(S->swc_header[S->swc_memory_banks_offset]);
    nbmem = (uint8_t)RD(memreq[0],NBALLOC_LW2);
    memreq_physical[0] = (intPtr_t)(uint64_t)(S->swc_instance_addr);

    for (imem = 1; imem < nbmem; imem++)
    {   /* create pointers to the right memory bank */
        memreq_physical[imem] = pack2linaddr_int(S->long_offset, memreq[NBW32_MEMREQ_LW2 * imem], LINADDR_UNIT_BYTE);
    }      

    if (TEST_BIT(S->swc_header[0], LOADFMT_LW0_LSB))
    {   /* push the FORMAT of the arcs */
        narc = MIN(MAX_NB_STREAM_PER_SWC, RD(S->swc_header[0], NBARCW_LW0));

        for (j = 0; j < narc; j++)
        {   uint32_t *F, ifmt;
            if (ARC_RX0TX1_TEST & (S->arcID)[j])        // is it a TX arc ? push the "producer" format 
            {   ifmt = STREAM_FORMAT_SIZE_W32 * 
                    (ARC_RX0TX1_CLEAR & RD((S->arcID)[j],PRODUCFMT_ARCW0));
            }
            else
            {   ifmt = STREAM_FORMAT_SIZE_W32 *         // is it a RX arc ? push the "consumer" format 
                    (ARC_RX0TX1_CLEAR & RD((S->arcID)[j],CONSUMFMT_ARCW1));
            }

            F = &(S->all_formats[ifmt]);
            for (iformat = 0; iformat < STREAM_FORMAT_SIZE_W32; iformat++)
            {   memreq_physical[imem++] = F[iformat]; 
            }
        }      
    }

    /* reset the component with the parameter TraceID (6bits) */
    ST(S->pack_command, COMMAND_CMD, STREAM_RESET);
    ST(S->pack_command, SWC_TAG_CMD, RD((S->swc_header)[S->swc_parameters_offset], TRACEID_LW3));

    stream_calls_swc (S,
        (stream_handle_t) memreq_physical, 
        (stream_xdmbuffer_t *) arm_stream_services, 
        &check);
}


/**
  @brief         Set new parameters
  @param[in]     instance   pointer to the static area of the current Stream instance
                 ptr_param32b : 32bits-aligned section of parameters (32 TAGS + 
                    parameters following, see PARAM_TAG_LW3)
  @return        none
    
  @remark
 */

static void set_new_parameters (arm_stream_instance_t *S, uint32_t *ptr_param32b)
{
    uint32_t tmp;
    int status;

    /*
        BOOTPARAMS: 
        PARAM_TAG : 4  index to parameter (0='all parameters')
        PRESET    : 4  preset index (SWC delivery)
        TRACEID   : 8  
        W32LENGTH :16  nb of WORD32 to skip at run time, 0 means NO PARAMETER, max=256kB
    */

    if (1 < RD((S->swc_header)[S->swc_parameters_offset], W32LENGTH_LW3))
    {
        /* change the SWC command to "Set Parameter" */
        ST(S->pack_command, COMMAND_CMD, STREAM_SET_PARAMETER);

        tmp = RD(*ptr_param32b, PARAM_TAG_LW3); /* copy the param_tag to swc_tag for parameter index */
        ST(S->pack_command, SWC_TAG_CMD, tmp);
        ptr_param32b++;

        stream_calls_swc (S, 
                S->swc_instance_addr,
                (stream_xdmbuffer_t *)ptr_param32b, 
                &status);
    }
}


/**
  @brief         Execution of a Node
  @param[in]     instance   pointer to the static area of the current Stream instance

  @return        none

  @par           The nodes must not have access to the graph area. A temporary buffer is used
                 to load pairs of "pointers + size". For input arcs this is the arc read address, 
                 for output arcs this is the arc write address.

  @remark
 */

static void run_node (arm_stream_instance_t *S)
{
    stream_xdmbuffer_t xdm_data[MAX_NB_STREAM_PER_SWC];
    uint32_t check;
    uint8_t loop_counter;
  
    /* push all the ARCs on the stack and check arcs buffer are ready */
    if (0 == arc_index_update(S, xdm_data, 0))
    {   return; /* buffers are not ready */
    }

    /* flag : still one component is processing data in the graph */
    SET_BIT(S->scheduler_control, STILDATA_SCTRL_LSB);
   
    ST(S->pack_command, COMMAND_CMD, STREAM_RUN);

    /* check memory segments for copy to TCM/fast memory @@@*/
    if (S->memory_segment_swap)
    {   load_memory_segments (S, 0);
    }

    loop_counter = MAX_SWC_REPEAT;
    do 
    {
        /* call the SWC, returns the information "0 == SWC needs to be called again" 
           to give some CPU periods for trigering data moves 
           long SWC can be split to allow data moves without RTOS */
        stream_calls_swc (S,
            S->swc_instance_addr, xdm_data,  &check);
    } 
    while ((check == TASKS_NOT_COMPLETED) && ((--loop_counter) > 0));
    
    /*  output FIFO write pointer is incremented AND a check is made for data 
        re-alignment to base adresses (to avoid address looping)
        The SWC don't wait and let the consumer manage the alignement 
    */
    arc_index_update(S, xdm_data, 1); 

    /* check memory segments for restore / swap back */
    if (S->memory_segment_swap)
    {   load_memory_segments (S, 1);
    }    

    if (script_option & STREAM_SCHD_SCRIPT_AFTER_EACH_SWC) {script_processing (S->main_script);}
}

 

#ifdef __cplusplus
}
#endif
