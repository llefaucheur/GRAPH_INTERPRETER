/* ----------------------------------------------------------------------
 

        WORK ON GOING



* Project:      CMSIS Stream
 * Title:        arm_stream_filter.c
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

////@@@@@@
//#define _CRT_SECURE_NO_DEPRECATE 1
//#include <stdio.h>
//FILE *ptfdbg;

#include "stream_const.h"
#include "stream_types.h"
#include "dsp\filtering_functions.h"
#include "arm_stream_filter.h"

/*
;----------------------------------------------------------------------------------------
;6.	arm_stream_filter
;----------------------------------------------------------------------------------------
;   Operation : receives one multichannel stream and produces one filtered multichannel stream. 
;   Filtering is identical on each channel. GUI parameters are given in float, implementation 
;   is CMSIS-DSP through Stream services, when compiled with STREAM_SERVICE_EXTDSPML=1
;   Parameters : biquad filters coefficients used in cascade. Implementation is 2 Biquads max.
;   (see www.w3.org/TR/audio-eq-cookbook)
;
;   presets:
;   #1 : LPF fc = 1/4
;   #11..42: delay-line of 1..32 samples
;
;   parameter of filter : 
;   - number of biquads in cascade (1 or 2)
;   - coefficients in Q15
;
;
node arm_stream_dsp_filter
    3  i8; 0 0 0        instance, preset, tag
    parameter_start
    1  i8; 2          Two biquads : [b0b1b2 a1a2 shift]
    1  i8; 2          postShift = 2, explaination below
    5 i16; 4657,  7464,  4657, 25576, -11147, 
    5 i16; 4657,  2868,  4657,  9328, -26591,
    parameter_end

    PRESET #1 
    Half-band low-pass filter :
    [b,a] = ellip(4, 1, 40, 3600/8000, 'low')  MATLAB
    b = [ 0.0808134359 0.1792766 0.241380 0.179276 0.080813435960181 ]
    a = [ 1 -1.06520401 1.3738840 -0.730252 0.27605 ]

    decomposed/split in 2 Biquads (b0 b1 b2 a1 a2) :
    2.842770e-01,  4.555821e-01,  2.842770e-01,  7.805347e-01, -3.401758e-01 
    2.842770e-01,  1.750586e-01,  2.842770e-01,  2.846693e-01, -8.115139e-01 

    Translation in Q15 without looking at saturations in the recursive paths:
    9315, 14928,  9315, 25576, -11147, 
    9315,  5736,  9315,  9328, -26591, 

    Translation to q15 fixed-point arithmetics using a 32bits accumulator (16x16=>32bits)
    First stage :
        postShift = 1 to compensate for a 6dB attenuation in b0/b1/b2 
            added because of the 6dB gain in the recursive path (a1 a2 are untouched)
    Second stage :
        6dB attenuation in b0/b1/b2 
            because of 16dB gain in the recursive path at frequencies but the first 
            stage is attenuating by ~4dB and we kept -6dB from the first stage 
            => 16dB -4 - 6 = 6dB attenuation on the numerator

    Translation in Q15 + postShift to manage saturation:
     4657,  7464,  4657, 25576, -11147,  (pre shifted by 1 on the numerator)
     4657,  2868,  4657,  9328, -26591,  (additional pre shift by 1 on the numerator)
     postShift = 2       to compensate the attenuations put on the two stages
*/


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_filter (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = SWC_TASK_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, PRESET, TAG, NB ARCS IN/OUT)
                instance = memory_results and all memory banks following
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the SWC manifest

                memresult[0] : instance of the component (one pointer and 2 bytes)
                memresult[1] : pointer to the allocated memory (biquad states and coefs)
                memresult[2] : input arc Word 0 SIZSFTRAW_FMT0 (frame size..)
                memresult[3] : input arc Word 1 SAMPINGNCHANM1_FMT1 
                memresult[4] : output arc Word 0 SIZSFTRAW_FMT0 
                memresult[5] : output arc Word 1 SAMPINGNCHANM1_FMT1 

                preset (8bits) : number of biquads in cascade, max = 4, from SWC manifest 
                tag (8bits)  : unused
        */
        case STREAM_RESET: 
        {   
            uint8_t *pt8b, i, n;
            intPtr_t *memreq;
            arm_filter_instance *pinstance;

            memreq = (intPtr_t *)instance;
            pinstance = (arm_filter_instance *) (*memreq++);

            /* here reset */
            pt8b = (uint8_t *) (pinstance->static_mem.state);
            n = sizeof(pinstance->static_mem.state);
            for (i = 0; i < n; i++) { pt8b[i] = 0; }

            pinstance->services = (stream_services_entry *)(uint64_t)data;

//ptfdbg=fopen("..\\..\\..\\stream_test\\DBGIIR.raw","wb");
            break;
        }       

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {    uint8_t *pt8bsrc, i, numStages;
            uint16_t *pt16src, *pt16dst;
            int8_t postShift;
            arm_filter_instance *pinstance = (arm_filter_instance *) instance;

            /* copy the parameters 
                parameter_start
                1  i8;  2                           Two biquads
                1  i8;  0                           postShift
                5 h16; 5678 2E5B 71DD 2166 70B0     b0/b1/b2/a1/a2 
                5 h16; 5678 2E5B 71DD 2166 70B0     second biquad
                parameter_end                
            */
            pt8bsrc = (uint8_t *) data;
            numStages = (*pt8bsrc++);
            postShift = (*pt8bsrc++);

            pt16src = (uint16_t *)pt8bsrc;
            pt16dst = (uint16_t *)(&(pinstance->static_mem.coefs[0]));
            for (i = 0; i < numStages; i++)
            {   /* format:  {b10, 0, b11, b12, a11, a12, b20, 0, b21, b22, a21, a22, ...} */
                *pt16dst++ = *pt16src++;    // b10
                *pt16dst++ = 0;             // 0
                *pt16dst++ = *pt16src++;    // b11    
                *pt16dst++ = *pt16src++;    // b12
                *pt16dst++ = *pt16src++;    // a11
                *pt16dst++ = *pt16src++;    // a12
            }

            arm_biquad_cascade_df1_init_q15(
                &(pinstance->static_mem.biquad_casd_df1_inst_q15),
                numStages,
                (const q15_t *)&(pinstance->static_mem.coefs[0]),
                (q15_t *)&(pinstance->static_mem.state),
                postShift);

           /* optimized kernels */
            pinstance->iir_service = PACK_SERVICE(0,0,STREAM_SERVICE_CASCADE_DF1_Q15,STREAM_SERVICE_DSP_ML);

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
            intPtr_t nb_data, stream_xdmbuffer_size;
            stream_xdmbuffer_t *pt_pt;
            int16_t *inBuf, *outBuf;

            pt_pt = data;   inBuf = (int16_t *)pt_pt->address;   
                            stream_xdmbuffer_size = pt_pt->size;  /* data amount in the input buffer */

            pt_pt++;        outBuf = (int16_t *)(pt_pt->address); 

            nb_data = stream_xdmbuffer_size / sizeof(int16_t);

            pinstance->services(
                pinstance->iir_service,
                (uint8_t*)inBuf, 
                (uint8_t*)outBuf,
                (uint8_t*)(&(pinstance->static_mem.biquad_casd_df1_inst_q15)),
                (uint32_t)nb_data
                );
            break;
        }

        default : break;
    }
}
#ifdef __cplusplus
}
#endif
    
