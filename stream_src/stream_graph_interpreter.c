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


#include <stdint.h>


#include "stream_const.h"
#include "stream_types.h"
#include "stream_extern.h"
#include <string.h>         /* memcpy */
SECTION_START 


static void stream_copy_graph(arm_stream_instance_t *stream_instance, uint32_t *graph);
static void stream_init_io(arm_stream_instance_t *stream_instance);

#define L S->long_offset
#define G S->graph

/*
   Main entry point, used by the application and scripts

   Interactions between the graph and its outside world :
   - direct call to arm_stream()
   - platform_io streams to FIFO

   Commands :
   - reset, run, end
   - interpreter of a FIFO of commands

 */

/**
  @brief            Graph interpreter entry point 
  @param[in]        command     operation to do (reset, run, stop)
  @param[in]        instance    Graph interpreter instance pointer
  @param[in]        data        graph to process
  @return           none

  @par              reset, run and stop a graph
                      
  @remark
 */
void arm_graph_interpreter (uint32_t command,  arm_stream_instance_t *S, uint32_t *data)
{   
	switch (command)
    {
        /* usage: arm_stream(STREAM_RESET, &instance,graph_input); */
	    case STREAM_RESET: 
	    {   stream_copy_graph (S, data);
            S->linked_list_ptr = S->linked_list;

            stream_scan_graph (S, STREAM_RESET);

            //@@@@@ /* skip the first component = debug script node */
            //#define XX 8 // size of the SWC of the debug trace script
            //#define YY 3 // offset to the instance of the script
            //S->script_offsets = (uint32_t *)PACK2LINADDR(S->long_offset, S->linked_list[YY]);
            //S->linked_list = &(S->linked_list[XX]);
            //S->linked_list_ptr = S->linked_list;

            stream_init_io (S);
            break;

        }

        /* usage: arm_stream(STREAM_RUN, &instance,0); */
	    case STREAM_RUN:   
	    {   if (RD(S->parameters, INSTANCE_ON_PARINST))
            {   stream_scan_graph (S, STREAM_RUN);
            }
            break;
        }   

        /* usage: arm_graph_interpreter (STREAM_STOP, &instance, 0); */
        case STREAM_STOP:
	    {   stream_scan_graph (S, STREAM_STOP);
            break;
        }

        /* arm_graph_interpreter (STREAM_READ_PARAMETER, 8 bytes use-case, 0, 0)*/
        case STREAM_READ_PARAMETER: 
        {   /* byte stream format 64bits/8bytes: command + length, byte-stream. Use-case : 
                node set/read parameters 
               Implemented with the main script of the (sub) graph
             */
            break;
        }

        //  Eight registered callback for scripts (CALS [8..15])
        //  Default "CALS" callback(stream_script_callback)
        //      #8  sleep / deep - sleep activation
        //      #9  system regsters access : who am I ?
        //      #10 timer control(default implementation with SYSTICK)
        // 
        //    example specific : IP address, password to share, Ping IP to blink the LED, read RSSI, read IP@

        default:
            break;
    }
}


/**
  @brief        Graph copy in RAM (STREAM_RESET phase)
  @param[in]    instance   global data of this instance
  @param[in]    graph      input graph
  @return       none

  @par          Initialization of the Graph interpreter instance 
                   long offsets, address of nodes, arcs, stream formats

                The Graph is a table using uint32 :
                - size of the graph in Words
                [0] 27b RAM address of part/all the graph, HW-shared MEM configuration, which part is going in RAM
                [1] number of FORMAT, size of SCRIPTS
                [2] size of LINKEDLIST, number of STREAM_INSTANCES
                [3] number of ARCS, number of DEBUG registers

                The application shares a pointer, the register [0] tells the graph is either:
                - already in RAM : nothing to do
                - to move in RAM (internal / external) partially or totally
                
                stream_copy_graph() implements this data move and calls platform AL to synchronize the other processor 
                to let them initialize the node instances in parallel.
  @remark
 */
/*---------------------------------------------------------------------------
 * Application execution
 *---------------------------------------------------------------------------*/
