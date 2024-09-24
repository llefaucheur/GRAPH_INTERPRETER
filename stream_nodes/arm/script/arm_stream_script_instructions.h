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
   

#ifndef carm_stream_script_INSTRUCTIONS_H
#define carm_stream_script_INSTRUCTIONS_H


/*
   Script format : 
        Scripts receive parameters:
        - Label of Entry point or Flag "comparison OK" set during the RESET sequence, reset otherwise
                                                                                    1011
        Instance memory area (from const.h)
            TX ARC descriptor: locks execution, 
                base address = instance, nb registers + nb ptr/2
                length = code length + byte code format
                read index = start of stack & start of parameters in the other direction
                write index = synchronization byte

            stack and registers are preserved between calls 
            the conditional flag is set at reset to allow specific initialization
            type conversions are managed by the interpreter
                during calls, arithmetics operations..
            The initial SP position is preserved for fast return


    Registers 
        Instance static = R0(CTRL) + 10 registers (R1..R10) + (R11 = K) + (R12 = DST) + R13(free) | stack (R14=SP, R15-SP+1) 
        R0 means "not used" or "zero",    S' is the top-1 stack position 



    W32 script offset table[6b = 64 SCRIPT_LW0] to the byte codes 
        [SCRIPTSSZW32_GR1] = 
        |    nb_script x { word offset, byteCode Format, shared RAM, ARC } 
        |
        |    Flash at the offset position :
        |        { nb Label W32, nb data index W32, nregs }
        |        [ Labels offset in the code and parameters ]
        |        [ Data index in the RAM buffer ]
        |        Byte codes
        |        Parameters 

        RAM ARC descriptor 5 words
            
        ARC buffer 
        |    Registers 8 Bytes : R0 (state) .. R(nregs)
        |    Stack 8 Bytes 
        |    Heap 4 Bytes

    before/after nodes (can be reused for several nodes)



    Script instance = 
       
    
    
    BYTECODE XXXXXXXXXXXXXXX
    
    INSTANCE (arc descriptor address = *script_instance
           |   
           v                    <--- nStack + 1 ------->
           R0 R1 R2 ..  nregs   R13  R14 R15             
           <--- registers--->   RegK SP  SP+1
    STACK                            [.................]
                                     SP init = nregs+2                  
                               
    HEAP / PARAM (4bytes/words)                                [............]

    III_____________________________ if yes, if not, no test, break-point (+ margin)
    ___yyyyy________________________ 32 op-code 
    ________-OPAR___________________ 32 arithmetic op
    _____________DST.SRC1SRC2_______ 10 Registers(r1..r10) + sp(r14) + sp1(r15) + ctrl(r0) + rK(r13)
    _____________________xxxxxxxxxxx 11-bits Constant = +/- 0 .. 1000 / Bit field for {1000 < K < 1024}
    Bitfield_LEN              xxxxxx    BITFIELD_LEN_LSB
    Bitfield_POS         xxxxx          BITFIELD_POS_LSB
    
    registers format :                      DTYPE
                <---- MSB word ----------------> <---- LSB word ---------------->  
                FEDCBA9876543210FEDCBA987654____ FEDCBA9876543210FEDCBA987654321|  
    uint8       ____________________________   0 <------------------------------>  
    uint16      ____________________________   1 <------------------------------>  
    int16       ____________________________   2 <------------------------------>  
    uint32      ____________________________   3 <------------------------------>  used for R0 = 0
    int32       ____________________________   4 <------------------------------>  used for R0 = 0
    int64-4bits <-------------------------->   5 <------------------------------>  LSB are patched
    fp16        ____________________________   6 <------------------------------>  
    fp32        ____________________________   7 <------------------------------>  
    fp64        <-------------------------->   8 <------------------------------>  mantissa is patched
    TIME16      ____________________________   9 ________________<-------------->  
    TIME32      ____________________________  10 <------------------------------>  
    TIME64      <-------------------------->  11 <------------------------------>    
    28b + DTYPE ---BASE------SIZE-18------__  12 DTYPE<------ 28bits address---->  typed pointer + circular option (10+18)
*/    


#define  BASE_REGS1_MSB U(31) /* 10  Base */
#define  BASE_REGS1_LSB U(22) 
#define  SIZE_REGS1_MSB U(21) /* 18 Size */
#define  SIZE_REGS1_LSB U( 4) 
#define DTYPE_REGS1_MSB U( 3) /* 4  DTYPE*/
#define DTYPE_REGS1_LSB U( 0) 
                              
