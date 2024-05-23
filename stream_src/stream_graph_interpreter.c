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


static void stream_copy_graph(arm_stream_instance_t *stream_instance);
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
void arm_graph_interpreter (uint32_t command,  arm_stream_instance_t *S, uint8_t *data, uint32_t size)
{   
	switch (RD(command, COMMAND_CMD))
    {
        /* usage: arm_stream(STREAM_RESET, &instance,graph_input, 0); */
	    case STREAM_RESET: 
	    {   stream_copy_graph (S);
            /* @@@ TODO  check scheduler_control S->scheduler_control .BOOT_SCTRL to clear 
                the backup memory area STREAM_COLD_BOOT / STREAM_WARM_BOOT
             */
            stream_scan_graph (S, STREAM_RESET, 0);
            stream_init_io (S);

            break;
        }

        /* usage: arm_stream(STREAM_RUN, &instance,0, 0); */
	    case STREAM_RUN:   
	    {   if (RD(S->parameters, INSTANCE_ON_PARINST))
            {   stream_scan_graph (S, STREAM_RUN, 0);
            }
            break;
        }   

        /* change the use-case  : 
            usage: arm_graph_interpreter (STREAM_SET_PARAMETER, &instance, &use-case, 0); */
        case STREAM_SET_PARAMETER:
	    {   /* @@@ calls the main script with SET_PARAMETER */
            /* update parameter of a specific nanoAppRT 
                usage: arm_graph_interpreter (STREAM_SET_NANOAPPRT_PARAMETER, &instance, &parameters, 0); */
	        /* @@@ calls specific nanoAppRT with SET_PARAMETER */
            stream_scan_graph (S, STREAM_SET_PARAMETER, data);
            break;
        }


        /* usage: arm_graph_interpreter (STREAM_STOP, &instance, 0, 0); */
        case STREAM_STOP:
	    {   stream_scan_graph (S, STREAM_STOP, 0);
            break;
        }

        /* manual data push/pop : read information (available free space, or data amount to read)
            arm_graph_interpreter(STREAM_FIFO_STATUS_PACK(io_platform_data_in_0), instance, *fifo size, 0) 
            */
        case STREAM_FIFO_STATUS: 
        {   extern void check_graph_boundaries(arm_stream_instance_t *S);
            check_graph_boundaries(S);
            *(uint32_t *)data = RD(S->ioctrl[STREAM_DATA_START_UNPACK_FIFO(command)], ONGOING_IOSIZE);
            break;
        }

        /* usage: arm_graph_interpreter(STREAM_DATA_START_PACK(FIFO_ID), instance, *data, size) */
        case STREAM_DATA_START: 
        {   arm_graph_interpreter_io_ack (STREAM_DATA_START_UNPACK_FIFO(command), data, size);
            break;
        }

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
static void stream_copy_graph(arm_stream_instance_t *S)
{
    uint8_t RAMsplit;
    uint32_t graph_words_in_ram, graph_size_word32;
    uint32_t *graph_src, *graph0;
    uint32_t *graph_dst;

    /* 
        start copying the graph in RAM 
        initialize the current Graph interpreter instance (->node_entry_points, ->graph)
        initialize the IO boundaries of the graph
     realign "graph0" */
    graph_size_word32 = S->graph[-1];
    graph0 = S->graph;

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
        S->pio = &(graph_dst[offsetWords]);   /* pio / fw_io_idx */

        offsetWords += (RD(graph0[1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32);
        S->all_formats = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32);
        S->script_offsets = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[1], SCRIPTS_SIZE_GR1));
        S->linked_list = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[2], LINKEDLIST_SIZE_GR2));
        S->all_arcs = &(graph_dst[offsetWords]);
        break;

    //case COPY_CONF_GRAPH0_FROM_LINKEDLIST:  /* option 2 */
    case COPY_CONF_GRAPH0_FROM_ARC_DESCS:   /* option 2 */

        offsetWords = GRAPH_HEADER_NBWORDS;
        S->pio = &(graph0[offsetWords]); 

        offsetWords += (RD(graph0[1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32);
        S->all_formats = &(graph0[offsetWords]);

        offsetWords += (RD(graph0[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32);
        S->script_offsets = &(graph0[offsetWords]);

        S->graph = graph0;                  /* graph base address is in Flash */
        graph_dst = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,G);

        //if (RAMsplit == COPY_CONF_GRAPH0_FROM_LINKEDLIST)   /* 1 */
        //{   
        //    offsetWords += (RD(graph0[1], SCRIPTS_SIZE_GR1));
        //    graph_src = &(graph0[offsetWords]);     /* linked list in Flash copied in RAM */
        //    S->linked_list = graph_dst;             /*  and used by instance in RAM */
        //    graph_words_in_ram -= offsetWords;      /* copy from linkedList */

        //    offsetWords = (RD(graph0[2], LINKEDLIST_SIZE_GR2)); /* offset not incremented */
        //    S->all_arcs = &(graph_dst[offsetWords]);        
        //}
        //else /* COPY_CONF_GRAPH0_FROM_ARC_DESCS */          /* 2 */
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
    //if (RD(S->scheduler_control, MAININST_SCTRL)) 
    //{   
    //    if (RD(S->scheduler_control, MAININST_SCTRL)) 
        {   uint32_t i; 
            for (i=0;i<graph_words_in_ram;i++)
            {   graph_dst[i]=graph_src[i];
            }
        }    
    //    /* declare the graph memory as shared */
    //    if (RD(graph0[0],SHAREDRAM_GRAPH0))
    //    {   platform_al (PLATFORM_MP_GRAPH_SHARED, 
    //            (uint8_t *)graph_dst,   /* shared RAM range */
    //            (uint8_t *)&(graph_dst[graph_words_in_ram]),0);
    //    }

    //    /* all other process can be released from wait state */
    //    platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); 
    //}
    //else
    //{   /* wait until the graph is copied in RAM */
    //    uint8_t wait; 
    //    do {
    //        platform_al (PLATFORM_MP_BOOT_WAIT, &wait, 0,0);
    //    } while (0u != wait);
    //}


    /* if the application sets the debug option then don't use the one from the graph
        and the application sets the scheduler return option in init_stream_instance() 
     */
    if (STREAM_SCHD_NO_SCRIPT == RD(S->scheduler_control, SCRIPT_SCTRL))
    {   uint32_t debug_script_option;
        debug_script_option = RD(graph0[3], SCRIPT_SCTRL_GR3);
        ST(S->scheduler_control, SCRIPT_SCTRL, debug_script_option);
    }

    /* does the graph is deciding the return-from-scheduler option */
    if (STREAM_SCHD_RET_NO_ACTION == RD(S->scheduler_control, RETURN_SCTRL))
    {   uint32_t return_script_option;
        return_script_option = RD(graph0[3], RETURN_SCTRL_GR3);

        /* the return option is undefined : return when no more data is available */
        if (return_script_option == 0) 
        {   return_script_option = STREAM_SCHD_RET_END_SWC_NODATA;
        }
        ST(S->scheduler_control, RETURN_SCTRL, return_script_option);
    }

    S->linked_list_ptr = S->linked_list;

    S->parameters = 0;
    ST (S->parameters, INSTANCE_ON_PARINST, 1);

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
    uint32_t *pio, *pio_base;
    uint32_t stream_format_io_setting;
    uint32_t io_mask;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    const p_io_function_ctrl *io_func;
    const p_stream_al_services *al_func;

    extern arm_stream_instance_t * platform_io_callback_parameter;
    /*-------------------------------------------*/

    /* if cold start : clear the backup area */
    if (TEST_BIT(S->scheduler_control, BOOT_SCTRL_LSB) == STREAM_COLD_BOOT)
    {   al_func = &(S->al_services[0]);
        (*al_func)(PACK_AL_SERVICE(0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
    }

    /* wait all the process have initialized the graph */
    {   //uint8_t wait; 
        //do {
        //    platform_al (PLATFORM_MP_RESET_WAIT, &wait, 0, RD(S->scheduler_control, NBINSTAN_SCTRL));
        //} while (wait == 0);
    }

    /*-------------------------------------------*/
    /* 
        initialization of the graph IO ports 
    */     
    io_mask = S->iomask;
    pio_base = S->pio;
    nio = RD(S->graph[1],NB_IOS_GR1);
    all_arcs = S->all_arcs;


    /* the Graph interpreter instance holds the memory pointers */
    platform_io_callback_parameter = S;

    for (fw_io_idx = 0; fw_io_idx < nio; fw_io_idx++)
    {
        uint8_t *address;
        uint32_t size;

        pio = &(pio_base[fw_io_idx * STREAM_IOFMT_SIZE_W32]);

        /* does this port is managed by the Graph interpreter instance ? */
        if (0 == (io_mask & (1U << fw_io_idx))) 
            continue; 

        /* no init for the unset IO */
        if (0 != TEST_BIT(*pio, QUICKSKIP_IOFMT_LSB))
            continue;

        /* default value settings */
        stream_format_io_setting = pio[1];
        io_func = &(S->platform_io[fw_io_idx]);
        if (*io_func == 0) 
        {   continue;
        }
        (*io_func)(STREAM_RESET, (uint8_t *)&stream_format_io_setting, 0);

        /* 
            IO-Interface expects the buffer to be declared by the graph 
        */
        if (0 == TEST_BIT(*pio, SHAREBUFF_IOFMT_LSB))
        {
            iarc = RD(*pio, IOARCID_IOFMT);
            iarc = SIZEOF_ARCDESC_W32 * iarc;
            address = (uint8_t *)pack2linaddr_ptr(S->long_offset, all_arcs[iarc + BUF_PTR_ARCW0]);
            size = (uint32_t)RD(all_arcs[iarc + BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);

            io_func = &(S->platform_io[fw_io_idx]);
            (*io_func)(STREAM_SET_BUFFER, address, size);
        }
    } 
}

/*--------------------------------------------------------------------------- */

SECTION_STOP 
#ifdef __cplusplus

}

#endif

