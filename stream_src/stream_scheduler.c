/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
 * Description:  
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

#include <string.h>             /* for memcpy */

static long DEBUG_CNT;


static void read_header (arm_stream_instance_t *S);
static void reset_component (arm_stream_instance_t *S);
static uint8_t lock_this_component (arm_stream_instance_t *S);
static void set_new_parameters (arm_stream_instance_t *S);
static void run_node (arm_stream_instance_t *S);
static uint8_t arcs_are_ready(arm_stream_instance_t *S, uint8_t nb_arc);
static uint8_t arc_ready_for_write(arm_stream_instance_t *S, uint32_t *arc, uint32_t *frame_size);
static uint8_t arc_ready_for_read(arm_stream_instance_t *S, uint32_t *arc, uint32_t *frame_size);
static intPtr_t arc_extract_info_int (arm_stream_instance_t *S, uint32_t *arc, uint8_t tag);

#define script_option (RD(S->scheduler_control, SCRIPT_SCTRL))
#define return_option (RD(S->scheduler_control, RETURN_SCTRL))
#define LO S->long_offset
#define GR S->graph

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
    ST(S->scheduler_control, NODE_EXECUTION, 1);

    /* xdm_buffer pointers are updated inside the component */
    S->address_swc (S->pack_command, instance, data, parameter);

    /* node execution is finished */
    ST(S->scheduler_control, NODE_EXECUTION, 0);
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
                There are eight memory banks ("offsets") :
                    MBANK_DMEM_EXT  external shared memory
                    MBANK_DMEM      internal shared memory
                    MBANK_DMEMPRIV  not shared memory space 
                    MBANK_DMEMFAST  DTCM Cortex-M or LLRAM Cortex-R, swapped between SWC calls if static 
                    MBANK_BACKUP    backup/retention SRAM 
                    MBANK_HWIODMEM  memory space for I/O and DMA buffers 
                    MBANK_PMEM      program RAM 
                    MBANK_FLASH     shared internal Flash

  @remark       pack2linaddr will be replaced by the macro PACK2LINADDR
 */

/*----------------------------------------------------------*/
static intPtr_t pack2linaddr_int(intPtr_t *long_offset, uint32_t x)
{
    intPtr_t dbg1, dbg2, dbg3;
     dbg1 = *((long_offset)+RD(x,DATAOFF_ARCW0));
     dbg2 =  BASEINWORD32 * RD((x),BASEIDX_ARCW0);

     if (RD(x,BASESIGN_ARCW0))  // to test @@@
        dbg3 = dbg1 - dbg2;
      else
        dbg3 = dbg1 + dbg2;

    return PACK2LINADDR(long_offset,x);
}

static uint8_t * pack2linaddr_ptr(intPtr_t *long_offset, uint32_t data)
{
    return (uint8_t *) (pack2linaddr_int(long_offset, data));
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
    uint32_t *all_formats, *F;

    read =  RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);
    size =  RD(arc[1], BUFF_SIZE_ARCW1);
    all_formats =  S->all_formats;

    switch (tag)
    {
    case consumer_frame_size:
        F = &(all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc[1],CONSUMFMT_ARCW1)]);
        ret = RD(*F, FRAMESIZE_FMT0);
        break;
    case producer_frame_size:
        F = &(all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc[0],PRODUCFMT_ARCW0)]);
        ret = RD(*F, FRAMESIZE_FMT0);
        break;
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
    long_base = pack2linaddr_ptr(S->long_offset, base);

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
  @brief         Translation of physical address to packed address in 27-bits
  @param[in]     instance   Global registers of the current instance
  @param[in]     address    Address to be translated to 
  @param[in]     data3      3rd parameter integer
  @return        none

  @par           Checks which of the 8 memory bank is suitable for the address
                 Translate the offset to te base address in an integer and shift
  @remark
 */

/*----------------------------------------------------------------------------
    convert a physical address to a portable multiprocessor address 
 *----------------------------------------------------------------------------*/
