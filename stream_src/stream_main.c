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

#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  
#include <string.h>         /* memcpy */


static void stream_copy_graph(arm_stream_instance_t *stream_instance, uint32_t *graph);
static void stream_init_io(arm_stream_instance_t *stream_instance);

#define L stream_instance->long_offset
#define G stream_instance->graph

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
  @brief            Stream entry point 
  @param[in]        command     operation to do (reset, run, stop)
  @param[in]        instance    Stream instance pointer
  @param[in]        data        graph to process
  @return           none

  @par              reset, run and stop a graph
                      
  @remark
 */
void arm_stream (uint32_t command,  arm_stream_instance_t *stream_instance, uint32_t *data)
{   
	switch (command)
    {
        /* usage: arm_stream(STREAM_RESET, &instance,graph_input); */
	    case STREAM_RESET: 
	    {   stream_copy_graph (stream_instance, data);
            stream_instance->linked_list_ptr = stream_instance->linked_list;

            stream_scan_graph (stream_instance, 1);

            /* skip the first component = debug script node */
            #define XX 8 // size of the SWC of the debug trace script
            #define YY 3 // offset to the instance of the script

            stream_instance->main_script = (uint32_t *)PACK2LINADDR(stream_instance->long_offset, stream_instance->linked_list[YY]);
                        stream_instance->linked_list = &(stream_instance->linked_list[XX]);
            stream_instance->linked_list_ptr = stream_instance->linked_list;

            stream_init_io (stream_instance);
            break;

        }

        /* usage: arm_stream(STREAM_RUN, &instance,0); */
	    case STREAM_RUN:   
	    {   if (RD(stream_instance->parameters, INSTANCE_ON_PARINST))
            {   stream_scan_graph (stream_instance, 0);
            }
            break;
        }   

        /* usage: arm_stream (STREAM_STOP, &instance, 0); */
        case STREAM_STOP:
	    {   stream_scan_graph (stream_instance, -1);
            break;
        }

        /* arm_stream (STREAM_INTERPRET_COMMANDS, byte stream, 0, 0)*/
        case STREAM_INTERPRET_COMMANDS: 
        {   /* byte stream format : command + length, byte-stream. Use-case : 
                node set/read parameters 
                I/O save/restore state, disable/enable, change settings
                ARCs : check content, read/write => "PLATFORM_COMMAND_OUT"
             */
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
static intPtr_t pack2linaddr_int(intPtr_t *long_offset, uint32_t data)
{
    return PACK2LINADDR(long_offset,data);
}

static uint8_t * pack2linaddr_ptr(intPtr_t *long_offset, uint32_t data)
{   return (uint8_t *) (pack2linaddr_int(long_offset, data));
}

/**
  @brief        Graph copy in RAM (STREAM_RESET phase)
  @param[in]    instance   global data of this instance
  @param[in]    graph      input graph
  @return       none

  @par          Initialization of the Stream instance 
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
static void stream_copy_graph(arm_stream_instance_t *stream_instance, uint32_t *graph0)
{
    uint32_t *graph_src;
    uint32_t *graph_dst; 
    uint32_t graph_words_in_ram;
    uint8_t RAMsplit;
    uint32_t offsetWords; //, *offset1, *offset2, *offset3;

    /* 
        start copying the graph in RAM 
        initialize the current Stream instance (->node_entry_points, ->graph)
        initialize the IO boundaries of the graph
    */

    /* read the processors' long_offset table */
    platform_al (PLATFORM_OFFSETS, (uint8_t *)&(stream_instance->long_offset),0,0);
    
    /* default assignments in flash */
    stream_instance->graph = &(graph0[1]);;
    graph_src = graph_dst = &(graph0[1]);
    RAMsplit = RD((graph0[1]),RAMSPLIT_GRAPH0);

    switch (RAMsplit)
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

        graph_src = &(graph0[1]);
        graph_dst = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,G);
        graph_words_in_ram = graph0[0];

        if (RD(stream_instance->scheduler_control, MAININST_SCTRL)) 
        {   uint32_t i; 
            for (i=0;i<graph_words_in_ram;i++)
            {   graph_dst[i]=graph_src[i];
            }
        }
        stream_instance->graph = graph_dst;
        break;

    /*
       Graph is read from Flash and a small part is copied in RAM
       to save RAM (for example Cortex-M0 with 2kB of internal SRAM)

        graph_src = parameter const uint32_t *graph  +  graph[0].OFFSET_GRAPH0
        graph_dst = pack2linaddr(parameter *graph[2] RAM)
        graph_size= graph[0].LENGTHRAM_GRAPH0
        parameters->graph = graph_src;

                                   2 options :
        IO 2 words                 Flash  Flash
        FORMAT 3 words             Flash  Flash
        SCRIPTS                    Flash  Flash
        LINKED-LIST                RAM    Flash
        STREAM INSTANCE 3 words    RAM    RAM
        ARC descriptors 4 words    RAM    RAM
        Debug registers, Buffers   RAM    RAM
    */
    case COPY_CONF_GRAPH0_FROM_LINKEDLIST:
    case COPY_CONF_GRAPH0_FROM_STREAM_INST:

    // @@@@ TO BETTER COMMENT ... 

        offsetWords = 
            RD(graph0[1+1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32 +
            RD(graph0[1+1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32 +
            RD(graph0[1+1], SCRIPTS_SIZE_GR1);

        graph_words_in_ram = 
            RD(graph0[1+2], LINKEDLIST_SIZE_GR2) +
            RD(graph0[1+2], NB_ST_INSTANCE_GR2) * STREAM_INSTANCE_SIZE +
            RD(graph0[1+3], NB_ARCS_GR3) * SIZEOF_ARCDESC_W32 +
            RD(graph0[1+3], DEBUGREG_SIZE_GR3);

        if (RAMsplit == COPY_CONF_GRAPH0_FROM_STREAM_INST)
        {   offsetWords += RD(graph0[1+2], LINKEDLIST_SIZE_GR2);
            graph_words_in_ram -= RD(graph0[1+2], LINKEDLIST_SIZE_GR2);
        }

        graph_src = (&(graph0[1+ offsetWords]));
        graph_dst = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,G);

        if (RD(stream_instance->scheduler_control, MAININST_SCTRL)) 
        {   uint32_t i; 
            for (i=0;i<graph_words_in_ram;i++)
            {   graph_dst[i]=graph_src[i];
            }
        }
        stream_instance->graph = &(graph0[1]);
        break;

    /*
        Graph is already in RAM, No data move
    */
    default:
    case COPY_CONF_GRAPH0_ALREADY_IN_RAM:
        break;
    }

    /* finalize the copy */   
    if (RD(stream_instance->scheduler_control, MAININST_SCTRL)) 
    {   /* declare the graph memory as shared */
        if (RD(graph0[1],SHAREDRAM_GRAPH0))
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

    /*   local stream instance initialization 
                                   3 options :
        IO 2 words                 RAM  Flash  Flash Offset1
        FORMAT 3 words             RAM  Flash  Flash 
        SCRIPTS                    RAM  Flash  Flash 
        LINKED-LIST                RAM  RAM    Flash Offset2
        STREAM INSTANCE 3 words    RAM  RAM    RAM   Offset3
        ARC descriptors 4 words    RAM  RAM    RAM   
        Debug registers, Buffers   RAM  RAM    RAM   
    */
    { uint32_t *graph = stream_instance->graph;
    offsetWords = GRAPH_HEADER_NBWORDS;
    stream_instance->pio = &(graph[offsetWords]);

    offsetWords += RD(graph[1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32;
    stream_instance->all_formats = &(graph[offsetWords]);

    offsetWords += RD(graph[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32;
    stream_instance->main_script = &(graph[offsetWords]);

    offsetWords += RD(graph[1], SCRIPTS_SIZE_GR1);
    stream_instance->linked_list = &(graph[offsetWords]);

    offsetWords += RD(graph[2], LINKEDLIST_SIZE_GR2) + 1; /* add 1 for GRAPH_LAST_WORD */
    //stream_instance->pinst = &(graph[offsetWords]);

    offsetWords += RD(graph[2], NB_ST_INSTANCE_GR2) * STREAM_INSTANCE_SIZE;
    stream_instance->all_arcs = &(graph[offsetWords]);

    /* if the application sets the debug option then don't use the one from the graph */
    if (STREAM_SCHD_NO_SCRIPT == RD(stream_instance->scheduler_control, SCRIPT_SCTRL))
    {   uint32_t debug_script_option;
        debug_script_option = RD(graph0[1 + 3], SCRIPT_SCTRL_GR3);
        ST(stream_instance->scheduler_control, SCRIPT_SCTRL, debug_script_option);
    }

    /* initialize local static */
    //@@@@ initialize stream_instance->whoami_ports .. 

    arm_stream_services(STREAM_SERVICE_INTERNAL_RESET, (uint8_t *)&(stream_instance), 0, 0, 0); 
    }
}


/**
  @brief        Initialization and start of the IOs 
  @param[in]    instance   global data of this instance
  @return       none

  @par          Read the IOs descriptor the instance is in charge of
                Interface to platform-specific stream controls (set, start)
  @remark
 */
static void stream_init_io(arm_stream_instance_t *stream_instance)
{
    uint32_t nio;
    uint32_t iio; 
    uint32_t fw_io_idx;
    uint32_t *pio;
    uint32_t stream_format_io;
    uint32_t stream_format_io_setting;
    struct platform_control_stream set_stream;
    uint32_t io_mask;
    uint32_t iarc; 
    uint32_t *all_arcs; 


    /*-------------------------------------------*/

    /* if cold start : clear the backup area */
    if (TEST_BIT(stream_instance->scheduler_control, BOOT_SCTRL_LSB) == STREAM_COLD_BOOT)
    {   platform_al (PLATFORM_CLEAR_BACKUP_MEM, 0,0,0);
    }

    platform_al (PLATFORM_MP_RESET_DONE, 0,0, RD(stream_instance->scheduler_control, INSTANCE_SCTRL));

    /* wait all the process have initialized the graph */
    {   uint8_t wait; 
        do {
            platform_al (PLATFORM_MP_RESET_WAIT, &wait, 0, RD(stream_instance->scheduler_control, NBINSTAN_SCTRL));
        } while (wait);
    }

    /*-------------------------------------------*/
    /* 
        initialization of the graph IO ports 
    */     
    io_mask = stream_instance->iomask;
    pio = stream_instance->pio;
    nio = RD(stream_instance->graph[1],NB_IOS_GR1);
    all_arcs = stream_instance->all_arcs;


    for (iio = 0; iio < nio; iio++)
    {
        stream_format_io = pio[0 + iio*STREAM_IOFMT_SIZE_W32];
        stream_format_io_setting = pio[1 + iio*STREAM_IOFMT_SIZE_W32];

        /* does this port is managed by the Stream instance ? */
        if (0 == (io_mask & (1U << iio))) 
            continue; 

        fw_io_idx = RD(stream_format_io, FW_IO_IDX_IOFMT);
        set_stream.fw_io_idx = fw_io_idx;

        /* default value settings */
        set_stream.domain_settings = stream_format_io_setting;

        /* the platform manifest tell if IO needs an allocated memory buffers, 
            in this case the arc descriptor is not associated to a buffer
           */
        if (IO_COMMAND_SET_BUFFER == TEST_BIT(stream_format_io, IOCOMMAND_IOFMT_LSB))
        {
            iarc = SIZEOF_ARCDESC_W32 * RD(stream_format_io, IOARCID_IOFMT);
            set_stream.buffer.address = 
                pack2linaddr_int(stream_instance->long_offset, all_arcs[iarc + BUF_PTR_ARCW0]);
            set_stream.buffer.size = 
                (uint32_t)RD(all_arcs[iarc + BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
        }
        else
        {   set_stream.buffer.address = 0;
            set_stream.buffer.size = 0;
        }
        /* the Stream instance holds the memory offsets */
        set_stream.instance = stream_instance;
        platform_al (PLATFORM_IO_SET_STREAM, (uint8_t *)&set_stream, 0, 0);
    }
}

/*--------------------------------------------------------------------------- */
 
#ifdef __cplusplus
}
#endif
 