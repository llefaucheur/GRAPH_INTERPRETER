/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_rescaler.c
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
#include "arm_stream_rescaler.h"

/*
;----------------------------------------------------------------------------------------
;8.	arm_stream_rescaler
;----------------------------------------------------------------------------------------
;    Operation : applies vq = interp1(x,v,xq) using int16 data format
;    Following https://fr.mathworks.com/help/matlab/ref/interp1.html
;    Parameters : X,V vectors, size max = 32 points
;
;   presets control
;   #1 : linear interpolation
;   #2 : second-order polynomial interpolation 
;
arm_stream_rescaler; 
    3  i8; 0 1 0        instance, preset, tag
    PARSTART
    2; i16; -32768; 32768;
    2; i16; -20000; 20000;  this table creates the abs(x) conversion
    2; i16; -10000; 10000;
    2; i16;      0;     0;
    2; i16;  10000; 10000;
    2; i16;  20000; 20000;
    2; i16;  32767; 32767;
    PARSTOP
;
*/


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_rescaler (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
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

            arm_stream_rescaler_instance *pinstance = (arm_stream_rescaler_instance *) *memresults;
            memresults++;
            break;
        }    




        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
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
            arm_stream_rescaler_instance *pinstance = (arm_stream_rescaler_instance *) instance;
            intPtr_t nb_data, stream_xdmbuffer_size;
            stream_xdmbuffer_t *pt_pt;
            #define SAMP_IN uint8_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;   
            stream_xdmbuffer_size    = pt_pt->size;
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 

            nb_data = stream_xdmbuffer_size / sizeof(SAMP_IN);

            arm_stream_rescaler_process (pinstance, (void *)inBuf, (void *)outBuf, &nb_data);
            
            break;
        }



        /* func(command = STREAM_STOP, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_STOP:  break;    /* func(STREAM_STOP, instance, 0) */
    }
}

#ifdef __cplusplus
}
#endif