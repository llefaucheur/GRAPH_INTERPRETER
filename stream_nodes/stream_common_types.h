/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_common.h
 * Description:  Common data definition
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


#ifdef __cplusplus
 extern "C" {
#endif

#ifndef cSTREAM_COMMON_TYPES_H
#define cSTREAM_COMMON_TYPES_H

#include "platform.h"
#include "stream_common_const.h"


/* ------------------------------------------------------------------------------------------
    floating-point emulation
*/

#if STREAM_FLOAT_ALLOWED==1
typedef float float_t;
typedef double double_t;

#else
//union floatb {
//    uint32_t i;
//    struct {
//        unsigned m : 23;   /* Mantissa */
//        unsigned e : 8;    /* Exponent */
//        unsigned s : 1;    /* Sign bit */
//    } b;
//};

struct {
    unsigned m : 23;        /* Mantissa */
    unsigned e : 8;         /* Exponent */
    unsigned s : 1;         /* Sign bit */
} float_t_struct;

//union doubleb {
//    uint64_t i;
//    struct {
//        unsigned ml: 32;   /* Mantissa low */
//        unsigned mh: 20;   /* Mantissa high */
//        unsigned e : 11;   /* Exponent */
//        unsigned s : 1;    /* Sign bit */
//    } b;
//};

struct {
    unsigned ml: 32;        /* Mantissa low */
    unsigned mh: 20;        /* Mantissa high */
    unsigned e : 11;        /* Exponent */
    unsigned s : 1;         /* Sign bit */
} double_struct;

typedef uint32_t float_t;
typedef uint64_t double_t;
#endif


/* ------------------------------------------------------------------------------------------
    opaque access to the static area of the node 
*/
typedef void *stream_handle_t;  

///* 
//    if 64 bits architectures are reading the graph:
//    #define intptr_t uint64_t
//    #define MAX_ADD_OFFSET 0x7FFFFFFFFFFFFFFFL
//*/
//#ifdef PLATFORM_ARCH_64BIT
//#define NBBITS_PER_INT 64
//typedef uint64_t  intptr_t;
//typedef  int64_t sintptr_t;
//#endif
//
//#ifdef PLATFORM_ARCH_32BIT
//#define NBBITS_PER_INT 32
//typedef uint32_t  intptr_t;
//typedef  int32_t sintptr_t;
//#endif

/* ------------------------------------------------------------------------------------------
    stream buffers
*/

struct stream_xdmbuffer
{   
    intptr_t address; 
    intptr_t size;
};

typedef struct stream_xdmbuffer stream_xdmbuffer_t;

typedef void    (stream_al_services) (uint32_t service_command, void *ptr1, void *ptr2, void *ptr3, uint32_t n); 
typedef void (*p_stream_al_services) (uint32_t service_command, void *ptr1, void *ptr2, void *ptr3, uint32_t n); 


#endif /* cSTREAM_COMMON_TYPES_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
    

