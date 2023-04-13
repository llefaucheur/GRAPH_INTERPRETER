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

#include "stream_const.h"      /* graph list */
#include "stream_types.h"

#include <string.h>             /* for memcpy */


/*----------------------------------------------------------------------------
   execute a swc
   this is the only place where a swc is called for safety reason 
   swc can check the return-address is identical
 *----------------------------------------------------------------------------*/

static void stream_calls_swc (p_stream_node address_swc, int32_t command, 
                    uint32_t *instance, 
                    data_buffer_t *data,
                    uint32_t *parameter
                   )
{
     /* xdm_buffer pointers are updated inside the component */
     address_swc (command, instance, data, parameter);
}
    
/*----------------------------------------------------------------------------*/

intPtr_t arc_extract_info_int (uint32_t *arc, uint8_t tag, intPtr_t *offsets)
{
    uint32_t read;
    uint32_t write;
    uint32_t size;
    uint32_t base;
    intPtr_t ret;

    read =  EXTRACT_FIELD(arc[RDFLOW_ARCW2   ], READ_ARCW2);
    write = EXTRACT_FIELD(arc[WRIOCOLL_ARCW3 ], WRITE_ARCW3);
    size =  EXTRACT_FIELD(arc[BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
    base =  EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], BASEIDX_ARCW0);

    switch (tag)
    {
    case arc_data_amount  : ret = write - read; 
        break;
    case arc_free_area    : ret = size - write; 
        break;
    case arc_buffer_size  : ret = size; 
        break;
    default : ret = *offsets; 
    }
    return ret;
}

intPtr_t * arc_extract_info_pt (uint32_t *arc, uint8_t tag, intPtr_t *offsets)
{
    uint32_t read;
    uint32_t write;
    uint32_t size;
    uint32_t base;
    uint32_t shift;
    uint32_t idxoff;
    intPtr_t *long_base;
    intPtr_t *ret;

    read =  EXTRACT_FIELD(arc[RDFLOW_ARCW2   ], READ_ARCW2);
    write = EXTRACT_FIELD(arc[WRIOCOLL_ARCW3 ], WRITE_ARCW3);
    size =  EXTRACT_FIELD(arc[BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
    base =  EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], BASEIDX_ARCW0);
    shift = EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], BASESHIFT_ARCW0);
    idxoff =EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], DATAOFF_ARCW0);
    long_base = pack2linaddr_ptr(base,offsets);

    switch (tag)
    {
    case arc_base_address : ret = long_base; 
        break;
    case arc_ceil_address : ret = &(long_base[size]); 
        break;
    case arc_read_address : ret = &(long_base[read]); 
        break;
    case arc_write_address: ret = &(long_base[write]); 
        break;
    default : ret = 0u; 
    }
    return ret;
}

/*----------------------------------------------------------------------------
    script 
    [0] FIFO of byte codes for debug script (FIFO #0)
 *----------------------------------------------------------------------------*/
int stream_execute_script(intPtr_t *parameters)
{    
    uint32_t i = RD(parameters[STREAM_PARAM_CTRL], INSTIDX_PARAM); 
    //arm_script();
    return 1;
}

/*----------------------------------------------------------------------------
    convert a physical address to a portable multiprocessor address 
 *----------------------------------------------------------------------------*/
uint32_t physical_to_offset (uint8_t *buffer, intPtr_t *long_offset)
{
    uint64_t mindist;
    uint64_t pack;
    uint64_t buff;
    uint8_t i;
    uint8_t ibest;
    uint8_t ishift;

    buff = (uint64_t)buffer;

    /* find the base offset */
    mindist = MAX_ADD_OFFSET;
    ibest = 0;
    for (i = 0; i < (uint8_t)NB_MEMINST_OFFSET; i++)
    {
        if (long_offset[i] > buff) 
        { continue; }

        {   uint64_t tmp64;
            tmp64 = buff - long_offset[i];
            if (tmp64 < mindist)
            {   mindist = tmp64;
                ibest = i;
            }
        }
    }

    /* code it to 22 bits base + 2 bits shifts */
#define MAX_SHIFT (uint8_t)((U(1) << U(BASESHIFT_ARCW0_MSB-BASESHIFT_ARCW0_LSB+U(1)))-U(1))
    for (ishift = 0; ishift < MAX_SHIFT; ishift++)
    {
        if (mindist < (U(1)<<BASESHIFT_ARCW0_LSB)-U(1)) { break; }
        mindist = mindist >> 2;
    }    
    
    pack = mindist | ((uint64_t)ishift << (uint8_t)BASESHIFT_ARCW0_LSB) | ((uint64_t)ibest << (uint8_t)DATAOFF_ARCW0_LSB);
    
    return (uint32_t)pack;
}

