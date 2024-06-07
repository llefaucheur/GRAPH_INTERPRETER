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


#include <stdint.h>


#include "stream_const.h"
#include "stream_types.h"
#include "stream_extern.h"
#include <string.h>         /* memcpy */
SECTION_START 


extern void platform_init_copy_graph(arm_stream_instance_t *stream_instance);
extern void platform_init_io(arm_stream_instance_t *stream_instance);


/*
   Main entry point, used by the application and scripts

   Interactions between the graph and its outside world :
   - direct call to arm_stream()
   - platform_io streams to FIFO

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
void arm_graph_interpreter (uint32_t command,  arm_stream_instance_t *S, uint8_t *data, uint32_t size)
{   
	switch (RD(command, COMMAND_CMD))
    {
        /* usage: arm_stream(STREAM_RESET, &instance,graph_input, 0); */
	    case STREAM_RESET: 
	    {   platform_init_copy_graph (S);
            /* @@@ TODO  check scheduler_control S->scheduler_control .BOOT_SCTRL to clear 
                the backup memory area STREAM_COLD_BOOT / STREAM_WARM_BOOT
             */
            /* if the application sets the debug option then don't use the one from the graph
                and the application sets the scheduler return option in platform_init_stream_instance() 
             */
            if (STREAM_SCHD_NO_SCRIPT == RD(S->scheduler_control, SCRIPT_SCTRL))
            {   uint32_t debug_script_option;
                debug_script_option = RD((S->graph)[3], SCRIPT_SCTRL_GR3);
                ST(S->scheduler_control, SCRIPT_SCTRL, debug_script_option);
            }

            /* does the graph is deciding the return-from-scheduler option */
            if (STREAM_SCHD_RET_NO_ACTION == RD(S->scheduler_control, RETURN_SCTRL))
            {   uint32_t return_script_option;
                return_script_option = RD((S->graph)[3], RETURN_SCTRL_GR3);

                /* the return option is undefined : return when no more data is available */
                if (return_script_option == 0) 
                {   return_script_option = STREAM_SCHD_RET_END_SWC_NODATA;
                }
                ST(S->scheduler_control, RETURN_SCTRL, return_script_option);
            }

            S->linked_list_ptr = S->linked_list;

            ST(S->scheduler_control, INSTANCE_SCTRL, 1); /* this instance is active */ 

            arm_stream_services(STREAM_SERVICE_INTERNAL_RESET, (uint8_t *)&(S), 0, 0, 0); 

            stream_scan_graph (S, STREAM_RESET, 0);

            //if (RD(S->scheduler_control, MAININST_SCTRL)) 
            //{   /* all other process can be released from wait state */
            //    platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); 

            platform_init_io (S);

            //} else
            //{   /* wait until the graph is copied in RAM */
            //    uint8_t wait; 
            //    do {
            //        platform_al (PLATFORM_MP_BOOT_WAIT, &wait, 0,0);
            //    } while (0u != wait);
            //}
    

            break;
        }

        /* usage: arm_stream(STREAM_RUN, &instance,0, 0); */
	    case STREAM_RUN:   
	    {   if (RD(S->scheduler_control, INSTANCE_SCTRL))
            {   stream_scan_graph (S, STREAM_RUN, 0);
            }
            break;
        }   

        /* change the use-case  : 
            usage: arm_graph_interpreter (STREAM_SET_PARAMETER, &instance, &use-case, 0); */
        case STREAM_SET_PARAMETER:
	    {   /* @@@ calls the main script with SET_PARAMETER */
            /* update parameter of a specific nanoAppRT 
                usage: arm_graph_interpreter (STREAM_SET_NANOAPPRT_PARAMETER, &instance, &parameters, 0); */
	        /* @@@ calls specific nanoAppRT with SET_PARAMETER */
            stream_scan_graph (S, STREAM_SET_PARAMETER, (uint32_t *)data);
            break;
        }


        /* usage: arm_graph_interpreter (STREAM_STOP, &instance, 0, 0); */
        case STREAM_STOP:
	    {   stream_scan_graph (S, STREAM_STOP, 0);
            break;
        }

        /* manual data push/pop : read information (available free space, or data amount to read)
            arm_graph_interpreter(STREAM_FIFO_STATUS_PACK(io_platform_data_in_0), instance, *fifo size, 0) 
            */
        case STREAM_FIFO_STATUS: 
        {   extern void check_graph_boundaries(arm_stream_instance_t *S);
            //check_graph_boundaries(S);
            //*(uint32_t *)data = RD(S->ioctrl[STREAM_DATA_START_UNPACK_FIFO(command)], ONGOING_IOSIZE);
            break;
        }

        /* usage: arm_graph_interpreter(STREAM_DATA_START_PACK(FIFO_ID), instance, *data, size) */
        case STREAM_DATA_START: 
        {   
            //// @@@@@
            //*(uint32_t *)data = RD(S->ioctrl[STREAM_DATA_START_UNPACK_FIFO(command)], ONGOING_IOSIZE);
            break;
        }

        default:
            break;
    }
}

/*--------------------------------------------------------------------------- */

SECTION_STOP 
#ifdef __cplusplus

}

#endif

