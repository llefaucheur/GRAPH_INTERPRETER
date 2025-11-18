/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_modulator.h
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
   

#ifndef cARM_STREAM_MODULATOR_H
#define cARM_STREAM_MODULATOR_H


#include "stream_const.h"      
#include "stream_types.h"  

#define PATTERN_SIZE 64

typedef struct
{
   float_t f0;          //;f32 wave frequency [Hz], 0Hz translates to DC level.
                        //;f32 modulation frequency [Hz] separating two data bits/samples from the arc
                        //;u16 wave type : 1=sine 2=square 3=white noise 4=pink noise 
                        //;            5=sawtooth 6=triangle 7=pulse
                        //;            8=prerecorded pattern playback from arc 
                        //;            9=sigma-delta with OSR control for audio on PWM ports and 8b DAC
                        //;            10=PWM 11=ramp 12=step
                        //;u16 modulation index (a) and offset (b), in q15 as fraction of frequency or amplitude
                        //;    modulation y=ax+b, x=input data.
                        //;u16 linear amplitude, format UQ16 0dB .. -96dB
                        //;u16 amplitude offset, format Q15 [-1 .. +1]
                        //;s16 starting phase, format Q15 [-pi .. +pi]/pi
                        //;u16 modulation type, 0:amplitude, 1:frequency (reserved 2:FSK, ..)
                        //;u16 modulation, 0:none 1=from arc bit stream, 2=from arc q15 words
} stream_modulator_parameters;

typedef struct
{
    stream_services *stream_entry;
    stream_modulator_parameters param;
} arm_stream_modulator_instance;



extern void arm_stream_modulator_process (arm_stream_modulator_instance *instance, void *input, void *output, intptr_t *nsamp);


#endif

#ifdef __cplusplus
}
#endif
 