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


#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_script_instructions.h"
#include "arm_stream_script.h"

static intPtr_t pack2linaddr_int(uint8_t **long_offset, uint32_t x, uint8_t extend)
{
    const uint8_t *dbg1;
    intPtr_t dbg2;
    const uint8_t *dbg3;

    dbg1 = long_offset[RD(x,DATAOFF_ARCW0)]; 
    dbg2 = ((intPtr_t)RD((x),BASEIDX_ARCW0)) << (2*extend);
    dbg3 = &(dbg1[dbg2]);
    return (intPtr_t)dbg3;  
}

static void * pack2linaddr_ptr(uint8_t **long_offset, uint32_t data, uint8_t extend)
{
    return (void *) (pack2linaddr_int(long_offset, data, extend));
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
        {   
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_script_instance_t *pinstance = (arm_script_instance_t *) *memresults++;

            /* "data" is the Stream interpreter instance, for access to the services */
            pinstance->services = (stream_al_services *)(intPtr_t)data;
            break;
        }

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   arm_script_instance_t *pinstance = (arm_script_instance_t *) instance;
            pinstance->byte_code = (uint32_t *) data;
            break;
        }

        /* byte-code execution,                 
        xdm_data[0].address = (intPtr_t)S;
        xdm_data[1].address = (intPtr_t)arc;
        xdm_data[2].address = (intPtr_t)buffer;
        */
        case STREAM_RUN:   
        {   
            stream_xdmbuffer_t *pt_pt;
            arm_script_instance_t *pinstance = (arm_script_instance_t *) instance;

            pt_pt = data;   pinstance->S = (arm_script_instance_t *)pt_pt->address;  
            pt_pt++;        pinstance->arc_desc = (uint32_t *)pt_pt->address; 
            pt_pt++;        pinstance->REGS = (regdata_t *)pt_pt->address;  

            /* reset the instance (arc buffer address) */
            pinstance->ctrl.nstack = RD(pinstance->arc_desc[DBGFMT_SCRARCW4], NSTACK_SCRARCW4);
            pinstance->ctrl.nregs = RD(pinstance->arc_desc[DBGFMT_SCRARCW4], NREGS_SCRARCW4);
            pinstance->ctrl.SP = pinstance->ctrl.nregs;     // after { R0 .. R(nregs),R12 }
            pinstance->ctrl.PC = 0;         // PC pre-incremented before read
            pinstance->ctrl.test_flag = 0;
            pinstance->ctrl.cycle_downcounter = MAXCYCLES;

            /*
            *  BYTECODE 
            *         v
            *         XXXXXXXXXXXXXXX
            * 
            *  INSTANCE (arc descriptor address = *script_instance
            *         |   
            *         v                    <--- nStack + 1 ------->
            *         R0 R1 R2 ..  nregs   R13  R14 R15             
            *         <--- registers--->   RegK SP  SP+1
            *  STACK                            [.................]
            *                                   SP init = nregs+2                  
            *                             
            *  HEAP / PARAM (4bytes/words)                                [............]
            */
            arm_stream_script_interpreter (pinstance);
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
    
