/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_amplitude.c
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
    intPtr_t *TCM;                      /* Stream provides a pointer to TCM */
    float state[2];
    float coef[5];
} arm_stream_amplitude_instance;

/**
  @brief         Processing function for the floating-point Biquad cascade filter.
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
void arm_stream_amplitude_run(arm_stream_amplitude_instance *instance, 
                     float *in,   
                     int32_t nb_samples, 
                     float *outBufs)
{
    /* fake equalizer */
    MEMCPY(outBufs, in, nb_samples * sizeof(float));
}

/**
  @brief         Processing function for the floating-point Biquad cascade filter.
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
int32_t arm_stream_amplitude (int32_t command, void *instance, void *data, void *parameters)
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
        {   
            uint32_t preset, *memreq = (uint32_t *)instance;
            arm_stream_amplitude_instance *pinstance = (arm_stream_amplitude_instance *)*memreq++;
            pinstance->TCM =  (uint32_t *)*memreq++;

            preset = (uint32_t)(uint64_t)parameters;
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
            uint16_t param_tag;
            uint16_t preset;
            uint8_t *ptr_param8b;
            uint8_t *pdst;
            uint8_t numStages;
            float *pCoeffs;
            float *pState;
            arm_stream_amplitude_instance *pinstance = (arm_stream_amplitude_instance *) instance;

            ptr_param8b =  (uint8_t *)data;
            preset = 0xFFF & (uint64_t)parameters;
            param_tag = 0xFF & (((uint64_t)parameters)>>16);

            /* does all the parameters are set */
            if (param_tag == ALLPARAM_)
            {
                numStages = *ptr_param8b;
                pCoeffs = pinstance->coef;
                pState = pinstance->state;

                pdst = &numStages;
                MEMCPY(pdst,ptr_param8b,1); 
                ptr_param8b += 1;

                /* copy 5 coefficients */
                pdst = (uint8_t *) pCoeffs; 
                MEMCPY(pdst,ptr_param8b,5*sizeof(float)); 
                ptr_param8b += 5*sizeof(float);

                /* reset 2 state registers */
                pdst = (uint8_t *)pState;  
                memset(pdst,0,2*sizeof(float)); 
            }
            else
            {
            }

            //void arm_biquad_cascade_df2T_init_f32(
            //        arm_biquad_cascade_df2T_instance_f32 * S,
            //        uint8_t numStages,
            //  const float32_t * pCoeffs,
            //        float32_t * pState)

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
            int32_t nb_samples;
            int32_t buffer_size;
            int32_t bufferout_free;
            int32_t increment;
            uint32_t *pt_pt;
            float *inBuf;
            float *outBufs;

            pt_pt = (uint32_t *)data;
            inBuf = (float *)(uint64_t)(*pt_pt++);   
            buffer_size = (uint32_t )(*pt_pt++);
            outBufs = (float *)(uint64_t)(*pt_pt++); 
            bufferout_free = (uint32_t) (*pt_pt++); 

            nb_samples = buffer_size / sizeof(float);
            arm_stream_amplitude_run(instance, (float *)inBuf,  
                                      nb_samples, 
                                      outBufs
                                      );

            increment = (nb_samples * sizeof(float));
            pt_pt = (uint32_t *)data;
            *pt_pt = (uint32_t)(uint64_t)(inBuf += increment);
            increment = (nb_samples * sizeof(int16_t));
            pt_pt += 2;
            *pt_pt = (uint32_t)(uint64_t)(outBufs += increment);   /* only the pointer is used for arc updates */
            
            break;
        }



        /* func(command = STREAM_END, 
               instance,  
               data = unused
               parameters = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_END:  break;    /* func(STREAM_END, instance, 0) */
    }
    return swc_returned_status;
}