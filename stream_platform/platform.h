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

#define PLATFORM_COMPUTER               //Windows 32/64bits
//#define PLATFORM_WIOTERMINAL            //<<<<
//#define PLATFORM_LPC55S69EVK            //LPCXPRESSO line-in line-out
//#define PLATFORM_MSPM0L1306             //SYSOSC 24MHz/320 = FS 75kHz
//#define PLATFORM_MCXC041                //2K RAM IMU temp LED
//#define PLATFORM_MCXN236                //DMIC, audio, dual M33, crypto, display
//#define PLATFORM_CY8CKIT_062S2          //M4
//#define PLATFORM_CY8CKIT_E84            //M55
//#define PLATFORM_GROVEVISIONAIV2        //
//#define PLATFORM_EFM32PG32              //Simplicity Studio
//#define PLATFORM_RP2040                 //<<<<
//#define PLATFORM_RP2350                 //<<<<
//#define PLATFORM_STM32U083DK            //<<<<
//#define PLATFORM_STM32C0116DK           //PACK
//#define PLATFORM_STM32L475EIOT01        //---
//#define PLATFORM_STM32U585IOT02A        //PACK    Light,TOF,Pressure,IMU,Temp,MIC
//#define PLATFORM_STEVALMKBOXPRO         //CubeIDE Pressure,IMU,Temp,MIC
//#define PLATFORM_NUVOTON_M2003          //CM0 24MHz, SAR, PWM, 4kB/32kB
//#define PLATFORM_RENESAS_RA4M1          //CM4 48MHz, 16kB/256kB
//#define PLATFORM_ARDU_NANO33SENSEV2     //Arduino 
//#define PLATFORM_ARDU_RP2040            //Arduino IMU,MIC
//#define PLATFORM_ARDU_UNOR4W            //Arduino 
//#define PLATFORM_ARDU_ZERO              //Arduino 
//#define PLATFORM_PORTENTAC33            //Arduino SE050C2 Secure Element

/* default streaming for the demos :

* i data_in0 from application 
* i adc0 or file_in
* i timer0
* o gpio_out0 or led
* o data_out0 to the application 
* o dac0/pwm or file_out
*   
*/

/*==========================================================================*/

#ifdef PLATFORM_COMPUTER
#include "computer/platform_computer.h"
#endif
#ifdef PLATFORM_LPC55S69EVK
#include "lplpc55s69evk/platform_lpc55s69evk.h"
#endif
#ifdef PLATFORM_MSPM0L1306
#include "lp-mspm0l1306/platform_lp-mspm0l1306.h"
#endif
/*==========================================================================*/

#endif /* #ifndef PLATFORM_H */
