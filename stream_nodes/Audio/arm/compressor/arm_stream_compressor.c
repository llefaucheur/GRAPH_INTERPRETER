/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_compressor.c
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



#include "stream_const.h"
#include "stream_types.h"
#include "arm_stream_compressor.h"


#ifdef __cplusplus
 extern "C" {
#endif
/*
;----------------------------------------------------------------------------------------
;9.	    arm_stream_compressor
;----------------------------------------------------------------------------------------
;   Operation : compression 
;   Parameters : coding scheme and a block of 16 parameter bytes for codecs
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
void arm_stream_compressor (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = TASKS_COMPLETED;    /* default return status, unless processing is not finished */

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
        {   stream_al_services *stream_entry = (stream_al_services *)data;
            intPtr_t *memreq = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_stream_compressor_instance *pinstance = (arm_stream_compressor_instance *) (memreq[0]);
            pinstance->TCM = (uint32_t *) (memreq[1]);       /* second bank = fast memory */

            pinstance->output_format[0] = (memreq[6]);
            pinstance->output_format[1] = (memreq[7]);
            pinstance->output_format[2] = (memreq[8]);
            pinstance->output_format[3] = (memreq[9]);

            /* save the address of the "services" */
            pinstance->stream_service_entry = (stream_al_services *)(intPtr_t)data;
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
            arm_stream_compressor_instance *pinstance = (arm_stream_compressor_instance *)instance;
            intPtr_t nb_samp, stream_xdmbuffer_size, nb_bytes, bufferout_free;
            stream_xdmbuffer_t *pt_pt;
            #define SAMP_IN int16_t 
            #define SAMP_OUT uint8_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;   inBuf  = (SAMP_IN *)pt_pt->address;   
                            stream_xdmbuffer_size = pt_pt->size;

            pt_pt++;        outBuf = (SAMP_OUT *)(pt_pt->address); 
                            bufferout_free = pt_pt->size;

            nb_samp = stream_xdmbuffer_size / sizeof(SAMP_IN);


            switch (RD(command,PRESET_CMD))
            {
                case ENCODER_IMADPCM            :
                    nb_bytes = (nb_samp >> 1);   /* two samples generates 1 byte */ 

                    encode_imadpcm(&(pinstance->state[0]), inBuf, nb_samp, outBuf);

                    /*  update only the size field 
                        the NODE is producing an amount of data different from the consumed one (see xdm11 in the manifest) 
                    */
                    pt_pt = data;   *(&(pt_pt->size)) = nb_samp * sizeof(SAMP_IN);      /* amount of data consumed */
                    pt_pt ++;       *(&(pt_pt->size)) = nb_bytes * sizeof(SAMP_OUT);    /* amount of data produced */

                    break;

                default:
                case ENCODER_LPC                :
                case ENCODER_CVSD               :
                case ENCODER_LC3                :
                case ENCODER_SBC                :
                case ENCODER_MSBC               :
                case ENCODER_OPUS_SILK          :
                case ENCODER_MP3                :
                case ENCODER_MPEG4_AACPLUS_V2   :
                case ENCODER_OPUS_CELT          :
                case ENCODER_JPEG               :
                    break;
            }



            
            break;
        }

        default:
        case STREAM_SET_PARAMETER:  
        case STREAM_READ_PARAMETER:  
        case STREAM_STOP:  
            break;       
    }
}


    


#ifdef __cplusplus
}
#endif