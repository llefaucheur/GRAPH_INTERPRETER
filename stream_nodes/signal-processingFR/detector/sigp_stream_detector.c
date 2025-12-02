/* ----------------------------------------------------------------------
 * Title:        sigp_stream_detector.c
 * Description:  filters
 *
 * $Date:        15 February 2024
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2013-2024 signal-processing.fr. All rights reserved.
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

#include "presets.h"
#ifdef CODE_SIGP_STREAM_DETECTOR

#ifdef __cplusplus
 extern "C" {
#endif
   
 
#include <stdint.h>
#include "stream_common_const.h"        // 
#include "stream_common_types.h"        // types for the interface to the scheduler

#include "sigp_stream_detector.h"

/*
;----------------------------------------------------------------------------------------
;7.	sigp_stream_detector
;----------------------------------------------------------------------------------------
;   Operation : provides a boolean output stream from the detection of a rising 
;   edge above a tunable signal to noise ratio. 
;   A tunable delay allows to maintain the boolean value for a minimum amount of time 
;   Use-case example 1: debouncing analog input and LED / user-interface.
;   Use-case example 2: IMU and voice activity detection (VAD)
;   Parameters : time-constant to gate the output, sensitivity of the use-case
;
;   presets control
;   #1 : no HPF pre-filtering, fast and high sensitivity detection (button debouncing)
;   #2 : VAD with HPF pre-filtering, time constants tuned for ~10kHz
;   #3 : VAD with HPF pre-filtering, time constants tuned for ~44.1kHz
;   #4 : IMU detector : HPF, slow time constants
;   #5 : IMU detector : HPF, fast time constants
;
;   preset parameter : 8bit sensitivity
;
;   Metadata information can be extracted with the command "TAG_CMD" from parameter-read:
;   0 read the floor noise level
;   1 read the current signal peak
;   2 read the signal to noise ratio
;
sigp_stream_detector
    3  i8; 0 0 0        instance, preset, tag
    PARSTART
    8; i8; 1;2;3;4;5;6;7;8; the 8 bytes of "struct detector_parameters"
    PARSTOP  
*/

#ifndef CODE_SIGP_STREAM_DETECTOR
void sigp_stream_detector (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status) {}
#else
#define NB_PRESET 6
const detector_parameters detector_preset [NB_PRESET] = 
{   /*  log2counter, log2decfMASK, 
        high_pass_shifter, low_pass_shifter, low_pass_z7_z8,  
        vad_rise, vad_fall, THR */

    {MINIF(1,12), 8,   3, 6, 11,  MINIF(1,18), MINIF(1,20), MINIF(3,0)}, /* #0 no HPF, fast for button debouncing */
    {MINIF(1,12), 8,   3, 6, 11,  MINIF(1,18), MINIF(1,20), MINIF(3,0)}, /* #1 VAD with HPF pre-filtering, tuned for Fs <20kHz */
    {MINIF(1,12), 8,   3, 6, 11,  MINIF(1,18), MINIF(1,20), MINIF(3,0)}, /* #2 VAD with HPF pre-filtering, tuned for Fs >20kHz */
    {MINIF(1,12), 8,   3, 6, 11,  MINIF(1,18), MINIF(1,20), MINIF(3,0)}, /* #3 IMU detector : HPF, slow time constants */
    {MINIF(1,12), 8,   3, 6, 11,  MINIF(1,18), MINIF(1,20), MINIF(3,0)}, /* #4 IMU detector : HPF, fast time constants */
    {MINIF(1,12), 6,   4, 6, 11,  MINIF(1,20), MINIF(1,22), MINIF(4,0)}, /* #5 same as #1 with faster reaction time  */
};

/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void sigp_stream_detector (unsigned int command, void *instance, void *data, unsigned int *status)
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
        {   //stream_services *stream_entry = (stream_services *)data;
            intptr_t *memresults = (intptr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            sigp_detector_instance *pinstance = (sigp_detector_instance *) *memresults++;
            pinstance->backup =  (sigp_backup_memory *)*memresults++;

            /* trace ID */
            pinstance->traceID_tag = RD(command, NODE_TAG_CMD);

            /* reset the instance */            
            /* floor noise is preserved after a wsigp boot */
            if (COMMDEXT_COLD_BOOT == RD(command, COMMDEXT_CMD))
            {
                /* here COLD reset */
                pinstance->z1 = F2Q31(0.00001);
                pinstance->z6 = F2Q31(0.00001);
            }
            else /* wsigp boot */
            {
                //int32_t z1;    /* memory of the high-pass filter (recursive part) */
                //int32_t z2;    /* memory of the high-pass filter (direct part) */
                //int32_t z3;    /* output of the high-pass filter */
                //int32_t z6;    /* memory of the first low-pass filter */

                //int32_t z7;    /* memory of the floor-noise tracking low-pass filter */
                //int32_t z8;    /* memory of the envelope tracking low-pass filter */
                //int32_t accvad;/* accumulator / estimation */
                //int32_t Flag;  /* accumulator 2 / estimation  */
                //int32_t down_counter;    /* memory of the debouncing downcounter  */
                //uint8_t previous_vad; 
            }

            pinstance->config = detector_preset[preset];    /* preset data move */
            pinstance->services = (stream_services *)data;
            pinstance->decf = decfMASK; 
            pinstance->backup->z8 = F2Q31(0.00001);
            pinstance->backup->z7 = F2Q31(0.001);
            pinstance->backup->down_counter = 0;

            break;
        }  
        
        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:
        {   sigp_detector_instance *pinstance = (sigp_detector_instance *) instance;
            pinstance->config = detector_preset[RD(command,NODE_TAG_CMD)];

            if (RD(command,NODE_TAG_CMD) == NODE_ALL_PARAM) 
            {   uint8_t *pt8bsrc, *pt8bdst, i, n;

                /* copy the parameters */
                n = sizeof(detector_parameters);   
                pt8bsrc = (uint8_t *) data;     
                pt8bdst = (uint8_t *) &(pinstance->config);
                for (i = 0; i < n; i++)
                {   pt8bdst[i] = pt8bsrc[i];
                }
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
            sigp_detector_instance *pinstance = (sigp_detector_instance *) instance;
            intptr_t nb_data, stream_xdmbuffer_size, bufferout_free;
            stream_xdmbuffer_t *pt_pt;
            #define SAMP_IN int16_t 
            #define SAMP_OUT int32_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;
            stream_xdmbuffer_size    = pt_pt->size;
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 
            bufferout_free        = pt_pt->size;

            nb_data = stream_xdmbuffer_size / sizeof(SAMP_IN);

            sigp_stream_detector_process (pinstance, inBuf, (int32_t)nb_data, outBuf);

            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = 1 * sizeof(SAMP_OUT);   /* amount of data produced */
            break;
        }
        case STREAM_STOP :
        {
            break;
        }
    }
}

#endif

#ifdef __cplusplus
}
#endif
 
#else
void sigp_stream_detector (unsigned int command, void *instance, void *data, unsigned int *status) { /* fake access */ if(command || instance || data || status) return;}
#endif
