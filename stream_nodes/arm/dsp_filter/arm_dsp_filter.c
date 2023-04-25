/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_filter.c
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

#include "arm_dsp_filter.h"


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_dsp_filter (int32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *status)
{
    *status = 1;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, PRESET, TAG, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memory_results are followed by the first two words of STREAM_FORMAT_SIZE_W32 
                memory pointers are in the same order as described in the SWC manifest

                memresult[0] : instance of the component (one pointer and 2 bytes)
                memresult[1] : pointer to the allocated memory (biquad states and coefs)
                memresult[2] : input arc Word 0 SIZSFTRAW_FMT0 
                memresult[3] : input arc Word 1 SAMPINGNCHANM1_FMT1 
                memresult[4] : output arc Word 0 SIZSFTRAW_FMT0 
                memresult[5] : output arc Word 1 SAMPINGNCHANM1_FMT1 

                preset (8bits) : number of biquads in cascade, max = 4, from SWC manifest 
                tag (8bits)  : unused
        */
        case STREAM_RESET: 
        {   stream_entrance *stream_entry = (stream_entrance *)(uint64_t)data;
            intPtr_t *memresults = instance;
            uint8_t *pt8b, i, n;
            arm_filter_instance *pinstance = (arm_filter_instance *) (memresults[0]);

            pinstance->frameSize = RD(memresults[2], FRAMESIZE_FMT0);
            ST(pinstance->format, NUMSTAGE_FLT,  RD(command, PRESET_CMD));
            ST(pinstance->format, RAWDATA_FLT,   RD(memresults[2], RAW_FMT0));
            ST(pinstance->format, NBCHAN_FLT,  1+RD(memresults[3], NCHANM1_FMT1));
            ST(pinstance->format, INTERLEAV_FLT, RD(memresults[2], INTERLEAV_FMT0));
            ST(pinstance->format, TIMESTAMP_FLT, RD(memresults[3], TIMSTAMP_FMT1));
            ST(pinstance->format, NUMSTAGE_FLT,  RD(command, PRESET_CMD));

            /* TCM area (when possible), first field of the instance for MP */
            pinstance->TCM_working = (intPtr_t *)(memresults[1]);   

            /* here reset */
            pt8b = (uint8_t *) &(pinstance->U.biq_q15);
            n = sizeof(arm_filter_biquad_q15);
            for (i = 0; i < n; i++)
            {   pt8b[i] = 0;
            }
            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
       {    uint8_t *pt8bsrc, *pt8bdst, i, n;
            arm_filter_instance *pinstance = (arm_filter_instance *) instance;

            /* copy the parameters */
            pt8bsrc = (uint8_t *) data;
            pinstance->numstages = (uint32_t)(*pt8bsrc);
            pt8bsrc = pt8bsrc+4;

            /* arm_stream_dsp_filter can manage Q15 and float */
            if (RD(pinstance->format, RAWDATA_FLT) == STREAM_FP32)
            {   pt8bdst = (uint8_t *) &(pinstance->U.biq_float);
                n = sizeof(arm_filter_biquad_float);
            }
            else
            {   pt8bdst = (uint8_t *) &(pinstance->U.biq_q15);
                n = sizeof(arm_filter_biquad_q15);
            }


            if (RD(command, TAG_CMD) == ALLPARAM_)
            {   for (i = 0; i < n; i++)
                {   pt8bdst[i] = pt8bsrc[i];
                }
            }
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
            arm_filter_instance *pinstance = (arm_filter_instance *) instance;
            int32_t nb_data, data_buffer_size, bufferout_free;
            data_buffer_t *pt_pt;
            #define SAMP_IN int16_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;   
            data_buffer_size    = pt_pt->size;  /* data amount in the original buffer */
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 
            bufferout_free        = pt_pt->size; /* free area in the original buffer */

            nb_data = data_buffer_size / sizeof(SAMP_IN);

            //processing(pinstance, inBuf, nb_data, outBuf);
            { int i;
                for (i = 0; i < nb_data; i++)
                    outBuf[i] = inBuf[i];
            }

            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_OUT);   /* amount of data produced */
            
            break;
        }



        /* func(command = STREAM_END, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_END:  break;    
    }
}