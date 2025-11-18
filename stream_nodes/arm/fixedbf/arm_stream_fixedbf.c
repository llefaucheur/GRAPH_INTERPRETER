/* ----------------------------------------------------------------------
* Project:      CMSIS Stream
 * Title:        arm_stream_fixedbf.c
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

#include "presets.h"
#ifdef CODE_ARM_FIXEDBF

#include <stdint.h>

#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_fixedbf.h"

void arm_stream_fixedbf (uint32_t command, void *instance, void *data, uint32_t *status);


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_fixedbf (uint32_t command, void *instance, void *data, uint32_t *status)
{

    *status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = memory_results and all memory banks following
                data = address of Stream function
                
                memresults are followed by 4 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the NODE manifest

                memresult[0] : instance of the component
                memresult[1] : pointer to the allocated memory (biquad states and coefs)

                memresult[2] : input arc Word 0 SIZSFTRAW_FMT0 (frame size..)
                memresult[ ] : input arc Word 1 SAMPINGNCHANM1_FMT1 
                ..
                memresult[ ] : output arc Word 0 SIZSFTRAW_FMT0 
                memresult[ ] : output arc Word 1 SAMPINGNCHANM1_FMT1 

                preset (8bits) : number of biquads in cascade, max = 4, from NODE manifest 
                tag (8bits)  : unused
        */
        case STREAM_RESET: 
        {   
            uint8_t *pt8b;
            uint8_t i;
            uint8_t n;
            arm_fixedbf_memory **memreq;
            arm_fixedbf_instance *pinstance;
            uint8_t preset;
            uint16_t *pt16dst;

            preset = (uint8_t) RD(command, PRESET_CMD);

            /* read memory banks */
            pinstance = *((arm_fixedbf_instance **) instance);           /* main instance */
            memreq = (arm_fixedbf_memory **) instance;
            memreq = &(memreq[1]);
            pinstance->TCM = (*memreq);       /* second bank = fast memory */

            // DYNAMIC ALLOCATION EXAMPLE, CAN DEPEND ON ARCS FORMAT
            // if (STREAM_DYN_MALLOC == RD(command, COMMDEXT_CMD))
            // {   memreq[0] = sizeof(arm_fixedbf_instance);    // memory allocation segment 0
            //     memreq[1] = 2* sizeof(arm_fixedbf_memory);   // malloc of segment 1 for 2 biquads
            // }

            /* here reset */
            pt8b = (uint8_t *) (pinstance->TCM->state);
            n = sizeof(pinstance->TCM->state);
            for (i = 0; i < n; i++) { pt8b[i] = 0; }

            /* load presets */
            pt16dst = (uint16_t *)(&(pinstance->TCM->coefs[0]));
            switch (preset)
            {   default: 
                case 0:     /* by-pass*/
                    break;
                case 1:     /* LPF fc=fs/4 */
                    break;
                case 2:     /* HPF fc=fs/8 */
                    break;
            }

            pinstance->services = (stream_services *)data;
            break;
        }       

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   uint8_t *pt8bsrc;
            uint8_t i; 
            uint8_t cmsisFormat, rawFormat, numStages, postShift;
            uint16_t *pt16src, *pt16dst;
            arm_fixedbf_instance *pinstance;

            pinstance = (arm_fixedbf_instance *) instance;
            /* copy the parameters 
                parameter_start
                1  u8;  0                               ; CMSIS format
                1  u8;  1                               ; q15 format
                1  u8;  2                               ; numStages
                1  s8;  1                               ; postShift
                5 s16; 681   422   681 23853 -15161     ; INT16 elliptic band-pass 1450..1900/16kHz
                5 s16; 681 -1342   681 26261 -15331     ; 
                parameter_end                
            */
            pt8bsrc = (uint8_t *) data;
            pt16src = (uint16_t *) data;

            cmsisFormat = *pt8bsrc++;
            rawFormat = *pt8bsrc++;
            numStages = *pt8bsrc++;
            postShift = *pt8bsrc++;

            pt16src = &(pt16src[2]);    /* skip the above 4bytes header */
            pt16dst = (uint16_t *)(&(pinstance->TCM->coefs[0]));

            for (i = 0; i < numStages; i++)
            {   /* destination format:  {b10, 0, b11, b12, a11, a12,   b20, 0, b21, b22, a21, a22, ...} */
                *pt16dst++ = *pt16src++;    // b10
                *pt16dst++ = 0;             // 0
                *pt16dst++ = *pt16src++;    // b11    
                *pt16dst++ = *pt16src++;    // b12
                *pt16dst++ = *pt16src++;    // a11
                *pt16dst++ = *pt16src++;    // a12
            }

            /* optimized kernels INIT */
            pinstance->iir_service = PACK_SERVICE(SERV_DSP_INIT,NOOPTION_SSRV,NOTAG_SSRV, SERV_DSP_CASCADE_DF1_Q15,SERV_GROUP_DSP_ML);

            //pinstance->services(                                            // void arm_stream_services (      
            //    pinstance->iir_service,                                     //      uint32_t command, 
            //    (intptr_t)&(pinstance->TCM->biquad_cascade_df1_inst_q15),   //      intptr_t ptr1, 
            //    (intptr_t)&(pinstance->TCM->coefs),                         //      intptr_t ptr2, 
            //    (intptr_t)&(pinstance->TCM->state),                         //      intptr_t ptr3, 
            //    (intptr_t)(numStages << 8u) | postShift                     //      intptr_t n)
            //    );
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
            arm_fixedbf_instance *pinstance;
            intptr_t nb_data;
            intptr_t stream_xdmbuffer_size_in, stream_xdmbuffer_size_out;
            stream_xdmbuffer_t *pt_pt;
            int16_t *inBuf;
            int16_t *outBuf;

            pinstance = (arm_fixedbf_instance *) instance;

            pt_pt = data;   inBuf = (int16_t *)pt_pt->address;   
                            stream_xdmbuffer_size_in = pt_pt->size;  /* data amount in the input buffer */
            pt_pt++;        outBuf = (int16_t *)(pt_pt->address); 
                            stream_xdmbuffer_size_out = pt_pt->size;  /* data free in the input buffer */
                            
            if (stream_xdmbuffer_size_in > stream_xdmbuffer_size_out) {
                nb_data = stream_xdmbuffer_size_out / sizeof(int16_t);
            } else  {
                nb_data = stream_xdmbuffer_size_in / sizeof(int16_t);
            }

            /* optimized kernels RUN */
            pinstance->iir_service = PACK_SERVICE(SERV_DSP_RUN,NOOPTION_SSRV,NOTAG_SSRV,SERV_DSP_CASCADE_DF1_Q15,SERV_GROUP_DSP_ML);

            //pinstance->services(
            //    pinstance->iir_service,
            //    (intptr_t)(&(pinstance->TCM->biquad_cascade_df1_inst_q15)),
            //    (intptr_t)inBuf, 
            //    (intptr_t)outBuf,
            //    (intptr_t)nb_data
            //    );

            pt_pt = data;   *(&(pt_pt->size)) = nb_data * sizeof(int16_t); /* amount of data consumed */
            pt_pt ++;       *(&(pt_pt->size)) = nb_data * sizeof(int16_t); /* amount of data produced */
            
            break;


        }

        case STREAM_READ_PARAMETER:
        case STREAM_UPDATE_RELOCATABLE:
        default : break;
    }

}

#ifdef __cplusplus
}
#endif

#endif // ifdef