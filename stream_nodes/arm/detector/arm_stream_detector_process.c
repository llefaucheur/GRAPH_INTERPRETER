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

#ifdef __cplusplus
 extern "C" {
#endif
   


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  


#define PRINTF 0

#include "arm_stream_detector.h"

#include <inttypes.h>
#include <stdbool.h>


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
void arm_stream_detector_process (arm_detector_instance *instance, 
                     int16_t *in, int32_t inputLength, 
                     int16_t *pResult)
{
    int32_t isamp = 0;
    int32_t input_data;

    //static long dbgZ4, dbgC;
    //static long counter = 0;

    while (isamp < inputLength) 
    {
        // Algorithm sensitive to scaling and filter initialisation, adjust scaling up here samp*(2^shift)
        // Clean voice: 8 to 12
        // Noisy voice   9
        // Acceleromter  9
        int shift = 9;
        input_data = ConvertSamp(in[isamp], shift);


        // Note: Adding two Q numbers may give a result in Q+1 making these steps prone to overflow
        Z2 = Z1  - DIVBIN(Z1 , SHPF) + input_data;
        Z3 = Z2 - Z1;
        Z1 = Z2;

        //dbgZ4 = Z3;
        Z3 = (Z3 < 0) ? (-Z3) : Z3;

        Z6 = DIVBIN(Z3, SLPF) + (Z6 - DIVBIN(Z6, SLPF));
        Z8 = DIVBIN(Z6, SFloorPeak) + (Z8 - DIVBIN(Z8, SFloorPeak));
        Z8 = MAX(Z6, Z8);

        DECF = decfMASK & (DECF -1);
        if (DECF == 0)
        {   Z7 = DIVBIN(Z6, SFloorPeak) + (Z7 - DIVBIN(Z7, SFloorPeak));
            Z7 = MAX(CLAMP_MIN, MIN(Z7, Z6));
            DECF = decfMASK;
        }

        if (Z8 > Z7 * THR)
        {   ACCVAD = MIN(CLAMP_MAX, ACCVAD + VADRISE);
        }
        else
        {   ACCVAD = MAX(CLAMP_MIN, ACCVAD - VADFALL);
        }

        if (ACCVAD > F2Q31(0.3))
        {   FLAG = MIN(CLAMP_MAX, FLAG + VADFALL);
        }
        else
        {
            FLAG = MAX(CLAMP_MIN, FLAG - VADRISE);
        }

        if (DOWNCOUNTER > 0)
        {   DOWNCOUNTER --;
        }
        else
        {   PREVIOUSVAD = 0;
        }

        /* signal detected => maintain the decision for some time */
        if (FLAG > F2Q31(0.5))
        {   if (PREVIOUSVAD == 0)
            {   uint64_t tmp64;
                tmp64 = (uint64_t)1 << (uint64_t)(RD(RELOADCOUNTER, EXPONENT));
                tmp64 = (uint64_t)tmp64 * RD(RELOADCOUNTER, MULTIPLIER);
                DOWNCOUNTER = (uint32_t) (tmp64 >> (MULTIPLIER_MSB+1));
                PREVIOUSVAD = 1;
            }
        }

        pResult[isamp] = (DOWNCOUNTER > 0);

#if PRINTF        
        {   
            // static long counter;
            counter++;
            if (pResult[isamp] != PREVIOUSVAD) 
            {   
                printf("VAD toggled to %d at %i \n", pResult[isamp], counter);
                PREVIOUSVAD = (int8_t) (pResult[isamp]);
            }
        }

        {
            #include <stdio.h>
            extern FILE *ptf_trace;
            long x; 
            if (dbgC++ == 73000)
                dbgC = dbgC;
            x = Z6<<2;                  fwrite(&x, 1, 4, ptf_trace);
            x = Z7<<2;                  fwrite(&x, 1, 4, ptf_trace);
            x = input_data;             fwrite(&x, 1, 4, ptf_trace);
            x = dbgZ4;                  fwrite(&x, 1, 4, ptf_trace);

            // x = Z8<<4;                  fwrite(&x, 1, 4, ptf_trace);
            x = ACCVAD;                 fwrite(&x, 1, 4, ptf_trace);
            x = FLAG;                   fwrite(&x, 1, 4, ptf_trace);
            x = Z8<<4;                  fwrite(&x, 1, 4, ptf_trace);
            x = in[isamp];   fwrite(&x, 1, 4, ptf_trace);
            x = (pResult[isamp])<<30;   fwrite(&x, 1, 4, ptf_trace);
        }
#endif     
        isamp++;
    }
}

#ifdef __cplusplus
}
#endif
 