#if 0
        case STREAM_RESET: 
        {   stream_entrance *stream_entry = (stream_entrance *)(uint64_t)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint8_t *pt8b, i, n;
            arm_filter_instance *pinstance = (arm_filter_instance *) (memresults[0]);

            pinstance->frameSize = RD(memresults[2], FRAMESIZE_FMT0);
            ST(pinstance->format, NUMSTAGE_FLT,  RD(command, PRESET_CMD));
            ST(pinstance->format, RAWDATA_FLT,   RD(memresults[2], RAW_FMT0));
            ST(pinstance->format, NBCHAN_FLT,  1+RD(memresults[3], NCHANM1_FMT1));
            ST(pinstance->format, INTERLEAV_FLT, RD(memresults[2], INTERLEAV_FMT0));
            ST(pinstance->format, TIMESTAMP_FLT, RD(memresults[3], TIMSTAMP_FMT1));

            /* TCM area (when possible), first field of the instance for MP */
            pinstance->TCM_working = (intPtr_t *)(memresults[1]);

            /* here reset */
            pt8b = (uint8_t *) &(pinstance->U.biq_q15);
            n = sizeof(arm_filter_biquad_q15);
            for (i = 0; i < n; i++)
            {   pt8b[i] = 0;
            }
//arm_stream_services(PACK_SERVICE(RD(command,INST_CMD), STREAM_SERVICE_INTERNAL_DEBUG_TRACE), "FLTR", 0, (uint8_t *)4);

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

            /* arm_stream_filter can manage Q15 and float */
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
//arm_stream_services(PACK_SERVICE(RD(command,INST_CMD), STREAM_SERVICE_INTERNAL_DEBUG_TRACE), "FLTS", 0, (uint8_t *)4);
            break;
        }
#endif