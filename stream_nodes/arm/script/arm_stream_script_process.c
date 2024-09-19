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
static void arithmetic_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2);
static void read_register(arm_script_instance_t *I, int32_t *dst, int32_t src);
static void read_constant(arm_script_instance_t *I, int32_t k11);


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
    return returnPC;
}


/**
  @brief  push register content of src1/src2 on stack
*/
static void optional_push (arm_script_instance_t *I, int32_t src1, int32_t src2)
{
    if (src1 != Reg0) { I->REGS[I->SP++] = I->REGS[src1]; }
    if (src1 != Reg0) { I->REGS[I->SP++] = I->REGS[src2]; }
}

/**
  @brief  BRANCH / CALL / LABELS
*/
static uint8_t JMP_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2, int32_t K11)
{
    int8_t return_operation;
    regdata_t *r;

    return_operation = 0;

    //switch (opar)
    //{
    //case OPBR_JUMP    : I->PC = search_label(I, K11);   // JMP label_K11, PUSH SRC1/SRC2
    //                    optional_push(I, src1, src2);  
    //                    break;

    //case OPBR_JUMPA   : I->PC = I->REGS[dst].v_i32[REGS_DATA];    // JMPA  jump to computed address DST, PUSH SRC1/2
    //                    optional_push(I, src1, src2);  
    //                    break;

    //case OPBR_JUMPOFF : I->PC += K11;                   // JMP  Label_K11 signed offset  
    //                    break;

    //case OPBR_BANZ    : I->REGS[src1].v_i32[REGS_DATA] --;  // BANZ SRC1 Label_K11  "branch and decrement while non-zero 
    //                    if (I->REGS[src1].v_i32[REGS_DATA] != 0)             // see ti.com/lit/ds/symlink/tms320c25.pdf
    //                    {   I->PC = search_label(I, K11);
    //                    }
    //                    break;

    //case OPBR_CALL    : r = &(I->REGS[I->SP]);          // CALL Label_K11  and optional push 2 registers
    //                    r->v_i32[REGS_DATA] = (1+ I->PC);         
    //                    ST(r->v_i32[REGS_TYPE], REGS_DTYPE, DTYPE_INT32); 
    //                    I->SP ++;                       // push return address

    //                    I->PC = search_label(I, K11);   // branch to the new address
    //                    optional_push(I, src1, src2);  
    //                    break;

    //case OPBR_CALA    : r = &(I->REGS[I->SP]);
    //                    ST(r->v_i32[REGS_TYPE], REGS_DTYPE, DTYPE_INT32);
    //                    r->v_i32[REGS_DATA] = (1+ I->PC);         // CALA  call computed address DST, PUSH SRC1/2
    //                    I->SP++;

    //                    I->PC = I->REGS[dst].v_i32[REGS_DATA];    // branch (dst)
    //                    optional_push(I, src1, src2);  
    //                    break;

    //case OPBR_CALLSYS : // CALLSYS  {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
    //                    {   const p_stream_al_services *al_func;
    //                        al_func = &(I->S->al_services[0]);
    //                        (*al_func)(PACK_SERVICE(0,0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
    //                    }
    //                    break;

    //case OPBR_CALLSCRIPT : // CALLSCRIPT {K11} common scripts and node control   
    //                    {   const p_stream_al_services *al_func;
    //                        al_func = &(I->S->al_services[0]);
    //                        (*al_func)(PACK_SERVICE(0,0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
    //                    }
    //                    break;

    ////case OPBR_PUSHCTRL: I->REGS[I->SP].v_i32[REGS_DATA] = I->script_ctrl.test_flag; // PUSH CONTROL (flag and control bits) 
    ////                    ST(I->REGS[I->SP].v_i32[REGS_TYPE], REGS_DTYPE, DTYPE_CONTROL);
    ////                    I->SP++;
    ////                    break;

    //case OPBR_CALLAPP : // CALLAPP  {K11} 0K6=64 local callbacks 1K6= 64 global callbacks    
    //                    {   const p_stream_al_services *al_func;
    //                        al_func = &(I->S->al_services[0]);
    //                        (*al_func)(PACK_SERVICE(0,0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
    //                    }
    //                    break;

    //case OPBR_SAVEREG : break;

    //case OPBR_RETURN  : // RETURN  
    //                    if (I->SP == RegSP) // is it a return to the graph scheduler ?
    //                    {   return_operation = OPBR_RETURN;
    //                    } else
    //                    {   I->SP --; 
    //                        I->PC = (int32_t)I->REGS[I->SP].v_i32[REGS_DATA];
    //                    }
    //                    break;
    //default:
    //case OPBR_LABEL    : I->REGS[I->SP].v_i32[REGS_DATA] = search_label(I, K11); // PUSHLABEL  Label_K11 address on the stack
    //                    ST(I->REGS[I->SP].v_i32[REGS_TYPE], REGS_DTYPE, DTYPE_INT32);
    //                    I->SP++;
    //                    break;

    //}
    return (return_operation);
}




