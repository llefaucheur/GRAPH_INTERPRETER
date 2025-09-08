/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_script_process.c
 * Description:  filters
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * --------------------------------------------------------------------
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
#include "presets.h"
#ifdef CODE_ARM_STREAM_SCRIPT

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>
#include <string.h> // memcpy
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_script.h"
#include "arm_stream_script_instructions.h"

static void arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, int32_t *dst, int32_t src1, int32_t src2);
static void test_and_arithmetic_operation(arm_script_instance_t *I);
static void readreg(arm_script_instance_t *I, regdata_t *data, int32_t srcID, uint8_t K);
static void writreg(arm_script_instance_t *I, int32_t dstID, regdata_t *src, uint8_t dtype);
static void jmov_operation(arm_script_instance_t *I);


//float minifloat_to_float(uint8_t mf) {
//    // Extract bits
//    int sign = (mf >> 7) & 0x01;
//    int exp = (mf >> 3) & 0x0F;
//    int mantissa = mf & 0x07; // 3 bits mantissa
//
//    float value;
//    int bias = 7; // Exponent bias = 2^(4-1) - 1
//
//    if (exp == 0) {
//        if (mantissa == 0) {
//            // Zero
//            value = 0.0f;
//        } else {
//            // Denormalized number
//            float frac = mantissa / 8.0f; // 3 mantissa bits
//            value = powf(2, 1 - bias) * frac;
//        }
//    } else {
//        // Normalized number
//        float frac = 1 + (mantissa / 8.0f);
//        value = powf(2, exp - bias) * frac;
//    }
//
//    // Apply sign
//    if (sign)
//        value = -value;
//
//    return value;
//}


/*
*   arithmetics operations and test on mantissa + exponents
*/
static void arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, int32_t *dst_, int32_t src1, int32_t src2)
{
    int32_t dst; 

    dst = 0xFFFFFFFF;
    switch (opar)
    {
    default:
    case OPAR_NOP  : dst = src2;                            break;
    case OPAR_ADD  : dst = src1 + (src2);                   break;
    case OPAR_SUB  : dst = src1 - (src2);                   break;
    case OPAR_RSUB : dst = src2 - (src1);                   break;
    case OPAR_MUL  : dst = src1 * (src2);                   break;
    case OPAR_DIV  : dst = (src2 == 0) ? 0 : src1 / src2;   break;
    case OPAR_RDIV : dst = (src1 == 0) ? 0 : src2 / src1;   break;
                  
    case OPAR_MAX  : dst = MAX(src2, src1);                 break;
    case OPAR_MIN  : dst = MIN(src2, src1);                 break;
                   
    case OPAR_OR   : dst = src1 | src2;                     break;
    case OPAR_NOR  : dst = !(src1 | src2);                  break;
    case OPAR_AND  : dst = src1 & src2;                     break;
    case OPAR_XOR  : dst = src1 ^ src2;                     break;
    case OPAR_RSHFT: dst = src1 >> src2;                    break;
    case OPAR_SET  : dst = src1 | (1<<src2);                break;
    case OPAR_CLR  : dst = src1 & (~(1<<src2));             break;
    }

    *t = 0;
    switch (opcode)
    {
    case OP_TESTEQU : if (*dst_ == dst ) *t = 1; break;
    case OP_TESTLEQ : if (*dst_ <= dst ) *t = 1; break;
    case OP_TESTLT  : if (*dst_  < dst ) *t = 1; break;
    case OP_TESTNEQ : if (*dst_ != dst ) *t = 1; break;
    case OP_TESTGEQ : if (*dst_ >= dst ) *t = 1; break;
    case OP_TESTGT  : if (*dst_  > dst ) *t = 1; break;
    }

    if (opcode == OP_LD)
    {   *dst_ = dst;   
    }
}



/**
  @brief  push valid register index on stack 
*/
static void optional_push (arm_script_instance_t *I, int32_t src1)
{
    if (src1 != RegNone) 
    { I->REGS[I->SP++] = I->REGS[src1]; 
    }
}



