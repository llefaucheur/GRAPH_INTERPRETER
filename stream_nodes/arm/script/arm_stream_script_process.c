/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_script_process.c
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


#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_script.h"
#include "arm_stream_script_instructions.h"

static void float_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, sfloat *dst, sfloat src1, sfloat src2);
static void int_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, int32_t *dst, int32_t src1, int32_t src2);
static void test_arithmetic_operation(arm_script_instance_t *I);
static void readreg(arm_script_instance_t *I, int32_t *dst, int32_t src, uint8_t K);
static void writreg(arm_script_instance_t *I, int32_t dst, int32_t src);
static void jmov_operation(arm_script_instance_t *I);



static void float_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, sfloat *dst, sfloat src1, sfloat src2)
{
#if 0
    sfloat tmp; 

    switch (opar)
    {
    case OPAR_ADD  : FADD(tmp, src1, src2); break;  
    case OPAR_SUB  : FSUB(tmp, src1, src2); break; 
    case OPAR_MUL  : FMUL(tmp, src1, src2); break; 
    case OPAR_DIV  : FDIV(tmp, src1, src2); break; 
    case OPAR_MAX  : FMAX(tmp, src2, src1); break;
    case OPAR_MIN  : FMIN(tmp, src2, src1); break;
    case OPAR_AMAX : FAMAX(tmp, src2, src1); break;
    case OPAR_AMIN : FAMAX(tmp, src2, src1); break;
    case OPAR_OR   : tmp = I2F(F2I(src1) |  F2I(src2));         break;  // OR  iSRC1 | iSRC2 (or K)        if iSRC is a pointer then it is decoded as *(iSRC)
    case OPAR_NOR  : tmp = I2F(!F2I(src1)|  F2I(src2));         break;  // NOR !(iSRC1 | iSRC2) (or K)         example TEST (*R1) > (*R2) + 3.14   or   R1 = (*R2) + R4
    case OPAR_AND  : tmp = I2F(F2I(src1) &  F2I(src2));         break;  // AND iSRC1 & iSRC2 (or K)        
    case OPAR_XOR  : tmp = I2F(F2I(src1) ^  F2I(src2));         break;  // XOR iSRC1 ^ iSRC2 (or K)        
    case OPAR_SHR  : tmp = I2F(F2I(src1) >> F2I(src2));         break;  // SHR iSRC1 << iSRC2 (or K)       SHIFT   
    case OPAR_SHL  : tmp = I2F(F2I(src1) << F2I(src2));         break;  // SHL iSRC1 >> iSRC2 (or K)       
    case OPAR_SET  : tmp = I2F(F2I(src1) | (1<<F2I(src2)));     break;  // SET iSRC1 | (1 << iSRC2 (or K)) BSET      
    case OPAR_CLR  : tmp = I2F(F2I(src1) & (~(1<<F2I(src2))));  break;  // CLR iSRC1 & (1 << iSRC2 (or K)) BCLR     TESTBIT 0/R0 OPAR_SHIFT(iSRC1, 1<<K5)
    case OPAR_NORM :  //  NORM *iDST = normed on MSB(*iSRC1), applied shift in *iSRC2 
            {   uint32_t count = 0U, mask = 1L << 31;  
                
                while ((src1 & mask) == 0U) { count += 1U; mask = mask >> 1U; }
                tmp = src1 << count; src2 = count;
            }
            break;
    }
    switch (opcode)
    {
    case OP_TESTEQU : *t = FTESTEQU(tmp, dst); break;
    case OP_TESTLEQ : *t = FTESTLEQ(tmp, dst); break;
    case OP_TESTLT  : *t = FTESTLT (tmp, dst); break;
    case OP_TESTNEQ : *t = FTESTNEQ(tmp, dst); break;
    case OP_TESTGEQ : *t = FTESTGEQ(tmp, dst); break;
    case OP_TESTGT  : *t = FTESTGT (tmp, dst); break;
    }

    if (opcode == OP_LD)
    {   *dst = tmp;   
    }
#endif
}

