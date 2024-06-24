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

#include "stream_const.h"      
#include "stream_types.h"  
#include "stream_extern.h"
#include "graph_0_define.h"


/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int
  @remark
 */
void main_call(void)
{
    extern void platform_init_stream_instance(arm_stream_instance_t *instance);
    arm_stream_instance_t instance;

    /* copy the graph, initializes the interpreter instance */
    platform_init_stream_instance (&instance);

    /* reset the graph */
    arm_graph_interpreter (STREAM_RESET, &instance, 0, 0);

    /* run the graph */
    for (int i = 0; i < 100000L; i++)
    {  arm_graph_interpreter (STREAM_RUN, &instance, 0, 0);
    }  

    /* stop the graph */
    arm_graph_interpreter (STREAM_STOP, &instance, 0, 0);
}
