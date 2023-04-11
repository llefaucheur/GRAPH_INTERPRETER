/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_detector.c
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

/*
    stream_gating

    Operation : provides a boolean output stream from the detection of a rising (of falling) 
    edge above a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time (for 
    debouncing and LED / user-interface).

    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.
*/

#define NPRESETS 4
/* coefficients (hpf,slow,fast,timer), rising/falling, signal to noise ratio */
typedef struct
{   
    uint8_t coef[6];        
} arm_stream_detector_parameters;

const arm_stream_detector_parameters presets[NPRESETS] = 
{
    {0,0,0,0,0,0},  /* energy detection , no HPF , */
    {0,0,0,0,0,0},  /* digital detection, DC canceller (HPF 1/256)  */
    {0,0,0,0,0,0},  /* audio detection in noise (HPF 1/16) */
    {0,0,0,0,0,0},  /* audio detection in silence (HPF 1/64) */
};


typedef struct
{   
    int16_t state[4];       /* filter states (hpf,slow,fast,counter) */
    uint8_t coef[4];        /* coefficients (hpf,slow,fast,timer) */
    uint8_t edge;           /* rising/falling */
    uint8_t snr;            /* signal to noise ratio */
    uint8_t dataFormat;     /* float/Q15 operation */
} arm_stream_detector_instance;




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

    switch (command)
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

            arm_stream_detector_instance *pinstance = (arm_stream_detector_instance *) memresults++;
            /* here reset */

            break;
        }   
  

        /* func(command = (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
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
            int32_t nb_data, data_buffer_size, bufferout_free, increment;
            data_buffer_t *pt_pt;
            uint8_t *inBuf_encoded_data;
            int16_t *outBuf_decoded_sample;
            arm_stream_detector_instance *pinstance = (arm_stream_detector_instance *) instance;

            pt_pt = data;
            inBuf_encoded_data  = pt_pt->address;   
            data_buffer_size    = pt_pt->size;
            pt_pt++;
            outBuf_decoded_sample = (int16_t *)(pt_pt->address); 
            bufferout_free        = pt_pt->size;

            nb_data = data_buffer_size / sizeof(uint8_t);

            //arm_imadpcm_run(pinstance, 
            //        inBuf_encoded_data, nb_data, 
            //        outBuf_decoded_sample);

            increment = (nb_data * sizeof(uint8_t));
            pt_pt = data;
            *(pt_pt->address) += increment;
            pt_pt->size       -= increment;
            pt_pt ++;
            increment = (nb_data * sizeof(int16_t));
            *(pt_pt->address) += increment;
            pt_pt->size       -= increment;
            
            break;
        }


        case STREAM_READ_PARAMETER:  
        case STREAM_END:  
        default:
            break;
    }
}