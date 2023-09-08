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
#ifdef __cplusplus
 extern "C" {
#endif
   
    
#ifndef cSTREAM_MACRO_H
#define cSTREAM_MACRO_H


/*============================ BIT-FIELDS MANIPULATIONS ============================*/
/*
 *  stream constants / Macros.
 */
 
#define SHIFT_SIZE(base,shift) ((base) << ((shift) << 2));           


#define MIN(a, b) (((a) > (b))?(b):(a))
#define MAX(a, b) (((a) < (b))?(b):(a))
#define MAXINT32 0x7FFFFFFFL
//#define MEMCPY(dst,src,n) {void *x; uint32_t i; x=memcpy((dst),(src),(n));}
#define MEMCPY(dst,src,n) {uint32_t i; for(i=0;i<n;i++){dst[i]=src[i];}}


/* bit-field manipulations */
#define CREATE_MASK(msb, lsb)               (((U(1) << ((msb) - (lsb) + U(1))) - U(1)) << (lsb))
#define MASK_BITS(arg, msb, lsb)            ((arg) & CREATE_MASK(msb, lsb))
#define EXTRACT_BITS(arg, msb, lsb)         (MASK_BITS(arg, msb, lsb) >> (lsb))
#define INSERT_BITS(arg, msb, lsb, value) \
    ((arg) = ((arg) & ~CREATE_MASK(msb, lsb)) | (((value) << (lsb)) & CREATE_MASK(msb, lsb)))
#define MASK_FIELD(arg, field)              MASK_BITS((arg), field##_MSB, field##_LSB)

#define EXTRACT_FIELD(arg, field)           U(EXTRACT_BITS((U(arg)), field##_MSB, field##_LSB))
#define RD(arg, field) U(EXTRACT_FIELD(arg, field))

#define INSERT_FIELD(arg, field, value)     INSERT_BITS((arg), field##_MSB, field##_LSB, value)
#define ST(arg, field, value) INSERT_FIELD((arg), field, U(value)) 

#define LOG2BASEINWORD32 2
#define BASEINWORD32 (1<<LOG2BASEINWORD32)
#define PACK2LINADDR(o,x) (o[RD(x,DATAOFF_ARCW0)] + \
        (RD(x,BASESIGN_ARCW0))? \
            (1 + ~(((intPtr_t)RD((x),BASEIDX_ARCW0))<<LOG2BASEINWORD32)):\
            ( ((intPtr_t)RD((x),BASEIDX_ARCW0))<<LOG2BASEINWORD32))

#define SET_BIT(arg, bit)   ((arg) |= (U(1) << U(bit)))
#define CLEAR_BIT(arg, bit) ((arg) = U(arg) & U(~(U(1) << U(bit))))
#define TEST_BIT(arg, bit)  (U(arg) & (U(1) << U(bit)))

#define FLOAT_TO_INT(x) ((x)>=0.0f?(int)((x)+0.5f):(int)((x)-0.5f))


#endif /* #ifndef cSTREAM_MACRO_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
 