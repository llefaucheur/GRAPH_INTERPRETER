/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_convert.c
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
void arm_stream_converter_run(
                     void *in,   
                     int32_t n, 
                     void *out,
                     uint16_t preset)
{
    int16_t *p16, i;
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
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_converter (int32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *status)
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
            int32_t nb_data, data_buffer_size, bufferout_free;
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

            //processing(pinstance, inBuf, nb_data, outBuf);

            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_OUT);   /* amount of data produced */

            
            break;
        }



        /* func(command = STREAM_END, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_END:  break;    
    }
}