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


#define Reg0     0          // NULL-data / "don't use" / Control regsiter
// Reg1 .. nRegs free to use for the programmer
 
#define RegK    13          // constant extracted from the code 
#define RegSP   14          // [SP]   "14" is dynamically remapped to [SP] position
#define RegSP1  15          // [SP+1]


#define LAST_TEST_OPCODE 12
#define LAST_TEST_OPCODE_WITH_K 6

/*---------------------------------------------------------------------------------------------------------*/
/*----- MAIN OPCODES ----   SYNTAX EXAMPLES                                      SRC1=R0   SRC2=R0   */
#define OP_TESTEQU    0  // TESTEQU DST OPAR SRC1 SRC2  TEST DST =  SRC1 OPAR SRC2 =  SRC2   =  SRC1
#define OP_TESTLEQ    1  //                             TEST DST <= SRC1 OPAR SRC2 <= SRC2   <= SRC1
#define OP_TESTLT     2  //                             TEST DST <  SRC1 OPAR SRC2 <  SRC2   <  SRC1
#define OP_TESTNEQ    3  //                             TEST DST != SRC1 OPAR SRC2 != SRC2   != SRC1
#define OP_TESTGEQ    4  //                             TEST DST >= SRC1 OPAR SRC2 >= SRC2   >= SRC1
#define OP_TESTGT     5  //                             TEST DST >  SRC1 OPAR SRC2 >  SRC2   >  SRC1
#define OP_TESTEQUK   6  // TESTEQU DST OPAR SRC1 #K    TEST DST =  SRC1 OPAR K    =  K      =  SRC1
#define OP_TESTLEQK   7  // TESTEQU DST MUL SRC1 #K     TEST DST <= SRC1 OPAR K    <= K      <= SRC1
#define OP_TESTLTK    8  // TESTEQU DST - #K            TEST DST <  SRC1 OPAR K    <  K      <  SRC1
#define OP_TESTNEQK   9  // TESTEQU DST - SRC1          TEST DST != SRC1 OPAR K    != K      != SRC1
#define OP_TESTGEQK  10  //                             TEST DST >= SRC1 OPAR K    >= K      >= SRC1
#define OP_TESTGTK   11  //                             TEST DST >  SRC1 OPAR K    >  K      >  SRC1

#define OP_LD        12  // LD DST OPAR SRC1 SRC2
#define OP_LDK       13  // LD DST OPAR SRC1 #K immediate

#define OP_JMP       14  // JMP, CALL, LABEL
                       
#define OP_MOV       15  // " various instructions "
/*---------------------------------------------------------------------------------------------------------*/

/* DTYPE */
#define DTYPE_INT       0    // 0000    register type identification 
#define DTYPE_UINT      1    // 0001    K11 and small constants    
#define DTYPE_Q15       2    // 0010
#define DTYPE_FP32      3    // 0010
#define DTYPE_TIME64    4    // 0011
#define DTYPE_FP64      5    // 0100
#define DTYPE_INT64     6    // 0101
#define DTYPE_TIME16    7    // 0110
#define DTYPE_TIME32    8    // 0111
#define DTYPE_PTR28B    9    // 1000
#define DTYPE_CHAR     10    // 1001
#define DTYPE_CONTROL  11    // 1010

#define K_MAX  1011
#define K_INT     (K_MAX + DTYPE_INT    )   /*   0 K11 bit-fields */
#define K_UINT    (K_MAX + DTYPE_UINT   )   //   1
#define K_Q15     (K_MAX + DTYPE_Q15    )   //   2
#define K_FP32    (K_MAX + DTYPE_FP32   )   //   3
#define K_TIME64  (K_MAX + DTYPE_TIME64 )   //   4
#define K_FP64    (K_MAX + DTYPE_FP64   )   //   5
#define K_INT64   (K_MAX + DTYPE_INT64  )   //   6
#define K_TIME16  (K_MAX + DTYPE_TIME16 )   //   7
#define K_TIME32  (K_MAX + DTYPE_TIME32 )   //   8
#define K_PTR28B  (K_MAX + DTYPE_PTR28B )   //   9
#define K_CHAR    (K_MAX + DTYPE_CHAR   )   //  10
#define K_CONTROL (K_MAX + DTYPE_CONTROL)   //  11

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


