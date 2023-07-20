/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_script.c
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


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  




/**
  @brief         Processing function for _______________
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
int32_t arm_script_calls_stream (int32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *status)
{
    stream_entrance *STREAM; /* function pointer used for debug/trace, memory move and free, signal processing */
    arm_script_instance *pinstance = (arm_script_instance *)instance;

    switch (command)
    {   
        //case STREAM_NODE_REGISTER: first command to register the SWC
        //case STREAM_DEBUG_TRACE:
        //{  
        //}
        default: 
            return 0;
    }
    STREAM = pinstance->stream_entry;
    STREAM (command, (uint8_t*)instance, (uint8_t*)data, 0);  /* single interface to STREAM for controls ! */
    return 0;
}


/**
  @brief         Processing function 
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none

  Word1_scripts : script size, global register used + backup MEM, stack size
  Wordn_scripts : stack and byte-codes, see stream_execute_script()  

  AL commands : 
    STREAM_FORMAT_IO => (PLATFORM_IO_SET_STREAM / PLATFORM_IO_STOP_STREAM, &stream (timer, sensor, ..));
    read ARC content and debug data
    write to ARC (and initiate 

 */
void script_processing (arm_script_instance *instance)
{
    /* byte-code interpreter*/ 
#if 0
    enum fct {
      lit,opr,lod,sto,cal,Int,jmp,jpc
    } ;

    struct opcode {
      int f:4; // 
      int a:4; // 
    };

    enum { stacksize = 500 };
    int p, b, t; //{program-, base-, topstack-registers}
    int i; //{instruction register}
    int s[stacksize]; //[1..stacksize];

      t = 0; b = 1; p = 0;
      s[1] = s[2] = s[3] = 0;
      do {
        printf("[%d]\n", p );
        struct opcode i = code[p]; p = p + 1;
        switch(i.f) { default:
            break; case lit: { t++; s[t] = i.a; }
            break; case opr:
              switch(i.a) { default:
                break; case 0:
                  { // {return}
                    t = b - 1; p = s[t + 3]; b = s[t + 2];
                  }
                break; case 1: s[t] = -s[t];
                break; case 2: t--; s[t] = s[t] + s[t + 1];
                break; case 3: t--; s[t] = s[t] - s[t + 1];
                break; case 4: t--; s[t] = s[t] * s[t + 1];
                break; case 5: t--; s[t] = s[t] / s[t + 1];
                break; case 6: s[t] = (s[t] & 1); //ord(odd(s[t]));
                break; case 8: t--; s[t] = (s[t] == s[t + 1]);
                break; case 9: t--; s[t] = (s[t] != s[t + 1]);
                break; case 10: t--; s[t] = (s[t] < s[t + 1]);
                break; case 11: t--; s[t] = (s[t] >= s[t + 1]);
                break; case 12: t--; s[t] = (s[t] > s[t + 1]);
                break; case 13: t--; s[t] = (s[t] <= s[t + 1]);
              }
            break; case lod: t++; s[t] = s[base(i.l) + i.a];
            break; case sto: s[base(i.l)+i.a] = s[t]; printf("%d\n",s[t]); t--;
            break; case cal:
              { //{generate new block mark}
                s[t + 1] = base(i.l); s[t + 2] = b; s[t + 3] = p;
                b = t + 1; p = i.a;
              }
            break; case Int: t = t + i.a;
            break; case jmp: p = i.a;
            break; case jpc: if( s[t] == 0) { p = i.a; t = t - 1; }
          }
      } while( p != 1 ); //< (sizeof(code)/sizeof(struct opcode)) );
      puts(" end pl/0");
    }