/**
  @brief  LOAD / STORE
*/
static void LD_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2)
{
    //int32_t *DST , *SRC1, *SRC2, t;

    //DST  = &(I->REGS[dst].v_i32[REGS_DATA]);
    //SRC1 = &(I->REGS[src1].v_i32[REGS_DATA]);
    //SRC2 = &(I->REGS[src2].v_i32[REGS_DATA]);
    //t = I->script_ctrl.test_flag;

    //switch (opar)
    //{
    //case OPLD_WR_BITF : // *DST (bitfield) = *SRC1 
    //                    {   uint32_t mask = (1 << I->script_ctrl.len) -1;  
    //                        *DST &= ~mask;
    //                        *DST |= (*SRC1 & mask) << I->script_ctrl.lsb;
    //                        break;
    //                    }
    //                   
    //case OPLD_RD_BITF : // *DST = *SRC (bitfield) 
    //                        *SRC1 >>= I->script_ctrl.lsb;
    //                        *DST = (*SRC1 & ((1 << I->script_ctrl.len) -1));
    //                        break;

    //case OPLD_BITFLD  : // BITFIELD START,STOP loads 12bits in "control" for *DST = *SRC (bitfield) 
    //                        I->script_ctrl.len = RD(*SRC2, CTRL_LEN);
    //                        I->script_ctrl.lsb = RD(*SRC2, CTRL_LSB);
    //                        break;
 
    //case OPLD_PTRINC  : // PTRINC *DST *SRC1 K11 = +/-1 +/-inc 
    //                        *DST = *SRC1 + *SRC2;
    //                        break;
    //                    
    //case OPLD_PTRMOD :  // PTRINC *DST +/-1 +/-inc +/-inc_modulo 
    //                    {   int32_t base = RD(I->REGS[dst].v_i32[REGS_DATA], PTR28BBASE_REGS);
    //                        int32_t size = RD(I->REGS[dst].v_i32[REGS_DATA], PTR28BSIZE_REGS);
    //                        *DST = *SRC1 + *SRC2;
    //                        while (*DST >  base + size)
    //                        {   *DST -= size;
    //                        }
    //                        break;
    //                    }
    //case OPLD_MODSET  : // SET PTRMOD *DST K11      set base6b/size5b of a PTR
    //                    {   int32_t base = RD(*SRC2, OP_K11_BASE);
    //                        int32_t size = RD(*SRC2, OP_K11_SIZE);
    //                        ST(I->REGS[dst].v_i32[REGS_TYPE], PTR28BBASE_REGS, base);
    //                        ST(I->REGS[dst].v_i32[REGS_TYPE], PTR28BSIZE_REGS, size);
    //                        break;
    //                    }
    //case OPLD_NORM    : //NORM *DST = normed on MSB(*SRC1), applied shift in *SRC2 
    //                    {   uint32_t count = 0U, mask = 1L << 31;  
    //                        while ((*SRC1 & mask) == 0U)
    //                        {
    //                          count += 1U;
    //                          mask = mask >> 1U;
    //                        }
    //                        *DST = *SRC1 << count;
    //                        I->REGS[*SRC2].v_i32[REGS_DATA] = count;
    //                        break;
    //                    }

    //case OPLD_WR_SP   : I->REGS[I->SP + *SRC2].v_i32[REGS_DATA] = *DST; break;     // WRSP *SP[*SRC2/K] = *DST 

    //case OPLD_RD_SP   : *DST = I->REGS[I->SP + *SRC2].v_i32[REGS_DATA]; break;     // RDSP *DST = *SP[*SRC2/K] 

    //case OPLD_SWAP    : I->REGS[*SRC1].v_i32[REGS_DATA] = *DST; *DST = *SRC1; break; // SWAP *SRC1, *DST 

    //default:    break;
    //}

    ///* Reg0 is used as *DST with OPC_TESTxxx instructions */
    //if (*DST != Reg0) 
    //{   I->REGS[*DST].v_i32[REGS_DATA] = *DST;
    //}
}



