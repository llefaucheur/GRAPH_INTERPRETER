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

/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 0

#define _CRT_SECURE_NO_DEPRECATE 1
#if DATA_FROM_FILES
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "stream_const.h"      
#include "stream_types.h"

#include "platform_computer.h"


extern const uint8_t platform_audio_out_bit_fields[];



/* 
    this table will be extended with pointers to nodes loaded 
    dynamically and compiled with position independent execution options

    it is aligned with the list of SOFTWARE COMPONENTS MANIFESTS in "files_manifests_computer.txt"
*/
/* -----------------------------------
    Full node descriptions given in ./stream_tools/TEMPLATE_GRAPH.txt
*/

extern p_stream_node arm_stream_script;         /*  1*/
extern p_stream_node arm_stream_router;         /*  2*/
extern p_stream_node arm_stream_converter;      /*  3*/
extern p_stream_node arm_stream_amplifier;      /*  4*/
extern p_stream_node arm_stream_mixer;          /*  5*/
extern p_stream_node arm_stream_filter;         /*  6*/
extern p_stream_node arm_stream_detector;       /*  7*/
extern p_stream_node arm_stream_rescaler;       /*  8*/
extern p_stream_node arm_stream_compressor;     /*  9*/
extern p_stream_node arm_stream_decompressor;   /* 10*/
extern p_stream_node arm_stream_modulator;      /* 11*/
extern p_stream_node arm_stream_demodulator;    /* 12*/
extern p_stream_node arm_stream_interpolator;   /* 13*/
extern p_stream_node arm_stream_qos;            /* 14*/
extern p_stream_node arm_stream_share;          /* 15*/
extern p_stream_node arm_stream_detector2D;     /* 16*/
extern p_stream_node arm_stream_filter2D;       /* 17*/
extern p_stream_node arm_stream_interpolator2D; /* 18*/
extern p_stream_node arm_stream_synchro;        /* 19*/

#define TBD 0

p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS] =
{
    /*--------- ARM ---------*/
    /*  0*/ (void *)0,                          /* node disabled */
    /*  1*/ (void *)&arm_stream_script,         /* byte-code interpreter, index "arm_stream_script_INDEX" */
    /*  2*/ (void *)&arm_stream_router,         /* copy input arcs and subchannel and output arcs and subchannels   */     
    /*  3*/ (void *)&arm_stream_converter,      /* raw data format converter */
    /*  4*/ (void *)&arm_stream_amplifier,      /* amplifier mute and un-mute with ramp and delay control */
    /*  5*/ (void *)&arm_stream_mixer,          /* multichannel mixer with mute/unmute and ramp control */
    /*  6*/ (void *)&arm_stream_filter,         /* cascade of DF1 filters */
    /*  7*/ (void *)&arm_stream_detector,       /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
    /*  8*/ (void *)&arm_stream_rescaler,       /* raw data values remapping using "interp1" */
    /*  9*/ (void *)&arm_stream_compressor,     /* raw data compression with adaptive prediction */
    /* 10*/ (void *)&arm_stream_decompressor,   /* raw data decompression */
    /* 11*/ (void *)&arm_stream_modulator,      /* signal generator with modulation */
    /* 12*/ (void *)&arm_stream_demodulator,    /* signal demodulator, frequency estimator */
    /* 13*/ (void *)&arm_stream_interpolator,   /* asynchronous sample-rate converter */
    /* 14*/ (void *)&arm_stream_qos,            /* raw data interpolator with synchronization with one HQoS stream */
    /* 15*/ (void *)&arm_stream_share,          /* let a buffer be used by several nodes */
    /* 16*/ (void *)&arm_stream_detector2D,     /* motion detect */
    /* 17*/ (void *)&arm_stream_filter2D,       /* arm_stream_filter2D,        */
    /* 18*/ (void *)&arm_stream_interpolator2D, /* arm_stream_interpolator2D,     */
    /* 19*/ (void *)&arm_stream_synchro,        /* arm_stream_multistream_synchro, */
};


static uint32_t MEXT[SIZE_MBANK_DMEM_EXT];
static uint32_t TCM1[SIZE_MBANK_DMEMFAST]; 
const intPtr_t long_offset[MAX_NB_MEMORY_OFFSET] = 
{   (const intPtr_t) &(MEXT[0]),
    (const intPtr_t) &(TCM1[0]),
};


uint32_t platform_iocontrol[LAST_IO_FUNCTION_PLATFORM];      /* bytes/flags of on-going activity */
arm_stream_instance_t * platform_io_callback_parameter;


extern p_io_function_ctrl data_in_0;      
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

     stream_in_0.txt        0    data from the application
     analog_sensor_0.txt    1    ADC 
     gpio_out_0.txt         2    PWM
     line_out_0.txt         3    audio out stereo