/* ----- Operation fields ----- */
#define OP_COND_MSB U(31) 
#define OP_COND_LSB U(30) /*  2 conditional fields */

#define OP_INST_MSB U(29) 
#define OP_INST_LSB U(24) /*  6 instruction code */

#define OP_OPAR_MSB U(23) 
#define OP_OPAR_LSB U(19) /*  5 arithmetic operand */

#define  OP_DST_MSB U(18) 
#define  OP_DST_LSB U(15) /*  4 DST field */

#define OP_SRC1_MSB U(14) 
#define OP_SRC1_LSB U(11) /*  4 constant and SRC1 field */

#define OP_SRC2_MSB U(10) 
#define OP_SRC2_LSB U( 7) /*  4 constant and SRC2 field */

#define  OP_K11_MSB U(10) 
#define  OP_K11_LSB U( 0) /* 11 constant +-/1000 */

/* --- K11 used to load circular pointer base/size of DTYPE_PTR28B --- */
#define  OP_K11_SIZE_MSB U(10) 
#define  OP_K11_SIZE_LSB U( 5) /* 5  */
#define  OP_K11_BASE_MSB U( 5) 
#define  OP_K11_BASE_LSB U( 0) /* 6  */


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



/*----- OP_JUMP ----*/
#define OPBR_JUMP         0  // JMP LABEL K11 and PUSH SRC1/SRC2
#define OPBR_JUMPA        1  // JMPA  jump to computed address DST, PUSH SRC1/SRC2
#define OPBR_JUMPOFF      2  // JMP signed_K11  signed word32 offset  and push 3 registers 
#define OPBR_BANZ         5  // BANZ SRC1   Label_K11    
#define OPBR_CALL         6  // CALL LABEL K11 and PUSH SRC1/SRC2
#define OPBR_CALA         7  // CALA  call computed address DST, PUSH SRC1/SRC2
#define OPBR_CALLSYS      8  // CALLSYS   {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
#define OPBR_CALLSCRIPT   8  // CALLSCRIPT{K11} common scripts and node control   
#define OPBR_CALLAPP      9  // CALLAPP   {K11} 0K6=64 local callbacks 1K6= 64 global callbacks    
#define OPBR_SAVEREG     10  // push K11 registers
#define OPBR_RESTOREREG  11  // pop k11 registers
#define OPBR_RETURN      12  // RETURN {keep registers 
#define OPBR_LABEL       13  // DST = Label_K11 (code, parameter area (flash), heap (RAM))
/* parameters ( CMD, *, *, * )
*   CMD = command 12bits    Parameter 10bits            Parameter2 12bits
*           command         NodeID, arcID, function     Size, sub-library
*/                   


/*----- OP_MOV ----*/
#define OPLD_CAST       0   // DST_ptr = (DTYPE) 
#define OPLD_CASTPTR    1   // DST_ptr = (POINTER DTYPE) 
#define OPLD_BASE       2   // R4_PTR.base = R5, base for cicular addressing
#define OPLD_SIZE       3   // R4_PTR.size = R5, size for cicular addressing
#define OPLD_PTRINC     4   // R4_PTR = R5_PTR +/-1 +/-inc +/-inc_modulo 
#define OPLD_SCATTER    5   // R2[R4] = R3    indirect, write with indexes 
#define OPLD_SCATTERK   6   // R2[K] = R3     indirect, write with indexes 
#define OPLD_GATHER     7   // R2 = R3[R4]    indirect, read with indexes 
#define OPLD_GATHERK    8   // R2 = R3[K]     indirect, read with indexes 
#define OPLD_WR2BF      9   // R2(bitfield) = R3 
#define OPLD_RDBF      10   // R2 = R3(bitfield)
#define OPLD_NORM      11   // R2 = normed on MSB(R3), applied shift in R4
#define OPLD_SWAP      12   // SWAP SRC1, DST 

                  
#endif  // if carm_stream_script_INSTRUCTIONS_H