/**
  @brief  ALU
*/
static void arithmetic_operation(arm_script_instance_t *I, int32_t opar, int32_t dst, int32_t src1, int32_t src2)
{
    int32_t *DST , *SRC1, *SRC2, t;

    DST  = &(I->REGS[dst].v_i32[REGS_DATA]);        // @@@@ check dst/src are pointers and read their pointed data instead
    SRC1 = &(I->REGS[src1].v_i32[REGS_DATA]);       // @@@@ if DST/SRC = SP then apply stack rules 
    SRC2 = &(I->REGS[src2].v_i32[REGS_DATA]);
    t = I->script_ctrl.test_flag;

    switch (opar)
    {
    case  OPAR_ADD  : *DST = *SRC1 + *SRC2;                     break;
    case  OPAR_SUB  : *DST = *SRC1 - *SRC2;                     break;
    case  OPAR_MUL  : *DST = *SRC1 * *SRC2;                     break;
    case  OPAR_DIV  : *DST = (*SRC2 == 0) ? 0 : *SRC1 / *SRC2;  break;

    case  OPAR_MAX  : *DST = MAX(*SRC2, *SRC1);                 break;
    case  OPAR_MIN  : *DST = MIN(*SRC2, *SRC1);                 break;
    case  OPAR_AMAX : *DST = MAX(ABS(*SRC2), ABS(*SRC1));       break;
    case  OPAR_AMIN : *DST = MIN(ABS(*SRC2), ABS(*SRC1));       break;

    case  OPAR_OR   : *DST = *SRC1 | *SRC2;                     break;
    case  OPAR_NOR  : *DST = !(*SRC1 | *SRC2);                  break;
    case  OPAR_AND  : *DST = *SRC1 & *SRC2;                     break;
    case  OPAR_XOR  : *DST = *SRC1 ^ *SRC2;                     break;
    case  OPAR_SHR  : *DST = *SRC1 >> *SRC2;                    break;
    case  OPAR_SHL  : *DST = *SRC1 << *SRC2;                    break;
    case  OPAR_SET  : *DST = *SRC1 | (1<<*SRC2);                break;
    case  OPAR_CLR  : *DST = *SRC1 & (~(1<<*SRC2));             break;
    default: break;
    }    

    /* Reg0 is used as dst with OPC_TESTxxx instructions */
    if (dst != Reg0) 
    {   I->REGS[dst].v_i32[REGS_DATA] = *DST;
    }
    
    I->script_ctrl.test_flag = t;
}