/*  Encoded instructions : 
     COND   INST    DST  LD-TST/ JMP  / MOV     SRC1  SRC2/#K
     0      1       2     ------- 3 -------     4     5  : field index
     ifyes  ld/k    0     add    jmp    wr2bf   0     0   + #iufdtc and constant 
     ifno   test/k  r1    min    cal    ptrinc  r1    r1
            jmp     r10   set                   r10   r10
            mov     sp           ret            sp    sp
            label   sp1                         sp1   sp1
                                                rk    rk

    remove  = { } 
    look for "."  "#"  [ ]  L_

    Assembler                          Binary encoding 
    ---------                          ----------------
    OP_TEST/TESTK family
      testleq r6 sp                      OP_TESTLEQ  R6 NOP R14 R0        
      testleq r2 mul { r4 r6 }	         OP_TESTLEQ  R2 MUL R4 R6 
      testleq r2 add { r4 #int 15}	     OP_TESTLEQK R2 ADD R4 #15        decode RK=15
      testleq r2 max { r4 #float 3.14}	 OP_TESTLEQK R2 MAX R4 DTYPE_W32  >RK=float on extra word
      testleq r2 max { r4 #double 3.141} OP_TESTLEQK R2 MAX R4 DTYPE_W64  >RK=double on two extra words

    OP_LD/OP_LDK family : DST = OP {SRC1 SRC2/K}
      ld r2 add { r4 r6 }                OP_LD R2 ADD R4 R6          R2_ptr = R4_ptr [R6]  DTYPE of R4
      ld r6 sp                           OP_LD R6 NOP R14 R0         pop
      ld r6 add { sp r1 }                OP_LD R6 ADD R14 R1         alu on stack
      ld r6 add { sp sp1 }               OP_LD R6 ADD R14 R15        alu on stack
      ls sp add { r1 r2 }                OP_LD R14 ADD R1 R2         push
      ld r2 mul { r4 r6 }	             OP_LD R2 MUL R4 R6          R2 = R4 + R6
      ld r6 #int 15                      OP_LDK R6 NOP RK #15        R6 = RK=15 
      ld r6 #ptr 6.97112    (off+@)      OP_LDK R6 NOP RK #6.97112   r6 = RK=Ptr
      ld r2 add { r4 #uint8 15}          OP_LDK R2 ADD R4 #15        decode OP_LDK : RK=15 + decode MUL
      ld r2 = add r4 15          
      ld r2 max { r4 #float 3.14}        OP_LDK R2 MAX R4 DTYPE_W32  OP_LDK1 : RK=float on extra word
      ld r2 max r4 3.14                  
      ld r2 max { r4 #double 3.14159}    OP_LDK R2 MAX R4 DTYPE_W64  OP_LDK1 : RK=double on two extra words
      ld r2 norm r3                      OP_LD R2 NORM R3 R0         normed on MSB(R2),
      ld r2 norm r3 r4                   OP_LD R2 NORM R3 R4         normed on MSB(R2), applied shift->R4


    OP_MOV family    DST field = 
      ld r2 type #float     OP_MOV R0 OPMV_CAST        R2 #DTYPE   DST_ptr = (DTYPE) 
      ld r2 typeptr #float  OP_MOV R0 OPMV_CASTPTR     R2 #DTYPE_8b DST_ptr = (POINTER DTYPE) + (DTYPE FLOAT)
      ld r4 base r5         OP_MOV R4 OPMV_BASE        R5 R0       R4_PTR.base = R5, base for cicular addressing
      ld r4 size r5         OP_MOV R4 OPMV_SIZE        R5 R0       R4_PTR.size = R5, size for cicular addressing
      ld r4 add r5 #INCTYPE OP_MOV R4 OPMV_PTRINC      R5 #INCTYPE R4_PTR = R5_PTR +/-1 +/-inc +/-inc_modulo 
      ld r2 [r4] r3         OP_MOV R2 OPMV_SCATTER     R3 R4       R2[R4] = R3    indirect, write with indexes 
      ld r2 [#k] r3         OP_MOV R2 OPMV_SCATTERK    R3 #K       R2[K] = R3     indirect, write with indexes 
      ld r2 r3 [r4]         OP_MOV R2 OPMV_GATHER      R3 R4       R2 = R3[R4]    indirect, read with indexes 
      ld r2 r3 [#k]         OP_MOV R2 OPMV_GATHERK     R3 #K       R2 = R3[K]     indirect, read with indexes 
      ld r2 [LEN.POS] r3    OP_MOV R2 OPMV_WR2BF       R3 #LEN.POS R2(bitfield) = R3 
      ld r2 r3 [LEN.POS]    OP_MOV R2 OPMV_RDBF        R3 #LEN.POS R2 = R3(bitfield)
      ld r2 swap r3         OP_MOV R0 OPMV_SWAP        R2 R3       SWAP SRC1, DST 

    OP_JMP family
      jump L_15             OP_JUMP R0 OPBR_JUMP       R0 #15      JMP LABEL K11 and PUSH SRC1/SRC2
      jump r4               OP_JUMP R4 OPBR_JUMPA      R0 R0       JMPA  jump to computed address DST, PUSH SRC1/SRC2
      jump -14              OP_JUMP R0 OPBR_JUMPOFF    R0 #-14     JMP signed_K11  signed word32 offset  and push 3 registers 
      banz r4 L_15          OP_JUMP R4 OPBR_BANZ       R0 #15      BANZ SRC1   Label_K11    
      call L15              OP_JUMP R0 OPBR_CALL       R0 #15      CALL LABEL K11 and PUSH SRC1/SRC2
      call r4               OP_JUMP R4 OPBR_CALA       R0 R0       CALA  call computed address DST, PUSH SRC1/SRC2
      call #15              OP_JUMP R0 OPBR_CALLSYS    R0 #15      CALLSYS   {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
      script 15             OP_JUMP R0 OPBR_CALLSCRIPT R0 #15      CALLSCRIPT{K11} common scripts and node control   
      callapp 15            OP_JUMP R0 OPBR_CALLAPP    R0 #15      CALLAPP   {K11} 0K6=64 local callbacks 1K6= 64 global callbacks    
      save 3,4,5            OP_JUMP R0 OPBR_SAVEREG    R0 #1Ch     push K11 registers
      restore 3,4,5         OP_JUMP R0 OPBR_RESTOREREG R0 #1Ch     pop k11 registers
      return                OP_JUMP R0 OPBR_RETURN     R0 R0       RETURN {keep registers 
      label R4 L_11         OP_JUMP R4 OPBR_LABEL      R0 #K       DST = Label_K15 (offset in code/parameter area (flash) or heap (RAM))

    Labels = L_<symbol> (no instruction)    
*/


