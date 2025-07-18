/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_graph_interpreter.c
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


#include <stdint.h>

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"
#include "stream_types.h"
#include "stream_extern.h"
#include <string.h>       

/*
   Main entry point, used by the application s

   Commands :
   - reset, run, end
   - interpreter of a FIFO of commands

 */

/**
  @brief            Graph interpreter entry point 
  @param[in]        command     operation to do (reset, run, stop)
  @param[in]        instance    Graph interpreter instance pointer
  @param[in]        data        graph to process
  @return           none

  @par              reset, run and stop a graph
                      
  @remark
 */
void arm_graph_interpreter (uint32_t command,  arm_stream_instance_t *S, uintptr_t ptr1, uintptr_t ptr2)
{   
	switch (RD(command, COMMAND_CMD))
    {
        /* usage: arm_stream(STREAM_RESET, &instance,graph_input, 0); */
	    case STREAM_RESET: 
	    {   extern void platform_init_stream_instance(arm_stream_instance_t *S);
            platform_init_stream_instance (S);
            stream_scan_graph (S, STREAM_RESET, 0);
            break;
        }


        /* usage: arm_stream(STREAM_RUN, &instance,0, 0); */
	    case STREAM_RUN:   
	    {   stream_scan_graph (S, STREAM_RUN, 0);
            break;
        }   


        /* change the parameters of a node  : 
            usage: 
                1) update the table of node_offset + parameters 
                2) call arm_graph_interpreter (STREAM_SET_PARAMETER, &instance, node offset, 0); 
         */
        case STREAM_SET_PARAMETER:
	    {   stream_scan_graph (S, STREAM_SET_PARAMETER, ptr1);
            break;
        }


        /* usage: arm_graph_interpreter (STREAM_STOP, &instance, 0, 0); */
        case STREAM_STOP:
	    {   stream_scan_graph (S, STREAM_STOP, 0);
            break;
        }

        default:
            break;
    }
}

/*--------------------------------------------------------------------------- */

#ifdef __cplusplus

}

#endif

