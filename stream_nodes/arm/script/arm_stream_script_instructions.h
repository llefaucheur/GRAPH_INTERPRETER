/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_script.h
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
   

#ifndef carm_stream_script_instructions_h
#define carm_stream_script_instructions_h


/*
   Script format : 
        Scripts receive parameters:

        Instance memory area (from const.h)
            TX ARC descriptor: locks execution, 
                base address = instance, nb registers + (stack + heap)
                length = code length + byte code format
                read index = start of stack & start of parameters 
                write index = synchronization byte

    Registers 
        Instance static (ARC buffer) = 128 Bytes + processing = 150 Bytes
            14 r0..r13 data in FP E16M32 format or in Pointer format
               r10..r13 are preset as pointers to heap (PTR_MEMBANK_HEAP)
            1  r14 null register and used to load int/fp32 constant
            1  r15 is mapped to the stack index "SP"  of the instance
            T  internal flag, result of the tests 0=No, 1=Yes

    W32 script offset table[7 = 127 SCRIPT_LW0] to the byte codes 
        [SCRIPTSSZW32_GR1] = 
        |    nb_script x { word offset, byteCode Format, shared RAM, ARC } 
        |
        |    Flash at the offset position :
        |        Byte codes
        |        Parameters 

        RAM ARC descriptor 5 words (+test flag)
            
         
    INSTANCE (arc descriptor address = *script_instance
           |   
           v                 <---- nStack 64b---><--- heap 32b--->
           R0 R1 ... r13 r14 
           <---registers---> [..................][...............]
                               SP init = nregs


    FEDCBA9876543210FEDCBA9876543210
    III_____________________________  nop, if_yes / and_if, if_not, or_if
    ___yyy__________________________  8 opcode families (eq, le, lt, ne, ge, gt) + OP_ALU + OP_SETJUMP
    ______ OPAR_____________________  operation 5 bits
    ___________DST0abcSRC1abcSRC2abc  Destination(SRC0) and source registers(SRC1 & SRC2)/constants
    ___________XXXXXXX//////////////  SYSCALL K7 + 14bits field associated to each register to push
        
    3-bits fields (abc) following register name:
                registers               pointers/stack-R15          Plain constant=R14
        ab :    0,+/-1,extra_Int32      P[0,+/-sizeof,int32Bytes]   R14_10.0 int32 R14_11.0 float
        c  :    post-increment "+"      post-inc/(push/pop)         R14_10.1 int7  R14_11.1 int14 compact (inline)
                                        
        RKR = "P[n]+ / R / int/fp / sp[n]/pop/push"
        Px[RKR]+ = Rz                   Ry/k32, scatter
        Rz = Px[RKR]+                   Ry/k32, gather
        Px[k]+ = Py[k]+ OPAR Pz[k]+     k=0/+-1/k32, circular increment, each P of different type
        Rz = (Ry += K1) + (R14) K2
        S[1] = S[8] + S[1]+
         
    __________ DST0abcSRC1abc_______  RKR | LSB5 MSB5 | = RKR   write a bit-field 
    __________ DST0abcSRC1abc_______  RKR = RKR | LSB5 MSB5 |   read a bit-field 
    
    p --- MSB word ----------------> <----- LSB word --------------->  
    1HHH____SIZE(12   BASE(12___TYPE <------------------------------> data TYPE for pointer access (p=1)
    0_________________XXXXXXXX__TYPE <----either float or sint32----> (p=0) provision for extra matissa accuracy for integer operations
    FEDCBA9876543210FEDCBA987654321_ FEDCBA9876543210FEDCBA987654321_

    Encoded instructions : 

    forget  { } \ /
    comment ; 
    token   0x == <= => < > !=   r0..r14   p0..p14   [  ]   |  | 
    instructions 
            preamble : nothing, if_yes, and (if), or (if) / compact indexes
            tests : if .. <test> ..
            
            label move swap pop push call syscall jump banz return
            type typeptr base size setptr setreg 

    OP_TESTxx family                                                ab.c    ab.c    ab.c
    ---------------- REGS:ab=0/+-1/r ;exttype c=ext   PTR/STACK:ab=0/+-1/ext  c=inc    
     
    test_if r1  == r3 + 0x123  NO_COND_EXE OP_TESTEQU OPAR_NOP   R1-00.- R3-00.- R14-11._  0x00000123   R14 = K
    test_if r1  == -3.14       NO_COND_EXE OP_TESTEQU OPAR_NOP   R1-00.- R1411.- R14-00._  0xc048f5c3   R14 = none(00)
    test_if r1  != 0x123       NO_COND_EXE OP_TESTNEQ OPAR_NOP   R1-00.- R1411.- R14-00._  0x00000123
    and_if  12  <  r3          NO_COND_EXE OP_TESTLT  OPAR_NOP   R1410.- R3-00.- R14-00._  000000000C
    test_if r1  != r1          NO_COND_EXE OP_TESTEQU OPAR_NOP   R1-00.- R1-00.- --------  set the test flag to FALSE

    test_if p1[12] == p3[13] * p4[14]+      
                               NO_COND_EXE OP_TESTEQU OPAR_ADD   R1-11.0 R3-11.0- R4-11.1  000000E 000000D 000000C
    test_if p1[12] == p3[13] * p4[14]+      
                             NO_COND_EXE_PACK OP_TESTEQU OPAR_ADD   R1-11.0 R3-11.0- R4-11.1  00E0D0C

    and_if r1   == r2      +  s[2]  AND_IF OP_TESTEQU OPAR_ADD   R1-00.0 R2-00.0 R15-11.0  0x00002
    or_if  s[0] == p1[1]+  * -3.14  OR_IF  OP_TESTEQU OPAR_MUL   ST-00.0 P1-01.1 R14-11.-  0xc048f5c3 (= -3.14)

    if_yes p1[1] |  2 8 |  =  s[8]  IF_YES OP_ALU     OPAR_WR2BF P1-01.0 ST-11.0   2 8    0x00008
    if_yes p1[1] | 10 8 |  =  s[8]  IF_YES OP_ALU     OPAR_WR2BF P1-01.0 ST-11.0 __-__._  0x00A08 0x00008
    if_no  s[1]  = p1[9]+  | 10 8 | IF_NOT OP_ALU     OPAR_RDBF  ST-01.0 P1-11.1 __-__._  0x00009 0x00A08
    if_yes jump Label r1 r2         IF_YES OP_SETJUMP OPLJ_JUMP  R14-10.0 ___________XX_  <label>
    if_no  call relative r2 r3      IF_NOT OP_SETJUMP OPLJ_CALL  R14-10.1 __________XX__  <offset>

    OP_ALU family                  NO_COND_EXE             ab.c    ab.c    ab.c
    ---------------- REGS:ab=type c=ext   PTR/STACK:ab=0/+-1/ext  c=inc            inserted "EXT" from right to left
    r1     = r3     + 1            OP_ALU OPAR_ADD      R1-11.0 R3-01.0 __-__._
    r1     = 0x123                 OP_ALU OPAR_NOP      R1-11.0 RE-10.1 000123
    r1     = 0x1234                OP_ALU OPAR_NOP      R1-11.0 RE-10.0 __-__._  0x0001234
    r1     = r2     + pop          OP_ALU OPAR_ADD      R1-11.0 R2-11.0 ST-10.1   stack[0], postdecrement(read)
    r1     = r2     + stack        OP_ALU OPAR_ADD      R1-11.0 R2-11.0 ST-00.0   stack[0]
    r1     = r2     + stack[1]     OP_ALU OPAR_ADD      R1-11.0 R2-11.0 ST-01.0   stack[1] 
    r1     = r2     + stack[2]     OP_ALU OPAR_ADD      R1-11.0 R2-11.0 ST-11.0  0x00000002
    push   = p1[0]  * (-3.14)      OP_ALU OPAR_MUL      ST-01.1 P1-00.0 R0-11.0  0xc048f5c3 (= -3.14)   push=R15[1]+
    p1[7]  = p2[8] >> p3[9]        OP_ALU OPAR_LRSHFT   R1-11.0 R2-11.0 R3-11.0  0x00009 0x00008 0x00007 
    p1[12] = s[2]   | 8 10 |       OP_ALU OPAR_RDBF     P1-11.0 ST-11.0 __-__._  0x00002 0x0000C 0x00A08  
    p1[5]+ = p2[6]  * p3[7]        OP_ALU OPAR_MUL      P1-11.1 P2-11.0 P3-11.0  0x00007 0x00006 0x00005 
                                                                          
    p2 [r4] = r3                   OP_ALU OPLJ_SCATTER  R2-00.0 R4-00.0 R3-00.0
    p2 [r4[1]+]+ = r3              OP_ALU OPLJ_SCATTER  R2-00.1 R4-01.1 R3-00.0
    r2 = P4 [r3]                   OP_ALU OPAR_GATHER   R2-00.0 R4-00.0 R3-00.0
    r2 = P4 [3]      NO_COND_EXE_PACK OP_ALU OPAR_NOP      R2-00.0 P4-11.0  003
    r2 = P4 [1024]                 OP_ALU OPAR_NOP      R2-00.0 P4-11.0 R14 00.0 0x00400
    r2 <> r3                       OP_ALU OPAR_SWAP     R2-11.0 R3-11.0 R14-00.0
    p2[1]+ <> r3[-1]+              OP_ALU OPAR_SWAP     R2-01.1 R3-10.1 R14-00.0
    r2 = bswap r3 bitreverse       OP_ALU OPAR_BSWAP    R2-00.0 R3-00.0 code


    OP_SETJUMP family              NO_COND_EXE                ab.c    ab.c    ab.c
    ---------------- REGS:ab=type c=ext   PTR/STACK:ab=0/+-1/ext  c=inc             inserted "EXT" from right to left
    setptr p2 fp32                 OP_SETJUMP OPLJ_CASTREG R2-00.0            fp32  translate to register and cast
    setptr p2 base r5              OP_SETJUMP OPLJ_CASTREG R2-01.0      R5
    setptr p2 base K14             OP_SETJUMP OPLJ_CASTREG R2-01.1     K14
    setptr p2 size r5              OP_SETJUMP OPLJ_CASTREG R2-10.0      R5
    setptr p2 size K14             OP_SETJUMP OPLJ_CASTREG R2-10.1     K14

    setptr p2 fp32                 OP_SETJUMP OPLJ_CASTPTR R2-00.0            fp32  translate to register and cast
    setptr p2 base r5              OP_SETJUMP OPLJ_CASTPTR R2-01.0      R5
    setptr p2 base K14             OP_SETJUMP OPLJ_CASTPTR R2-01.1     K14
    setptr p2 size r5              OP_SETJUMP OPLJ_CASTPTR R2-10.0      R5
    setptr p2 size K14             OP_SETJUMP OPLJ_CASTPTR R2-10.1     K14

    jump   Label                   OP_SETJUMP OPLJ_JUMP    R14_00.0   <label>
    jump   Label r1 r2             OP_SETJUMP OPLJ_JUMP    R14_01.0   <label>     ___________XX_
    jump   relative r1 r2          OP_SETJUMP OPLJ_JUMP    R14_01.1  <offset>     ___________XX_ 
    call   Label                   OP_SETJUMP OPLJ_CALL    R14_0x.y   <label>

    syscall K7 r1 r2               OP_SETJUMP OPLJ_SYSCALL --K7--   ___________XX_
    banz   label r3-               OP_SETJUMP OPLJ_BANZ    R3 -11.1    <label>      post-decrement R3

    save   P3[2]+ R1 R2            OP_SETJUMP OPLJ_SAVE    R3 -11.1 ___________XX_  0x000000002  save to *R3
    push   R1 R2                   OP_SETJUMP OPLJ_SAVE    R15-11.1 ___________XX_               save on stack
    restore P3[-1]+ R1 R2          OP_SETJUMP OPLJ_RESTORE R15-10.1 ___________XX_  
    restore P3[1]+  R1 R2          OP_SETJUMP OPLJ_RESTORE R15-01.1 ___________XX_  restore in the reverse order
        pop         R1 R2          OP_SETJUMP OPLJ_RESTORE R15-10.1 ___________XX_

*/


