/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_filter.c
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
   

#ifndef cARM_DSP_FILTER_H
#define cARM_DSP_FILTER_H


#include "stream_const.h"      
#include "stream_types.h"  
#include "dsp\filtering_functions.h"


typedef int16_t q15_t;
#define MAX_NB_BIQUAD_Q15 2

/* ----------------------------- */
/*
    format:  {b10, 0, b11, b12, a11, a12, b20, 0, b21, b22, a21, a22, ...}
    y[n] = 2^postShift (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) + a1 * y[n-1] + a2 * y[n-2]
    a1 and a2 have the opposite sign given by Matlab
*/

typedef struct
{
    int8_t numStages;        
    q15_t  coefs[MAX_NB_BIQUAD_Q15*6];        
    int8_t postShift;        
} arm_biquad_casd_df1_q15_preset;


typedef struct
{
    q15_t coefs[MAX_NB_BIQUAD_Q15*6];
    q15_t state[MAX_NB_BIQUAD_Q15*4];    
    arm_biquad_casd_df1_inst_q15 biquad_casd_df1_inst_q15;

} arm_filter_memory;

typedef struct
{
    uint32_t iir_service;
    stream_services_entry *services;
    arm_filter_memory static_mem;

} arm_filter_instance;

#endif

#ifdef __cplusplus
}
#endif
 