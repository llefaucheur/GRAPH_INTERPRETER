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
    
#include <stdint.h>
#include "presets.h"
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      /* graph list */
#include "stream_types.h"
#include "stream_extern.h"


static void read_header (arm_stream_instance_t *S);
static void reset_component (arm_stream_instance_t *S);
static uint8_t lock_this_component (arm_stream_instance_t *S);
static uint8_t unlock_this_component (arm_stream_instance_t *S);
static void set_reset_parameters (arm_stream_instance_t *S, uint32_t *ptr_param32b);
static void upload_new_parameters (arm_stream_instance_t *S);

static void run_node (arm_stream_instance_t *S);
static uint8_t arc_ready_for_write(arm_stream_instance_t *S, uint32_t *arc, uint32_t *frame_size);
static uint8_t arc_ready_for_read(arm_stream_instance_t *S, uint32_t *arc, uint32_t *frame_size);
static intptr_t arc_extract_info_int (uint32_t *arc, uint8_t tag);
static void load_clear_memory_segments (arm_stream_instance_t *S, uint8_t pre0post1);
static void check_graph_boundaries(arm_stream_instance_t *S);

#define script_option (RD(S->scheduler_control, SCRIPT_SCTRL_HW1))
#define return_option (RD(S->scheduler_control, RETURN_SCTRL_HW1))


/*----------------------------------------------------------------------------
   execute a node
   this is the only place where a node is called for safety reason 
   node can check the return-address is identical
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

static void stream_calls_node (arm_stream_instance_t *S,
                    void *instance, 
                    void *data,
                    uint32_t *parameter
                   )
{
    /* node execution is starting */
    ST(S->scheduler_control, NODEEXEC_SCTRL, 1);

    /* xdm_buffer pointers are updated inside the component */
    S->address_node (S->pack_command, instance, data, parameter);

    /* node execution is finished */
    ST(S->scheduler_control, NODEEXEC_SCTRL, 0);
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
static intptr_t pack2linaddr_int(uint32_t x, uint8_t ** LL)
{
    uint32_t R;
#if 0
    {   uint8_t *long_base;                                     
        uint8_t extend;                                         
        int32_t signed_base;                                    
                                                            
        long_base = LL[x];                                       
        signed_base = RD(x, SIGNED_SIZE_FMT0);                  
        signed_base = signed_base << (32-SIGNED_SIZE_FMT0_MSB); 
        signed_base = signed_base >> (32-SIGNED_SIZE_FMT0_MSB); 
        extend = (uint8_t)RD(x, EXTENSION_FMT0);                
        signed_base <<= (extend << 1);                          
        R = (intptr_t)(&(long_base[signed_base]));              
    }
#else
    PACK2LIN(R,LL,x);
#endif
    return R;  
}

static void * pack2linaddr_ptr(uint32_t data, uint8_t ** long_offset)
{
    return (void *) (pack2linaddr_int(data, long_offset));
}

/**
  @brief        debug script 
  @param[in]    int        index of the script to run
  @param[in]    int        parameter :
                    #define SCRIPT_PRERUN 1         executed before calling the node : the Z flag is set
                    #define SCRIPT_POSTRUN 2        executed after
  @return       inPtr_t    address in the format of the processor

  @par          execution depends on "S->script_option" scheduler configuration :
    STREAM_SCHD_SCRIPT_BEFORE_EACH_NODE  script is called before each NODE called 
    STREAM_SCHD_SCRIPT_AFTER_EACH_NODE   script is called after each NODE called 
    STREAM_SCHD_SCRIPT_END_PARSING      script is called at the end of the loop 
    STREAM_SCHD_SCRIPT_START            script is called when starting 
    STREAM_SCHD_SCRIPT_END              script is called before return 

 */

/*----------------------------------------------------------*/
static void script_processing(uint32_t script_index, uint32_t parameter)
{   // @@@@@@
    if (script_index == parameter)
        parameter = parameter;
}


/**
  @brief         Tool box : Arc descriptor fields extraction, returns an integer
  @param[in]     arc        pointer to the arc descriptor to read
  @param[in]     command    operation to do
  @return        int

  @par           Arcs are 4-words packed structures
                 arc_extract_info_int returns the amount of data or free area, ..
  @remark
 */