#define RegNoneK     14 
#define cRegNoneK   "r14"
#define Stack    15             // [SP]

#define SCRIPT_REGSIZE 8            /* 8 bytes per register */

#define TEST_KO 0
#define TEST_OK 1

/* ------------ MSB REGISTERS --------------*/
                              
#define  PTR1_REGS0_MSB U(31) /*  1 REG=0 PTR=1  */
#define  PTR1_REGS0_LSB U(31) 
                              /* POINTER */
#define    PTRH_PTR_MSB U(30) /*  3 tells if the offset is to  */
#define    PTRH_PTR_LSB U(28) /*     absolute(HHH=00)/parameter(01)/heap(10)/graph(11) */

#define PTR_MEMBANK_ABS    0u
#define PTR_MEMBANK_PARAM  1u
#define PTR_MEMBANK_HEAP   2u
#define PTR_MEMBANK_GRAPH  3u

#define    BASE_PTR_MSB U(27) /* 12 Base */
#define    BASE_PTR_LSB U(16) 
#define    SIZE_PTR_MSB U(15) /* 12 Size */
#define    SIZE_PTR_LSB U( 4) 
#define   DTYPE_PTR_MSB U( 3) /*  4 DTYPE*/
#define   DTYPE_PTR_LSB U( 0) 

                              /* NOT POINTER */

