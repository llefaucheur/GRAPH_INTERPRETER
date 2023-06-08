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

// #ifdef _MSC_VER 
#include "../../CMSIS-Stream/stream_al/platform_windows.h"
#include "../../CMSIS-Stream/stream_src/stream_const.h"      
#include "../../CMSIS-Stream/stream_src/stream_types.h"  
#include "../stream_nodes/arm/gate/arm_stream_detector.h"
// #include "platform_windows.h"
// #include "stream_const.h"      
// #include "stream_types.h"  
// #endif
/* 
    global parameters 
*/

const uint32_t graph_input[] = 
{
    #include "../stream_graph/graph_iir_gate.txt"
    //#include "../stream_graph/graph_imadpcm.txt"
};

arm_global_data_t arm_stream_global;

int main(void)
{
    extern void stream_demo_init(uint8_t stream_instance, 
            uint8_t total_nb_stream_instance,
            const uint32_t *graph_input, 
            uint32_t graph_size,
            uint8_t warm_boot
            );

#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 1
#define STREAM_WARM_BOOT 0

    stream_demo_init
    (   STREAM_CURRENT_INSTANCE, 
        STREAM_NB_INSTANCE, 
        graph_input, 
        sizeof(graph_input), 
        STREAM_WARM_BOOT
    );


    arm_detector_instance instance;
     uint32_t status;
     // Note code is intended for 32 bit but debugging on 64; casts/pointer mismatches must be considered
 
   data_buffer_t *dummy_for_set_param;
    // arm_stream_detector(STREAM_SET_PARAMETER, (uint32_t*)&instance, dummy_for_set_param , &status);

    int STREAM_DETECTOR_PRESET_VAD_48kHz = 1;
    // #define PACK_COMMAND(I,O,T,P,CMD) (((I)<<28)| ((O)<<24)| ((T)<<16)| ((P)<<8)| (CMD))


    //GETTING WEIRD ERROR HERE SAYING PACK_COMMAND NEEDS 6 ARGS

    // #define PACK_COMMAND(I,O,T,P,INST,CMD) (((I)<<28)|((O)<<24)|((T)<<16)|((P)<<8)|((INST)<<4)|(CMD))
    arm_stream_detector(PACK_COMMAND(0,0,0xFE,STREAM_DETECTOR_PRESET_VAD_48kHz,(uint32_t)&instance,STREAM_SET_PARAMETER), (uint32_t*)&instance, dummy_for_set_param , &status);


// #define PACK_COMMAND(I,O,T,P,CMD) (((I)<<28)| ((O)<<24)| ((T)<<16)| ((P)<<8)| (CMD))

    arm_stream
(   STREAM_SET_PARAMETER, 
    (void *)STREAM_CURRENT_INSTANCE,
    (void *)STREAM_SCHD_RET_END_ALL_PARSED, 
    (void *)STREAM_SCHD_NO_SCRIPT
);

    printf("here\n");
    /* run the graph */
	// for (int i = 0; i < 1000000000L; i++)
	for (int i = 0; i < 10; i++)
    {
		arm_stream
        (   STREAM_RUN, 
            (void *)STREAM_CURRENT_INSTANCE,
            (void *)STREAM_SCHD_RET_END_ALL_PARSED, 
            (void *)STREAM_SCHD_NO_SCRIPT
        );
    }  
}