//            typedef struct
//            {
//                arm_stream_instance_t *S;   
//                uint32_t *byte_code;
//                regdata_t *REGS;              /* registers and stack */
//                      typedef union
//                      {   char    c;       char v_c[8];
//                          int8_t  i8;    int8_t v_i8[8];
//                          int16_t i16;  int16_t v_i16[4];
//                          int32_t i32;  int32_t v_i32[2];
//                          int64_t i64;
//                          sfloat  f32;   sfloat v_f32[2]; 
//                          #define REGS_DATA 0
//                          #define REGS_TYPE 1
//                          sdouble f64;
//                      } regdata_t;
// 
//                struct
//                    unsigned int free : 3;
//                    unsigned int nregs : 4;
//                    unsigned int test_flag : 1;
//                    unsigned int PC : 8;
//                    unsigned int SP : 8;       /* in REGS unit */
//                    unsigned int cycle_downcounter : 8;
//                } ctrl;


#define Reg0     0          // NULL-data / "don't use" / Control regsiter
// Reg1 .. nRegs free to use for the programmer
 
#define RegK    13          // constant extracted from the code 
#define RegSP   14          // [SP]   "14" is dynamically remapped to [SP] position
#define RegSP1  15          // [SP+1]


#define SCRIPT_REGSIZE 8            /* 8 bytes per register */


/*---------------------------------------------------------------------------------------------------------*/
/*----- MAIN OPCODES ----   SYNTAX EXAMPLES                                      SRC1=R0   SRC2=R0   */
#define OP_TESTEQU    0  // TESTEQU DST OPAR SRC1 SRC2  TEST DST =  SRC1 OPAR SRC2 =  SRC2   =  SRC1
#define OP_TESTLEQ    1  //                             TEST DST <= SRC1 OPAR SRC2 <= SRC2   <= SRC1
#define OP_TESTLT     2  //                             TEST DST <  SRC1 OPAR SRC2 <  SRC2   <  SRC1
#define OP_TESTNEQ    3  //                             TEST DST != SRC1 OPAR SRC2 != SRC2   != SRC1
#define OP_TESTGEQ    4  //                             TEST DST >= SRC1 OPAR SRC2 >= SRC2   >= SRC1
#define OP_TESTGT     5  //                             TEST DST >  SRC1 OPAR SRC2 >  SRC2   >  SRC1

