/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_demodulator.c
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
#include "platform.h"
#ifdef CODE_ARM_STREAM_DEMODULATOR


#ifdef __cplusplus
 extern "C" {
#endif
   



#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_demodulator.h"

//
//;----------------------------------------------------------------------------------------
//;12. arm_stream_demodulator
//;----------------------------------------------------------------------------------------
//;    Operation : decode a bit-stream from analog data. Use-case: IR decoder, UART on 
//;       SPI/I2S audio, frequency detector.
//;    Parameters : clock and parity setting or let the algorithm discover the frame setting after 
//;        some time. https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter
//;
//;   presets control :
//;   #1 : frequency estimator
//;    Operation : input signal frequency estimator. Best-effort accuracy on time-stamped streams. 
//;    Use second-order autoregression
//;    Use-case example: measure 50Hz from power lines on the three phases (a -1% deviation means "power-down").
//;    Parameters : none. output data estimate sampling rate is provided at reset
//;   #2 .. 10: provision for other demodulators
//;
//;   Metadata information can be extracted with the command "parameter-read":
//;   TAG_CMD = 1 read the signal amplitude
//;   TAG_CMD = 2 read the signal to noise ratio
//;
//arm_stream_demodulator 
//    3  i8; 0 1 0        instance, preset frequency estimator, tag
//;


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_demodulator (uint32_t command, void *instance, void *data, uint32_t *status)
{
    *status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the NODE manifest
        */
        case STREAM_RESET: 
        {   stream_al_services *stream_entry = (stream_al_services *)data;
            intptr_t *memresults = (intptr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);
            uint16_t tag = RD(command, NODE_TAG_CMD);        /* extra parameters */

            arm_stream_demodulator_instance *pinstance = (arm_stream_demodulator_instance *) *memresults;
            memresults++;
            /* here reset */

            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
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
            arm_stream_demodulator_instance *pinstance = (arm_stream_demodulator_instance *) instance;
            intptr_t nb_data, stream_xdmbuffer_size, bufferout_free;
            stream_xdmbuffer_t *pt_pt;
            #define SAMP_IN uint8_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;   
            stream_xdmbuffer_size    = pt_pt->size;
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 
            bufferout_free        = pt_pt->size;

            nb_data = stream_xdmbuffer_size / sizeof(SAMP_IN);

            // arm_stream_demodulator_process(pinstance, inBuf, outBuf, &nb_data);

            pt_pt = data;   *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN);     /* amount of data consumed */
            pt_pt ++;       *(&(pt_pt->size)) = nb_data * sizeof(SAMP_OUT);    /* amount of data produced */
            
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
 
#else
void arm_stream_demodulator (unsigned int command, void *instance, void *data, unsigned int *status) { /* fake access */ if(command || instance || data || status) return;}
#endif  // #ifndef CODE_ARM_STREAM_DEMODULATOR
