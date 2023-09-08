/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_amplitude.c
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
   

#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  

#include "arm_stream_amplifier.h"

/*
;----------------------------------------------------------------------------------------
;4.	arm_stream_amplifier
;----------------------------------------------------------------------------------------
;   Operation : control of the amplitude of the input stream with controlled time of ramp-up and ramp-down. 
;   The gain control “mute” is used to store the current gain setting, being reloaded when the command “unmute” is sent
;   Parameters :  new gain/mute/unmute, ramp-up/down slope, delay before starting the slope.
;
;   Tags used for individual parameter-setting :
;   TAG_CMD = 0 to unmute (default state at reset)
;   TAG_CMD = 1 to mute
;   TAG_CMD = 2 to change the gain (linear Q15.16 format, 0x00010000 (0dB) at reset)
;   TAG_CMD = 3 delay before applying the ramp-up, in samples (uint16, reset value=0)
;   TAG_CMD = 4 delay before applying the ramp-down, in samples (uint16)
;   TAG_CMD = 5 ramp-up slope (uint8, 1st-order IIR shifter coefficient value, tbd)
;   TAG_CMD = 6 ramp-down slope (uint8, reset value= 0 sample slope)
;
;   Reset default state is 0dB amplification, 0 delays
;
arm_stream_amplifier; 
    3  i8; 0 0 0        instance, no preset, tag=unmute
    PARSTART
    1  i8;  0           muted state
    1 h32;  10000       gain
    2 i16;  0 0         delay-up/down
    2  i8;  4 4         rising/falling slope
    PARSTOP 

*/

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

            arm_amplitude_instance *pinstance = (arm_amplitude_instance *)  *memresults;
            memresults++;
            /* here reset */

            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   arm_amplitude_instance *pinstance = (arm_amplitude_instance *) instance;
            uint8_t *pt8bsrc, *pt8bdst, i, n;
            
            pt8bsrc = (uint8_t *) data;     
            #define pII(n) (uint8_t *)&(pinstance->I[n])
            switch (RD(command,TAG_CMD))
            // uint16_t, Q16,   TAG_CMD = 1, ramp-up slope, 1st-order IIR shifter coefficient value
            // uint16_t, Q16,   TAG_CMD = 2, ramp-down slope, 1st-order IIR shifter coefficient value
            // uint16_t, U16,   TAG_CMD = 3 delay before applying the ramp-up, in samples
            // uint16_t, U16,   TAG_CMD = 4 delay before applying the ramp-down, in samples
            // uint32_t, Q15.16,TAG_CMD = 5 current gain (linear Q15.16 format, 0x00010000 (0dB) at reset)
            // uint32_t, Q15.16,TAG_CMD = 6 desired gain in ramp-up/down process
            // uint8_t, mute,   TAG_CMD = 7 to unmute (default state at reset)
            //                  TAG_CMG = 8 to mute (state=1)
            // Reset default state is 0dB amplification, 0 delays

            {   default :
                case TAG_CMD_RAMPUP:        pt8bdst = pII(0); n = 2;  break;
                case TAG_CMD_RAMPDOWN:      pt8bdst = pII(0); n = 2; pt8bdst = &(pt8bdst[2]); break;
                case TAG_CMD_DELAY_RAMPUP:  pt8bdst = pII(1); n = 2;  break;
                case TAG_CMD_DELAY_RAMPDOWN:pt8bdst = pII(1); n = 2; pt8bdst = &(pt8bdst[2]); break;
                case TAG_CMD_CURRENT_GAIN:  pt8bdst = pII(2); n = 4; break;
                case TAG_CMD_DESIRED_GAIN:  pt8bdst = pII(3); n = 4; break;
                case TAG_CMD_UNMUTE:        pt8bdst = pII(4); n = 1; break;
                case TAG_CMG_MUTE:          pt8bdst = pII(4); n = 1; break;
                case ALLPARAM_:             pt8bdst = pII(0); n = 17;break;
            }
           
            for (i = 0; i < n; i++)
            {   pt8bdst[i] = pt8bsrc[i];
            }
        }

        /* func(command = STREAM_READ_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG/index of a parameter to read (Metadata, Needle), 0xFF means "read all the parameters"
                *instance, 
                data = *parameter(s) to read
        */ 
        case STREAM_READ_PARAMETER:  
        {   arm_amplitude_instance *pinstance = (arm_amplitude_instance *) instance;
            uint8_t *pt8bsrc, *pt8bdst, i, n;
            
            pt8bdst = (uint8_t *) data;     

            switch (RD(command,TAG_CMD))
            {   default:
                case TAG_CMD_RAMPUP:        pt8bsrc = pII(0); n = 2; break;
                case TAG_CMD_RAMPDOWN:      pt8bsrc = pII(0); n = 2; pt8bsrc = &(pt8bsrc[2]); break;
                case TAG_CMD_DELAY_RAMPUP:  pt8bsrc = pII(1); n = 2; break;
                case TAG_CMD_DELAY_RAMPDOWN:pt8bsrc = pII(1); n = 2; pt8bsrc = &(pt8bsrc[2]); break;
                case TAG_CMD_CURRENT_GAIN:  pt8bsrc = pII(2); n = 4; break;
                case TAG_CMD_DESIRED_GAIN:  pt8bsrc = pII(3); n = 4; break;
                case TAG_CMD_UNMUTE:        pt8bsrc = pII(4); n = 1; break;
                case TAG_CMG_MUTE:          pt8bsrc = pII(4); n = 1; break;
                case ALLPARAM_:             pt8bsrc = pII(0); n = 17;break;
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
           used to free memory allocated with the C standard library
        */  
        case STREAM_STOP:  break;    
    }
}

#ifdef __cplusplus
}
#endif
 