#define LAST_TEST_OPCODE_WITH_K 6

#define OP_TESTEQUK   6  // TESTEQU DST OPAR SRC1 #K    TEST DST =  SRC1 OPAR K    =  K      =  SRC1
#define OP_TESTLEQK   7  // TESTEQU DST MUL SRC1 #K     TEST DST <= SRC1 OPAR K    <= K      <= SRC1
#define OP_TESTLTK    8  // TESTEQU DST - #K            TEST DST <  SRC1 OPAR K    <  K      <  SRC1
#define OP_TESTNEQK   9  // TESTEQU DST - SRC1          TEST DST != SRC1 OPAR K    != K      != SRC1
#define OP_TESTGEQK  10  //                             TEST DST >= SRC1 OPAR K    >= K      >= SRC1
#define OP_TESTGTK   11  //                             TEST DST >  SRC1 OPAR K    >  K      >  SRC1

#define LAST_TEST_OPCODE 11

#define OP_LD        12  // LD DST OPAR SRC1 SRC2
#define OP_LDK       13  // LD DST OPAR SRC1 #K immediate

#define OP_JMP       14  // JMP, CALL, LABEL
                       
#define OP_MOV       15  // " various instructions "
/*---------------------------------------------------------------------------------------------------------*/

/* DTYPE */
#define DTYPE_UINT8    0
#define DTYPE_UINT16   1
#define DTYPE_INT16    2
#define DTYPE_UINT32   3
#define DTYPE_INT32    4
#define DTYPE_INT64    5
#define DTYPE_FP16     6
#define DTYPE_FP32     7
#define DTYPE_FP64     8
#define DTYPE_TIME16   9
#define DTYPE_TIME32  10
#define DTYPE_TIME64  11
#define DTYPE_PTR28B  12
                      
#define K_MAX  1011
#define K_UINT8  (K_MAX + DTYPE_UINT8 )   /*   0 K11 bit-fields */
#define K_UINT16 (K_MAX + DTYPE_UINT16)   //   1
#define K_INT16  (K_MAX + DTYPE_INT16 )   //   2
#define K_UINT32 (K_MAX + DTYPE_UINT32)   //   3
#define K_INT32  (K_MAX + DTYPE_INT32 )   //   4
#define K_INT64  (K_MAX + DTYPE_INT64 )   //   5
#define K_FP16   (K_MAX + DTYPE_FP16  )   //   6
#define K_FP32   (K_MAX + DTYPE_FP32  )   //   7
#define K_FP64   (K_MAX + DTYPE_FP64  )   //   8
#define K_TIME16 (K_MAX + DTYPE_TIME16)   //   9
#define K_TIME32 (K_MAX + DTYPE_TIME32)   //  10
#define K_TIME64 (K_MAX + DTYPE_TIME64)   //  11
#define K_PTR28B (K_MAX + DTYPE_PTR28B)   //  12

/*
*   K11 used for bitfields : LENGTH 5bits, POSITION 6bits
*/
#define BITFIELD_LEN_MSB 10
#define BITFIELD_LEN_LSB  6
#define BITFIELD_POS_MSB  5
#define BITFIELD_POS_LSB  0


/* ----- Condition bits fields ----- */
#define NO_TEST 0
#define IF_YES 1
#define IF_NOT 2


/* ----- byte-code instructions fields ----- */
#define OP_COND_MSB U(31) 
#define OP_COND_LSB U(30) /*  2 conditional fields */
#define OP______MSB U(29) 
#define OP______LSB U(29) /*  2 unused */
#define OP_INST_MSB U(28) 
#define OP_INST_LSB U(24) /*  5 instruction code */
#define OP_OPAR_MSB U(23) 
#define OP_OPAR_LSB U(19) /*  5 arithmetic operand */
#define  OP_DST_MSB U(18) 
#define  OP_DST_LSB U(15) /*  4 DST field */
#define OP_SRC1_MSB U(14) 
#define OP_SRC1_LSB U(11) /*  4 constant and SRC1 field */
#define OP_SRC2_MSB U(10) 
#define OP_SRC2_LSB U( 7) /*  4 constant and SRC2 field */

#define  OP_K11_MSB U(10) 
#define  OP_K11_LSB U( 0) /* 11 constant +-/1000 overlap SRC2 */
#define  OP_K15_MSB U(14) 
#define  OP_K15_LSB U( 0) /* 15 constant bitfields overlap SRC1 + SRC2 */
#define  OP_K19_MSB U(18) 
#define  OP_K19_LSB U( 0) /* 19 constant bitfields overlap SRC1 + SRC2 + DST */