/**
  @brief  read register
          *data <- *(srcID)
*/
static void readreg(arm_script_instance_t *I, regdata_t *data, int32_t srcID, uint8_t K) 
{
    uint32_t instruction = I->instruction;

    if (K)
    {   if (0 == RD(instruction, SRC2LONGK_PATTERN_INST))
        {   if (1 == RD(instruction, OP_RKEXT_INST))    // extended constant
            {   data->v_i32[REGS_DATA] = I->byte_code[I->PC++];
                data->v_i32[REGS_TYPE] = RD(instruction, DTYPE_REGS1);
            } 
            else                                        
            {   if (srcID == RegSP0)                    // simple stack read + dtype
                {   *data = I->REGS[I->SP];
                }
                else if (srcID == RegSP1)               // pop data (SP --)
                {   *data = I->REGS[I->SP]; 
                    I->SP --;
                }
                else                                    // read register data + dtype
                {   *data = I->REGS[srcID];
                }
            }
        }
        else 
        {   data->v_i32[REGS_DATA] = RD(instruction, OP_K_INST);    // 12bits signed short constant
            data->v_i32[REGS_DATA] -= UNSIGNED_K_OFFSET;            // [-2016  2048]
            data->v_i32[REGS_TYPE] = DTYPE_INT32;
        }        
    }
    /* read src2 */
    else
    {   *data = I->REGS[srcID];                         // read src2 data + dtype
    }
}


/**
  @brief  write to dstID register 
        operation : 
          (*dstID)  <-  *(src data) + dtype
*/
static void writreg(arm_script_instance_t *I, int32_t dstID, regdata_t *src, uint8_t dtype)
{
    regdata_t *pdst;

    if (dstID == RegSP0)
    {   pdst = &(I->REGS[I->SP]);
    }
    else if (dstID == RegSP1)
    {   pdst = &(I->REGS[I->SP]);
        I->SP ++;
        if (I->SP > I->nregs + I->nstack)    // check stack underflow
        {   I->errors |= ERROR_STACK_OVERFLOW;
            I->SP--;
        }
    }
    else
    {   pdst = &(I->REGS[dstID]);
    }

    pdst->v_i32[REGS_DATA] = src->v_i32[REGS_DATA];
    pdst->v_i32[REGS_TYPE] = dtype;
}