#define    EXP_DATA_MSB U( 8) 
#define    EXP_DATA_LSB U( 0) /* 9 bits : IEEE-754 signed exponent is 8bits, we add the MSB "1" in mantissa */


/*---------------------------------------------------------------- BYTE-CODE INSTRUCTIONS FIELDS------
    FEDCBA9876543210FEDCBA9876543210
    III_____________________________  if_yes / and_if, if_not, or_if
    ___yyy__________________________  8 op - code(eq, le, lt, ne, ge, gt) + OP_ALU + OP_SETJUMP
    ______ OPAR_____________________  OPAR
    ___________DST0xyzSRC1xyzSRC2xyz  Destination(SRC0) and source registers(SRC1 & SRC2)

           registers       pointers/stack 
   ab :    /,/,FP32,I32    [0,+/-1, I32]
   c  :    unused          post-increment 
*/
#define   OP_COND_INST_MSB 31 /*    and_if / if_yes,  or_if, if_no */ 
#define   OP_COND_INST_LSB 29 /* 3  conditional fields */
#define        OP_INST_MSB 28       
#define        OP_INST_LSB 26 /* 3  instruction code TEST, LDJUMP, xx */
#define   OP_OPAR_INST_MSB 25       
#define   OP_OPAR_INST_LSB 21 /* 5  operand */
#define   OP_SRC0_INST_MSB 20       
#define   OP_SRC0_INST_LSB 14 /* 7  DST/SRC0 4bits of register and 3 bits for control */
#define   OP_SRC1_INST_MSB 13       
#define   OP_SRC1_INST_LSB  7 /* 7  SRC1 4bits for register and 3 bits for control */
#define   OP_SRC2_INST_MSB  6       
#define   OP_SRC2_INST_LSB  0 /* 7  SRC2 4bits for register and 3 bits for control */


