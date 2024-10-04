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

static int32_t search_label(arm_script_instance_t *I, int32_t label_to_seach);
static uint8_t JMP_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2, int32_t K11);
static void LD_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2);
static void arithmetic_operation(arm_script_instance_t *I, int32_t opcode, int32_t opar, int32_t dst, int32_t src1, int32_t src2);
static void read_register(arm_script_instance_t *I, int32_t *dst, int32_t src);
static void read_constant(arm_script_instance_t *I, int32_t k11);

#if 0

/**
  @brief  ALU
*/
static void arithmetic_operation(arm_script_instance_t *I, int32_t opcode, int32_t opar, int32_t dst, int32_t src1, int32_t src2)
{
    int32_t *DST , *SRC1, *SRC2, *TMP, t;
    int8_t DSTtype, SRC1type, SRC2type;
    regdata_t tmp; 


    /* ---------------------------------------- ONLY INT32 DTYPE    No pointer No stack ---------------------------- */
    TMP  = &(tmp.v_i32[REGS_DATA]);       
    DST  = &(I->REGS[dst].v_i32[REGS_DATA]);      
    SRC1 = &(I->REGS[src1].v_i32[REGS_DATA]);       
    SRC2 = &(I->REGS[src2].v_i32[REGS_DATA]);

    DSTtype  = RD((I->REGS[dst].v_i32[REGS_TYPE]), DTYPE_REGS1);        
    SRC1type = RD((I->REGS[src1].v_i32[REGS_DATA]), DTYPE_REGS1);      
    SRC2type = RD((I->REGS[src2].v_i32[REGS_DATA]), DTYPE_REGS1);
    t = I->ctrl.test_flag;

    switch (opar)
    {
    case OPAR_ADD  : *TMP = *SRC1 + (*SRC2);                   break; //  ADD     SRC1 + SRC2 (or K) PUSH: S=R+0  POP:R=S+R0   DUP: S=S+R0  DEL: R0=S+R0  DEL2: R0=S'+R0
    case OPAR_SUB  : *TMP = *SRC1 - (*SRC2);                   break; //  SUB     SRC1 - SRC2 (or K)     MOVI #K: R=R0+K
    case OPAR_MUL  : *TMP = *SRC1 * (*SRC2);                   break; //  MUL     SRC1 * SRC2 (or K)
    case OPAR_DIV  : *TMP = (*SRC2 == 0) ? 0 : *SRC1 / *SRC2;  break; //  DIV     SRC1 / SRC2 (or K) DIV  
                  
    case OPAR_MAX  : *TMP = MAX(*SRC2, *SRC1);                 break; //  MAX     MAX (SRC1, SRC2)           
    case OPAR_MIN  : *TMP = MIN(*SRC2, *SRC1);                 break; //  MIN     MIN (SRC1, SRC2)           
    case OPAR_AMAX : *TMP = MAX(ABS(*SRC2), ABS(*SRC1));       break; //  AMAX    AMAX (abs(SRC1), abs(SRC2))
    case OPAR_AMIN : *TMP = MIN(ABS(*SRC2), ABS(*SRC1));       break; //  AMIN    AMIN (abs(SRC1), abs(SRC2))
                  
    case OPAR_OR   : *TMP = *SRC1 | *SRC2;                     break; //  OR      SRC1 | SRC2 (or K)        if SRC is a pointer then it is decoded as *(SRC)
    case OPAR_NOR  : *TMP = !(*SRC1 | *SRC2);                  break; //  NOR     !(SRC1 | SRC2) (or K)         example TEST (*R1) > (*R2) + 3.14   or   R1 = (*R2) + R4
    case OPAR_AND  : *TMP = *SRC1 & *SRC2;                     break; //  AND     SRC1 & SRC2 (or K)        
    case OPAR_XOR  : *TMP = *SRC1 ^ *SRC2;                     break; //  XOR     SRC1 ^ SRC2 (or K)        
    case OPAR_SHR  : *TMP = *SRC1 >> *SRC2;                    break; //  SHR     SRC1 << SRC2 (or K)       SHIFT   
    case OPAR_SHL  : *TMP = *SRC1 << *SRC2;                    break; //  SHL     SRC1 >> SRC2 (or K)       
    case OPAR_SET  : *TMP = *SRC1 | (1<<*SRC2);                break; //  SET     SRC1 | (1 << SRC2 (or K)) BSET      
    case OPAR_CLR  : *TMP = *SRC1 & (~(1<<*SRC2));             break; //  CLR     SRC1 & (1 << SRC2 (or K)) BCLR     TESTBIT 0/R0 OPAR_SHIFT(SRC1, 1<<K5)
    case OPAR_NORM    :                                               //NORM *DST = normed on MSB(*SRC1), applied shift in *SRC2 
         {   uint32_t count = 0U, mask = 1L << 31;  
             while ((*SRC1 & mask) == 0U)
             { count += 1U;
               mask = mask >> 1U;
             }
             *DST = *SRC1 << count;
             I->REGS[*SRC2].v_i32[REGS_DATA] = count;
             break;
         }
    default: break;
    }    
#if 0
    switch (opcode)
    {
    case OP_TESTEQU :  case OP_TESTEQUK :  t = (*TMP == *DST); break;
    case OP_TESTLEQ :  case OP_TESTLEQK :  t = (*TMP <= *DST); break;
    case OP_TESTLT  :  case OP_TESTLTK  :  t = (*TMP  < *DST); break;
    case OP_TESTNEQ :  case OP_TESTNEQK :  t = (*TMP != *DST); break;
    case OP_TESTGEQ :  case OP_TESTGEQK :  t = (*TMP >= *DST); break;
    case OP_TESTGT  :  case OP_TESTGTK  :  t = (*TMP  > *DST); break;
    }

    if (opcode > LAST_TEST_OPCODE)
    {   *DST = *TMP;    //@@@
    }

    /* Reg0 is used as dst with OPC_TESTxxx instructions */
    if (dst != Reg0) 
    {   I->REGS[dst].v_i32[REGS_DATA] = *DST;
    }
    
    I->ctrl.test_flag = t;
}


