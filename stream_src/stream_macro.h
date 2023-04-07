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
#ifndef cSTREAM_MACRO_H
#define cSTREAM_MACRO_H


/*============================ BIT-FIELDS MANIPULATIONS ============================*/
/*
 *  stream constants / Macros.
 */
 
#define SHIFT_SIZE(base,shift) ((base) << ((shift) << 2));           


#define MIN(a, b) (((a) > (b))?(b):(a))
#define MAX(a, b) (((a) < (b))?(b):(a))
#define MEMCPY(dst,src,n) memcpy((dst),(src),(n))


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

#define PACK2LINADDR(x,o) ((RD((x),BASEIDX_ARCW0)<<(RD((x),BASESHIFT_ARCW0)<<2))+(*((o)+RD(x, DATAOFF_ARCW0))))

#define SET_BIT(arg, bit)   ((arg) |= (U(1) << U(bit)))
#define CLEAR_BIT(arg, bit) ((arg) = U(arg) & U(~(U(1) << U(bit))))
#define TEST_BIT(arg, bit)  (U(arg) & (U(1) << U(bit)))

#define FLOAT_TO_INT(x) ((x)>=0.0f?(int)((x)+0.5f):(int)((x)-0.5f))

#ifdef _MSC_VER 
#define DATA_MEMORY_BARRIER
#define INSTRUCTION_SYNC_BARRIER
#else
#if MULTIPROCESS == 1
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
 


#endif /* #ifndef cSTREAM_MACRO_H */
/*
 * -----------------------------------------------------------------------
 */
