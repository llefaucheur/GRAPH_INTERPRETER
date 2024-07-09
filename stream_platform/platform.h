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

#define PLATFORM_COMPUTER 0             //Windows 32/64bits
#define PLATFORM_LPC55S69EVK 1          //LPCXPRESSO line-in line-out
#define PLATFORM_MSPM0L1306 0           //CCS  + MSPM0C1104
#define PLATFORM_LPCX860MAX 0           //
#define PLATFORM_LPCX804B 0             // 
#define PLATFORM_WIOTERMINAL 0          //<<<<
#define PLATFORM_GROVEVISIONAIV2 0      //
#define PLATFORM_EFM32PG32 0            //Simplicity Studio
#define PLATFORM_RP2040 0               //<<<<
#define PLATFORM_RP2350 0               //<<<<
#define PLATFORM_STM32U083DK 0          //<<<<
#define PLATFORM_STM32C0116DK 0         //PACK
#define PLATFORM_STM32L475EIOT01 0      //---
#define PLATFORM_STM32U585IOT02A        //PACK    Light,TOF,Pressure,IMU,Temp,MIC
#define PLATFORM_STEVALMKBOXPRO 0       //CubeIDE Pressure,IMU,Temp,MIC
#define PLATFORM_ARDU_NANO33SENSEV2 0   //Arduino 
#define PLATFORM_ARDU_RP2040 0          //Arduino IMU,MIC
#define PLATFORM_ARDU_UNOR4W 0          //Arduino 
#define PLATFORM_ARDU_ZERO 0            //Arduino 
#define PLATFORM_ARDU_NICLAVISION 0     //Arduino 
#define PLATFORM_PORTENTAC33 0          //Arduino SE050C2 Secure Element

/*============================ SWITCHES =====================================*/
#define PLATFORM_ARCH_32BIT
//#define PLATFORM_ARCH_64BIT

/*==========================================================================*/

#endif /* #ifndef PLATFORM_H */
