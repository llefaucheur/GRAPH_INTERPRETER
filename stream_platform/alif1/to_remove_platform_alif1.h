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

#ifndef cPLATFORM_ALIF1_H
#define cPLATFORM_ALIF1_H

/*----- NODES ENABLED FOR "COMPUTER" ------------------------------------------------------------------------*/
#if PLATFORM_PROCESSOR == 1 || PLATFORM_PROCESSOR == 2
#define CODE_ARM_STREAM_SCRIPT          /* byte-code interpreter, index "arm_stream_script_INDEX" */
#define CODE_ARM_STREAM_FILTER          /* cascade of DF1 filters */
#define CODE_SIGP_STREAM_DETECTOR       /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
#endif

#if PLATFORM_PROCESSOR == 1
#define CODE_ARM_STREAM_ROUTER          /* copy input arcs and subchannel and output arcs and subchannels   */     
#define CODE_ARM_ANR                    /* SDS audio noise reduction */
#define CODE_ARM_FIXEDBF                /* SDS audio fixed direction beamformer */
#define CODE_ARM_KWS                    /* SDS keyword spotting */
#define CODE_SIGP_STREAM_COMPRESSOR     /* raw data compression with adaptive prediction */
#define CODE_SIGP_STREAM_DECOMPRESSOR   /* raw data decompression */
#endif

#if PLATFORM_PROCESSOR == 2
//#define CODE_SIGP_STREAM_DETECTOR2D     /* activity detection, pattern detection */
//#define CODE_ARM_STREAM_FILTER2D        /* Filter, rescale, rotate, exposure compensation */
//#define CODE_ARM_STREAM_ANALYSIS        /* arm_stream_analysis, */
//#define CODE_BITBANK_JPEGENC            /* bitbank_JPEGENC */
//#define CODE_TJPGDEC                    /* TjpgDec */
#endif


/*----- SERVICES ENABLED FOR "ALIF1" ------------------------------------------------------------------------*/
// SERV_GROUP_INTERNAL           /* 0  internal : Semaphores, DMA, Clocks */
    //#define PLATFORM_SERV_INTERNAL_SLEEP_CONTROL                              0
    //#define PLATFORM_SERV_INTERNAL_CPU_CLOCK_UPDATE                           1
    //#define PLATFORM_SERV_INTERNAL_READ_MEMORY                                2
    //#define PLATFORM_SERV_INTERNAL_READ_MEMORY_FAST_MEM_ADDRESS               3
    //#define PLATFORM_SERV_INTERNAL_SERIAL_COMMUNICATION                       4
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP      5
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_RD_BYTE_MP                6
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP                7
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_CLEAR_BIT_MP              8
    //#define PLATFORM_SERV_INTERNAL_READ_TIME                                  9
    //#define PLATFORM_SERV_INTERNAL_READ_TIME64                               10
    //#define PLATFORM_SERV_INTERNAL_READ_TIME32                               11
    //#define PLATFORM_SERV_INTERNAL_READ_TIME16                               12
    //#define PLATFORM_SERV_INTERNAL_KEYEXCHANGE                               13

// SERV_GROUP_SCRIPT             /* 1  script : Node parameters  */

//#undef SERV_GROUP_CONVERSION         /* 2  Compute : raw conversions */
    //#undef PLATFORM_SERV_SERV_CONVERSION_INT16_FP32

//#undef SERV_GROUP_STDLIB             /* 3  Compute : malloc, string */
    //#undef PLATFORM_SERV_STREAM_ATOF    
    //#undef PLATFORM_SERV_STREAM_ATOI    
    //#undef PLATFORM_SERV_STREAM_MEMSET  
    //#undef PLATFORM_SERV_STREAM_STRCHR  
    //#undef PLATFORM_SERV_STREAM_STRLEN  
    //#undef PLATFORM_SERV_STREAM_STRNCAT 
    //#undef PLATFORM_SERV_STREAM_STRNCMP 
    //#undef PLATFORM_SERV_STREAM_STRNCPY 
    //#undef PLATFORM_SERV_STREAM_STRSTR  
    //#undef PLATFORM_SERV_STREAM_STRTOK  
    //#undef PLATFORM_SERV_STREAM_FREE    
    //#undef PLATFORM_SERV_STREAM_MALLOC  

//#undef SERV_GROUP_MATH               /* 4  math.h */
    //#undef PLATFORM_SERV_SERV_SQRT_Q15  
    //#undef PLATFORM_SERV_SERV_SQRT_F32  
    //#undef PLATFORM_SERV_SERV_LOG_Q15   
    //#undef PLATFORM_SERV_SERV_LOG_F32   
    //#undef PLATFORM_SERV_SERV_SINE_Q15  
    //#undef PLATFORM_SERV_SERV_SINE_F32  
    //#undef PLATFORM_SERV_SERV_COS_Q15   
    //#undef PLATFORM_SERV_SERV_COS_F32   
    //#undef PLATFORM_SERV_SERV_ATAN2_Q15 
    //#undef PLATFORM_SERV_SERV_ATAN2_F32 
    //#undef PLATFORM_SERV_SERV_SORT      