static void stream_copy_graph(arm_stream_instance_t *S, uint32_t *graph0)
{
    uint8_t RAMsplit;
    uint32_t graph_words_in_ram, graph_size_word32;
    uint32_t *graph_src;
    uint32_t *graph_dst;

    /* 
        start copying the graph in RAM 
        initialize the current Graph interpreter instance (->node_entry_points, ->graph)
        initialize the IO boundaries of the graph

        read the processors' long_offset table */
    platform_al (PLATFORM_OFFSETS, (uint8_t *)&(S->long_offset),0,0);
    
    /* realign "graph0" */
    graph_size_word32 = graph0[0];
    graph0 = &(graph0[GRAPH_SIZE_SKIP_WORD0]);

    /* default : 
        COPY_CONF_GRAPH0_COPY_ALL_IN_RAM    graph0 in flash -> GRAPH_RAM_OFFSET_PTR()
        COPY_CONF_GRAPH0_ALREADY_IN_RAM     graph0 in RAM   -> GRAPH_RAM_OFFSET_PTR()  
     */
    RAMsplit = (uint8_t)RD((graph0[0]),RAMSPLIT_GRAPH0);
    graph_words_in_ram = graph_size_word32;
    graph_src = graph0;
    graph_dst = S->graph = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,graph0);

    /* Graph is read from Flash and a small part is copied in RAM
       to save RAM (for example Cortex-M0 with 2kB of internal SRAM)

                   RAMsplit Option 0/3    1     2 
        IO x2 words                RAM   Flash Flash  
        FORMAT x3 words            RAM   Flash Flash  
        SCRIPTS                    RAM   Flash Flash  
        LINKED-LIST                RAM   RAM   Flash   RAM allows SWC to be desactivated
        ARC descriptors 4 words    RAM   RAM   RAM    
        Debug registers, Buffers   RAM   RAM   RAM    
    */
    switch (RAMsplit)
    {  
        uint32_t offsetWords;
    /*
        Graph is already in RAM, No data move, initialize the instance 
    */
    default:
    case COPY_CONF_GRAPH0_COPY_ALL_IN_RAM:
    case COPY_CONF_GRAPH0_ALREADY_IN_RAM:

        offsetWords = GRAPH_HEADER_NBWORDS;
        S->pio = &(graph_dst[offsetWords - STREAM_IOFMT_SIZE_W32]);   /* pio starts with fw_io_idx = 1, index 0 is unused */

        offsetWords += (RD(graph0[1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32);
        S->all_formats = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32);
        S->script_offsets = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[1], SCRIPTS_SIZE_GR1));
        S->linked_list = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[2], LINKEDLIST_SIZE_GR2));
        S->all_arcs = &(graph_dst[offsetWords]);
        break;

    case COPY_CONF_GRAPH0_FROM_LINKEDLIST:  /* option 2 */
    case COPY_CONF_GRAPH0_FROM_ARC_DESCS:   /* option 3 */

        offsetWords = GRAPH_HEADER_NBWORDS;
        S->pio = &(graph0[offsetWords - STREAM_IOFMT_SIZE_W32]);   /* pio starts with fw_io_idx = 1, index 0 is unused */

        offsetWords += (RD(graph0[1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32);
        S->all_formats = &(graph0[offsetWords]);

        offsetWords += (RD(graph0[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32);
        S->script_offsets = &(graph0[offsetWords]);

        S->graph = graph0;                  /* graph base address is in Flash */
        graph_dst = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,G);

        if (RAMsplit == COPY_CONF_GRAPH0_FROM_LINKEDLIST)   /* 1 */
        {   
            offsetWords += (RD(graph0[1], SCRIPTS_SIZE_GR1));
            graph_src = &(graph0[offsetWords]);     /* linked list in Flash copied in RAM */
            S->linked_list = graph_dst;             /*  and used by instance in RAM */
            graph_words_in_ram -= offsetWords;      /* copy from linkedList */

            offsetWords = (RD(graph0[2], LINKEDLIST_SIZE_GR2)); /* offset not incremented */
            S->all_arcs = &(graph_dst[offsetWords]);        
        }
        else /* COPY_CONF_GRAPH0_FROM_ARC_DESCS */          /* 2 */
        {   offsetWords += (RD(graph0[1], SCRIPTS_SIZE_GR1));
            S->linked_list = &(graph0[offsetWords]);/*  */

            offsetWords += (RD(graph0[2], LINKEDLIST_SIZE_GR2));
            graph_src = &(graph0[offsetWords]);     /* linked list in Flash */
            S->all_arcs = graph_dst;                /* arc in RAM */
            graph_words_in_ram -= offsetWords;      /* copy from arcs */
        }
        break;
    }

    /* finalize the copy by telling the other processors */   
    if (RD(S->scheduler_control, MAININST_SCTRL)) 
    {   
        if (RD(S->scheduler_control, MAININST_SCTRL)) 
        {   uint32_t i; 
            for (i=0;i<graph_words_in_ram;i++)
            {   graph_dst[i]=graph_src[i];
            }
        }    
        /* declare the graph memory as shared */
        if (RD(graph0[0],SHAREDRAM_GRAPH0))
        {   platform_al (PLATFORM_MP_GRAPH_SHARED, 
                (uint8_t *)graph_dst,   /* shared RAM range */
                (uint8_t *)&(graph_dst[graph_words_in_ram]),0);
        }

        /* all other process can be released from wait state */
        platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); 
    }
    else
    {   /* wait until the graph is copied in RAM */
        uint8_t wait; 
        do {
            platform_al (PLATFORM_MP_BOOT_WAIT, &wait, 0,0);
        } while (0u != wait);
    }

    /* if the application sets the debug option then don't use the one from the graph */
    if (STREAM_SCHD_NO_SCRIPT == RD(S->scheduler_control, SCRIPT_SCTRL))
    {   uint32_t debug_script_option;
        debug_script_option = RD(graph0[3], SCRIPT_SCTRL_GR3);
        ST(S->scheduler_control, SCRIPT_SCTRL, debug_script_option);
    }

    /* initialize local static */
    {
        struct HW_params params;
        platform_al (PLATFORM_PROC_HW, (uint8_t *)&params, 0, 0);
        ST (S->whoami_ports, PROCID_PARCH, params.procID);
        ST (S->whoami_ports, ARCHID_PARCH, params.archID);
        ST (S->whoami_ports, INSTANCE_PARCH, RD(S->scheduler_control, INSTANCE_SCTRL));
        S->iomask = params.iomask;
        S->ioctrl = params.ioctrl;
    }

    S->parameters = 0;
    ST (S->parameters, INSTANCE_ON_PARINST, 1);

    /* initialize the pointer to set/start/stop of IOs */
    platform_al (PLATFORM_IO_STRUCT, (uint8_t *)(&(S->platform_io)), 0, 0);

    arm_stream_services(STREAM_SERVICE_INTERNAL_RESET, (uint8_t *)&(S), 0, 0, 0); 
}


