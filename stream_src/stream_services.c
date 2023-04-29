/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
 * Description:  
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

/*
        call back for the low layers of the platform
        mode data to the ring buffers , as described in the graph
          or simple assign the base address of the ring buffer to the data (no data move)
        check there is no flow error
  */

void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3)
{   
	switch (RD(command,COMMAND_CMD))
    {
    case 0:
        break;
    /*----------------------------------------------------------------------------
       arm_stream interface is used for "special" services       
       examples : 
       - access to compute libraries, data converters and compression
       - access to time, stdlib, stdio for SWC delivered in binary
       - report information of change in format of the output stream (MPEG decoder)
       - access to platform IOs, data interfaces and associated services
       - report error and metadata

        To avoid to have initialization steps when calling a complex LINK 
        service (rfft, ssrc, ..) the call to STREAM is made with an int32 of value 
        zero at first call. STREAM will detect this value as a request for allocation 
        of memory for this instance, and make the corresponding initializations and 
        return a tag used to address the same instance on the next call. To save memory, 
        it is recommended to free this memory with FREE_INSTANCE.

        For example : int32_t ssrc_instance_id;
        arm_stream_services(SSRC_CONVERT, type, in1, out1, ratio, quality, &(ssrc_instance_id = 0));
            ssrc_intance = index to an internal memory area managed by STREAM
        arm_stream_services(SSRC_CONVERT, type, in2, out2, ratio, quality, &ssrc_instance_id);
        ..
        Terminated by arm_stream_services(FREE_INSTANCE, ssrc_instance_id);  free STREAM internal memory

	case STREAM_SERVICE_XXXX:
	{   
    }   
     *----------------------------------------------------------------------------*/


    /* ----------------------------------------------------------------------------------*/
    case STREAM_NODE_REGISTER: /* called during STREAM_NODE_DECLARATION to register the SWC callback */
        #ifndef _MSC_VER 
            //rtn_addr = __builtin_return_address(0); // check the lr matches with the node 
        #endif
        break; 
    /* ----------------------------------------------------------------------------------
        arm_stream_services(PACK_SERVICE(instance index, STREAM_DEBUG_TRACE), *int8_t, nb bytes, 0);
        arm_stream_services(DEBUG_TRACE_STAMPS, disable_0 / enable_1 time stamps);
     */
    case STREAM_DEBUG_TRACE:
        {   
            uint32_t *pinst;
            uint8_t arcid;
            uint32_t size;
            uint32_t *arc;

            /* extraction of the arc index used for the traces of this Stream instance */
            pinst = (uint32_t *)GRAPH_STREAM_INST();
            pinst = &(pinst[RD(command, INST_SRV) * STREAM_INSTANCE_SIZE]); 
            arcid = RD(pinst[STREAM_INSTANCE_PARAMETERS], TRACE_ARC_PARINST);
            arc = GRAPH_ARC_LIST_ADDR();
            arc = &(arc[arcid * SIZEOF_ARCDESC_W32]);

            /* copy the trace data @@@ no overflow check */
            size = (uint32_t)ptr2;
            arc_data_operations (arc, arc_move_to_arc, ptr1, size);
        }
        break;
    case STREAM_DEBUG_TRACE_STAMPS:
        break;


    default:
        break;
    }
}