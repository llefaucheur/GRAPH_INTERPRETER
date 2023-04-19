/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_demo.c
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

#include "stream_const.h"
#include "stream_types.h"   /* all non preprocessor directives */
#include <string.h>         /* memcpy */

/*----------------------------------------------------------*/

intPtr_t pack2linaddr_int(uint32_t data, intPtr_t *offsets)
{
    intPtr_t tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    uint32_t tmp0;
    tmp0 = RD(data, BASEIDX_ARCW0);      tmp1 = (intPtr_t)tmp0;
    tmp0 = RD(data, BASESHIFT_ARCW0);    tmp2 = (intPtr_t)tmp0;
    tmp0 = RD(data, DATAOFF_ARCW0);      tmp3 = (intPtr_t)tmp0;

    tmp4 = tmp1 << (tmp2 << U(2));
    tmp5 = offsets[tmp3];
    tmp6 = tmp4 + tmp5;

    return tmp6;
}

intPtr_t * pack2linaddr_ptr(uint32_t data, intPtr_t *offsets)
{
    return convert_int_to_ptr (pack2linaddr_int(data, offsets));
}


/*---------------------------------------------------------------------------
 * Application execution
 *---------------------------------------------------------------------------*/
void stream_demo_init(uint8_t stream_instance, uint8_t total_nb_stream_instance,
            uint32_t *graph, 
            uint32_t graph_size,
            intPtr_t *parameters
            )
{
    uint32_t nio;
    uint32_t iio; 
    uint32_t fw_idx;
    /* struct stream_local_instance */uint32_t *pinst;
    uint32_t *pio;
    uint32_t stream_format_io;
    uint32_t stream_format_io_setting;
    struct platform_control_stream set_stream;
    uint32_t io_mask;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    uint32_t *graph_src;
    uint32_t *graph_dst; 
    uint32_t graph_bytes_in_ram;
    uint8_t procID;
    uint8_t archID;
    intPtr_t *long_offset;
    uint32_t tmp32;

    /* 
        start copying the graph in RAM 
        initialize the current Stream instance (->node_entry_points, ->graph)
        initialize the IO boundaries of the graph
    */

    /* read the processors' long_offset table */
    platform_al (PLATFORM_OFFSETS, (uint8_t *)(&long_offset),0,0);

    graph_src = graph_dst = graph; 

    /* if I am the master processor (archID=1 + procID=0), copy the graph, or wait */
    platform_al (PLATFORM_PROC_ID, &procID, &archID,0);
    if ((archID == 1U) && (procID == 0U))
    {
        tmp32 = RD(graph[0],COPY_CONF_GRAPH0);
        switch (tmp32)
        {
        /*
           Graph is read from Flash and copied in RAM, 
           for speed, ability to patch the parameters of each SWC at boot time
           and on-the-fly with the bit NEWPARAM_GI

           data move: 
            graph_src = parameter const uint32_t *graph
            graph_dst = pack2linaddr(parameter *graph[2] RAM)
            graph_size= parameter uint32_t graph_size,
            parameters->graph = graph_dst;
        */
        case COPY_CONF_GRAPH0_COPY_ALL_IN_RAM:

            graph_src = graph;
            graph_dst = GRAPH_RAM_OFFSET_PTR(graph,long_offset);
            graph_bytes_in_ram = graph_size;

            MEMCPY((char *)graph_dst, (char *)graph_src, graph_bytes_in_ram);
            parameters[STREAM_PARAM_GRAPH] = convert_ptr_to_int(graph_dst);
            break;

        /*
           Graph is read from Flash and a small part is copied in RAM
           to save RAM (for example Cortex-M0 with 2kB of SRAM)

            graph_src = parameter const uint32_t *graph  +  graph[0].OFFSET_GRAPH0
            graph_dst = pack2linaddr(parameter *graph[2] RAM)
            graph_size= graph[0].LENGTHRAM_GRAPH0
            parameters->graph = graph_src;
        */
        case COPY_CONF_GRAPH0_COPY_PARTIALLY:
            tmp32 = RD(graph[0], OFFSET_GRAPH0);
            graph_src = (&(graph[tmp32]));
            graph_dst = GRAPH_RAM_OFFSET_PTR(graph,long_offset);
            graph_bytes_in_ram = sizeof(uint32_t) * RD(graph[0], LENGTHRAM_GRAPH0);

            MEMCPY((char *)graph_dst, (char *)graph_src, graph_bytes_in_ram);

            parameters[STREAM_PARAM_GRAPH] = convert_ptr_to_int(graph);
            break;

        /*
            Graph is already in RAM, possibly not in the right memory banks
            but relative offset are still valid. Absolute offset are recomputed

            No data move
            Absolute offset graph[1] "Flash" and graph[2] "RAM" are identical and reloaded 
            with the value recomputed using physical_to_offset(const uint32_t *graph)
            parameters->graph = graph_src;
        */
        default:
        case COPY_CONF_GRAPH0_ALREADY_IN_RAM:
            graph_src = graph;
            graph_src[1] = physical_to_offset((uint8_t *)graph,long_offset);
            graph_src[2] = graph_src[1];
            graph_dst = &(graph_src[graph_size/sizeof(uint32_t)]);

            parameters[STREAM_PARAM_GRAPH] = graph_src[1];
            break;
        }

        /* all other process can be released from wait state */
        platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); 
    }

    /* declare the graph memory as shared */
    if (RD(graph[0],SHAREDMEM_GRAPH0))
    {   platform_al (PLATFORM_MP_GRAPH_SHARED, (uint8_t *)graph_src, (uint8_t *)graph_dst,0);
    }

    /* I wait the graph is copied in RAM */
    {   uint8_t wait; 
        do {
            platform_al (PLATFORM_MP_BOOT_WAIT, &wait, 0,0);
        } while (0u != wait);
    }

    parameters[STREAM_PARAM_CTRL] = stream_instance;  

    /*-------------------------------------------*/

    /* local stream instance initialization */
    pinst = (/* struct stream_local_instance */uint32_t *)GRAPH_STREAM_INST(graph, long_offset);
    pinst = &(pinst[stream_instance]);
    pinst[STREAM_INSTANCE_NODE_ENTRY_POINTS] = convert_ptr_to_int(node_entry_point_table);
    pinst[STREAM_INSTANCE_WHOAMI_PORTS] = PACKWHOAMI(stream_instance,procID,archID,0x003); // scan 2 io ports

    /* 
        initialization of all the SWC 
    */
    arm_stream(STREAM_RESET, parameters, 
            (void *)STREAM_SCHD_RET_END_ALL_PARSED, 
            (void *)STREAM_SCHD_NO_SCRIPT); 

    platform_al (PLATFORM_MP_RESET_DONE, &stream_instance,0,0);

    /* wait all the process have initialized the graph */
    {   uint8_t wait; 
        do {
            platform_al (PLATFORM_MP_RESET_WAIT, &wait, &total_nb_stream_instance, 0);
        } while (wait);
    }

    /*-------------------------------------------*/
    /* 
        initialization of the graph IO ports 
    */     
    io_mask = RD(pinst[STREAM_INSTANCE_WHOAMI_PORTS], BOUNDARY_PARCH);
    pio = GRAPH_IO_CONFIG_ADDR(graph,long_offset);
    nio = RD(graph[0],NBIO_GRAPH0);
    all_arcs = GRAPH_ARC_LIST_ADDR(graph,long_offset);


    for (iio = 0; iio < nio; iio++)
    {
        uint8_t io_command;

        stream_format_io = pio[0];
        stream_format_io_setting = pio[1];

        /* does this port is managed by the Stream instance ? */
        if (0 == (io_mask & (1U << iio))) 
            continue; 

        fw_idx = RD(stream_format_io, FW_IO_IDX_IOFMT);
        set_stream.fw_idx = fw_idx;

        /* default value settings */
        set_stream.domain_settings = stream_format_io_setting;

        /* the graph compiler uses the platform manifest to know the need for each IO 
           to receive from Stream an allocated memory buffers, and its arc descriptor.
           When the processing is in-place, this arc descriptor is not associated to a 
           buffer.
           */
        io_command = RD(stream_format_io, IOCOMMAND_IOFMT);

        if (IO_COMMAND_SET_BUFFER_RX == io_command ||
            IO_COMMAND_SET_BUFFER_TX == io_command  )
        {
            iarc = sizeof(uint32_t) * RD(stream_format_io, IOARCID_IOFMT);
            set_stream.buffer.address = 
                pack2linaddr_int(all_arcs[iarc + BUF_PTR_ARCW0],long_offset);
            set_stream.buffer.size = 
                (uint32_t)RD(all_arcs[iarc + BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
        }
        else
        {   set_stream.buffer.address = 0;
            set_stream.buffer.size = 0;
        }
        /* the Stream instance holds the memory offsets */
        set_stream.graph = convert_int_to_ptr(parameters[STREAM_PARAM_GRAPH]);
        platform_al (PLATFORM_IO_SET_STREAM, (uint8_t *)&set_stream, 0, 0);

        pio += STREAM_IOFMT_SIZE_W32;
    }

    /* release the stack before graph execution */
}

/*--------------------------------------------------------------------------- */
 
