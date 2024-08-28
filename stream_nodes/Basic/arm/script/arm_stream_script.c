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

static intPtr_t pack2linaddr_int(uint8_t **long_offset, uint32_t x, uint32_t unit)
{
    const uint8_t *dbg1;
    intPtr_t dbg2;
    const uint8_t *dbg3;
    intPtr_t result;

    dbg1 = long_offset[RD(x,DATAOFF_ARCW0)];    
    dbg2 = (intPtr_t)(unit * (intPtr_t)RD((x),BASEIDX_ARCW0));
    dbg3 = &(dbg1[dbg2]);
    result = (intPtr_t)dbg3;

    return result;  
}

static void * pack2linaddr_ptr(uint8_t **long_offset, uint32_t data, uint32_t unit)
{
    return (void *) (pack2linaddr_int(long_offset, data, unit));
}


/*
    command  = reset/set-param/stop/run
    instance = pointer to the descriptor base address of the arc)
    data     = XDM[0] data to byte codes XDM[1] Stream instance 
    status 
*/
void arm_stream_script (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
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
        {   stream_al_services *stream_entry = (stream_al_services *)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_stream_instance_t *pinstance = (arm_stream_instance_t *) *memresults++;
        }

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, ALLPARAM_ means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   uint32_t *pt32bsrc;
            uint32_t *arc_desc = (uint32_t *) instance;
            pt32bsrc = (uint32_t *) data;
            arc_desc[SCRIPT_UC0_SCRARCW1] = (*pt32bsrc++);       // 2x4-bytes use-case communicated by uper layers
            arc_desc[SCRIPT_UC1_SCRARCW2] = (*pt32bsrc++);       // 
            SET_BIT(arc_desc[SCRIPT_PTR_SCRARCW0], NEW_USE_CASE_SCRIPT_LSB);
        }

        /* byte-code execution,                 
            xdm_data[0].address = (intPtr_t)byte_codes;
            xdm_data[1].address = (intPtr_t)S;
            xdm_data[2].address = (intPtr_t)arc;
        */
        case STREAM_RUN:   
        {   uint32_t clear_size;
            stream_xdmbuffer_t *pt_pt;
            arm_stream_instance_t *S;
            uint32_t *arc_desc = (uint32_t *) instance;
            uint16_t *byte_code;
            uint8_t *src;

            pt_pt = data;   byte_code = (uint16_t *)pt_pt->address;  
            pt_pt++;        S = (arm_stream_instance_t *)pt_pt->address; 
            pt_pt++;        arc_desc = (uint32_t *)pt_pt->address;  

            /* reset the instance */
            clear_size =  (SCRIPT_REGSIZE + 1) * RD(arc_desc[SCRIPT_PTR_SCRARCW0], NBREGS_SCRARCW3);
            clear_size += (SCRIPT_REGSIZE + 1) * RD(arc_desc[WRIOCOLL_SCRARCW3], NSTACK_SCRARCW3);
            src = (uint8_t *)pack2linaddr_ptr(S->long_offset, RD(arc_desc[SCRIPT_PTR_SCRARCW0], BASEIDXOFFSCRARCW0), LINADDR_UNIT_W32);
            MEMSET(src, 0, clear_size);

            arm_stream_script_interpreter (S, arc_desc, byte_code, src);
            break;
        }



        default:
        case STREAM_READ_PARAMETER: 
        case STREAM_UPDATE_RELOCATABLE: 
        case STREAM_STOP: 
        {   break;
        }    
    }
}

#ifdef __cplusplus
}
#endif
    
