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

        Instance memory area (from const.h)
            TX ARC descriptor: locks execution, 
                base address = instance, nb registers + (stack + heap)
                length = code length + byte code format
                read index = start of stack & start of parameters 
                write index = synchronization byte

     Registers 
        Instance static = 12 registers (R0..r11) stack (R13=SP, R14-SP+1) 

    W32 script offset table[6b = 64 SCRIPT_LW0] to the byte codes 
        [SCRIPTSSZW32_GR1] = 
        |    nb_script x { word offset, byteCode Format, shared RAM, ARC } 
        |
        |    Flash at the offset position :
        |        Byte codes
        |        Parameters 

        RAM ARC descriptor 5 words (+test flag)
            
        ARC buffer 
        |    Registers 8 Bytes :  R0 .. r11
        |    Stack 8 Bytes 
        |    Heap 4 Bytes

    BYTECODE XXXXXXXXXXXXXXX
    

    INSTANCE (arc descriptor address = *script_instance
           |   
           v                 <---- nStack  ----> <--- heap --->
           R0 R1 R2 ..   r11 
           <---registers---> [..................][............]
                               SP init = nregs


    FEDCBA9876543210FEDCBA9876543210
    II______________________________ if yes, if not, no test, break-point (+ margin)
    __yyy___________________________  8 op-code (eq,le,lt,ne,ge,gt) + ldjmp  + xxx 
    _____-OPARDST_SRC1xxxxxxxxxxxxxx  OPLJ_HEAP OPLJ_PARAM OPLJ_BASE OPLJ_SIZE OP_LD OP_TEST OPLJ_SCATTER OPLJ_GATHER OPLJ_CAST OPLJ_JUMP OPLJ_BANZ OPLJ_CALL OPLJ_CALLSYS (save 2 regs)
                      xx<----K12--->  signed_K(sw) = unsigned_K12(32..4095)-2048 = [2048 .. -2016]
    __________________0000000000SRC2  detection of 0000.000_.____ = (X & FE0) => either SRC2 or K-long
    __________________0000000001TTTT  DTYPE + extra word (and DTYPE
    )
    __________DST_SRC1__LLLLLLPPPPPP  OPLJ_WR2BF OPLJ_RDBF 6+6-bits(Len Pos) 
    __________SRC0SRC1SRC3SRC4__SRC2  SAVE/RESTORE 5 Registers(r0..r11)

    IIyyy-OPARDST_SRC1__<--K12-0SRC2

                <---- MSB word ----------------> <---- LSB word ---------------->  
                FEDCBA9876543210FEDCBA987654TYPE FEDCBA9876543210FEDCBA987654321|  
    uint8       _______H<---BASE----SIZE--->   0 000000000000000000000000<------>  computed as int32
    uint16      _______H<---BASE----SIZE--->   1 0000000000000000<-------------->  'H1C0' offset to heap(1) or code(0)
    int16       _______H<---BASE----SIZE--->   2 ssssssssssssssss<-------------->  
    uint32      _______H<---BASE----SIZE--->   3 <------------------------------>  computed as int32 !
    int32       _______H<---BASE----SIZE--->   4 <------------------------------>  
    int64       unused
    fp16        _______H<---BASE----SIZE--->   6 <------------------------------>  translated to FP32
    fp32        _______H<---BASE----SIZE--->   7 <------------------------------>  
    fp64        unused
    TIME16      0000000000000000000000000000   9 0000000000000000<-------------->  
    TIME32      0000000000000000000000000000  10 <------------------------------>  
    TIME64      <-------------------------->  11 <------------------------------> DTYPE moved to MSB
    28b + DTYPE _______H<---BASE----SIZE--->  12 TYPE<------ 28bits address-----> typed pointer + circular option (10+10)

    ---------SYSCALL--------
    callsys 63 r1 r2 r3 r4           ; OP_JMOV CALLSYS   R1 R2 R3 R4 + K@
    callsys 63 r1 r2 r3    12 4 128  ; OP_JMOV CALLSYS   R1 R2 R3    + K@  +  { 12 4 128 }

    code        description
     0          Scheduling control, 
     1 .. 10    Application callbacks (cmd * * * n)
    11 .. 20    arc-descriptor access; 12 read data; 13 write data
                14 node parameters read; 15 update parameters w/wo reset
                16 node init/reset; 17 node's graph update
    21 .. 30    Update IO parameters, Stop/reset
    31 .. 40    Compute library
*/

/* ------------ MSB REGISTERS --------------*/

#define  _____REGS1_MSB U(31) /*  7 unused  */
#define  _____REGS1_LSB U(25) 
#define  H1C0_REGS1_MSB U(24) /*  1 R2[5] = R4 : tells if the offset is to code 0 or heap 1 (arc buffer) */
#define  H1C0_REGS1_LSB U(24) 
#define  BASE_REGS1_MSB U(23) /* 10 Base */
#define  BASE_REGS1_LSB U(14) 
#define  SIZE_REGS1_MSB U(13) /* 10 Size */
#define  SIZE_REGS1_LSB U( 4) 
#define DTYPE_REGS1_MSB U( 3) /* 4  DTYPE*/
#define DTYPE_REGS1_LSB U( 0) 
                              
/*  Encoded instructions : 

    forget  { } \ / 
    comment ; 
    token   # =  r0..r11 sp sp1   [ ind ]   | bits | 
    instructions testleq..testlt if_yes if_no 
            move swap pop push save restore call callsys jump banz return
            type typeptr base size 
    ALU     nop add addmod sub submod mul duv or nor and xor shr shl set clr max min amax amin norm 
    type    int8 int16 int32 float16 float double ptrfloat ptruint8 ptrint32.. 

    Assembler                          Binary encoding 
    ---------                          -------------------------------------
    OP_TESTxx family                   
    test_equ r1 nop    r3/type k      ; OP_TESTEQU OPAR_NOP  R1   R2   R3/RK                         
    test_leq r1 add r2 r3/type k      ; OP_TESTLEQ OPAR_ADD 
    test_lt  r1 sub r2 r3/type k      ; OP_TESTLT  OPAR_SUB 
    test_neq r1 mul r2 r3/type k      ; OP_TESTNEQ OPAR_MUL 
    test_geq r1 div r2 r3/type k      ; OP_TESTGEQ OPAR_DIV 
    test_gt  r1 or  r2 r3/type k      ; OP_TESTGT  OPAR_OR  
                      
    OP_LD family                   
    r1 =        r3/type k             ; OP_LD      OPAR_NOP  R1   R2   R3                          
    r1 = add r2 r3/type k             ; OP_LD      OPAR_ADD 
    r1 = sub r2 r3/type k             ; OP_LD      OPAR_SUB 
    r1 = mul r2 r3/type k             ; OP_LD      ..etc..
                                      
    OP_JMOV family                            OPAR      DST  SRC1 SRC2 /RK
 -    set r2 type #float            0 OP_JMOV OPLJ_CAST     R2   ___  RK=DTYPE_FP32=7
 -    set r2 typeptr #float         1 OP_JMOV OPLJ_CASTPTR  R2   ___  RK=DTYPE_FP32=7
 -    set r4 base r5/k              2 OP_JMOV OPLJ_BASE     R4   ___  RK=base for circular addressing / loops
 -    set r4 size r5/k              3 OP_JMOV OPLJ_SIZE     R4   ___  RK=size for circular addressing
      set r4 param K12              4 OP_JMOV OPLJ_PARAM    R4   ___  RK  set H1C0=0
      set r4 heap K12               5 OP_JMOV OPLJ_HEAP     R4   ___  RK  set H1C0=1
      set r4 graph K12              6 OP_JMOV OPLJ_GRAPH    R4   ___  RK  set H1C0=1
      [r2 r4/k]+ = r3               7 OP_JMOV OPLJ_SCATTER  R2   R3   R4/RK=immediate R2[R4] = R3 cast to destination format 
      r2 = [r3]+ r4/k               8 OP_JMOV OPLJ_GATHER   R2   R3   R4/RK=immediate R2 = R3[R4] cast from source format
      move r2 | lenK posK | r3      9 OP_JMOV OPLJ_WR2BF    R2   R3   K12 6b + 6b   bit-field
      move r2 r3 | lenK posK |     10 OP_JMOV OPLJ_RDBF     R2   R3   K12 6b + 6b   bit-field
-     swap sp r3                   11 OP_JMOV OPLJ_SWAP     SP   R3   
-     delete r4/K                  12 OP_JMOV OPLJ_DELETE   ___  ___  R4/RK move SP
      jump k r1 r2                 13 OP_JMOV OPLJ_JUMP     R1/N R2/N RK  
      banz L_replaced_Label r1 r2  14 OP_JMOV OPLJ_BANZ     R1/N R2/N RK
      call L_replaced_Label r1 r2  15 OP_JMOV OPLJ_CALL     R1/N R2/N RK
      callsys 63 r1 r2 r3 r4       16 OP_JMOV OPLJ_CALLSYS  R1/N R2/N R3/N R4/N K6
      save r3 r0 r11               17 OP_JMOV OPLJ_SAVE     R1/N R2/N R3/N R4/N R5/N push up to 5 registers 
      restore r3 r0 r11            18 OP_JMOV OPLJ_RESTORE  R1/N R2/N R3/N R4/N R5/N pop up to 5 registers 
      return                       19 OP_JMOV OPLJ_RETURN   ___  ___  ___
                       
      Examples :                      
                                      
      test_leq r6 sp                  ; OP_TESTLEQ  NOP   R6   ___  R14      
      test_leq r6 #int 3              ; OP_TESTLEQ  NOP   R6   ___  RK=0x1E03  1E=immediate => K +/-127     
      test_leq r6 #float 3.14         ; OP_TESTLEQ  NOP   R6   ___  RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word
      test_leq r2 mul {r4 r6 }        ; OP_TESTLEQ  MUL   R2   R4   R6 
      test_leq r2 add {r4 #int 3}     ; OP_TESTLEQ  ADD   R2   R4   RK=0x1E03  1E=immediate => K +/-127
      test_leq r2 max {r4 #float 3.1} ; OP_TESTLEQ  MAX   R2   R4   RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word
      test_leq r2 max {r4 #double 3.1}; OP_TESTLEQ  MAX   R2   R4   RK=0x1F07  1F=extension DTYPE = 7 = translated to FP32 + 1 word

      r5 = #ptrfloat 6 97112          ; OP_LD       NOP  R5    ___  RK=0x1F0C  1F=extension DTYPE = C = type + pointer28b (+1 word_
      if_no r6 = #int 3               ; IF OP_LD    NOP  R6    ___  RK=0x1E03  1E=immediate => K +/-127
      r6 = #float 3.14                ; OP_LD       NOP  R6    ___  RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word      
      r2 = mul { r4 r6 }              ; OP_LD       MUL  R2    R4   R6 
      r2 = add { r4 #int 3}           ; OP_LD       ADD  R2    R4   RK=0x1E0F  1E=immediate => K +/-127
      r2 = max { r4 #float 3.14}      ; OP_LD       MAX  R2    R4   RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word
      r2 = max { r4 #double 3.14}     ; OP_LD       MAX  R2    R4   RK=0x1F07  1F=extension DTYPE = 7 = translated to FP32 + 1 word
      r6 = sp                         ; OP_LD       NOP  R6    ___  R13      
      r5 = add { sp r1 }              ; OP_LD       ADD  R5    R13  R1          alu on stack
      r5 = add { sp sp1 }             ; OP_LD       ADD  R5    R13  R15         alu on stack
      sp = add { r1 r2 }              ; OP_LD       ADD  R13   R1   R2          push sp++
      r2 = add { r4 #uint8 3}         ; OP_LD       ADD  R2    R4   RK          RK=0x1E03  1E=immediate => K +/-127
      r2 = max { r4 #float 3.14}      ; OP_LD       MAX  R2    R4   DTYPE_W32   OP_LDK1 : RK=float on extra word
      r2 = max r4 3.14                ; OP_LD                                  
      r2 = max { r4 #double 3.14159}  ; OP_LD       MAX  R2    R4   DTYPE_W64   OP_LDK1 : RK=double on two extra words
      r2 = norm r3                    ; OP_LD       NORM R2    R3   R0         normed on MSB(R2),
      r2 = norm r3 r4                 ; OP_LD       NORM R2    R3   R4         normed on MSB(R2), applied shift->R4
      r2 = addmod r4 3                ; OP_LD    ADDMOD  R2    R4   RK=0x1E03  1E=immediate => K +/-127
      r2 = add { r4 r5 }              ; OP_LD       ADD  R2    R4   R5         R2 = R4_ptr_28b [r5]  DTYPE of R4

    Labels = <symbol, not Rii, not a number> (no instruction)    
*/

/*  <<<--------<<- analysis direction
    S1 = S1 + S1    read+pop  read+pop   write+push
    S1 = S1 + S0    read      read+pop   write+push
    S0 = S0 + S1    read+pop  read       write 
*/

#define RegNone     12 
#define RN          "r12"

#define RegSP0      14  // [SP]     keep SP at its position after read/write
#define RegSP1      15  // [SP++]   post-increment SP after write and post-decrement after read

#define SCRIPT_REGSIZE 8            /* 8 bytes per register */

#define TEST_KO 0
#define TEST_OK 1
#define NO_TEST 0
#define IF_YES 1
#define IF_NOT 2
#define IF_EXTRA 3  
/*---------------------------------------------------------------------------------------------------------*/

/* DTYPE int64 translated to fp32 in source code */
#define DTYPE_UINT8     0
#define DTYPE_UINT16    1
#define DTYPE_INT16     2
#define DTYPE_UINT32    3
#define DTYPE_INT32     4
//#define DTYPE_INT64     5
#define DTYPE_FP16      6
#define DTYPE_FP32      7
//#define DTYPE_FP64      8
#define DTYPE_TIME16    9
#define DTYPE_TIME32   10
#define DTYPE_TIME64   11
#define DTYPE_PTR28B   12
                       
/*---------------------------------------------------------------------------------------------------------*/
/*----- MAIN OPCODES ----   SYNTAX EXAMPLES   */
#define OP_TESTEQU         0  // TESTEQU DST OPAR SRC1 SRC2  TEST DST =  SRC1 OPAR SRC2
#define OP_TESTLEQ         1  //                             TEST DST <= SRC1 OPAR SRC2
#define OP_TESTLT          2  //                             if SRC2 = 15 then SRC2 = K13
#define OP_TESTNEQ         3  //                             
#define OP_TESTGEQ         4  //                             
#define OP_TESTGT          5  //                             
#define OP_LD              6  // LD 
#define OP_JMOV            7  // MOV JUMP ST 


/*---------------------------------------------------------------- BYTE-CODE INSTRUCTIONS FIELDS------*/
/*                    xx<----K12--->
    __________________0000000000SRC2
    __________DST_SRC1SRC3SRC4__SRC2
*/
#define  OP_COND_INST_MSB 31u 
#define  OP_COND_INST_LSB 30u /*  2 conditional fields */
#define       OP_INST_MSB 29u 
#define       OP_INST_LSB 27u /*  3 instruction code TEST, LDJUMP, xx */
#define  OP_OPAR_INST_MSB 26u 
#define  OP_OPAR_INST_LSB 22u /*  5 operand */
#define   OP_DST_INST_MSB 21u 
#define   OP_DST_INST_LSB 18u /*  4 DST field */
#define  OP_SRC1_INST_MSB 17u 
#define  OP_SRC1_INST_LSB 14u /*  4 SRC1 field */          
#define  OP_SRC3_INST_MSB 13u                                       
#define  OP_SRC3_INST_LSB 10u /*  4 SRC3 field */                   
#define  OP_SRC4_INST_MSB  9u                                       
#define  OP_SRC4_INST_LSB  6u /*  4 SRC4 field */                   
                           
#define OP_RKEXT_INST_MSB  4u                                       
#define OP_RKEXT_INST_LSB  4u /*  1 RK is on the following words */ 
#define  OP_SRC2_INST_MSB  3u                                       
#define  OP_SRC2_INST_LSB  0u /*  4 SRC2 field / RK */              
                            
#define  OP_EXT1_INST_MSB 13u /* OPLJ_SCATTER index update yes=1 */
#define  OP_EXT1_INST_LSB 13u  
#define  OP_EXT0_INST_MSB 12u /* OPLJ_SCATTER pre-increment yes=1 otherwise pre-increment */
#define  OP_EXT0_INST_LSB 12u  
                            
#define      INDEX_UPDATE  1u /* OP_EXT1_INST=1 for SCATTER/GATHER + index update */
#define     PRE_INCREMENT  1u /* OP_EXT0_INST=1 for SCATTER/GATHER + pre-increment */
                            
#define     OP_K_INST_MSB 11u
#define     OP_K_INST_LSB  0u  

#define UNSIGNED_K_OFFSET 2048u         /* ST(INST[0], OP_K_INST, iK + UNSIGNED_K_OFFSET); */
#define     MAX_LITTLE_K 2047           /* coded as 2047 + 2048 = 3FFF */
#define     MIN_LITTLE_K (-2016)        /* coded as -2016 + 2048 = 20 */
#define SRC2LONGK_PATTERN_INST_MSB 11u  /* 0 <=> PATTERN_SRC2_LONGK */
#define SRC2LONGK_PATTERN_INST_LSB  5u 
                            
#define CALLSYS_K_INST_MSB 5u
#define CALLSYS_K_INST_LSB 0u  
 
        /* OP_JMOV */
#define BITFIELD_MSB_INST_MSB 11 /* 6  OPLJ_WR2BF OPLJ_RDBF MSB 0..63 */
#define BITFIELD_MSB_INST_LSB  6
#define BITFIELD_LSB_INST_MSB  5 /* 6  LSB 0..63 */
#define BITFIELD_LSB_INST_LSB  0

//__________SRC0SRC1##########SRC2  10-bits Jump for JUMP/BANZ/CALL  #+/-512 
//__________SRC0SRC1SRC3######SRC2  CALLSYS #6bits + 4 Registers(r0..r11)
//#define    JUMPIDX_INST_MSB 13 /* 10  signed offset for JUMP/CALL */
//#define    JUMPIDX_INST_LSB  4
//#define CALLSYSIDX_INST_MSB  9 /*  6  unsigned CALLSYS */
//#define CALLSYSIDX_INST_LSB  4


/*-------------------------------------------------------------------------------OP_ALU_TEST_/K--------*/
#define OPAR_NOP          0 // ---     SRC2/K                          Ri = #K                            
#define OPAR_ADD          1 // add     SRC1 + SRC2 (or K)              PUSH: S=R+0  POP:R=S+R0   DUP: S=S+R0  DEL: R0=S+R0  DEL2: R0=S'+R0
#define OPAR_SUB          2 // sub     SRC1 - SRC2 (or K)                  MOVI #K: R=R0+K
#define OPAR_MUL          3 // mul     SRC1 * SRC2 (or K)
#define OPAR_DIV          4 // div     SRC1 / SRC2 (or K)              DIV  
#define OPAR_OR           5 // or      SRC1 | SRC2 (or K)              if SRC is a pointer then it is decoded as *(SRC)
#define OPAR_NOR          6 // nor     !(SRC1 | SRC2) (or K)               example TEST (*R1) > (*R2) + 3.14   or   R1 = (*R2) + R4
#define OPAR_AND          7 // and     SRC1 & SRC2 (or K)
#define OPAR_XOR          8 // xor     SRC1 ^ SRC2 (or K)
#define OPAR_SHR          9 // shr     SRC1 << SRC2 (or K)             SHIFT   
#define OPAR_SHL         10 // shl     SRC1 >> SRC2 (or K)
#define OPAR_SET         11 // set     SRC1 | (1 << SRC2 (or K))       BSET      
#define OPAR_CLR         12 // clr     SRC1 & (1 << SRC2 (or K))       BCLR     TESTBIT 0/R0 OPAR_SHIFT(SRC1, 1<<K5)
#define OPAR_MAX         13 // max     MAX (SRC1, SRC2)                     
#define OPAR_MIN         14 // min     MIN (SRC1, SRC2)                      
#define OPAR_AMAX        15 // amax    AMAX (abs(SRC1), abs(SRC2)) 
#define OPAR_AMIN        16 // amin    AMIN (abs(SRC1), abs(SRC2))
#define OPAR_NORM        17 // norm    normed on MSB(SRC1), applied shift in SRC2
#define OPAR_ADDMOD      18 // addmod  SRC1 + SRC2 (or K) MODULO_DST   DST = OPAR SRC1 SRC2/K      
#define OPAR_SUBMOD      19 // submod  SRC1 - SRC2 (or K) MODULO_DST   works for PTR    

/*--------------------------------------------------------------------------------OP_LD---------------*/
#define OPLJ_CAST         0 // set r2 type #float               operations always done in fp32 or int 
#define OPLJ_CASTPTR      1 // set r2 typeptr #float (28b_ptr)  and used for data read R[K]
#define OPLJ_BASE         2 // set r4 base r5         
#define OPLJ_SIZE         3 // set r4 size r5         
#define OPLJ_PARAM        4 // set r1 param xxx     load offset in param (sets H1C0 = 0)
#define OPLJ_HEAP         5 // set r3 heap xxx      load offset in heap  (sets H1C0 = 1)
#define OPLJ_GRAPH        6 // set r3 graph xxx     load offset of graph 
#define OPLJ_SCATTER      7 // [ dst src2/k ]+ = src1   [ dst ]+ src2/k = src1  use H1C0 to select DST memory
#define OPLJ_GATHER       8 // dst = [ src1 src2/k ]+   dst = [ src1 ]+ src2/k  OP_EXT1_INST for increment   
#define OPLJ_WR2BF        9 // dst | lsb msb | = src1     write to bit-field
#define OPLJ_RDBF        10 // dst = src1 | lsb msb |     read from bit-field
#define OPLJ_SWAP        11 // swap r2 r3       
#define OPLJ_DELETE      12 // delete #n from stack without save
#define OPLJ_JUMP        13 // jump signed_K  + push dst src1
#define OPLJ_BANZ        14 // banz +/-K decrement dst 
#define OPLJ_CALL        15 // call +/-K and push dst src1
#define OPLJ_CALLSYS     16 // callsys {K}  bit8=0 {dst src1 src2}  bit8=1 {dst + extra word for #n + #arc/#node}
#define OPLJ_SAVE        17 // save up to 5 registers
#define OPLJ_RESTORE     18 // restore up to 5 registers   
#define OPLJ_RETURN      19 // return {keep registers fields}


#define OPLJ_NONE        32  

/* parameters ( CMD, *, *, * )
*   CMD = command 12bits    Parameter 10bits            Parameter2 12bits
*           command         NodeID, arcID, function     Size, sub-library
*/                   
          
#define script_label "label"            
/*
    FEDCBA9876543210FEDCBA9876543210            OP_TESTxx and OP_LD :
    IIyyy-OPARDST_SRC1<----K14-xSRC2  eq,le,lt,ne,ge,gt,ld dst src1 src2

                                                OP_JMOV : 
    IIyyy-OPARDST_______<--K12----->  OPLJ_CAST         dst src1 dtype
    IIyyy-OPARDST_______<--K12----->  OPLJ_CAST(PTR)    dst src1 dtype
    IIyyy-OPARDST_______<--K12-0SRC2  OPLJ_BASE         dst src2/K    
    IIyyy-OPARDST_______<--K12-0SRC2  OPLJ_SIZE         dst src2/K
    IIyyy-OPARDST______________1_int  OPLJ_PARAM        
    IIyyy-OPARDST______________1_int  OPLJ_HEAP         
    IIyyy-OPARDST______________1_int  OPLJ_GRAPH
    IIyyy-OPARDST_SRC1pp<--K12-xSRC2  OPLJ_SCATTER      [dst src2/k]+ = src1 
    IIyyy-OPARDST_SRC1pp<--K12-xSRC2  OPLJ_GATHER       dst = [src1]+ src2/k 
    IIyyy-OPARDST_SRC1__LLLLLLPPPPPP  OPLJ_WR2BF  
    IIyyy-OPARDST_SRC1__LLLLLLPPPPPP  OPLJ_RDBF   
    IIyyy-OPARDST_SRC1______________  OPLJ_SWAP   
    IIyyy-OPARDST_______<--K12----->  OPLJ_DELETE       SP0/SP1 are not used
    IIyyy-OPARDST_SRC1__<--K12----->  OPLJ_JUMP         SP0/SP1 are not used
    IIyyy-OPARDST_SRC1__<--K12----->  OPLJ_BANZ         SP0/SP1 are not used
    IIyyy-OPARDST_SRC1__<--K12----->  OPLJ_CALL         SP0/SP1 are not used
    IIyyy-OPARDST_SRC1SRC3SRC4<-K6->  OPLJ_CALLSYS      SP0/SP1 are not used
    IIyyy-OPARDST_SRC1SRC3SRC4__SRC2  OPLJ_SAVE         SP0/SP1 are not used
    IIyyy-OPARDST_SRC1SRC3SRC4__SRC2  OPLJ_RESTORE      SP0/SP1 are not used
    IIyyy-OPAR______________________  OPLJ_RETURN 

    FEDCBA9876543210FEDCBA9876543210

    STACK INCREMENT : pre-check SRC2 on :
        IIyyy-OPARDST_SRC1<----K14-xSRC2  eq,le,lt,ne,ge,gt,ld dst src1 src2
            check src2=SP1 : {src2ID=SP, SP--}

        test SRC2=SP1 on OPLJ_BASE OPLJ_SIZE OPLJ_SCATTER OPLJ_GATHER
            check src2=SP1 : {src2ID=SP, SP--}

        then src1 , then dst
*/

#endif  // if carm_stream_script_INSTRUCTIONS_H

#ifdef __cplusplus
}
#endif
 