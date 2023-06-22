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


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  



/* 
    global parameters of this processor = 
        compiled graph data, its address, memory offsets
*/

uint32_t graph_input[] = 
{   
#include "graph_bin_1.h"
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
    instance.scheduler_control = PACK_STREAM_PARAM(
            STREAM_NB_INSTANCE,
            STREAM_WARM_BOOT,
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
