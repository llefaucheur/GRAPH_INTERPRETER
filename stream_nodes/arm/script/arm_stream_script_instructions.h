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
           v                  <---- nStack  ---->
           R0 R1 R2 ..   r11  R13 R14
           <---registers--->  SP SP+1
                    STACK :   [..................]
                              SP init = nregs
                    HEAP / PARAM (4bytes/words)  [............]

    FEDCBA9876543210FEDCBA9876543210
    II______________________________ if yes, if not, no test, break-point (+ margin)
    __yyy___________________________  8 op-code (eq,le,lt,ne,ge,gt) + ldjmp  + xxx 
    _____-OPARDST_SRC1xxxxxxxxxxxxxx  OPLJ_BASE OPLJ_SIZE OP_LD OP_TEST  K= unsigned_K14(>32)-8192 = [8191 .. -8160]
    __________________0000000000SRC2  detection of 00.0000.000_.____ = (X & 0x3FE0) => either SRC2 or K-long
    __________________0000000001TTTT  DTYPE + extra word (and DTYPE of OPLJ_CAST)
    _____-OPARDST___OOxxxxxxxxxxxxxx  LABELs : K14 word32 offset, C0B1 =0 for code, =1 for heap/arc_buffer

    FEDCBA9876543210FEDCBA9876543210
    __________SRC0SRC1##########SRC2  10-bits Jump for JUMP/BANZ/CALL  #+/-512 + 3 registers pushed before jump
    __________SRC0SRC1SRC3######SRC2  CALLSYS #6bits + (r1,r2,r3,r4)
    EEEEmmmmmmmm22222222223333333333  CALLSYS second word if SRC3=RK   FP12 I10 I10 (node/arc)
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  CALLSYS second word if SRC2=RK   SRC2=DTYPE of the 2nd word

    __________DST_SRC10#________SRC2  SCATTER/GATHER DST[SRC2]= SRC1 selection with bit14=0
    __________DST_SRC11#_<---K11--->  SCATTER/GATHER DST[K11 +/-1024] = SRC1 '#' = post-increment + circular

    __________DST_SRC1SRC3SRC4__SRC2  SAVE/RESTORE 5 Registers(r0..r11)
    __________DST_SRC1__LLLLLLPPPPPP  OPLJ_WR2BF OPLJ_RDBF 6+6-bits(Len Pos) 
    FEDCBA9876543210FEDCBA9876543210



                <---- MSB word ----------------> <---- LSB word ---------------->  
                FEDCBA9876543210FEDCBA987654TYPE FEDCBA9876543210FEDCBA987654321|  
    uint8       _______H<---BASE----SIZE--->   0 000000000000000000000000<------>  computed as int32
    uint16      _______H<---BASE----SIZE--->   1 0000000000000000<-------------->  'H1C0' offset to heap(1) or code(0)
    int16       _______H<---BASE----SIZE--->   2 ssssssssssssssss<-------------->  
    uint32      _______H<---BASE----SIZE--->   3 <------------------------------>  computed as int32 !
    int32       _______H<---BASE----SIZE--->   4 <------------------------------>  
    int64       _______H<---BASE----SIZE--->   5 <------------------------------>  
    fp16        _______H<---BASE----SIZE--->   6 <------------------------------>  translated to FP32
    fp32        _______H<---BASE----SIZE--->   7 <------------------------------>  
    fp64        _______H<---BASE----SIZE--->   8 <------------------------------>  translated to FP32
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

