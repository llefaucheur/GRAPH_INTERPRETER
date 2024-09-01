/******************************************************************************
 * @file     filtering_functions.h
 * @brief    Public header file for CMSIS DSP Library
 * @version  V1.10.0
 * @date     08 July 2021
 * Target Processor: Cortex-M and Cortex-A cores
 ******************************************************************************/
/*
 * Copyright (c) 2010-2020 Arm Limited or its affiliates. All rights reserved.
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
 */

 
#ifndef _FILTERING_FUNCTIONS_H_
#define _FILTERING_FUNCTIONS_H_

#include "../arm_math_types.h"
#include "../arm_math_memory.h"

#include "../dsp/none.h"
#include "../dsp/utils.h"

#include "../dsp/support_functions.h"
#include "../dsp/fast_math_functions.h"

#ifdef   __cplusplus
extern "C"
{
#endif



#define DELTA_Q31          ((q31_t)(0x100))
#define DELTA_Q15          ((q15_t)0x5)

/**
 * @defgroup groupFilters Filtering Functions
 */
    
  /**
   * @brief Instance structure for the Q15 Biquad cascade filter.
   */
  typedef struct
  {
          q15_t *pState;           /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
    const q15_t *pCoeffs;          /**< Points to the array of coefficients.  The array is of length 5*numStages. */
          int8_t numStages;        /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
          int8_t postShift;        /**< Additional shift, in bits, applied to each output sample. */
  } arm_biquad_casd_df1_inst_q15;


  /**
   * @brief Instance structure for the floating-point Biquad cascade filter.
   */
  typedef struct
  {
          uint32_t numStages;      /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
          float32_t *pState;       /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
    const float32_t *pCoeffs;      /**< Points to the array of coefficients.  The array is of length 5*numStages. */
  } arm_biquad_casd_df1_inst_f32;

#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
  /**
   * @brief Instance structure for the modified Biquad coefs required by vectorized code.
   */
  typedef struct
  {
      float32_t coeffs[8][4]; /**< Points to the array of modified coefficients.  The array is of length 32. There is one per stage */
  } arm_biquad_mod_coef_f32;
#endif 

  /**
   * @brief Processing function for the Q15 Biquad cascade filter.
   * @param[in]  S          points to an instance of the Q15 Biquad cascade structure.
   * @param[in]  pSrc       points to the block of input data.
   * @param[out] pDst       points to the block of output data.
   * @param[in]  blockSize  number of samples to process.
   */
  void Computer_arm_biquad_cascade_df1_q15(
  const arm_biquad_casd_df1_inst_q15 * S,
  const q15_t * pSrc,
        q15_t * pDst,
        uint32_t blockSize);

  /**
   * @brief  Initialization function for the Q15 Biquad cascade filter.
   * @param[in,out] S          points to an instance of the Q15 Biquad cascade structure.
   * @param[in]     numStages  number of 2nd order stages in the filter.
   * @param[in]     pCoeffs    points to the filter coefficients.
   * @param[in]     pState     points to the state buffer.
   * @param[in]     postShift  Shift to be applied to the output. Varies according to the coefficients format
   */
  void Computer_arm_biquad_cascade_df1_init_q15(
        arm_biquad_casd_df1_inst_q15 * S,
        uint8_t numStages,
  const q15_t * pCoeffs,
        q15_t * pState,
        int8_t postShift);

  /**
   * @brief Fast but less precise processing function for the Q15 Biquad cascade filter for Cortex-M3 and Cortex-M4.
   * @param[in]  S          points to an instance of the Q15 Biquad cascade structure.
   * @param[in]  pSrc       points to the block of input data.
   * @param[out] pDst       points to the block of output data.
   * @param[in]  blockSize  number of samples to process.
   */
  void Computer_arm_biquad_cascade_df1_fast_q15(
  const arm_biquad_casd_df1_inst_q15 * S,
  const q15_t * pSrc,
        q15_t * pDst,
        uint32_t blockSize);


  /**
   * @brief Processing function for the floating-point Biquad cascade filter.
   * @param[in]  S          points to an instance of the floating-point Biquad cascade structure.
   * @param[in]  pSrc       points to the block of input data.
   * @param[out] pDst       points to the block of output data.
   * @param[in]  blockSize  number of samples to process.
   */
  void Computer_arm_biquad_cascade_df1_f32(
  const arm_biquad_casd_df1_inst_f32 * S,
  const float32_t * pSrc,
        float32_t * pDst,
        uint32_t blockSize);

  /**
   * @brief  Initialization function for the floating-point Biquad cascade filter.
   * @param[in,out] S          points to an instance of the floating-point Biquad cascade structure.
   * @param[in]     numStages  number of 2nd order stages in the filter.
   * @param[in]     pCoeffs    points to the filter coefficients.
   * @param[in]     pCoeffsMod points to the modified filter coefficients (only MVE version).
   * @param[in]     pState     points to the state buffer.
   */
#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
  void Computer_arm_biquad_cascade_df1_mve_init_f32(
      arm_biquad_casd_df1_inst_f32 * S,
      uint8_t numStages,
      const float32_t * pCoeffs, 
      arm_biquad_mod_coef_f32 * pCoeffsMod, 
      float32_t * pState);
#endif
  
  void Computer_arm_biquad_cascade_df1_init_f32(
        arm_biquad_casd_df1_inst_f32 * S,
        uint8_t numStages,
  const float32_t * pCoeffs,
        float32_t * pState);


#ifdef   __cplusplus
}
#endif

#endif /* ifndef _FILTERING_FUNCTIONS_H_ */
