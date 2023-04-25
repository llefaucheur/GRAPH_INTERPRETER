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
   Main entry point, used by the application and scripts

   Interactions between the graph and its outside world :
   - direct call to arm_stream()
   - platform_io streams to FIFO

   Commands :
   - reset, run, end
   - interpreter of a FIFO of commands

 */
void arm_stream (uint32_t command, void *ptr1, void *ptr2, void *ptr3)
{   
    intPtr_t *parameters;
    uint8_t scheduling;
    uint8_t script;

    parameters = (intPtr_t *)ptr1;
    scheduling = U8(convert_ptr_to_int(ptr2));
    script = U8(convert_ptr_to_int(ptr3));

	switch (command)
    {
	    case STREAM_RESET: 
	    {   
            stream_scan_graph (parameters, 
                scheduling,
                script,
                1);
            break;

        }
	    case STREAM_RUN:   
	    {  
            stream_scan_graph (parameters, 
                scheduling,
                script,
                0);
            break;
        }   

        case STREAM_END:
	    {  
            stream_scan_graph (parameters, 
                scheduling,
                script,
                -1);
            break;
        }

        /* arm_stream (STREAM_INTERPRET_COMMANDS, byte stream, 0, 0)*/
        case STREAM_APP_SET_PARAMETER: 
        {
            /* byte stream format : command + length, byte-stream 
               use-case : 
                node set/read parameters 
                I/O save/restore state, disable/enable, change settings
                ARCs : check content, read/write => "PLATFORM_COMMAND_OUT"
               */
        }



        default:
            break;
    }
}


