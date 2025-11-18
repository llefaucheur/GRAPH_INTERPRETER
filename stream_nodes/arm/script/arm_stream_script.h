/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_filter.c
 * Description:  filters
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * --------------------------------------------------------------------
 *
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
#include "stream_common_types.h"  

     /* IEEE 754 : S(1) expf(8)  fraction (23) */
#define IEEE_MANTISSA 0x007FFFFFL
#define IEEE_FP32EXP 8
#define IEEE_FP32MANTISSA 23

//typedef union floatb {
//    uint32_t i;
//    struct {
//        unsigned m : 23;   /* Mantissa */
//        unsigned e : 8;    /* Exponent */
//        unsigned s : 1;    /* Sign bit */
//    } b;
//} floatb_t;
//
//
//typedef union doubleb {
//    uint64_t i;
//    struct {
//        unsigned ml: 32;   /* Mantissa low */
//        unsigned mh: 20;   /* Mantissa high */
//        unsigned e : 11;   /* Exponent */
//        unsigned s : 1;    /* Sign bit */
//    } b;
//} doubleb_t;


typedef union
{      char v_c[8];
     int8_t v_i8[8];
    int16_t v_i16[4];
    int32_t v_i32[2];
    #define REGS_DATA 0
    #define REGS_TYPE 1
    float_t v_f32[2];
    double_t f64;
} regdata_t;


//typedef union
//{   int32_t i32;  
//    float_t  f32;   
//} regdata32_t;

/* error codes going to I->ctrl.errors */
#define ERROR_STACK_UNDERFLOW   (1 << 0)
#define ERROR_STACK_OVERFLOW    (1 << 1)
#define ERROR_TIME_UNDERFLOW    (1 << 2)

/*
    THIS GOES IN ARC DESC 

    ARC buffer 
    |    Registers 8 Bytes :  R0 .. r11
    |    Stack 8 Bytes 
    |    Heap 4 Bytes

    BYTECODE XXXXXXXXXXXXXXX
    
    INSTANCE (arc descriptor address = *script_instance
           |
           v                 <---- nStack 64b---><--- heap 32b--->
           R0 R1 R2 ..   r13
           <---registers---> [..................][...............]
                               SP init = nregs
*/

    typedef struct
    {
        arm_stream_instance_t *S;   
        uint32_t *arc_desc;
        stream_services *services;
        regdata_t *REGS;                /* registers and stack */
        regdata_t work_regs[3];         /* src0 1 2 */
        uint32_t *byte_code;            /* program to run */
        uint32_t instruction;           /* current instruction */
        uint16_t PC;                    /* in uint32 */
        uint16_t codes;                 /* code size */          
        uint16_t cycles_downcounter;    /* error detection */
#define MAXCYCLES 100
        uint8_t SP;                     /* in REGS unit */          
        uint8_t nstack;          
        uint8_t nregs;          
        uint8_t errors;
        uint8_t test_flag;              /* test result */
        uint8_t compact;                /* indexes are packed in a single W32 */
        uint8_t inst_nbw32;             /* number of words of the instruction */
        uint8_t debug;                  /* register dump in each cycle */
    } arm_script_instance_t;


extern void arm_stream_script_interpreter (arm_script_instance_t *I);


#endif  // if carm_stream_script_H

#ifdef __cplusplus
}
#endif
 