/**
  @brief  find a Label and jump
*/
static int32_t search_label(arm_script_instance_t *I, int32_t label_to_seach) 
{
    int32_t icode;
    int32_t opcode, opar;
    int32_t instruction, K11, returnPC;

#define MAX_CODE 100
    returnPC = 0;

    for (icode = 0; icode < MAX_CODE; icode++)
    {
        instruction = I->byte_code[icode];
        opcode = RD(instruction, OP_INST);
        opar = RD(instruction, OP_OPAR);
        K11 = RD(instruction, OP_K11);              /* for OP_JUMP */

        /* find and skip instructions depending on their size */
        if (opcode == OP_TESTEQUK || opcode == OP_TESTLEQK || opcode == OP_TESTLTK || 
            opcode == OP_TESTNEQK || opcode == OP_TESTGEQK || opcode == OP_TESTGTK || opcode ==  OP_LDK)
        {   if (K11 > K_MAX)
            {   icode = icode +1;                   /* read one more word for long constants */       
            }
        }
         
        if (opcode == OP_JMP && opar == OPBR_LABEL)
        {   if (K11 == label_to_seach)
            {   returnPC = icode +1;                   /* label found ! : jump to this address+1 */
                break;
            }
        }
    }
#endif
    return returnPC;
}


/**
  @brief  push register content of src1/src2 on stack
*/
static void optional_push (arm_script_instance_t *I, int32_t src1, int32_t src2)
{
    if (src1 != Reg0) { I->REGS[I->ctrl.SP++] = I->REGS[src1]; }
    if (src1 != Reg0) { I->REGS[I->ctrl.SP++] = I->REGS[src2]; }
}

