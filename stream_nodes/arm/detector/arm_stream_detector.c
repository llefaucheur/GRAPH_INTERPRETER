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

#ifdef __cplusplus
 extern "C" {
#endif
   
 

#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>  /* for sprintf */
#include <string.h>  /* for memset */

#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  

#include "arm_stream_detector.h"


/*
;----------------------------------------------------------------------------------------
;7.	arm_stream_detector
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
arm_stream_detector
    3  i8; 0 0 0        instance, preset, tag
    PARSTART
    8; i8; 1;2;3;4;5;6;7;8; the 8 bytes of "struct detector_parameters"
    PARSTOP  
*/

#define NB_PRESET 5
const detector_parameters detector_preset [NB_PRESET] = 
{   /* log2counter, log2decfMAX, 
        high_pass_shifter, low_pass_shifter, low_pass_z7_z8,  
        vad_rise, vad_fall, THR */
    {12, 8,   3, 6, 12,  MINIF(3,12), MINIF(1,7), 3}, /* #1 no HPF, fast for button debouncing */
    {12, 8,   3, 6, 12,  MINIF(3,5),  MINIF(25,0),3}, /* #2 VAD with HPF pre-filtering, tuned for Fs <20kHz */
    {12, 8,   4, 5,  5,  MINIF(3,12), MINIF(1,7), 3}, /* #3 VAD with HPF pre-filtering, tuned for Fs >20kHz */
    {12, 8,   3, 6, 12,  MINIF(3,12), MINIF(1,7), 3}, /* #4 IMU detector : HPF, slow time constants */
    {12, 8,   3, 6, 12,  MINIF(3,12), MINIF(1,7), 3}, /* #5 IMU detector : HPF, fast time constants */
};

char dbg[100];
long x;

/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_detector (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = 1;    /* default return status, unless processing is not finished */
    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, PRESET, TAG, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function

                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the SWC manifest
        */
        case STREAM_RESET: 
        {   stream_services_entry *stream_entry = (stream_services_entry *)(uint64_t)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);
            uint8_t *pt8bdst, i, n;

            arm_detector_instance *pinstance = (arm_detector_instance *) *memresults;

            /* here reset */
            pt8bdst = (uint8_t *) pinstance;
            n = sizeof(arm_detector_instance);
            for (i = 0; i < n; i++)
            {   pt8bdst[i] = 0;
            }
            pinstance->config = detector_preset[preset];    /* preset data move */

            pinstance->z1 = F2Q31(0.001);
            pinstance->z6 = F2Q31(0.001);
            pinstance->z7 = F2Q31(0.001);
            pinstance->z8 = F2Q31(0.001);
            
            break;
        }  
        
        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:
        {   arm_detector_instance *pinstance = (arm_detector_instance *) instance;
            pinstance->config = detector_preset[RD(command,TAG_CMD)];

            if (RD(command,TAG_CMD) == 0xFF) 
            {   if (RD(command, TAG_CMD) == 0xFF)  /* copy all the parameter */
                {   uint8_t *pt8bsrc, *pt8bdst, i, n;

                    n = sizeof(detector_parameters);
                    pt8bsrc = (uint8_t *) data;     
                    pt8bdst = (uint8_t *) &(pinstance->config);
                    for (i = 0; i < n; i++)
                    {   pt8bdst[i] = pt8bsrc[i];
                    }
                } 
            }
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
            intPtr_t nb_data, stream_xdmbuffer_size, bufferout_free;
            stream_xdmbuffer_t *pt_pt;
            #define SAMP_IN int16_t 
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
            arm_stream_detector_process (pinstance, inBuf, (int32_t)nb_data, outBuf);

            /* the SWC is producing an amount of data different from the consumed one (see xdm11 in the manifest) */
            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_OUT);   /* amount of data produced */

            //--------------------DEBUG---------------------
            //sprintf (dbg, "Z2%5d Z7%3d Z8%4d VAD%2d", (pinstance->z2)>>12, (pinstance->z7)>>12, (pinstance->z8)>>12, outBuf[0]);
            //arm_stream_services(PACK_SERVICE(RD(command,INST_CMD), STREAM_SERVICE_INTERNAL_DEBUG_TRACE), dbg, (uint8_t *)strlen(dbg), 0);            
/*            x = (inBuf[0])<<12;  memcpy(&(dbg[0]), &x, 4);
            x = (pinstance->z2)<<2;  memcpy(&(dbg[4]), &x, 4);
            x = (pinstance->z3)<<2;  memcpy(&(dbg[8]), &x, 4);
            x = (pinstance->accvad)<<2; memcpy(&(dbg[12]),&x, 4);
            x = (outBuf[0])<<15;   memcpy(&(dbg[16]),&x, 4);
            arm_stream_services(PACK_SERVICE(RD(command,INST_CMD), STREAM_SERVICE_INTERNAL_DEBUG_TRACE), dbg, (uint8_t *)20, 0);     */       
            
            break;
        }
        case STREAM_STOP :
        {
            break;
        }
    }
}

#ifdef __cplusplus
}
#endif
 