static intptr_t arc_extract_info_int (uint32_t *arc, uint8_t tag)
{
    uint32_t read;
    uint32_t write;
    uint32_t size;
    intptr_t ret;

    read =  RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);
    size =  RD(arc[1], BUFF_SIZE_ARCW1);

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
  @brief         Arc descriptor fields extraction, returns a byte pointer
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
    uint8_t *long_base;
    uint8_t *ret;

    PACK2LIN(read, (S->long_offset), arc[0]);
    long_base = (uint8_t *) read;

    read =  RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);

    switch (tag)
    {
    case arc_read_address : ret = &(long_base[read]); 
        break;
    case arc_write_address: ret = &(long_base[write]); 
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
    uint32_t i;

    fifosize =  RD(arc[1], BUFF_SIZE_ARCW1);
    write = RD(arc[3], WRITE_ARCW3);

    /* does the write index is already far, to ask for data realignment? */
    i = (uint8_t) RD(arc[4],PRODUCFMT_ARCW4);
    i = i * STREAM_FORMAT_SIZE_W32;
    producer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

    /* the consumer reset this bit after data realignment */
    if (fifosize < producer_frame_size + write)
    {   SET_BIT(arc[2], ALIGNBLCK_ARCW2_LSB);
    }
    else
    {   CLEAR_BIT(arc[2], ALIGNBLCK_ARCW2_LSB);
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
    uint32_t fifosize, write, *all_formats, i;

    all_formats = S->all_formats;
    write = RD(arc[3], WRITE_ARCW3);
    fifosize = RD(arc[1], BUFF_SIZE_ARCW1);
  
    i = STREAM_FORMAT_SIZE_W32 * RD(arc[4],PRODUCFMT_ARCW4);
    producer_frame_size = RD(all_formats[i], FRAMESIZE_FMT0);

    *free_for_writes =  (uint32_t)(fifosize - write); /* memory available for writes */

    if (write + producer_frame_size > fifosize)
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
    uint32_t fifosize, read, write, *all_formats;
    uint8_t ret;

    all_formats = S->all_formats;
    read = RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);
    fifosize = RD(arc[1], BUFF_SIZE_ARCW1);

    consumer_frame_format = all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc[4u],CONSUMFMT_ARCW4)];
    consumer_frame_size = RD(consumer_frame_format, FRAMESIZE_FMT0);
    *frame_size = (uint32_t)(write - read);     /* size of data ready for read */

    if (*frame_size >= consumer_frame_size)
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
    uint32_t size;
    uint8_t *long_base;
    uint8_t *src;
    uint8_t* dst;

    PACK2LIN(read, (S->long_offset), arc[0]);
    long_base = (uint8_t *) read;

    switch (tag)
    {
    /* data is left shifted to the base address to avoid circular addressing */ 
    /*   or, buffer is empty but R/W are at the end of the buffer => reset/loop the indexes */ 

    case arc_data_realignment_to_base:
        read =  RD(arc[2], READ_ARCW2);
        if (read == 0u)
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
        {   uint32_t loop;
            uint8_t x;
            for (loop = 0; loop < datasize; loop++, dst++, src++)
            {   x = *dst; *dst = *src; *src = x;
            }
        }
        break;
               
    default : 
        break; 
    }

    /* flush the cache and the memory barriers for buffers used with multiprocessing */
    if (0 != TEST_BIT(arc[2], MPFLUSH_ARCW2_LSB))
    {   DATA_MEMORY_BARRIER
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
                 then the "ALIGNBLCK_ARCW3" is set. The NODE don't wait and let the 
                 consumer manage the alignement
                 "Simple" arcs have the exact size of the frame length


  @remark
 */
static uint8_t arc_index_update (arm_stream_instance_t *S, stream_xdmbuffer_t *xdm_data, uint8_t pre0post1)
{
    uint32_t fifosize, read, write;
    uint32_t *arc;
    uint32_t iarc, arcID;
    uint8_t ret, narc, updates_made;
    uint32_t tmp;      // same frame size between input and output arcs "1 to 1 XDM frame size"
     
    /* all is fine by default */
    ret = 1;        
    updates_made = 0;
   
    /* if this is a call to a script : XDM is bytes code + Stream instance + arc */
    if (arm_stream_script_index == RD(S->node_header[0], NODE_IDX_LW0))
    {   //uint32_t *byte_codes;
        uint32_t *buffer;

        arc =  &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & (uint32_t)(S->arcID[0]))]);
        buffer = (uint32_t *)arc_extract_info_pt(S, arc, arc_read_address);
        xdm_data[0].address = (intptr_t)S;      xdm_data[0].size = 0;
        xdm_data[1].address = (intptr_t)arc;    xdm_data[1].size = 0;
        xdm_data[2].address = (intptr_t)buffer; xdm_data[2].size = 0;
        return (ret);
    }

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

    narc = (uint8_t) RD((S->node_header)[0], NBARCW_LW0);

    for (iarc = 0; iarc < narc; iarc++)
    {   uint8_t arc_ready, hqos;

        arcID = S->arcID[iarc];
        arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
        hqos = RD(arc[3], HIGH_QOS_ARCW2);

        if (ARC_RX0TX1_TEST & arcID)
        {   arc_ready = arc_ready_for_write(S, arc, &tmp);  /* TX arc */
        }
        else
        {   arc_ready = arc_ready_for_read(S, arc, &tmp);   /* RX arc */
        } 

        if (arc_ready != 0 && hqos != 0)    /* if high QoS arc with data     */
        {   ret = 1;                        /* then force a call to the node */
            break;
        } 
        else
        {   ret = ret & arc_ready;  /* else consolidate decision on all arcs */
        }
    }   

    if ((0u == pre0post1) && (0u == ret))
    {   return (ret);     /* arcs are not ready, stop execution */
    }


    for (iarc = 0u; iarc < narc; iarc++)
    {   
        arcID = (S->arcID[iarc]);
        arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
        read = RD(arc[2], READ_ARCW2);
        write = RD(arc[3], WRITE_ARCW3);
        fifosize =  RD(arc[1], BUFF_SIZE_ARCW1);

        if (ARC_RX0TX1_TEST & arcID)        /* is it a TX arc ? */
        {   if (0u == pre0post1)
            {   xdm_data[iarc].address = (intptr_t)(arc_extract_info_pt (S, arc, arc_write_address));
                xdm_data[iarc].size    = arc_extract_info_int (arc, arc_free_area);
            }
            else /* TX - post-processing */
            {   /* the NODE put the amount of data produced in "size"
                        output buffer of the NODE : check the pointer increment */
                write = write + (uint32_t)(xdm_data[iarc].size);
                ST(arc[3], WRITE_ARCW3, write);

                /* set ALIGNBLCK_ARCW3 if (fifosize - write < producer_frame_size) */
                set_alignment_bit (S, arc);

                updates_made = 1;   /*  modification of the descriptor */

                /* is there debug/monitoring activity to do ? */
                if (RD(arc[4], COMPUTCMD_ARCW4) != 0u)
                { /* @@@ TODO : implement the data monitoring 
                    arm_arc_monitor(*, n, RD(,COMPUTCMD_ARCW4), RD(,DEBUG_REG_ARCW4))
                    implement the pointer loop to base if there is not enough free-space
                    for the next dump 
                   */
                }
            }
        } 
        else                                /* RX arc case */
        {   if (0u == pre0post1)
            {   
                /* if the NODE generating the input data was blocked (ALIGNBLCK_ARCW3=1) 
                    then it is the responsibility of the consumer node (current SWC) to realign the 
                    data, and clear the flag. 
                */
                if (TEST_BIT (arc[2], ALIGNBLCK_ARCW2_LSB))
                {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                }            
            
                xdm_data[iarc].address = (intptr_t)(arc_extract_info_pt (S, arc, arc_read_address));
                xdm_data[iarc].size    = arc_extract_info_int (arc, arc_data_amount);

                /* is there debug/monitoring activity to do ? */
                if (RD(arc[4], COMPUTCMD_ARCW4) != 0u)
                { /* @@@ TODO : implement the data monitoring 
                    arm_arc_monitor(*, n, RD(,COMPUTCMD_ARCW4), RD(,DEBUG_REG_ARCW4))
                    implement the pointer loop to base if there is not enough data
                    for the next read (continuous play of a test pattern) 
                   */
                }
            }
            else /* RX - post-processing */
            {   uint32_t producer_frame_size, fmt;
                /* to save code and cycles, the NODE is not incrementing the pointers*/
                /* the NODE put the amount of data consumed in "size"
                        input buffer of the SWC, update the read index*/
                read = read + (uint32_t)(xdm_data[iarc].size);
                ST(arc[2], READ_ARCW2, read);

                updates_made = 1;   /*  modification of the descriptor */

                /* does data realignement must be done ? : realign and clear the bit */
                fmt = RD(arc[4],PRODUCFMT_ARCW4) * STREAM_FORMAT_SIZE_W32;
                producer_frame_size = RD(S->all_formats[fmt], FRAMESIZE_FMT0);
                if (write > U(fifosize - producer_frame_size))
                {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                }
            }
        }

        /* check for data flush : done when the consumers are not on the same processor */
        if (updates_made & TEST_BIT(arc[2], MPFLUSH_ARCW2_LSB))
        {   DATA_MEMORY_BARRIER
        }
    }


    /* if critical fast memory is relocatable => use STREAM_UPDATE_RELOCATABLE  
    *  from external script
    */

    return ret;
 }