/**
  @brief  ALU
*/
static void int_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, int32_t *dst, int32_t src1, int32_t src2)
{
    int32_t tmp; 

    switch (opar)
    {
    case OPAR_ADD  : tmp = src1 + (src2);                    break;
    case OPAR_SUB  : tmp = src1 - (src2);                    break;
    case OPAR_MUL  : tmp = src1 * (src2);                    break;
    case OPAR_DIV  : tmp = (src2 == 0) ? 0 : src1 / src2;    break;
                  
    case OPAR_MAX  : tmp = MAX(src2, src1);                  break;
    case OPAR_MIN  : tmp = MIN(src2, src1);                  break;
    case OPAR_AMAX : tmp = MAX(ABS(src2), ABS(src1));        break;
    case OPAR_AMIN : tmp = MIN(ABS(src2), ABS(src1));        break;
                   
    case OPAR_OR   : tmp = src1 | src2;                      break;
    case OPAR_NOR  : tmp = !(src1 | src2);                   break;
    case OPAR_AND  : tmp = src1 & src2;                      break;
    case OPAR_XOR  : tmp = src1 ^ src2;                      break;
    case OPAR_SHR  : tmp = src1 >> src2;                     break;
    case OPAR_SHL  : tmp = src1 << src2;                     break;
    case OPAR_SET  : tmp = src1 | (1<<src2);                 break;
    case OPAR_CLR  : tmp = src1 & (~(1<<src2));              break;
    case OPAR_NORM :  //  NORM *iDST = normed on MSB(*iSRC1), applied shift in *iSRC2 
            {   uint32_t count = 0U, mask = 1L << 31;  
                while ((src1 & mask) == 0U) { count += 1U; mask = mask >> 1U; }
                tmp = src1 << count; src2 = count;
            }
            break;
    }

    switch (opcode)
    {
    case OP_TESTEQU : *t = (tmp == *dst); break;
    case OP_TESTLEQ : *t = (tmp <= *dst); break;
    case OP_TESTLT  : *t = (tmp  < *dst); break;
    case OP_TESTNEQ : *t = (tmp != *dst); break;
    case OP_TESTGEQ : *t = (tmp >= *dst); break;
    case OP_TESTGT  : *t = (tmp  > *dst); break;
    }

    if (opcode == OP_LD)
    {   *dst = tmp;   
    }
}




/**
  @brief  push register content of src1/src2 on stack
*/
static void optional_push (arm_script_instance_t *I, int32_t src1)
{
    if (src1 != RegNone) { I->REGS[I->ctrl.SP++] = I->REGS[src1]; }
}




/**
  @brief  read register
*/
static void readreg(arm_script_instance_t *I, int32_t *dst, int32_t src, uint8_t K)
{
    uint32_t instruction = I->instruction;

    if (K)
    {   if (0 == RD(instruction, SRC2LONGK_PATTERN_INST))
        {   if (1 == RD(instruction, OP_RKEXT_INST))        // extended constant
            {   *dst = I->byte_code[I->ctrl.PC++];
                switch (RD(instruction, DTYPE_REGS1))
                {
                case DTYPE_UINT8 : case DTYPE_UINT16: case DTYPE_INT16 : case DTYPE_UINT32: 
                case DTYPE_INT32 : case DTYPE_TIME16: case DTYPE_TIME32: case DTYPE_PTR28B: 
                    /* k = src2_K.i32; */
                    break;
                case DTYPE_FP16  : case DTYPE_FP32  : case DTYPE_FP64  : 
                    /* kl.f32 */
                    break;
                case DTYPE_INT64 : case DTYPE_TIME64: 
                    /* read one more word */
                    *dst = I->byte_code[I->ctrl.PC++];
                    break;
                }
            } 
            else                                            // register SRC2 
            {   
                if (src == RegSP0)                          // simple stack read
                {   *dst = I->REGS[I->ctrl.SP].v_i32[REGS_DATA];
                }
                else if (src == RegSP1)                     // pop data 
                {   *dst = I->REGS[I->ctrl.SP].v_i32[REGS_DATA]; 
                    I->ctrl.SP --;
                }
                else                                        // read register data
                {   *dst = I->REGS[src].v_i32[REGS_DATA];
                }
            }
        }
        else 
        {   *dst = RD(instruction, OP_K_INST);      // 14bits signed short constant
            *dst = (*dst) - 8192;                   // [8191 .. -8160]
        }        
    }
}

