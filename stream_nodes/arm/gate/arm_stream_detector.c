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

/*
    9.	stream_detector
    Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
    a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time 
    for debouncing and LED / user-interface).
    
    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.
*/

typedef struct
{
    uint32_t debouncing_counter; /* sample counter : maintain the "detected" flag at least for this number of samples */
    uint8_t log2decfMAX;        /* decimation will be a power of 2 (-1)*/
    uint8_t high_pass_shifter;  /* z1 */
    uint8_t low_pass_shifter;   /* z6 */
    uint8_t floor_noise_shifter;/* z7 */
    uint8_t peak_signal_shifter;/* z8 */
} detector_configuration;

typedef struct
{
    detector_configuration config;

    int16_t z1;    /* memory of the high-pass filter (recursive part) */
    int16_t z2;    /* memory of the high-pass filter (direct part) */
    int16_t z3;    /* output of the high-pass filter */
    int16_t z6;    /* memory of the first low-pass filter */
    int16_t z7;    /* memory of the floor-noise tracking low-pass filter */
    int16_t z8;    /* memory of the envelope tracking low-pass filter */
    int32_t down_counter;    /* memory of the debouncing downcounter  */
} arm_detector_instance;

#define NB_PRESET 4
const detector_configuration detector_preset [NB_PRESET] = 
{
    { 10000, 7, 1, 4, 7, 5},   /* preset #0 = VAD at 16kHz */
    { 10000, 7, 1, 4, 7, 5},   /* preset #1 = VAD at 48kHz */
    { 10000, 7, 1, 4, 7, 5},   /* preset #2 = peak detector 1 TBD */
    { 10000, 7, 1, 4, 7, 5},   /* preset #3 = peak detector 2 TBD */
};

/**
  @brief         Processing function 
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data = 0x7FFF when detected, else 0
  @param[in]     blockSize  number of samples to process
  @return        none
 */
void detector_processing (arm_detector_instance *instance, 
                     uint8_t *in, int32_t nb_data, 
                     int16_t *outBufs)
{
    int i;
    for (i = 0; i < nb_data; i++)
    {
        outBufs[i] = (int16_t)((int16_t)(in[i])<<8);
    }
}

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

            arm_detector_instance *pinstance = (arm_detector_instance *) memresults++;
            
            /* here reset : clear the instance and preload in the memory instance the preset */

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