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


/*----- ALL THE DEFAULT NODES ARE DISABLED BY DEFAULT -----------------------------------------------------------------*/
#undef STREAM_PLATFORM_SERVICES      /* call the platform service with its fast libraries w/wo accelerators */
#undef CODE_ARM_STREAM_SCRIPT        /*  1 byte-code interpreter, index "arm_stream_script_INDEX" */
#undef CODE_ARM_STREAM_ROUTER        /*  2 copy input arcs and subchannel and output arcs and subchannels   */     
#undef CODE_ARM_STREAM_AMPLIFIER     /*  3 amplifier rescaler mute and un-mute with ramp and delay control */
#undef CODE_ARM_STREAM_FILTER        /*  4 cascade of DF1 filters */
#undef CODE_ARM_STREAM_MODULATOR     /*  5 signal generator with modulation */
#undef CODE_ARM_STREAM_DEMODULATOR   /*  6 signal demodulator, frequency estimator */
#undef CODE_ARM_STREAM_FILTER2D      /*  7 Filter, rescale, rotate, exposure compensation */
#undef CODE_ARM_ANR                  /*  8 audio noise reduction */
#undef CODE_ARM_FIXEDBF              /*  9 audio fixed direction beamformer */
#undef CODE_SIGP_KWS                 /* 10 keyword spotting yes/no */
#undef CODE_SIGP_STREAM_DETECTOR     /* 11 estimates peaks/floor of the mono input and triggers a flag on high SNR */
#undef CODE_SIGP_STREAM_DETECTOR2D   /* 12 activity detection, pattern detection */
#undef CODE_SIGP_STREAM_RESAMPLER    /* 13 asynchronous sample-rate converter */
#undef CODE_SIGP_STREAM_COMPRESSOR   /* 14 raw data compression with adaptive prediction */
#undef CODE_SIGP_STREAM_DECOMPRESSOR /* 15 raw data decompression */
#undef CODE_BITBANK_JPEGENC          /* 16 bitbank_JPEGENC */
#undef CODE_TJPGDEC                  /* 17 TjpgDec */

/*----- PLATFORM CHARACTERISTICS  ------------------------------------------------------------------------*/

#undef STREAM_FLOAT_ALLOWED         /* Floating point library allowed */
#undef NB_NODE_ENTRY_POINTS         /* max number of nodes installed at compilation time */
#undef MAX_NB_APP_CALLBACKS         /* max number of application callbacks used from NODE and scripts */
#undef MULTIPROCESSING              /* single processor by default */

/*----- PLATFORM DEFAULT CALLBACKS ------------------------------------------------------------------------*/
#undef CALLBACK_USE_CASE_CONTROL    // S->application_callbacks[USE_CASE_CONTROL] 






/*----- PLATFORM SELECTION ------------------------------------------------------------------------------*/
//#define PLATFORM_ALIF1      
//#define PLATFORM_COMPUTER               //Windows 
#define PLATFORM_RA8E1FPB               //M85 
//#define PLATFORM_RENESAS_RA8P1EK        //M85+M33
//#define PLATFORM_LPC55S69EVK            //Dual M33
//#define PLATFORM_STM32C0116DK           //M0 2kB

//------------------------------------------------------------------------------------
#ifdef PLATFORM_ALIF1
#define PLATFORM_ARCHITECTURE 1         /* arbitrary index */
#define PLATFORM_ARCH_32BIT             /* or PLATFORM_ARCH_64BIT */

#define PLATFORM_PROCESSOR 1            /* HE=1 orocessor */
//#define PLATFORM_PROCESSOR 2            /* HP=2 processor */
#include "alif1/top_manifest_alif1.h"
#endif

//------------------------------------------------------------------------------------
#ifdef PLATFORM_RA8E1FPB
#define PLATFORM_ARCHITECTURE 1         /* arbitrary index */
#define PLATFORM_ARCH_32BIT             /* or PLATFORM_ARCH_64BIT */

#define PLATFORM_PROCESSOR 1            /* HE=1 orocessor */
#include "ra8e1/top_manifest_ra8e1.h"
#endif
//------------------------------------------------------------------------------------

#ifdef PLATFORM_COMPUTER
#define PLATFORM_ARCH_32BIT           
//#define PLATFORM_ARCH_64BIT         
#define PLATFORM_PROCESSOR 1            
#define PLATFORM_ARCHITECTURE 1         /* arbitrary index */
#include "computer/top_manifest_computer.h"
#endif


//--------PLATFORMS TO BE SUPPORTED-------------------------------------------------------------
//#include "platform_ra8e1sdk.h"
//#include "platform_lpc55s69evk.h"
//#define PLATFORM_MSPM0L1306             //SYSOSC 24MHz/320 = FS 75kHz
//#include "platform_lp-mspm0l1306.h"
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



