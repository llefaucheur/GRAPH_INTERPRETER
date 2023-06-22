/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_filter.c
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


/* ----------------------------- */
typedef struct
{
    int32_t b0, b1, b2, a1, a2;     /* coefficients */
    int32_t q0, z1, z2;             /* error spectral shaping, 2 states */
} arm_filter_biquad_q15;
/* ----------------------------- */
typedef struct
{
    float b0, b1, b2, a1, a2;     /* coefficients */
    float q0, z1, z2;             /* error spectral shaping, 2 states */
} arm_filter_biquad_float;
/* ----------------------------- */

typedef struct
{
    /* the first field of instance is used to give the TCM address */
    intPtr_t *TCM_working;

    uint32_t frameSize;

#define       UNUSED_FLT_MSB U(31)
#define       UNUSED_FLT_LSB U(20) /* 12  */
#define    TIMESTAMP_FLT_MSB U(19)
#define    TIMESTAMP_FLT_LSB U(18) /*  2 time_stamp_format_type*/
#define    INTERLEAV_FLT_MSB U(17) 
#define    INTERLEAV_FLT_LSB U(16) /*  2 frame_format_type */ 
#define       NBCHAN_FLT_MSB U(15)
#define       NBCHAN_FLT_LSB U(10) /*  6 nbchan (1 + nbchanM1) */
#define      RAWDATA_FLT_MSB U( 9)  
#define      RAWDATA_FLT_LSB U( 4) /*  6 raw data format "stream_raw_data" */
#define     NUMSTAGE_FLT_MSB U( 3)
#define     NUMSTAGE_FLT_LSB U( 0) /*  4 number of biquad stages */
    uint32_t format;

    uint32_t numstages;

    /* memory and coefficients, in cascade x , 
        copied to *TCM_working before computations */
    union
    {   arm_filter_biquad_q15   biq_q15;
        arm_filter_biquad_float biq_float;
    } U;

} arm_filter_instance;

#endif

#ifdef __cplusplus
}
#endif
 