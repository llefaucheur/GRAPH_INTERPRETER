/* ----------------------------------------------------------------------
 * Title:        sigp__stream_detector2D_process.c
 * Description:  filters
 *
 * $Date:        15 February 2024
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2013-2024 signal-processing.fr. All rights reserved.
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
#ifdef CODE_SIGP_STREAM_DETECTOR2D

#ifdef __cplusplus
 extern "C" {
#endif
   
#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"

#define PRINTF 0 // debug

#include "sigp_stream_detector2D.h"

#include <inttypes.h>
#include <stdbool.h>


/*
    16.	stream_detector2D
    Operation : 

    Parameters : 
*/


/**
  @brief         Processing function 
  @param[in]     instance     points to an instance of the Biquad cascade structure
  @param[in]     pSrc         points to the block of input data
  @param[out]    pResult      points to the result flag = 0x7FFF when detected, else 0
  @param[in]     inputLength  number of samples to process
  @return        none
 */
void sigp_stream_detector2D_process (sigp_detector2D_instance *instance, 
                     int16_t *in, int32_t inputLength, 
                     int16_t *pResult)
{
}

#ifdef __cplusplus
}
#endif
 

#endif