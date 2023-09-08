/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_amplitude.h
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

#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  

#define TAG_CMD_RAMPUP 1
#define TAG_CMD_RAMPDOWN  2          
#define TAG_CMD_DELAY_RAMPUP  3      
#define TAG_CMD_DELAY_RAMPDOWN  4    
#define TAG_CMD_CURRENT_GAIN  5      
#define TAG_CMD_DESIRED_GAIN  6      
#define TAG_CMD_UNMUTE  7            
#define TAG_CMG_MUTE  8              

typedef struct
{
    uint32_t I[5];

    //uint16_t iirc_ramp_up;  /* ramp-up Q16 coefficient for ramp-ups: the command is larger than current_amp */
    //uint16_t iirc_ramp_down;/* ramp-down Q16 coefficient for ramp-downs: the command is smaller than current_amp */
#define iirc_ramp_up(I) ((uint16_t)(I[0]))
#define iirc_ramp_down(I) ((uint16_t)((I[0])>>16))

    //uint16_t delay_ramp_up; /* delay in samples before applying the ramp-up */
    //uint16_t delay_ramp_down; /* delay in samples before applying the ramp-down */
#define delay_ramp_up(I) ((uint16_t)(I[1]))
#define delay_ramp_down(I) ((uint16_t)((I[1])>>16))

    //uint32_t current_amp;   /* filtered amplification in Q15.16*/
    //uint32_t desired_amp;   /* desired amplification in Q15.16 */
#define current_amp(I) (I[2])
#define desired_amp(I) (I[3])

    //uint8_t mute_state;     /* forces amplification command to 0 when =1*/
#define mute_state(I) (I[4])
} arm_amplitude_instance;


extern void arm_stream_amplitude_process (arm_amplitude_instance *instance, void *input, void *output, intPtr_t *nsamp);

#endif

#ifdef __cplusplus
}
#endif
 