/* --- K11 used for INCPTR  +/-1 +/-INCTYPE +/-inc_modulo  */
#define OP_K11_INCTYPE_MSB U(10) 
#define OP_K11_INCTYPE_LSB U(10) /* 1  */
#define   OP_K11_SINCK_MSB U( 9) 
#define   OP_K11_SINCK_LSB U( 0) /* 10  */

#define PTR_INCK  0     // ptr += K10 (signed constant)
#define PTR_CINCK 1     // ptr += K10 (circular buffer increment)


/*--- OP_ALU_TEST_/K ---        SYNTAX              */
#define OPAR_NOP      0     //  ---     SRC1 
#define OPAR_ADD      1     //  ADD     SRC1 + SRC2 (or K)              PUSH: S=R+0  POP:R=S+R0   DUP: S=S+R0  DEL: R0=S+R0  DEL2: R0=S'+R0
#define OPAR_SUB      2     //  SUB     SRC1 - SRC2 (or K)                  MOVI #K: R=R0+K
#define OPAR_MUL      3     //  MUL     SRC1 * SRC2 (or K)
#define OPAR_DIV      4     //  DIV     SRC1 / SRC2 (or K)              DIV  
#define OPAR_OR       5     //  OR      SRC1 | SRC2 (or K)              if SRC is a pointer then it is decoded as *(SRC)
#define OPAR_NOR      6     //  NOR     !(SRC1 | SRC2) (or K)               example TEST (*R1) > (*R2) + 3.14   or   R1 = (*R2) + R4
#define OPAR_AND      7     //  AND     SRC1 & SRC2 (or K)
#define OPAR_XOR      8     //  XOR     SRC1 ^ SRC2 (or K)
#define OPAR_SHR      9     //  SHR     SRC1 << SRC2 (or K)             SHIFT   
#define OPAR_SHL     10     //  SHL     SRC1 >> SRC2 (or K)
#define OPAR_SET     11     //  SET     SRC1 | (1 << SRC2 (or K))       BSET      
#define OPAR_CLR     12     //  CLR     SRC1 & (1 << SRC2 (or K))       BCLR     TESTBIT 0/R0 OPAR_SHIFT(SRC1, 1<<K5)
#define OPAR_MAX     13     //  MAX     MAX (SRC1, SRC2)                     
#define OPAR_MIN     14     //  MIN     MIN (SRC1, SRC2)                      
#define OPAR_AMAX    15     //  AMAX    AMAX (abs(SRC1), abs(SRC2)) 
#define OPAR_AMIN    16     //  AMIN    AMIN (abs(SRC1), abs(SRC2))
#define OPAR_NORM    17     //  NORM    normed on MSB(SRC1), applied shift in SRC2


/*----- OP_MOV ----*/
#define OPMV_CAST       0   // DST_ptr = (DTYPE) 
#define OPMV_CASTPTR    1   // DST_ptr = (POINTER DTYPE) 
#define OPMV_BASE       2   // R4_PTR.base = R5, base for cicular addressing
#define OPMV_SIZE       3   // R4_PTR.size = R5, size for cicular addressing
#define OPMV_PTRINC     4   // R4_PTR = R5_PTR +/-1 +/-inc +/-inc_modulo 
#define OPMV_SCATTER    5   // R2[R4] = R3    indirect, write with indexes 
#define OPMV_SCATTERK   6   // R2[K] = R3     indirect, write with indexes 
#define OPMV_GATHER     7   // R2 = R3[R4]    indirect, read with indexes 
#define OPMV_GATHERK    8   // R2 = R3[K]     indirect, read with indexes 
#define OPMV_WR2BF      9   // R2(bitfield) = R3 
#define OPMV_RDBF      10   // R2 = R3(bitfield)
#define OPMV_SWAP      11   // SWAP SRC1, DST 



