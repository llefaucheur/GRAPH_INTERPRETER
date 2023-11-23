/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_script_process.c
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


#include "stream_const.h"
#include "stream_types.h"
#include "arm_script.h"


/**
  @brief         arm_script : byte-code interpreter 
  @param[in]     pinst      instance of the component
  @param[in]     reset      tells to set the conditional flag before the call
  @return        status     finalized processing
                 reference SWEET16 
                 https://archive.org/details/BYTE_Vol_02-11_1977-11_Sweet_16

  @par  
    Script format : 
        Word1 : script size, stack size
        Wordm : byte-codes 

        Scripts receive parameters:
            - Flag "comparison OK" set during the RESET sequence
            - R0, R4

    Registers 
        Instance static = 
            Bank of 8+8 registers ( 8bytes each, loop counters, thresholds) + hidden DTYPE
                R0 used for data comparison with the stack(top) = "S0"
                R1 used for arithmetics operations
                R2 used as loop counter
                R3 general purpose and pointer post-increments
                R4 .. R7 for general purpose and pointers addressed by "pp" fields
                R8 general purpose and pointers type control (DTYPE)
                R9..R15 general purpose 

            DTYPE 4bits: 
                0,1: FP32/FP64, 
                2,3,4,5: INT8/INT16/INT32/INT64, 
                6,7,8: STREAM_TIME16/32/64, data of S0 type
                9: 8-bits characters (terminated with /0), 
                10: boolean result from comparison
                11: 27bits offset format pointer to an object of S0 type
                12: hardware address pointer to an object of S0 type
                13: type defined by the stack (POPP/PSHP)
                14,15 : unused

        Instance memory area = 
             rising address :       <----------->>>>--------->
             registers              0 1 2 .. Nreg
             stack                                xxxxxxxxxxxx
             reset position of SP                             ^

             The stack lines and registers are tagged with DTYPE
                stack and registers are preserved between calls 
                the conditional flag is set at reset to allow specific initialization
                R0 is a parameter for function calls and reset (use-case setting)
                type conversions are managed by the interpreter
                    during calls, arithmetics operations..
                a CALS/CALL pushes the flags, address and SP position, and OPRC-RET pops it
             Flag "comparison OK" is used for conditional call/jump
                the flag is set during STREAM_RESET and reset during STREAM_RUN
             Flag "jump to label" selects the offset/label jump for the field "llll"
                the flag is reset with "llll" interpreted as a label for JUMP/JMPC
                LABL 0 toggles it to the interpretation of relative jumps to +/- 8 bytes

    Instructions 
        format 8its : IIIIXXXX
        15 labels for conditional/local jump + relative jump
        15 labels for subroutines

        0000 dddd  PSHC  push the next bytes as a constant of type DTYPE on stack 
        0001 rrrr  PSHR  push register[0..15] to S0 : on stack (*--SP)
        0010 iipp  PSHP  push *(R4..R7) to S0, increments[0,+,-,R3], forced DTYPE(R8)
        0011 rrrr  POPR  pop stack (*SP++) to register[0..15] 
        0100 iipp  POPP  pop S0 to *(R4..R7), increments[0,+,-,R3], forced DTYPE(R8)
        0101 llll  JUMP  unconditional jump to labels/offset, label#0 means jump to [PC + POP S0]
        0110 llll  JMPC  conditional jump to label/offset, label#0 means jump to [PC + POP S0]
        0111 iiii  CALS  system call and application callbacks
        1000 ssss  CALL  call label (1..7 local, 8..15 global(0-7), #0 reserved) callee saves registers not the flag
        1001 cccc  OPRC  operations for control
        1010 aaaa  OPRA  operations for arithmetics
        1011 llll  LABL  local-label (1..15), #0 toggles the "jump to label" flag
        1100 ....
        1101 ....
        1110 ....
        1111 ....  

        OPRC  16 control operations:
        0 RETS  subroutine return, restore SP,flag(not R9..R15?)
        1 RETK  keep S0 and S1 on the stack, restore and return
        2 EQUS  does S0 == S1               NES   does S0 != S1
        3 EQUR  does S0 == R0               NER   does S0 != R0
        4 GTES  does S0 >= S1               LTS   does S0 <  S1
        5 GTER  does S0 >= R0               LTR   does S0 <  R0
        6 LTES  does S0 <= S1               GTS   does S0 >  S1
        7 LTER  does S0 <= R0               GTR   does S0 >  R0
        8 INVF  NOP or invert the next comparison to have ^^^^^
        9 MAXS  S0 = max (S0, S1)
        A MAXR  S0 = max (S0, R0)
        B MINS  S0 = min (S0, S1)
        C MINR  S0 = min (S0, R0)
        D PSHT  push test result on stack for OPRA logical operations (DTYPE = boolean)
        E RSTS  S0 is a bit-field of data to reset (stack, SP, registers, flags)
        F ----

        OPRA  16 arithmetics/logical operations:
        0 ADDS  S0 = S0 + S1                OPRA  ORS : logical  OR for booleans 
        1 SUBS  S0 = S0 - S1                OPRA XORS : logical XOR for booleans
        2 MULS  S0 = S0 x S1                OPRA ANDS : logical AND for booleans
        3 DIVS  S0 = S0 / S1                OPRA NORS : logical NOR for booleans
        4 MODS  S0 = S0 mod S1              OPRA NOTS : S0 = not(S0) 
        5 ADDR  S0 = S0 + R1                OPRA  ORR : logical  OR for booleans
        6 SUBR  S0 = S0 - R1                OPRA XORR : logical XOR for booleans 
        7 MULR  S0 = S0 x R1                OPRA ANDR : logical AND for booleans
        8 DIVR  S0 = S0 / R1                OPRA NORR : logical NOR for booleans
        9 MODR  S0 = S0 mod R1              OPRA ASHR : S0 = S0 << R1 arithmetic shift
        A BANZ  R2 is decremented, conditional flag set with (R2 != 0)?
        B SWPS  swap S0 <-> S1
        C SWPR  swap R0 <-> R1
        D CNVS  S1 translated to the format of S0 and poped to S0
        E ----  -
        F EXTA  8bits extension code



    Default "CALS" callback (stream_script_callback) [MAX_NB_APP_CALLBACKS]
        #8  sleep/deep-sleep activation
        #9  system regsters access: who am I ?
        #10 timer control (default implementation with SYSTICK)

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
    SYSCALL : 
        reprogram the setting of stream_format_io[]

 */

