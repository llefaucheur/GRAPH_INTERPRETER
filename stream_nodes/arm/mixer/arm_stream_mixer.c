/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_mixer.c
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
#include "arm_stream_mixer.h"

/*
;----------------------------------------------------------------------------------------
;5.	arm_stream_mixer
;----------------------------------------------------------------------------------------
;   Operation : receives several mono or multichannel streams and produces one output arc. 
;   Mixer manages up to 8 multichannels input arcs, the number is given at STREAM_RESET stage.
;   Parameters : input arcs names, their associated gain and time-constant in [ms], name of the 
;   output arc and an extra mixing gain
;
;   preset : 
;   #0 (default) : mixer-4 with all arcs unmuted with gain = 0dB, slopes of 1000 samples
;   #1 (shut-down) : mixer-4 with all arcs with gain = -96dB, slopes of 100 samples
;   >1 : number of input channels
;
;   parameters of mixer (variable size): 
;   - slopes of rising and falling gains, identical to all channels
;     slope coefficient = 0..15 (iir_coef = 1-1/2^coef) = 0 .. 0.9999
;     Convergence time to 90% of the target in samples:
;      slope   nb of samples to converge
;          0           0
;          1           3
;          2           8
;          3          17
;          4          36
;          5          73
;          6         146
;          7         294
;          8         588
;          9        1178
;         10        2357
;         11        4715
;         12        9430
;         13       18862
;         14       37724
;         15       75450
;         convergence in samples = abs(round(1./abs(log10(1-1./2.^[0:15])')))
;
;   - output mixer gain format FP_8m4e followed by a list of 32bits words
;     FP_8m4em : 8b mantissa, e=4b exponent, (gain * X) = (m * X) >> (e + 2)
;       max = FF0 = 255>>(0+2) = 63.75 = 36dB
;       min = 01F = 1>>(15+2) = 7.6e-6 = -102dB
;       reset = 805 = 128>>(5+2) = 1 = 0dB
;       accuracy = 0.2% 
;
;   format to be packed in 32bits words: 
;           input arc index (4b)        up to 15 arcs
;           sub-channel index -1 (5b)   up to 32 channels per arc of the input arc
;           sub-channel index -1 (5b)   up to 32 channels per arc of the output arc
;           gain (12b)                  format FP_8m4e (0.5% accuracy in -96..+36dB) 0dB = 0x80A
;           muted (1b)                  1 = "muted"
;
node arm_stream_mixer            
;
;   Example: 4 input one mono, one stereo, output is stereo
    3  i8; 0 4 0            instance, number of arcs, preset/tag = 0 arcs unmuted with gain = 0dB
;
    parameter_start <optional label for scripts>
    2  i8; 6 6              rising/falling slope on all arcs
    1 h32; 807              -12dB output gain in FP_8m4e format; 
;    
    4 i16; 0 0 0            arcID0 inputChan0 outputChan0 
    1 h32; 805              0dB gain; mono mixed on the left output

    4 i16; 0 0 1            arcID0 inputChan0 outputChan1 
    1 h32; 805              0dB gain; mono mixed on the right output
                   
    4 i16; 1 0 0            arcID1 inputChan0 outputChan0 
    1 h32; 805              0dB gain; stereo input left to the left

    4 i16; 1 1 1            arcID1 inputChan0 outputChan1 
    1 h32; 803             +12dB gain; stereo input right to the right
    parameter_end   
;
*/

#define NB_PRESET 1
const uint32_t mixer_parameter_presets [NB_PRESET][NB_W32_PARAMETERS] = 
{   
    /* mixer-4 mono to mono output, gain-0dB, muted */
    {   PACKMIXERCONFIG_W0(128,5,4,5),  
        PACKMIXERCHANNELS_W2(128,2,0,0), PACKMIXERCHANNELS_W3(0,0),      
        PACKMIXERCHANNELS_W2(128,2,0,0), PACKMIXERCHANNELS_W3(0,0),      
        PACKMIXERCHANNELS_W2(128,2,0,0), PACKMIXERCHANNELS_W3(0,0),      
        PACKMIXERCHANNELS_W2(128,2,0,0), PACKMIXERCHANNELS_W3(0,0),      
    },
};

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
            uint8_t iarc, nb_input_arc;

            arm_mixer_instance *pinstance = (arm_mixer_instance *) *memresults;
            pinstance->services = (stream_services_entry *)(uint64_t)data;

            memresults++;   /* memresult points to the */

            /* here reset */
            if (preset <= 1)
            {   nb_input_arc = RD(mixer_parameter_presets[preset][0], NBINPUTARC_W0);
            }
            else
            {   nb_input_arc = (uint8_t)preset;
            }

            /* copy the frame-size, interleaving scheme and nb of channels of each arc */
            for (iarc = 0; iarc < nb_input_arc; iarc++)
            {   uint32_t tmp32;

                tmp32 = RD(memresults[0], FRAMESIZE_FMT0);
                ST(pinstance->channel_fmt[iarc], FRAMESIZE_FMT0_FMT, tmp32);

                tmp32 = RD(memresults[0], INTERLEAV_FMT0);
                ST(pinstance->channel_fmt[iarc], INTERLEAVING_FMT, tmp32);

                tmp32 = RD(memresults[0], NCHANM1_FMT1);
                ST(pinstance->channel_fmt[iarc], NBCHANM1_MIXER_FMT, tmp32);
            }
            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   uint8_t *pt8bsrc, *pt8bdst, i, n;
            arm_mixer_instance *pinstance = (arm_mixer_instance *) instance;

            pt8bsrc = (uint8_t *) data;     
            pt8bdst = (uint8_t *) &(pinstance->parameters[0]); 

            switch (RD(command,TAG_CMD))
            {   default : break;
                case TAG_CMD_SLOPE:         pt8bdst += 3; n = 1; break;
                case TAG_CMD_OUTPUT_GAIN:   pt8bdst += 0; n = 2; break;
                case TAG_CMD_NB_INPUT:      pt8bdst += 2; n = 1; break;
                case ALLPARAM_:             n = NB_W32_PARAMETERS*4; break;
            }
           
            for (i = 0; i < n; i++)
            {   pt8bdst[i] = pt8bsrc[i];
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
        {   /* mixing of the 4 input arcs */
            arm_stream_mixer_process((arm_mixer_instance *) instance, data, 4);
            break;
        }
    }
}

#ifdef __cplusplus
}
#endif
    
