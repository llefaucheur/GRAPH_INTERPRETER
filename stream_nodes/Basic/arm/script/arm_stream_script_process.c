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


#include "stream_const.h"
#include "stream_types.h"
#include "arm_stream_script.h"
#include "arm_stream_script_instructions.h"


/**
  @brief         arm_stream_script : byte-code interpreter 
  @param[in]     pinst      instance of the component
  @param[in]     reset      tells to set the conditional flag before the call
  @return        status     finalized processing
                 reference SWEET16 
                 https://archive.org/details/BYTE_Vol_02-11_1977-11_Sweet_16

  @par  
    Script format : 
        Scripts receive parameters:
            - Flag "comparison OK" set during the RESET sequence, reset otherwise
            - R0 (64bits) 

    Registers 
        Instance static = 
            14 registers 8bytes R0..R13 general purpose and pointers 
            'R14'=S0 'R15'=S1
            DTYPE 4bits: 
                0: int32    default types at reset
                1: q15
                2: int64
                3: boolean
                4: 8-bits characters (terminated with /0) 
                5,6,7: STREAM_TIME16/32/64
                8: 27bits offset format pointer + 4bit type
                9: type defined by S0
                A: fp32
                B: fp64

        Instance memory area (from const.h)
            TX ARC descriptor: locks execution, 
                base address = instance, registers
                length = code length + byte code format
                read index = start of stack index
                write index = start of parameters index + synchronization byte

            stack and registers are preserved between calls 
            the conditional flag is set at reset to allow specific initialization
            R0 is a parameter for function calls and reset (use-case setting)
            type conversions are managed by the interpreter
                during calls, arithmetics operations..
            a CALS/CALL saves the flags, address and [OPCT RET] pops it
            Flag "comparison OK" is used for conditional call/jump
                the flag is set during STREAM_RESET and reset during STREAM_RUN
            Flag "forced operation with boolean" after [OPCT BOOL]
            The interpreter keeps the trace of the last 4 instructions for [JUMPOFF backward]
            The initial SP position is preserved for fast return

    Instructions 
        format 16its : IIII.rrrr + XXXX.YYYY sometime X.Y is not used 

        0000 0000 Clll.llll (C)JUMPLAB un/conditional jump to label
        0000 0001 CSoo.oooo (C)JUMP un/conditional offset (+/-63) jump 
        0000 0010 Clll.llll (C)CALL un/conditional call to label defined in the next byte (0..127)
        0000 0011 Clll.llll (C)CALLSYS un/conditional system call (0..127)
        0000 0100 C0ll.llll (C)CALLAPP un/conditional application callbacks (0..63)
        0000 0100 C1ll.llll (C)CALLSCRIPT un/conditional shared scripts (0..63)
        0000 0101 0lll.llll LABL local-label extra byte is used for the label (0..127)

        0001 _SiX XXXX.YYYY COPY RX <= RY (R15 unused) iX/iY=0/1/2/3 increment 0/+1/-1/+1_both
             S = 0 keep destination type DT, S = 1 copy source type ST
                  POP S0            0010 _100 0100.1110 S0    =>  R4     keep destination type (DEFAULT)
                  POP               0010 _S00 1111.1110 S0    =>  null  
                  COPY R0 R3 ST     0010 _S01 0000.0011 R3    =>  R0   : simple copy, keep source type
             with pointers:
                  LOAD R0 R1 +      0010 _S01 0000.0001 *R1++ =>  R0   : R1 of type pointer, keep destination type
                  MEMCPY ST ++      0010 _S11 0000.0001 *R1++ => *R0++ : R0 and R1 of type pointer, keep source type
                  STORE ST -        0010 _S01 0000.0001 R1    => *R0-- : R0 of type pointer, keep source type 

        0010 TTTT XXXX.YYYY LOADK  load the next word16 as a constant of TTTT DTYPE to RX/RY (R15 unused)
                  LOADPTR mbank offset type 
        0011 ____
        01__ ____
        10__ ____

        11cc cccc XXXXYYYY control and arithmetic followed by  [yyyyxxxx] R14=S0 R15=S1 (popped from stack)
           when the data types are different the one of DESTINATION is used  
           (stack[SP++] push, stack[--SP] pop)

    CODES corresponding to "arm_stream_script_instructions.h"
        00 RET   subroutine return, restore SP, flags : C000
        01 RETP  keep RX and RY on the stack as parameters, restore and return : C1XY
        02 PSHT  push test result on stack for logical operations (DTYPE = boolean) 
        03 RESET RX is a bit-field of data to reset (stack, SP, registers, flags)
        04 DUP   S0 is duplicated (repushed)
        05 NOP
        06 SWAPXY swap Rx <-> Ry from the next byte information [yyyyxxxx] R14=S0
        07 CCOPY conditional copy without conversion
        08 EQU   does RX == RY
        09 LTE   does RX <= RY
        0A LTS   does RX <  RY
        0B EQ0   does RX == 0
        0C LE0   does RX <= 0
        0D LT0   does RX < 0
        0E NEGC  negate comparison
        0F BIT0  does RX & (1 << RY) = 0
        10 BIT1  does RX & (1 << RY) > 0
        11 ADD   RX = RX + RY                    
        12 SUB   RX = RX - RY 
        13 RSB   RX = RY - RX
        14 MUL   RX = RX x RY                    
        15 DIV   RX = RX / RY                    
        16 DIVI  RX = RY / RX                    
        17 MOD   RX = RX mod RY                  
        18 ASHFT RX = RX << RY arithm-shift (+/-)
        19 OR    logical  OR 
        1A XOR   logical XOR 
        1B AND   logical AND 
        1C NOR   logical NOR 
        1D NOT   RX = not(RY)
        1E NEG   RX = -RY
        1F NORM  RX normed on MSB, applied shift in RY
        20 SQRA  RX = RX + (RY * RY) 
        21 INC   RX = RX + 1
        22 DEC   RX = RX - 1 
        23 BANZ  RX = RX - 1 flag set with (RX != 0)?
        24 BITSET RX = RX | (1 << RY)
        25 BITCLR RX = RX & ~(1 << RY)
        26 MAX   RX = max (RX, RY) sets F when RY>RX
        27 MIN   RX = min (RX, RY) sets F when RY<RX
        28 CONV  RX converted to the format of RY
        29 CAST  RX takes the format of RY without data conversion
  
        From Android CHRE  https://source.android.com/docs/core/interaction/contexthub
        String/array utilities: memcmp, memcpy, memmove, memset, strlen
            IEC61131-3 : LEN , LEFT, RIGHT, MID, CONCAT,INSERT, DELETE , REPLACE, FIND
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
    SYSCALL : 
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

/*
    FIFOTX buffer : instance | types[regs + stack] | registers & stack | scratch/parameters
    stack     | 0| 1| 2| 3|    Push => stack     | X| 1| 2| 3|
      SP        |                        SP           |
*/

