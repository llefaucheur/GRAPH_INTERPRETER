/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        platform.h
 * Description:  top-level platform informations
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
#ifndef PLATFORM_H
#define PLATFORM_H

#define PLATFORM_COMPUTER 1

#define PLATFORM_MSPM0L1306 0
#define PLATFORM_MSPM0G3507 0
#define PLATFORM_MSPM0C1104 0
#define PLATFORM_LPC55S69EVK 0
#define PLATFORM_LPCX860MAX 0
#define PLATFORM_LPCX804B 0
#define PLATFORM_LPCX55S69A1 0
#define PLATFORM_WIOTERMINAL 0
#define PLATFORM_GROVEVISIONAIV2 0
#define PLATFORM_EFM32PG32 0
#define PLATFORM_RP2040 0
#define PLATFORM_RP2350 0
#define PLATFORM_STM32U083DK 0
#define PLATFORM_STM32C0116DK 0
#define PLATFORM_STEVAL_MKBOXPRO 0
#define PLATFORM_STM32L475EIOT01 0
#define PLATFORM_STEVALMKBOXPRO 0
#define PLATFORM_ARDUNANO32SENSEV2 0
#define PLATFORM_ARDURP2040 0
#define PLATFORM_ARDU_UNOR4W 0
#define PLATFORM_ARDU_ZERO 0

/*============================ SWITCHES =====================================*/
#define PLATFORM_ARCH_32BIT
//#define PLATFORM_ARCH_64BIT

/* 
    if 64 bits architectures are reading the graph:
    #define intPtr_t uint64_t
    #define MAX_ADD_OFFSET 0x7FFFFFFFFFFFFFFFL
*/
#ifdef PLATFORM_ARCH_32BIT
#define intPtr_t uint32_t 
#define sintPtr_t int32_t
#else
#define intPtr_t uint64_t 
#define sintPtr_t int64_t 
#endif
/*==========================================================================*/

#endif /* #ifndef PLATFORM_H */
