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



#include "presets.h"

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


extern const uint8_t platform_audio_out_bit_fields[];

/* 
    this table will be extended with pointers to nodes loaded 
    dynamically and compiled with position independent execution options

    it is aligned with the list of SOFTWARE COMPONENTS MANIFESTS in "files_manifests_computer.txt"
*/
/* -----------------------------------
    Full node descriptions given in ./stream_tools/TEMPLATE_GRAPH.txt
*/

extern p_stream_node arm_stream_null_task;       /*  */
extern p_stream_node arm_stream_script;          /*  1  #define arm_stream_script_index 1 */
extern p_stream_node arm_stream_router;          /*  2 */
extern p_stream_node arm_stream_modulator;       /*  3 */
extern p_stream_node arm_stream_kws;             /*  4 */
extern p_stream_node arm_stream_fixedbf;         /*  5 */
extern p_stream_node arm_stream_filter2D;        /*  6 */
extern p_stream_node arm_stream_filter;          /*  7 */
extern p_stream_node arm_stream_demodulator;     /*  8 */
extern p_stream_node arm_stream_anr;             /*  9 */
extern p_stream_node arm_stream_amplifier;       /* 10 */
extern p_stream_node sigp_stream_compressor;     /* 11 */
extern p_stream_node sigp_stream_decompressor;   /* 12 */
extern p_stream_node sigp_stream_detector;       /* 13 */
extern p_stream_node sigp_stream_detector2D;     /* 14 */
extern p_stream_node sigp_stream_resampler;      /* 15 */
extern p_stream_node bitbank_JPEGENC;            /* 16 */
extern p_stream_node TjpgDec;                    /* 17 */

#define TBD 0


const p_stream_node node_entry_points[NB_NODE_ENTRY_POINTS] =
{
    /*  0 */ (p_stream_node)&arm_stream_null_task,      /* ID | PROC ARCH    */
    /*  1 */ (p_stream_node)&arm_stream_script,         /*  1 |   0   0      byte-code interpreter, index "arm_stream_script_INDEX" */
    /*  2 */ (p_stream_node)&arm_stream_router,         /*  2 |   0   1      copy input arcs and subchannel and output arcs and subchannels   */
    /*  3 */ (p_stream_node)&arm_stream_modulator,      /*  3 |   0   1      signal generator with modulation */
    /*  4 */ (p_stream_node)&arm_stream_kws,            /*  4 |   0   1
    /*  5 */ (p_stream_node)&arm_stream_fixedbf,        /*  5 |   0   1
    /*  6 */ (p_stream_node)&arm_stream_filter2D,       /*  6 |   0   1
    /*  7 */ (p_stream_node)&arm_stream_filter,         /*  7 |   0   1      cascade of DF1 filters */
    /*  8 */ (p_stream_node)&arm_stream_demodulator,    /*  8 |   0   1      signal demodulator, frequency estimator */
    /*  9 */ (p_stream_node)&arm_stream_anr,            /*  9 |   0   1
    /* 10 */ (p_stream_node)&arm_stream_amplifier,      /* 10 |   0   1      amplifier mute and un-mute with ramp and delay control */
    /* 11 */ (p_stream_node)&sigp_stream_compressor,    /* 11 |   0   1      raw data compression with adaptive prediction */
    /* 12 */ (p_stream_node)&sigp_stream_decompressor,  /* 12 |   0   1      raw data decompression */
    /* 13 */ (p_stream_node)&sigp_stream_detector,      /* 13 |   0   1      estimates peaks/floor of the mono input and triggers a flag on high SNR */
    /* 14 */ (p_stream_node)&sigp_stream_detector2D,    /* 14 |   0   2      activity detection, pattern detection */
    /* 15 */ (p_stream_node)&sigp_stream_resampler,     /* 15 |   0   1      asynchronous sample-rate converter */
    /* 16 */ (p_stream_node)&bitbank_JPEGENC,           /* 16 |   0   1      bitbank_JPEGENC */
    /* 17 */ (p_stream_node)&TjpgDec,                   /* 17 |   0   1      TjpgDec */
};    




/*
    ==============
    Base addresses 
    ==============
        1 1 9 4 number of architectures, number of processors, number of memory banks, number of offsets


;------------------------------------------------
                TOP_MANIFEST_COMPUTER.TXT
;------------------------------------------------
;   ID        offset ID (<15) shared memory by several processors
;   Size      in Bytes
;   Private   0 shared, 1 private memory
;   Access    0 read only, 1 R/W, 2 HW-read only, 3 HW R/W
;   Speed     0 slow/best effort, 1 fast/internal, 2 TCM
;   Data/prog 0 Data, 1 Prog, 2 Both
;   R         0 Static, 1 Retention

;   ID   SIZE   P A S D R   Comments
    0   10000   0 1 0 2 0   general purpose
    1    1000   1 1 2 0 0   simulates DTCM 
    2    1000   1 1 2 1 1   simulates ITCM
    3    1000   0 1 0 0 1   simulates retention


 */

