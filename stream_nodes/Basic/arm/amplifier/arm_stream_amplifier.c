/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_amplifier.c
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
#include "arm_stream_amplifier.h"
#include "platform.h"

/*
;----------------------------------------------------------------------------------------
;4.	arm_stream_amplifier
;----------------------------------------------------------------------------------------
;   Operation : control of the amplitude of the input stream with controlled time of ramp-up/ramp-down. 
;   The gain control “mute” is used to store the current gain setting, being reloaded with the command “unmute”
;   Parameters :  new gain/mute/unmute, ramp-up/down slope, delay before starting the slope.
;
;   parameters of amplifier (variable size): 
;   TAG_CMD = 1, uint8_t, 1st-order IIR shifter coefficient value (0..75k samples)
;   TAG_CMD = 2, uint16_t, desired gain FP_8m4e, 0dB=0x0805
;   TAG_CMD = 3, uint8_t, set/reset mute state
;   TAG_CMD = 4, uint16_t, delay before applying unmute, in samples
;   TAG_CMD = 5, uint16_t, delay before applying mute, in samples
;
node arm_stream_amplifier; 
    3  i8; 0 0 0        instance, no preset, tag=unmute
    parameter_start <optional label for scripts>
    1  i8;  1           rising/falling coefficient slope
    1 h16;  805         gain -100dB .. +36dB (+/- 1%)
    1  i8;  0           muted state
    2 i16;  0 0         delay-up/down
    parameter_end    
*/
#ifndef CODE_ARM_STREAM_AMPLIFIER
void arm_stream_amplifier (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
}
#else

/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_amplifier (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the SWC manifest
        */
        case STREAM_RESET: 
        {   stream_al_services *stream_entry = (stream_al_services *)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            /* here reset */
            arm_amplitude_instance *pinstance = (arm_amplitude_instance *) *memresults;
            pinstance->services = (stream_al_services *)data;

            memresults++;   /* memresult points to the */

            /* copy the frame-size, interleaving scheme and nb of channels of each arc */
            uint32_t tmp32;

            tmp32 = RD(memresults[0], FRAMESIZE_FMT0);
            ST(pinstance->channel_fmt, FRAMESIZE_FMT0_FMT, tmp32);

            tmp32 = RD(memresults[1], INTERLEAV_FMT1);
            ST(pinstance->channel_fmt, INTERLEAVING_FMT, tmp32);

            tmp32 = RD(memresults[1], NCHANM1_FMT1);
            ST(pinstance->channel_fmt, NBCHANM1_AMPLI_FMT, tmp32);
            
            /* reset here */
            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, ALLPARAM_ means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   arm_amplitude_instance *pinstance = (arm_amplitude_instance *) instance;
            uint8_t *pt8bsrc, *pt8bdst, i, n;

            pt8bsrc = (uint8_t *) data;     
            pt8bdst = (uint8_t *) &(pinstance->parameters[0]); 

            switch (RD(command,SWC_TAG_CMD))
            {   default : n = 1; break;
                case TAG_CMD_RAMP :         pt8bdst += 2; n = 1; break;
                case TAG_CMD_DESIRED_GAIN:  pt8bdst += 0; n = 2; break;
                case TAG_CMG_MUTE:          pt8bdst += 3; n = 1; break;
                case TAG_CMD_DELAY_RAMPUP:  pt8bdst += 6; n = 2; break;
                case TAG_CMD_DELAY_RAMPDOWN:pt8bdst += 4; n = 2; break;
                case ALLPARAM_:             n = 8;               break;
            }
           
            for (i = 0; i < n; i++)
            {   pt8bdst[i] = pt8bsrc[i];
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
            arm_amplitude_instance *pinstance = (arm_amplitude_instance *) instance;
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
            arm_stream_amplitude_process(pinstance, inBuf, outBuf, &nb_data);
            
            break;
        }



        /* func(command = STREAM_STOP, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
        */  
        case STREAM_STOP:  break;    
    }
#endif
}

#ifdef __cplusplus
}
#endif  // #ifndef CODE_ARM_STREAM_AMPLIFIER
 