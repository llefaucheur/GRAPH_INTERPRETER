/*
 * Copyright (c) 2021 ARM Limited
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /**
  * \file main.c
  * \brief This file holds the subroutine estimating the voice activity
  */
  
  /*

    Test of the VAD

    The test pattern embedded is at 48kHz sampling rate Q15 format
    VAD introduces 100ms latency (4800 samples) : samples must be buffered 
    to recover the start of utterances
*/

#include <stdio.h>
#include <stdint.h>
#include "../arm_stream_detector.h"
#include "stream_types.h"
#include <inttypes.h>

const int16_t AudioInputSamples[] = {
/* 
Fixed-point toggle times:
VAD toggled to 1 at 218953
VAD toggled to 0 at 275565
VAD toggled to 1 at 349572
VAD toggled to 0 at 405834

Floating-point toggle times:
VAD toggled to 1 at 218952
VAD toggled to 0 at 276285
VAD toggled to 1 at 349566
VAD toggled to 0 at 406267
*/
#include "TestPattern.txt"
};

#define FS 48000

void main (void)
{
    arm_detector_instance instance;
    uint32_t status;
    // Note code is intended for 32 bit but debugging on 64; casts/pointer mismatches must be considered

    data_buffer_t *dummy_for_set_param;
    arm_stream_detector(STREAM_SET_PARAMETER, (uint32_t*)&instance, dummy_for_set_param , &status);

    data_buffer_t input = {.address = (intptr_t)AudioInputSamples, .size = sizeof(AudioInputSamples)};
    arm_stream_detector(STREAM_RUN, (uint32_t*)&instance, &input, &status);
}
