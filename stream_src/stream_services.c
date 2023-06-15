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

#ifdef __cplusplus
 extern "C" {
#endif

#include "stream_const.h" 
#include "stream_types.h"



/**
  @brief        Service entry point for nodes
  @param[in]    command    Bit-field of command (see enum stream_command) 
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  @param[in]    data3      integer data

  @return       none

  @par          Services of DSP/ML computing, access to stdlib, advanced DSP operations (Codec)
                and data stream interface (debug trace, access to additional arcs used for 
                control and metadata reporting the index is given in nodes' parameters).
                arm_stream_services() hold a static memory area, independent for each processor, 
                initialized at Stream instance creation, holding the key pointers to the graph, 
                and to the 

  @remark
 */

void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3)
{   
    static stream_instance_short_t stream_instance_short;

	switch (RD(command,COMMAND_CMD))
    {
        case STREAM_SERVICE_RESET:  /* arm_stream_services(STREAM_SERVICE_RESET, stream_instance_short, 0, 0); */
        {   stream_instance_short = *(stream_instance_short_t *)ptr1;
            break;
        }
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


        /* ----------------------------------------------------------------------------------*/
        case STREAM_NODE_REGISTER: /* called during STREAM_NODE_DECLARATION to register the SWC callback */
            #ifndef _MSC_VER 
                //rtn_addr = __builtin_return_address(0); // check the lr matches with the node 
            #endif
            break; 

         /*----------------------------------------------------------------------------*/

         // SECTIONS OF ARC APIs
         /* 
            read the SWC arc ID from its header in the graph :
                    linked_list = &(linked_list[RD(S->S0.pinst[STREAM_INSTANCE_DYNAMIC], SWC_W32OFF_DINST)]);
         */

        /* ----------------------------------------------------------------------------------
            arm_stream_services(PACK_SERVICE(instance index, STREAM_DEBUG_TRACE), *int8_t, 0, nb bytes);
            arm_stream_services(DEBUG_TRACE_STAMPS, disable_0 / enable_1 time stamps);
         */
        case STREAM_DEBUG_TRACE:
        {   
            uint8_t arcid;
            uint32_t *arc;
            uint32_t free_area;

            /* extraction of the arc index used for the traces of this Stream instance */
            arcid = RD(stream_instance_short.pinst[STREAM_INSTANCE_PARAMETERS], TRACE_ARC_PARINST);
            arc = &(stream_instance_short.all_arcs[arcid * SIZEOF_ARCDESC_W32]);
            free_area = RD(arc[1], BUFF_SIZE_ARCW1) - RD(arc[3], WRITE_ARCW3);
            if (free_area < data3)
            {   platform_al(PLATFORM_ERROR, 0,0,0); /* overflow issue */
                data3 = free_area;
            }

            arc_data_operations ((arm_stream_instance_t *)&stream_instance_short, arc, arc_IO_move_to_arc, ptr1, data3);
            break;
        }

        case STREAM_DEBUG_TRACE_STAMPS:
            break;

        default:
            break;
    }
}
#ifdef __cplusplus
}
#endif