/*------------------------------------   6543 21 0  ---------------*/
/*------------------------------------   rrrr ab.c  ---------------*/
#define  REG_INDEX_MSB  6       
#define  REG_INDEX_LSB  3 /* 4  register selection */
#define   AB_FIELD_MSB  2 
#define   AB_FIELD_LSB  1 /* 2 ab 0:INT, 1:FP32 2:INT64, 3:FP64 */
#define    C_FIELD_MSB  0       
#define    C_FIELD_LSB  0 /* 1 c  1=push on store and pop on read */

// registers ab_field
#define K_INT32 3
#define K_FP32  2
// Pointers  ab 0:P[0], 1:P[1], 2:P[-1], 3:P[EXT32]   c  1=post increment (w/wo circular addressing
#define K_INCREMENT 1
#define K_DECREMENT 2


/* BIT-FIELD second instruction */
#define   BITFMSB_INST_MSB 15       
#define   BITFMSB_INST_LSB  8 /* 8   MSB 0..31 */
#define   BITFLSB_INST_MSB  7       
#define   BITFLSB_INST_LSB  0 /* 8   LSB 0..31 */

/*-----------------------------------------OP_COND_INST------------------------------------------------------*/
#define NO_COND_EXE       0 // plain OP_INST 

#define IF_YES            1 // generic conditional execution
#define IF_NOT            2 // generic conditional execution
#define AND_IF            3 // only with OP_TESTxx (not compact)
#define OR_IF             4 // only with OP_TESTxx (not compact)

#define NO_COND_EXE_PACK  5 // indexes/data are all packed on sint8     
//test_if p1[12] == p3[13] * p4[14]+  NO_COND_EXE      OP_TESTEQU OPAR_NOP   R1-11.0 R3-11.0 R4-11.1  000000E 00000D 00000C 
//test_if p1[12] == p3[13] * p4[14]+  NO_COND_EXE_PACK OP_TESTEQU OPAR_NOP   R1-11.0 R3-11.0 R4-11.1  00E0D0C 

#define IF_YES_PACK       6 // generic conditional execution compact
#define IF_NOT_PACK       7 // generic conditional execution compact

