/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_interpolator.c
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
#include "arm_stream_interpolator.h"

/*-------------------------------------
    data format conversion
---------------------------------------
*/
extern void arm_stream_interpolator_process (arm_stream_interpolator_instance *instance, uint8_t TAG_CMD, stream_xdmbuffer_t *in_out);

/*
;----------------------------------------------------------------------------------------
;Node #3.	arm_stream_interpolator
;----------------------------------------------------------------------------------------
;   Operation : converter raw input data format to the format of the output arc. 
;     + synchronous rate convertion with +/- 100ppm adjustment
;     + conversion of time-stamp asynchronous to synchronous stream
;   
;   presets bits :
;   0,1 : raw data conversion (0), in/out formats exchanged during STREAM_RESET stage
          SSRC (1) synchronous rate converter, FS in/out are exchanged during STREAM_RESET
          ASRC (2) asynchronous rate converter using time-stamps (in) to synchronous FS (out)
          unused(3)
;
;   drift compensation managed with STREAM_SET_PARAMETER command:
;   TAG_CMD = 0 to stop drift compensation
;   TAG_CMD = 1 to apply a drift compensation of +100ppm
;   TAG_CMD = 2 to apply a drift compensation of -100ppm
;
    arm_stream_interpolator; 0 instance; 0 raw conversion; 0 no parameter; 
*/



/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_interpolator (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = SWC_TASK_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, PRESET, TAG, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                    => used by the router to know the size of each raw element 
                    (see stream_bitsize_of_raw())
                memory pointers are in the same order as described in the SWC manifest
        */
        case STREAM_RESET: 
        {   stream_services_entry *stream_entry = (stream_services_entry *)(uint64_t)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_stream_interpolator_instance *pinstance = (arm_stream_interpolator_instance *) *memresults;
            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = parameters
        */ 
        case STREAM_SET_PARAMETER:  
        {  // uint16_t *pt8bsrc, *pt8bdst, i, n;
            
            arm_stream_interpolator_instance *pinstance = (arm_stream_interpolator_instance *) instance;

            /* copy the parameters from preset or from the graph */
            if (RD(command, PRESET_CMD) == 0)
            { 
            //arm_stream_interpolator_process ()
            }
            break;
        }

        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input size} {*output size}]
        */         
        case STREAM_RUN:   
        {
            arm_stream_interpolator_instance *pinstance = (arm_stream_interpolator_instance *) instance;
            //intPtr_t nb_data, stream_xdmbuffer_size, bufferout_free;
            //stream_xdmbuffer_t *pt_pt;

            //pt_pt = data;
            //router_processing (pinstance, pt_pt);
            //break;
        }

        default :
            break;
    }
}
#ifdef __cplusplus
}
#endif
 