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

#include "stream_common_const.h"
#include "presets.h"


/* ------------------------------------------------------------------------------------------
    opaque access to the static area of the node
*/
typedef void* stream_handle_t;



/* ------------------------------------------------------------------------------------------
    stream buffers
*/
struct stream_xdmbuffer
{   intptr_t address;
    intptr_t size;
};

typedef struct stream_xdmbuffer stream_xdmbuffer_t;

typedef void    (stream_services)(uint32_t service_command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n);
typedef void (*p_stream_services) (uint32_t service_command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n);


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


#endif /* cSTREAM_COMMON_TYPES_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
    

