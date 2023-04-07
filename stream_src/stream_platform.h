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
#ifndef cSTREAM_PLATFORM_H
#define cSTREAM_PLATFORM_H


#include "stream_const.h"

#define U(x) ((uint32_t)(x))
#define U8(x) ((uint8_t)(x))

//enum platform_al_services       
#define PLATFORM_PROC_ID            1u           /* who am i ? */
#define PLATFORM_MP_GRAPH_SHARED    2u   /* need to declare the graph area as "sharable" in S = MPU_RASR[18] */
#define PLATFORM_MP_BOOT_SYNCHRO    3u   /* collission of access to the graph at boot time */
#define PLATFORM_MP_BOOT_WAIT       4u      /* wait master processor copies the graph */
#define PLATFORM_MP_BOOT_DONE       5u      /* to confirm the graph was copied in RAM */
#define PLATFORM_MP_RESET_WAIT      6u     /* wait the graph is initialized */
#define PLATFORM_MP_RESET_DONE      7u     /* tell the reset sequence was executed for that Stream instance */
#define PLATFORM_IO_SET_STREAM_ALL  8u   /* launch all the graph interfaces */
#define PLATFORM_IO_SET_STREAM      9u     /* share &platform_io(), buffer address, *selection of setting EXTENSION/option, data format */
#define PLATFORM_IO_DATA           10u           /* "data exchanges */ 
#define PLATFORM_IO_STOP_STREAM    11u    /*  */
#define PLATFORM_IO_ACK            12u            /* interface callback to arm_stream_io */
#define PLATFORM_BACKUP_MEM        13u        /* share the list of memory areas to backup periodically */
#define PLATFORM_RESTORE_MEM       14u       /* restore memory areas before warm boot */
#define PLATFORM_EXEC_TIME         15u        /* time counter since last call */
#define PLATFORM_ERROR             16u             /* error to report to the application */
#define PLATFORM_TIME_SET          17u
#define PLATFORM_TIMER_SET         18u
#define PLATFORM_TIME_READ         19u
#define PLATFORM_OFFSETS           20u


//enum error_codes 
#define ERROR_MEMORY_ALLOCATION     1u



//enum io_buffer_allocation 
#define ALLOC_FROM_APPLICATION      0u
#define ALLOC_FROM_IO               1u

//enum io_master_follower
#define IO_IS_MASTER                0u
#define IO_IS_FOLLOWER              1u


/*---- Platform Sensor configuration : io_domain ----*/
#define     UNUSED_IOMEM_MSB        31u 
#define     UNUSED_IOMEM_LSB        28u /* 4   */
#define  NOTISOCHRONOUS_IOMEM_MSB   27u /*    used to size the FIFO:  FRAMESIZE_FMT0 x SAMPLING_FMT1 */
#define  NOTISOCHRONOUS_IOMEM_LSB   27u /* 1  1:bursty asynchronous : frame size and FS give the peak data-rate */
#define            ARCH_IOMEM_MSB   26u 
#define            ARCH_IOMEM_LSB   23u /* 4  arch bit-field usable 0=any for the list of platform_manifest[arch] */
#define            PROC_IOMEM_MSB   22u 
#define            PROC_IOMEM_LSB   15u /* 8  locks this IO to proc (among 8), 0=any from this architecture */
#define   MEM_SPACE_OFF_IOMEM_MSB   14u  
#define   MEM_SPACE_OFF_IOMEM_LSB   12u /* 3  64bits offset selection : 0/internal/external/io = idx_memory_base_offset */
#define  BUFFER_FROM_HW_IOMEM_MSB   11u /*    "io_buffer_allocation"  */
#define  BUFFER_FROM_HW_IOMEM_LSB   11u /* 1  Buffer is provided by the driver = 1, data pointer is provided by the application at set-up = 0 */
#define MASTER_FOLLOWER_IOMEM_MSB   10u /*    "io_master_follower" */
#define MASTER_FOLLOWER_IOMEM_LSB   10u /* 1  Master=0 Follower=1  */
#define PLATFORM_IO_IDX_IOMEM_MSB    9u /*    io_index */  
#define PLATFORM_IO_IDX_IOMEM_LSB    6u /* 4  platform_io_idx from domains below */
#define          DOMAIN_IOMEM_MSB    5u
#define          DOMAIN_IOMEM_LSB    0u /* 6  64 physical domains = enum  stream_io_domain */

#define PACK_IOMEMDOMAIN(ARCH,PROC,OFFSET,HWBUFFER,MASTER,IOIDX,DOMAIN) \
    (((ARCH)<<23)|((PROC)<<15)|((OFFSET)<<12)|((HWBUFFER)<<11)|((MASTER)<<10)|((IOIDX)<<6)|((DOMAIN)<<0))


#endif /* #ifndef cSTREAM_PLATFORM_H */
/*
 * -----------------------------------------------------------------------
 */
