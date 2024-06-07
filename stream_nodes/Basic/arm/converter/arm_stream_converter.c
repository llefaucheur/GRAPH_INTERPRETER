/* ----------------------------------------------------------------------


        WORK ON GOING



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

#ifdef __cplusplus
 extern "C" {
#endif
   


#include "stream_const.h"
#include "stream_types.h"
#include "arm_stream_converter.h"


/*
;----------------------------------------------------------------------------------------
;3.	arm_stream_converter
;----------------------------------------------------------------------------------------
;   Operation : convert input raw data format to the format of the output arc. 
;   
arm_stream_converter
    3  i8; 0 0 0        instance, task = raw conversion, no tag. No parameter
*/



/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_converter (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                    => used by the router to know the size of each raw element 
                    (see stream_bitsize_of_raw())
                memory pointers are in the same order as described in the SWC manifest
        */
        case STREAM_RESET: 
        {   stream_al_services *stream_entry = (stream_al_services *)(uint64_t)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_stream_converter_instance *pinstance = (arm_stream_converter_instance *) *memresults;

            /* read the stream format information */
            pinstance->input.raw_fmt = RD(memresults[2], RAW_FMT1);
            pinstance->input.nchan_fmt = RD(memresults[2], NCHANM1_FMT1) +1;
            pinstance->input.interleaving_fmt = RD(memresults[2], INTERLEAV_FMT1);
            pinstance->input.framesize_fmt = RD(memresults[2], FRAMESIZE_FMT0);

            pinstance->output.raw_fmt = RD(memresults[2], RAW_FMT1);
            pinstance->output.nchan_fmt = RD(memresults[2], NCHANM1_FMT1) +1;
            pinstance->output.interleaving_fmt = RD(memresults[2], INTERLEAV_FMT1);
            pinstance->output.framesize_fmt = RD(memresults[2], FRAMESIZE_FMT0);

            break;
        }    

        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input size} {*output size}]
        */         
        case STREAM_RUN:   
        {
            arm_stream_converter_instance *pinstance = (arm_stream_converter_instance *) instance;
            intPtr_t nb_data, stream_xdmbuffer_size;
            stream_xdmbuffer_t *pt_pt;

            #define SAMP_IN int16_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;   
            stream_xdmbuffer_size    = pt_pt->size;  /* data amount in the original buffer */
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 

            nb_data = stream_xdmbuffer_size / sizeof(SAMP_IN);

            pt_pt = data;
            arm_stream_converter_process (&(pinstance->input), &(pinstance->output), pt_pt);

            break;
        }

        default :
            break;
    }
}
#ifdef __cplusplus
}
#endif
 