/**
  @brief  BRANCH / CALL / LABELS
*/
static void jmov_operation(arm_script_instance_t *I)
{
    int32_t K, *DST;
    regdata_t reg_src2K;
    uint8_t dst, src1, src2;
    int32_t instruction = I->instruction;
    int32_t opar = RD(instruction, OP_OPAR_INST);

    // stack pointer increment is interpreted from right to left when reading the line 
    // STACK INCREMENT : pre-check SRC2 on :
    //  test SRC2=SP1 on OPLJ_BASE OPLJ_SIZE OPLJ_SCATTER OPLJ_GATHER OPLJ_SYSCALL
    if (opar == OPLJ_BASE || opar == OPLJ_SIZE || opar == OPLJ_SCATTER || opar == OPLJ_GATHER ||opar == OPLJ_SYSCALL)
    {   
    }

    src1= (uint8_t)RD(instruction, OP_SRC1_INST);
    if (src1 == RegSP0) 
    { src1 = (uint8_t)(I->SP); 
    } 
    if (src1 == RegSP1) 
    { src1 = (uint8_t)(I->SP); I->SP --; 
    }

    dst = (uint8_t)RD(instruction, OP_DST_INST);     
    if (dst == RegSP0) 
    { dst = (uint8_t)(I->SP); 
    } 
    if (dst == RegSP1) 
    { dst = (uint8_t)(I->SP); I->SP ++;       // stack destination => increment 
    }
    DST = &(I->REGS[dst].v_i32[REGS_TYPE]);


    reg_src2K.v_i32[REGS_DATA] = 0; reg_src2K.v_i32[REGS_TYPE] = 0;
    src2 =  RD(instruction, OP_SRC2_INST);
    readreg(I, &reg_src2K, src2, 1);

    //// can be a float, convert it to int32
    //if (RD(I->REGS[src2].v_i32[REGS_TYPE], DTYPE_REGS1) > DTYPE_INT32)
    //{   I->REGS[src2].v_i32[REGS_DATA] = (int32_t)(I->REGS[src2].v_f32[REGS_DATA]);
    //    ST(I->REGS[src2].v_i32[REGS_TYPE], DTYPE_REGS1, DTYPE_INT32);
    //}

    K = reg_src2K.v_i32[REGS_DATA]; 

    switch (opar)
    {
    // data type cast
    // IIyyy-OPARDST_______<--K12-0SRC2     OPLJ_CASTPTR    dst  dtype
    case OPLJ_CASTPTR:
        ST(I->REGS[dst].v_i32[REGS_TYPE], DTYPE_REGS1, K); 
        break;

    // set the base of cicular buffer addressing
    // IIyyy-OPARDST_______<--K12-0SRC2     OPLJ_BASE       dst src2/K    
    case OPLJ_BASE:
        ST(I->REGS[dst].v_i32[REGS_TYPE], BASE_REGS1, K); 
        break;

    // set the size of the circular buffer
    // IIyyy-OPARDST_______<--K12-0SRC2     OPLJ_SIZE       circular addressing control
    case OPLJ_SIZE:
        ST(I->REGS[dst].v_i32[REGS_TYPE], SIZE_REGS1, K); 
        break;

    // the gathered data will be read from the parameter area
    // IIyyy-OPARDST_______<--K12-0SRC2     OPLJ_PARAM      set r1 param xxx load offset in param (sets H1C0 = 0):
    case OPLJ_PARAM:           
    case OPLJ_GRAPH:
        ST(I->REGS[dst].v_i32[REGS_TYPE], H1C0_REGS1, 0); 
        I->REGS[dst].v_i32[REGS_DATA] = K;
        break;

    // the gathered data will be from the heap
    // IIyyy-OPARDST_______<--K12-0SRC2     OPLJ_HEAP       set r3 heap xxx load offset in heap  (sets H1C0 = 1)     
    case OPLJ_HEAP:                 // 
        ST(I->REGS[dst].v_i32[REGS_TYPE], H1C0_REGS1, 1); 
        I->REGS[dst].v_i32[REGS_DATA] = K;
        break;

    // scatter data access : R[K}=R
    // IIyyy-OPARDST_SRC1pp<--K12-0SRC2     OPLJ_SCATTER    [ dst src2/k BYTES ]+ = src1  pre-increment
    //    cast to the destination format                    [ dst ]+ src2/k BYTES = src1  use H1C0 to select DST memory
    case OPLJ_SCATTER:  
        {   uint32_t index;
            uint8_t  *p8src, *p8dst, nbytes;
            uint8_t preinc, destH1C0, dsttype, updateptr;
        
        destH1C0 = RD(I->REGS[dst].v_i32[REGS_TYPE], H1C0_REGS1); // select H1C0
        preinc  = (uint8_t)RD(instruction, OP_EXT0_INST);
        updateptr = (uint8_t)RD(instruction, OP_EXT1_INST);
        dsttype = (uint8_t)RD(I->REGS[dst].v_i32[REGS_TYPE], DTYPE_REGS1); // destination type
        nbytes = 0;
        index = 0;

        if (destH1C0 == 1)                                      // destination in heap ?
        {   p8dst = (uint8_t *)&(I->REGS[(I->nregs + I->nstack)*2]);
        }
        else                                                    // destination in the code param area? (graph is in RAM)
        {   p8dst = (uint8_t *)&(I->byte_code[I->codes]);
        }

        // check pre-increment
        if (preinc)
        {   index = K;
        }
        p8src = (uint8_t *)&(I->REGS[src1].v_i32[REGS_DATA]);
        p8dst = &(p8dst[index]);

        /* cast the source to the type of the destination to allow byte addressing */ 
        switch (dsttype)
        {
        case DTYPE_UINT8:  nbytes = 1; break;
        case DTYPE_INT16: case DTYPE_FP16: nbytes = 2; break;
        case DTYPE_UINT32:case DTYPE_INT32:case DTYPE_FP32:case DTYPE_TIME32:case DTYPE_PTR28B: nbytes = 4; break;
        }

        memcpy (p8dst, p8src, nbytes);
 
        // check post-increment, post-increment without update is useless
        if (0 == preinc)
        {   index = K;
        }        

        // check update
        if (updateptr)
        {   I->REGS[dst].v_i32[REGS_DATA] += index;
        }        

        break;
        }

    // gather data access : R=R[K}
    // IIyyy-OPARDST_SRC1pp<--K12-0SRC2     OPLJ_GATHER  :  dst = [ src1 src2/k ]+   pre-increment
    //    cast from the source format,                      dst = [ src1 ]+ src2/k   post-increment
    case OPLJ_GATHER:
        {   uint32_t index;
            uint8_t  *p8src, *p8dst, nbytes;
            uint8_t preinc, destH1C0, srctype, updateptr;
        
        destH1C0 = RD(I->REGS[dst].v_i32[REGS_TYPE], H1C0_REGS1); // select H1C0
        preinc  = (uint8_t)RD(instruction, OP_EXT0_INST);
        updateptr = (uint8_t)RD(instruction, OP_EXT1_INST);
        srctype = (uint8_t)RD(I->REGS[dst].v_i32[REGS_TYPE], DTYPE_REGS1); // destination type
        nbytes = 0;
        index = 0;

        if (destH1C0 == 1)                                      // source in heap ?
        {   p8src = (uint8_t *)&(I->REGS[(I->nregs + I->nstack)*2]);
        }
        else                                                    // source in the code param area? 
        {   p8src = (uint8_t *)&(I->byte_code[I->codes]);
        }

        // check pre-increment
        if (preinc)
        {   index = K;
        }

        p8src = &(p8src[index]);
        p8dst = (uint8_t *)&(I->REGS[src1].v_i32[REGS_DATA]);

        /* clear the destination word according the size of the source */ 
        switch (srctype)
        {
        case DTYPE_UINT8:  nbytes = 1; memset(p8dst, 0, sizeof(uint32_t)); break;
        case DTYPE_INT16: case DTYPE_FP16: nbytes = 2; memset(p8dst, 0, sizeof(uint32_t)); break;
            // read the lsb bytes of int64
        case DTYPE_INT64: case DTYPE_UINT32: case DTYPE_INT32:case DTYPE_FP32:case DTYPE_TIME32:case DTYPE_PTR28B: nbytes = 4; break;
        // case DTYPE_FP64: read FP64, convert it to FP32, TODO
        }
        memcpy (p8dst, p8src, nbytes);              // write 8b, 16b or 32b
 
        // check post-increment, post-increment without update is useless
        if (0 == preinc)
        {   index = K;
        }        

        // check update
        if (updateptr)
        {   I->REGS[dst].v_i32[REGS_DATA] += index;
        }        
        break;
        }

    // write to a destination bit-field R[lsb msb]=R
    // IIyyy-OPARDST_SRC1__LLLLLLPPPPPP     OPLJ_WR2BF     move r2 | lenK posK | r3 
    case OPLJ_WR2BF:
        {
        uint8_t msb, lsb;
        uint32_t mask, tmp;
        msb = (uint8_t)RD(instruction, BITFIELD_MSB_INST);
        lsb = (uint8_t)RD(instruction, BITFIELD_LSB_INST);
        mask = (uint32_t)(-1) >> (31-(msb-lsb));
        tmp = I->REGS[src1].v_i32[REGS_DATA] & mask;
        mask <<= lsb;
        I->REGS[dst].v_i32[REGS_DATA] &= ~mask;
        
        I->REGS[dst].v_i32[REGS_DATA] |= tmp << lsb;
        break;
        }

    // extract a bit-field  R=R[lsb msb]
    // IIyyy-OPARDST_SRC1__LLLLLLPPPPPP     OPLJ_RDBF      move r2 r3 | lenK posK |
    case OPLJ_RDBF:
        {   
        uint8_t msb, lsb;
        uint32_t mask, tmp;
        msb = (uint8_t)RD(instruction, BITFIELD_MSB_INST);
        lsb = (uint8_t)RD(instruction, BITFIELD_LSB_INST);
        mask = (uint32_t)(-1) >> (31-(msb-lsb));
        mask <<= lsb;
        tmp = I->REGS[src1].v_i32[REGS_DATA] & mask;
        I->REGS[dst].v_i32[REGS_DATA] = tmp >> lsb;
        break;
        }

    // swap two registers (or stack)    swap r1 r2
    // IIyyy-OPARDST_SRC1______________     OPLJ_SWAP   
    case OPLJ_SWAP:
        reg_src2K = I->REGS[src1]; 
        I->REGS[src1] = I->REGS[dst]; 
        I->REGS[dst] = reg_src2K; 
        break;

    // remove several registers from the stack : delete 4
    // IIyyy-OPARDST______________YYYYY     OPLJ_DELETE 
    case OPLJ_DELETE:
        I->SP = (uint8_t)(I->SP - K);
        break;

    // jump to an address and optional save 2 registers : jump label R1
    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_JUMP   
    case OPLJ_JUMP    : 
        I->PC = (uint16_t)(I->PC + K-1);   // JMP offset_K8, PUSH SRC1/SRC2/SRC3, PC was already post incremented
        optional_push(I, dst); optional_push(I, src1); 
        break;

    // decrement a register and branch is not null : banz label R1
    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_BANZ         
    // see ti.com/lit/ds/symlink/tms320c25.pdf
    case OPLJ_BANZ    : 
        I->REGS[RD(instruction,OP_DST_INST)].v_i32[REGS_DATA] --;   // decrement loop counter
        if (I->REGS[src1].v_i32[REGS_DATA] != 0)   
        {   I->PC = (uint16_t)(I->PC + K-1);
        }
        break;

    // call (return address push on the stack) and save registers:    call label R1
    // IIyyy-OPARDST_SRC1__<--K12-0SRC2  OPLJ_CALL
    case OPLJ_CALL    : 
        reg_src2K.v_i32[REGS_DATA] = (1+ I->PC);
        I->REGS[I->SP] = reg_src2K;
        I->SP ++;                      // push return address
        I->PC = (uint16_t)(I->PC + K-1);           // call
        optional_push(I, dst);  optional_push(I, src1);  
        break;

    // IIyyy-OPARDST_SRC1SRC3SRC4<-K6->  OPLJ_SYSCALL K6 R1(dst) R2(src1) R3(src3) R4(src4)
    // FEDCBA9876543210FEDCBA9876543210
    /*
    | Syscall 1st index                | register parameters                                          |
    | -------------------------------- | ------------------------------------------------------------ |
    | 1 (access to nodes)              | R1: address of the node<br/>R2: command (tag, reset id, cmd)<br/>    set/read parameter=2/3<br/>R3: address of data<br/>R4: number of bytes |
    | 2 (access to arcs)               | R1: arc's ID<br/>R2: command <br/>    set/read data=8/9<br/>R3: address of data<br/>R4: number of bytes |
    | 3 (callbacks of the application) | R1: application_callback's ID<br/>R2: parameter1 (depends on CB)<br/>R3: parameter2 (depends on CB)<br/>R4: parameter3 (depends on CB) |
    | 4 (IO settings)                  | R1: IO's graph index<br/>R2: command <br/>    set/read parameter=2/3<br/>R3: address of data<br/>R4: number of bytes |
    | 5 (debug and trace)              | TBD                                                          |
    | 6 (computation)                  | TBD                                                          |
    | 7 (low-level functions)          | TBD, peek/poke directly to memory, direct access to IOs (I2C driver, GPIO setting, interrupts generation and settings) |
    | 8 (idle controls)                | TBD, Share to the application the recommended Idle strategy to apply (small or deep-sleep). |
    | 9 (time)                         | R1: command and time format <br/>R2: parameter1 (depends on CB)<br/>R3: parameter2 (depends on CB)<br/>R4: parameter3 (depends on CB) |
    */
    case OPLJ_SYSCALL : // SYSCALL  {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
        {   
        const p_stream_services *al_func;
        //uint8_t K_service = (uint8_t)RD(instruction, SYSCALL_K_INST); 
        //uint8_t src3 = (uint8_t)RD(instruction, OP_SRC3_INST); 
        //uint8_t src4 = (uint8_t)RD(instruction, OP_SRC4_INST); 

        /* void arm_stream_services (uint32_t command, void *ptr1, void *ptr2, void *ptr3, uint32_t n); */
        al_func = &(I->S->al_services);
        //(*al_func)(PACK_SERVICE(0,0,NOTAG_SSRV, SERV_INTERNAL_PLATFORM_CLEAR_BACKUP_MEM, K_service), 
        //    (void *)(I->REGS[dst].v_i32[REGS_DATA]),  
        //    (void *)(I->REGS[src1].v_i32[REGS_DATA]),
        //    (void *)(I->REGS[src3].v_i32[REGS_DATA]),
        //    (uint32_t)(I->REGS[src4].v_i32[REGS_DATA])
        //    );
        }
        break;

    // up to 5 register push on stack
    // IIyyy-OPARDST_SRC1SRC3SRC4__SRC2  OPLJ_SAVE 
    case OPLJ_SAVE   : 
        {
        src1= (uint8_t)RD(instruction, OP_DST_INST ); optional_push(I, src1);      
        src1= (uint8_t)RD(instruction, OP_SRC1_INST); optional_push(I, src1);      
        src1= (uint8_t)RD(instruction, OP_SRC2_INST); optional_push(I, src1);      
        src1= (uint8_t)RD(instruction, OP_SRC3_INST); optional_push(I, src1);      
        src1= (uint8_t)RD(instruction, OP_SRC4_INST); optional_push(I, src1);      
        }
        break;

    // up to 5 pop from stack
    // IIyyy-OPARDST_SRC1SRC3SRC4__SRC2  OPLJ_RESTORE
    case OPLJ_RESTORE: 
        if (RegNone != (src1 = (uint8_t)RD(instruction, OP_DST_INST ))) I->REGS[src1] = I->REGS[I->SP++];     
        if (RegNone != (src1 = (uint8_t)RD(instruction, OP_SRC1_INST))) I->REGS[src1] = I->REGS[I->SP++];     
        if (RegNone != (src1 = (uint8_t)RD(instruction, OP_SRC2_INST))) I->REGS[src1] = I->REGS[I->SP++];     
        if (RegNone != (src1 = (uint8_t)RD(instruction, OP_SRC3_INST))) I->REGS[src1] = I->REGS[I->SP++];     
        if (RegNone != (src1 = (uint8_t)RD(instruction, OP_SRC4_INST))) I->REGS[src1] = I->REGS[I->SP++];  
        break;

    default:
    // return from subroutine 
    // IIyyy-OPAR______________________  OPLJ_RETURN 
    case OPLJ_RETURN : 
        I->PC = (uint16_t)(I->REGS[I->SP++].v_i32[REGS_DATA]);
        break;
    }
}