#define  _____REGS1_MSB U(31) /* 7  unused  */
#define  _____REGS1_LSB U(25) 
#define  C0B1_REGS1_MSB U(24) /* 1  R2[5] = R4 : tells if the offset is to code 0 or heap 1 (arc buffer) */
#define  C0B1_REGS1_LSB U(24) 
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

    Assembler                          Binary encoding of the 19 instructions
    ---------                          -------------------------------------
    OP_TEST family                             OPAR   DST  SRC1 SRC2 /RK
      test r1 op r2 r3/type k         ; OP_TEST  OP.. R1   R2   R3/RK=0x1Exx immediate_xx or RK=0x1Fxx long immediate
                                      
    OP_LD family                                 OPAR DST  SRC1 SRC2 /RK
      r1 = op r2 r3/type K            ; OP_LD    OP.. R1   R2   R3/RK=0x1Exx immediate_xx or RK=0x1Fxx long immediate
      predefined "K" : r2 = L_xxx (for OPLJ_LABEL)
        L_BASE_PARAM_SET  H1C0 = 0
        L_BASE_PARAM_RAM  H1C0 = 1
                                      
    OP_JMOV family                              OPAR      DST  SRC1 SRC2 /RK
 -    set r2 type #float              ; OP_JMOV CAST      R2   ___  RK=0x1E07  1E=immediate => DTYPE_FP32=7
 -    set r2 typeptr #float           ; OP_JMOV CASTPTR   R2   ___  RK=0x1E07  1E=immediate => DTYPE_FP32=7
 -    set r4 base r5/k                ; OP_JMOV BASE      R4   ___  R5/RK=0x1Exx  1E=immediate => xx base for cicular addressing / loops
 -    set r4 size r5/k                ; OP_JMOV SIZE      R4   ___  R5/RK=0x1Exx  1E=immediate => xx
 -    st r2 [r4/k] r3                 ; OP_JMOV SCATTER   R2   R3   R4/RK=0x1Exx  1E=immediate => xx  R2[R4] = R3 
 -    ld r2 r3 [r4/k]                 ; OP_JMOV GATHER    R2   R3   R4/RK=0x1Exx  1E=immediate => xx  R2 = R3[R4]
      move r2 | lenK posK | r3        ; OP_JMOV WR2BF     R2   R3   K12 6b + 6b   bit-field
      move r2 r3 | lenK posK |        ; OP_JMOV RDBF      R2   R3   K12 6b + 6b   bit-field
-     swap sp r3                      ; OP_JMOV SWAP      SP   R3   
-     delete r4/K                     ; OP_JMOV DELETE    ___  ___  R4/RK=0x1Exx        pop stack without saving in registers
      save r3 r0 r11                  ; OP_JMOV SAVE      R1   R2   R3   R4   R5        push up to 5 registers  FEDCBA9876543210FEDCBA9876543210
      restore r3 r0 r11               ; OP_JMOV RESTORE   R1   R2   R3   R4   R5        pop up to 5 registers   ___________DST_SRC1SRC2SRC3SRC4_
      jump k r1,2,3                   ; OP_JMOV JUMP      R1/none   R2   R3 + K +/-128
      banz L_replaced_Label r1        ; OP_JMOV BANZ      R--  R2   R3 + K +/-128
      call L_replaced_Label r1        ; OP_JMOV CALL      R1   R2   R3 + K +/-128
      callsys 63 r1 r2 r3 r4          ; OP_JMOV CALLSYS   R1/none   R2   R3  R4 + #63  
      return                          ; OP_JMOV RETURN    ___  ___  ___
                                      
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

      r2 = Label                      ; OP_LD    NOP  R2   RK=0x1E03  1E=immediate => K +/-127
    Labels = <symbol, not Rii, not a number> (no instruction)    
*/

/*  <<<--------<<- analysis direction
    S1 = S1 + S1    read+pop  read+pop   write+push
    S1 = S1 + S0    read      read+pop   write+push
    S0 = S0 + S1    read+pop  read       write 
*/

#define RegNone     12 
#define RN          "r12"

/*#define RegK        13 */ 
#define RegSP0      14  // [SP]     keep SP at its position after read/write
#define RegSP1      15  // [SP++]   post-increment SP after write and post-decrement after read

#define SCRIPT_REGSIZE 8            /* 8 bytes per register */

#define TEST_KO 0
#define TEST_OK 1
#define NO_TEST 0
#define IF_YES 1
#define IF_NOT 2
/*---------------------------------------------------------------------------------------------------------*/

/* DTYPE int64 translated to fp32 in source code */
#define DTYPE_UINT8     0
#define DTYPE_UINT16    1
#define DTYPE_INT16     2
#define DTYPE_UINT32    3
#define DTYPE_INT32     4
#define DTYPE_INT64     5
#define DTYPE_FP16      6
#define DTYPE_FP32      7
#define DTYPE_FP64      8
#define DTYPE_TIME16    9
#define DTYPE_TIME32   10
#define DTYPE_TIME64   11
#define DTYPE_PTR28B   12
                       
