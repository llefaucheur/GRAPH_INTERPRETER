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


#include <string.h>

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      
#include "stream_types.h"  
#include "stream_extern.h"


#include "platform.h"
#ifdef PLATFORM_COMPUTER

arm_stream_instance_t instance;

/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_init(void)
{
    extern void platform_init_stream_instance(arm_stream_instance_t *instance);

    /* copy the graph, initializes the interpreter instance */
    platform_init_stream_instance (&instance);

    /* reset the graph */
    arm_graph_interpreter (STREAM_RESET, &instance, 0, 0);
}


/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_run(void)
{
    arm_graph_interpreter (STREAM_RUN, &instance, 0, 0);
}  

void Push_Ping_Pong(uint32_t *data, uint32_t size)
{
    extern void arm_graph_interpreter_io_ack (uint8_t graph_io_idx, uint8_t *data,  uint32_t size);
    extern uint8_t platform_io_al_idx_to_graph[];

    arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_ANALOG_SENSOR_0], (uint8_t *)data, size);
}

/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_stop(void)
{
    arm_graph_interpreter (STREAM_STOP, &instance, 0, 0);
}
#endif