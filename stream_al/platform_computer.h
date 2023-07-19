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
#include <stdint.h>

#define PLATFORM_COMPUTER 1

#ifdef _MSC_VER 
#include <stdio.h>
#endif


/*============================ SWITCHES =====================================*/
//#define PLATFORM_ARCH_32BIT
#define PLATFORM_ARCH_64BIT
#define MULTIPROCESSING 1     

#ifdef _MSC_VER 
#define RUN_ON_COMPUTER_ 1
#else
#define RUN_ON_COMPUTER_ 0
#endif
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

/*
 * --- platform specific  -------------------------------------------------------
 */
#define PLATFORM_NB_IO_DOMAIN 2                 /* APP and audio out */

/*
 * --- maximum number of processors using STREAM in parallel - read by the graph compiler
 */
#define STREAM_NB_PROCESSES 1      /* Max number of process(or) reeading the graph simultaneously */

#define STREAM_MAXNB_THREADS_PER_PROC 1
#define STREAM_MAXNB_PROCESSOR_PER_ARCH 1
#define STREAM_NB_ARCHITECTURES 1


/*
   list of IO ports for the graph
   see stream_local_instance : whoami_ports [BOUNDARY_IOMASK]
      and GRAPH_IO_CONFIG_ADDR

    list of streams available on this platform 
    (see also stream_types.h : enum stream_io_domain)
*/ 
#define PLATFORM_DATA_STREAM_IN_INSTANCE_0  0u
#define PLATFORM_AUDIO_OUT_INSTANCE_0       1u
#define PLATFORM_COMMAND_OUT_INSTANCE_0     2u

#define LAST_IO_FUNCTION_PLATFORM 3

extern uint32_t check_hw_compatibility(uint32_t whoami, uint32_t header);

#endif /* #ifndef PLATFORM_COMPUTER_H */