/*---------------------------------------------------------------------------------------------------------*/
/*----- MAIN OPCODES ----   SYNTAX EXAMPLES   */
#define OP_TESTEQU    0  // TESTEQU DST OPAR SRC1 SRC2  TEST DST =  SRC1 OPAR SRC2
#define OP_TESTLEQ    1  //                             TEST DST <= SRC1 OPAR SRC2
#define OP_TESTLT     2  //                             if SRC2 = 15 then SRC2 = K13
#define OP_TESTNEQ    3  //                             
#define OP_TESTGEQ    4  //                             
#define OP_TESTGT     5  //                             
#define OP_LD         6  // LD 
#define OP_JMOV       7  // MOV JUMP ST 


/*---------------------------------------------------------------- BYTE-CODE INSTRUCTIONS FIELDS------*/
#define      OP_COND_INST_MSB 31 
#define      OP_COND_INST_LSB 30 /*  2 conditional fields */
#define           OP_INST_MSB 29 
#define           OP_INST_LSB 27 /*  3 instruction code TEST, LDJUMP, xx */
#define      OP_OPAR_INST_MSB 26 
#define      OP_OPAR_INST_LSB 22 /*  5 operand */
#define       OP_DST_INST_MSB 21 
#define       OP_DST_INST_LSB 18 /*  4 DST field */
#define      OP_SRC0_INST_MSB OP_DST_INST_MSB
#define      OP_SRC0_INST_LSB OP_DST_INST_LSB
#define      OP_SRC1_INST_MSB 17 
#define      OP_SRC1_INST_LSB 14 /*  4 SRC1 field */          
#define      OP_SRC3_INST_MSB 13                                        // ^ ^  K10:  -512 .. +511 (JUMPIDX_INST)
#define      OP_SRC3_INST_LSB 10 /*  4 SRC3 field */                    // | |   
#define      OP_SRC4_INST_MSB  9                                        // |^|^
#define      OP_SRC4_INST_LSB  6 /*  4 SRC4 field */                    // |||| K6 :  0 .. 63 (CALLSYSIDX_INST)
#define     _________INST_MSB  5                                        // ||||
#define     _________INST_LSB  5 /*  1 unused  */                       // ||||
#define     OP_RKEXT_INST_MSB  4                                        // ||||
#define     OP_RKEXT_INST_LSB  4 /*  1 RK is on the following words */  // ||vv
#define      OP_SRC2_INST_MSB  3                                        // ||   K10: -512 .. +512 (SCGA_K11_INST)
#define      OP_SRC2_INST_LSB  0 /*  4 SRC2 field / RK */               // vv   K14: -8160 .. +8191 (OP_K_INST, OPLJ_LABEL)
        
#define         OP_K_INST_MSB 13
#define         OP_K_INST_LSB  0  
#define UNSIGNED_K_OFFSET 8192
#define  MAX_LITTLE_K 8191          /* coded as 8191 + 8192 = 3FFF */
#define  MIN_LITTLE_K (-8160)       /* coded as -8160 + 8192 = 20 */
#define SRC2LONGK_PATTERN_INST_MSB 13  /* 0 <=> PATTERN_SRC2_LONGK */
#define SRC2LONGK_PATTERN_INST_LSB  5 
#define   OP_K_DTYPE_INST_MSB  3 
#define   OP_K_DTYPE_INST_LSB  0  /* 4 DTYPE */


        
#define LABEL_OFFSET_INST_MSB  1 /* 2  C0B1=0 for code, =1 for heap/arc_buffer */
#define LABEL_OFFSET_INST_LSB  0 /*    moved to bit24 of MSB word of the register */

        /* OP_JMOV */
#define SCGA_POSTINC_INST_MSB 12 /* 1  SCATTER/GATHER post-increment */
#define SCGA_POSTINC_INST_LSB 11
#define  SCGA_unused_INST_MSB 11 
#define  SCGA_unused_INST_LSB 11
#define     SCGA_K11_INST_MSB 10 /* 11 SCATTER/GATHER immediate index */
#define     SCGA_K11_INST_LSB  0

#define BITFIELD_LEN_INST_MSB 11 /* 6  OPLJ_WR2BF OPLJ_RDBF length 0..63 */
#define BITFIELD_LEN_INST_LSB  6
#define BITFIELD_POS_INST_MSB  5 /* 6  position 0..63 */
#define BITFIELD_POS_INST_LSB  0

