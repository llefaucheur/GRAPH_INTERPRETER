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


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  

#include "dsp\filtering_functions.h"


typedef int16_t q15_t;
#define MAX_NB_BIQUAD_Q15 4

/* ----------------------------- */
/*
    y[n] = 2^postShift (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) + a1 * y[n-1] + a2 * y[n-2]
    a1 and a2 have the opposite sign given by Matlab
*/

typedef struct
{
    int8_t numStages;        
    q15_t  coefs[MAX_NB_BIQUAD_Q15*5];        
    int8_t postShift;        
} arm_biquad_casd_df1_q15_preset;


typedef struct
{
    uint32_t iir_service;
    stream_services_entry *services;

    arm_biquad_casd_df1_inst_q15 biquad_casd_df1_inst_q15;
    q15_t coefs[MAX_NB_BIQUAD_Q15*5];
    q15_t state[MAX_NB_BIQUAD_Q15*4];

    // @@@@ create a UNION with the float version of the Biquad and call STREAM_SERVICE_CASCADE_DF1_F32
    //#define       UNUSED_FLT_MSB     
    //#define       UNUSED_FLT_LSB      /* 12  */
    //#define    TIMESTAMP_FLT_MSB     
    //#define    TIMESTAMP_FLT_LSB      /*  2 time_stamp_format_type*/
    //#define    INTERLEAV_FLT_MSB      
    //#define    INTERLEAV_FLT_LSB      /*  2 frame_format_type */ 
    //#define      RAWDATA_FLT_MSB       
    //#define      RAWDATA_FLT_LSB      /*  2 raw data format q15=0  f32=1 */
    //
    //    union
    //    {   arm_biquad_casd_df1_inst_q15 biq_q15;
    //        arm_biquad_casd_df1_inst_f32 biq_f32;
    //    } UC;
    //    union
    //    {   q15_t coefs_q15[MAX_NB_BIQUAD_Q15*5];
    //        q15_t coefs_f32[MAX_NB_BIQUAD_Q15*5];
    //    } US;
    //    union
    //    {   q15_t state_q15[MAX_NB_BIQUAD_Q15*4];
    //        float state_f32[MAX_NB_BIQUAD_Q15*4];
    //    } UM;
} arm_filter_instance;

#endif

#ifdef __cplusplus
}
#endif
 