/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_control.c
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

/*
    command  = reset/set-param/stop/run
    instance = pointer to the descriptor base address of the arc)
    data     = XDM[0] data to byte codes XDM[1] Stream instance 
    status 
*/
void arm_stream_graph_control (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = memory_results and all memory banks
                data = address of Stream function
                No arc
                memresult[0] : instance of the component (FIFOTX descbase address)
                    base address = instance, registers
                    length = code length + byte code format
                    read index = start of stack index
                    write index = start of parameters index + synchronization byte
        */
        case STREAM_RESET: 
        {   
            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, ALLPARAM_ means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  

        /* byte-code execution,                 
            xdm_data[0].address = (intPtr_t)byte_codes;
            xdm_data[1].address = (intPtr_t)S;
            xdm_data[2].address = (intPtr_t)arc;
        */
        case STREAM_RUN:   

        default: break;
    }
}

#ifdef __cplusplus
}
#endif
    
