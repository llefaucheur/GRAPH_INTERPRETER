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

#ifndef cPLATFORM_COMPUTER_H
#define cPLATFORM_COMPUTER_H

/* conditional compilation */
#define STREAM_PLATFORM_SERVICES        /* call the platform service with its fast libraries w/wo accelerators */

#define CODE_ARM_STREAM_SCRIPT          /* byte-code interpreter, index "arm_stream_script_INDEX" */
#define CODE_ARM_STREAM_router          /* copy input arcs and subchannel and output arcs and subchannels   */     
//#define CODE_ARM_STREAM_AMPLIFIER       /* amplifier mute and un-mute with ramp and delay control */
#define CODE_ARM_STREAM_FILTER          /* cascade of DF1 filters */
//#define CODE_ARM_STREAM_MODULATOR       /* signal generator with modulation */
//#define CODE_ARM_STREAM_DEMODULATOR     /* signal demodulator, frequency estimator */
//#define CODE_ARM_STREAM_FILTER2D        /* Filter, rescale, rotate, exposure compensation */
#define CODE_SIGP_STREAM_DETECTOR       /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
//#define CODE_SIGP_STREAM_DETECTOR2D     /* activity detection, pattern detection */
//#define CODE_SIGP_STREAM_RESAMPLER      /* asynchronous sample-rate converter */
#define CODE_SIGP_STREAM_COMPRESSOR     /* raw data compression with adaptive prediction */
#define CODE_SIGP_STREAM_DECOMPRESSOR   /* raw data decompression */
//#define CODE_BITBANK_JPEGENC            /* bitbank_JPEGENC */
//#define CODE_TJPGDEC                    /* TjpgDec */


/*------ Floating point allowed ------*/
#define STREAM_FLOAT_ALLOWED 1

/* max number of nodes installed at compilation time */
#define NB_NODE_ENTRY_POINTS 30

/* max number of application callbacks used from NODE and scripts */
#define MAX_NB_APP_CALLBACKS 8

/* code size saving switch */
#define STREAM_SERVICE_LOW_MEMORY 1 /* FFT tables are computed */

#define STREAM_SERVICE_EXTSTDLIB 0  
#define STREAM_SERVICE_EXTMATH   0
#define STREAM_SERVICE_EXTDSPML  0
#define STREAM_SERVICE_EXTAUDIO  0
#define STREAM_SERVICE_EXTIMAGE  0

// to test the multiprocessing macro
#define MULTIPROCESSING 1     

#ifdef _MSC_VER 
#define DATA_MEMORY_BARRIER
#define INSTRUCTION_SYNC_BARRIER
#else
#if MULTIPROCESSING == 1
#define DATA_MEMORY_BARRIER //DMB()
#define INSTRUCTION_SYNC_BARRIER //ISB()
#else
#define DATA_MEMORY_BARRIER 
#define INSTRUCTION_SYNC_BARRIER
#endif
#endif

#define WR_BYTE_MP_(address,x) { *(volatile uint8_t *)(address) = (x); DATA_MEMORY_BARRIER; }
#define RD_BYTE_MP_(x,address) { DATA_MEMORY_BARRIER; (x) = *(volatile uint8_t *)(address);}
#define CLEAR_BIT_MP(arg, bit) {((arg) = U(arg) & U(~(U(1) << U(bit)))); DATA_MEMORY_BARRIER; } 



/*----------------------------------------PLATFORM MANIFEST--------------------------------------------
                  +-----------------+
                  | static |working |
   +--------------------------------+
   |external RAM  |        |        |
   +--------------------------------+
   |internal RAM  |        |        |
   +--------------------------------+
   |fast TCM      |  N/A   |        |
   +--------------+--------+--------+
*/
// MEMORY BANKS

#define PROC_ID 1                       /* must be >0 */
#define ARCH_ID 0 

#define MBANK_GRAPH     0               /* graph base address (shared) */
#define MBANK_DMEMFAST  1               /* not shared DTCM Cortex-M/LLRAM Cortex-R, swapped between NODE calls if static */
#define SIZE_MBANK_DMEM_EXT    0x1000   /* 4K external (buffers) */
#define SIZE_MBANK_BACKUPMEM1   0x20    /* OFFSET 1 */
#define SIZE_MBANK_TCM2         0x20    /* TCM (fast RAM) */
#define SIZE_MBANK_FLASH3       0x20    /* FLASH */


#ifdef _MSC_VER 
#define RUN_ON_COMPUTER_ 1
#else
#define RUN_ON_COMPUTER_ 0
#endif

/*
 * --- maximum number of processors using STREAM in parallel - read by the graph compiler
 */
#define STREAM_NB_PROCESSES 1           /* Max number of process(or) reeading the graph simultaneously */

#define STREAM_MAXNB_THREADS_PER_PROC 1
#define STREAM_MAXNB_PROCESSOR_PER_ARCH 1
#define STREAM_NB_ARCHITECTURES 1

        /* WARNING : changing the indexes impacts the "top_graph_interface" of each graph.txt */
#define IO_PLATFORM_DATA_IN_0        0 
#define IO_PLATFORM_DATA_IN_1        1 
#define IO_PLATFORM_ANALOG_SENSOR_0  2 
#define IO_PLATFORM_MOTION_IN_0      3 
#define IO_PLATFORM_AUDIO_IN_0       4 
#define IO_PLATFORM_2D_IN_0          5 
#define IO_PLATFORM_LINE_OUT_0       6 
#define IO_PLATFORM_GPIO_OUT_0       7 
#define IO_PLATFORM_GPIO_OUT_1       8 
#define IO_PLATFORM_DATA_OUT_0       9


#define LAST_IO_FUNCTION_PLATFORM (IO_PLATFORM_DATA_OUT_0+1)  /* table of platform_io[io_al_idx] */

//#define MAX_IO_FUNCTION_PLATFORM 128     /* table of platform_io[io_al_idx] */

/*===========================================================================
 in platform_computer.c : 
    void platform_specific_long_offset(intptr_t long_offset[])
        long_offset[MBANK_GRAPH]    = (const intptr_t)&(MEXT[10]); 
        long_offset[MBANK_DMEMFAST] = (const intptr_t)&(TCM1[10]); 
*/
#define MAX_NB_MEMORY_OFFSET 4
#define CRITICAL_FAST_SEGMENT_IDX (MAX_NB_MEMORY_OFFSET-1)


/* memory consumption optimization */
#define MAX_NB_STREAM_PER_NODE 16u 

#endif /* ifndef cPLATFORM_COMPUTER_H */