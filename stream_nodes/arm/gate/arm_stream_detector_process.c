/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_detector.c
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

#define F2Q31(f) (long)(0x7FFFFFFL*(f))
#define F2Q15(f) (long)(0x7FFFL*(f))

#define ConvertSamp(f) (f<<15)
#define DIVBIN(s,n) (s>>n)

#include "stream_const.h"
#include "stream_types.h"

#include "arm_stream_detector.h"

// Debug
#include <stdio.h>
#include <inttypes.h>
/*
    9.	stream_detector
    Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
    a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time 
    for debouncing and LED / user-interface).
    
    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.
*/


/**
  @brief         Processing function 
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data = 0x7FFF when detected, else 0
  @param[in]     blockSize  number of samples to process
  @return        none
 */
void detector_processing (arm_detector_instance *instance, 
                     int16_t *in, int32_t nb_data, 
                     int16_t *outBufs)
{

    for (int i = 0; i < 2; i++)

    {
        printf("SampleRaw_detector_processing: %i \n", in[i]);
        int16_t input_data = ConvertSamp(in[i]);
        printf("SampConverted_detector_processing: %i \n", input_data);
    }

    // int i;

    // for (i = 0; i < nb_data; i++)
    // {
    //     // simplified VAD : reload the counter on energy detection
    //     if (in[i] > 500)
    //     {   instance->down_counter = 1 << (instance->config.log2counter);
    //     }

    //     if (instance->down_counter > 0) 
    //         outBufs[i] = 32767;
    //     else
    //         outBufs[i] = 0;

    //     if (instance->down_counter >= 1)
    //         instance->down_counter --;
    // }
}
