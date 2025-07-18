/* ----------------------------------------------------------------------
 * Title:        sigp_stream_decompressor.c
 * Description:  filters
 *
 * $Date:        15 February 2024
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2013-2024 signal-processing.fr. All rights reserved.
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

#include "presets.h"
#ifdef CODE_SIGP_STREAM_DECOMPRESSOR

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>
#include "presets.h"                    // compilation options to save code size 
#include "stream_common_const.h"        // 
#include "stream_common_types.h"        // types for the interface to the scheduler

#include "sigp_stream_decompressor.h"



/*
;----------------------------------------------------------------------------------------
;10.    sigp_stream_decompressor
;----------------------------------------------------------------------------------------
;   Operation : decompression of encoded data
;   Parameters : coding scheme and a block of 16 parameter bytes for codecs
;
;       WARNING : if the output format can change (mono/stereo, sampling-rate, ..)
;           the variation is detected by the node and reported to the scheduler with 
;           "STREAM_SERVICE_INTERNAL_FORMAT_UPDATE", the "uint32_t *all_formats" must be 
;           mapped in a RAM for dynamic updates with "COPY_CONF_GRAPH0_COPY_ALL_IN_RAM"
;
;       Example of data to share with the application
;           outputFormat: AndroidOutputFormat.MPEG_4,
;           audioEncoder: AndroidAudioEncoder.AAC,
;           sampleRate: 44100,
;           numberOfChannels: 2,
;           bitRate: 128000,
;
;   presets provision
;   #1 : decoder IMADPCM
;   #2 : decoder LPC
;   #3 : MIDI player / tone sequencer
;   #4 : decoder CVSD for BT speech 
;   #5 : decoder LC3 
;   #6 : decoder SBC
;   #7 : decoder mSBC
;   #7 : decoder OPUS Silk
;   #8 : decoder MP3
;   #9 : decoder MPEG-4 aacPlus v2 
;   #10: decoder OPUS CELT
;   #11: decoder JPEG 
*/

/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void sigp_stream_decompressor (unsigned int command, void *instance, void *data, unsigned int *status)
{
    *status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresult[0] : instance of the component
                memresult[1] : pointer to the second allocated memory 
                memresult[2] : input arc WORD 0 
                memresult[3] : input arc WORD 1 
                memresult[4] : input arc WORD 2 
                memresult[5] : input arc WORD 3  
                memresult[6] : output arc WORD 0 - frame size
                memresult[7] : output arc WORD 1 - time-stamp, raw format, interleaving, nchan
                memresult[8] : output arc WORD 2  is domain-dependent : sampling rate
                memresult[9] : output arc WORD 3  is domain-dependent : audio mapping  
        */
        case STREAM_RESET: 
        {   //stream_services *stream_entry = (stream_services *)data;
            intptr_t *memreq = (intptr_t *)instance;
            //uint16_t preset = RD(command, PRESET_CMD);

            sigp_stream_decompressor_instance *pinstance = (sigp_stream_decompressor_instance *) (memreq[0]);
            pinstance->TCM = (uint32_t *) (memreq[1]);       /* second bank = fast memory */

            pinstance->output_format[0] = (memreq[6]);
            pinstance->output_format[1] = (memreq[7]);
            pinstance->output_format[2] = (memreq[8]);
            pinstance->output_format[3] = (memreq[9]);

            /* save the address of the "services" */
            pinstance->stream_service_entry = (stream_services *)(intptr_t)data;

            /* clear memory */
            MEMSET(&(pinstance->memory_state[0]), 0, sizeof(pinstance->memory_state));

            pinstance->decoder_state = STATE_RUN;

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
            sigp_stream_decompressor_instance *pinstance = (sigp_stream_decompressor_instance *) instance;
            intptr_t nb_data, nb_samp, stream_xdmbuffer_size, bufferout_free;
            stream_xdmbuffer_t *pt_pt;
            #define SAMP_IN uint8_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;   inBuf  = (SAMP_IN *)pt_pt->address;   
                            stream_xdmbuffer_size = pt_pt->size;

            pt_pt++;        outBuf = (SAMP_OUT *)(pt_pt->address); 
                            bufferout_free = pt_pt->size;

            nb_data = stream_xdmbuffer_size / sizeof(SAMP_IN);

            switch (RD(command,PRESET_CMD))
            {
                case DECODER_IMADPCM            :
                    nb_samp = (nb_data << 1);   /* one byte generates 2 samples */ 
                    decode_imadpcm((int32_t *)&(pinstance->memory_state[0]), inBuf, nb_samp, outBuf, pinstance->decoder_state);

                    /*  update only the size field 
                        the NODE is producing an amount of data different from the consumed one (see xdm11 in the manifest) 
                    */

                    if (pinstance->decoder_state == STATE_PAUSE)
                    {   nb_data = 0; /* no data was consumed during the PAUSE state */
                    }

                    pt_pt = data;   *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN);      /* amount of data consumed */
                    pt_pt ++;       *(&(pt_pt->size)) = nb_samp * sizeof(SAMP_OUT);     /* amount of data produced */
                    break;

                default:
                case DECODER_LPC                :
                case DECODER_MIDI               :
                case DECODER_CVSD               :
                case DECODER_LC3                :
                case DECODER_SBC                :
                case DECODER_MSBC               :
                case DECODER_OPUS_SILK          :
                case DECODER_MP3                :
                case DECODER_MPEG4_AACPLUS_V2   :
                case DECODER_OPUS_CELT          :
                case DECODER_JPEG               :
                    break;
            }
            
            break;
        }

        default:
            break;
        case STREAM_SET_PARAMETER:  
        {   //sigp_stream_decompressor_instance *pinstance = (sigp_stream_decompressor_instance *) instance;
            //pinstance->decoder_state = STATE_PAUSE;
            //pinstance->decoder_state = STATE_FAST_FORWARD2;
            //pinstance->decoder_state = STATE_FAST_FORWARD4;
            break;
        }
        case STREAM_READ_PARAMETER:  
        case STREAM_STOP:  
            break;    
    }
}

#ifdef __cplusplus
}
#endif


#else
void sigp_stream_decompressor (unsigned int command, void *instance, void *data, unsigned int *status) { /* fake access */ if(command || instance || data || status) return;}
#endif // CODE_SIGP_STREAM_DECOMPRESSOR    