#ifdef __cplusplus
}
#endif
 
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

        registers format :                      DTYPE
                    <---- MSB word ----------------> <---- LSB word ---------------->  
                    FEDCBA9876543210FEDCBA987654____ FEDCBA9876543210FEDCBA987654321|  
        Control     <-------------------------->0000 <------------------------------>  NULL DATA
        int32       ____________________________0001 <------------------------------>  
        q15         ____________________________0010 <------------------------------>  
        fp32        ____________________________0011 <------------------------------>  
        TIME64      <-------------------------->0100 <------------------------------>    
        fp64        <-------------------------->0101 <------------------------------>  mantissa is patched
        int64-4bits <-------------------------->0110 <------------------------------>  
        TIME16      ____________________________0111 ________________<-------------->  
        TIME32      ____________________________1000 <------------------------------>  
        28b + DTYPE ---BASE--- ---SIZE---_______1001 DTYPE<------ 28bits address---->  typed pointer + circular option
 
        CONTROL/ R0: increment_rrr, TEST-flag, bields 12b:[0..63->0..63], stack pointer, 

    Instructions 32bits + 32/64bits extra words when K >= 0x3E9..3FF (1001..1023)
        1001d : next word is int32       
        1002d : next word is q15         
        1003d : next word is fp32              MOVI #fp32
        1004d : next word is TIME64      
        1005d : next word is fp64        
        1006d : next word is int64-4bits 
        1007d : next word is TIME16      
        1008d : next word is TIME32      
        1009d : next word is 28b + DTYPE 
        1010d : next word is Pointer     
        1011d : next word is char /0     

    GENERAL : 
        II______________________________ if yes, if not, no test, break-point
        __yyyyyy________________________ Op-code 
        ________-OPAR___________________ arithmetic op
        _____________DST.SRC1SRC2_______ Registers
        _____________________xxxxxxxxxxx 11-bits Constant = +/- 0 .. 1000 / Bit field for {1000 < K11 < 1024}
        _________________xxxxxxxxxxxxxxx 15-bits K15 +/- 16K
        _____________xxxxxxxxxxxxxxxxxxx 19-bits K19 +/- 250K

 

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

    Example of program :
;
;   algorithm : read arc #2 (mono input arc), GPIO is connected to arc #3
;       set the GPIO value to 0 or 1 depending if arc data is above a threshold
;       set new parameters to node "amplitude"
;
    PSHL INT8  12   ; detection threshold=12
    POPR 0          ; load R0 for this comparison
    PSHL INT8   2   ; push arcID 2
    CALS READ_ARC   ; push arc data and base address (for pp addressing)
    OPRC GTR        ; result=stack(-1) > R0 (threshold) ?
    PSHL INT8   3   ; arcID 3 (output to GPIO(x))
    JMPC 0          ; yes: jump to 0 (set gpio=1)
    PSHL INT8   0   ; no:  set gpio=0
    JMPL 1          ; set gpio and return
    LABL 0          ;  label 0
    PSHL INT8   1   ; no:  set gpio=1
    LABL 1          ;  label 1
    CALS WRITE_ARC  ; syscall: write data(stack(-1)) to arc[stack(-2)]
;
    PSHL INT8   arm_stream_amplifier ; this is in the symbol table (node_entry_point_table[])
    PSHL INT8   0   ; instance
    PSHL PTR amplitude_set
    CALS SETPARAM   ; syscall: setparam(amplitude, instance0, new_param=amplitude_set)
;
    PSHL INT8   arm_stream_amplifier ; this is in the symbol table (node_entry_point_table[])
    PSHL INT8   0   ; instance
    PSHL INT8   2   ; parameter TAG_CMD
    PSHL FP32   3.14; parameter data
    CALS SETPARAM   ; syscall: setparam(amplitude, instance0, new_param=amplitude_set)
;
    OPRC RET        ; return to scheduler
;    
    PARSTART amplitude_set
    1  i8; 0            preset=0
    1  i8; 1            load only the gain (Tag=1)
    1 i32; h10000       gain; 
    PARSTOP    
    PARSTART router_set2
    1  i8; 1            preset=1
    1  i8; 1            load only the second index (Tag=2)
    4 i16; 0 0 2 0      move arc0,chan0, to arc2,chan0
    PARSTOP    
    ENDSCRIPT       ; end of source code

    -----------------------------------------------
  
*/