*/
const p_io_function_ctrl platform_io [LAST_IO_FUNCTION_PLATFORM] =
{
    (void *)&data_in_0,        // 0  ..windows/platform_manifest/io_platform_data_in_0.txt      
    (void *)&analog_sensor_0,    // 0 if used polling 1  ..windows/platform_manifest/io_platform_analog_sensor_0.txt
    (void *)&motion_in_0,      // 2  ..windows/platform_manifest/io_platform_motion_in_0.txt    
    (void *)&audio_in_0,       // 3  ..windows/platform_manifest/io_platform_audio_in_0.txt     
    (void *)&d2_in_0,          // 4  ..windows/platform_manifest/io_platform_d2_in_0.txt        
    (void *)&line_out_0,       // 5  ..windows/platform_manifest/io_platform_line_out_0.txt     
    (void *)&gpio_out_0,       // 6  ..windows/platform_manifest/io_platform_gpio_out_0.txt     
    (void *)&gpio_out_1,       // 7  ..windows/platform_manifest/io_platform_gpio_out_1.txt     
    (void *)&data_out_0,       // 8  ..windows/platform_manifest/io_platform_data_out_0.txt     
};


const uint32_t graph_input[] = 
{ 
#include "graph_0_bin.txt"
};


/*
    Four registered callback for SWC and scripts (CALS  system call (0..7) and application callbacks (8..15))
    Use-case : software timers, event detection trigger, metadata sharing
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

extern const p_stream_al_services al_services;
const p_stream_al_services al_service[1] =
{   (void *)&al_services
};

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
uint32_t lin2pack (arm_stream_instance_t *S, uint8_t *buffer)
{
    sintPtr_t distance;
    uint8_t i;

    /* packed address range is [ long_offset[IDX]  +/- 8MB ]*/
#define MAX_PACK_ADDR_RANGE (((1<<(BASEIDX_ARCW0_MSB - BASEIDX_ARCW0_LSB+1))-1))

    /* find the base offset */

    for (i = 0; i < (uint8_t)MAX_NB_MEMORY_OFFSET; i++)
    {
        distance = (sintPtr_t)buffer - (sintPtr_t)(S->long_offset[i]);
        if (ABS(distance) < MAX_PACK_ADDR_RANGE) 
        {   break;
        }
    }

    return (uint32_t)(distance | ((uint64_t)i << (uint8_t)DATAOFF_ARCW0_LSB));
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

void init_stream_instance(arm_stream_instance_t *instance, uint8_t stream_instance_idx)
{

extern p_stream_node node_entry_point_table[];
extern void platform_specific_long_offset(intPtr_t long_offset[]);
extern const uint32_t graph_input[];
extern const p_stream_al_services application_callbacks[];


#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 1

    /* reset instance */
    memset (instance, 0, sizeof(arm_stream_instance_t));

    instance->graph = (uint32_t *) &(graph_input[1]);

    instance->scheduler_control = PACK_STREAM_PARAM(
            STREAM_MAIN_INSTANCE,
            STREAM_ANY_PRIORITY,
            STREAM_NB_INSTANCE,
            STREAM_COLD_BOOT,
            STREAM_SCHD_NO_SCRIPT, 
            STREAM_SCHD_RET_END_ALL_PARSED
            );

    ST(instance->whoami_ports, ARCHID_PARCH, ARCH_ID);
    ST(instance->whoami_ports, PROCID_PARCH, PROC_ID);
    ST(instance->whoami_ports, INSTANCE_PARCH, stream_instance_idx);
    
    instance->iomask = 0x03FF;                      /* 10 IOs */
    instance->ioctrl= &(platform_iocontrol[0]);     /* list of bytes holding the status of the on-going data moves (MP) */

    instance->al_services = al_service;
    instance->application_callbacks = application_callbacks;
    instance->platform_io = platform_io;
    instance->node_entry_point_table = node_entry_point_table;
    instance->long_offset = long_offset;
}



#if MULTIPROCESSING != 0
#if 0
/**
  @brief        Memory banks initialization
  @param[in]    none
  @return       none

  @par          Loads the global variable of the platform holding the base addresses 
                to the physical memory banks described in the "platform manifest"

  @remark       
 */
uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   volatile uint8_t *pt8;
    pt8 = pt8b;

    *pt8 = code;
    INSTRUCTION_SYNC_BARRIER;

    /* no need to use LDREX, don't wait and escape if collision occurs */
    DATA_MEMORY_BARRIER;

    return (*pt8 == code);
}

#else

uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   return 1u;
}
#endif
#endif



/*
 * -----------------------------------------------------------------------
 */
