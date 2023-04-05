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
  
/* 

                                     +--------------------+
    "parameters"                     |                    |
    memory area for malloc           |       STREAM       |
    pointer to the graph       +---->+                    |
    index of the Stream instance     |                    |
                                     |                    |
                                     |                    |
    graph array = ring buffers +---->+                    |
       boundaries configuration      |                    |
       linked-list of nodes          |                    |
       boot parameters of nodes      |                    |
       memory area of node instances |                    |
       memory of arc                 |                    |
       Stream instances:             |                    |
         offset of arcs to memory    |                    |
         address of the HAL          |                    |
         address of the nodes' code  |                    |
                                     +--------------------+

    global data in ROM : entry points of SWC, IO manifest
*/

#include "stream_const.h"
#include "stream_types.h"

/* 
    global parameters 
*/


uint32_t graph_input[] = 
{
    #include "../stream_graph/graph_iir_gate.txt"
    //#include "../stream_graph/graph_imadpcm.txt"
};


/*-------------------PLATFORM MANIFEST-----------------------
                  +-----------------+
                  | static |working |
   +--------------------------------+
   |external RAM  |        |        |
   +--------------------------------+
   |internal RAM  |        |        |
   +--------------------------------+
   |fast TCM      |  N/A   |        |
   +--------------+--------+--------+
*/
#define SIZE_MBANK_DMEM_EXT   0x20000  /* external */
#define SIZE_MBANK_DMEM       0x8000    /* internal */
#define SIZE_MBANK_DMEMFAST   0x4000    /* TCM */
#define SIZE_MBANK_BACKUP     0x10      /* BACKUP */
#define SIZE_MBANK_HWIODMEM   0x1000    /* DMA buffer */
#define SIZE_MBANK_PMEM       0x100     /* patch */

uint32_t MEXT[SIZE_MBANK_DMEM_EXT];
uint32_t RAM1[SIZE_MBANK_DMEM];
uint32_t RAM2[SIZE_MBANK_DMEM];
uint32_t RAM3[SIZE_MBANK_DMEM];
uint32_t RAM4[SIZE_MBANK_DMEM];
uint32_t TCM1[SIZE_MBANK_DMEMFAST]; 
uint32_t TCM2[SIZE_MBANK_DMEMFAST]; 
uint32_t BKUP[SIZE_MBANK_BACKUP]; 
uint32_t HWIO[SIZE_MBANK_HWIODMEM];
uint32_t PMEM[SIZE_MBANK_PMEM];


int main(void)
{
    stream_parameters_t parameters;

    extern void stream_demo_init(uint8_t stream_instance, 
            uint8_t total_nb_stream_instance,
            uint32_t *graph_input, 
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