/*----- ALL THE SERVICES ARE DISABLED BY DEFAULT   EXCEPT A SHORT LIST ALWAYS PRESENT  -----------*/
//                          index mask
// SERV_GROUP_INTERNAL    /* 0    1  internal : Semaphores, DMA, Clocks */
// SERV_GROUP_SCRIPT      /* 1    2  script : Node parameters  */
// SERV_GROUP_STDLIB      /* 2    4  stdlib,, string, malloc */
// SERV_GROUP_MATH        /* 3    8  math.h */
// SERV_GROUP_DSP_ML      /* 4   16  cmsis-dsp */
// SERV_GROUP_DEEPL       /* 5   32  cmsis-nn */



//    // SERV_GROUP_INTERNAL                /* 0  internal : Semaphores, DMA, Clocks */
#define SERV_INTERNAL_SLEEP_CONTROL                          0
#define SERV_INTERNAL_CPU_CLOCK_UPDATE                       1
#define SERV_INTERNAL_READ_MEMORY                            2
#define SERV_INTERNAL_READ_MEMORY_FAST_MEM_ADDRESS           3
#define SERV_INTERNAL_SERIAL_COMMUNICATION                   4
#define SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP  5
#define SERV_INTERNAL_MUTUAL_EXCLUSION_RD_BYTE_MP            6
#define SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP            7
#define SERV_INTERNAL_MUTUAL_EXCLUSION_CLEAR_BIT_MP          8
#define SERV_INTERNAL_INTERNAL_READ_TIME                     9
#define SERV_INTERNAL_READ_TIME64                           10
#define SERV_INTERNAL_READ_TIME32                           11
#define SERV_INTERNAL_READ_TIME16                           12
#define SERV_INTERNAL_KEYEXCHANGE                           13

//    SERV_GROUP_SCRIPT                     /* 1  script, set of node parameters  */
        
//    SERV_GROUP_STDLIB                     /* 2  stdlib,, string, malloc */
        
//    SERV_GROUP_MATH                       /* 3  math.h */
        
//    SERV_GROUP_DSP_ML                     /* 4  cmsis-dsp */

    //#define SERV_DSP_CASCADE_DF1_Q15     /* IIR filters, use SERV_CHECK_COPROCESSOR */
    //use #define PLATFORM_SERV_DSP_CASCADE_DF1_Q15 for specific implmentations
    typedef struct
    {       int16_t *pState;      
      const int16_t *pCoeffs;     
            int8_t numStages;     
            int8_t postShift;     
    }  generic_biquad_cascade_df1_inst_q15;        

    typedef void    (df1_q15_init) (
        generic_biquad_cascade_df1_inst_q15 * S,
            uint8_t numStages,
      const int16_t * pCoeffs,
            int16_t * pState,
            int8_t postShift );

    typedef void    (df1_q15) (         /* platform-accelerated DF1 will use this template */
      const generic_biquad_cascade_df1_inst_q15 * S,
      const int16_t * pSrc,
            int16_t * pDst,
            uint32_t blockSize );

    //#define SERV_DSP_CASCADE_DF1_F32            
    //use #define PLATFORM_SERV_DSP_CASCADE_DF1_F32 for specific implmentations
    typedef struct
    {       uint32_t numStages;    
            float *pState;         
      const float *pCoeffs;        
    } generic_biquad_cascade_df1_inst_f32;

    typedef void (df1_init_f32) (
        generic_biquad_cascade_df1_inst_f32 * S,
          uint8_t numStages,
    const float * pCoeffs,
          float * pState);

    typedef void (df1_f32) (        /* platform-accelerated DF1 will use this template */
    const generic_biquad_cascade_df1_inst_f32 * S,
    const float * pSrc,
          float * pDst,
          uint32_t blockSize);


    //#define SERV_DSP_WINDOW                
    //#define SERV_DSP_WINDOW_DB             
    //#define SERV_DSP_rFFT_Q15           /* RFFT windowing, module, dB , use SERV_CHECK_COPROCESSOR */
    //#define SERV_DSP_rFFT_F32           /* default FFT with tables rebuilded */
    //#define SERV_DSP_cFFT_Q15           /* cFFT windowing, module, dB */
    //#define SERV_DSP_cFFT_F32           

// SERV_GROUP_DEEPL              /* 5  cmsis-nn */
    //#define SERV_ML_FC                  /* fully connected layer Mat x Vec */
    //#define SERV_ML_CNN                 /* convolutional NN : 3x3 5x5 fixed-weights */



/*---------------------------------------------------------------------------------------------------------------------*/

#endif