/**
  @brief  BRANCH / CALL / LABELS
*/
static uint8_t JMP_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2, int32_t K11)
{
    int8_t return_operation;
    regdata_t *r;

    return_operation = 0;

    switch (opar)
    {
    case OPBR_JUMP    : I->ctrl.PC = search_label(I, K11);   // JMP label_K11, PUSH SRC1/SRC2
                        optional_push(I, src1, src2);  
                        break;

    case OPBR_JUMPA   : I->ctrl.PC = I->REGS[dst].v_i32[REGS_DATA];    // JMPA  jump to computed address DST, PUSH SRC1/2
                        optional_push(I, src1, src2);  
                        break;

    case OPBR_JUMPOFF : I->ctrl.PC += K11;                   // JMP  Label_K11 signed offset  
                        break;

    case OPBR_BANZ    : I->REGS[src1].v_i32[REGS_DATA] --;  // BANZ SRC1 Label_K11  "branch and decrement while non-zero 
                        if (I->REGS[src1].v_i32[REGS_DATA] != 0)             // see ti.com/lit/ds/symlink/tms320c25.pdf
                        {   I->ctrl.PC = search_label(I, K11);
                        }
                        break;

    case OPBR_CALL    : r = &(I->REGS[I->ctrl.SP]);          // CALL Label_K11  and optional push 2 registers
                        r->v_i32[REGS_DATA] = (1+ I->ctrl.PC);         
                        ST(r->v_i32[REGS_TYPE], DTYPE_REGS1, DTYPE_INT32); 
                        I->ctrl.SP ++;                       // push return address

                        I->ctrl.PC = search_label(I, K11);   // branch to the new address
                        optional_push(I, src1, src2);  
                        break;

    case OPBR_CALA    : r = &(I->REGS[I->ctrl.SP]);
                        ST(r->v_i32[REGS_TYPE], DTYPE_REGS1, DTYPE_INT32);
                        r->v_i32[REGS_DATA] = (1+ I->ctrl.PC);         // CALA  call computed address DST, PUSH SRC1/2
                        I->ctrl.SP++;

                        I->ctrl.PC = I->REGS[dst].v_i32[REGS_DATA];    // branch (dst)
                        optional_push(I, src1, src2);  
                        break;

    case OPBR_CALLSYS : // CALLSYS  {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
                        {   const p_stream_al_services *al_func;
                            al_func = &(I->S->al_services[0]);
                            (*al_func)(PACK_SERVICE(0,0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
                        }
                        break;

    case OPBR_CALLSCRIPT : // CALLSCRIPT {K11} common scripts and node control   
                        {   const p_stream_al_services *al_func;
                            al_func = &(I->S->al_services[0]);
                            (*al_func)(PACK_SERVICE(0,0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
                        }
                        break;

    case OPBR_CALLAPP : // CALLAPP  {K11} 0K6=64 local callbacks 1K6= 64 global callbacks    
                        {   const p_stream_al_services *al_func;
                            al_func = &(I->S->al_services[0]);
                            (*al_func)(PACK_SERVICE(0,0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
                        }
                        break;

    case OPBR_SAVEREG : break;

    case OPBR_RESTOREREG : break;

    case OPBR_RETURN  : // RETURN  
                        if (I->ctrl.SP == RegSP) // is it a return to the graph scheduler ?
                        {   return_operation = OPBR_RETURN;
                        } else
                        {   I->ctrl.SP --; 
                            I->ctrl.PC = (int32_t)I->REGS[I->ctrl.SP].v_i32[REGS_DATA];
                        }
                        break;
    default:
    case OPBR_LABEL    : I->REGS[I->ctrl.SP].v_i32[REGS_DATA] = search_label(I, K11); // DST = Label_K11 (code, parameter area (flash), heap (RAM))
                        ST(I->REGS[I->ctrl.SP].v_i32[REGS_TYPE], DTYPE_REGS1, DTYPE_INT32);
                        I->ctrl.SP++;
                        break;

    }
    return (return_operation);
}




/**
  @brief  LOAD / STORE
*/
static void MOV_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2)
{
    int32_t *DST , *SRC1, *SRC2, t;

    DST  = &(I->REGS[dst].v_i32[REGS_DATA]);
    SRC1 = &(I->REGS[src1].v_i32[REGS_DATA]);
    SRC2 = &(I->REGS[src2].v_i32[REGS_DATA]);
    t = I->ctrl.test_flag;

    switch (opar)
    {
    case OPMV_CAST    : break;   // DST_ptr = (DTYPE) 
    case OPMV_CASTPTR : break;   // DST_ptr = (POINTER DTYPE) 
    case OPMV_BASE    : break;   // R4_PTR.base = R5, base for cicular addressing
                        {   ST(I->REGS[dst].v_i32[REGS_TYPE], BASE_REGS1, I->REGS[RegK].v_i32[REGS_DATA]);
                            break;
                        }
    case OPMV_SIZE    : break;   // R4_PTR.size = R5, size for cicular addressing
                        {   ST(I->REGS[dst].v_i32[REGS_TYPE], SIZE_REGS1, I->REGS[RegK].v_i32[REGS_DATA]);
                            break;
                        }
    case OPMV_PTRINC  : break;   // R4_PTR = R5_PTR +/-1 +/-INCTYPE +/-inc_modulo 
                        {   int32_t inc = RD(I->REGS[RegK].v_i32[REGS_DATA], OP_K11_SINCK);
                            int32_t inctype = RD(I->REGS[RegK].v_i32[REGS_DATA], OP_K11_INCTYPE);
                            if (PTR_INCK == inctype)
                            {   *DST += inc;
                            }
                            else
                            {   /* cicular increment */
                                int32_t base, size;
                                base = RD(I->REGS[RegK].v_i32[REGS_TYPE], BASE_REGS1);
                                size = RD(I->REGS[RegK].v_i32[REGS_TYPE], SIZE_REGS1);
                                *DST += inc;
                                if (*DST > base) {*DST -= inc; }
                                if (*DST < base) {*DST += inc; }
                            }
                        }
    case OPMV_SCATTER : break;   // R2[R4] = R3    indirect, write with indexes 
    case OPMV_SCATTERK: break;   // R2[K] = R3     indirect, write with indexes 
    case OPMV_GATHER  : break;   // R2 = R3[R4]    indirect, read with indexes 
    case OPMV_GATHERK : break;   // R2 = R3[K]     indirect, read with indexes 
    case OPMV_WR2BF   : break;   // R2(bitfield) = R3 
                        {   uint32_t pos, len, mask, masklsb;
                            pos = RD(I->REGS[RegK].v_i32[REGS_TYPE], BITFIELD_POS);
                            len = 1+ RD(I->REGS[RegK].v_i32[REGS_TYPE], BITFIELD_LENM1);
                            masklsb = (1 << len) -1;  
                            mask <<= pos;
                            *DST &= ~mask;
                            *DST |= (*SRC1 & masklsb) << pos;
                            break;
                        }
    case OPMV_RDBF    :     // R2 = R3(bitfield)
                        {   uint32_t pos, len, masklsb;
                            pos = RD(I->REGS[RegK].v_i32[REGS_TYPE], BITFIELD_POS);
                            len = 1+ RD(I->REGS[RegK].v_i32[REGS_TYPE], BITFIELD_LENM1);
                            masklsb = (1 << len) -1;  
                            *SRC1 >>= pos;
                            *DST = (*SRC1 & masklsb);
                            break;
                        }
    case OPMV_SWAP    : I->REGS[*SRC1].v_i32[REGS_DATA] = *DST; *DST = *SRC1; break;   // SWAP SRC1, DST 

    default:    break;
    }

    /* Reg0 is used as *DST with OPC_TESTxxx instructions */
    if (*DST != Reg0) 
    {   I->REGS[*DST].v_i32[REGS_DATA] = *DST;
    }
}


/**
  @brief  read constant  (K)
*/
static void read_register(arm_script_instance_t *I, int32_t *dst, int32_t src)
{
    if (src == RegSP)           // access to the stack
    {   *dst = I->ctrl.SP;
    }
    else if (src == RegSP1)    // access to stack -1
    {   *dst = (I->ctrl.SP) + 1;
    }
    else 
    {   *dst = src; 
    }
}


/**
  @brief  read constant  (K)

  registers format :                      DTYPE
              <---- MSB word ----------------> <---- LSB word ---------------->  
              FEDCBA9876543210FEDCBA987654____ FEDCBA9876543210FEDCBA987654321|  
  uint8       ____________________________   0 <------------------------------>  
  int32       ____________________________   4 <------------------------------>  used for R0 = 0
  int64-4bits <-------------------------->   5 <------------------------------>  LSB are patched
   
*/
static void read_constant(arm_script_instance_t *I, int32_t k11)
{
    if (k11 < K_MAX)            /* little constants */
    {   I->REGS[RegK].v_i32[REGS_DATA] = k11;
        I->REGS[RegK].v_i32[REGS_TYPE] = DTYPE_INT32;
    }
    else
    {   switch (k11)            /* several words for long constants */
        {
            case K_TIME64 : case K_FP64   : case K_INT64  :
            {
              I->REGS[RegK].v_i32[REGS_DATA] = I->byte_code[I->ctrl.PC++];
              I->REGS[RegK].v_i32[REGS_TYPE] = I->byte_code[I->ctrl.PC++];
              ST(I->REGS[RegK].v_i32[REGS_TYPE], DTYPE_REGS1, k11 - K_MAX);
              break;
            }
            case K_INT32  : case K_UINT32  : case K_FP32   : case K_TIME16 :
            case K_TIME32 : case K_PTR28B :
            {
              I->REGS[RegK].v_i32[REGS_DATA] = I->byte_code[I->ctrl.PC++];
              I->REGS[RegK].v_i32[REGS_TYPE] = k11 - K_MAX;
              break;
            }
        }
    }
}


/**
  @brief         arm_stream_script : 16bits virtual machine, or Cortex-M0 binary codes
  @param[in]     pinst      instance of the component
  @param[in]     reset      tells to set the conditional flag before the call
  @return        status     finalized processing
                 
  @par  
    FIFOTX buffer = regs[IDX_REG_MAX] + stack + scratch/parameters
       stack     | 0| 1| 2| 3|    Push => stack     | X| 1| 2| 3|
         SP         |                        SP           |
         SP=IDX_REG_MAX                      SP=IDX_REG_MAX +1
*/

void arm_stream_script_interpreter (
    arm_script_instance_t *I,
    int32_t *descriptor,
    int32_t *bytecode,
    int32_t *ram)
{
#if 0
    int32_t  cond, opcode, opar, k11, dst, src1, src2;
    int32_t instruction;

    extern void arm_stream_services (int32_t service_command, int32_t *ptr1, int32_t *ptr2, int32_t *ptr3, int32_t n);
    extern void * pack2linaddr_ptr(const intPtr_t *long_offset, int32_t data, int32_t unit);

    while (I->ctrl.cycle_downcounter-- > 0)
    {
        // if (I->ctrl.cycle_downcounte == 1) { LOG ERROR "CYCLE OVERFLOW " }

        instruction = I->byte_code[I->ctrl.PC++];
        cond = RD(instruction, OP_COND);
        opcode = RD(instruction, OP_INST);
        opar = RD(instruction, OP_OPAR);
        k11 = RD(instruction, OP_K11);
        read_register(I, &dst,  RD(instruction, OP_DST));   /* dst  */
        read_register(I, &src1, RD(instruction, OP_SRC1));  /* src1 */
        read_register(I, &src2, RD(instruction, OP_SRC2));  /* src2 */
        read_constant(I, k11);  /* load RegK */

        /* conditional execution applies to all instructions */
        if ((cond == IF_YES) && (I->ctrl.test_flag == 0))
        {   continue;
        } else 
        if ((cond == IF_NOT) && (I->ctrl.test_flag == 1))
        {   continue;
        }

        /* arithmetics and TEST operations */
        if (opcode < LAST_TEST_OPCODE)
        {   if (opcode < LAST_TEST_OPCODE_WITH_K)
            {   src2 = RegK;
            } 
            arithmetic_operation(I, opcode, opar, dst, src1, src2);
        }

        /* LD/ST operations */
        else if ((opcode == OP_LD) || (opcode == OP_LDK))
        {   if (opcode == OP_LDK) 
            {   src2 = RegK;
            }
            MOV_operation(I, opar, dst, src1, src2);
        }

        /* LD/ST operations */
        else if ((opcode == OP_JMP) || (opcode == OP_LDK))
        {   if (OPBR_RETURN == JMP_operation(I, opar, dst, src1, src2, I->REGS[RegK].v_i32[REGS_DATA]))
            {   break;
            }
        }
    }
#endif
}

#endif
