/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_detector.c
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

#include "stream_const.h"
#include "stream_types.h"

#include "arm_stream_detector.h"

/*
    9.	stream_detector
    Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
    a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time 
    for debouncing and LED / user-interface).
    
    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.
*/

#define NB_PRESET 4
const detector_parameters detector_preset [NB_PRESET] = 
{   /* log2counter, log2decfMAX, high_pass_shifter, low_pass_shifter, floor_noise_shifter, peak_signal_shifter */
    { 12, 6, 1, 3, 7, 4},   /* preset #0 = VAD at 16kHz */
    { 12, 7, 2, 4, 7, 5},   /* preset #1 = VAD at 48kHz */
    { 12, 6, 0, 4, 6, 4},   /* preset #2 = peak detector 1 no HPF */
    { 12, 7, 2, 4, 7, 5},   /* preset #3 = peak detector 2 TBD */
};


extern void detector_processing (arm_detector_instance *instance, 
                     uint8_t *in, int32_t nb_data, 
                     int16_t *outBufs);
/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_detector (int32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *status)
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
            uint32_t *memresults = instance;
            uint16_t preset = RD(command, PRESET_CMD);
            uint8_t *pt8b, i, n;

            arm_detector_instance *pinstance = (arm_detector_instance *)(memresults[0]);
            
            /* here reset : clear the instance and preload in the memory instance the preset */
            /* here reset */
            pt8b = (uint8_t *) pinstance;
            n = sizeof(arm_detector_instance);
            for (i = 0; i < n; i++)
            {
                pt8b[i] = 0;
            }
            break;
        }    

        /* func(command = (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   uint8_t *pt8bsrc, *pt8bdst, i, n;
            uint8_t *new_parameters = (uint8_t *)data;
            arm_detector_instance *pinstance = (arm_detector_instance *) instance;

            /* copy the parameters */
            pt8bsrc = (uint8_t *) data;
            pt8bdst = (uint8_t *) &(pinstance->config);
            n = sizeof(detector_parameters);
            for (i = 0; i < n; i++)
            {   pt8bdst[i] = pt8bsrc[i];
            }
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
            arm_detector_instance *pinstance = (arm_detector_instance *) instance;
            int32_t nb_data, data_buffer_size, bufferout_free, increment;
            data_buffer_t *pt_pt;
            #define SAMP_IN uint8_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;   
            data_buffer_size    = pt_pt->size;
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 
            bufferout_free        = pt_pt->size;

            nb_data = data_buffer_size / sizeof(SAMP_IN);

            detector_processing (pinstance, inBuf, nb_data, outBuf);

            increment = (nb_data * sizeof(SAMP_IN));
            pt_pt = data;
            *(pt_pt->address) += increment;
            pt_pt ++;
            increment = (nb_data * sizeof(SAMP_OUT));
            *(pt_pt->address) += increment;

            
            break;
        }



        /* func(command = STREAM_END, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  

        case STREAM_READ_PARAMETER:  
        case STREAM_END:  
            break;    
    }
}