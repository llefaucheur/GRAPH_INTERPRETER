/* ----------------------------------------------------------------------
 

        WORK ON GOING



* Project:      CMSIS Stream
 * Title:        arm_stream_anr.c
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


#include "presets.h"
#include <string.h>

#ifdef CODE_ARM_ANR

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_anr.h"

void arm_stream_anr (uint32_t command, void *instance, void *data, uint32_t *status);

long total_speex_alloc;

/*
;----------------------------------------------------------------------------------------
;6.	arm_stream_anr
;----------------------------------------------------------------------------------------

*/


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_anr (uint32_t command, void *instance, void *data, uint32_t *status)
{

    *status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID/tag, NB ARCS IN/OUT)
                instance = memory banks 
                data = address of Stream function
                
                memresults addresses ("intptr_t") in the same order as described in the NODE manifest followed by 
                    the optional user KEY and platform Key 
                    the 4 words of Rx/Tx formats all the arcss 

                in the case of arm_stream_anr : 
                    memresult[0] : instance of the component

                    memresult[1] : input arc Word 0 FRAMESZ_FMT0 (frame size..)
                    memresult[4] : input arc Word 3 DOMAINSPECIFIC_FMT3 
                    ..
                    memresult[5] : output arc Word 0 FRAMESZ_FMT0 
                    memresult[8] : output arc Word 1 DOMAINSPECIFIC_FMT3 

                preset (8bits) : number of biquads in cascade, max = 4, from NODE manifest 
                tag (8bits)  : unused
        */
        case STREAM_RESET: 
        {   
            intptr_t * memresult;
            uint32_t frame_size = 256, sampling_rate = 16000;
            SpeexPreprocessState* p_state; 

            memresult = (intptr_t*)*(intptr_t*)instance;
            memresult[1] = 0;
            memresult[0] = (intptr_t)  & (memresult[2]);    // [0]= pointer, [1]=cumul, [2]=start of the heap

            p_state = speex_preprocess_state_init((uint8_t *) & (memresult[0]), frame_size, sampling_rate);
            break;
        }       

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   SpeexPreprocessState* p_state;
            p_state = (SpeexPreprocessState*) instance;
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
            intptr_t* memresult;
            uint32_t frame_size = 256, sampling_rate = 16000;
            SpeexPreprocessState* p_state;
            intptr_t nb_data;
            intptr_t stream_xdmbuffer_size_in, stream_xdmbuffer_size_out;
            stream_xdmbuffer_t *pt_pt;
            int16_t *inBuf;
            int16_t *outBuf;


            memresult = (intptr_t*)instance;
            memresult = memresult + 2;
            p_state = (SpeexPreprocessState*)memresult;    // [0]= pointer, [1]=cumul, [2]=INSTANCE

            pt_pt = data;   inBuf = (int16_t *)pt_pt->address;   
                            stream_xdmbuffer_size_in = pt_pt->size;  /* data amount in the input buffer */
            pt_pt++;        outBuf = (int16_t *)(pt_pt->address); 
                            stream_xdmbuffer_size_out = pt_pt->size;  /* data free in the input buffer */
                            
            if (stream_xdmbuffer_size_in > stream_xdmbuffer_size_out) {
                nb_data = stream_xdmbuffer_size_out / sizeof(int16_t);
            } else  {
                nb_data = stream_xdmbuffer_size_in / sizeof(int16_t);
            }

            speex_preprocess_run(p_state, inBuf);   /* in-place processing */
            memcpy(outBuf, inBuf, nb_data * sizeof(int16_t)); /* copy the result */

            pt_pt = data;   *(&(pt_pt->size)) = nb_data * sizeof(int16_t); /* amount of data consumed */
            pt_pt ++;       *(&(pt_pt->size)) = nb_data * sizeof(int16_t); /* amount of data produced */
            
            break;


        }

        case STREAM_READ_PARAMETER:
        case STREAM_UPDATE_RELOCATABLE:
        default : break;
    }
}

#ifdef __cplusplus
}
#endif
#else
     void arm_stream_anr(uint32_t command, void* instance, void* data, uint32_t* status)
     {
     }
#endif  // #if CODE_ARM_ANR