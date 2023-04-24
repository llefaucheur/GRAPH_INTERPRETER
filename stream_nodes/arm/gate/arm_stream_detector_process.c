/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm__stream_detector_process.c
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

#include "arm_stream_detector.h"

#include <inttypes.h>

#define F2Q31(f) (long)(0x7FFFFFFL*(f))
#define F2Q15(f) (long)(0x7FFFL*(f))

#define ConvertSamp(f) (f<<15)
#define DIVBIN(s,n) (s>>n)

// Threshold/clamping to prevent the value 0 entering filter calculations
#define lowestS  F2Q31(0.001)

#define INVTHR  F2Q15(1 / 3.0)
#define MAXVAD  F2Q31(0.98)

// Filter variables
#define Z1 instance->z1
#define Z2 instance->z2
#define Z3 instance->z3
#define Z6 instance->z6
#define Z7 instance->z7
#define Z8 instance->z8
#define SHPF instance->config.high_pass_shifter
#define SLPF instance->config.low_pass_shifter // S1 in tinyVad
// TODO Verify if these can be replaced by a single value 
// S2 in tinyvad. Used for both floor & peak calculations (z7 & z8)
#define SFloorandPeak instance->config.floor_noise_shifter
// Replace above with #define SPeak  instance->config.peak_signal_shifter if we do need separate values for each

// Time constants for VAD algorithm
#define VADRISE F2Q31(instance->config.vad_rise)
#define VADFALL F2Q31(instance->config.vad_fall)

// Value of decremented counter is 2^log2decfMAX -1 to allow mask to be used to detect rollover
#define decfMAX ((1 << (instance->config.log2decfMAX)) -1)

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
  @param[in]     instance     points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc         points to the block of input data
  @param[out]    pResult      points to the result flag = 0x7FFF when detected, else 0
  @param[in]     inputLength  number of samples to process
  @return        none
 */
void detector_processing (arm_detector_instance *instance, 
                     int16_t *in, int32_t inputLength, 
                     int16_t *pResult)
{
    // TODO:
    //  Add nullpointer and other memory checks if necessary
    //  Windowing or ping-pong buffer to allow live stream data to be processed

    // WIP We are using int16_t for outbuf now so types don't match, try this simple correction for now
    int16_t previous_vad = 0;
    int32_t accvad, Flag, tmpz8;

    Z6 = F2Q31(0.001);
    Z7 = F2Q31(0.1);
    Z1 = Z2 = Z3 = Z8 = accvad = Flag = 0;

    int32_t decf = decfMAX;
    int32_t isamp = 0;
    int32_t input_data;

    // Filtering Cascade 
    while (isamp < inputLength) {
        {
            input_data = ConvertSamp(in[isamp]);
            Z2 = Z1  - DIVBIN(Z1 , SHPF) + input_data;
            Z3 = Z2 - Z1;
            Z1  = Z2;

            decf = MIN(MAX(decf - 1, 0), decfMAX);
            Z3 = (Z3 < 0) ? (-Z3) : Z3;//abs(y)
            Z6 = DIVBIN(Z3, SLPF) + (Z6 - DIVBIN(Z6, SLPF));
            Z8 = DIVBIN(Z6, SFloorandPeak) + (Z8 - DIVBIN(Z8, SFloorandPeak));
            Z8 = MAX(Z6, Z8);

            if (decf == 0)
            {
                Z7 = DIVBIN(Z6, SFloorandPeak) + (Z7 - DIVBIN(Z7, SFloorandPeak));
                Z7 = MAX(lowestS, MIN(Z7, Z6));
                decf = decfMAX;
            }

            tmpz8 = ((Z8 >> 16) * INVTHR) >> 15;
            if (tmpz8 > (Z7 >> 16))  accvad = MIN(MAXVAD, accvad + VADRISE);
            else
                accvad = MAX(0, accvad - VADFALL);

            if (accvad > F2Q31(0.3))
                Flag = MIN(F2Q31(0.98), Flag + VADFALL);
            else
                Flag = MAX(0, Flag - VADRISE);
            pResult = (Flag > F2Q31(0.5));
        }
        isamp++;

        if (pResult != previous_vad) {
            printf("VAD toggled to %d at %i", pResult, isamp);
            previous_vad = pResult;
        }
    }
}