void arm_script_interpreter (uint32_t *instance, uint8_t flag_reset)
{
    /* byte-code interpreter*/ 

}


#ifdef __cplusplus
}
#endif
    
#if 0
;PSHC
        PSHC INT8   12
        PSHC INT32  h55AA
        PSHC CHAR   "test"
        PSHC FP64   3.14159265359
;
;
;PSHR
        PSHR R0
        PSHR R15
;
;
;PSHP
        PSHP R4
        PSHP R5+
        PSHP R6(R3)
;
;
;POPR
        POPR R1
        POPR R14
;
;
;POPP
        POPP R4
        POPP R5+
        POPP R6(R3)
;
;
;JMPL
        JMPL 1
        JMPL 15
;
;
;JMPC
        JMPC 2
        JMPC 14
;
;
;CALP
        CALP 0 P1
        CALP 3 P2
;
;
;CALS
        CALS 0
        CALS 15
;
;
;OPRC
        OPRC RET 
        OPRC RETS
        OPRC EQUS
        OPRC EQUR
        OPRC GTES
        OPRC GTER
        OPRC LTES
        OPRC LTER
        OPRC GTS 
        OPRC GTR 
        OPRC LTS 
        OPRC LTR 
        OPRC EDSB
;
;
;OPRA
        OPRA ADDS
        OPRA SUBS
        OPRA MULS
        OPRA DIVS
        OPRA MODS
        OPRA ADDR
        OPRA SUBR
        OPRA MULR
        OPRA DIVR
        OPRA MODR
        OPRA BANZ
        OPRA SWPS
        OPRA SWPR
;
;
;CALL
        CALL 0
        CALL 15
;
;
;CALC
        CALC 1
        CALC 14
;
;
;LABL
        LABL 0
        LABL 15

;
;
;LABS
        LABL 0
        LABL 15
;
;
        CALL 1              ; call 1
        LABS 1              ; subroutine 1
        LITS INT32  h55AA   ; push on stack
        RETS                ; return stack
        OPRC EDSB           ; interpreter tag "end of subroutine"
#endif