/**
  @brief  test_arithmetic_operation

    FEDCBA9876543210FEDCBA9876543210
    IIyyy-OPARDST_SRC10000000000SRC2  eq,le,lt,ne,ge,gt,ld dst src1 src2
    IIyyy-OPARDST_SRC1xxxxxxxxxxxxxx  eq,le,lt,ne,ge,gt,ld dst src1 K
    IIyyy-OPARDST_SRC1xxxxxxxxx1TTTT  eq,le,lt,ne,ge,gt,ld dst src1 dtype + K-extented

*/
static void test_and_arithmetic_operation(arm_script_instance_t *I)
{
    regdata_t dst, src1, src2;
    uint8_t db1, db2, dbdst;

    uint8_t t = (uint8_t)(I->test_flag); 
    int32_t instruction = I->instruction;
    int8_t opcode = (uint8_t)RD(instruction, OP_INST);
    int8_t opar = (uint8_t)RD(instruction, OP_OPAR_INST);

    db1 = (uint8_t)RD(instruction, OP_SRC1_INST);
    db2 = (uint8_t)RD(instruction, OP_SRC2_INST);
    dbdst=(uint8_t)RD(instruction, OP_DST_INST);
    db1 = (uint8_t)RD(I->REGS[db1].v_i32[REGS_TYPE], DTYPE_REGS1);
    db2 = (uint8_t)RD(I->REGS[db2].v_i32[REGS_TYPE], DTYPE_REGS1);


    readreg(I, &src2, db1,  1);
    readreg(I, &src1, db2,  0);
    readreg(I, &dst, dbdst, 0);

    arithmetic_operation(opcode, opar, &t, &(dst.v_i32[REGS_DATA]), src1.v_i32[REGS_DATA], src2.v_i32[REGS_DATA]);
    writreg(I, dbdst, &dst, DTYPE_INT32);

    I->test_flag = t;
}

