/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        platform_computer.h
 * Description:  constants specific to this platform
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

#include <stdint.h>

#ifndef cPLATFORM_H
#define cPLATFORM_H


/*----- ALL THE NODES ARE DISABLED BY DEFAULT ------------------------------------------------------------------------*/
#undef STREAM_PLATFORM_SERVICES      /* call the platform service with its fast libraries w/wo accelerators */
#undef CODE_ARM_STREAM_SCRIPT        /* byte-code interpreter, index "arm_stream_script_INDEX" */
#undef CODE_ARM_STREAM_ROUTER        /* copy input arcs and subchannel and output arcs and subchannels   */     
#undef CODE_ARM_STREAM_AMPLIFIER     /* amplifier rescaler mute and un-mute with ramp and delay control */
#undef CODE_ARM_STREAM_FILTER        /* cascade of DF1 filters */
#undef CODE_ARM_STREAM_MODULATOR     /* signal generator with modulation */
#undef CODE_ARM_STREAM_DEMODULATOR   /* signal demodulator, frequency estimator */
#undef CODE_ARM_STREAM_FILTER2D      /* Filter, rescale, rotate, exposure compensation */
#undef CODE_SIGP_STREAM_DETECTOR     /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
#undef CODE_SIGP_STREAM_DETECTOR2D   /* activity detection, pattern detection */
#undef CODE_SIGP_STREAM_RESAMPLER    /* asynchronous sample-rate converter */
#undef CODE_SIGP_STREAM_COMPRESSOR   /* raw data compression with adaptive prediction */
#undef CODE_SIGP_STREAM_DECOMPRESSOR /* raw data decompression */
#undef CODE_BITBANK_JPEGENC          /* bitbank_JPEGENC */
#undef CODE_TJPGDEC                  /* TjpgDec */

/*----- ALL THE SERVICES ARE DISABLED BY DEFAULT ------------------------------------------------------------------------*/
//    // SERV_GROUP_INTERNAL           /* 0  internal : Semaphores, DMA, Clocks */
//#define SERV_INTERNAL_SLEEP_CONTROL                           0
//#define SERV_INTERNAL_CPU_CLOCK_UPDATE                        1
//#define SERV_INTERNAL_READ_MEMORY                             2
//#define SERV_INTERNAL_READ_MEMORY_FAST_MEM_ADDRESS            3
//#define SERV_INTERNAL_SERIAL_COMMUNICATION                    4
#define SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP   5
#define SERV_INTERNAL_MUTUAL_EXCLUSION_RD_BYTE_MP             6
#define SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP             7
#define SERV_INTERNAL_MUTUAL_EXCLUSION_CLEAR_BIT_MP           8
//#define PLATFORM_SERV_INTERNAL_READ_TIME                      9
//#define SERV_INTERNAL_READ_TIME64                            10
//#define SERV_INTERNAL_READ_TIME32                            11
//#define SERV_INTERNAL_READ_TIME16                            12
#define SERV_INTERNAL_KEYEXCHANGE                            13
//
//    // SERV_GROUP_SCRIPT             /* 1  script : Node parameters  */
//
//    // SERV_GROUP_CONVERSION         /* 2  Compute : raw conversions */
//#define SERV_CONVERSION_INT16_FP32
//
//    // SERV_GROUP_STDLIB             /* 3  Compute : malloc, string */
//#define SERV_STDLIB_ATOF    
//#define SERV_STDLIB_ATOI    
//#define SERV_STDLIB_MEMSET  
//#define SERV_STDLIB_STRCHR  
//#define SERV_STDLIB_STRLEN  
//#define SERV_STDLIB_STRNCAT 
//#define SERV_STDLIB_STRNCMP 
//#define SERV_STDLIB_STRNCPY 
//#define SERV_STDLIB_STRSTR  
//#define SERV_STDLIB_STRTOK  
//#define SERV_STDLIB_FREE    
//#define SERV_STDLIB_MALLOC  
//
//    // SERV_GROUP_MATH               /* 4  math.h */
//#define SERV_MATH_SQRT_Q15  
//#define SERV_MATH_SQRT_F32  
//#define SERV_MATH_LOG_Q15   
//#define SERV_MATH_LOG_F32   
//#define SERV_MATH_SINE_Q15  
//#define SERV_MATH_SINE_F32  
//#define SERV_MATH_COS_Q15   
//#define SERV_MATH_COS_F32   
//#define SERV_MATH_ATAN2_Q15 
//#define SERV_MATH_ATAN2_F32 
//#define SERV_MATH_SORT      
//
//    // SERV_GROUP_DSP_ML             /* 5  cmsis-dsp */
//#define SERV_DSP_CHECK_COPROCESSOR   /* check for services() */
//#define SERV_DSP_CHECK_END_COMP      /* check completion for the caller */
//#define SERV_DSP_DFT_Q15             /* DFT/Goertzel windowing, module, dB */
//#define SERV_DSP_DFT_F32            



