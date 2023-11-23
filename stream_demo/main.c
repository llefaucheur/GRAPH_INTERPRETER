/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        main.c
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


#include <string.h> // for memset
#include "platform_al.h"

#include "stream_const.h"      
#include "stream_types.h"  
#include "stream_extern.h"



/* 
    global parameters of this processor = 
        compiled graph data, its address, memory offsets
*/

uint32_t graph_input[] = 
{   
#include "graph_0_bin.txt"
};


/*
    Four registered callback for SWC and scripts (CALS #n)
    Use-case : software timers, event detection trigger, metadata sharing
*/
p_stream_script_callback application_callbacks[MAX_NB_APP_CALLBACKS] =
{   /*  0*/ (void*)0,
    /*  1*/ (void*)0,
    /*  2*/ (void*)0,
    /*  2*/ (void*)0
};


/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int

  @par              initialize a Stream instance : share the address of the graph and
                    and the index of this instance within the application (other processors
                    or RTOS processes are using different index). 
                    Then start an infinite loop for the stream processing.
  @remark
 */

int main(void)
{
#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 1

    arm_stream_instance_t instance;


    memset (&instance, 0, sizeof(instance));
    instance.application_callbacks = application_callbacks;
    instance.scheduler_control = PACK_STREAM_PARAM(
            STREAM_MAIN_INSTANCE,
            STREAM_ANY_PRIORITY,
            STREAM_NB_INSTANCE,
            STREAM_COLD_BOOT,
            STREAM_SCHD_NO_SCRIPT, 
            STREAM_SCHD_RET_END_ALL_PARSED,
            STREAM_CURRENT_INSTANCE
            );

	arm_stream (STREAM_RESET, &instance, graph_input);

    /* run the graph */
	for (int i = 0; i < 1000000000L; i++)
    {   arm_stream (STREAM_RUN, &instance, 0);
    }  

    arm_stream (STREAM_STOP, &instance, 0);
}
