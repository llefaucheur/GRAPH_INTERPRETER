/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_format_convert.c
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

/*-------------------------------------
    data format conversion
---------------------------------------
*/


enum conversion_presets
{
    //q31_to_fp32 = 1,
    //fp32_to_q31 = 2,
    q15_to_fp32 = 3,
    fp32_to_q15 = 4,
};

/**
  @brief         Processing function for the stream format converter
  @param[in]     in         input data pointer
  @param[in]     n          number of samples to process
  @param[out]    out        pointer to the output buffer
  @return        none
 */
void arm_format_converter_run(
                     void *in,   
                     int32_t n, 
                     void *out,
                     uint16_t preset)
{
    int16_t *p16, i;
    int32_t *p32;
    float *fp32;

    switch (preset)
    {
    //case q31_to_fp32: 
    //    p32=(int32_t*)in; 
    //    fp32=(float*)out; 
    //    for(i=0;i<n;i++){(*fp32++) = (*p32++)/2.15e9f;}
    //    break;
    //case fp32_to_q31: 
    //    p32=(int32_t*)out; 
    //    fp32=(float*)in; 
    //    for(i=0;i<n;i++){(*p32++) = (int32_t)((*fp32++)*2.15e9f);}
    //    break;
    case q15_to_fp32: 
        p16=(int16_t*)in; 
        fp32=(float*)out; 
        for(i=0;i<n;i++){(*fp32++) = (*p16++)/3.3e4f;}
        break;
    case fp32_to_q15: 
        p16=(int16_t*)out; 
        fp32=(float*)in; 
        for(i=0;i<n;i++){(*p16++) = (int16_t)((*fp32++)*3.3e4f);}
        break;
    }
}

/**
  @brief         Interface of the format converter
  @param[in]     command    reset, set/read parameter, run
  @param[in]     intance    points to the memory instance of the node, during command "run"
  @param[in]     data       pointer to the list of input and output buffers
  @param[in]     parameters a 12bits field indicating the use-case configuration
  @return        int32      "1" tells the processing of this data frame is finished, "0" otherwise
 */
int32_t arm_format_converter (int32_t command, void *instance, void *data, void *parameters)
{
    switch (command)
    { 
        /* func(command = STREAM_RESET, 
               instance = *memory_results,  
               data = address of Stream
               parameter = #preset on LSB (0x0000.0PPP)  

               memory pointers are in the same order as in MEMREQ, 
        */
        case STREAM_RESET: 
        {   stream_entrance *stream_entry = (stream_entrance *)(uint64_t)data;
            uint16_t preset = (uint8_t)(uint64_t)parameters;
            /* here reset */
            break;
        }    

        /* func(command = STREAM_SET_PARAMETER, 
                *instance, 
                data = parameter(s) (one or all)
                parameters = 0x00TT.0PPP : tag of a parameter to set (MSB) on top of a default preset (LSB)
                xxxx = FFFF means memory_results are followed by the 
                the sampling-rate, number of channel and interleaving scheme
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
            int32_t nb_samples;
            int32_t buffer_size;
            int32_t bufferout_free;
            int32_t increment;
            uint32_t *pt_pt;
            uint8_t *inBuf;
            uint8_t *outBufs;
            uint16_t preset = (uint8_t)(uint64_t)parameters;

            pt_pt = (uint32_t *)data;
            inBuf = (uint8_t *)(uint64_t)(*pt_pt++);   
            buffer_size = (uint32_t )(*pt_pt++);
            outBufs = (uint8_t *)(uint64_t)(*pt_pt++); 
            bufferout_free = (uint32_t) (*pt_pt++); 

            nb_samples = buffer_size / sizeof(float);
            arm_format_converter_run(inBuf, nb_samples, outBufs, preset);

            increment = (nb_samples * sizeof(float));
            pt_pt = (uint32_t *)data;
            *pt_pt = (uint32_t)(uint64_t)(inBuf += increment);
            increment = (nb_samples * sizeof(int16_t));
            pt_pt += 2;
            *pt_pt = (uint32_t)(uint64_t)(outBufs += increment);
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
    return 1;
}