/**
  @brief  read constant  (K)
*/
static void read_register(arm_script_instance_t *I, int32_t *dst, int32_t src)
{
    if (src == RegSP)           // access to the stack
    {   *dst = I->SP;
    }
    else if (src == RegSP1)    // access to stack -1
    {   *dst = (I->SP) + 1;
    }
    else 
    {   *dst = src; 
    }
}


/**
  @brief  read constant  (K)
*/
static void read_constant(arm_script_instance_t *I, int32_t k11)
{
    if (k11 < K_MAX)            /* little constants */
    {   I->REGS[RegK].v_i32[REGS_DATA] = k11;
        I->REGS[RegK].v_i32[REGS_TYPE] = DTYPE_INT;
    }
    else
    {   switch (k11)            /* several words for long constants */
        {
        case K_TIME64 : case K_FP64   : case K_INT64  :
          {
           //@@@@@@ Patch mantissa
            break;
          }
        case K_INT  : case K_UINT  : case K_FP32   : case K_TIME16 :
        case K_TIME32 : case K_PTR28B : case K_CHAR   : case K_CONTROL:
          {
            I->REGS[RegK].v_i32[REGS_DATA] = I->byte_code[I->PC++];
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
    int32_t  cond, opcode, opar, k11, dst, src1, src2;
    int32_t instruction;

    int32_t time, timeMax;
    extern void arm_stream_services (int32_t service_command, int32_t *ptr1, int32_t *ptr2, int32_t *ptr3, int32_t n);
    extern void * pack2linaddr_ptr(const intPtr_t *long_offset, int32_t data, int32_t unit);

    I->PC = 0;     // PC pre-incremented before read
    I->script_ctrl.test_flag = 0;
    time = 0;
    timeMax = 0xFFFFL;

    /*
    *  BYTECODE 
    *         v
    *         XXXXXXXXXXXXXXX
    * 
    *  INSTANCE (arc descriptor address = *script_instance
    *         |   
    *         v                    <--- nStack + 1 ------->
    *         R0 R1 R2 ..  nregs   R13  R14 R15             
    *         <--- registers--->   RegK SP  SP+1
    *  STACK                            [.................]
    *                                   SP init = nregs+2                  
    *                             
    *  HEAP / PARAM (4bytes/words)                                [............]
    */
    I->SP = RegSP;


    while (time++ < timeMax)
    {
        instruction = I->byte_code[I->PC++];
        cond = RD(instruction, OP_COND);
        opcode = RD(instruction, OP_INST);
        opar = RD(instruction, OP_OPAR);
        k11 = RD(instruction, OP_K11);
        read_register(I, &dst,  RD(instruction, OP_DST));   /* dst  */
        read_register(I, &src1, RD(instruction, OP_SRC1));  /* src1 */
        read_register(I, &src2, RD(instruction, OP_SRC2));  /* src2 */
        read_constant(I, k11);  /* load RegK */


        /* conditional execution applies to all instructions */
        if ((cond == IF_YES) && (I->script_ctrl.test_flag == 0))
        {   continue;
        } else 
        if ((cond == IF_NOT) && (I->script_ctrl.test_flag == 1))
        {   continue;
        }

        /* arithmetics and TEST operations */
        if (opcode < LAST_TEST_OPCODE)
        {   if (opcode < LAST_TEST_OPCODE_WITH_K)
            {   src2 = RegK;
            } 
            arithmetic_operation(I, opar, dst, src1, src2);
        }

        /* LD/ST operations */
        else if ((opcode == OP_LD) || (opcode == OP_LDK))
        {   if (opcode == OP_LDK) 
            {   src2 = RegK;
            }
            LD_operation(I, opar, dst, src1, src2);
        }

        /* LD/ST operations */
        else if ((opcode == OP_JMP) || (opcode == OP_LDK))
        {   if (OPBR_RETURN == JMP_operation(I, opar, dst, src1, src2, I->REGS[RegK].v_i32[REGS_DATA]))
            {   break;
            }
        }
    }
}


