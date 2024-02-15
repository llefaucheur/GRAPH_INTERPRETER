/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_filter.c
 * Description:  filters
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
   

#ifndef carm_stream_script_H
#define carm_stream_script_H


#include "stream_const.h"      
#include "stream_types.h"  

/*
    content of registers[0] / stack[0]
*/

#define REGSIZE 8   /* size of regdata_t */
typedef union
{   char    c;       char v_c[8];
    int8_t  i8;    int8_t v_i8[8];
    int16_t i16;  int16_t v_i16[4];
    int32_t i32;  int32_t v_i32[2];
    int64_t i64;
    sfloat  f32;    sfloat v_f32[2]; 
    sdouble f64;
} regdata_t;

typedef struct
{
#define     __unused_SCRIPT_MSB U(31)  /*  6    */     
#define     __unused_SCRIPT_LSB U(26)  
#define       FORMAT_SCRIPT_MSB U(25)  /*  2   native/stream byte codes format */
#define       FORMAT_SCRIPT_LSB U(24)  /*      */
#define       PRESET_SCRIPT_MSB U(23)  /*  4   control parameter */
#define       PRESET_SCRIPT_LSB U(20)  /*      */
#define LOG2MAXCYCLE_SCRIPT_MSB U(19)  /*  8   minifloat reset and return when reaching this number of decoded instructions */
#define LOG2MAXCYCLE_SCRIPT_LSB U(12)  /*      */
#define NEW_USE_CASE_SCRIPT_MSB U(11)  /*  1   new use-case arrived */
#define NEW_USE_CASE_SCRIPT_LSB U(11)  /*      */
#define       NSTACK_SCRIPT_MSB U(10)  /*  7   size of the stack */
#define       NSTACK_SCRIPT_LSB U( 4)  /*      */
#define        NREGS_SCRIPT_MSB U( 3)  /*  4   number of registers */
#define        NREGS_SCRIPT_LSB U( 0)  /*      */
    
    uint32_t state;
    const intPtr_t *long_offset;          /* pointer to "intPtr_t long_offset[MAX_NB_MEMORY_OFFSET];" */
    uint32_t use_case[2];

} arm_stream_script_instance;

extern void arm_stream_script_interpreter (
    arm_stream_script_instance *S,
    stream_xdmbuffer_t *byte_code);

#endif

#ifdef __cplusplus
}
#endif
 