//__________SRC0SRC1##########SRC2  10-bits Jump for JUMP/BANZ/CALL  #+/-512 
//__________SRC0SRC1SRC3######SRC2  CALLSYS #6bits + 4 Registers(r0..r11)
#define    JUMPIDX_INST_MSB 13 /* 10  signed offset for JUMP/CALL */
#define    JUMPIDX_INST_LSB  4
#define CALLSYSIDX_INST_MSB  9 /*  6  unsigned CALLSYS */
#define CALLSYSIDX_INST_LSB  4

/* OPLJ_LABEL */
#define         H0C1_INST_MSB 14
#define         H0C1_INST_LSB 14  
#define        LABEL_INST_MSB 13
#define        LABEL_INST_LSB  0  

/*-------------------------------------------------------------------------------OP_ALU_TEST_/K--------*/
#define OPAR_NOP      0     //  ---     SRC2/K                          Ri = #K                            
#define OPAR_ADD      1     //  add     SRC1 + SRC2 (or K)              PUSH: S=R+0  POP:R=S+R0   DUP: S=S+R0  DEL: R0=S+R0  DEL2: R0=S'+R0
#define OPAR_SUB      2     //  sub     SRC1 - SRC2 (or K)                  MOVI #K: R=R0+K
#define OPAR_MUL      3     //  mul     SRC1 * SRC2 (or K)
#define OPAR_DIV      4     //  div     SRC1 / SRC2 (or K)              DIV  
#define OPAR_OR       5     //  or      SRC1 | SRC2 (or K)              if SRC is a pointer then it is decoded as *(SRC)
#define OPAR_NOR      6     //  nor     !(SRC1 | SRC2) (or K)               example TEST (*R1) > (*R2) + 3.14   or   R1 = (*R2) + R4
#define OPAR_AND      7     //  and     SRC1 & SRC2 (or K)
#define OPAR_XOR      8     //  xor     SRC1 ^ SRC2 (or K)
#define OPAR_SHR      9     //  shr     SRC1 << SRC2 (or K)             SHIFT   
#define OPAR_SHL     10     //  shl     SRC1 >> SRC2 (or K)
#define OPAR_SET     11     //  set     SRC1 | (1 << SRC2 (or K))       BSET      
#define OPAR_CLR     12     //  clr     SRC1 & (1 << SRC2 (or K))       BCLR     TESTBIT 0/R0 OPAR_SHIFT(SRC1, 1<<K5)
#define OPAR_MAX     13     //  max     MAX (SRC1, SRC2)                     
#define OPAR_MIN     14     //  min     MIN (SRC1, SRC2)                      
#define OPAR_AMAX    15     //  amax    AMAX (abs(SRC1), abs(SRC2)) 
#define OPAR_AMIN    16     //  amin    AMIN (abs(SRC1), abs(SRC2))
#define OPAR_NORM    17     //  norm    normed on MSB(SRC1), applied shift in SRC2
#define OPAR_ADDMOD  18     //  addmod  SRC1 + SRC2 (or K) MODULO_DST   DST = OPAR SRC1 SRC2/K      
#define OPAR_SUBMOD  19     //  submod  SRC1 - SRC2 (or K) MODULO_DST   works for PTR    

/*--------------------------------------------------------------------------------OP_LDJUMP---------------*/
#define OPLJ_CAST         0  // set r2 type #float               operations always done in fp32 or int 
#define OPLJ_CASTPTR      1  // set r2 typeptr #float (28b_ptr)  and used for data read R[K]
#define OPLJ_BASE         2  // set r4 base r5         
#define OPLJ_SIZE         3  // set r4 size r5         
#define OPLJ_SCATTER      4  // dst [ src2/k ] = src1       save to memory index, use H1C0 to select DST memory
#define OPLJ_GATHER       5  // dst = src1 [ src2/k ]       read from memory index, see H1C0 for SRC1
#define OPLJ_WR2BF        6  // dst | lenK posK | = src1    write to bit-field
#define OPLJ_RDBF         7  // dst = src1 | lenK posK |    read from bit-field
#define OPLJ_SWAP         8  // swap r2 r3       
#define OPLJ_DELETE       9  // delete #n from stack without save
#define OPLJ_JUMP        10  // jump signed_K  + push dst src1
#define OPLJ_BANZ        11  // banz +/-K decrement dst 
#define OPLJ_CALL        12  // call +/-K and push dst src1
#define OPLJ_CALLSYS     13  // callsys {K}  bit8=0 {dst src1 src2}  bit8=1 {dst + extra word for #n + #arc/#node}
#define OPLJ_SAVE        14  // save up to 5 registers
#define OPLJ_RESTORE     15  // restore up to 5 registers   
#define OPLJ_RETURN      16  // return {keep registers fields}
#define OPLJ_LABEL       17  // dst = Label   load 17+1b Label  (bit telling Code/Heap offset H1C0)

