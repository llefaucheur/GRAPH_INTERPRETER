/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_detector.h
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

#include "stream_const.h"
#include "stream_types.h"

/*
    9.	stream_detector
    Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
    a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time 
    for debouncing and LED / user-interface).
    
    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.
*/

typedef struct
{
    uint8_t log2counter;        /* sample counter= 2^log2counter: maintains the "detected" flag at least for this number of samples */
    uint8_t log2decfMAX;        /* decimation = a power of 2 (-1) */
    uint8_t high_pass_shifter;  /* for z1 */
    uint8_t low_pass_shifter;   /* for z6 */
    uint8_t floor_noise_shifter;/* for z7 */
    uint8_t peak_signal_shifter;/* for z8 */
} detector_parameters;

typedef struct /* total = 22 Bytes => 24bytes/6word32 */
{
    detector_parameters config; /* 6 bytes */

    int16_t z1;    /* memory of the high-pass filter (recursive part) */
    int16_t z2;    /* memory of the high-pass filter (direct part) */
    int16_t z3;    /* output of the high-pass filter */
    int16_t z6;    /* memory of the first low-pass filter */
    int16_t z7;    /* memory of the floor-noise tracking low-pass filter */
    int16_t z8;    /* memory of the envelope tracking low-pass filter */
    int32_t down_counter;    /* memory of the debouncing downcounter  */
} arm_detector_instance;