/*-----------------------------------------OP_INST-----------------------------------------------------------*/

#define OP_TESTEQU        0 // == 
#define OP_TESTLEQ        1 // <=        
#define OP_TESTLT         2 // <         
#define OP_TESTNEQ        3 // !=                            
#define OP_TESTGEQ        4 // >=                            
#define OP_TESTGT         5 // >                            
#define OP_ALU            6 // load / store + ALU
#define OP_SETJUMP        7 // sets and jumps 

/*-----------------------------------------OP_ALU----------------------------------------------------------*/
// FEDCBA9876543210FEDCBA9876543210
// III_____________________________  if_yes / and_if, if_not, or_if
// ___yyy__________________________  8 op - code(eq, le, lt, ne, ge, gt) + OP_ALU + OP_SETJUMP
// ______ OPAR_____________________  OPAR
// ___________DST0xyzSRC1xyzSRC2xyz  Destination(SRC0) and source registers(SRC1 & SRC2)


#define OPAR_NOP          0 // ---    SRC2/K                          Ri = #K                            
#define OPAR_ADD          1 // +      SRC1 + SRC2 (or K)              PUSH: S=R+0  POP:R=S+R0   DUP: S=S+R0  DEL: R0=S+R0  DEL2: R0=S'+R0
#define OPAR_SUB          2 // -      SRC1 - SRC2 (or K)                  MOVI #K: R=R0+K
#define OPAR_MUL          4 // *      SRC1 * SRC2 (or K)
#define OPAR_DIV          5 // /      SRC1 / SRC2 (or K)              DIV  
#define OPAR_LRSHFT       7 // >>     SRC1 >> SRC2 (or K)             logical right shift (sign not propagated)
#define OPAR_OR           8 // |      SRC1 | SRC2 (or K)              if SRC is a pointer then it is decoded as *(SRC)
#define OPAR_NOR          9 // nor    !(SRC1 | SRC2) (or K)               example TEST (*R1) > (*R2) + 3.14   or   R1 = (*R2) + R4
#define OPAR_AND         10 // &      SRC1 & SRC2 (or K)
#define OPAR_XOR         11 // ^      SRC1 ^ SRC2 (or K)
#define OPAR_MAX         12 // max    MAX (SRC1, SRC2)                     
#define OPAR_MIN         13 // min    MIN (SRC1, SRC2)                      
#define OPAR_MOD         14 // %      SRC1 mod SRC2 (or K)             modulo             
#define OPAR_ADDMOD      15 // +%     SRC1 + SRC2 (or K) MODULO_DST    DST = OPAR SRC1 SRC2/K      
#define OPAR_SUBMOD      16 // -%     SRC1 - SRC2 (or K) MODULO_DST    works for PTR    
#define OPAR_SCATTER     17 // [PTR + src2/k8(y=1)/sp(k4)/REXT] = src1/sp(k4)/REXT/k8(x=1)   scatter
#define OPAR_GATHER      18 // [PTR + src2/k8(y=1)/sp(k4)/REXT]         gather
#define OPAR_WR2BF       19 // DST | LEN POS | = SRC1     write a bit-field
#define OPAR_RDBF        20 // DST = SRC1 | LEN POS |     read a bit-field
#define OPAR_SWAP        21 // - SRC1-- - SRC2-- - swap r2 r3                Swap registers
#define OPAR_BSWAP       22 // - SRC1-- - SRC2-- - byteswap r2 r3 XX         Byte swap ABCD->BADC / DCBA / BitReverse
#define OPAR_SQRT        23 // - DST -- - SRC1--   r2 = sqrt r3         invsqrt r3
#define OPAR_LOG10       24 // - DST -- - SRC1--   r2 = log10 r3/r4     20xlog10 r3/r4
#define OPAR_ABS         25 // - DST -- - SRC1--   r2 = abs r3
// SIN COS TAN ASIN ACOS ATAN, exp 10^x ln log, round up/down to 3 decimal, PI sqrt(2) sqrt(0.5), is_even/odd, 
// forced in this range (min/max), rescale y=ax+b, random integer in this range, toggle bit, little/big endian
// convert to int/float
// sprintf, strcmp, 

#define OPAR_NONE        32  

/*----------------------------------------OP_SETJUMP-------------------------------------------------------*/