#define OPLJ_NONE        32  

/* parameters ( CMD, *, *, * )
*   CMD = command 12bits    Parameter 10bits            Parameter2 12bits
*           command         NodeID, arcID, function     Size, sub-library
*/                   
          
#define script_label "label"            
/*
    FEDCBA9876543210FEDCBA9876543210
    IIyyy-OPARDST_SRC10000000000SRC2  eq,le,lt,ne,ge,gt,ld dst src1 src2
    IIyyy-OPARDST_SRC1xxxxxxxxxxxxxx  eq,le,lt,ne,ge,gt,ld dst src1 K
    IIyyy-OPARDST_SRC1xxxxxxxxx1TTTT  eq,le,lt,ne,ge,gt,ld dst src1 dtype + K-extented

    IIyyy-OPARDST______________1TTTT  OPLJ_CAST(PTR)    dst src1 dtype
    IIyyy-OPARDST_____xxxxxxxxxYYYYY  OPLJ_BASE         dst src2/K    
    IIyyy-OPARDST_____xxxxxxxxxYYYYY  OPLJ_SIZE   
    IIyyy-OPARDST_SRC10#________SRC2  OPLJ_SCATTER
    IIyyy-OPARDST_SRC11#_<---K11--->  OPLJ_SCATTER
    IIyyy-OPARDST_SRC10#________SRC2  OPLJ_GATHER 
    IIyyy-OPARDST_SRC11#_<---K11--->  OPLJ_GATHER 
    IIyyy-OPARDST_SRC1__LLLLLLPPPPPP  OPLJ_WR2BF  
    IIyyy-OPARDST_SRC1__LLLLLLPPPPPP  OPLJ_RDBF   
    IIyyy-OPARDST_SRC1______________  OPLJ_SWAP   
    IIyyy-OPARDST______________YYYYY  OPLJ_DELETE 
    IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_JUMP   
    IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_BANZ   
    IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_CALL   
    IIyyy-OPARSRC0SRC1SRC3######SRC2  OPLJ_CALLSYS
    IIyyy-OPARSRC0SRC1SRC3SRC4__SRC2  OPLJ_SAVE   
    IIyyy-OPARSRC0SRC1SRC3SRC4__SRC2  OPLJ_RESTORE
    IIyyy-OPAR______________________  OPLJ_RETURN 
    IIyyy-OPARDST___OOxxxxxxxxxxxxxx  OPLJ_LABEL : unsigned 14bits word32 offset + C0B1=0 for code, =1 for heap/arc_buffer
    FEDCBA9876543210FEDCBA9876543210
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
    - Callback returning the current use-case to set parameters
    - Node Set/ReadParam/Run, patch a field in the header
    - Read Time 16/32/64 from arc/AL, compute differences,
    - Select new IO settings (set timer period, change gains)
    - Trace "string" + data
    - Callbacks (cmd (cmd+LIbName+Subfunction , X, n) for I2C/Radio .. Libraries 

    - FIFO read/set, data amount/free, debug reg read/set, time-stamp of last access (ASRC use-case)
    - Update Format framesize, FS
    - Jump to +/- N nodes in the list,      => read HEADER + test fields + jump
    - Un/Lock a section of the graph
    - AL decides deep sleep with the help of parameters (wake-up in X[s])
    - Call a relocatable binary section in the Param area
    - Share Physical HW addresses
    - Compute Median from data in a circular buffer + mean/STD

        //  move R(i) to/from arc FIFOdata / debugReg / with/without read index update
        //  Basic DSP: moving average, median(5), Max(using VAD's forgetting factors).
        //  time elapsed from today, from a reference, from reset, UTC/local time

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
        //  Modulo 60 function for the translation to mn. Wake me at 5AM.
        //  Registers : 64bits(addressable in int8/16/32) + 8bits (type: time, temperature, pressure, 4xint16, counter-current-max)
        //Command from arm_stream_command_interpreter() : return the code version number, ..  
*/