static void writreg(arm_script_instance_t *I, int32_t dst, int32_t src)
{
    int32_t *pdst;
    int8_t incSP_write;

    incSP_write = 0;
    if (dst == RegSP0)
    {   pdst = &(I->REGS[I->ctrl.SP].v_i32[REGS_DATA]);
    }
    else if (dst == RegSP1)
    {   incSP_write = 1;
        pdst = &(I->REGS[I->ctrl.SP].v_i32[REGS_DATA]);
    }
    else
    {   pdst = &(I->REGS[dst].v_i32[REGS_DATA]);
    }

    *pdst = src;
    I->ctrl.SP += I->ctrl.SP + incSP_write;
}


/**
  @brief  BRANCH / CALL / LABELS
*/
static void jmov_operation(arm_script_instance_t *I)
{
    int32_t k, tmp;
    regdata32_t src2_K;
    regdata_t reg;
    uint8_t dst, src1, src2, src3, src4;
    int32_t instruction = I->instruction;
    int32_t opar = RD(instruction, OP_OPAR_INST);

    dst = RD(instruction, OP_DST_INST);
    src1= RD(instruction, OP_SRC1_INST);
    src2= RD(instruction, OP_SRC2_INST);
    src3= RD(instruction, OP_SRC3_INST);
    src4= RD(instruction, OP_SRC4_INST);



     // ^ ^  K10:  -512 .. +511 (JUMPIDX_INST)
     // | |   
     // |^|^
     // |||| K6 :  0 .. 63 (CALLSYSIDX_INST)
     // ||||
     // ||||
     // ||||
     // ||vv
     // ||   K10: -512 .. +512 (SCGA_K11_INST)
     // vv   K14: -8160 .. +8191 (OP_K_INST)


    if (opar == OPLJ_JUMP || opar == OPLJ_BANZ || opar == OPLJ_CALL)      
    {   k = RD(instruction, JUMPIDX_INST);      // 10bits signed extended 
        k = (k << (32-JUMPIDX_INST_MSB)) >> (32-JUMPIDX_INST_MSB);        
    } else                                                                    
    if (opar == OPLJ_CALLSYS)                                             
    {   k = RD(instruction, CALLSYSIDX_INST);   // 6bits unsigned         
    } else
    if (opar == OPLJ_SCATTER || opar == OPLJ_GATHER)
    {   k = RD(instruction, SCGA_K11_INST);     // 10bits signed    
        k = (k << (32-SCGA_K11_INST_MSB)) >> (32-SCGA_K11_INST_MSB);     
    } else
    {  readreg(I, &(src2_K.i32), src2, 1);
    }

    switch (opar)
    {
    // IIyyy-OPARDST______________1TTTT  OPLJ_CAST(PTR)    dst  dtype
    case OPLJ_CAST:
        ST(I->REGS[I->ctrl.SP].v_i32[REGS_TYPE], DTYPE_REGS1, RD(instruction, DTYPE_REGS1)); 
        break;

    // IIyyy-OPARDST_____xxxxxxxxxYYYYY  OPLJ_BASE         dst src2/K    
    case OPLJ_BASE:
        ST(I->REGS[I->ctrl.SP].v_i32[REGS_TYPE], BASE_REGS1, src2_K.i32); 
        break;

    // IIyyy-OPARDST_____xxxxxxxxxYYYYY  OPLJ_SIZE   
    case OPLJ_SIZE:
        ST(I->REGS[I->ctrl.SP].v_i32[REGS_TYPE], SIZE_REGS1, src2_K.i32); 
        break;

    // IIyyy-OPARDST_SRC10#________SRC2  OPLJ_SCATTER  : st r2 [r4/k] r3  => R2[R4] = R3 
    case OPLJ_SCATTER:
        tmp = I->REGS[dst].v_i32[REGS_DATA];
        tmp = tmp + src2_K.i32;
        I->heap[tmp] = I->REGS[src1].v_i32[REGS_DATA];  // [tmp = dst+K] = src1
        break;

    // IIyyy-OPARDST_SRC11#_<---K11--->  OPLJ_GATHER  : ld r2 r3 [r4/k] =>  R2 = R3[R4]
    case OPLJ_GATHER:
        tmp = I->REGS[src1].v_i32[REGS_DATA];
        tmp = tmp + src2_K.i32;
        I->REGS[dst].v_i32[REGS_DATA] = I->heap[tmp];  // dst = [tmp = src1+K]
        break;

    // IIyyy-OPARDST_SRC1__LLLLLLPPPPPP  OPLJ_WR2BF     move r2 | lenK posK | r3 
    case OPLJ_WR2BF:
        {
        uint32_t pos, len, mask, masklsb;
        pos = RD(instruction, BITFIELD_POS_INST);
        len = RD(instruction, BITFIELD_LEN_INST);
        masklsb = (1 << len) -1;  
        mask = masklsb << pos;
        I->REGS[dst].v_i32[REGS_DATA] &= ~mask;
        I->REGS[dst].v_i32[REGS_DATA] |= (I->REGS[src1].v_i32[REGS_DATA] & masklsb) << pos;
        break;
        }
    // IIyyy-OPARDST_SRC1__LLLLLLPPPPPP  OPLJ_RDBF      move r2 r3 | lenK posK |
    case OPLJ_RDBF:
        {   
        uint32_t pos, len, masklsb;
        pos = RD(instruction, BITFIELD_POS_INST);
        len = RD(instruction, BITFIELD_LEN_INST);
        masklsb = (1 << len) -1;  
        I->REGS[src1].v_i32[REGS_DATA] >>= pos;
        I->REGS[dst].v_i32[REGS_DATA] = ((I->REGS[src1].v_i32[REGS_DATA] >> pos) & masklsb);
        break;
        }

    // IIyyy-OPARDST_SRC1______________  OPLJ_SWAP   
    case OPLJ_SWAP:
        reg = I->REGS[src1]; 
        I->REGS[src1] = I->REGS[dst]; 
        I->REGS[dst] = reg; break; 
        break;

    // IIyyy-OPARDST______________YYYYY  OPLJ_DELETE 
    case OPLJ_DELETE:
        I->ctrl.SP -= src2_K.i32;
        break;

    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_JUMP   
    case OPLJ_JUMP    : 
        I->ctrl.PC += src2_K.i32;   // JMP offset_K8, PUSH SRC1/SRC2/SRC3
        optional_push(I, dst); optional_push(I, src1); optional_push(I, src2); 
        break;

    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_BANZ                           
    case OPLJ_BANZ    : 
        I->REGS[RD(instruction,OP_SRC0_INST)].v_i32[REGS_DATA] --; // decrement loop counter
        if (I->REGS[src1].v_i32[REGS_DATA] != 0)             // see ti.com/lit/ds/symlink/tms320c25.pdf
        {   I->ctrl.PC += src2_K.i32;
        }
        break;

    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_CALL
    case OPLJ_CALL    : 
        reg.v_i32[REGS_DATA] = (1+ I->ctrl.PC);
        I->REGS[I->ctrl.SP] = reg;
        I->ctrl.SP ++;                      // push return address
        I->ctrl.PC += src2_K.i32;           // call
        optional_push(I, dst);  optional_push(I, src1); optional_push(I, src2); optional_push(I, src3); 
        break;

    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_CALLSYS
    case OPLJ_CALLSYS : // CALLSYS  {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
        {   
        const p_stream_al_services *al_func;
        al_func = &(I->S->al_services[0]);
        (*al_func)(PACK_SERVICE(0,0,PLATFORM_CLEAR_BACKUP_MEM,src2_K.i32), 
            (intPtr_t)(I->REGS[dst].v_i32[REGS_DATA]),  
            (intPtr_t)(I->REGS[src1].v_i32[REGS_DATA]),
            (intPtr_t)(I->REGS[src2].v_i32[REGS_DATA]), 
            I->REGS[src3].v_i32[REGS_DATA]);
        }
        break;

    case OPLJ_LABEL    : 
        I->REGS[dst].v_i32[REGS_DATA] = src2_K.i32;
        ST(I->REGS[dst].v_i32[REGS_TYPE], DTYPE_REGS1, DTYPE_INT32);
        break;

    default:
    case OPLJ_RETURN : break;
    }
}