#define OPLJ_SETREG       0 // setreg 
#define OPLJ_SETPTR       1 // setptr r2 / DTYPE_XXX / absolute 0 param 1 heap 2 graph 3 (PTR_MEMBANK_xx)
#define OPLJ_DELETE       2 // delete n from stack without save
#define OPLJ_JUMP         4 // jump signed {K7} and push registers
#define OPLJ_BANZ         5 // branch if non-zero to signed {K7} and decrement register (bitfield)
#define OPLJ_CALL         7 // call {K7} and push registers
#define OPLJ_SYSCALL      8 // syscall {K7} and push registers
#define OPLJ_SAVE         9 // save up to 14 registers
#define OPLJ_RESTORE     10 // restore up to 14 registers   
#define OPLJ_RETURN      11 // return {keep registers}

#define OPLJ_NONE        32  


/* parameters ( CMD, *, *, * )
*   CMD = command 12bits    Parameter 10bits            Parameter2 12bits
*           command         NodeID, arcID, function     Size, sub-library
*/                   
          
#define script_label "label"            


/* TYPE of register used as pointers --------------- DTYPE_REGS1--------------*/
#define DTYPE_INT8     0  /* i8 */
#define DTYPE_UINT8    1  /* u8 */  
#define DTYPE_INT16    2  /* i16 */
#define DTYPE_UINT16   3  /* u16 */
#define DTYPE_INT32    4  /* i32 */
#define DTYPE_UINT32   5  /* u32 */
#define DTYPE_TIME32   6  /* reserved */
#define DTYPE_INT64    7  /* i64*/
#define DTYPE_UINT64   8  /* u64 last integer type */
#define DTYPE_FP8_E4M3 9  /* fp8 reserved */
#define DTYPE_FP8_E5M2 10 /* fp8 reserved */
#define DTYPE_FP16     11 /* fp16 reserved */
#define DTYPE_FP32     12 /* fp32 */
#define DTYPE_FP64     13 /* fp64 reserved */
#define DTYPE_PTR28B   14 /* pointer with software MMU */


#endif  // if carm_stream_script_instructions_h

#ifdef __cplusplus
}
#endif
 
/*
    IEEE-754 float32:
              S<--E8--><--------M23----------> 32bits
              |EDCBA98|6543210FEDCBA9876543210   S x 2^(E8-127) x (1 + M23/2^23)
              00111111100000000000000000000000 = 1.0   = 2^(127-127) x (1 + 00 0000/80 0000) = 1 x 1.0
              00111111111111111111111111111111 = 1.999 = 2^(127-127) x (1 + 7F FFFF/80 0000) = 1 x 1.999
              01000000000000000000000000000000 = 2.0   = 2^(128-127) x (1 + 00 0000/80 0000) = 2 x 1.0
              01000000010000000000000000000000 = 3.0   = 2^(128-127) x (1 + 40 0000/80 0000) = 2 x 1.5
              MAX = 3.4E38    MIN = 1.2E-38

    Format allowing full 32bits accuracy (for bit masking) and float32 accuracy :
     01111111 01111111111111111111111111111111 = AMAX  = 2^127 x 0.999 = 1.701E38
     10000000 01000000000000000000000000000000 = AMIN  = 2^-128 x 0.5 = 1.47E-39

     <--E8--> <------------M32--------------->  floating point input
     76543210 FEDCBA9876543210FEDCBA9876543210   2^(signed E8) x (signed M32 >> 32)
     00000001 01000000000000000000000000000000 = 1.0   = 2^( 1 ) x (4000 0000/0 8000 0000) = 2 x 0.5
     00000001 01111111111111111111111111111111 = 1.999 = 2^( 1 ) x (7FFF FFFF/0 8000 0000) = 2 x 0.999
     00000010 01000000000000000000000000000000 = 2.0   = 2^( 2 ) x (4000 0000/0 8000 0000) = 4 x 0.5
     00000010 01100000000000000000000000000000 = 3.0   = 2^( 2 ) x (6000 0000/0 8000 0000) = 4 x 0.75

    <-- E8--> <------------M32--------------->  int32 input r1 = -3    r1 = 0x4321  r1 = 0x87654321
    000000010 01100000000000000000000000000000 =          3 =  4 x 0.75 =  4 x (0.5 + 0.25)
    000000010 11100000000000000000000000000000 =         -3 = -4 x 0.75 = -4 x (0.5 + 0.25)
    000010000 01000011001000010000000000000000 =     0x4321 = 4321 << 16
    000100000 10000111011001010100001100100001 = 0x87654321 = 87654321 << 32 + COMPILATION WARNING

    */