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
    II._____________________________ if yes, if not, no test, break-point (+ margin)
    ___yyy__________________________  8 op-code (eq,le,lt,ne,ge,gt) + ldjmp  + xxx 
    ______-OPAR_____________________ 32 arithmetic op
    ___________DST_SRC1SRC2SRC3SRC4_    push/pop up to 5 Registers(r0..r11) + sp('13'[SP]) + sp1('14'[SP+1])
    ___________________11110xxxxxxxx  8-bits constant for OP_LD immediate 
    ___________________11111____TTTT  8-bits constant for OP_LD + TYPE + extra word
    ___________aaaabbbbcccc0xxxxxxxx  8-bits constant for JUMP/BANZ/CALL/CALLSYS #+/-128 + 3 registers pushed before jump
    ___________________Txxxxxxxxxxxx 13-bits T=1 for circular increment   K=12bits +/- 2047
    ____________________LLLLLLPPPPPP  6+6-bits BITFIELD_LEN_LSB + POS

                <---- MSB word ----------------> <---- LSB word ---------------->  
                FEDCBA9876543210FEDCBA987654____ FEDCBA9876543210FEDCBA987654321|  
    uint8       ___8____<---BASE----SIZE--->   0 000000000000000000000000<------>  computed as int32
    uint16      ___8____<---BASE----SIZE--->   1 0000000000000000<-------------->  
    int16       ___8____<---BASE----SIZE--->   2 ssssssssssssssss<-------------->  
    uint32      ___8____<---BASE----SIZE--->   3 <------------------------------>  computed as int32 !
    int32       ___8____<---BASE----SIZE--->   4 <------------------------------>  
    int64       ___8____<---BASE----SIZE--->   5 <------------------------------>  
    fp16        ___8____<---BASE----SIZE--->   6 <------------------------------>  translated to FP32
    fp32        ___8____<---BASE----SIZE--->   7 <------------------------------>  
    fp64        ___8____<---BASE----SIZE--->   8 <------------------------------>  translated to FP32
    TIME16      0000000000000000000000000000   9 0000000000000000<-------------->  
    TIME32      0000000000000000000000000000  10 <------------------------------>  
    TIME64      <-------------------------->  11 <------------------------------> DTYPE moved to MSB
    28b + DTYPE ___8____<---BASE----SIZE--->  12 TYPE<------ 28bits address-----> typed pointer + circular option (10+10)
*/    

/* ------------ MSB REGISTERS --------------*/