/*----------------------------------------------------------------------------
    Copy in/out the data of ring buffers
 *----------------------------------------------------------------------------*/
void arc_data_operations (
        intPtr_t *arc, 
        uint8_t tag, 
        intPtr_t *long_offset, 
        uint8_t *buffer, 
        uint32_t datasize
        )
{
    uint32_t read;
    uint32_t write;
    uint32_t fill_thr;
    uint32_t base;
    uint32_t shift;
    uint32_t idxoff;
    uint32_t fifosize;
    uint32_t size;
    uint8_t *long_base;
    uint8_t *src;
    uint8_t* dst;
    uint8_t bool;

    //xdm_data = (data_buffer_t *)buffer;
    fifosize =  EXTRACT_FIELD(arc[BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
    base =      EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], BASEIDX_ARCW0);
    shift =     EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], BASESHIFT_ARCW0);
    idxoff =    EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], DATAOFF_ARCW0);
    long_base = (uint8_t *)pack2linaddr_ptr(base,long_offset);

    switch (tag)
    {
    /* data is left shifted to the base address to avoid circular addressing */ 
    case arc_data_realignment_to_base:
        read =  EXTRACT_FIELD(arc[RDFLOW_ARCW2  ], READ_ARCW2);
        write = EXTRACT_FIELD(arc[WRIOCOLL_ARCW3], WRITE_ARCW3);
        size = U(write - read);
        src = &(long_base[read]);
        dst =  long_base;
        MEMCPY (dst, src, size);

        /* update the indexes Read=0, Write=dataLength */
        ST(arc[RDFLOW_ARCW2  ], READ_ARCW2, 0);
        ST(arc[WRIOCOLL_ARCW3], WRITE_ARCW3, size);

        /* DMB on RD/WR to let the producer be aware */
        DATA_MEMORY_BARRIER;
    break;

    /* move in and increment write index, update filling status */
    case arc_move_to_arc : 
        /* only one node can read the write-index at a time : no collision is possible */
        write = EXTRACT_FIELD(arc[WRIOCOLL_ARCW3], WRITE_ARCW3);
        src = buffer;
        dst = &(long_base[write]);
        MEMCPY (dst, src, datasize);
        write = write + datasize;

        /* update the write index */
        ST(arc[WRIOCOLL_ARCW3], WRITE_ARCW3, write);

        /* are there enough data in the FIFO to tell it ready for read ? */
        fill_thr = EXTRACT_FIELD(arc[RDFLOW_ARCW2  ], THRESHOLD_ARCW2);
        read =     EXTRACT_FIELD(arc[RDFLOW_ARCW2  ], READ_ARCW2);
        write =    EXTRACT_FIELD(arc[WRIOCOLL_ARCW3], WRITE_ARCW3);
        bool = (U(write - read) > U(datasize >> U(U(1) + fill_thr))) ? 1 : 0;
        ST(arc[WRIOCOLL_ARCW3], READY_R_ARCW3, bool);
        DATA_MEMORY_BARRIER;
        break;

    /* move out was done by the consumer, increment read index, update emptiness status */
    case arc_moved_from_arc  : 
        /* only one node can update the read-index at a time : no collision is possible */
        read = EXTRACT_FIELD(arc[RDFLOW_ARCW2], READ_ARCW2);
        read = read + datasize;

        /* update the read index */
        ST(arc[RDFLOW_ARCW2], READ_ARCW2, read);

        /* are there enough free-space in the FIFO to tell it ready for write ? */
        fill_thr = EXTRACT_FIELD(arc[RDFLOW_ARCW2  ], THRESHOLD_ARCW2);
        write =    EXTRACT_FIELD(arc[WRIOCOLL_ARCW3], WRITE_ARCW3);
        {   uint32_t a;
            uint32_t b;
            a = U(fifosize - write);
            b = U(datasize >> U(U(1) + fill_thr));
            bool = (a > b) ? 1 : 0;
        }
        ST(arc[RDFLOW_ARCW2], READY_W_ARCW2, bool);
        DATA_MEMORY_BARRIER;
        break;

    /* large buffer : no data move but update the arc descriptor to the
        buffer provided by the arm_stream_io() callback */

    /* set the buffer to IO addres with R=0  and W=Size */
    case arc_set_base_address_to_arc :
        ST(arc[BUF_PTR_ARCW0 ], BASEIDXOFFARCW0, physical_to_offset(buffer, long_offset));
        ST(arc[WRIOCOLL_ARCW3], READY_R_ARCW3, 1);
        ST(arc[RDFLOW_ARCW2  ], READ_ARCW2, 0);
        ST(arc[WRIOCOLL_ARCW3], WRITE_ARCW3, datasize);

        DATA_MEMORY_BARRIER;
        break;

    /* set the buffer to IO addres with R=W=0 */
    case arc_set_base_address_from_arc :
        ST(arc[BUF_PTR_ARCW0 ], BASEIDXOFFARCW0, physical_to_offset(buffer, long_offset));
        ST(arc[RDFLOW_ARCW2  ], READY_W_ARCW2, 1);
        ST(arc[RDFLOW_ARCW2  ], READ_ARCW2, 0);
        ST(arc[WRIOCOLL_ARCW3], WRITE_ARCW3, 0);

        DATA_MEMORY_BARRIER;
        break;
        
    default : 
        break; 
    }
}



