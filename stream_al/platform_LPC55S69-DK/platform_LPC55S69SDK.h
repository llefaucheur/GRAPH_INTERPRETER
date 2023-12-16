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
#ifndef PLATFORM_COMPUTER_H
#define PLATFORM_COMPUTER_H

#define PLATFORM_COMPUTER 1

#ifdef _MSC_VER 
#include <stdio.h>
#endif


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
#else
#define intPtr_t uint64_t 
#endif


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
 



#ifdef _MSC_VER 
#define RUN_ON_COMPUTER_ 1
#else
#define RUN_ON_COMPUTER_ 0
#endif

/*
 * --- maximum number of processors using STREAM in parallel - read by the graph compiler
 */
#define STREAM_NB_PROCESSES 1      /* Max number of process(or) reeading the graph simultaneously */

#define STREAM_MAXNB_THREADS_PER_PROC 1
#define STREAM_MAXNB_PROCESSOR_PER_ARCH 1
#define STREAM_NB_ARCHITECTURES 1


/*
    list of streams available on this platform 
    (see also stream_types.h : enum stream_io_domain)

    this data is reported to "stream_tools_files_manifests_XXXXXX.txt" :

   c;   stream_al\platform_imu.txt          
   c;   stream_al\platform_microphone.txt   
   c;   stream_al\platform_line_in.txt   
   c;   stream_al\platform_gpio0.txt   
   c;   stream_al\platform_gpio1.txt  
*/ 

/* replicated fw_io_dx : stream_al/platform_computer.h <=> stream_tools/files_manifests_computer.txt */
#define IO_PLATFORM_STREAM_IN_0      1       /* interface to the application processor see stream_al\io_platform_stream_in_0 */
#define IO_PLATFORM_IMU_0            2       /* 3D motion sensor see stream_al\io_platform_imu_0 */
#define IO_PLATFORM_MICROPHONE_0     3       /* audio in mono see stream_al\io_platform_microphone_0.txt  */
#define IO_PLATFORM_LINE_IN_0        4       /* audio in stereo  stream_al\io_platform_line_in_0.txt     */
#define IO_PLATFORM_LINE_OUT_0       5       /* audio out stereo stream_al\io_platform_line_out_0.txt    */
#define IO_PLATFORM_ANALOG_SENSOR_0  6       /* analog converter stream_al\io_platform_analog_sensor_0.txt */
#define IO_PLATFORM_GPIO_OUT_0       7       /* PWM              stream_al\io_platform_gpio_out_0.txt    */
#define IO_PLATFORM_GPIO_OUT_1       8       /* LED              stream_al\io_platform_gpio_out_1.txt    */
#define IO_PLATFORM_DATA_IN_0        9       /* shock detector   stream_al\io_platform_imu_metadata_0.txt */
#define IO_PLATFORM_DATA_IN_1       10       /* temp. sensor     stream_al\io_platform_imu_temperature_0.txt */

#define LAST_IO_FUNCTION_PLATFORM (IO_PLATFORM_DATA_IN_1+1) /* table of platform_io[fw_io_idx] */
#define MAX_IO_FUNCTION_PLATFORM 32                         /* table of platform_io[fw_io_idx] MAX 32 IOs  later 255 */

/*===========================================================================
 in platform_computer.c : 
    void platform_specific_long_offset(intPtr_t long_offset[])
        long_offset[MBANK_GRAPH]    = (const intPtr_t)&(MEXT[10]); 
        long_offset[MBANK_DMEMFAST] = (const intPtr_t)&(TCM1[10]); 
*/
#define MAX_NB_MEMORY_OFFSET 2 /* 2 memory banks offsets */


#endif /* #ifndef PLATFORM_COMPUTER_H */


/*===========================================================================*/
/*
 * --- platform_io -------------------------------------------------------------
 */
//enum enum_io_state {
//#define IO_NOT_INITIALIZED 1
//#define IO_STATE_INITIALIZED 2  
//#define IO_STATE_IDLE 3
//#define IO_STATE_RUN 4
//#define IO_STATE_STOPPED 5
//};
//#define PLATFORM_NB_IO_DOMAIN 2                 /* APP and audio out */