#define  _____REGS1_MSB U(31) /* 8  unused  */
#define  _____REGS1_LSB U(24) 
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
            type typeptr base size move swap pop push save restore call callsys jump banz return
    ALU     nop add addmod sub submod mul duv or nor and xor shr shl set clr max min amax amin norm 
    type    int8 int16 int32 float16 float double ptrfloat ptruint8 ptrint32.. 

    Assembler                          Binary encoding of the 19 instructions
    ---------                          -------------------------------------
    OP_TEST family                             OPAR DST  SRC1 SRC2 /RK
      test r1 op r2 r3/type k         ; OP_TEST  OP.. R1   R2   R3/RK=0x1Exx immediate_xx or RK=0x1Fxx long immediate
                                      
    OP_LD family                                 OPAR DST  SRC1 SRC2 /RK
      r1 = op r2 r3/type K            ; OP_LD    OP.. R1   R2   R3/RK=0x1Exx immediate_xx or RK=0x1Fxx long immediate
                                      
    OP_JMOV family                              OPAR      DST  SRC1 SRC2 /RK
      set r2 type #float              ; OP_JMOV CAST      R2   ___  RK=0x1E07  1E=immediate => DTYPE_FP32=7
      set r2 typeptr #float           ; OP_JMOV CASTPTR   R2   ___  RK=0x1E07  1E=immediate => DTYPE_FP32=7
      set r4 base r5/k                ; OP_JMOV BASE      R4   ___  R5/RK=0x1Exx  1E=immediate => xx base for cicular addressing / loops
      set r4 size r5/k                ; OP_JMOV SIZE      R4   ___  R5/RK=0x1Exx  1E=immediate => xx
      st r2 [r4/k] r3                 ; OP_JMOV SCATTER   R2   R3   R4/RK=0x1Exx  1E=immediate => xx  R2[R4] = R3 
      ld r2 r3 [r4/k]                 ; OP_JMOV GATHER    R2   R3   R4/RK=0x1Exx  1E=immediate => xx  R2 = R3[R4]
      move r2 | lenK posK | r3        ; OP_JMOV WR2BF     R2   R3   K12 6b + 6b   bit-field
      move r2 r3 | lenK posK |        ; OP_JMOV RDBF      R2   R3   K12 6b + 6b   bit-field
      swap sp r3                      ; OP_JMOV SWAP      SP   R3   ____
      delete r4/K                     ; OP_JMOV POP       ___  ___  R4/RK=0x1Exx        pop stack without saving in registers
      save r3 r0 r11                  ; OP_JMOV SAVE      R1   R2   R3   R4   R5        push up to 5 registers  FEDCBA9876543210FEDCBA9876543210
      restore r3 r0 r11               ; OP_JMOV RESTORE   R1   R2   R3   R4   R5        pop up to 5 registers   ___________DST_SRC1SRC2SRC3SRC4_
      jump k r1                       ; OP_JMOV JUMP      R1   R2   R3 + K +/-128
      banz L_replaced_Label r1        ; OP_JMOV BANZ      R1   R2   R3 + K +/-128
      call L_replaced_Label r1        ; OP_JMOV CALL      R1   R2   R3 + K +/-128
      callsys 127 r1 r2 r3            ; OP_JMOV CALLSYS   R1   R2   R3 + K  
      return                          ; OP_JMOV RETURN    ___  ___  ___
                                      
      Examples :                      
                                      
      testleq r6 sp                   ; OP_TESTLEQ NOP R6   ___  R14      
      testleq r6 #int 3               ; OP_TESTLEQ NOP R6   ___  RK=0x1E03  1E=immediate => K +/-127     
      testleq r6 #float 3.14          ; OP_TESTLEQ NOP R6   ___  RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word
      testleq r2 mul { r4 r6 }        ; OP_TESTLEQ MUL R2   R4   R6 
      testleq r2 add { r4 #int 3}     ; OP_TESTLEQ ADD R2   R4   RK=0x1E03  1E=immediate => K +/-127
      testleq r2 max { r4 #float 3.14}; OP_TESTLEQ MAX R2   R4   RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word
      testleq r2 max { r4 #double 3.14} ; OP_TESTLEQ MAX R2   R4   RK=0x1F07  1F=extension DTYPE = 7 = translated to FP32 + 1 word

      r5 = #ptrfloat 6 97112          ; OP_LD    NOP  R5   ___  RK=0x1F0C  1F=extension DTYPE = C = type + pointer28b (+1 word_
      if_no r6 = #int 3               ; IF OP_LD NOP  R6   ___  RK=0x1E03  1E=immediate => K +/-127
      r6 = #float 3.14                ; OP_LD    NOP  R6   ___  RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word      
      r2 = mul { r4 r6 }              ; OP_LD    MUL  R2   R4   R6 
      r2 = add { r4 #int 3}           ; OP_LD    ADD  R2   R4   RK=0x1E0F  1E=immediate => K +/-127
      r2 = max { r4 #float 3.14}      ; OP_LD    MAX  R2   R4   RK=0x1F06  1F=extension DTYPE = 6 = FP32 + 1 word
      r2 = max { r4 #double 3.14}     ; OP_LD    MAX  R2   R4   RK=0x1F07  1F=extension DTYPE = 7 = translated to FP32 + 1 word
      r6 = sp                         ; OP_LD    NOP  R6   ___  R13      
      r5 = add { sp r1 }              ; OP_LD    ADD  R5   R13  R1          alu on stack
      r5 = add { sp sp1 }             ; OP_LD    ADD  R5   R13  R15         alu on stack
      sp = add { r1 r2 }              ; OP_LD    ADD  R13  R1   R2          push sp++
      r2 = add { r4 #uint8 3}         ; OP_LD    ADD  R2   R4   RK          RK=0x1E03  1E=immediate => K +/-127
      r2 = max { r4 #float 3.14}      ; OP_LD    MAX  R2   R4   DTYPE_W32   OP_LDK1 : RK=float on extra word
      r2 = max r4 3.14                ; OP_LD                              
      r2 = max { r4 #double 3.14159}  ; OP_LD    MAX  R2   R4   DTYPE_W64   OP_LDK1 : RK=double on two extra words
      r2 = norm r3                    ; OP_LD    NORM R2   R3   R0         normed on MSB(R2),
      r2 = norm r3 r4                 ; OP_LD    NORM R2   R3   R4         normed on MSB(R2), applied shift->R4
      r2 = addmod r4 3                ; OP_LD  ADDMOD R2   R4   RK=0x1E03  1E=immediate => K +/-127
      r2 = add { r4 r5 }              ; OP_LD    ADD  R2   R4   R5         R2 = R4_ptr_28b [r5]  DTYPE of R4

    Labels = <symbol, not Rii, not a number> (no instruction)    
*/

#define RegUnused   12  //          unused register index 
#define RegSP       13  // [SP]     dynamically remapped to [SP] position
#define RegSP1      14  // [SP+1]
#define RegK        15  //          reg15 index used for constants

#define SCRIPT_REGSIZE 8            /* 8 bytes per register */

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

//  FEDCBA9876543210FEDCBA9876543210
//  ___yyy__________________________  8 op-code (eq,le,lt,ne,ge,gt) + ldjmp  + xxx 
//  ______-OPAR_____________________ 32 arithmetic op
//  ___________DST_SRC1SRC2_________ 13 Registers(r0..r11) + sp('13'[SP]) + sp1('14'[SP+1]) + RK('15')
//  _______________________SRC3SRC4_    up to 5 register fields 
//  ___________________11111____tttt    1bit for extensions 
//  ___________________11110xxxxxxxx  8-bits constant  

#define OP_COND_MSB U(31) 
#define OP_COND_LSB U(30) /*  2 conditional fields */
#define OP_INST_MSB U(29) 
#define OP_INST_LSB U(27) /*  3 instruction code TEST, LDJUMP, xx */
#define OP_OPAR_MSB U(26) 
#define OP_OPAR_LSB U(22) /*  5 arithmetic operand */
#define  OP_DST_MSB U(21) 
#define  OP_DST_LSB U(18) /*  4 DST field */
#define OP_SRC1_MSB U(17) 
#define OP_SRC1_LSB U(14) /*  4 SRC1 field */
#define OP_SRC2_MSB U(13) 
#define OP_SRC2_LSB U( 9) /*  4 SRC2 field / RK */
#define OP_SRC3_MSB U( 8) 
#define OP_SRC3_LSB U( 5) /*  4 SRC3 field */
#define OP_SRC4_MSB U( 4) 
#define OP_SRC4_LSB U( 1) /*  4 SRC4 field */

#define  OP_K_MSB U( 7) 
#define  OP_K_LSB U( 0)  /*   8 constant */

#define  OP_K_DTYPE_MSB U( 3) 
#define  OP_K_DTYPE_LSB U( 0)  /* 4 DTYPE */

#define BITFIELD_LEN_MSB 11 /* length 0..63 */
#define BITFIELD_LEN_LSB  6
#define BITFIELD_POS_MSB  5 /* position 0..63 */
#define BITFIELD_POS_LSB  0

/* --- K used for INCPTR  +/-1 +/-INCTYPE +/-inc_modulo  */
#define OP_K_INCTYPE_MSB U(12) 
#define OP_K_INCTYPE_LSB U(12) /* 1   cicrular increment */
#define   OP_K_SINCK_MSB U(11) 
#define   OP_K_SINCK_LSB U( 0) /* 12  signed increment */
#define PTR_INCK  0            /* reg/ptr += K12 (signed constant) */
#define PTR_CINCK 1            /* reg/ptr += K12 (circular buffer increment) */

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
#define OPLJ_CAST         0  // cast r2 type #float          operations always done in fp32 or int 
#define OPLJ_CASTPTR      1  // castptr r2 typeptr #float      and used for data read (cast)
#define OPLJ_BASE         2  // base r4 r5         
#define OPLJ_SIZE         3  // size r4 r5         
#define OPLJ_SCATTER      4  // dst[src2] = src1            save to memory index
#define OPLJ_GATHER       5  // dst = src1[src2]            read from memory index
#define OPLJ_WR2BF        6  // dst | lenK posK | = src1    write to bit-field
#define OPLJ_RDBF         7  // dst = src1 | lenK posK |    read from bit-field
#define OPLJ_SWAP         8  // swap r2 r3       
#define OPLJ_DELETE       9  // POP #n from stack without save
#define OPLJ_JUMP        10  // jmp signed_K  + push dst src1
#define OPLJ_BANZ        11  // banz +/-K decrement dst 
#define OPLJ_CALL        12  // call +/-K and push dst src1
#define OPLJ_CALLSYS     13  // callsys {K}  bit8=0 {dst src1 src2}  bit8=1 {dst + extra word for #n + #arc/#node}
#define OPLJ_SAVE        14  // save up to 5 registers
#define OPLJ_RESTORE     15  // restore up to 5 registers   
#define OPLJ_RETURN      16  // return {keep registers fields}



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