/**
  @brief        copy/swap FIFO to fast memory
  @return       none

  @par          parse memory_segment_swap
                use the arcID descriptor for the address and length of data to copy/swap
                parameter pre0post1 tells if we are before or after processing
                the field SWAP_LW2S tells to copy or swap the memory segment

  @remark
 */
/* --------------------------------------------------------------------------------------------------
      check input ring buffers :
      each Stream instance has a list of graph boundary to check for in/out data

      CLEARSWAP_LW2S is set => at least one memory segment needs a clear or a swap
 */
void load_clear_memory_segments (arm_stream_instance_t *S, uint8_t pre0post1)
{
    uint8_t imem;
    intptr_t *memaddr;
    uint32_t *memreq, memlen, memReqWord2;
    
    imem = pre0post1;   // provision for swap postprocessing @@@@

    memreq = &(S->node_header[S->node_memory_banks_offset]);  /* list of memory segments to copy */
    
    for (imem = 0; imem < MAX_NB_MEM_REQ_PER_NODE; imem++)
    {   
        memaddr = (intptr_t *)pack2linaddr_ptr (memreq[NBW32_MEMREQ_LW2 * imem + ADDR_LW2], (uint8_t **)S->long_offset);
        memReqWord2 = memreq[NBW32_MEMREQ_LW2 * imem + SIZE_LW2];

        /* swap memory with the arc buffer "SWAPBUFID" */ 
        if (TEST_BIT(memReqWord2, SWAP_LW2S_LSB))
        {   uint8_t arcID;
            uint32_t *arc;

            arcID = (uint8_t) RD(memaddr, SWAPBUFID_LW2S);
            arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
            memlen = RD(arc[1], BUFF_SIZE_ARCW1);

            arc_data_operations (S, arc, data_swapped_with_arc, (uint8_t *)memaddr, memlen);
        }

        /* clear memory */
        if (TEST_BIT(memreq[NBW32_MEMREQ_LW2 * imem + SIZE_LW2], CLEAR_LW2S_LSB))
        {   memlen = RD(memreq[NBW32_MEMREQ_LW2 * imem + SIZE_LW2], BUFF_SIZE_LW2S);
            MEMSET(memaddr, 0, memlen);
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
static void check_graph_boundaries(arm_stream_instance_t *S)
{
    uint8_t graph_io_idx;
    uint8_t need_data_move;
    uint32_t size;
    //uint32_t offset_to_stream_io;
    uint8_t *buffer;
    uint32_t *arc;
    uint8_t *ongoing;
    uint32_t *pio_control;
    const p_io_function_ctrl *io_func;
static int dbgc;

    //nio = RD((S->graph)[1],NB_IOS_GR1);

    for (graph_io_idx = 0; graph_io_idx < S->nb_graph_io; graph_io_idx++)
    {
        /* jump to next graph port */
        //offset_to_stream_io = GRAPH_HEADER_NBWORDS + RD((S->graph)[0], NBHWIOIDX_GR0);
        //pio_control = &(S->graph[offset_to_stream_io + graph_io_idx * STREAM_IOFMT_SIZE_W32]);
        pio_control = &(S->pio_graph[graph_io_idx * STREAM_IOFMT_SIZE_W32]);
        ongoing = &(S->ongoing[graph_io_idx]);

        if (0u == U(S->iomask & ((const uint64_t)1 << graph_io_idx)))
            continue;
        
        /* a previous request is in process or if the IO is commander on the interface, then no 
            need to ask again */
        if ((TEST_BIT(*ongoing, ONGOING_IO_LSB)) ||
            (IO_IS_COMMANDER0 == TEST_BIT(*pio_control, SERVANT1_IOFMT0_LSB)))
        {   continue;
        }

        {   uint8_t arc_idx;
            arc_idx = (uint8_t)(ARC_RX0TX1_CLEAR & RD(*pio_control, IOARCID_IOFMT0));
            arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * arc_idx]);
        }

        size = 0;

        /* if this is an input stream : check the buffer is empty  */
        if (RX0_TO_GRAPH == TEST_BIT(*pio_control, RX0TX1_IOFMT0_LSB))
        {   need_data_move = arc_ready_for_write(S, arc, &size);
            buffer = arc_extract_info_pt(S, arc, arc_write_address);
            if (size == 0u) /* size free for writes = 0 ? */
            {   continue;   /* look next IO */
            }
        }

        /* if this is an output stream : check the buffer has data */
        else
        {   need_data_move = arc_ready_for_read(S, arc, &size);
            buffer = arc_extract_info_pt(S, arc, arc_read_address);
            if (size == 0u)     /* size free for read = 0 ? */
            {   continue;       /* look next IO */
            }
        }
        
        if (0u != need_data_move)
        {   stream_xdmbuffer_t pt_pt;
        
            /*  TODO 
                When the IO is slave and stream_io_domain=PLATFORM_ANALOG_SENSOR_XX 
                 check the time interval from last frame (by a read of the time-stamp in 
                 the FIFO) and current time, to deliver a data rate close to :
                 "platform_io_control.stream_settings_default.SAMPLING_FMT1_"
                Trigger the data request using script and delta-time read
                MP(A+M) 
                Tell data transfer on-going (REQMADE_IO_LSB)
            */
            SET_BIT(*ongoing, ONGOING_IO_LSB);
            
            /* fw function index is in the control field, while platform_io[] has all the possible functions */
            io_func = &(S->platform_io[RD(*pio_control, FWIOIDX_IOFMT0)]);

            /* the main application do not give control to data requests skip this IO */
            if (*io_func == 0u)
            {   continue;
            }

            /* io_func has its internal buffer, don't give the final destination */
            if (IO_COMMAND_SET_BUFFER != RD(*pio_control, SET0COPY1_IOFMT0))
            {   buffer = 0;
            }

            /* io_func : data move + io_stream notification */
            pt_pt.address = (intptr_t)buffer; pt_pt.size = 0;
            (*io_func)(STREAM_RUN, &pt_pt);

            /* if this is an input stream : check the buffer needs alignment by the consumer */
            if (RX0_TO_GRAPH == TEST_BIT(*pio_control, RX0TX1_IOFMT0_LSB))
            {   //set_alignment_bit (S, arc);

                /* does data realignement must be done ? : realign and clear the bit */
                if (TEST_BIT (arc[2], ALIGNBLCK_ARCW2_LSB))
                {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                }
            }

        }
    }
}


/*----------------------------------------------------------------------------
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
    uint32_t whoami = S->scheduler_control;
    uint32_t header = (S->node_header)[0];

    if (RD(header, ARCHID_LW0) > 0u) /* do we care about the architecture ID ? */
    {   match = (uint8_t)(RD(header, ARCHID_LW0) == RD(whoami, ARCHID_LW0));
    }

    if (RD(header, PROCID_LW0) > 0u) /* do we care about the processor ID ? */
    {   match = match & (RD(header, PROCID_LW0) == RD(whoami, PROCID_LW0));
    }

    if (RD(header, PRIORITY_LW0) > 0u) /* do we care about the priority ? */
    {   match = match & (RD(header, PRIORITY_LW0) == RD(whoami, PRIORITY_LW0));
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


void stream_scan_graph (arm_stream_instance_t *S, int8_t command, uintptr_t data) 
{   
    static long DEBUG_CNT;

    /* parameter change : call from a script or from  arm_graph_interpreter(STREAM_SET_PARAMETER...) 
        data -> line index of the node in the graph
        parameters are in the global list (in RAM) of [node idx; parameter address]..[0;0] 
    */
    if (command == STREAM_SET_PARAMETER)
    {   uint32_t *backup_linked_list_ptr;
        backup_linked_list_ptr = S->linked_list_ptr;            // save position of the scheduler
        S->linked_list_ptr = &((S->linked_list)[data]);         // point to the node to update
        read_header(S);                                         // read the header of the node to update
        SET_BIT((S->pt8b_collision_arc) [-7], NEW_PARAM_ARCW2_LSB); // point to the previous W32 "RDFLOW_ARCW2"
        S->linked_list_ptr = backup_linked_list_ptr;            // restore original position of the scheduler
        return;
    }


    //if (script_option & STREAM_SCHD_SCRIPT_START) { script_processing (S->main_script, 0);}

    /* continue from the last position, index in W32 */
    S->linked_list_ptr = &((S->linked_list)[RD(S->link_offset, NODE_LINK_W32OFF)]);


    /* loop until all the components are blocked by the data streams */
	do 
    {  
        /* start scanning the list assuming no data is processed */
        CLEAR_BIT(S->scheduler_control, STILDATA_SCTRL_LSB);

        /* detection of the end of the NODE linked list */
        CLEAR_BIT(S->scheduler_control, ENDLLIST_SCTRL_LSB);

        /* read the linked-list until finding the NODE index "LAST_WORD" */
	    do 
        {   DEBUG_CNT++; if (DEBUG_CNT == 3)
                DEBUG_CNT = DEBUG_CNT;
            
            /* check the boundaries of the graph, not during end/stop periods */
            if (command == STREAM_RUN) 
            {   check_graph_boundaries(S); 
            }
         
            /* read all the information about the Node and the way to set its parameters */
            read_header (S);
            if (S->idx_node == 0)
            {   continue;
            }

            /* does the NODE is executable on this processor */
            if (0U == check_hwsw_compatibility(S))
            {   continue;
            }

            /* does an other process/processor is trying to execute the same Node ? */
            if (0u == lock_this_component (S))
            {   continue;
            }

            /* ---------------- parameter was changed, or reset phase ? -------------------- */
            if (command == STREAM_RESET)
            {   reset_component (S);

                /* read the parameter */
                set_reset_parameters (S, &((S->node_header)[S->node_parameters_offset]));
            }


            /* end of graph processing ? */
            if (command == STREAM_STOP)
            {   uint32_t returned;
                ST(S->pack_command, COMMAND_CMD, STREAM_STOP);
                stream_calls_node (S, S->node_instance_addr, 0u, &returned);
            }

            /* check input arc has enough data and output arc is free */
            if (command == STREAM_RUN) 
            {    run_node (S);
            }
    
            unlock_this_component(S);

            if (return_option == STREAM_SCHD_RET_END_EACH_NODE)
            {   break;
            }

	    }  while (0u == TEST_BIT(S->scheduler_control, ENDLLIST_SCTRL_LSB));

        if ((return_option == STREAM_SCHD_RET_END_ALL_PARSED) || 
            (return_option == STREAM_SCHD_RET_END_EACH_NODE))
        {   CLEAR_BIT(S->scheduler_control, STILDATA_SCTRL_LSB); 
            break;
        }

        //if (script_option & STREAM_SCHD_SCRIPT_END_PARSING) {script_processing (S->main_script, 0); }

    } while ((return_option == STREAM_SCHD_RET_END_NODE_NODATA) && 
                (TEST_BIT(S->scheduler_control, STILDATA_SCTRL_LSB)));
}


/**
  @brief         Read one software component description
  @param[in]     instance   pointer to the static area of the current Stream instance

  @return        none

  @par           Data is read from "linked_list_ptr". The function prepares the pointers to
                 the NODE subroutine to call, the pointer to the parameter and preset
                 Format :
                    - Header    (saved in S->node_header)
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
    uint8_t TX_found;

    S->node_header = S->linked_list_ptr;                             // linked_list_ptr => HEADER
    narc = (uint8_t) RD(S->node_header[0], NBARCW_LW0);

    S->node_memory_banks_offset = (uint8_t)(1u + ((1u + narc)>>1u)); // memreq is at +1(header) +narc/2

    S->node_parameters_offset = S->node_memory_banks_offset;

    x = RD(S->node_header[0], NALLOCM1_LW0);
    S->node_parameters_offset = S->node_parameters_offset + (uint8_t)(NBW32_MEMREQ_LW2 * (1u + x));
    
    if (0 != RD(S->node_header[1], KEY_LW1))
    {   S->node_parameters_offset += NBWORDS_KEY_USER_PLATFORM;
    }

    /* default parameters of the node */
    S->pack_command = PACK_COMMAND(
        0,      /* TRACEID tag */
        RD((S->node_header)[S->node_parameters_offset], PRESET_LW4), /* PRESET */
        narc,   /* number of arcs*/
        RD(S->scheduler_control, BOOT_SCTRL), /* cold/warm boot, NODE knows */
        0);     /* command */

    /* physical address of the instance (descriptor address for scripts) */
    S->node_instance_addr = (stream_handle_t)pack2linaddr_int(S->node_header[S->node_memory_banks_offset], (uint8_t **)S->long_offset);

    /* read the arc indexes */
    TX_found = 0;
    for (iarc = 0; iarc < MIN(narc, MAX_NB_STREAM_PER_NODE); iarc+=2)
    {   
        S->arcID[iarc]   = (uint16_t)((S->node_header)[1+iarc/2]);
        S->arcID[iarc+1u]= (uint16_t)(((S->node_header)[1+iarc/2]) >> 16);

        if (TX_found == 0)
        {   if (ARC_RX0TX1_TEST & S->arcID[iarc])
            {   TX_found = 1;
                /* the first TX arc holds the byte pointer for locking the node */
                x = WRIOCOLL_ARCW3 + SIZEOF_ARCDESC_W32 * iarc;  
                S->pt8b_collision_arc = (uint8_t *)&(S->all_arcs[x]);
                S->pt8b_collision_arc = &(S->pt8b_collision_arc[COLLISION_ARC_BYTE]);
            }
        }
        if (TX_found == 0)
        {   if (ARC_RX0TX1_TEST & S->arcID[iarc+1u])
            {   TX_found = 1;
                /* the first TX arc holds the byte pointer for locking the node */
                x = WRIOCOLL_ARCW3 + SIZEOF_ARCDESC_W32 * (iarc +1u);  
                S->pt8b_collision_arc = (uint8_t *)&(S->all_arcs[x]);
                S->pt8b_collision_arc = &(S->pt8b_collision_arc[COLLISION_ARC_BYTE]);
            }
        }
    }    

    /* set the linkedList pointer to the next node */
    x = (uint16_t)RD((S->node_header)[S->node_parameters_offset], W32LENGTH_LW4) +
        S->node_parameters_offset;
        
    S->linked_list_ptr = &(S->linked_list_ptr[x]);      // linked_list_ptr => next SWC.

    /* check for a rewind to the start of the list (end = NODE index 0b11111..111) */
    if (GRAPH_LAST_WORD == RD(*S->linked_list_ptr,NODE_IDX_LW0))
    {   /* rewind the index when the last word is detected */
        SET_BIT(S->scheduler_control, ENDLLIST_SCTRL_LSB);
        S->linked_list_ptr = S->linked_list;
    } 

    /* save the position in word32 */
    ST(S->link_offset, NODE_LINK_W32OFF, (uint32_t)(S->linked_list_ptr - S->linked_list));

    /* read the physical address */
    S->idx_node = (uint16_t)RD(S->node_header[0], NODE_IDX_LW0);
    S->address_node = S->node_entry_points[S->idx_node];
}


/**
  @brief         search the address of the new parameters and call "set_reset_parameters" which calls the Node
                 global data parameters -> list of [node idx; parameter address(reset after parameter set)]..[0;0]

  @param[in]     instance   pointer to the static area of the current Stream instance

  @return        none

  @par           When the parameters are updated reset the address 
  @remark
 */


static void upload_new_parameters (arm_stream_instance_t *S)
{   
#define NEWPARAM_INDEX 0
#define NEWPARAM_ADDRESS 1

    uintptr_t *new_parameters = (uintptr_t *)(S->new_parameters);
    uint32_t max_param_search = MAX_NB_PENDING_PARAM_UPDATES;
    uint32_t node_offset = (S->linked_list_ptr) - (S->linked_list);

    while ((new_parameters[NEWPARAM_INDEX] != 0) && (new_parameters[NEWPARAM_ADDRESS] != 0) && (max_param_search > 0))
    {
        if (new_parameters[NEWPARAM_INDEX] == node_offset)
        {   set_reset_parameters(S, (uint32_t *)(new_parameters[NEWPARAM_ADDRESS]));
            new_parameters[NEWPARAM_ADDRESS] = 0;   // tell the parameters have been taken into account
            break;
        }
        max_param_search--;
    }
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
    uint8_t check;
    uint8_t whoAmI;
    const p_stream_services *al_func;

    al_func = &(S->al_services);
    whoAmI = RD(S->scheduler_control, INST_ID_SCTRL);  
    (*al_func)(PACK_SERVICE(0,0,0,SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP,SERV_GROUP_INTERNAL), 
        (uintptr_t)(S->pt8b_collision_arc), (uintptr_t)&check, (uintptr_t)&whoAmI, 0);

    if (0u == check)
    {   /* a collision occured, don't wait, jump to next nanoAppRT */
        return 0;   
    }
    else
    {   /* ------------------SWC is now locked for me ! --------------------*/            
        return 1;
    }
}

static uint8_t unlock_this_component (arm_stream_instance_t *S)
{
    p_stream_services *al_func = &(S->al_services);
    uint8_t tmp = 0;

    /*  PACK_SERVICE(COMMAND,OPTION,TAG,FUNC,GROUP) */
    (*al_func)(PACK_SERVICE(0,0,0,SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP,SERV_GROUP_INTERNAL), 
        (uintptr_t)(S->pt8b_collision_arc), (uintptr_t)&tmp, 0, 0);

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
    uint8_t imem, j, iformat, narc, imem_graph;
    uint32_t *memreq, check, *key;
    
    #define MEMRESET (MAX_NB_MEM_REQ_PER_NODE + (STREAM_FORMAT_SIZE_W32*MAX_NB_STREAM_PER_NODE))
    intptr_t memreq_physical[MEMRESET];
    
    nbmem = (uint8_t)RD(S->node_header,NALLOCM1_LW0);
    nbmem ++;
    memreq_physical[0] = (intptr_t)(S->node_instance_addr);

    /* start the loop with the second memory bank */
    memreq = &(S->node_header[S->node_memory_banks_offset]);
    imem_graph = NBW32_MEMREQ_LW2;          
    for (imem = 1; imem < nbmem; imem++)
    {   /* create pointers to the right memory bank */
        memreq_physical[imem] = pack2linaddr_int(memreq[imem_graph], (uint8_t **)S->long_offset);
        imem_graph += NBW32_MEMREQ_LW2;
    }
    
    /* are there keys to share */
    if (RD(S->arcID[0], KEY_LW1))
    {   
        memreq_physical[imem] = memreq[imem_graph];    imem ++; /* user key */
        memreq_physical[imem] = memreq[imem_graph +1]; imem ++;
        
        arm_stream_services (PACK_SERVICE(NOCOMMAND_SSRV,NOOPTION_SSRV,NOTAG_SSRV, SERV_INTERNAL_KEYEXCHANGE, SERV_GROUP_INTERNAL), (uintptr_t)&key, 0, 0, 0);
        memreq_physical[imem] = key[0]; imem ++;    /* platform key */
        memreq_physical[imem] = key[1]; imem ++;
    }

    /* push the FORMAT of the arcs */
    narc = (uint8_t) (MIN(MAX_NB_STREAM_PER_NODE, RD(S->node_header[0], NBARCW_LW0)));

    for (j = 0; j < narc; j++)
    {   uint32_t *F, ifmt, arcID, *arc, tmp;

        arcID = (S->arcID)[j];
        arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
        if (ARC_RX0TX1_TEST & arcID)        // is it a TX arc ? push the "producer" format 
        {   tmp =  RD(arc[4],PRODUCFMT_ARCW4);
        }
        else
        {   tmp =  RD(arc[4],CONSUMFMT_ARCW4); // is it a RX arc ? push the "consumer" format 
        }

        ifmt = STREAM_FORMAT_SIZE_W32 * tmp;
        F = &(S->all_formats[ifmt]);
        for (iformat = 0; iformat < STREAM_FORMAT_SIZE_W32; iformat++)
        {   memreq_physical[imem++] = F[iformat]; 
        }
    }      
    

    /* reset the component with the parameter TraceID (6bits) */
    ST(S->pack_command, COMMAND_CMD, STREAM_RESET);  
    ST(S->pack_command, COMMDEXT_CMD, RD(S->scheduler_control, BOOT_SCTRL));   // warm / cold boot selection   
    ST(S->pack_command, NODE_TAG_CMD, RD((S->node_header)[S->node_parameters_offset], TRACEID_LW4));

    stream_calls_node (S,
        (void *) memreq_physical, 
        (void *) arm_stream_services, 
        &check);
}


/**
  @brief         Set reset parameters
  @param[in]     instance   pointer to the static area of the current Stream instance
                 ptr_param32b : 32bits-aligned section of parameters (32 TAGS + 
                    parameters following, see PARAM_TAG_LW4)
  @return        none
    
  @remark
 */

static void set_reset_parameters (arm_stream_instance_t *S, uint32_t *ptr_param32b)
{
    uint32_t tmp;
    uint32_t status;

    /*
        BOOTPARAMS: 
        PARAM_TAG : 4  index to parameter (0='all parameters')
        PRESET    : 4  preset index (SWC delivery)
        TRACEID   : 8  
        W32LENGTH :16  nb of WORD32 to skip at run time, 0 means NO PARAMETER, max=256kB
    */

    if (1 < RD((S->node_header)[S->node_parameters_offset], W32LENGTH_LW4))
    {
        /* change the NODE command to "Set Parameter" */
        ST(S->pack_command, COMMAND_CMD, STREAM_SET_PARAMETER);

        tmp = RD(*ptr_param32b, PARAM_TAG_LW4); /* copy the param_tag to node_tag for parameter index */
        ST(S->pack_command, NODE_TAG_CMD, tmp);
        ptr_param32b++;

        stream_calls_node (S, 
                S->node_instance_addr,
                (void *)ptr_param32b, 
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
    stream_xdmbuffer_t xdm_data[MAX_NB_STREAM_PER_NODE];
    uint32_t check;
    uint8_t loop_counter;

    /* is there a pending request to update the parameters of this node ? */  
    if (TEST_BIT((S->pt8b_collision_arc)[-7], NEW_PARAM_ARCW2_LSB))
    {   upload_new_parameters (S);
        CLEAR_BIT((S->pt8b_collision_arc)[-7], NEW_PARAM_ARCW2_LSB);
    }

    /* push all the ARCs on the stack and check arcs buffer are ready */
    if (0u == arc_index_update(S, xdm_data, 0))
    {   return; /* buffers are not ready */
    }

    /* flag : still one component is processing data in the graph */
    SET_BIT(S->scheduler_control, STILDATA_SCTRL_LSB);
   
    /* list of memory segment to swap */
    if (TEST_BIT(((uint32_t *)S->node_instance_addr)[SIZE_LW2], CLEARSWAP_LW2S_LSB))
    {   load_clear_memory_segments (S, 0);
    }

    /* pre-execution script */
    script_processing (RD(S->node_header[0], SCRIPT_LW0), SCRIPT_PRERUN);

    ST(S->pack_command, COMMAND_CMD, STREAM_RUN);
    loop_counter = MAX_NODE_REPEAT;
    do 
    {
        /* call the SWC, returns the information "0u == NODE needs to be called again" 
           to give some CPU periods for trigering data moves 
           long NODE can be split to allow data moves without RTOS */
        stream_calls_node (S,
            S->node_instance_addr, xdm_data,  &check);
    } 
    while ((check == NODE_TASKS_NOT_COMPLETED) && ((--loop_counter) > 0));
    
    /*  output FIFO write pointer is incremented AND a check is made for data 
        re-alignment to base adresses (to avoid address looping)
        The NODE don't wait and let the consumer manage the alignement 
    */
    arc_index_update(S, xdm_data, 1); 

    /* check memory segments for restore / swap back */
    if (TEST_BIT(((uint32_t *)S->node_instance_addr)[SIZE_LW2], CLEARSWAP_LW2S_LSB))
    {   load_clear_memory_segments (S, 1);
    }    

    if (script_option & STREAM_SCHD_SCRIPT_AFTER_EACH_NODE) {script_processing (S->main_script, 0);}
}

 

#ifdef __cplusplus
}
#endif
