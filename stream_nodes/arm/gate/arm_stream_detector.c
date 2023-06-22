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
{   /* log2counter, log2decfMAX, 
        high_pass_shifter, low_pass_shifter, low_pass_z7_z8,  
        vad_rise, vad_fall, THR */
    {12, 8,  3, 6, 12, MINIF(3,12), MINIF(1,7), 3},   /* preset #0 = VAD at 16kHz */
    {12, 8,  3, 6, 12, MINIF(3,5),  MINIF(25,0),3},   /* preset #1 = VAD at 48kHz */
    {12, 8,  4, 5,  5, MINIF(3,12), MINIF(1,7), 3},   /* preset #2 = Accelerometer 103Hz */
    {12, 8,  3, 6, 12, MINIF(3,12), MINIF(1,7), 3},   /* preset #3 = peak detector */
};

char dbg[100];
long x;


extern void detector_processing (arm_detector_instance *instance, 
                     int16_t *in, int32_t nb_data, 
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
            break;
        }  
        
        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:
        { 
            arm_detector_instance *pinstance = (arm_detector_instance *) instance;
               if (RD(command,TAG_CMD) == 0xFF) {

                switch (RD(command, PRESET_CMD)){
                    case 0 :{ 
                        // pinstance->config = detector_preset[0];
                        printf("Unsupported Preset 0 \n");}
                        break;
                    case STREAM_DETECTOR_PRESET_VAD_48kHz :{
                        pinstance->config = detector_preset[STREAM_DETECTOR_PRESET_VAD_48kHz];
                        printf("Preset VAD 48kHz \n");}

                        break;
                    case STREAM_DETECTOR_PRESET_ACCEL_103Hz : {
                        pinstance->config = detector_preset[STREAM_DETECTOR_PRESET_ACCEL_103Hz];
                        printf("Preset Accel 103Hz \n");}
                        break;
                    
                    case 3 :{ 
                        // pinstance->config = detector_preset[3];
                        printf("Unsupported Preset 3\n");}
                        break;
                }
            }
            //TODO Add support for configuring individual parameters of presets
            // uint8_t *pt8bsrc, *pt8bdst, i, n;
            // arm_detector_instance *pinstance = (arm_detector_instance *) instance;

            // /* copy the parameters */
            // pt8bsrc = (uint8_t *) data;
            // pt8bdst = (uint8_t *) &(pinstance->config);

            // /* copy all the parameter */
            // if (RD(command, TAG_CMD) == 0xFF)
            // {   n = sizeof(detector_parameters);
            //     for (i = 0; i < n; i++)
            //     {   pt8bdst[i] = pt8bsrc[i];
            //     }
            //     break;
            // } 
            // /* copy a single parameter indexed by TAG, after loading the preset */
            // else 
            // {   pinstance->config = detector_preset[RD(command, PRESET_CMD)];
            //     pt8bdst = &(pt8bdst[RD(command, TAG_CMD)]);
            //     (*pt8bdst) = (*pt8bsrc);
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
            intPtr_t nb_data, data_buffer_size, bufferout_free;
            data_buffer_t *pt_pt;
            #define SAMP_IN int16_t 
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
            detector_processing (pinstance, inBuf, (int32_t)nb_data, outBuf);

            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_OUT);   /* amount of data produced */

            //--------------------DEBUG---------------------
            //sprintf (dbg, "Z2%5d Z7%3d Z8%4d VAD%2d", (pinstance->z2)>>12, (pinstance->z7)>>12, (pinstance->z8)>>12, outBuf[0]);
            //arm_stream_services(PACK_SERVICE(RD(command,INST_CMD), STREAM_DEBUG_TRACE), dbg, (uint8_t *)strlen(dbg), 0);            
/*            x = (inBuf[0])<<12;  memcpy(&(dbg[0]), &x, 4);
            x = (pinstance->z2)<<2;  memcpy(&(dbg[4]), &x, 4);
            x = (pinstance->z3)<<2;  memcpy(&(dbg[8]), &x, 4);
            x = (pinstance->accvad)<<2; memcpy(&(dbg[12]),&x, 4);
            x = (outBuf[0])<<15;   memcpy(&(dbg[16]),&x, 4);
            arm_stream_services(PACK_SERVICE(RD(command,INST_CMD), STREAM_DEBUG_TRACE), dbg, (uint8_t *)20, 0);     */       
            
            break;
        }

        /* func(command = STREAM_END, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  

        case STREAM_READ_PARAMETER:  
        case STREAM_STOP:  
            break;    
    }
}

#ifdef __cplusplus
}
#endif
 