/**
  @brief         arm_stream_script : 16bits virtual machine, or Cortex-M0 binary codes
  @param[in]     pinst      instance of the component
  @param[in]     reset      tells to set the conditional flag before the call
  @return        status     finalized processing
                 
  @par  
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

void arm_stream_script_interpreter (arm_script_instance_t *I)
{
    int32_t  cond, opcode, opar;

    while (1)
    {
        /* check cycles overflow */
        if (I->cycles_downcounter == 1)
        {   I->services(    // TODO : implement the service 
                PACK_SERVICE(SERV_SCRIPT_DEBUG_TRACE, 0, 0, 0, SERV_GROUP_SCRIPT), // uint32_t command, 
                0, 0, 0, 0
            );
        }

        I->instruction = I->byte_code[I->PC++];
        cond = RD(I->instruction, OP_COND_INST);

        /* dump the virtual machine state every cycle */
        if (I->debug)
        {   I->services(    // void arm_stream_services (      
                PACK_SERVICE(SERV_SCRIPT_DEBUG_TRACE, 0, 0, 0, SERV_GROUP_SCRIPT), // uint32_t command, 
                (intptr_t)I->REGS, 0, 0, (intptr_t)(I->nregs) * sizeof(regdata_t)
            );
        }

        /* conditional execution is possible on all instructions */
        if ((cond == IF_YES) && (I->test_flag == TEST_KO))
        {   continue;
        } else 
        if ((cond == IF_NOT) && (I->test_flag == TEST_OK))
        {   continue;
        }

        opcode = RD(I->instruction, OP_INST);
        opar = RD(I->instruction, OP_OPAR_INST);

        if (opcode == OP_JMOV && opar == OPLJ_RETURN)
        {   if (I->SP == I->nregs)
            {  return;
            }
            else 
            {  I->SP--; 
               I->PC = (int16_t)(I->REGS[I->SP].v_i32[REGS_DATA]);
            }
        }

        if (opcode == OP_JMOV)
        {   jmov_operation(I);
        }
        else        /* OP_LD  +  OP_TESTxxx */
        {   test_and_arithmetic_operation(I);
        }

        I->cycles_downcounter--;
    }
}

#ifdef __cplusplus
 }
#endif

#endif //CODE_ARM_STREAM_SCRIPT