/**
  @brief  test_arithmetic_operation

    FEDCBA9876543210FEDCBA9876543210
    IIyyy-OPARDST_SRC10000000000SRC2  eq,le,lt,ne,ge,gt,ld dst src1 src2
    IIyyy-OPARDST_SRC1xxxxxxxxxxxxxx  eq,le,lt,ne,ge,gt,ld dst src1 K
    IIyyy-OPARDST_SRC1xxxxxxxxx1TTTT  eq,le,lt,ne,ge,gt,ld dst src1 dtype + K-extented

*/
static void test_arithmetic_operation(arm_script_instance_t *I)
{
    int32_t dst, src1, src2;
    uint8_t t = I->ctrl.test_flag; 
    int32_t instruction = I->instruction;
    int32_t opcode = RD(instruction, OP_INST);
    int32_t opar = RD(instruction, OP_OPAR_INST);


    /* ONLY INTEGERS */

    readreg(I, &src2, RD(instruction, OP_SRC2_INST), 1);
    readreg(I, &src1, RD(instruction, OP_SRC1_INST), 0);
    int_arithmetic_operation(opcode, opar, &t, &dst, src1, src2);
    writreg(I, RD(instruction, OP_DST_INST), dst);

    I->ctrl.test_flag = t;
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

void arm_stream_script_interpreter (
    arm_script_instance_t *I,
    int32_t *descriptor,
    int32_t *bytecode)
{
    int32_t  cond, opcode, opar;

    while (I->ctrl.cycle_downcounter-- > 0)
    {
        // if (I->ctrl.cycle_downcounte == 1) { LOG ERROR "CYCLE OVERFLOW " }

        I->instruction = I->byte_code[I->ctrl.PC++];
        cond = RD(I->instruction, OP_COND_INST);

        /* conditional execution applies to all instructions */
        if ((cond == IF_YES) && (I->ctrl.test_flag == TEST_KO))
        {   continue;
        } else 
        if ((cond == IF_NOT) && (I->ctrl.test_flag == TEST_OK))
        {   continue;
        }

        opcode = RD(I->instruction, OP_INST);
        opar = RD(I->instruction, OP_OPAR_INST);

        if (opcode == OP_JMOV && opar == OPLJ_RETURN)
        {    I->ctrl.SP --; 
             I->ctrl.PC = (int32_t)I->REGS[I->ctrl.SP].v_i32[REGS_DATA];
             if (I->ctrl.PC == 0)
             {  return;
             }
        }

        if (opcode == OP_JMOV)
        {   jmov_operation(I);
        }
        else
        {   test_arithmetic_operation(I);
        }
    }
}