/**
  @brief        Initialization and start of the IOs 
  @param[in]    instance   global data of this instance
  @return       none

  @par          Read the IOs descriptor the instance is in charge of
                Interface to platform-specific stream controls (set, start)
  @remark
 */
static void stream_init_io(arm_stream_instance_t *S)
{
    uint32_t nio;
    uint8_t fw_io_idx;
    uint32_t *pio;
    uint32_t stream_format_io;
    uint32_t stream_format_io_setting;
    uint32_t io_mask;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    io_function_control_ptr io_func;
    struct platform_io_control *io_manifest;

    /*-------------------------------------------*/

    /* if cold start : clear the backup area */
    if (TEST_BIT(S->scheduler_control, BOOT_SCTRL_LSB) == STREAM_COLD_BOOT)
    {   platform_al (PLATFORM_CLEAR_BACKUP_MEM, 0,0,0);
    }

    platform_al (PLATFORM_MP_RESET_DONE, 0,0, RD(S->scheduler_control, INSTANCE_SCTRL));

    /* wait all the process have initialized the graph */
    {   uint8_t wait; 
        do {
            platform_al (PLATFORM_MP_RESET_WAIT, &wait, 0, RD(S->scheduler_control, NBINSTAN_SCTRL));
        } while (wait == 0);
    }

    /*-------------------------------------------*/
    /* 
        initialization of the graph IO ports 
    */     
    io_mask = S->iomask;
    pio = S->pio;
    nio = RD(S->graph[1],NB_IOS_GR1);
    all_arcs = S->all_arcs;


    for (fw_io_idx = 1; fw_io_idx < nio; fw_io_idx++)
    {
        uint8_t *address;
        uint32_t size;
        extern arm_stream_instance_t * platform_io_callback_parameter[MAX_IO_FUNCTION_PLATFORM];

        /* does this port is managed by the Graph interpreter instance ? */
        if (0 == (io_mask & (1U << fw_io_idx))) 
            continue; 

        /* no init for the IO  dedicated to control */
        if (0 == fw_io_idx)
            continue;

        stream_format_io = pio[fw_io_idx*STREAM_IOFMT_SIZE_W32];

        /* no init for the unset IO */
        if (0xFFFFFFFFL == stream_format_io)
            continue;

        /* default value settings */
        stream_format_io_setting = pio[1 + fw_io_idx*STREAM_IOFMT_SIZE_W32];

        /* IO-Interface can define its buffer on its own
           */
        if (0 != TEST_BIT(stream_format_io, SET0COPY1_IOFMT_LSB))
        {
            iarc = RD(stream_format_io, IOARCID_IOFMT);
            iarc = SIZEOF_ARCDESC_W32 * iarc;
            address = (uint8_t *)pack2linaddr_ptr(S->long_offset, all_arcs[iarc + BUF_PTR_ARCW0]);
            size = (uint32_t)RD(all_arcs[iarc + BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
        }
        else
        {   address = 0;
            size = 0;
        }

        /* the Graph interpreter instance holds the memory offsets */
        platform_io_callback_parameter [fw_io_idx] = S;

        io_manifest = &(S->platform_io[fw_io_idx]);
        io_func = io_manifest->io_set;
        (*io_func)(&stream_format_io_setting, address, size);
    } 
}

/*--------------------------------------------------------------------------- */

SECTION_STOP 
#ifdef __cplusplus

}

#endif

