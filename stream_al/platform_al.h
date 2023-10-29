/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
 * Description:  
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

#ifndef platform_al_H
#define platform_al_H

/*------------------------*/
#include <stdint.h>

#include "platform_computer.h"

//#include "platform_arduino_nano33.h"
/*------------------------*/

extern uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code);


/*============================  ARCHITECTURES ============================================*/

enum stream_processor_architectures  /* 8bits : 256 architectures */
{
    ANY_ARCH    = 0,    /* source code without intrinsics and inline assembly */
    ARMv6_M0    = 1,    /* Cortex-M0+ */
    ARMv6_M3    = 2,    /* Cortex-M3 */
    ARMv7E_M    = 3,    /* Cortex-M4/M7 */
    ARMv8_M     = 4,    /* Cortex-M33 */
    ARMv8p1_M   = 5,    /* Cortex-M55 */

    ARMv7_R     = 21,   /* Cortex-R4 */
    ARMv8_R     = 22,   /* Cortex-R52 */

    ARMv7_A     = 31,   /* Cortex-A7 */

    ARMv8_A32   = 41,   /* Cortex-A32 */
    ARMv8_A64   = 42,   /* Cortex-A55 */

    ARMv9_A     = 51,

    C166        = 81, 
    C51         = 82,  
    LAST_ARCH,
};

enum stream_processor_sub_arch_fpu    /* bit-fields, coded on 6b max => 64 options */
{
    ARCH_INTEGER    = 1,  /* computations with integers */
    ARCH_FPU        = 2,  /* scalar fp32 */
    ARCH_DPFPU      = 3,  /* scalar fp32 + fp64 */
    ARCH_VECTOR     = 4,  /* NEON/MVE + fp16 + fp32 + dotProd8b */
    ARCH_ADV_VECTOR = 5,  /* NN CA55 */
};


/*======================================================================================*/

extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3);
 
extern int32_t extract_sensor_field (const uint8_t *platform_bit_fields, 
                              const int32_t *settings,
                              uint8_t setting_line,
                              uint8_t index);

#endif /* #ifndef platform_al_H */
#ifdef __cplusplus
}
#endif
 