/*----- OP_JUMP ----*/
#define OPBR_JUMP         0  // JMP LABEL K11 and PUSH SRC1/SRC2
#define OPBR_JUMPA        1  // JMPA  jump to computed address DST, PUSH SRC1/SRC2
#define OPBR_JUMPOFF      2  // JMP signed_K11  signed word32 offset  and push 3 registers 
#define OPBR_BANZ         3  // BANZ SRC1   Label_K11    
#define OPBR_CALL         4  // CALL LABEL K11 and PUSH SRC1/SRC2
#define OPBR_CALA         5  // CALA  call computed address DST, PUSH SRC1/SRC2
#define OPBR_CALLSYS      6  // CALLSYS   {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
#define OPBR_CALLSCRIPT   7  // CALLSCRIPT{K11} common scripts and node control   
#define OPBR_CALLAPP      8  // CALLAPP   {K11} 0K6=64 local callbacks 1K6= 64 global callbacks    
#define OPBR_SAVEREG      9  // push K11 registers
#define OPBR_RESTOREREG  10  // pop k11 registers
#define OPBR_RETURN      11  // RETURN {keep registers 
#define OPBR_LABEL       12  // DST = Label_K11 (code, parameter area (flash), heap (RAM))
/* parameters ( CMD, *, *, * )
*   CMD = command 12bits    Parameter 10bits            Parameter2 12bits
*           command         NodeID, arcID, function     Size, sub-library
*/                   

                  
#endif  // if carm_stream_script_INSTRUCTIONS_H

#ifdef __cplusplus
}
#endif
 
/*
 =========================================================================================

    Math library: Commonly used single-precision floating-point functions:
        Basic operations: ceilf, fabsf, floorf, fmaxf, fminf, fmodf, roundf, lroundf, remainderf
        Exponential/power functions: expf, log2f, powf, sqrtf
        Trigonometric/hyperbolic functions: sinf, cosf, tanf, asinf, acosf, atan2f, tanhf

    Default "CALLSYS" 
            sleep/deep-sleep activation
            system regsters access: who am I ?
            timer control (default implementation with SYSTICK)
            returns the PTR27 address of FIFO[read] FIFO[write]
   -----------------------------------------------

    SYSCALLS
    --------
  SYSCALL : 
    - Node Set/ReadParam/Run, patch a field in the header
    - FIFO read/set, data amount/free, debug reg read/set, time-stamp of last access (ASRC use-case)
    - Script TX buffer is starting with a table of indexes (Labels) to the data to address (SYSCALL return the address)
    - Update Format framesize, FS
    - Read Time 16/32/64 from arc/AL, compute differences,
    - Jump to +/- N nodes in the list,      => read HEADER + test fields + jump
      Un/Lock a section of the graph
    - Select new IO settings (set timer period, change gains)
    - AL decides deep sleep with the help of parameters (wake-up in X[s])
    - Trace "string" + data
    - Callbacks (cmd (cmd+LIbName+Subfunction , X, n) for I2C/Radio .. Libraries 
    - Callback returning the current use-case to set parameters
    - Call a relocatable binary section in the Param area
    - Share Physical HW addresses
    - Compute Median from data in a circular buffer + mean/STD

        reprogram the setting of stream_format_io[]
        //  format 6b control + 2b register
        //  load "t", from stack(i) = R(i) or #const
        //  move R(i) to/from arc FIFOdata / debugReg / with/without read index update
        //  compare R(i) with "t" : <> = != and skip next instruction
        //  jump to #label
        //  dsz decrement and jump on non-zero
        //  arithmetic add,sub,(AND,shift), #const/R(j)
        //  Basic DSP: moving average, median(5), Max(using VAD's forgetting factors).
        //  time difference, time comparison, change the setting of the timer (stop/restart)
        //  time elapsed from today, from a reference, from reset, UTC/local time
        //  computations on time-stamps
        //  default implementation with SYSTICK
        //  convert in ASCII format ISO 8601
        //  Modulo 60 function for the translation to mn. Wake me at 5AM.
        //  Activate timer 0.1s , 1s, 10s 1h 1D 1M
        //  Registers : 64bits(addressable in int8/16/32) + 8bits (type: time, temperature, pressure, 4xint16, counter-current-max)
        //  if {data arrived from the button queue}
        //  Registered callback for low-level operations for one or all instances
        //    fixed format f(cmd,ptr,x,n)
        //    example specific : IP address, password to share, Ping IP to blink the LED, read RSSI, read IP@
        //  Default callbacks: sleep/deep-sleep activation, timer control, who am I
        //    DAC/PWM/GPIO controlled with standard stream Arcs
        //  Low-level interface : Fill the I2C control string and callback
        //  Minimum services : average, timer, data formating/rescale/Interp, polling IOs
        //  Power meter process is using 3 phases x voltage, current, reactive power
        //  Save the state of a button (shutter button)
        //Command from arm_stream_command_interpreter() : return the code version number, ..  
*/