/* --------------------------------------------------------------------------------------------------
      
      read arc's pointer and size => XDM structure
 */
static void read_arc_xdm (uint32_t *arc, uint32_t in0out1, intPtr_t *long_offset, 
                   data_buffer_t *xdm_data/*, uint32_t *all_formats*/)
{
    uint32_t  fifosize;

    fifosize = EXTRACT_FIELD(arc[BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);

    if (U(0) != in0out1)
    {
        /* output buffer of the SWC */
        xdm_data->address = (uint8_t *)(arc_extract_info_pt (arc, arc_write_address, long_offset));
        xdm_data->size    = arc_extract_info_int (arc, arc_free_area, long_offset);
        // TODO implement size modulo frame_size of the format to recover synchronization errors
        // solution synchronize on the producer: the distance to WRITE_ARCW3 is a multiple of the frame size
    }
    else
    {
        /* input buffer of the SWC */
        xdm_data->address = (uint8_t *)(arc_extract_info_pt (arc, arc_read_address, long_offset));
        xdm_data->size    = arc_extract_info_int (arc, arc_free_area, long_offset);
    }
}


/*----------------------------------------------------------------------------
    Multiprocess realignment of data to base addresses, algorithm :

    - output FIFO write pointer is incremented AND a check is made for data 
    re-alignment to base adresses (to avoid address looping)
    then the "ALIGNBLCK_ARCW3" is set. The SWC don't wait and let the 
    consumer manage the alignement

 *----------------------------------------------------------------------------*/
static void check_arc_and_rewind (uint32_t *arc, uint32_t in0out1, intPtr_t *long_offset, 
                           data_buffer_t *xdm_data)
{
    uint32_t fill_thr;
    uint32_t fifosize;
    uint32_t diff;
    intPtr_t long_base;
    uint8_t bool;
    uint8_t *write;
    uint8_t *data;


    /* is it simple/null arc */
    if (RD(arc[BUFSIZDBG_ARCW1], SHAREDARC_ARCW1) == 1u)
    {   
        ST(arc[WRIOCOLL_ARCW3], WRITE_ARCW3, 0);
        ST(arc[RDFLOW_ARCW2], READ_ARCW2, 0);
        ST(arc[RDFLOW_ARCW2], READY_W_ARCW2, 1);
        ST(arc[WRIOCOLL_ARCW3], READY_R_ARCW3, 1);
        return;
    }
     
    fifosize =  EXTRACT_FIELD(arc[BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
    long_base = EXTRACT_FIELD(arc[BUF_PTR_ARCW0  ], BASEIDX_ARCW0);

    /* is it an output arc ? */
    if (0u != in0out1)
    {
        /* output buffer of the SWC : check the pointer increment */
        write = (uint8_t *)arc_extract_info_pt(arc, arc_write_address, long_offset);
        data = xdm_data->address;

        /* is there debug/monitoring activity to do ? */
        if (RD(arc[RDFLOW_ARCW2], COMPUTCMD_ARCW2) != 0u)
        { /* TODO : implement the data monitoring 
            arm_arc_monitor(*, n, RD(,COMPUTCMD_ARCW2), RD(,DEBUG_REG_ARCW1))
           */
        }


        diff = convert_ptr_to_int(data) - convert_ptr_to_int(write);   // for debug

        data = xdm_data->address;
        diff = convert_ptr_to_int(data) - convert_ptr_to_int((uint8_t *)long_base);   
        ST(arc[WRIOCOLL_ARCW3], WRITE_ARCW3, diff);

        /* are there enough data in the FIFO to tell for data realignment? */
        fill_thr = RD(arc[RDFLOW_ARCW2], THRESHOLD_ARCW2);
        bool = ((fifosize - diff) > (fifosize >> (1U + fill_thr))) ? 1 : 0;
        ST(arc[WRIOCOLL_ARCW3], ALIGNBLCK_ARCW3, bool);
        DATA_MEMORY_BARRIER;
    }
    /* this is an input arc */
    else
    {
        /* input buffer of the SWC, update the read index*/
        diff = convert_ptr_to_int(xdm_data->address) - convert_ptr_to_int((uint8_t *)long_base);   
        ST(arc[RDFLOW_ARCW2], READ_ARCW2, diff);

        /* check the need for data flush */
        if (U(0) != TEST_BIT(arc[BUFSIZDBG_ARCW1], MPFLUSH_ARCW1_LSB))
        {   DATA_MEMORY_BARRIER
        }
    }

}

/* --------------------------------------------------------------------------------------------------
      check input ring buffers :
      each Stream instance has a list of graph boundary to check for in/out data
 */
static void check_graph_boundaries(
        /* struct stream_local_instance */uint32_t *pstream_inst, 
        uint32_t *pio, 
        uint8_t nio, 
        uint32_t *arcs,
        intPtr_t *long_offset)
{
    uint32_t iio;
    uint32_t io_mask;
    uint32_t arc_idx;
    uint8_t need_data_move;
    uint32_t size;
    uint8_t *buffer;
    struct platform_control_stream p_data_move;

    io_mask = EXTRACT_FIELD(pstream_inst[STREAM_INSTANCE_WHOAMI_PORTS], BOUNDARY_PARCH);
    for (iio = 0u; iio < nio; iio++)
    {
        /* ports where the peripheral is master are not checked (MASTER_FOLLOWER_IOMEM) */

        /* does this port needs to be managed by the Stream instance ? */
        if (0u == U(io_mask & ((uint32_t)1 << iio))) 
            continue; 

        /* a previous request is in process, no need to ask again */
        if (0u != TEST_BIT(*pio, REQMADE_IOFMT_LSB))
        {   continue;
        }

        arc_idx = EXTRACT_FIELD(*pio, IOARCID_IOFMT);

        /* if this is an input stream : check the buffer is empty  */
        if (RX0_TO_GRAPH == EXTRACT_FIELD(*pio, RX0TX1_IOFMT))
        {   need_data_move = ((TEST_BIT(arcs[arc_idx + (uint32_t)RDFLOW_ARCW2], READY_W_ARCW2_LSB)) > 0u)? 1 : 0;

            /* input ring buffer starts from "write" index for the io producer */
            buffer = (uint8_t *)(arc_extract_info_pt (&(arcs[arc_idx]), arc_write_address, long_offset));
            size = arc_extract_info_int (&(arcs[arc_idx]), arc_free_area, long_offset);
        }

        /* if this is an output stream : check the buffer is READY_R */
        else
        {   need_data_move = ((TEST_BIT(arcs[arc_idx + WRIOCOLL_ARCW3], READY_R_ARCW3_LSB)) > 0u) ? 1 : 0;

            /* output ring buffer starts from "read" index for the io consumer */
            buffer = (uint8_t *)(arc_extract_info_pt (&(arcs[arc_idx]), arc_read_address, long_offset));
            size = arc_extract_info_int (&(arcs[arc_idx]), arc_data_amount, long_offset);
        }
        
        if (0u != need_data_move)
        {
            /* 
                TODO When the IO is slave and stream_io_domain=PLATFORM_LOW_DATA_RATE_XX 
                 check the time interval from last frame (by a read of the time-stamp in 
                 the FIFO) and current time, to deliver a data rate close to :
                 "platform_io_manifest.stream_settings_default.SAMPLING_FMT1_"
                Trigger the data request some time ahead to let the converters
                 have the time to exchange data (image, remote temperature sensors,
                 characters on a display, ..)
            */

            /* ask the firmware to awake this port */
            p_data_move.fw_idx = EXTRACT_FIELD(*pio, FW_IO_IDX_IOFMT);

            /* using the address of the ring buffer */
            p_data_move.buffer.address = buffer;
            p_data_move.buffer.size = size;
            platform_al(PLATFORM_IO_DATA, (uint8_t *)&p_data_move, 0u, 0u);
        }
    }
}

#if MULTIPROCESSING
uint32_t check_hw_compatibility(uint32_t whoami, uint32_t bootParamsHeader) 
{
    uint8_t match = 1;

    if (RD(bootParamsHeader, ARCHID_BP) > 0u) /* do I care about the architecture ID ? */
    {   match = U8(RD(bootParamsHeader, ARCHID_BP) == RD(whoami, ARCHID_PARCH));
    }

    if (RD(bootParamsHeader, PROCID_BP) > 0u) /* do I care about the processor ID ? */
    {   match = match & U8(RD(bootParamsHeader, PROCID_BP) == RD(whoami, PROCID_PARCH));
    }
    return match;
}
#else
#define check_hw_compatibility(whoami, bootParamsHeader) 1u  
#endif


/*----------------------------------------------------------------------------
   scan the graph and execute a "scan_function_ptr" task
   algorithm :
     - scan the linked-list of node
     - searching for the ones having input data and free space in the ouput buffer
     - check the input ring buffers at the boundary of the graph are full
     - check the output ring buffers at the boundary of the graph are empty

     Format of the linked-list :
     word 0 = 32bits header, bit-field giving the index of the node 
     word 1 =  optional (flag in the header) instance index 
     word 2 =  2 x 16bits indexes to the input and output arcs
     word 3 =  optional boot parameters (variable length of this field)


    Scheduler algorithm :
    1 the SWC is checked for data input amount and free area on output
    - the SWC is checked not to be blocked on its output (ALIGNBLCK_ARCW3=0)

    2 the SWC is locked from other process activity :
        COLLISION_GI<=INST_ID_PARCH
    - if COLLISION_GI<>INST_ID_PARCH collision occured : continue to next SWC

    (3) if the SWC generating the input data was blocked (ALIGNBLCK_ARCW3=0) 
        during its attempt to realign the data to base address, then it is 
        the responsibility of the consumer node (current SWC) to realign the 
        data, and clear the flag. 

    - SWC is called for execution ("NODE_RUN")

    - input FIFO read pointer is updated (incremented)

    - output FIFO write pointer is incremented AND a check is made for data 
        re-alignment to base adresses (to avoid address looping)
        if the arc is locked by the SWC consumer, then the "ALIGNBLCK_ARCW3" 
        is set. The SWC don't wait and let the consumer manage the alignement

    - depending on the SWC return value, the STATUS_GI is set to "need_rerun"
        and the SWC will be run again asap after checking the graph boundaries
        for data moves

    - SWC is unlocked : COLLISION_GI<=0
     
 *----------------------------------------------------------------------------*/

void stream_scan_graph (intPtr_t *parameters, int8_t return_option, 
    int8_t script_option, int8_t reset_option) 
{
    /* struct stream_local_instance */uint32_t *pinst;

    uint8_t *pt8b_collision_arc_1;
    uint8_t tmp8;
	uint32_t *graph;
    uint32_t *all_arcs;
    uint32_t *linked_list_ptr;
    uint32_t *linked_list_ptr_header;
    uint32_t *linked_list_ptr_bootparam;
    intPtr_t *long_offset;
    uint32_t swc_instance;
    uint32_t *swc_instance_ptr;
    uint16_t preset;

    uint32_t swc_header;
    uint32_t whoami;
    p_stream_node address_swc;
    uint8_t nio;
    p_stream_node *all_nodes;
    uint32_t *pio;
    uint32_t check;
    uint8_t some_components_have_data;
    data_buffer_t xdm_data[MAX_NB_STREAM_SWC];
    uint32_t *arc0;
    uint32_t *arc1;
    uint32_t *all_formats;
    uint32_t bootParamsHeader;

    platform_al (PLATFORM_OFFSETS,(uint8_t *)&long_offset,0,0);

    /* load the graph pointers */
    graph = convert_int_to_ptr(parameters[STREAM_PARAM_GRAPH]);
    {   /* struct stream_local_instance */uint32_t *ptmp;
        ptmp = GRAPH_STREAM_INST(graph,long_offset);
        ptmp = &(ptmp[RD(parameters[STREAM_PARAM_CTRL], INSTIDX_PARAM)]);
        pinst = (/* struct stream_local_instance */uint32_t *)GRAPH_STREAM_INST(graph,long_offset);
    }
    all_arcs = GRAPH_ARC_LIST_ADDR(graph,long_offset);

    all_nodes = convert_int_to_ptr(pinst[STREAM_INSTANCE_NODE_ENTRY_POINTS]);
    whoami = pinst[STREAM_INSTANCE_WHOAMI_PORTS];
    pio = (uint32_t *)(GRAPH_IO_CONFIG_ADDR(graph,long_offset));
    nio = RD(graph[0],NBIO_GRAPH0);
    all_formats = GRAPH_FORMAT_ADDR(graph,long_offset);

    if (script_option == STREAM_SCHD_SCRIPT_START) { stream_execute_script(parameters);}

    /* loop until all the components are blocked by the data streams */
	do 
    {  
        linked_list_ptr = GRAPH_LINKDLIST_ADDR(graph,long_offset); 

        /* continue from the last position */
        linked_list_ptr = &(linked_list_ptr[RD(parameters[STREAM_PARAM_CTRL],LASTSWC_PARAM)]);
        some_components_have_data = 0u;  

        /* read the linked-list until finding the SWC index "LAST_WORD" */
	    do 
        {   
            /* check the boundaries of the graph, not during end/stop periods */
            if (reset_option == 0) { check_graph_boundaries(pinst, pio, nio, all_arcs, long_offset); }
            
            /* check this node can be executed on this processor */
            linked_list_ptr_header = linked_list_ptr;
            swc_header = *linked_list_ptr;    
            linked_list_ptr++;
            arc0 = &(all_arcs[SIZEOF_ARCDESC_W32 * RD(swc_header, ARCIN_GI)]);
            arc1 = &(all_arcs[SIZEOF_ARCDESC_W32 * RD(swc_header, ARCOUT_GI)]);
            
            {   uint16_t x;
                uint16_t idx_swc;
                idx_swc = (uint16_t)RD(swc_header, SWC_IDX_GI);
                address_swc = all_nodes[idx_swc];
                swc_instance = *linked_list_ptr;
                linked_list_ptr++;
                linked_list_ptr = &(linked_list_ptr[RD(swc_instance, NBALLOC_INST)]); // skip the memreq table
                bootParamsHeader = *linked_list_ptr;
                linked_list_ptr++;
                preset = (uint16_t)RD(bootParamsHeader, PRESET_BP);
                linked_list_ptr_bootparam = linked_list_ptr;
                x = (uint16_t)RD(bootParamsHeader, N32LENGTH_BP); // skip the table of parameters
                linked_list_ptr = &(linked_list_ptr[x]);
            
                /* does the SWC is executable on this processor */
                if (0U == check_hw_compatibility(whoami, bootParamsHeader))
                {   continue;
                }
            }
            
            /* ================================================================*/
            /* ---------------------SWC reservation attempt -------------------*/            
            /* if the SWC is already used skip it */
            pt8b_collision_arc_1 = (uint8_t *)arc1;
            {   uint32_t j;
                j = U(4u*WRIOCOLL_ARCW3 + COLLISION_ARC_OFFSET_BYTE);
                pt8b_collision_arc_1 = &(pt8b_collision_arc_1[j]);
            }
            RD_BYTE_MP_(check, pt8b_collision_arc_1);
            if (check != U8(0))
            {   continue;
            }
            tmp8 = (uint8_t)RD(whoami, INST_ID_PARCH);   

            if (0u == WR_BYTE_AND_CHECK_MP_(pt8b_collision_arc_1, tmp8))
            {   continue;   /* a collision occured, don't wait */
            }
            /* ------------------SWC is now locked for me ! --------------------*/            
            /* =================================================================*/

            swc_instance = pack2linaddr_int(swc_instance,long_offset);
            swc_instance_ptr = (uint32_t *)convert_int_to_ptr(swc_instance);

            /* ---------------- reset phase of the graph ? -------------------- */
            if (reset_option > 0)
            {   
            #define MEMRESET (MAX_NB_MEM_REQ_PER_NODE + (SIZEOF_ARCDESC_SHORT_W32*MAX_NB_ARC_PER_SWC_V0))
                intPtr_t memreq_physical[MAX_NB_MEM_REQ_PER_NODE + (SIZEOF_ARCDESC_SHORT_W32*MAX_NB_ARC_PER_SWC_V0)];
                uint16_t nbmem;
                uint8_t i;
                uint32_t *memreq;


                memreq = &(linked_list_ptr_header[1]);
                nbmem = (uint16_t)RD(memreq[0],NBALLOC_INST);
                memreq_physical[0] = swc_instance;

                for (i = 1; i < MAX_NB_MEM_REQ_PER_NODE; i++)
                {   /* create pointers to the right memory bank */
                    memreq_physical[i] = (i <= nbmem) ? 
                        pack2linaddr_int(memreq[i], long_offset) : 0u;
                }      

                /* push the information related to the format of the arcs */
                //for (j = 0; j < MAX_NB_ARC_PER_SWC_V0; j++)
                {   uint32_t *F;
                    F = &(all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc0,FORMATIDX_ARCW0)]);
                    memreq_physical[i++] = *F++;    /* Word 0 SIZSFTRAW_FMT0 */
                    memreq_physical[i++] = *F;      /* Word 1 SAMPINGNCHANM1_FMT1 */
                    F = &(all_formats[STREAM_FORMAT_SIZE_W32 * RD(arc1,FORMATIDX_ARCW0)]);
                    memreq_physical[i++] = *F++;    /* Word 0 SIZSFTRAW_FMT0 */
                    memreq_physical[i++] = *F;      /* Word 1 SAMPINGNCHANM1_FMT1 */
                }      
                /*
                    add for each arc : [nb channel, sampling rate, interleaving, time-stamp format]
                */
                stream_calls_swc (address_swc, PACK_COMMAND(1,1,0,preset,STREAM_RESET), 
                    (uint32_t *)memreq_physical, 
                    (data_buffer_t *)arm_stream_services, 
                    &check);
            }

            /* ---------------- parameter was changed, or reset ? -------------------- */
            if (reset_option > 0 ||
                (LINKEDLIST_IS_IN_RAM(graph) && TEST_BIT(swc_header, NEWPARAM_GI_LSB)))
            {   
                uint8_t i;
                uint8_t nparam;
                uint8_t *ptr_param8b;

                /*
                    TODO : allocate a scratch area to copy the parameters, and avoid SWC 
                        to have access to Stream internal memory area
                */
                ptr_param8b = (uint8_t *)linked_list_ptr_bootparam;
                nparam = *ptr_param8b;
                ptr_param8b++;

                /* do we load the full set of parameters ? */
                if (nparam == ALLPARAM_)
                {   
                    stream_calls_swc (address_swc, PACK_COMMAND(1,1,ALLPARAM_,preset,STREAM_SET_PARAMETER), 
                        swc_instance_ptr,
                        (data_buffer_t *)ptr_param8b, 
                        &check);
                }
                else
                /* else : load the parameters one-by-one */
                {   uint8_t param_tag;
                    uint8_t param_nb_bytes;
                    for (i = 0; i < nparam; i++)
                    {   param_tag = *ptr_param8b;
                        ptr_param8b++;
                        param_nb_bytes = *ptr_param8b;
                        ptr_param8b++;
                
                        stream_calls_swc (address_swc, PACK_COMMAND(1,1,param_tag,preset,STREAM_SET_PARAMETER), 
                            swc_instance_ptr,
                            (data_buffer_t *)ptr_param8b, 
                            &check);
                        ptr_param8b = &(ptr_param8b[param_nb_bytes]);
                    }
                }
                
                if (reset_option > 0)
                {   /* SWC is unlocked */
                    WR_BYTE_MP_(pt8b_collision_arc_1, 0u);
                    continue;
                }
                /* ready to accept new parameters : clear the flag */
                if (LINKEDLIST_IS_IN_RAM(graph)) 
                {   CLEAR_BIT_MP(swc_header, NEWPARAM_GI_LSB);
                }
            }

            
            /* ==========================================================================*/
            /* end of graph ? */
            if (reset_option < 0)
            {   stream_calls_swc (address_swc, PACK_COMMAND(1,1,0,0,STREAM_END), 
                    (uint32_t *)swc_instance, 0u, &check);
                /* SWC is unlocked */
                WR_BYTE_MP_(pt8b_collision_arc_1, 0u);
                continue;
            }
            /* ==========================================================================*/
            
            /* several iterations to benefit from the hot cache */
            //uint8_t iswc_repeat;
            //for (iswc_repeat = 0; iswc_repeat < MAX_SWC_REPEAT; iswc_repeat++)

            /* check input arc has enough data and output arc is empty and unblocked 
                all those arcs data structures are made to have those two lines the most
                efficiently implemented : spend the least possible time to decide to skip 
                this SWC
            */
            if (0u  != TEST_BIT (arc0[WRIOCOLL_ARCW3], READY_R_ARCW3_LSB)
                && (0u != TEST_BIT (arc1[RDFLOW_ARCW2], READY_W_ARCW2_LSB)))
            {   continue;
            }
            some_components_have_data = 1u;

            /* if the SWC generating the input data was blocked (ALIGNBLCK_ARCW3=0) 
                during its attempt to realign the data to base address, then it is 
                the responsibility of the consumer node (current SWC) to realign the 
                data, and clear the flag. 
            */
            if (0u != TEST_BIT (arc0[WRIOCOLL_ARCW3], ALIGNBLCK_ARCW3_LSB))
            {
                arc_data_operations (arc0, arc_data_realignment_to_base, long_offset, 0, 0);
                CLEAR_BIT (arc0[WRIOCOLL_ARCW3], ALIGNBLCK_ARCW3_LSB);
                DATA_MEMORY_BARRIER;
            }
        
            /* prepare the pointers to input and output buffers (MAX_NB_ARC_PER_SWC_V0) */
            read_arc_xdm (arc0, 0u, long_offset, &(xdm_data[0])/*, all_formats*/);
            read_arc_xdm (arc1, 1u, long_offset, &(xdm_data[1])/*, all_formats*/);

            /* if the SWC is using a relocatable memory segment to TCM, the base address is loaded 
                in the first address of its instance */
            if (0u != RD(swc_header, TCM_INST_GI))
            {   intPtr_t *tmp = (intPtr_t *)swc_instance;
                *tmp = long_offset[MBANK_DMEMFAST];
            }

            if (script_option == STREAM_SCHD_SCRIPT_BEFORE_EACH_SWC) {stream_execute_script(parameters); }

            do 
            {
                #define SWC_TASK_NOT_COMPLETED 0
                /* call the SWC, returns the information "0 == SWC needs to be called again" 
                   to give some CPU periods for trigering data moves 
                   long SWC can be split to allow data moves without RTOS */
                stream_calls_swc (address_swc, PACK_COMMAND(1,1,0,preset,STREAM_RUN), 
                    swc_instance_ptr, xdm_data,  &check);

                if (SWC_TASK_NOT_COMPLETED == check)
                {   /* check the boundaries of the graph */
                    check_graph_boundaries(pinst, pio, nio, all_arcs, long_offset);
                }
            } 
            while (check == SWC_TASK_NOT_COMPLETED);


            if (script_option == STREAM_SCHD_SCRIPT_AFTER_EACH_SWC) {stream_execute_script(parameters); }

            /*  output FIFO write pointer is incremented AND a check is made for data 
                re-alignment to base adresses (to avoid address looping)
                if the arc is locked by the SWC consumer, then the "ALIGNBLCK_ARCW3" 
                is set. The SWC don't wait and let the consumer manage the alignement 
            */
            check_arc_and_rewind(arc0, 0u, long_offset, &(xdm_data[0])); 
            check_arc_and_rewind(arc1, 1u, long_offset, &(xdm_data[1])); 
            
            /* SWC is unlocked : change the output arc */
            WR_BYTE_MP_(pt8b_collision_arc_1, 0u);

            tmp8 = (GRAPH_LAST_WORD == RD(*linked_list_ptr,SWC_IDX_GI)) ? 1 : 0;
            if (tmp8 == 1)
            {   /* rewind the index when the last word is detected */
                linked_list_ptr = GRAPH_LINKDLIST_ADDR(graph,long_offset);
            }

            if (return_option == STREAM_SCHD_RET_END_EACH_SWC)
            {   break;
            }

	    }  while (tmp8);

        if ((return_option == STREAM_SCHD_RET_END_ALL_PARSED) || (return_option == STREAM_SCHD_RET_END_EACH_SWC))
        {   some_components_have_data = 0u; 
            break;
        }
        if (script_option == STREAM_SCHD_SCRIPT_END_PARSING) {stream_execute_script(parameters); }

    } while ((return_option == STREAM_SCHD_RET_END_SWC_NODATA) && (0u != some_components_have_data));


    if (script_option == STREAM_SCHD_SCRIPT_END) {stream_execute_script(parameters); }

    /* save last position */
    ST(parameters[STREAM_PARAM_CTRL],LASTSWC_PARAM, linked_list_ptr - GRAPH_LINKDLIST_ADDR(graph,long_offset));
}