uint8_t MEXT[SIZE_MBANK_DMEM_EXT];
uint8_t DTCM[SIZE_MBANK_DTCM]; 
uint8_t ITCM[SIZE_MBANK_ITCM]; 
uint8_t BACKUP[SIZE_MBANK_RETENTION]; 

const uint8_t * long_offset[MAX_NB_MEMORY_OFFSET] = 
{   &(MEXT[0]),
    &(DTCM[0]),
    &(ITCM[0]),
    &(BACKUP[0]),
};


/*
     pointer to the current graph interpreter instance 
*/
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


/*  ;-----IO AFFINITY WITH PROCESSOR 1-------------------------------
    ;Path      Manifest       IO_AL_idx    Comments               
    1 io_platform_data_in_0.txt       0    application proc
    1 io_platform_data_in_1.txt       1    application proc
    1 io_platform_analog_sensor_0.txt 2    ADC                    
    1 io_platform_audio_in_0.txt      4    microphone       
    1 io_platform_line_out_0.txt      6    audio out stereo       
    1 io_platform_gpio_out_0.txt      7    GPIO/LED               
    1 io_platform_data_out_0.txt      9    application proc  
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

/*
*   the graph to be executed 
*/
const uint32_t graph_computer_router[] = 
{ 
#include "graphs/graph_computer_router_bin.txt"
};

const uint32_t graph_computer_filter[] =
{
#include "graphs/graph_computer_filter_bin.txt"
};

const uint32_t graph_computer_filter_detector[] =
{
#include "graphs/graph_computer_filter_detector_bin.txt"
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
const p_stream_services application_callbacks[MAX_NB_APP_CALLBACKS] =
{   (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0
};


/*
    Table of 
*/
#define NB_PENDING_PARAM_UPDATES 2 // must be lower than MAX_NB_PENDING_PARAM_UPDATES
uintptr_t new_node_parameters[(1+NB_PENDING_PARAM_UPDATES) * 2] =
{   0, 0,  // [node idx in the graph ; physical address to the parameters in "boot" format]
    0, 0,  // [node idx; parameter address]
    // .. 
    0, 0,  // end of the list 
};

#define GRAPH_ROUTER 0
#define GRAPH_FILTER 1
#define GRAPH_FILTER_DETECTOR 2
uint32_t * get_graph_address(uint32_t graph_idx)
{
    if (graph_idx == GRAPH_ROUTER)
    {   return (uint32_t *) graph_computer_router;
    }
    if (graph_idx == GRAPH_FILTER)
    {
        return (uint32_t*)graph_computer_filter;
    }
    if (graph_idx == GRAPH_FILTER_DETECTOR)
    {
        return (uint32_t*)graph_computer_filter_detector;
    }
        
    return 0;
}

/**
  @brief            Shares the platform-specific data for the initialization of the interpreter instance 
  @param[in/out]    none
  @return           int
 */

void platform_init_specific(arm_stream_init_t *data)
{
#if GRAPH_FROM_PLATFORM == 2
    //data->graph = get_graph_address(GRAPH_ROUTER);          
    //data->graph = get_graph_address(GRAPH_FILTER);  
    data->graph = get_graph_address(GRAPH_FILTER_DETECTOR);
#endif

    data->long_offset = (uint8_t **) long_offset;           // pointer to "long_offset[MAX_NB_MEMORY_OFFSET]"

    data->application_callbacks = (p_stream_services)application_callbacks;  // callbacks used by scripts
    data->node_entry_points = (p_stream_node) node_entry_points;             // list of nodes
    data->platform_io = (p_io_function_ctrl)platform_io;                     // list of IO functions
    data->new_parameters = (uintptr_t)new_node_parameters;                              // list of pairs [offset; parameter address]

    data->procID = PROC_ID;
    data->archID = ARCH_ID;
}


/* here: test the need for memory recovery/swap
    does the application modified the memory banks used by the graph ? */
void arm_memory_swap(arm_stream_instance_t* S)
{
    //uint8_t memBankBitFields[(7 + MAX_PROC_MEMBANK) / 8];
}



#endif
/*
 * -----------------------------------------------------------------------
 */