uint32_t physical_to_offset (arm_stream_instance_t *S, uint8_t *buffer)
{
    uint64_t mindist;
    uint64_t pack;
    uint64_t buff;
    uint8_t i;
    uint8_t ibest;

    buff = (uint64_t)buffer;

    /* find the base offset */
    mindist = MAX_ADD_OFFSET;
    ibest = 0;
    for (i = 0; i < (uint8_t)NB_MEMINST_OFFSET; i++)
    {
        if (S->long_offset[i] > buff) 
        { continue; }

        {   uint64_t tmp64;
            tmp64 = buff - S->long_offset[i];
            if (tmp64 < mindist)
            {   mindist = tmp64;
                ibest = i;
            }
        }
    }
  
    pack = mindist | ((uint64_t)ibest << (uint8_t)DATAOFF_ARCW0_LSB);
    
    return (uint32_t)pack;
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
    uint32_t producer_frame_size, fifosize, write, needed_free_area;
    uint8_t i;

    fifosize =  RD(arc[1], BUFF_SIZE_ARCW1);
    write = RD(arc[3], WRITE_ARCW3);

    /* does the write index is already far, to ask for data realignment? */
    i = RD(arc[0],PRODUCFMT_ARCW0);
    i = i * STREAM_FORMAT_SIZE_W32;
    producer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);
    needed_free_area = fifosize - producer_frame_size;

    if (write >= needed_free_area)
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

    *free_for_writes =  (fifosize - producer_frame_size); /* memory available for writes */

    if (write >= (*free_for_writes))
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

    all_formats = S->all_formats;
    read = RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);
    fifosize = RD(arc[1], BUFF_SIZE_ARCW1);

    consumer_frame_format = all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc[1],CONSUMFMT_ARCW1)];
    consumer_frame_size = RD(consumer_frame_format, FRAMESIZE_FMT0);
    *frame_size = consumer_frame_size;
    fifo_data_amount = write - read;
    return (fifo_data_amount >= consumer_frame_size) ? 1 : 0;
}



/**
  @brief         Checks the input and output arcs can be used
  @param[in]     instance   global registers of this instance
  @param[in]     nb_arc     number of arcs to check in the instance's arc list
  @return        flag = 0 when at least one arc is not ready for being processed

  
  @par           The arc descriptor gives, in the 2nd word, the stream format used by
                 the node consuming data from this arc. On the input streams side, 
                 looking at the amount of data in the buffer and the frame-size
                 consumed by the node (the minimum amount of byte consumed per call), the
                 function returns a go/no-go flag. And on the output streams side, 
                 looking at the remaining free space in the buffer and the frame-size
                 used by the producer (the minimum amount of byte produced per call), the
                 function updates the go/no-go flag.
  @remark
 */

static uint8_t arcs_are_ready(arm_stream_instance_t *S, uint8_t nb_arc)
{
    uint16_t iarc, arc_id;
    uint8_t ret;
    uint32_t *arc, common_size, frame_size;

    ret = 1;
    common_size = MAXINT32;

    for (iarc = 0; iarc < nb_arc; iarc++)
    {
        arc_id = S->arcID[iarc];
        arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arc_id)]);

        if (0 == (ARC_RX0TX1_MASK & arc_id))
        {   
            ret &= arc_ready_for_read(S, arc, &frame_size);
            common_size = MIN(frame_size, common_size);
        } 
        else
        {
            ret &= arc_ready_for_write(S, arc, &frame_size);
            common_size = MIN(frame_size, common_size);
        }
    }

    S->xdm11_frame_size = common_size;
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

