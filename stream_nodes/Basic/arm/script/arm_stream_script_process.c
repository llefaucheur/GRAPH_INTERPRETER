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

    Registers 
        Instance static = 
            3 bits addressing for 6 x Float + 2 Stack 
            3 bits for pointers AD0 .. AD5 + SP01
            3 bits for Special regs : 0:(MaxInc, Inc, Address Loop for LDR/STR), 1(BitFieldPos, Length), 
                                      2:BanzLoop, 3,4,5:_unused   6:7:SP01

            minimum state memory = (2 registers + 1 pointer + 0 special) x 8 Bytes = 24 Bytes + stack
            standard state memory = (6 registers + 6 pointers + 3 special) x 8 Bytes = 120 Bytes + stack

            registers format :                                                       DTYPE
                        <---- MSB word ----------------><---- LSB word ---------------->
                        FEDCBA9876543210FEDCBA987654321|FEDCBA9876543210FEDCBA987654____
            int32       <------------------------------>____________________________0000
            q15         <------------------------------>____________________________0001
            fp32        <------------------------------>____________________________0010
            TIME16      ________________<-------------->____________________________0011
            TIME32      <------------------------------>____________________________0100
            27bits offset format pointer + 4bit data DTYPE
                        DTYPE<------ 27bits address---->____________________________0101

            64bits physical address pointer :
                        <----------------- 64 bits to logical shift right ----------1000
            TIME64      <----------------- 64 bits to shift right ------------------1001    
            fp64        <----------------- 64 bits 4bits of mantissa removed--------1010
            int64-4bits <----------------- 64 bits to arithmetic shift right -------1011 
            char /0     ________________________<------>____________________________1100
                        ............................................................1101
                        ............................................................1110
                        ............................................................1111

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



    Instructions 16bits + 16/32bits constants, all with conditional execution

    FEDCBA9876543210
    C00000000yyyyrrr 1 register
           yyyy
           0 EQ0    does rrr == 0
           1 LE0    does rrr <= 0
           2 LT0    does rrr < 0
           3 NE0    does rrr != 0
           4 GT0    does rrr > 0
           5 GE0    does rrr >= 0
           6 PUSH   vvv regs
           7 POP    vvv
           8 DUP    duplicate stack
           9 DELS   remove S0 or up to S1
          10 RET    subroutine return, restore just SP and flags
          11 RETR   subroutine return, restore all 
          12 JMPA   jump to computed address rrr
          13 CALA   call computed address rrr
          14 
          15 

    FEDCBA9876543210
   (C00000000yyyyrrr  1 register )
    C0000yyyyydddrrr  2 registers
         yyyyy= 
           2 EQ     does ddd == rrr
           3 LE     does ddd <= rrr
           4 LT     does ddd <  rrr
           5 NE     does ddd != rrr 
           6 GT     does ddd > rrr 
           7 GE     does ddd >= rrr
           8 BIT0   does ddd & (1 << rrr) = 0
           9 BIT1   does ddd & (1 << rrr) > 0                           
          10 SQRT   ddd = square-root(rrr)                              
          11 NEG    ddd = -(rrr)                                        
          12 SWAP   ddd <-> rrr                                         
          13 COPY   ddd <- rrr                                          
          14 SWAR   ad(ddd) <-> reg(rrr) 
          15 CPAR   ad(ddd) -> reg(rrr)  
          16 CPRA   ad(ddd) <- reg(rrr)  
          17 CPRS   special(ddd) <- reg(rrr)  
          18 PSHP   *(ddd) increment_rrr                                   
          19 POPP   *(ddd) increment_rrr                                              
          20 CNVI   convert (float)rrr to integer => ddd                                       
          21 CNVF   convert (int)rrr to float => ddd      
          22
          23
          24
          25
          26
          27
          28
          29  ??? CPYA  ad05 + SP01 => ad05 + SP01 
          30      CPYS  sp01 + SP01 => sp05 + SP01 
          31      CPYR  RR01 + SP01 => RR05 + SP01 


    FEDCBA9876543210
   (C0000yyyyydddrrr  2 registers)
    C0xxxxxddduuuvvv  3 registers
      xxxxx  
           4 MOD   ddd = uuu mod vvv                  
           5 ASHT  ddd = uuu << vvv (arithmetic)
           6 LSHT  ddd = uuu << vvv (logical)
           7 OR    ddd = uuu | vvv
           8 XOR   ddd = uuu ^ vvv
           9 AND   ddd = uuu & vvv
          10 NOR   ddd = !(bb | vvv)
          11 NORM  ddd = normed on MSB(vvv), applied shift in uuu
          12 BSET  ddd = uuu | (1 << vvv)   
          13 BCLR  ddd = uuu & ~(1 << vvv)  
          14 ADD   ddd = uuu + vvv                    
          15 SUB   ddd = uuu - vvv 
          16 RSUB  ddd = vvv - uuu
          17 MUL   ddd = uuu x vvv                    
          18 DIV   ddd = uuu / vvv                    
          19 RDIV  ddd = vvv / uuu
          20 MAX   ddd = max (uuu, vvv) sets F when uuu>vvv
          21 MIN   ddd = min (uuu, vvv) sets F when uuu<vvv
          22 AMAX  ddd = max (abs(uuu), abs(vvv)) sets F when abs(vvv)>abs(uuu)
          23 AMIN  ddd = min (abs(uuu), abs(RYvvv sets F when abs(vvv)<abs(uuu)
          24 SQRA  ddd = uuu + (vvv * vvv) 
          25 LDR   uuu = *(vvv) increment_ddd
          26 STR   *(vvv) increment_ddd = uuu
          27 
          28 
          29 
          30 
          31 
          32 
          

    FEDCBA9876543210    branch
   (C0xxxxxddduuuvvv  3 registers)
    C1000xxxxkkkkkkk  K7 JMP
         xxxx
             
           0 LABEL K7       to address byte-code and parameter constants
           1 JMP label_K7
           2 BANZ Label_K7
           3 JMP signed_K7  signed instruction offset
           4 CALL Label_K7
           5 CALLSYS {K7}   system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW)
           6 CALLSCRPT {K7} common scripts and node control
           7 CALLAPP {K7}   0K6=64 local callbacks 1K6= 64 global callbacks
           8 RETK1 return and keep registers in bitfield K7 
           9 RETK2 return and keep registers, Special, address
          10 PSHA Label_K7  push the address of Label K7 on the stack
          11 
          12 
          13 
          14 
          15 


    FEDCBA9876543210    movi, bit-test
   (C1000xxxxkkkkkkk  K7 JMP)
    C1xxxxxuuukkkkkk  K6 + R 
           4 BITS uuu = uuu | (1 << K6)
           5 BITC uuu = uuu & ~(1 << K6)
           6 ADDK uuu = uuu + K6        (inc)
           7 SUBK uuu = uuu - K6        (dec)
           8 RSBK uuu = K6 - uuu
           9 DIVK uuu = uuu / K6
          10 RDVK uuu = K6 / uuu
          11 MULK uuu = uuu x Signed_K6 (+/- 31)
          12 BIT0 does uuu & (1 << K6) = 0  
          13 BIT1 does uuu & (1 << K6) != 0
          14 ASHL uuu = uuu << K6
          15 ASHK uuu = uuu >> K6
          16 LSHK uuu = uuu >> K6 logical
          17 WRSP *SP[K6] = uuu
          18 RDSP uuu = *SP[K6]
          19 MOVI uuu = #signed_k6  (SET 1 / CLR)
          20 MVIS spc = #signed_k6  (set/clear flag, loop, mask)
          21 MOVI uuu = #DTYPE following
          22 MOVA adr = #DTYPE address /16 /32 /64 bits x 2 (integer / float)
          23 MOVI spc = #DTYPE to special regs
          24 BITF read : mask SP0, shift and save in uuu
          24 BITF write : shift uuu + mask to SP0
          25 
          26
          27
          28
          29
          30
          31


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
