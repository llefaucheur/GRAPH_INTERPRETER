/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_filter.c
 * Description:  filters
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * --------------------------------------------------------------------
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

    
typedef union
{   char    c;       char v_c[8];
    int8_t  i8;    int8_t v_i8[8];
    int16_t i16;  int16_t v_i16[4];
    int32_t i32;  int32_t v_i32[2];
    int64_t i64;
    sfloat  f32;   sfloat v_f32[2]; 
    #define REGS_DATA 0
    #define REGS_TYPE 1
    sdouble f64;
} regdata_t;


typedef union
{   int32_t i32;  
    sfloat  f32;   
} regdata32_t;


/*
    THIS GOES IN ARC DESC 

    ARC buffer 
    |    Registers 8 Bytes :  R0 .. r11
    |    Stack 8 Bytes 
    |    Heap 4 Bytes

    BYTECODE XXXXXXXXXXXXXXX
    
    INSTANCE (arc descriptor address = *script_instance
           |   
           v                  <---- nStack  ---->
           R0 R1 R2 ..   r11  R13 R14
           <---registers--->  SP SP+1
                    STACK :   [..................]
                              SP init = nregs
                    HEAP / PARAM (4bytes/words)  [............]
*/

    typedef struct
    {
        arm_stream_instance_t *S;   
        uint32_t *byte_code;
        int32_t *heap;                  /* working area */
        regdata_t *REGS;                /* registers and stack */
        uint32_t instruction;

        struct
        {   unsigned int free : 3;
            unsigned int nregs : 4;
            unsigned int test_flag : 1;
            unsigned int PC : 8;        /* in uint32 */
            unsigned int SP : 8;        /* in REGS unit */
    
#define MAXCYCLES 255
            unsigned int cycle_downcounter : 8;
        } ctrl;
    
    } arm_script_instance_t;


extern void arm_stream_script_interpreter (
    arm_script_instance_t *I,
    int32_t *descriptor,
    int32_t *byte_code);


#endif  // if carm_stream_script_H

#ifdef __cplusplus
}
#endif
 