void arm_stream_script_interpreter (
    arm_stream_instance_t *S,
    uint32_t *descriptor,
    uint16_t *code,
    uint8_t *ram)
{
    uint8_t *typ8, SP, F, tmp8, lownib, highnib, regX, regY, *src, *dst;
    uint16_t instruction;
    // uint8_t *paRAM, *paROM, typeX, typeY;
    regdata_t *R, *stack;
    uint32_t time, timeMax, PC;
    extern void arm_stream_services (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n);
    extern void * pack2linaddr_ptr(const intPtr_t *long_offset, uint32_t data);

    PC = 0;
    F = 0;

    tmp8 = RD(descriptor[SCRIPT_PTR_SCRARCW0], NBREGS_SCRARCW0);    
    SP = (REGSIZE +1) * tmp8;                       /* stack index is after registers */

    typ8 = ram;                                     /* register types  */
    R = (regdata_t *)&(typ8[SP + RD(descriptor[WRIOCOLL_SCRARCW3], NSTACK_SCRARCW3)]); /* registers are after types[regs + stack] */
    R = (regdata_t *)(((intPtr_t)R + 7) & (~7));  /* align on the next 8 Bytes boundary */

    stack = &(R[SP]);                                /* stack is just after the registers */
    //src = &(stack[RD(Script->state, NSTACK_SCRIPT)]);        /* soft parameters (RAM) are after the stack */
    //paRAM = (uint8_t *) src;
    //paROM = &(code[byte_code->size]);

    //timeMax = MINIFLOAT2Q31(RD(descriptor[SCRIPT_PTRWRIOCOLL_SCRARCW3], LOG2MAXCYCLE_SCRIPT));
    timeMax = 0x7FFFFFFFL;
    time = 0;

    /* byte-code interpreter - 
        restricted to int32, no label,  */

    while (time++ < timeMax)
    {
        instruction = code[PC++]; 
        lownib = instruction & 0xF; highnib = (instruction >> 4) & 0xF;

        switch(highnib)
        {
        case iBRANCHs:   /* branch */
        {   
        
        //tmp8 = code[PC++];
            switch(lownib & 0x7)
            {
            /* 0000 0000 Clll.llll (C)JUMPLAB un/conditional jump to label */
            case 0:
                break;

            /* 0000 0001 CSoo.oooo (C)JUMP un/conditional offset (+/-63) jump  */
            case 1:
                if (tmp8 & 0x80) /* conditional jump */ 
                {   if (F)
                    {   if (tmp8 & 0x20) PC = PC - (tmp8 & 0x1F);
                        else             PC = PC + (tmp8 & 0x1F);
                    } 
                }
                else /* unconditional jump */ 
                {   if (tmp8 & 0x20) PC = PC - (tmp8 & 0x1F);
                    else             PC = PC + (tmp8 & 0x1F);
                }
                break;

            /* 0000 0011 Clll.llll (C)CALLSYS un/conditional system call (0..127) */
            case 3:
                arm_stream_services( (uint32_t)((stack[SP-1]).i32), 
                    (uint8_t *)pack2linaddr_ptr(S->long_offset, (stack[SP-2]).i32), 
                    (uint8_t *)pack2linaddr_ptr(S->long_offset, (stack[SP-3]).i32), 
                    (uint8_t *)pack2linaddr_ptr(S->long_offset, (stack[SP-4]).i32), 
                    (uint32_t)((stack[SP-5]).i32));
                    SP = SP-5;
                break;

            /* 0000 0100 C0ll.llll (C)CALLAPP un/conditional application callbacks (0..63) 
               0000 0100 C1ll.llll (C)CALLSCRIPT un/conditional shared scripts (0..63)      */
            case 4:

                break;

            /* 0000 0101 0lll.llll LABL local-label extra byte is used for the label (0..127)*/
            case 5:
                break;
            }
        }

        /* 0001 _SiX XXXX.YYYY COPY RX <= RY (R15 unused) iX/iY=0/1/2/3 increment 0/+1/-1/+1_both */
        case iCOPY:   /* copy */
//            tmp8 = code[PC++]; 
            tmp8=0; regX = tmp8 >> 4; regY = tmp8 & 0xF; 
            R[regX].i32 = R[regY].i32;
            break;
        
        /* 0010 TTTT XXXX.YYYY LOADK  load the next word16 as a constant of TTTT DTYPE to RX/RY (R15 unused) */
        case iLOADK:   /* litterals */
  //          tmp8 = code[PC++]; regX = tmp8 >> 4; 
            src = (uint8_t *)&(code[PC]);
            dst = (uint8_t *)&(R[regX]);
            MEMCPY(dst, src, 4); 
            PC = PC+4;
            break;

        case 0x3: case 0x4: case 0x5: case 0x6: case 0x7: case 0x8: case 0x9: case 0xA: case 0xB:
        default:
            break;

        /* 11cc cccc XXXXYYYY control and arithmetic followed by  [yyyyxxxx] R14=S0 R15=S1 (popped from stack) */
        case 0xC:   /* control and arithmetics */
        case 0xD:
        case 0xE:
        case 0xF:
            switch(tmp8 & 0x3F)
            {
            case iRET: time = timeMax;
                break;

            case iEQU: 
                //tmp8 = code[PC++]; 
                tmp8=0; regX = tmp8 >> 4; regY = tmp8 & 0xF; 
                F = (R[regX].i32 == R[regY].i32);
                break;

            case iADD: time = timeMax;
//                tmp8 = code[PC++]; 
                tmp8=0; regX = tmp8 >> 4; regY = tmp8 & 0xF; 
                R[regX].i32 = R[regX].i32 + R[regY].i32;                
                break;
            }
        } /* switch(highnib) */
    } /* while (time++ < timeMax) */
}
