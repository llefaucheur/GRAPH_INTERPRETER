/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        platform_computer.c
 * Description:  platform-specific declarations (memory, nodes)
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
#include "platform.h"

#ifdef PLATFORM_COMPUTER

#ifdef __cplusplus
 extern "C" {
#endif
/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 1

//#define _CRT_SECURE_NO_DEPRECATE 1
#if DATA_FROM_FILES
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      
#include "stream_types.h"


extern void platform_init_copy_graph(arm_stream_instance_t *S, uint32_t **graph_dst);
extern void platform_init_io(arm_stream_instance_t *S);

extern const uint8_t platform_audio_out_bit_fields[];

#define L S->long_offset
#define G S->graph

/* 
    this table will be extended with pointers to nodes loaded 
    dynamically and compiled with position independent execution options

    it is aligned with the list of SOFTWARE COMPONENTS MANIFESTS in "files_manifests_computer.txt"
*/
/* -----------------------------------
    Full node descriptions given in ./stream_tools/TEMPLATE_GRAPH.txt
*/


extern p_stream_node arm_stream_null_task;       /*  0 */
extern p_stream_node arm_stream_script;          /*  1  #define arm_stream_script_index 1 */
extern p_stream_node arm_stream_router;          /*  2 */
extern p_stream_node arm_stream_amplifier;       /*  3 */
extern p_stream_node arm_stream_filter;          /*  4 */
extern p_stream_node arm_stream_modulator;       /*  5 */
extern p_stream_node arm_stream_demodulator;     /*  6 */
extern p_stream_node arm_stream_filter2D;        /*  7 */
extern p_stream_node sigp_stream_detector;       /*  8 */
extern p_stream_node sigp_stream_detector2D;     /*  9 */
extern p_stream_node sigp_stream_resampler;      /* 10 */
extern p_stream_node sigp_stream_compressor;     /* 11 */
extern p_stream_node sigp_stream_decompressor;   /* 12 */
extern p_stream_node bitbank_JPEGENC;            /* 13 */
extern p_stream_node TjpgDec;                    /* 14 */

#define TBD 0

const p_stream_node node_entry_points[NB_NODE_ENTRY_POINTS] =
{
    /*  0 */ (p_stream_node)&arm_stream_null_task,      /* node disabled */
    /*  1 */ (p_stream_node)&arm_stream_script,         /* byte-code interpreter, index "arm_stream_script_INDEX" */
    /*  2 */ (p_stream_node)&arm_stream_router,         /* copy input arcs and subchannel and output arcs and subchannels   */     
    /*  3 */ (p_stream_node)&arm_stream_amplifier,      /* amplifier mute and un-mute with ramp and delay control */
    /*  4 */ (p_stream_node)&arm_stream_filter,         /* cascade of DF1 filters */
    /*  5 */ (p_stream_node)&arm_stream_modulator,      /* signal generator with modulation */
    /*  6 */ (p_stream_node)&arm_stream_demodulator,    /* signal demodulator, frequency estimator */
    /*  7 */ (p_stream_node)&arm_stream_filter2D,       /* Filter, rescale, rotate, exposure compensation */
    /*  8 */ (p_stream_node)&sigp_stream_detector,      /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
    /*  9 */ (p_stream_node)&sigp_stream_detector2D,    /* activity detection, pattern detection */
    /* 10 */ (p_stream_node)&sigp_stream_resampler,     /* asynchronous sample-rate converter */
    /* 11 */ (p_stream_node)&sigp_stream_compressor,    /* raw data compression with adaptive prediction */
    /* 12 */ (p_stream_node)&sigp_stream_decompressor,  /* raw data decompression */
    /* 13 */ (p_stream_node)&bitbank_JPEGENC,           /* bitbank_JPEGENC */
    /* 14 */ (p_stream_node)&TjpgDec,                   /* TjpgDec */
};    


/*
    ==============
    Base addresses 
    ==============
        1 1 9 4 number of architectures, number of processors, number of memory banks, number of offsets
 */

uint8_t MEXT[SIZE_MBANK_DMEM_EXT];
uint8_t BACKUPMEM1[SIZE_MBANK_BACKUPMEM1]; 
uint8_t TCM2[SIZE_MBANK_TCM2]; 
uint8_t FLASH3[SIZE_MBANK_FLASH3]; 

const uint8_t * long_offset[MAX_NB_MEMORY_OFFSET] = 
{   &(MEXT[0]),
    &(BACKUPMEM1[0]),
    &(TCM2[0]),
    &(FLASH3[0]),
};

/* 
    translation of physical indexes known by the AL to index to the graph (pio_control with RX0/TX1, ARC, ..)
    usage : 
        arm_stream_io_ack (platform_io_al_idx_to_stream[IO_PLATFORM_ANALOG_SENSOR_0], (uint8_t *)data, size);
*/

uint8_t platform_io_al_idx_to_stream[LAST_IO_FUNCTION_PLATFORM]; 

arm_stream_instance_t * platform_io_callback_parameter;

extern p_io_function_ctrl data_in_0;      
extern p_io_function_ctrl data_in_1;      
extern p_io_function_ctrl analog_sensor_0;
extern p_io_function_ctrl motion_in_0;    
extern p_io_function_ctrl audio_in_0;     
extern p_io_function_ctrl d2_in_0;        
extern p_io_function_ctrl line_out_0;     
extern p_io_function_ctrl gpio_out_0;     
extern p_io_function_ctrl gpio_out_1;     
extern p_io_function_ctrl data_out_0;     

/*
    files_manifests_computer.txt :

    ;Path      Manifest         IO_AL_idx ProcCtrl clock-domain     definition          (code from platform_computer.h)
    1   io_platform_data_in_0.txt       0     1        0            application processor  IO_PLATFORM_DATA_IN_0      
    1   io_platform_data_in_1.txt       1     1        0            application processor  IO_PLATFORM_DATA_IN_1      
    1   io_platform_analog_sensor_0.txt 2     1        0            ADC                    IO_PLATFORM_ANALOG_SENSOR_0
    1   io_platform_motion_in_0.txt     3     1        0            accelero=gyro          IO_PLATFORM_MOTION_IN_0    
    1   io_platform_audio_in_0.txt      4     1        0            microphone             IO_PLATFORM_AUDIO_IN_0     
    1   io_platform_2d_in_0.txt         5     1        0            camera                 IO_PLATFORM_2D_IN_0        
    1   io_platform_line_out_0.txt      6     1        0            audio out stereo       IO_PLATFORM_LINE_OUT_0     
    1   io_platform_gpio_out_0.txt      7     1        0            GPIO/LED               IO_PLATFORM_GPIO_OUT_0     
    1   io_platform_gpio_out_1.txt      8     1        0            GPIO/PWM               IO_PLATFORM_GPIO_OUT_1     
    1   io_platform_data_out_0.txt      9     1        0            application processor  IO_PLATFORM_DATA_OUT_0  

*/
const p_io_function_ctrl platform_io [LAST_IO_FUNCTION_PLATFORM] =
{
    (p_io_function_ctrl)&data_in_0,        // 0  computer/platform_manifest/io_platform_data_in_0.txt      
    (p_io_function_ctrl)&data_in_1,        // 1  computer/platform_manifest/io_platform_data_in_1.txt      
    (p_io_function_ctrl)&analog_sensor_0,  // 2  computer/platform_manifest/io_platform_analog_sensor_0.txt
    (p_io_function_ctrl)&motion_in_0,      // 3  computer/platform_manifest/io_platform_motion_in_0.txt    
    (p_io_function_ctrl)&audio_in_0,       // 4  computer/platform_manifest/io_platform_audio_in_0.txt     
    (p_io_function_ctrl)&d2_in_0,          // 5  computer/platform_manifest/io_platform_d2_in_0.txt        
    (p_io_function_ctrl)&line_out_0,       // 6  computer/platform_manifest/io_platform_line_out_0.txt     
    (p_io_function_ctrl)&gpio_out_0,       // 7  computer/platform_manifest/io_platform_gpio_out_0.txt     
    (p_io_function_ctrl)&gpio_out_1,       // 8  computer/platform_manifest/io_platform_gpio_out_1.txt     
    (p_io_function_ctrl)&data_out_0,       // 9  computer/platform_manifest/io_platform_data_out_0.txt     
};


const uint32_t graph_computer_router[] = 
{ 
#include "graph_computer_router_bin.txt"
};




/*
 *  TIME (see stream_time64 definition)
 *
 *  "stream_time64" example of implementation using a global variable
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s]  140 Y + Q28 [s]
 *  systick increment for  1ms =  0x00041893 =  1ms x 2^28
 *  systick increment for 10ms =  0x0028F5C2 = 10ms x 2^28
 * 
 * Other implementation rely on an HW timer (RP2040)
*/

uint64_t global_stream_time64;


/*
    Callback for NODE and scripts (CALS  system call (0..7) and application callbacks (8..15))
        and ARC debug activities (ARC_APP_CALLBACK1) 
    Use-case : 
        deep-sleep proposal from the scheduler
        event detection trigger, software timers 
        metadata sharing from script
        allow execution of arm_graph_interpreter(STREAM_RESET..) after a graph remote reload (check IOs are ok)
*/
const p_stream_al_services application_callbacks[MAX_NB_APP_CALLBACKS] =
{   (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0
};


extern void arm_stream_services (uint32_t command, void *ptr1, void *ptr2, void *ptr3, uint32_t n);

//const p_stream_al_services p_arm_stream_services[1] =
//{   arm_stream_services
//};


//extern const p_stream_al_services al_services;
///* Forward declare al_services defined below. */
//stream_al_services al_services;
//const p_stream_al_services al_service[1] =
//{   (void *)&al_services
//};


uint32_t * get_graph_address(void)
{
    return (uint32_t *) graph_computer_router;
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
uint32_t platform_lin2pack (uint8_t *buffer)
{
    intptr_t distance;
    uint32_t ret;
    uint8_t i;

    /* packed address range is [ long_offset[IDX]  +/- 8MB ]*/
#define MAX_PACK_ADDR_RANGE (((1<<(SIZE_FMT0_MSB - SIZE_FMT0_LSB+1))-1))

    /* find the base offset */

    for (i = 0; i < (uint8_t)MAX_NB_MEMORY_OFFSET; i++)
    {
        distance = (intptr_t)buffer - (intptr_t)(long_offset[i]);
        if (ABS(distance) < MAX_PACK_ADDR_RANGE) 
        {   break;
        }
    }

    ret = 0;
    ST(ret, SIGNED_SIZE_FMT0, distance);
    ST(ret, DATAOFF_ARCW0, i);
    return ret;
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
intptr_t platform_pack2linaddr_int(uint32_t x)
{
    uint8_t *long_base;
    uint8_t *result8b;
    uint8_t extend;
    intptr_t result;
    int32_t signed_base;

    signed_base = RD(x, SIGNED_SIZE_FMT0);
    signed_base = signed_base << (32-SIGNED_SIZE_FMT0_MSB);
    signed_base = signed_base >> (32-SIGNED_SIZE_FMT0_MSB);
    extend = RD(x, EXTENSION_FMT0);
    signed_base <<= (extend << 1);

    result8b = (uint8_t *) &(long_offset[signed_base]);
    result = (intptr_t)result8b;
    return result;  
}

void * platform_pack2linaddr_ptr(uint32_t data)
{
    return (void *) (platform_pack2linaddr_int(data));
}

/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int

  @par              initialize a Graph interpreter  instance : share the address of the graph and
                    and the index of this instance within the application (other processors
                    or RTOS processes are using different index). 
  @remark
 */

void platform_init_stream_instance(arm_stream_instance_t *S)
{
#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 1

extern const p_stream_node node_entry_points[];

    uint32_t PIOoffsetWords, *graph_dst;
    uint32_t *graph_input;

    graph_input = get_graph_address();

    S->graph = (uint32_t *) &(graph_input[1]);      /* binary graph address loaded in the graph interpreter instance */ 
    S->long_offset = (uint8_t **) long_offset;      /* there is one single graph executed per platform */ 

    S->scheduler_control = PACK_STREAM_PARAM(
            STREAM_MAIN_INSTANCE,
            STREAM_NB_INSTANCE,
            STREAM_COLD_BOOT,
            STREAM_SCHD_NO_SCRIPT, 
            STREAM_SCHD_RET_END_ALL_PARSED
            );

    ST(S->whoami_ports, ARCHID_PARCH, ARCH_ID);  /* 3 fields used for NODE locking */
    ST(S->whoami_ports, PROCID_PARCH, PROC_ID);
    ST(S->whoami_ports, PRIORITY_PARCH, STREAM_INSTANCE_LOWLATENCYTASKS);
    ST(S->whoami_ports, NODE_W32OFF_PARCH, 0);   /* index in the linked list */
    S->al_services = arm_stream_services;
    S->application_callbacks = (p_stream_al_services)(&(application_callbacks[0]));
    S->node_entry_points = &(node_entry_points[0]);
    S->platform_io = platform_io;

    //{
    //    const p_stream_al_services *al_func;

    //    al_func = &(S->al_services);
    //    (*al_func)(PACK_SERVICE(0,0,NOTAG_SSRV, SERV_INTERNAL_PLATFORM_CLEAR_BACKUP_MEM, 0), 0,0,0,0);
    //}

    platform_init_copy_graph (S, &graph_dst);

    S->iomask = (1 << (RD((S->graph)[1], NB_IOS_GR1))) -1;      // IOs are enabled                                    

    PIOoffsetWords = GRAPH_HEADER_NBWORDS                       // Header
                   + (STREAM_IOFMT_SIZE_W32 * 
                      RD((S->graph)[1], NB_IOS_GR1));           // PIO settings
    S->script_offsets = &(S->graph[PIOoffsetWords]);

    PIOoffsetWords += RD((S->graph)[1], SCRIPTSSZW32_GR1);      // Scripts
    S->linked_list = &(S->graph[PIOoffsetWords]);
    S->linked_list_ptr = S->linked_list;

    PIOoffsetWords += RD((S->graph)[2], LINKEDLISTSZW32_GR2);   // linked list
    S->ongoing = (uint8_t *) &(graph_dst[PIOoffsetWords]);      //  on_going : from graph_dst[]

    PIOoffsetWords += RD((S->graph)[2], ARC_DEBUG_IDX_GR2) + ((3 + RD((S->graph)[1], NB_IOS_GR1))>>2);
    S->all_formats = &(graph_dst[PIOoffsetWords]);              // 

    PIOoffsetWords += RD((S->graph)[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32;
    S->all_arcs = &(graph_dst[PIOoffsetWords]);

    /* @@@ TODO  check scheduler_control S->scheduler_control .BOOT_SCTRL to clear 
        the backup memory area STREAM_COLD_BOOT / STREAM_WARM_BOOT
     */
    /* if the application sets the debug option then don't use the one from the graph
        and the application sets the scheduler return option in platform_init_stream_instance() 
     */
    if (STREAM_SCHD_NO_SCRIPT == RD(S->scheduler_control, SCRIPT_SCTRL))
    {   uint32_t debug_script_option;
        debug_script_option = RD((S->graph)[3], SCRIPT_SCTRL_GR3);
        ST(S->scheduler_control, SCRIPT_SCTRL, debug_script_option);
    }

    /* does the graph is deciding the return-from-scheduler option */
    if (STREAM_SCHD_RET_NO_ACTION == RD(S->scheduler_control, RETURN_SCTRL))
    {   uint32_t return_script_option;
        return_script_option = RD((S->graph)[3], RETURN_SCTRL_GR3);

        /* the return option is undefined : return when no more data is available */
        if (return_script_option == 0) 
        {   return_script_option = STREAM_SCHD_RET_END_NODE_NODATA;
        }
        ST(S->scheduler_control, RETURN_SCTRL, return_script_option);
    }

    S->linked_list_ptr = S->linked_list;

    ST(S->scheduler_control, INSTANCE_SCTRL, 1); /* this instance is active */ 

    //@@@ TODO for multiprocessing 
    //if (RD(S->scheduler_control, MAININST_SCTRL)) 
    //{   /* all other process can be released from wait state */
    //    platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); 

    platform_init_io (S);

    //@@@ TODO for multiprocessing 
    //} else
    //{   /* wait until the graph is copied in RAM */
    //    uint8_t wait; 
    //    do {
    //        platform_al (PLATFORM_MP_BOOT_WAIT, &wait, 0,0);
    //    } while (0u != wait);
    //}
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
                [0] 28b RAM address of part/all the graph, HW-shared MEM configuration, which part is going in RAM
                [1] number of FORMAT, size of SCRIPTS
                [2] size of LINKEDLIST, number of STREAM_INSTANCES
                [3] number of ARCS, number of DEBUG registers

                The application shares a pointer, the register [0] tells the graph is either:
                - already in RAM : nothing to do
                - to move in RAM (internal / external) partially or totally
                
                platform_init_copy_graph() implements this data move and calls platform AL to synchronize the other processor 
                to let them initialize the node instances in parallel.
  @remark
 */
void platform_init_copy_graph(arm_stream_instance_t *S, uint32_t **graph_dst)
{
    uint8_t RAMsplit;
    uint32_t graph_words_copy_in_ram, PIOongoing;
    uint32_t *graph_src, *graph_flash;

    /* 
        Graph is read from Flash and a small part is copied in RAM
        to save RAM (for example Cortex-M0 with 2kB of internal SRAM)

                   RAMsplit Option 0      1      2:already in RAM at graph0 address
        IO settings                RAM    Flash  
        SCRIPTS                    RAM    Flash  
        LINKED-LIST                RAM    Flash   RAM allows NODE to be desactivated
        PIO                        RAM    RAM    
        FORMAT x3 words            RAM    RAM
        ARC descriptors 4 words    RAM    RAM    
        Debug registers, Buffers   RAM    RAM    
     */


    *graph_dst = (uint32_t *)platform_pack2linaddr_ptr((intptr_t)(S->graph));
    graph_flash = S->graph;

    RAMsplit = (uint8_t)RD(((S->graph)[0]),RAMSPLIT_GR0);
    PIOongoing = 0;

    switch (RAMsplit)
    {   default :
            graph_src = &((S->graph)[0]);
            graph_words_copy_in_ram = 0;
            break;
        case COPY_CONF_GR0_COPY_ALL_IN_RAM: 
            graph_src = &((S->graph)[0]);
            graph_words_copy_in_ram = S->graph[-1]; 
            S->graph = *graph_dst;                       // take the new address after copy 
            break;
        case COPY_CONF_GR0_FROM_PIO: 
            PIOongoing = GRAPH_HEADER_NBWORDS                       // Header
                       + (STREAM_IOFMT_SIZE_W32 * RD((S->graph)[1], NB_IOS_GR1)) // PIO settings
                       + RD((S->graph)[1], SCRIPTSSZW32_GR1)        // Scripts
                       + RD((S->graph)[2], LINKEDLISTSZW32_GR2);    // linked list
            graph_src = &((S->graph)[PIOongoing]);
            graph_words_copy_in_ram = S->graph[-1] - PIOongoing; 
            break; 
    }

    /* finalize the copy by telling the other processors */   
    {   uint32_t i; 
        for (i=0;i<graph_words_copy_in_ram;i++)
        {   (*graph_dst)[i]=graph_src[i];
        }
    }    

    /* shift the address to make it used as if all the graph was in RAM */
    if (RAMsplit == COPY_CONF_GR0_FROM_PIO)
    {   *graph_dst -= PIOongoing;
    }

    /* copy the graph data to uint8_t platform_io_al_idx_to_stream[LAST_IO_FUNCTION_PLATFORM]; 
        to ease the translation from graph index to graph_io_idx used in arm_stream_io_ack() 
    */
    {   uint8_t graph_io_idx, tmpn;
        uint32_t tmpi, *pio_control;

        tmpn = (uint8_t) RD((S->graph)[1], NB_IOS_GR1);
        for (graph_io_idx = 0; graph_io_idx < tmpn; graph_io_idx++)
        {   
            pio_control = &(graph_flash[GRAPH_HEADER_NBWORDS + graph_io_idx * STREAM_IOFMT_SIZE_W32]);
            tmpi = (uint8_t) RD(*pio_control, FWIOIDX_IOFMT0);
            platform_io_al_idx_to_stream[tmpi] = graph_io_idx;
        }
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
void platform_init_io(arm_stream_instance_t *S)
{
    uint32_t nio;
    uint8_t graph_io_idx;
    uint8_t *ongoing;
    uint32_t *pio_control;
    uint32_t io_mask;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    const p_io_function_ctrl *io_func;
    const p_stream_al_services *al_func;

    extern arm_stream_instance_t * platform_io_callback_parameter;
    /*-------------------------------------------*/

    /* if cold start : clear the backup area */
    if (TEST_BIT(S->scheduler_control, BOOT_SCTRL_LSB) == STREAM_COLD_BOOT)
    {   al_func = &(S->al_services);
        (*al_func)(PACK_SERVICE(0,0,NOTAG_SSRV,SERV_INTERNAL_PLATFORM_CLEAR_BACKUP_MEM,SERV_GROUP_INTERNAL), 0,0, 0,0);
    }

    /* wait all the process have initialized the graph */
    {   //uint8_t wait; 
        //do {
        //    platform_al (PLATFORM_MP_RESET_WAIT, &wait, 0, RD(S->scheduler_control, NBINSTAN_SCTRL));
        //} while (wait == 0);
    }

    /*------------------------------------------- 
        initialization of the graph IO ports 
    */     
    io_mask = S->iomask;

    nio = RD(S->graph[1],NB_IOS_GR1);
    all_arcs = S->all_arcs;


    /* 
        the Graph interpreter instance holds the memory pointers 
        this is a read-only global variable to let the IO have access to ongoing[], list of arcs, formats[] ..
    */
    platform_io_callback_parameter = S;

    for (graph_io_idx = 0; graph_io_idx < nio; graph_io_idx++)
    {
        uint32_t *arc;
        stream_xdmbuffer_t io_setting;

        pio_control = &(S->graph[GRAPH_HEADER_NBWORDS + graph_io_idx * STREAM_IOFMT_SIZE_W32]);
        ongoing = &(S->ongoing[graph_io_idx]);

        /* does this port is managed by the Graph interpreter instance ? */
        if (0 == (io_mask & (1U << graph_io_idx))) 
            continue; 

        /* default value settings */
        io_setting.address = (intptr_t)(&(S->graph[GRAPH_HEADER_NBWORDS + graph_io_idx]));
        io_setting.size    = 4*(STREAM_IOFMT_SIZE_W32-1);
        io_func = &(S->platform_io[RD(*pio_control, FWIOIDX_IOFMT0)]);
        if (*io_func == 0) 
        {   continue;
        }
        (*io_func)(STREAM_RESET, &io_setting);

        /* 
            IO-Interface expects the buffer to be declared outside of the graph
            
            set the arc base address to the IO desired buffer address and size
        */
        if (IO_COMMAND_SET_BUFFER == RD(*pio_control, SET0COPY1_IOFMT0))
        {   
            stream_xdmbuffer_t pt_pt;
            pt_pt.address = 0x11112222;     // debug
            pt_pt.size = 0x33334444;        // debug

            io_func = &(S->platform_io[RD(*pio_control, FWIOIDX_IOFMT0)]);
            (*io_func)(STREAM_SET_BUFFER, &pt_pt);

            iarc = RD(*pio_control, IOARCID_IOFMT0);
            iarc = SIZEOF_ARCDESC_W32 * iarc;
            arc = &(all_arcs[iarc]);
            ST(arc[0], BASEIDXOFFARCW0, platform_lin2pack((uint8_t *)(pt_pt.address)));
            ST(arc[1], BUFF_SIZE_ARCW1, pt_pt.size);
            ST(arc[2], READ_ARCW2, 0);
            ST(arc[3], WRITE_ARCW3, 0);
        }
    } 
}


#endif
/*
 * -----------------------------------------------------------------------
 */
