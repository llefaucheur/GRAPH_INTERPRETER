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



typedef struct
{
    stream_entrance *stream_entry;
    float data0;
    float data1;
    float data2;
} arm_script_instance;


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
void script_processing (arm_script_instance *instance, 
                     uint8_t *in, int32_t nb_data, 
                     int16_t *outBufs)
{
    /* fake decoder*/ 
    int i;
    for (i = 0; i < nb_data; i++)
    {
        outBufs[i] = (int16_t)((int16_t)(in[i])<<8);
    }
}



/**
  @brief         Debug script executed before/after node execution
  @param[in]     none
  @return        none

  @par           Execute the script located at "GRAPH_DEBUG_SCRIPT"
  @remark
 */
uint8_t stream_execute_script(void)
{   // TBC  
    //script_processing();
    return 1;
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
        Word1 : script size, global register used + backup MEM, stack size
        Wordn : stack 
        Wordm : byte-codes 

    Registers 
        One register is used for test "t"
        Other registers placed on the stack 
        data types : X + time-stamp, Vector + time-stamp
    Instructions (see TI57 programming manual)
        load "t", from stack(i) = R(i) or #const
        move R(i) to/from arc FIFOdata / debugReg / with/without read index update
        compare R(i) with "t" : <> = != and skip next instruction 
        jump to #label 
        dsz decrement and jump on non-zero
        arithmetic add,sub,(AND,shift), #const/R(j)
        time difference, time comparison
        long-term stats: mean, deviation, max with forget factor
        set (a new configuration), start, stop an IO stream
        call nodes for data conditioning

    Use-cases:
        Smart homes
        When the Temperature > Thr1 and Light < Thr2 and Time in range [t1,t2], then Action_x
        when someone enters or leaves the room take Action_Y
        Smart manufacturing
        move the filled bottle to the capping station in three steps .. 
        Agriculture
        Use of timers for watering, soil/air/.. analysis
        Rain fall measurement: time measurement, valve control


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

#ifdef __cplusplus
}
#endif
    
