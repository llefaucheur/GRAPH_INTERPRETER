/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_gating.c
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

/*
  parameter format (12bits) : 
    2 : hpf coefficient
    2 : low-pass
    2 : fast filter
    2 : snr 
    1 : edge direction
    2 : hold time
    1 : (free)
*/
#define unused_GT_MSB U(11) 
#define unused_GT_LSB U(11) /* 1 */
#define   HOLD_GT_MSB U(10) 
#define   HOLD_GT_LSB U( 9) /* 2   */
#define   EDGE_GT_MSB U( 8) 
#define   EDGE_GT_LSB U( 8) /* 1   */
#define    SNR_GT_MSB U( 7) 
#define    SNR_GT_LSB U( 6) /* 2   */
#define   FAST_GT_MSB U( 4) 
#define   FAST_GT_LSB U( 4) /* 2  */
#define    LPF_GT_MSB U( 3) 
#define    LPF_GT_LSB U( 2) /* 2  */
#define    HPF_GT_MSB U( 1) 
#define    HPF_GT_LSB U( 0) /* 2  */


#define GATE_HPF 0
#define GATE_FAST 1
#define GATE_SLOW 2
#define GATE_DELAY 3

#define fp32_format 0

#define NPRESETS 4
/* coefficients (hpf,slow,fast,timer), rising/falling, signal to noise ratio */
typedef struct
{   
    uint8_t coef[6];        
} arm_stream_gating_parameters;

const arm_stream_gating_parameters presets[NPRESETS] = 
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
} arm_stream_gating_instance;

/**
  @brief         Processing function for detector and decision gating.
  @param[in]     in         input data pointer
  @param[in]     n          number of samples to process
  @param[out]    out        pointer to the output buffer
  @return        none
 */
void arm_stream_gating_run_float(arm_stream_gating_instance *instance, 
                     float *in,   
                     int32_t nb_samples, 
                     float *outBufs)
{
    int i;
    for (i = 0; i < nb_samples; i++)
    {   
        outBufs[i] = (in[i] > 0.1f)? 1.0f : 0.0f;
    }
}
/**
  @brief         Processing function for detector and decision gating.
  @param[in]     in         input data pointer
  @param[in]     n          number of samples to process
  @param[out]    out        pointer to the output buffer
  @return        none
 */
void arm_stream_gating_run_int16(arm_stream_gating_instance *instance, 
                     int16_t *in,   
                     int32_t nb_samples, 
                     int16_t *outBufs)
{
    int i;
    for (i = 0; i < nb_samples; i++)
    {   
        outBufs[i] = (in[i] > 3000)? 32767 : 0;
    }

}

/**
  @brief         Processing function for the floating-point Biquad cascade filter.
  @param[in]     command    reset, set/read parameter, run
  @param[in]     intance    points to the memory instance of the node, during command "run"
  @param[in]     data       pointer to the list of input and output buffers
  @param[in]     parameters a 12bits field indicating the use-case configuration
  @return        int32      "1" tells the processing of this data frame is finished, "0" otherwise
 */
int32_t arm_stream_gating (int32_t command, void *instance, void *data, void *parameters)
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
        */
        case STREAM_RESET: 
        {   
            uint32_t preset, *memreq = (uint32_t *)instance;
            arm_stream_gating_instance *pinstance = (arm_stream_gating_instance *)*memreq++;
            pinstance->state[GATE_HPF] = 0;
            pinstance->state[GATE_FAST] = 0;
            pinstance->state[GATE_SLOW] = 0;
            pinstance->state[GATE_DELAY] = 0;

            preset = (uint32_t)parameters;
            pinstance->coef[GATE_HPF] = RD(preset,HPF_GT);
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
            intPtr_t *pt_pt;
            arm_stream_gating_instance *pinstance = (arm_stream_gating_instance *)instance;
            pt_pt = (uint32_t *)data;
            
            if (pinstance->dataFormat == fp32_format)
            {
                float *inBuf;
                float *outBufs;

                inBuf = (float *)(uint64_t)(*pt_pt++);   
                buffer_size = (uint32_t )(*pt_pt++);
                outBufs = (float *)(uint64_t)(*pt_pt++); 
                bufferout_free = (uint32_t) (*pt_pt++); 

                nb_samples = buffer_size / sizeof(float);
                arm_stream_gating_run_float(instance, (float *)inBuf,  
                                          nb_samples, 
                                          outBufs
                                          );

                pt_pt = (uint32_t *)data;
                *pt_pt++ = (intPtr_t)(inBuf + nb_samples);
                *pt_pt++ = (intPtr_t)(nb_samples * sizeof(float));
                *pt_pt++ = (intPtr_t)(outBufs + nb_samples);
                *pt_pt++ = (intPtr_t)(bufferout_free - nb_samples * sizeof(float));
            }
            else
            {
                int16_t *inBuf;
                int16_t *outBufs;

                inBuf = (int16_t *)(uint64_t)(*pt_pt++);   
                buffer_size = (uint32_t )(*pt_pt++);
                outBufs = (int16_t *)(uint64_t)(*pt_pt++); 
                bufferout_free = (uint32_t) (*pt_pt++); 

                nb_samples = buffer_size / sizeof(int16_t);
                arm_stream_gating_run_int16(instance, (int16_t *)inBuf,  
                                          nb_samples, 
                                          outBufs
                                          );
                pt_pt = (uint32_t *)data;
                *pt_pt++ = (intPtr_t)(inBuf + nb_samples);
                *pt_pt++ = (intPtr_t)(buffer_size - nb_samples * sizeof(int16_t));
                *pt_pt++ = (intPtr_t)(outBufs + nb_samples);
                *pt_pt++ = (intPtr_t)(bufferout_free - nb_samples * sizeof(int16_t));
            }
            break;
        }

        case STREAM_SET_PARAMETER:  
        case STREAM_READ_PARAMETER:  
        case STREAM_END:  
        default:
            break;
    }
    return swc_returned_status;
}