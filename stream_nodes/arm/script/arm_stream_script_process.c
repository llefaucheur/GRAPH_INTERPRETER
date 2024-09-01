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


/**
  @brief         arm_stream_script : 16bits virtual machine, or Cortex-M0 binary codes
  @param[in]     pinst      instance of the component
  @param[in]     reset      tells to set the conditional flag before the call
  @return        status     finalized processing
                 
  @par  
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
        Instance static = 10 registers (R1..R10) + stack (R11-S, R12-S') + R0(CTRL) + R13,R14,R15(free)
        R0 means "not used" or "zero",    S' is the top-1 stack position 

        registers format :                      DTYPE
                    <---- MSB word ----------------> <---- LSB word ---------------->  
                    FEDCBA9876543210FEDCBA987654____ FEDCBA9876543210FEDCBA987654321|  
        int32       ____________________________0000 <------------------------------>  
        q15         ____________________________0001 <------------------------------>  
        fp32        ____________________________0010 <------------------------------>  
        TIME64      <-------------------------->0011 <------------------------------>    
        fp64        <-------------------------->0100 <------------------------------>  mantissa is patched
        int64-4bits <-------------------------->0101 <------------------------------>  
        TIME16      ____________________________0110 ________________<-------------->  
        TIME32      ____________________________0111 <------------------------------>  
        28b + DTYPE ---BASE--- ---SIZE---_______1000 DTYPE<------ 28bits address---->  typed pointer + circular option
        char /0     ____________________________1001 ________________________<------>  
        Control     <-------------------------->1010 <------------------------------>   
                R0: increment_rrr, TEST-flag, bit-fields 12b:[0..63->0..63], stack pointer, 

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
        if_II DST =(yyyyyyy) SRC1 uu SRC2 kk K7
        (IF Y/N), ((*P)/R/S)DST OPERAND_yyyyy ((*P)/R/S)SRC1,((*P)/R/S)SRC2, ARITHMETICS_OPERAND xx (K)

        FEDCBA9876543210FEDCBA9876543210
        II______________________________ if yes, if not, no test, break-point
        __yyyyyyy_______________________ Op-code 
        _________OPAR___________________ arithmetic op
        _____________DST.SRC1SRC2_______ Registers
        _____________________xxxxxxXXXXX 11-bits Constant = +/- 0 .. 1000 / Bit field for {1000 < K < 1024}

    Programming examples
        TEST R1 < R2 + 129
        TEST *P3 < S * 3.14159
        TEST bit 17 of R1
        R1 = (bit-field K11)
        R1 = R2 x R3
        P1 = P2 + #K11 (DTYPE)
        S0 = S0 + 3.14159
        R1 = 3.141592653589793
        R1 = S0 + S1 
        R1 = R2 << R3
        R1 = R2 & (1 << #K5)
        R1 = AMAX(R2, #K11)
        R1 = MIN(R2, R3)
        JUMP L_17 (R1)                      PUSH + JMP 
        CALSYS L_17, (R1,R2,R3)             PUSH + CALL 


    OPAR (SRC1 / SRC2) or (SRC1 / K)
         0  SRC1 + SRC2 (or K)              PUSH: S=R+0  POP:R=S+R0   DUP: S=S+R0  DEL: R0=S+R0  DEL2: R0=S'+R0
         1  SRC1 - SRC2 (or K)                  MOVI #K: R=R0+K
         2  SRC1 * SRC2 (or K)
         3  SRC1 / SRC2 (or K)              DIV  
         4  SRC2(or K) - SRC1               NEG and RSUB
         5  SRC2(or K) / SRC1               RDIV
         6  SRC1 | SRC2 (or K)              if SRC is a pointer then it is decoded as *(SRC)
         7  !(SRC1 | SRC2) (or K)               example TEST (*R1) > (*R2) + 3.14   or   R1 = (*R2) + R4
         8  SRC1 & SRC2 (or K)
         9  SRC1 ^ SRC2 (or K)
        10  SRC1 % SRC2 (or K)
        11  SRC1 << SRC2 (or K)             SHIFT   
        12  SRC1 >> SRC2 (or K)
        13  SRC1 | (1 << SRC2 (or K))       BSET      
        14  SRC1 & (1 << SRC2 (or K))       BCLR     TESTBIT 0/R0 OPAR_SHIFT(SRC1, 1<<K5)
        15  MAX   (SRC1, SRC2)                     
        16  MIN   (SRC1, SRC2)                      
        17  AMAX  (abs(SRC1), abs(SRC2)) 
        18  AMIN  (abs(SRC1), abs(SRC2))
        19  MAXF  (SRC1, SRC2) sets F when SRC1>SRC2                       
        20  MINF  (SRC1, SRC2) sets F when SRC1<SRC2                       
        21  AMAXF (abs(SRC1), abs(SRC2)) sets F when abs(SRC1)>abs(SRC2)   
        22  AMINF (abs(SRC1), abs(SRC2)) sets F when abs(SRC1)<abs(SRC2)
        23  SQRA (SRC1 + SRC2 x SRC2)

        24..31 free


    OPERATION FIELD yyyyyyy                 SRC1 = R0          K/SRC2 = 0/R0
         0  TEST DST =  SRC1 OPAR K      TEST DST =  K      TEST DST =  SRC1
         1  TEST DST <= SRC1 OPAR K      TEST DST <= K      TEST DST <= SRC1
         2  TEST DST <  SRC1 OPAR K      TEST DST <  K      TEST DST <  SRC1
         3  TEST DST != SRC1 OPAR K      TEST DST != K      TEST DST != SRC1
         4  TEST DST >= SRC1 OPAR K      TEST DST >= K      TEST DST >= SRC1
         5  TEST DST >  SRC1 OPAR K      TEST DST >  K      TEST DST >  SRC1
         6  TEST DST =  SRC1 OPAR SRC2   TEST DST =  SRC2   TEST DST =  SRC1
         7  TEST DST <= SRC1 OPAR SRC2   TEST DST <= SRC2   TEST DST <= SRC1
         8  TEST DST <  SRC1 OPAR SRC2   TEST DST <  SRC2   TEST DST <  SRC1
         9  TEST DST != SRC1 OPAR SRC2   TEST DST != SRC2   TEST DST != SRC1
        10  TEST DST >= SRC1 OPAR SRC2   TEST DST >= SRC2   TEST DST >= SRC1
        11  TEST DST >  SRC1 OPAR SRC2   TEST DST >  SRC2   TEST DST >  SRC1
        12  DST (bitfield) = SRC  
        13  DST = SRC (bitfield)
        14  DST = OPAR (SRC1 + SRC2)    (+ x & MIN)
        15  DST = OPAR (SRC1 + K)
        16  DST = SQRT(SRC)
        17  PTRINC DST +/-1 +/-inc +/-inc_modulo
        18  PTRINC DST +/-1 +/-inc +/-inc_modulo from the DTYPE of SRC1
        19  PTRMOD DST K11      set base6b/size5b of a PTR extra word if base+word>11bits
        20  NORM DST = normed on MSB(SRC1), applied shift in SRC2
        21  WRSP *SP[K] = DST
        22  RDSP DST = *SP[K]
        23  JMPA  jump to computed address DST 
        24  CALA  call computed address DST
        25  PUSH CONTROL (flag and control bits)
        26  CNVR DST converted to DTYPE of SRC
        27  CNVR DST, DTYPE
        28  PUSHLABEL  Label_K7  push the address of Label K7 on the stack    
        29  LABEL      K7        to address byte-code and parameter constants 
        30  JMP        (DST, SRC1, SRC2/R0=none),label_K7  
        31  BANZ DST   Label_K7   
        32  JMP        (DST, SRC1, SRC2/R0=none),Label_K7 / signed_K7  signed INSTRUCTION offset  and push 3 registers
        33  CALL       (DST, SRC1, SRC2/R0=none),Label_K7  and push up to 3 registers
        34  CALLSYS    (DST, SRC1, SRC2/R0=none),{K7} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers) 
        35  CALLSCRIPT (DST, SRC1, SRC2/R0=none),{K7} common scripts and node control  
        36  CALLAPP    (DST, SRC1, SRC2/R0=none),{K7} 0K6=64 local callbacks 1K6= 64 global callbacks   
        37  RETKEEP    return and keep registers in bitfield K11    
        38  RETURN 
        39  SWAP SRC, DST
        40  BITFIELD START,STOP loads 12bits in "control" for DST = SRC (bitfield)

        40..127 free

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
    FIFOTX buffer : instance | [regs + stack]   scratch/parameters
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
    extern void * pack2linaddr_ptr(const intPtr_t *long_offset, uint32_t data, uint32_t unit);

    PC = 0;
    F = 0;

    tmp8 = RD(descriptor[SCRIPT_PTR_SCRARCW0], NBREGS_SCRARCW3);    
    SP = (SCRIPT_REGSIZE +1) * tmp8;                       /* stack index is after registers */

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
                //arm_stream_services( (uint32_t)((stack[SP-1]).i32), 
                //    (uint8_t *)pack2linaddr_ptr(S->long_offset, (stack[SP-2]).i32, LINADDR_UNIT_W32), 
                //    (uint8_t *)pack2linaddr_ptr(S->long_offset, (stack[SP-3]).i32, LINADDR_UNIT_W32), 
                //    (uint8_t *)pack2linaddr_ptr(S->long_offset, (stack[SP-4]).i32, LINADDR_UNIT_W32), 
                //    (uint32_t)((stack[SP-5]).i32));
                //    SP = SP-5;
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
