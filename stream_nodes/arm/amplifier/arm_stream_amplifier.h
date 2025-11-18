/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_amplitude.h
 * Description:  filters
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

#ifndef CARM_STREAM_AMPLIFIER_H
#define CARM_STREAM_AMPLIFIER_H

#include <stdint.h>
//#include "../stream_nodes/stream_common_const.h"      
//#include "../stream_nodes/stream_common_types.h"  
#include "stream_common_const.h"      
#include "stream_common_types.h"  

#define TAG_CMD_RAMP 1
#define TAG_CMD_DESIRED_GAIN 2
#define TAG_CMG_MUTE 3              
#define TAG_CMD_DELAY_RAMPUP 4
#define TAG_CMD_DELAY_RAMPDOWN 5    

typedef struct
{

        /* word 0 : one word of 16bits + 2 bytes */
#define         MUTED_MSB 24  
#define         MUTED_LSB 24  /* 1 muted */   

#define         SLOPE_MSB 19  
#define         SLOPE_LSB 16  /* 4  slope of gain ramp-down */

#define  DESIRED_GAIN_MSB 11 
#define GAIN_EXPONENT_MSB 11  
#define GAIN_EXPONENT_LSB  8  /* 4   */  
#define GAIN_MANTISSA_MSB  7  
#define GAIN_MANTISSA_LSB  0  /* 8 FP_8m4em : 8b mantissa, e=4b exponent, gain=(m)>>(e+2) */  
#define  DESIRED_GAIN_LSB  0

        /* word 1 : two words of 16bits */
#define      DELAY_UP_MSB 31  
#define      DELAY_UP_LSB 16 /* 16 delay before applying unmute state */   
#define    DELAY_DOWN_MSB 15  
#define    DELAY_DOWN_LSB 0  /* 16 delay before applying mute state */   

    uint32_t parameters[2];

        /* word 1 format */
    uint32_t current_delay;

        /* state of the IIR filter used for the slope computation */
    uint32_t ampli_current_mantissa;

    /*
        multi-channel format 
    */
#define    AMPLICHAN___FMT_MSB 31  
#define    AMPLICHAN___FMT_LSB 30  
#define FRAMESIZE_FMT0_FMT_MSB 29   
#define FRAMESIZE_FMT0_FMT_LSB  6  /* 24 copy of arc's format FRAMESIZE_FMT0 */
#define   INTERLEAVING_FMT_MSB  5  
#define   INTERLEAVING_FMT_LSB  5  /* 1 interleaving "frame_format_type"  */  
#define NBCHANM1_AMPLI_FMT_MSB  4  
#define NBCHANM1_AMPLI_FMT_LSB  0  /* 5 up to 32 channels */  
    uint32_t channel_fmt;

    /* CMSIS-STREAM "services" : debug traces, compute library, .. */
    //stream_services *services;

} arm_amplitude_instance;


extern void arm_stream_amplitude_process (arm_amplitude_instance *instance, void *input, void *output /*, intptr_t *nsamp*/);

#endif

#ifdef __cplusplus
}
#endif
 