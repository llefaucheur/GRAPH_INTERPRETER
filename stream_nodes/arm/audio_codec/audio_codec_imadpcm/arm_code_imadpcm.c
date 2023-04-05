/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_imadpcm.c
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


typedef struct
{
    stream_entrance *stream_entry;
    float data0;
    float data1;
    float data2;
} arm_imadpcm_instance;


/**
  @brief         Processing function for the floating-point Biquad cascade filter.
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
int32_t arm_imadpcm_calls_stream (int32_t command, void *instance, void *data, void *parameters)
{
    stream_entrance *STREAM; /* function pointer used for debug/trace, memory move and free, signal processing */
    arm_imadpcm_instance *pinstance = (arm_imadpcm_instance *)instance;

    switch (command)
    {   
        //case STREAM_REGISTER_ME: first command to register the SWC
        //case STREAM_DEBUG_TRACE:
        //{  
        //}
        default: 
            return 0;
    }
    STREAM = pinstance->stream_entry;
    STREAM (command, instance, data, parameters);  /* single interface to STREAM for controls ! */
    return 0;
}


/**
  @brief         Processing function for the floating-point Biquad cascade filter.
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
void arm_imadpcm_run(arm_imadpcm_instance *instance, 
                     uint8_t *in, int32_t nb_data, 
                     int16_t *outBufs, int32_t nb_samples)
{
    /* fake decoder*/ 
    int i;
    for (i = 0; i < nb_samples; i++)
    {
        outBufs[i] = (int16_t)((int16_t)(in[i])<<8);
    }
}

/**
  @brief         Processing function for the floating-point Biquad cascade filter.
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
int32_t arm_imadpcm (int32_t command, void *instance, void *data, void *parameters)
{
    int32_t swc_returned_status = 0;

    switch (command)
    { 
        /* func(command = STREAM_RESET, 
               instance = *memory_results,  
               data = address of Stream
                parameter = #preset on LSB (0x0000.0PPP)  
                memory_results are followed by the first two words 
                of STREAM_FORMAT_SIZE_W32 

               memory pointers are in the same order as in MEMREQ, 
        */
        case STREAM_RESET: 
        {   stream_entrance *stream_entry = (stream_entrance *)(uint64_t)data;
            uint32_t *memresults = (uint32_t *)instance;
            uint16_t preset = (uint8_t)(uint64_t)parameters;

            arm_imadpcm_instance *pinstance = (arm_imadpcm_instance *) memresults++;
            /* here reset */

            break;
        }    

        /* func(command = STREAM_SET_PARAMETER, 
                *instance, 
                data = parameter(s) (one or all)
                parameters = 0x00TT.0PPP : tag of a parameter to set (MSB) on top of a default preset (LSB)
                    0xFFFF means "set all the parameters" in a raw
        */ 
        case STREAM_SET_PARAMETER:  
        {   
            uint8_t *new_parameters = (uint8_t *)data;
            break;
        }



        /* func(command = STREAM_READ_PARAMETER, 
                *instance, 
                data = *parameter(s) being read
                Index of parameter :  tag of a parameter to set, 0xFFFF means "read all the parameters"
        */ 
        case STREAM_READ_PARAMETER:  
        {   
            uint8_t *new_parameters = (uint8_t *)data;

            break;
        }
        


        /* func(command = STREAM_RUN, 
               instance,  
               data = array of [{*input size} {*output size}]
               parameters = preset (12bits)

               data format is given in the node's manifest used during the YML->graph translation
               this format can be FMT_INTERLEAVED or FMT_DEINTERLEAVED_1PTR
        */         
        case STREAM_RUN:   
        {
            int32_t nb_samples, nb_data, data_buffer_size, bufferout_free, increment;
            uint32_t *pt_pt;
            uint8_t *inBuf_encoded_data;
            int16_t *outBuf_decoded_sample;

            pt_pt = (uint32_t *)data;
            inBuf_encoded_data = (uint8_t *)(uint64_t)(*pt_pt++);   
            data_buffer_size = (uint32_t )(*pt_pt++);
            outBuf_decoded_sample = (int16_t *)(uint64_t)(*pt_pt++); 
            bufferout_free = (uint32_t) (*pt_pt++); 

            nb_data = data_buffer_size / sizeof(uint8_t);
            nb_samples = bufferout_free / sizeof(float);
            arm_imadpcm_run(instance, 
                    inBuf_encoded_data, nb_data, 
                    outBuf_decoded_sample, nb_samples );

            increment = (nb_data * sizeof(uint8_t));
            pt_pt = (uint32_t *)data;
            *pt_pt = (uint32_t)(uint64_t)(inBuf_encoded_data += increment);
            increment = (nb_samples * sizeof(int16_t));
            pt_pt += 2;
            *pt_pt = (uint32_t)(uint64_t)(outBuf_decoded_sample += increment);
            
            break;
        }



        /* func(command = STREAM_END, 
               instance,  
               data = unused
               parameters = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_END:  break;    
    }
    return swc_returned_status;
}