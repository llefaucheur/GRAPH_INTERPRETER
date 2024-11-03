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


#ifdef STREAM_PLATFORM_SERVICES 

    /* with MVE assembly or coprocessor acceleration */
    /* --------------------------------------------- */
    typedef int16_t q15_t;

    /**
     * @brief Copy of the Instance structure for the Q15 Biquad cascade filter.
     */
    typedef struct
    {
            q15_t *pState;           /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
      const q15_t *pCoeffs;          /**< Points to the array of coefficients.  The array is of length 5*numStages. */
            int8_t numStages;        /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
            int8_t postShift;        /**< Additional shift, in bits, applied to each output sample. */
    } Computer_arm_biquad_casd_df1_inst_q15;
  
 #else
    /* use CMSIS-DSP plain software  */
    /* ----------------------------- */
#include "filtering_functions.h"
    ///**
    // * @brief Instance structure for the Q15 Biquad cascade filter.
    // */
    //typedef struct
    //{
    //        q15_t *pState;           /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
    //  const q15_t *pCoeffs;          /**< Points to the array of coefficients.  The array is of length 5*numStages. */
    //        int8_t numStages;        /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
    //        int8_t postShift;        /**< Additional shift, in bits, applied to each output sample. */
    //} arm_biquad_casd_df1_inst_q15;


    ///**
    // * @brief Processing function for the Q15 Biquad cascade filter.
    // * @param[in]  S          points to an instance of the Q15 Biquad cascade structure.
    // * @param[in]  pSrc       points to the block of input data.
    // * @param[out] pDst       points to the block of output data.
    // * @param[in]  blockSize  number of samples to process.
    // */
    //extern void stream_filter_arm_biquad_cascade_df1_q15(
    //const arm_biquad_casd_df1_inst_q15 * S,
    //const q15_t * pSrc,
    //      q15_t * pDst,
    //      uint32_t blockSize);

    ///**
    // * @brief  Initialization function for the Q15 Biquad cascade filter.
    // * @param[in,out] S          points to an instance of the Q15 Biquad cascade structure.
    // * @param[in]     numStages  number of 2nd order stages in the filter.
    // * @param[in]     pCoeffs    points to the filter coefficients.
    // * @param[in]     pState     points to the state buffer.
    // * @param[in]     postShift  Shift to be applied to the output. Varies according to the coefficients format
    // */
    //extern void stream_filter_arm_biquad_cascade_df1_init_q15(
    //      arm_biquad_casd_df1_inst_q15 * S,
    //      uint8_t numStages,
    //const q15_t * pCoeffs,
    //      q15_t * pState,
    //      int8_t postShift);
#endif

/* ----------------------------- */

#define MAX_NB_BIQUAD_Q15 2

/* ----------------------------- */
/*
    format:  {b10, 0, b11, b12, a11, a12, b20, 0, b21, b22, a21, a22, ...}
    y[n] = 2^postShift (b0 * x[n] + b1 * x[n-1] + b2 * x[n-2]) + a1 * y[n-1] + a2 * y[n-2]
    a1 and a2 have the opposite sign given by Matlab
*/

//typedef struct
//{   int8_t numStages;        
//    q15_t  coefs[MAX_NB_BIQUAD_Q15*6];        
//    int8_t postShift;        
//} arm_biquad_casd_df1_q15_preset;


typedef struct
{   q15_t coefs[MAX_NB_BIQUAD_Q15*6];
    q15_t state[MAX_NB_BIQUAD_Q15*4];    

#ifdef STREAM_PLATFORM_SERVICES 
    Computer_arm_biquad_casd_df1_inst_q15 biquad_casd_df1_inst_q15;
#else
    arm_biquad_casd_df1_inst_q15 biquad_casd_df1_inst_q15;
#endif
} arm_filter_memory;


typedef struct
{   uint32_t iir_service;
    stream_al_services *services;
    arm_filter_memory *TCM;
} arm_filter_instance;

#endif

#ifdef __cplusplus
}
#endif
 