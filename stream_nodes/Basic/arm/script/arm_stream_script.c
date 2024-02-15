/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_script.c
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
#include "arm_stream_script.h"

/*
    command  = reset/run + nbreg + stack size
    instance = pointer to the static area (the base address of the arc)
    data     = byte codes
*/
void arm_stream_script (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = memory_results and all memory banks following
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the SWC manifest

                memresult[0] : instance of the component (FIFOTX base address)
                memresult[n+1]:pointer to the allocated memory
                memresult[2] : input arc Word 0 SIZSFTRAW_FMT0 (frame size..)
                memresult[3] : input arc Word 1 SAMPINGNCHANM1_FMT1 
                memresult[4] : output arc Word 0 SIZSFTRAW_FMT0 
                memresult[5] : output arc Word 1 SAMPINGNCHANM1_FMT1 

                preset (8bits) : unused
                tag (8bits)  : unused
        */
        case STREAM_RESET: 
        {   
            intPtr_t *memreq;
            arm_stream_script_instance *pinstance;
            uint32_t clear_size;

            memreq = (intPtr_t *)instance;
            pinstance = (arm_stream_script_instance *) (*memreq);

            /* reset the data after instance : registers_8B[] + type_1B[]*/
            clear_size = (REGSIZE + 1) * RD(pinstance->state, NREGS_SCRIPT) + RD(pinstance->state, NSTACK_SCRIPT);
            MEMSET((uint8_t *) &(pinstance[1]), 0, clear_size);

            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, ALLPARAM_ means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   uint32_t *pt32bsrc;
            arm_stream_script_instance *pinstance = (arm_stream_script_instance *) instance;
            pt32bsrc = (uint32_t *) data;
            pinstance->use_case[0] = (*pt32bsrc++);       // 8-bytes use-case communicated by uper layers
            pinstance->use_case[1] = (*pt32bsrc);
            SET_BIT(pinstance->state, NEW_USE_CASE_SCRIPT_LSB);
        }

        /* byte-code execution, reset the stack pointer, byte_code = data */       
        case STREAM_RUN:   
        {   
            arm_stream_script_instance *pinstance = (arm_stream_script_instance *) instance;
            arm_stream_script_interpreter (pinstance, data);
            break;
        }
        default: break;
    }
}

#ifdef __cplusplus
}
#endif
    
