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
#include "platform.h"
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

static void float_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, sfloat *dst, sfloat src1, sfloat src2);
static void int_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, int32_t *dst, int32_t src1, int32_t src2);
static void test_arithmetic_operation(arm_script_instance_t *I);
static void readreg(arm_script_instance_t *I, regdata32_t *data, int32_t srcID, uint8_t K);
static void writreg(arm_script_instance_t *I, int32_t dstID, regdata32_t src, uint8_t dtype);
static void jmov_operation(arm_script_instance_t *I);


/*
*   arithmetics operations and test on floating point data
*/

static void float_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, sfloat *dst, sfloat src1, sfloat src2)
{
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
            {   uint32_t count = 0U, mask = 1UL << 31;  
                
                while ((F2I(src1) & mask) == 0U) 
                { count += 1U; mask = mask >> 1U; }

                tmp = (sfloat)((F2I(src1)) << count); 
                src2 = (sfloat)count;
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
}



/*
*   arithmetics operations and test on integer data
*/
static void int_arithmetic_operation(uint8_t opcode, uint8_t opar, uint8_t *t, int32_t *dst, int32_t src1, int32_t src2)
{
    int32_t tmp; 

    tmp = 0xFFFFFFFF;
    switch (opar)
    {
    case OPAR_NOP  : tmp = src2;                            break;
    case OPAR_ADD  : tmp = src1 + (src2);                   break;
    case OPAR_SUB  : tmp = src1 - (src2);                   break;
    case OPAR_MUL  : tmp = src1 * (src2);                   break;
    case OPAR_DIV  : tmp = (src2 == 0) ? 0 : src1 / src2;   break;
                  
    case OPAR_MAX  : tmp = MAX(src2, src1);                 break;
    case OPAR_MIN  : tmp = MIN(src2, src1);                 break;
    case OPAR_AMAX : tmp = MAX(ABS(src2), ABS(src1));       break;
    case OPAR_AMIN : tmp = MIN(ABS(src2), ABS(src1));       break;
                   
    case OPAR_OR   : tmp = src1 | src2;                     break;
    case OPAR_NOR  : tmp = !(src1 | src2);                  break;
    case OPAR_AND  : tmp = src1 & src2;                     break;
    case OPAR_XOR  : tmp = src1 ^ src2;                     break;
    case OPAR_SHR  : tmp = src1 >> src2;                    break;
    case OPAR_SHL  : tmp = src1 << src2;                    break;
    case OPAR_SET  : tmp = src1 | (1<<src2);                break;
    case OPAR_CLR  : tmp = src1 & (~(1<<src2));             break;
    case OPAR_NORM :  //  NORM *iDST = normed on MSB(*iSRC1), applied shift in *iSRC2 
            {   uint32_t count = 0U, mask = 1u << 31;  
                while ((src1 & mask) == 0U) { count += 1U; mask = mask >> 1U; }
                tmp = src1 << count; src2 = count;
            }
            break;
    }

    *t = 0;
    switch (opcode)
    {
    case OP_TESTEQU : if (*dst == tmp ) *t = 1; break;
    case OP_TESTLEQ : if (*dst <= tmp ) *t = 1; break;
    case OP_TESTLT  : if (*dst  < tmp ) *t = 1; break;
    case OP_TESTNEQ : if (*dst != tmp ) *t = 1; break;
    case OP_TESTGEQ : if (*dst >= tmp ) *t = 1; break;
    case OP_TESTGT  : if (*dst  > tmp ) *t = 1; break;
    }

    if (opcode == OP_LD)
    {   *dst = tmp;   
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
  @brief  read SRC2/K  (SP0/SP1/regs/K) to data->i32
          *data <- *(srcID)
          when reading stack with SP0 the stack pointer does not move
          when reading stack with SP1 the stack pointer is post-decremented after the move
*/
static void readsrc2K (arm_script_instance_t *I, regdata_t *data, uint8_t *src2ID)
{
    uint32_t instruction = I->instruction;

    *src2ID = (uint8_t) RD(instruction, OP_SRC2_INST);

    if (0 == RD(instruction, SRC2LONGK_PATTERN_INST))               // not a short constant ?
    {   
        if (1 == RD(instruction, OP_RKEXT_INST))                    // extended constant
        {   *src2ID = RegNone;
            data->v_i32[REGS_DATA] = I->byte_code[I->PC++];    // read next word of the program
            ST(data->v_i32[REGS_TYPE], DTYPE_REGS1, 
                RD(instruction, DTYPE_REGS1));                      // set the type
        } 
        else                                                        // use register src2 
        {   
            if (*src2ID == RegSP0)                                  // simple stack read
            {   *data = I->REGS[I->SP];                        //  read data and type 
            }
            else if (*src2ID == RegSP1)                             // pop data (SP --)
            {   *data = I->REGS[I->SP];                        //  read data and type 
                I->SP --;
                if (I->SP < I->nregs)                     // check stack underflow
                {   I->errors |= ERROR_STACK_UNDERFLOW;
                    I->SP ++;
                }
            }
            else                                                    // read register data
            {   *data = I->REGS[*src2ID];                           //  read data and type 
            }
        }
    }
    else 
    {   *src2ID = RegNone;
        data->i32 = RD(instruction, OP_K_INST)-UNSIGNED_K_OFFSET;   // 12bits signed short constant
        ST(data->v_i32[REGS_TYPE], DTYPE_REGS1, DTYPE_INT32);       //  signed int32
    }        
}



/**
  @brief  read register
          *data <- *(srcID)
*/
static void readreg(arm_script_instance_t *I, regdata32_t *data, int32_t srcID, uint8_t K)
{
    uint32_t instruction = I->instruction;

    if (K)
    {   if (0 == RD(instruction, SRC2LONGK_PATTERN_INST))
        {   if (1 == RD(instruction, OP_RKEXT_INST))    // extended constant
            {   data->i32 = I->byte_code[I->PC++];
                switch (RD(instruction, DTYPE_REGS1))
                {
                case DTYPE_UINT8 : case DTYPE_UINT16: case DTYPE_INT16 : case DTYPE_UINT32: 
                case DTYPE_INT32 : case DTYPE_TIME32: case DTYPE_PTR28B: 
                    /* k = reg_src2K.v_i32[REGS_DATA].i32; */
                    break;
                case DTYPE_FP16  : case DTYPE_FP32  : //case DTYPE_FP64  : 
                    /* kl.f32 */
                    break;
                //case DTYPE_INT64 : case DTYPE_TIME64: 
                //    /* read one more word */
                //    data->i32 = I->byte_code[I->PC++];
                //    break;
                }
            } 
            else                                        // use register src2 
            {   
                if (srcID == RegSP0)                    // simple stack read
                {   data->i32 = I->REGS[I->SP].v_i32[REGS_DATA];
                }
                else if (srcID == RegSP1)               // pop data (SP --)
                {   data->i32 = I->REGS[I->SP].v_i32[REGS_DATA]; 
                    I->SP --;
                }
                else                                    // read register data
                {   data->i32 = I->REGS[srcID].v_i32[REGS_DATA];
                }
            }
        }
        else 
        {   data->i32 = RD(instruction, OP_K_INST);     // 14bits signed short constant
            data->i32 = (data->i32) - UNSIGNED_K_OFFSET; // [-2016  2048]
        }        
    }
    /* read src2 */
    else
    {
        data->i32 = I->REGS[srcID].v_i32[REGS_DATA];
    }
}


/**
  @brief  write to dstID register 
        operation : 
          (*dstID)  <-  *(src data) + dtype
*/
static void writreg(arm_script_instance_t *I, int32_t dstID, regdata32_t src, uint8_t dtype)
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

    pdst->v_i32[REGS_DATA] = src.i32;
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
    //  test SRC2=SP1 on OPLJ_BASE OPLJ_SIZE OPLJ_SCATTER OPLJ_GATHER OPLJ_CALLSYS
    if (opar == OPLJ_BASE || opar == OPLJ_SIZE || opar == OPLJ_SCATTER || opar == OPLJ_GATHER ||opar == OPLJ_CALLSYS)
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
    readsrc2K(I, &reg_src2K, &src2);
    K = reg_src2K.v_i32[REGS_DATA]; // can be float ..


    switch (opar)
    {
    // data type cast
    // IIyyy-OPARDST_______<--K12-0SRC2     OPLJ_CAST(PTR)  dst  dtype
    case OPLJ_CAST: 
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

        /* cast the source to the type of the destination to allow by te addressing */ 
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
        case DTYPE_UINT32:case DTYPE_INT32:case DTYPE_FP32:case DTYPE_TIME32:case DTYPE_PTR28B: nbytes = 4; break;
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
        I->SP -= K;
        break;

    // jump to an address and optional save 2 registers : jump label R1
    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_JUMP   
    case OPLJ_JUMP    : 
        I->PC += K-1;   // JMP offset_K8, PUSH SRC1/SRC2/SRC3, PC was already post incremented
        optional_push(I, dst); optional_push(I, src1); 
        break;

    // decrement a register and branch is not null : banz label R1
    // IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_BANZ         
    // see ti.com/lit/ds/symlink/tms320c25.pdf
    case OPLJ_BANZ    : 
        I->REGS[RD(instruction,OP_DST_INST)].v_i32[REGS_DATA] --;   // decrement loop counter
        if (I->REGS[src1].v_i32[REGS_DATA] != 0)   
        {   I->PC += K-1;
        }
        break;

    // call (return address push on the stack) and save registers:    call label R1
    // IIyyy-OPARDST_SRC1__<--K12-0SRC2  OPLJ_CALL
    case OPLJ_CALL    : 
        reg_src2K.v_i32[REGS_DATA] = (1+ I->PC);
        I->REGS[I->SP] = reg_src2K;
        I->SP ++;                      // push return address
        I->PC += K-1;           // call
        optional_push(I, dst);  optional_push(I, src1);  
        break;

    // system call : callsys 63 r1 r2 r3 r4
    // IIyyy-OPARDST_SRC1SRC3SRC4<-K6->  OPLJ_CALLSYS
    case OPLJ_CALLSYS : // CALLSYS  {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
        {   
        const p_stream_al_services *al_func;
        uint8_t K_service = (uint8_t)RD(instruction, CALLSYS_K_INST); 
        uint8_t src3 = (uint8_t)RD(instruction, OP_SRC3_INST); 
        uint8_t src4 = (uint8_t)RD(instruction, OP_SRC4_INST); 

        al_func = &(I->S->al_services);
        (*al_func)(PACK_SERVICE(0,0,NOTAG_SSRV, PLATFORM_CLEAR_BACKUP_MEM, K_service), 
            (void *)(I->REGS[dst].v_i32[REGS_DATA]),  
            (void *)(I->REGS[src1].v_i32[REGS_DATA]),
            (void *)(I->REGS[src3].v_i32[REGS_DATA]),
            (uint32_t)(I->REGS[src4].v_i32[REGS_DATA])
            );
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
        I->PC = I->REGS[I->SP++].v_i32[REGS_DATA];
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
static void test_arithmetic_operation(arm_script_instance_t *I)
{
    regdata32_t dst, src1, src2;
    uint8_t t = (uint8_t)(I->test_flag); 
    int32_t instruction = I->instruction;
    int8_t opcode = (uint8_t)RD(instruction, OP_INST);
    int8_t opar = (uint8_t)RD(instruction, OP_OPAR_INST);

    /* ONLY INTEGERS */
    {   uint8_t db1, db2, dbdst;
        db1 = (uint8_t)RD(instruction, OP_SRC1_INST);
        db2 = (uint8_t)RD(instruction, OP_SRC2_INST);
        dbdst=(uint8_t)RD(instruction, OP_DST_INST);
        db2 = db2;

        readreg(I, &src2, RD(instruction, OP_SRC2_INST), 1);
        readreg(I, &src1, RD(instruction, OP_SRC1_INST), 0);
        readreg(I, &dst,  RD(instruction, OP_DST_INST) , 0);
        int_arithmetic_operation(opcode, opar, &t, &(dst.i32), src1.i32, src2.i32);
        writreg(I, RD(instruction, OP_DST_INST), dst, DTYPE_INT32);
    }

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
    int32_t  cond, opcode, opar, count;
    count = I->max_cycle;

    while (count-- > 0)
    {
        // if (I->cycle_downcounte == 1) { LOG ERROR "CYCLE OVERFLOW " }

        I->instruction = I->byte_code[I->PC++];
        cond = RD(I->instruction, OP_COND_INST);

        /* conditional execution applies to all instructions */
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
               I->PC = (int32_t)I->REGS[I->SP].v_i32[REGS_DATA];
            }
        }

        if (opcode == OP_JMOV)
        {   jmov_operation(I);
        }
        else        /* OP_LD  +  OP_TESTxxx */
        {   test_arithmetic_operation(I);
        }
    }
}

#ifdef __cplusplus
 }
#endif

#endif //CODE_ARM_STREAM_SCRIPT