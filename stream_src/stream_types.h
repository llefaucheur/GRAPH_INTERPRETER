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
#include "platform.h"
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"


typedef uint32_t stream_service_command;


/* ------------------------------------------------------------------------------------------
    floating-point emulation
*/

#if STREAM_FLOAT_ALLOWED==1
typedef float sfloat;
typedef double sdouble;

#else
//union floatb {
//    uint32_t i;
//    struct {
//        unsigned m : 23;   /* Mantissa */
//        unsigned e : 8;    /* Exponent */
//        unsigned s : 1;    /* Sign bit */
//    } b;
//};

struct {
    unsigned m : 23;        /* Mantissa */
    unsigned e : 8;         /* Exponent */
    unsigned s : 1;         /* Sign bit */
} sfloat_struct;

//union doubleb {
//    uint64_t i;
//    struct {
//        unsigned ml: 32;   /* Mantissa low */
//        unsigned mh: 20;   /* Mantissa high */
//        unsigned e : 11;   /* Exponent */
//        unsigned s : 1;    /* Sign bit */
//    } b;
//};

struct {
    unsigned ml: 32;        /* Mantissa low */
    unsigned mh: 20;        /* Mantissa high */
    unsigned e : 11;        /* Exponent */
    unsigned s : 1;         /* Sign bit */
} sdouble_struct;

typedef uint32_t sfloat;
typedef uint64_t sdouble;
#endif


/* ------------------------------------------------------------------------------------------
   APIs : nodes, services, IO controls  
  
    parameters : 
      command STREAM_SET_PARAMETER STREAM_RUN STREAM_STOP STREAM_SET_BUFFER
      *instance pointer
      stream_xdmbuffer_t data pointers 
      "state" 
 */

typedef void    (stream_node) (unsigned int command, void *instance, void *data, unsigned int *status);
typedef void (*p_stream_node) (unsigned int command, void *instance, void *data, unsigned int *status);

typedef void    (io_function_ctrl) (unsigned int command, void *data, unsigned int length);   
typedef void (*p_io_function_ctrl) (unsigned int command, void *data, unsigned int length);  


/* ------------------------------------------------------------------------------------------
    Stream instance memory
*/
typedef struct  
{  
    const uint8_t **long_offset;        // pointer to "long_offset[MAX_NB_MEMORY_OFFSET]" 
    uint32_t *graph;                    // base address of the binary graph
    uint8_t *ongoing;                   // pointer to the RAM area to the IOs : on-going transfer flag

    uint32_t *all_formats;   
    uint32_t *all_arcs;
    uint32_t *linked_list;   
    uint32_t *script_offsets;   
    const p_io_function_ctrl *platform_io;
    const p_stream_node *node_entry_points;

    p_stream_al_services application_callbacks;
    p_stream_al_services al_services;

    p_stream_node address_node;
    uint32_t *linked_list_ptr;          // current position of the linked-list read pointer

    uint32_t *node_header;              // current node
    stream_handle_t node_instance_addr;
    uint8_t *pt8b_collision_arc;        // collision
    uint32_t pack_command;              // preset, narc, tag, instanceID, command
    uint32_t iomask;                    // _IOMASK_ fields 

    uint32_t scheduler_control;         // PACK_STREAM_PARAM(..);
    uint32_t whoami_ports;              // _PARCH_ fields 
    uint16_t arcID[MAX_NB_STREAM_PER_NODE];
    uint8_t node_memory_banks_offset;   // offset in words  
    uint8_t node_parameters_offset;     // 
    uint8_t main_script;                // debug script common to all nodes
    uint8_t nb_stream_instances;        // stream instances pointers (in words) = &(all_arcs[ -nb_stream_instances]) 
    uint8_t error_log;                  // bit-field of logged errors 
    uint8_t padding[3]; 

} arm_stream_instance_t;


#ifdef __cplusplus
}
#endif
#endif /* #ifndef cSTREAM_TYPES_H */

