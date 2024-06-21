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
    SCRIPT_REGSIZE=8
*/

typedef union
{   char    c;       char v_c[8];
    int8_t  i8;    int8_t v_i8[8];
    int16_t i16;  int16_t v_i16[4];
    int32_t i32;  int32_t v_i32[2];
    int64_t i64;
    sfloat  f32;    sfloat v_f32[2]; 
    sdouble f64;
} regdata_t;


extern void arm_stream_script_interpreter (
    arm_stream_instance_t *S,
    uint32_t *descriptor,
    uint16_t *byte_code,
    uint8_t *ram);

extern void * pack2linaddr_ptr(const intPtr_t *long_offset, uint32_t data, uint32_t unit);

#endif

#ifdef __cplusplus
}
#endif
 