// SERV_GROUP_DSP_ML             /* 5  cmsis-dsp */
    //#undef PLATFORM_SERV_CHECK_COPROCESSOR    /* check for services() */
    //#undef PLATFORM_SERV_SERV_CHECK_END_COMP       /* check completion for the caller */
    //#undef PLATFORM_SERV_SERV_DFT_Q15              /* DFT/Goertzel windowing, module, dB */
    //#undef PLATFORM_SERV_SERV_DFT_F32            

    #define PLATFORM_SERV_CASCADE_DF1_Q15           /* IIR filters, use SERV_CHECK_COPROCESSOR */
    //#define PLATFORM_SERV_CASCADE_DF1_F32         /* take the default implementation */

    //#undef PLATFORM_SERV_SERV_WINDOW                
    //#undef PLATFORM_SERV_SERV_WINDOW_DB             
    //#undef PLATFORM_SERV_SERV_rFFT_Q15             /* RFFT windowing, module, dB , use SERV_CHECK_COPROCESSOR */
    //#undef PLATFORM_SERV_SERV_rFFT_F32             /* default FFT with tables rebuilded */
    //#undef PLATFORM_SERV_SERV_cFFT_Q15             /* cFFT windowing, module, dB */
    //#undef PLATFORM_SERV_SERV_cFFT_F32           

//#undef SERV_GROUP_DEEPL              /* 6  cmsis-nn */
    //#undef PLATFORM_SERV_STREAM_FC                 /* fully connected layer Mat x Vec */
    //#undef PLATFORM_SERV_STREAM_CNN                /* convolutional NN : 3x3 5x5 fixed-weights */

//#undef PLATFORM_SERV_SERV_GROUP_MM_AUDIO           /* 7 speech/audio processing */

//#undef PLATFORM_SERV_SERV_GROUP_MM_IMAGE           /* 8 image processing */


///* conditional compilation */
//#define STREAM_PLATFORM_SERVICES        /* call the platform service with its fast libraries w/wo accelerators */
    //#undef PLATFORM_SERV_SERV_STDLIB
    //#undef PLATFORM_SERV_SERV_EXTMATH
    //#undef PLATFORM_SERV_SERV_EXTAUDIO
    //#undef PLATFORM_SERV_SERV_EXTIMAGE


/*----- PLATFORM DEFINITIONS FOR "ALIF1" ------------------------------------------------------------------------*/

#define GRAPH_FROM_PLATFORM   2         /* the graph comes from the platform */

/*------ Floating point allowed ------*/
#define STREAM_FLOAT_ALLOWED 1          /* selection of integer processing with float_t and double_t */

/* max number of nodes installed at compilation time */
#define NB_NODE_ENTRY_POINTS 30

/* max number of application callbacks used from NODE and scripts */
#define NB_APP_CALLBACKS 8

// to test the multiprocessing macro
#define MULTIPROCESSING 1     

#ifdef _MSC_VER 
#define DATA_MEMORY_BARRIER         /* */ 
#define INSTRUCTION_SYNC_BARRIER    /* */ 
#else
#if MULTIPROCESSING == 1
#define DATA_MEMORY_BARRIER /* DMB() */
#define INSTRUCTION_SYNC_BARRIER /* ISB() */
#else
#define DATA_MEMORY_BARRIER 
#define INSTRUCTION_SYNC_BARRIER
#endif
#endif

#define WR_BYTE_MP_(address,x) { *(volatile uint8_t *)(address) = (x); DATA_MEMORY_BARRIER; }
#define RD_BYTE_MP_(x,address) { DATA_MEMORY_BARRIER; (x) = *(volatile uint8_t *)(address);}
#define CLEAR_BIT_MP(arg, bit) {((arg) = U(arg) & U(~(U(1) << U(bit)))); DATA_MEMORY_BARRIER; } 


/*
 * --- maximum number of processors using STREAM in parallel - read by the graph compiler
 */
#define STREAM_NB_PROCESSES 1           /* Max number of process(or) reeading the graph simultaneously */

#define STREAM_MAXNB_THREADS_PER_PROC 1
#define STREAM_MAXNB_PROCESSOR_PER_ARCH 2
#define STREAM_NB_ARCHITECTURES 1



#endif /* ifndef cPLATFORM_ALIF1_H */