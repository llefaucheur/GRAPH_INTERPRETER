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

#include "stream_const.h"
#include "stream_types.h"

/* 
    global parameters 
*/


const uint32_t graph_input[] = 
{
    #include "../stream_graph/graph_iir_gate.txt"
    //#include "../stream_graph/graph_imadpcm.txt"
};



int main(void)
{
    stream_parameters_t parameters;

    extern void stream_demo_init(uint8_t stream_instance, 
            uint8_t total_nb_stream_instance,
            const uint32_t *graph_input, 
            uint32_t graph_size,
            stream_parameters_t *parameters
            );

    stream_demo_init(0, 1, graph_input, sizeof(graph_input), &parameters);

    /* run the graph */
	for (int i = 0; i < 10; i++)
    {
		arm_stream(STREAM_RUN, &parameters,
            (void *)STREAM_SCHD_RET_END_ALL_PARSED, 
            (void *)STREAM_SCHD_NO_SCRIPT);
    }  
}
