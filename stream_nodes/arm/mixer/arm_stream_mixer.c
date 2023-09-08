/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_mixer_process.c
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
#include "arm_stream_mixer.h"


//;----------------------------------------------------------------------------------------
//;5.	arm_stream_mixer (Q15)
//;----------------------------------------------------------------------------------------
//;   Operation : receives several mono or multichannel streams and produces one output arc. 
//;   Mixer manages up to 8 multichannels input arcs, the number is given at STREAM_RESET stage.
//;   One input arc can be tagged as “High QoS” and managed with priority : 
//;     whatever the data content on the other arcs, data is flowing synchronized by this one. 
//;     The other arcs are using interpolation scheme. 
//;   Parameters : input arcs names, their associated gain and time-constant in [ms], name of the 
//;   output arc and an extra mixing gain, name of the High QoS arc
//;
//;   presets : 3bits LSB
//;   #0 : all arcs unmuted with gain = 0dB
//;   #1 : mixing controlled with the table of parameters (below)
//;   #2 : all arcs unmuted with previous gains
//;   #3 : all arcs muted
//;   default slope is 16 samples.
//;
//;   parameters of mixer (variable size): 
//;   - slopes of rising gains (identical to all channels)
//;   - slopes of falling gains
//;   - index of the input arc with HQoS
//;   - output mixer gain (format Q6.12 (mute, -72dB .. +36dB)) followed by a list of 32bits words
//;
//;   format to be packed in 32bits words: 
//;           input arc index (4b)        up to 16 arcs
//;           sub-channel index -1 (5b)   up to 32 channels per arc of the input arc
//;           sub-channel index -1 (5b)   up to 32 channels per arc of the output arc
//;           gain (17b)                  format Q6.11 (-66dB .. +36dB) 0x00800 = 0dB
//;           muted (1b)                  1 = "muted"
//;
//node arm_stream_mixer            
//;
//;   Example: 4 input one mono, one stereo, output is stereo
//    3  i8; 0 4 0            instance, number of arcs, preset/tag = 0 arcs unmuted with gain = 0dB
//    parameter_start
//    2  i8; 6 6              rising/falling slope on all arcs
//    5  i8; 15               indexes of input/output HQoS (none=15)
//    1 h32; 200              -12dB output gain in H15.16 format; 
//;    
//    4 i16; 0 0 0            arcID0 inputChan0 outputChan0
//    1 h32; 800              0dB gain; mono mixed on the left output
//    4 i16; 0 0 1            arcID0 inputChan0 outputChan1
//    1 h32; 800              0dB gain; mono mixed on the right output
//    4 i16; 1 0 0            arcID1 inputChan0 outputChan0
//    1 h32; 10000            0dB gain; stereo input left to the left
//    4 i16; 1 1 1            arcID1 inputChan0 outputChan1
//    1 h32; 2000             +12dB gain; stereo input right to the right
//    parameter_end   



/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_mixer (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
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

            arm_mixer_instance *pinstance = (arm_mixer_instance *) *memresults;
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
            arm_mixer_instance *pinstance = (arm_mixer_instance *) instance;
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

            arm_stream_mixer_process(pinstance, inBuf, outBuf, &nb_data);
            
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
    
