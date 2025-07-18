/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_types.h
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

#ifndef cSTREAM_TYPES__H
#define cSTREAM_TYPES__H

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>
//#include "presets.h"
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"


typedef uint32_t stream_service_command;

/* ------------------------------------------------------------------------------------------
   APIs : nodes, services, IO controls  
  
    parameters : 
      command STREAM_SET_PARAMETER STREAM_RUN STREAM_STOP STREAM_SET_BUFFER
      *instance pointer 
      stream_xdmbuffer_t data pointers 
      "state" 
 */

typedef void    (stream_node) (uint32_t command, void *instance, void *data, uint32_t *status);
typedef void (*p_stream_node) (uint32_t command, void *instance, void *data, uint32_t *status);

typedef void    (io_function_ctrl) (uint32_t command, stream_xdmbuffer_t *data);   
typedef void (*p_io_function_ctrl) (uint32_t command, stream_xdmbuffer_t *data);  



/* ------------------------------------------------------------------------------------------
    Stream instance memory
*/
typedef struct  
{  
    uint32_t *graph;                            // 

    /* either in RAM or Flash section */
    uint8_t **long_offset;                      // software MMU
    uint32_t *pio_hw;                           // IO provided by the platform
    uint32_t *pio_graph;                        // IO used in this graph
    uint32_t *script;                           // indexed scripts 
    uint32_t *linked_list;                      // linked-list of nodes
    const p_io_function_ctrl *platform_io;
    const p_stream_node *node_entry_points;
    p_stream_services application_callbacks;
    p_stream_services al_services;

    /* only in RAM section */
    uint8_t *ongoing;                           // bytes of on-going transfer flag of IOs
    uint32_t *all_formats;                      // indexed stream formats (can be changed by the nodes)
    uint32_t *all_arcs;             

    /* working area of the graph interpreter */
    p_stream_node address_node;
    uint32_t *linked_list_ptr;                  // current position of the linked-list read pointer
    uint32_t *node_header;                      // current node
    uintptr_t new_parameters;                   // list of [node idx; parameter address]..[0;0]
    stream_handle_t node_instance_addr;
    uint8_t *pt8b_collision_arc;                // collision
    uint32_t pack_command;                      // preset, narc, tag, instanceID, command
    uint64_t iomask;                            // 64 simultaneous streams per graph instance (see NB_IOS_GR1)

    uint32_t scheduler_control;                 // current PROC/ARCH, 
    uint32_t link_offset;                       // graph read index
    uint16_t arcID[MAX_NB_STREAM_PER_NODE];
    uint16_t idx_node;                          // index of the node to the flash
    uint8_t node_memory_banks_offset;           // offset in words  
    uint8_t node_parameters_offset;             // 
    uint8_t main_script;                        // debug script common to all nodes
    uint8_t nb_graph_io;                        // number of graph IOs
    uint8_t error_log;                          // bit-field of logged errors 

} arm_stream_instance_t;



/* ------------------------------------------------------------------------------------------
    Stream initialization - platform-specific informations
*/
typedef struct  
{  
    uint32_t *graph;                            // the graph to interpret
    uint8_t ** long_offset;                     // pointer to "long_offset[MAX_NB_MEMORY_OFFSET]"
    p_stream_services application_callbacks;    // callbacks used by scripts
    p_stream_node node_entry_points;            // list of nodes
    p_io_function_ctrl platform_io;             // list of IO functions
    uintptr_t *new_parameters;                   // list of [node index, parameter address]..[0;0]
    uint8_t procID;
    uint8_t archID;

} arm_stream_init_t;




#ifdef __cplusplus
}
#endif
#endif /* #ifndef cSTREAM_TYPES_H */