#endif
}


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing

  @par  
    Script format : 
        Word1 : script size, stack size
        Wordm : byte-codes 

        SCRIPTS in Flash : index given as a parmeter of arm_script()
        Word0+n: table of index to the byte-codes (or ASCII strings)
        Word1+n: Byte-codes 
        Scripts in RAM are in the parameter field of arm_script()

    Registers 
        Instance static = 
            Bank of 2+2+12 registers (up to 8bytes each, loop counters, thresholds) + hidden DTYPE
            DTYPE: FP8/16/32/64, INT8/16/32/64, TIME32/64, Characters(terminated with \0), 
                pointer (27bits index, raw type), unused(4)
            Bank of 4 "soft pointers". indexes + memory offset + hidden format

        Instance working = 
            The stack lines are tagged with DTYPE
            Flag "comparison OK" for conditional call/jump

    Instructions 
        format 8its : IIIIXXXX

        0000 dddd  LITN  constant(s) pushed on stack of DTYPE (4bits)
        0001 rrrr  LDSR  load on stack[0] from register[16] (push[0..15])
        0010 iipp  LDSP  load on stack[0] from pointer[4] + increments[0+-]    
        0011 rrrr  STSR  save stack[0] to register[16] (pop[0..15])
        0100 iipp  STSP  save stack[0] to *pointer[4] + increments[0+-.]
        0101 llll  JMPL  jump to local labels (<16)
        0110 llll  JMPC  conditional jump to local labels (<16)
        0111 iipp  CALP  system call #IDP [4] returned result in pointer[4] and register[4] (arc buffer + size)
        1000 iiii  CALS  system call #ID [16] result on stack and registers 
        1001 cccc  OPRC  operations list for control
        1010 aaaa  OPRA  operations list for arithmetics
        1011 llll  CALL  call local Label (<16)
        1100 llll  CALC  conditional call to local Label (<16)
        1101 llll  LABL  entry-point label and its ID (<16), for code reuse and compactness
        1110 ....
        1111 .... 

        OPRC  16 control operations:
        0 RET   return from CALL
        1 RETR  push R1/R2 on the stack and return from CALL
        2 RETS  keep S0/S1 on the stack and return from CALL
        3 EQUS  does S0 == S1 
        4 EQUR  does S0 == R1
        5 GTES  does S0 >= S1
        6 GTER  does S0 >= R1
        7 LTES  does S0 <= S1
        8 LTER  does S0 <= R1
        9 GTS   does S0 >  S1
        A GTR   does S0 >  R1
        B LTS   does S0 <  S1
        C LTR   does S0 <  R1
        D SWPS  swap S0 <->S1
        E SWPR  swap R0 <->R1
        F EXTC  8bits control extension

        OPRA  16 arithmetics operations:
        0 ADDS  S0 = S0 + S1 (pop)
        1 SUBS  S0 = S0 - S1
        2 MULS  S0 = S0 x S1
        3 DIVS  S0 = S0 / S1
        4 MODS  S0 = S0 mod S1
        5 ADDR  S0 = S0 + R1
        6 SUBR  S0 = S0 - R1
        7 MULR  S0 = S0 x R1
        8 DIVR  S0 = S0 / R1
        9 MODR  S0 = S0 mod R1
        A CNVS  S0 is translated to the format of R0 
        B CNVR  R0 is translated to the format of S0 
        C BANZ  R0 is decremented + check does R0 != 0 ?
        D 
        E 
        F EXTA  8bits arithmetic extension  

    Use-cases:
        long-term stats: mean, deviation, max with forget factor
        set (a new configuration), start, stop an IO stream
        call nodes for data conditioning
        Smart homes
        When the Temperature > Thr1 and Light < Thr2 and Time in range [t1,t2], then Action_x
        when someone enters or leaves the room send metadata to arc_x
        Smart manufacturing
        move the filled bottle to the capping station in three steps .. 
        Agriculture
        Use of timers for watering, soil/air/.. analysis
        Rain fall measurement: time measurement, valve control
        detect button (rising edge) and wait falling edge time to decide 
        polling initiated for next loop (wait or not)


 */
void arm_script (int32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *status)
{
    *status = 1;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, PRESET, TAG, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memory_results are followed by the first two words of STREAM_FORMAT_SIZE_W32 
                memory pointers are in the same order as described in the SWC manifest
        */
        case STREAM_RESET: 
        {   stream_entrance *stream_entry = (stream_entrance *)(uint64_t)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_script_instance *pinstance = (arm_script_instance *)  *memresults;
            memresults++;
            /* here reset */

            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   
            uint8_t *new_parameters = (uint8_t *)data;
            break;
        }



        /* func(command = STREAM_READ_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG/index of a parameter to read (Metadata, Needle), 0xFF means "read all the parameters"
                *instance, 
                data = *parameter(s) to read
        */ 
        case STREAM_READ_PARAMETER:  
        {   
            uint8_t *new_parameters = (uint8_t *)data;

            break;
        }
        


        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input size} {*output size}]

               data format is given in the node's manifest used during the YML->graph translation
               this format can be FMT_INTERLEAVED or FMT_DEINTERLEAVED_1PTR
        */         
        case STREAM_RUN:   
        {
           break;
        }

        /* func(command = STREAM_STOP, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_STOP:  break;    
    }
}

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


#ifdef __cplusplus
}
#endif
    