void arc_data_operations (
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
    long_base = pack2linaddr_ptr(S->long_offset, base);

    switch (tag)
    {
    /* data is left shifted to the base address to avoid circular addressing */ 
    case arc_data_realignment_to_base:
        read =  RD(arc[2], READ_ARCW2);
        if (read == 0)
        {   break;      /* buffer is full but there is nothing to realign */
        }
        write = RD(arc[3], WRITE_ARCW3);
        size = U(write - read);
        src = &(long_base[read]);
        dst =  long_base;
        MEMCPY (dst, src, size);

        /* update the indexes Read=0, Write=dataLength */
        ST(arc[2], READ_ARCW2, 0);
        ST(arc[3], WRITE_ARCW3, size);

        /* clear the bit if there is enough free space after this move */
        set_alignment_bit (S, arc);

        /* DMB on RD/WR to let the producers be aware */
        DATA_MEMORY_BARRIER;
    break;

    /* move in and increment write index, update filling status */
    case arc_IO_move_to_arc : 
        /* only one node can read the write-index at a time : no collision is possible */
        write = RD(arc[3], WRITE_ARCW3);
        src = buffer;
        dst = &(long_base[write]);
        MEMCPY (dst, src, datasize);
        write = write + datasize;
        ST(arc[3], WRITE_ARCW3, write);

        /* check overflow */
        set_alignment_bit (S, arc);

        DATA_MEMORY_BARRIER;
        break;

    /* move out was notified in the driver, do the data move increment read index, 
        update emptiness status */
    case arc_IO_moved_from_arc  : 
        /* only one node can update the read-index at a time : no collision is possible */
        read = RD(arc[2], READ_ARCW2);
        src = &(long_base[read]);
        dst = buffer;
        MEMCPY (dst, src, datasize);

    /* just an index update of read after arm_stream_io(txslave) */
    case arc_IO_moved_from_arc_update:
        /* only one node can update the read-index at a time : no collision is possible */
        read = RD(arc[2], READ_ARCW2);
        read = read + datasize;

        /* update the read index */
        ST(arc[2], READ_ARCW2, read);

        /* clear the bit if there is enough free space after this move */
        set_alignment_bit (S, arc);

        DATA_MEMORY_BARRIER;
        break;        

    /* large buffer : no data move but update the arc descriptor to the
        buffer provided by the arm_stream_io() callback 
        set the buffer to IO address with R=0  and W=Size 
        RX buffer generated from IO => set the address 
        */
    case arc_set_base_address_to_arc :
        ST(arc[2], READ_ARCW2, 0);
        ST(arc[0], BASEIDXOFFARCW0, physical_to_offset(S, buffer));
        ST(arc[3], WRITE_ARCW3, datasize);

        DATA_MEMORY_BARRIER;
        break;

    case arc_set_base_address_from_arc :
        ST(arc[2], READ_ARCW2, 0);
        ST(arc[0], BASEIDXOFFARCW0, physical_to_offset(S, buffer));
        ST(arc[3], WRITE_ARCW3, 0);

        DATA_MEMORY_BARRIER;
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
  @return        none

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
static void arc_index_update (arm_stream_instance_t *S, stream_xdmbuffer_t *xdm_data, uint8_t pre0post1)
{
    uint32_t fifosize, read, write;
    uint32_t increment;
    uint32_t *arc;
    uint16_t iarc, arcID, *pt8_arc;
    uint8_t xdm11;
                           
    /* push all the ARCs on the data on the stack */
    pt8_arc = (uint16_t *)(S->swc_header);
    pt8_arc = &(pt8_arc[ARC_HEADER_BYTE_OFFSET]);    /* points to the start of the ARCs area in the header */
     
    xdm11 = TEST_BIT(S->swc_header[1], XDM11_LW1_LSB);

    for (iarc = 0; iarc < RD(*(S->swc_header), NBARCW_LW0); iarc++, pt8_arc++)
    {   
        arcID = (S->arcID[iarc]);
        arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & arcID)]);
        read = RD(arc[2], READ_ARCW2);
        write = RD(arc[3], WRITE_ARCW3);
        fifosize =  RD(arc[1], BUFF_SIZE_ARCW1);

        if (ARC_RX0TX1_MASK & arcID)
        {   /* TX arc case */
            if (0 == pre0post1)
            {   xdm_data[iarc].address = (intPtr_t)(arc_extract_info_pt (S, arc, arc_write_address));

                if (xdm11) /* output buffer of the SWC : size = common with RX and TX */   
                {   xdm_data[iarc].size = S->xdm11_frame_size;
                }
                else /* output buffer of the SWC : size = free area in the original buffer */  
                {   xdm_data[iarc].size    = arc_extract_info_int (S, arc, arc_free_area);
                }
            }
            else
            {   /* to save cycles the SWC is not incrementing the pointers*/
                if (xdm11) 
                {   increment = S->xdm11_frame_size;
                }
                else
                {   /* the SWC put the amount of data produced in "size"
                        output buffer of the SWC : check the pointer increment */
                    increment = (uint32_t)(xdm_data->size);
                }
                write = write + increment;
                ST(arc[3], WRITE_ARCW3, write);

                /* is there debug/monitoring activity to do ? */
                if (RD(arc[2], COMPUTCMD_ARCW2) != 0u)
                { /* @@@ TODO : implement the data monitoring 
                    arm_arc_monitor(*, n, RD(,COMPUTCMD_ARCW2), RD(,DEBUG_REG_ARCW1))
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
                {   xdm_data[iarc].size = S->xdm11_frame_size;
                }
                else /* input buffer of the SWC : size = data amount in the buffer */
                {   xdm_data[iarc].size    = arc_extract_info_int (S, arc, arc_data_amount);
                }
            }
            else
            {   /* to save code and cycles, the SWC is not incrementing the pointers*/
                if (xdm11) 
                {   increment = S->xdm11_frame_size;
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
                if (0u != TEST_BIT (arc[3], ALIGNBLCK_ARCW3_LSB))
                {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                }

                /* does data realignement is possible ? */
                set_alignment_bit (S, arc);
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
static void check_graph_boundaries(arm_stream_instance_t *S)
{
    uint32_t iio;
    uint32_t arc_idx;
    uint8_t need_data_move;
    uint32_t size;
    uint8_t *buffer;
    uint32_t *arc;
    struct platform_control_stream p_data_move;
    uint32_t *pio, *pio_base;
    uint32_t io_mask = S->iomask;
    uint32_t nio;

    pio_base = S->pio;
    nio = RD((S->graph)[1],NB_IOS_GR1);

    for (iio = 0u; iio < nio; iio++)
    {
        /* jump to next graph port */
        pio = &(pio_base[iio * STREAM_IOFMT_SIZE_W32]);
        arc_idx = SIZEOF_ARCDESC_W32 * RD(*pio, IOARCID_IOFMT);
        arc = &(S->all_arcs[ARC_RX0TX1_CLEAR & arc_idx]);

        /* ports where the peripheral is commander are not checked (commander_servant_IOMEM) */
        /* does this port needs to be managed by the Stream instance ? */
        if (0u == U(io_mask & ((uint32_t)1 << iio))) 
        {   continue; 
        }

        /* a previous request is in process or if the IO is commander on the interface, then no 
            need to ask again */
        if ((0u != TEST_BIT(S->ioreq, iio)) || 
            (0u == TEST_BIT(*pio, SERVANT_IOFMT_LSB)))
        {   continue;
        }



        /* if this is an input stream : check the buffer is empty  */
        if (RX0_TO_GRAPH == RD(*pio, RX0TX1_IOFMT))
        {   need_data_move = arc_ready_for_write(S, arc, &size);
            buffer = arc_extract_info_pt(S, arc, arc_write_address);
            if (size == 0)
            {   break;
            }
        }

        /* if this is an output stream : check the buffer is READY_R */
        else
        {   need_data_move = arc_ready_for_read(S, arc, &size);
            buffer = arc_extract_info_pt(S, arc, arc_read_address);
            if (size == 0)
            {   break;
            }
        }
        
        /* skip if the buffer is not empty when the IO is programmed for "set buffer" */
        {   if (TEST_BIT(*pio, IOCOMMAND_IOFMT_LSB) == IO_COMMAND_SET_BUFFER)
            {   if (0 != arc_extract_info_int(S, arc, arc_data_amount))
                {   continue;
                }
            }
        }

        if (0u != need_data_move)
        {   /*  TODO When the IO is slave and stream_io_domain=PLATFORM_ANALOG_SENSOR_XX 
                 check the time interval from last frame (by a read of the time-stamp in 
                 the FIFO) and current time, to deliver a data rate close to :
                 "platform_io_control.stream_settings_default.SAMPLING_FMT1_"
                Trigger the data request some time ahead to let the converters
                 have the time to exchange data (image, remote temperature sensors,
                 characters on a display, ..).
                data transfer on-going 
            */
            SET_BIT(S->ioreq, REQMADE_IO_LSB);

            /* ask the firmware to awake this port */
            p_data_move.fw_io_idx = RD(*pio, FW_IO_IDX_IOFMT);

            /* using the address of the ring buffer */
            p_data_move.buffer.address = (intPtr_t)buffer;
            p_data_move.buffer.size = size;
            platform_al(PLATFORM_IO_DATA, (uint8_t *)&p_data_move, 0u, 0u);

            /* if this is an input stream : check the buffer needs alignment by the consumer */
            if (RX0_TO_GRAPH == RD(*pio, RX0TX1_IOFMT))
            {   set_alignment_bit (S, arc);

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
  @brief        check_hw_compatibility
  @param[in]    none
  @return       none

  @par          Loads the global variable of the platform holding the base addresses 
                to the physical memory banks described in the "platform manifest"

  @remark       
 */
static uint32_t check_hw_compatibility(uint32_t whoami, uint32_t header) 
{
    uint8_t match = 1;

    if (RD(header, ARCHID_LW0) > 0u) /* do I care about the architecture ID ? */
    {   match = (RD(header, ARCHID_LW0) == RD(whoami, ARCHID_PARCH));
    }

    if (RD(header, PROCID_LW0) > 0u) /* do I care about the processor ID ? */
    {   match = match & (RD(header, PROCID_LW0) == RD(whoami, PROCID_PARCH));
    }
    return match;
}
   
/*----------------------------------------------------------------------------*/
/**
  @brief        Main scheduler loop of the linked list of nodes in the graph
  @param[in]    instance       pointer to the static area of the current Stream instance
  @param[in]    reset_option   Tells if this is a scan for reset, run or stop the graph
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


void stream_scan_graph (arm_stream_instance_t *S, int8_t reset_option) 
{
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
            if (reset_option == 0) 
            {   check_graph_boundaries(S); 
            }
         
            /* read all the information about the Node and the way to set its parameters */
            read_header (S);

            /* does the SWC is executable on this processor */
            if (0U == check_hw_compatibility(S->whoami_ports, (S->swc_header)[0]))
            {   continue;
            }

            /* does an other process/processor is try to execute the same Node ? */
            if (0 == lock_this_component (S))
            {   continue;
            }

            /* ---------------- parameter was changed, or reset phase ? -------------------- */
            if (reset_option > 0 ||
                (TEST_BIT(S->scheduler_control,L_IN_RAM_SCTRL_LSB) && 
                 TEST_BIT((S->swc_header)[S->swc_parameters_offset], NEWPARAM_LW3_LSB)))
            {   
                /* reset phase of the graph ?  */
                if (reset_option > 0)
                {   reset_component (S);
                }

                /* set the parameters at reset or when a script loaded new parameters (and set NEWPARAM_GI) */
                set_new_parameters (S);
            }

            /* end of graph processing ? */
            if (reset_option < 0)
            {   uint32_t returned;
                ST(S->pack_command, COMMAND_CMD, STREAM_STOP);
                stream_calls_swc (S,
                    S->swc_instance_addr, 0u, &returned);
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
                    if (0 == (ARC_RX0TX1_MASK & arcID))
                    {   arc = &(S->all_arcs[SIZEOF_ARCDESC_W32 * arcID]);
                        if (0u != TEST_BIT (arc[WRIOCOLL_ARCW3], ALIGNBLCK_ARCW3_LSB))
                        {   arc_data_operations (S, arc, arc_data_realignment_to_base, 0, 0);
                        }
                    }
                }
            }

            /* check input arc has enough data and output arc is free */
            if ((0 == reset_option) && 
                (0 != arcs_are_ready(S, RD(S->swc_header[0], ARCSRDY_LW0)))) 
            {   run_node (S);
            }
    
            /* now SWC can be unlocked */
            WR_BYTE_MP_(S->pt8b_collision_arc, 0u);   

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


    if (script_option & STREAM_SCHD_SCRIPT_END) {script_processing (S->main_script);}
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
    uint16_t x;
    uint16_t narc, iarc;
    uint16_t idx_swc;

    S->swc_header = S->linked_list_ptr;                     // linked_list_ptr => HEADER
    narc = RD(S->swc_header[0], NBARCW_LW0);
    S->swc_memory_banks_offset = 
        1 +(narc/2);
    S->swc_parameters_offset =  S->swc_memory_banks_offset 
        + RD(S->swc_header[S->swc_memory_banks_offset], NBALLOC_LW2);

    /* default parameters of the node */
    S->pack_command = PACK_COMMAND(
        ALLPARAM_,
        RD((S->swc_header)[S->swc_parameters_offset], PRESET_LW3),
        narc,
        RD(S->scheduler_control, INSTANCE_SCTRL),
        0);

    /* physical address of the instance and the code */
    idx_swc = (uint16_t)RD(S->swc_header[0], SWC_IDX_LW0);
    S->swc_instance_addr = (stream_handle_t)pack2linaddr_int
        (S->long_offset, 
         S->swc_header[S->swc_memory_banks_offset]
        );

    /* read the physical address, let the platform configure the memory protections */
    platform_al(PLATFORM_NODE_ADDRESS, (uint8_t *)(S->address_swc), 0, idx_swc);   

    /* read the arc indexes */
    for (iarc = 0; iarc < MIN(narc, MAX_NB_STREAM_PER_SWC); iarc+=2)
    {   uint32_t arcs;
        arcs = (S->swc_header)[1+iarc/2];
        S->arcID[iarc]   = RD(arcs, ARC0_LW1);
        S->arcID[iarc+1] = RD(arcs, ARC1_LW1);
    }    

    /* byte pointer for locking the node */
    x = RD(S->swc_header[0], ARCLOCK_LW0);
    iarc = S->arcID[x];
    x = WRIOCOLL_ARCW3 + SIZEOF_ARCDESC_W32 * (ARC_RX0TX1_CLEAR & iarc);
    S->pt8b_collision_arc = (uint8_t *)&(S->all_arcs[x]);
    S->pt8b_collision_arc = &(S->pt8b_collision_arc[COLLISION_ARC_OFFSET_BYTE]);

    /* set the linkedList pointer to the next node */
    x = 1+ (uint16_t)RD((S->swc_header)[S->swc_parameters_offset], W32LENGTH_LW3) +
        S->swc_parameters_offset;
        
    S->linked_list_ptr = &(S->linked_list_ptr[x]);      // linked_list_ptr => next SWC.

    /* check for a rewind to the start of the list */
    if (GRAPH_LAST_WORD == RD(*S->linked_list_ptr,SWC_IDX_LW0))
    {   /* rewind the index when the last word is detected */
        SET_BIT(S->scheduler_control, ENDLLIST_SCTRL_LSB);
        S->linked_list_ptr = S->linked_list;
    } 

    /* save word32 the position */
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
    uint32_t tmp;
    uint8_t check;
    uint32_t j;

    /* ---------------------SWC reservation attempt -------------------*/            
    /* if the SWC is already used skip it */
    j = U(4u*WRIOCOLL_ARCW3 + COLLISION_ARC_OFFSET_BYTE);

    RD_BYTE_MP_(check, S->pt8b_collision_arc);
    if (check != U8(0))
    {   return 0;
    }
 
    tmp = (uint8_t)RD(S->whoami_ports, INST_ID_PARCH);  
    if (0u == WR_BYTE_AND_CHECK_MP_(S->pt8b_collision_arc, tmp))
    {   return 0;   /* a collision occured, don't wait */
    }
    /* ------------------SWC is now locked for me ! --------------------*/            

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
#define MEMRESET (MAX_NB_MEM_REQ_PER_NODE + (SIZEOF_ARCDESC_SHORT_W32*MAX_NB_STREAM_PER_SWC))

    intPtr_t memreq_physical[MAX_NB_MEM_REQ_PER_NODE + 
                (SIZEOF_ARCDESC_SHORT_W32*MAX_NB_STREAM_PER_SWC)];
    uint16_t nbmem;
    uint8_t imem, j, narc;
    uint32_t *memreq, check;

    memreq = &(S->swc_header[S->swc_memory_banks_offset]);
    nbmem = (uint16_t)RD(memreq[0],NBALLOC_LW2);
    memreq_physical[0] = (intPtr_t)(uint64_t)(S->swc_instance_addr);


    for (imem = 1; imem < nbmem; imem++)
    {   /* create pointers to the right memory bank */
        memreq_physical[imem] = pack2linaddr_int(S->long_offset, memreq[imem]);
    }      

    /* push the FORMAT of the arcs */
    narc = MIN(MAX_NB_STREAM_PER_SWC, RD(S->swc_header[0], NBARCW_LW0));

    for (j = 0; j < narc; j++)
    {   uint32_t *F, ifmt;
        if (ARC_RX0TX1_MASK & (S->arcID)[j])
        {   ifmt = STREAM_FORMAT_SIZE_W32 * 
                (ARC_RX0TX1_CLEAR & RD((S->arcID)[j],PRODUCFMT_ARCW0));
        }
        else
        {   ifmt = STREAM_FORMAT_SIZE_W32 * 
                (ARC_RX0TX1_CLEAR & RD((S->arcID)[j],CONSUMFMT_ARCW1));
        }
        F = &(S->all_formats[ifmt]);
        memreq_physical[imem++] = F[0];     /* Word 0 SIZSFTRAW_FMT0 */
        memreq_physical[imem++] = F[1];     /* Word 1 SAMPINGNCHANM1_FMT1 */
    }      

    /* reset the component */
    ST(S->pack_command, COMMAND_CMD, STREAM_RESET);

    stream_calls_swc (S,
        (stream_handle_t) memreq_physical, 
        (stream_xdmbuffer_t *) arm_stream_services, 
        &check);
}


/**
  @brief         Set new parameters
  @param[in]     instance   pointer to the static area of the current Stream instance

  @return        none

  @par           When the silicon device has a small RAM footprint, the linked-list is graph
                 is organised with a part in flash (the data format does not change) and other
                 in RAM (arcs descriptors). When the Linked-List of nodes is in Flash the
                 parameters are loaded one at Reset time, otherwise the bit "NEWPARAM_GI" 
                 indicates a new parameter set (replacing the one at graph creation) must be 
                 reloaded in the Node instance. There is no risk of collision when writing
                 this bit: only the instance who locked it can have access to this word.
                 The default parameters are given with the "preset" code, and the graph can 
                 have a some patched of "nparam" parameters on top this preset. When "nparam"
                 is "ALLPARAM_" the full set of new parameters is loaded.

  @remark
 */

static void set_new_parameters (arm_stream_instance_t *S)
{
    uint8_t *ptr_param8b;
    uint32_t length, tmp, idata;

    /* no parameter ? */
    length = RD((S->swc_header)[S->swc_parameters_offset], W32LENGTH_LW3);
    if (0 == length)
    {   return;
    }

    /*
      BOOTPARAMS  : 
          BOOTPARAMS    : 
            unused      : 1  
            paramtype   : 1  0:all params   1:params sent 1-by-1
            skip        :16  nb of word32 to skip at run time, 0 means no parameter.
            verbose     : 1  level of details in the debug trace
            new param   : 1  a script has updated new parameters
            unused      : 4; 
            preset LSB  : 4; preset index (SWC delivery)

            if skip> 0 and paramtype= 1
                sequence of pairs {8b index/tag ; 24b byte length} { parameter(s) }

      TODO : allocate a scratch area to copy the parameters, and avoid SWC 
             to have access to Stream internal memory area
    */
    tmp = ((S->swc_header)[S->swc_parameters_offset]);
    tmp = RD(tmp, SELPARAM_LW3);

    /* start of the parameter byte-stream : one word after the header */
    ptr_param8b = (uint8_t *) &((S->swc_header)[1 + S->swc_parameters_offset]);

    /* do we load the full set of parameters ? (default setting of pack_command) */
    if (tmp == 0 /* SELPARAM_LW3 = "ALLPARAM_" */ )
    {   
        ST(S->pack_command, COMMAND_CMD, STREAM_SET_PARAMETER);

        stream_calls_swc (S,
            S->swc_instance_addr,
            (stream_xdmbuffer_t *)ptr_param8b, 
            &tmp);
    }
    else /* SELPARAM_LW3 == send the parameters one-by-one */
    {   uint8_t param_tag;
        uint8_t param_nb_bytes;

        ST(S->pack_command, COMMAND_CMD, STREAM_SET_PARAMETER);
        idata = 0;

        while (idata < length/4)
        {   /* sequence of pairs {8b index/tag ; 24b nb of bytes }  */
            param_tag = *ptr_param8b;
            ST(S->pack_command, TAG_CMD, param_tag);
            ptr_param8b++;  
            param_nb_bytes = *ptr_param8b++;
            param_nb_bytes += (*ptr_param8b++)<<8;
            param_nb_bytes += (*ptr_param8b++)<<16;
            idata += param_nb_bytes;
            
            stream_calls_swc (S, 
                S->swc_instance_addr,
                (stream_xdmbuffer_t *)ptr_param8b, 
                &tmp);

            ptr_param8b = &(ptr_param8b[param_nb_bytes]);
        }
    }

    /* ready to accept new parameters : clear the flag */
    if (TEST_BIT(S->scheduler_control,L_IN_RAM_SCTRL_LSB))
    {   CLEAR_BIT_MP((S->swc_header)[S->swc_parameters_offset], NEWPARAM_LW3_LSB);
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

    /* flag : still one component is processing data in the graph */
    SET_BIT(S->scheduler_control, STILDATA_SCTRL_LSB);
   
    /* push all the ARCs on the stack */
    arc_index_update(S, xdm_data, 0); 
    
    /* if the SWC is using a relocatable memory segment to TCM (multiprocessing with TCM), the base 
        address is loaded in the first address of its instance */
    if (0u != TEST_BIT((S->swc_header)[S->swc_memory_banks_offset], TCM_INST_LW2_LSB))
    {   intPtr_t *tmp = (intPtr_t *)S->swc_instance_addr;
        *tmp = S->long_offset[MBANK_DMEMFAST];
    }
    
    if (script_option & STREAM_SCHD_SCRIPT_BEFORE_EACH_SWC) {script_processing (S->main_script); }
    
    ST(S->pack_command, COMMAND_CMD, STREAM_RUN);

    // @@ TODO : repeat the SWC execution 'MAX_SWC_REPEAT' times 
    do 
    {
        #define SWC_TASK_NOT_COMPLETED 0
        /* call the SWC, returns the information "0 == SWC needs to be called again" 
           to give some CPU periods for trigering data moves 
           long SWC can be split to allow data moves without RTOS */
        stream_calls_swc (S,
            S->swc_instance_addr, xdm_data,  &check);
    
        if (SWC_TASK_NOT_COMPLETED == check)
        {   check_graph_boundaries(S); 
        }
    } 
    while (check == SWC_TASK_NOT_COMPLETED);
    
    
    if (script_option & STREAM_SCHD_SCRIPT_AFTER_EACH_SWC) {script_processing (S->main_script);}
    
    /*  output FIFO write pointer is incremented AND a check is made for data 
        re-alignment to base adresses (to avoid address looping)
        The SWC don't wait and let the consumer manage the alignement 
    */
    arc_index_update(S, xdm_data, 1); 
}

#ifdef __cplusplus
}
#endif
