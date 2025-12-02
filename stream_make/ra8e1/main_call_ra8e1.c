/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        main.c
 * Description:  graph interpreter demo
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

#include "presets.h"
#ifdef PLATFORM_RA8E1FPB

#include <string.h>
#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      
#include "stream_types.h"  
#include "stream_extern.h"
#include "../stream_platform/ra8e1/top_manifest_ra8e1.h"

/*
    global variables : all the instances of the graph interpreter
*/
#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 2

arm_stream_instance_t instance[STREAM_NB_INSTANCE];



/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_init(uint32_t *graph)
{ 
    extern void platform_init_stream_instance(arm_stream_instance_t *instance);

    instance[STREAM_CURRENT_INSTANCE].scheduler_control = 
        PACK_STREAM_PARAM(
            STREAM_INSTANCE_ANY_PRIORITY,       // any priority
            STREAM_MAIN_INSTANCE,               // this interpreter instance is the main one (multi-thread)
            STREAM_NB_INSTANCE,                 // total number of instances executing this graph
            COMMDEXT_COLD_BOOT,                 // is it a warm or cold boot
            STREAM_SCHD_NO_SCRIPT,              // debugging scheme used during execution
            STREAM_SCHD_RET_END_ALL_PARSED      // interpreter returns after all nodes are parsed
            );

    /* provision protocol for situation when the graph comes from the application */
    instance[STREAM_CURRENT_INSTANCE].graph = graph;

    /* reset the graph */
    arm_graph_interpreter (STREAM_RESET, &(instance[STREAM_CURRENT_INSTANCE]), 0, 0);
}


/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_set_parameters(void)
{
//    uint32_t new_parameters_arm_stream_router__0[4] = { 1,2,3,4 };
//
//#define arm_stream_router__0       0x26 // node position in the graph
//    arm_graph_interpreter (STREAM_SET_PARAMETER, &(instance[STREAM_CURRENT_INSTANCE]), 
//        arm_stream_router__0, (uintptr_t)new_parameters_arm_stream_router__0);  

/*
    STREAM_SET_PARAMETER from scripts :
    1) main script calls arm_graph_interpreter (STREAM_SET_PARAMETER, IDX, @param)
    2) the script associated to the node calls S->application_callbacks[USE_CASE_CONTROL] 
        then updates its node 
*/
}  


/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_run(void)
{
    /* here test the need for memory recovery/swap 
        does the application modified the memory banks used by the graph ? */
    if (0)
    {   arm_memory_swap(&(instance[STREAM_CURRENT_INSTANCE]));
    }

    arm_graph_interpreter (STREAM_RUN, &(instance[STREAM_CURRENT_INSTANCE]), 0, 0);

    /* here test the need for memory recovery/swap
        does the application intend to modify memory banks used by the graph ? */
    if (0)
    {   arm_memory_swap(&(instance[STREAM_CURRENT_INSTANCE]));
    }

}


/**
  @brief            Example of Master IO pushing new data
  @param[in/out]    none
  @return           int
  @remark
 */

void Push_Ping_Pong(uint32_t *data, uint32_t size)
{
    extern void arm_stream_io_ack (uint8_t graph_io_idx, void *data, uint32_t size);
    arm_stream_io_ack (IO_PLATFORM_AUDIO_IN_0, (uint8_t *)data, size);
}

/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_stop(void)
{
    arm_graph_interpreter (STREAM_STOP, &(instance[STREAM_CURRENT_INSTANCE]), 0, 0);
}
#endif