//#define SERV_DSP_CASCADE_DF1_Q15     /* IIR filters, use SERV_CHECK_COPROCESSOR */
    typedef struct
    {       int16_t *pState;      
      const int16_t *pCoeffs;     
            int8_t numStages;     
            int8_t postShift;     
    }  biquad_cascade_df1_inst_q15;        

    typedef void    (df1_q15_init) (
            biquad_cascade_df1_inst_q15 * S,
            uint8_t numStages,
      const int16_t * pCoeffs,
            int16_t * pState,
            int8_t postShift );

    typedef void    (df1_q15) (         /* platform-accelerated DF1 will use this template */
      const biquad_cascade_df1_inst_q15 * S,
      const int16_t * pSrc,
            int16_t * pDst,
            uint32_t blockSize );

//#define SERV_DSP_CASCADE_DF1_F32            
    typedef struct
    {
            uint32_t numStages;    
            float *pState;         
      const float *pCoeffs;        
    } biquad_cascade_df1_inst_f32;

    typedef void (df1_init_f32) (
          biquad_cascade_df1_inst_f32 * S,
          uint8_t numStages,
    const float * pCoeffs,
          float * pState);

    typedef void (df1_f32) (        /* platform-accelerated DF1 will use this template */
    const biquad_cascade_df1_inst_f32 * S,
    const float * pSrc,
          float * pDst,
          uint32_t blockSize);



//#define SERV_DSP_WINDOW                
//#define SERV_DSP_WINDOW_DB             
//#define SERV_DSP_rFFT_Q15           /* RFFT windowing, module, dB , use SERV_CHECK_COPROCESSOR */
//#define SERV_DSP_rFFT_F32           /* default FFT with tables rebuilded */
//#define SERV_DSP_cFFT_Q15           /* cFFT windowing, module, dB */
//#define SERV_DSP_cFFT_F32           
//
//    // SERV_GROUP_DEEPL             /* 6  cmsis-nn */
//#define SERV_ML_FC                  /* fully connected layer Mat x Vec */
//#define SERV_ML_CNN                 /* convolutional NN : 3x3 5x5 fixed-weights */
//
//    // SERV_GROUP_MM_AUDIO           /* 7 speech/audio processing */
//
//    // SERV_GROUP_MM_IMAGE           /* 8 image processing */

/*----- PLATFORM CHARACTERISTICS  ------------------------------------------------------------------------*/

#undef STREAM_FLOAT_ALLOWED         /*------ Floating point allowed ------*/
#undef NB_NODE_ENTRY_POINTS         /* max number of nodes installed at compilation time */
#undef MAX_NB_APP_CALLBACKS         /* max number of application callbacks used from NODE and scripts */
#undef MULTIPROCESSING 
#undef GRAPH_FROM_APP1PLATFORM2     /* the graph comes either from the "main" or from the platform */


/*----- PLATFORM DEFAULT CALLBACKS ------------------------------------------------------------------------*/

#define CALLBACK_USE_CASE_CONTROL 0 // S->application_callbacks[USE_CASE_CONTROL] 


/*----- PLATFORM SELECTION ------------------------------------------------------------------------------*/

#define PLATFORM_COMPUTER                 //Windows 32/64bits
    #define PLATFORM_ARCH_32BIT
    //#define PLATFORM_ARCH_64BIT
#include "platform_computer.h"

//------------------------------------------------------------------------------------
//#define PLATFORM_RENESAS_RA8E1SDK       //M85 
//#include "platform_ra8e1sdk.h"

//------------------------------------------------------------------------------------
//#define PLATFORM_LPC55S69EVK            //Dual M33
//#include "platform_lpc55s69evk.h"

//------------------------------------------------------------------------------------
//#define PLATFORM_MSPM0L1306             //SYSOSC 24MHz/320 = FS 75kHz
//#include "platform_lp-mspm0l1306.h"

//------------------------------------------------------------------------------------
//#define PLATFORM_WIOTERMINAL            //<<<<
//#define PLATFORM_MCXC041                //2K RAM IMU temp LED
//#define PLATFORM_MCXN236                //DMIC, audio, dual M33, cry  pto, display
//#define PLATFORM_CY8CKIT_062S2          //M4
//#define PLATFORM_CY8CKIT_E84            //M55
//#define PLATFORM_GROVEVISIONAIV2        //
//#define PLATFORM_EFM32PG32              //Simplicity Studio
//#define PLATFORM_RP2040                 //<<<<
//#define PLATFORM_RP2350                 //<<<<
//#define PLATFORM_STM32U083DK            //<<<<
//#define PLATFORM_STM32C0116DK           //PACK
//#define PLATFORM_STM32L475EIOT01        //---
//#define PLATFORM_STM32N6SDK             //M55
//#define PLATFORM_STM32U585IOT02A        //PACK    Light,TOF,Pressure,IMU,Temp,MIC
//#define PLATFORM_STEVALMKBOXPRO         //CubeIDE Pressure,IMU,Temp,MIC
//#define PLATFORM_NUVOTON_M2003          //M0 24MHz, SAR, PWM, 4kB/32kB
//#define PLATFORM_RENESAS_RA4M1          //M4 48MHz, 16kB/256kB
//#define PLATFORM_ARDU_NANO33SENSEV2     //Arduino 
//#define PLATFORM_ARDU_RP2040            //Arduino IMU,MIC
//#define PLATFORM_ARDU_UNOR4W            //Arduino 
//#define PLATFORM_ARDU_ZERO              //Arduino 
//#define PLATFORM_PORTENTAC33            //Arduino SE050C2 Secure Element

/*---------------------------------------------------------------------------------------------------------------------*/

#endif /* ifndef cPLATFORM_H */
