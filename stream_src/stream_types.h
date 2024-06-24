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
#include "stream_const.h"
#include "platform.h"


/* ------------------------------------------------------------------------------------------
    stream buffers
*/

struct stream_xdmbuffer
{   
    intPtr_t address; 
    intPtr_t size;
};

typedef struct stream_xdmbuffer stream_xdmbuffer_t;

typedef uint32_t stream_service_command;


/* ------------------------------------------------------------------------------------------
    floating-point emulation
*/

#if STREAM_FLOAT_ALLOWED==1
typedef float sfloat;
typedef double sdouble;

#else
union floatb {
    float f;
    uint32_t i;
    struct {
        unsigned m : 23;   /* Mantissa */
        unsigned e : 8;    /* Exponent */
        unsigned s : 1;    /* Sign bit */
    } b;
};

union doubleb {
    double d;
    uint64_t i;
    struct {
        unsigned ml: 32;   /* Mantissa low */
        unsigned mh: 20;   /* Mantissa high */
        unsigned e : 11;   /* Exponent */
        unsigned s : 1;    /* Sign bit */
    } b;
};
typedef uint32_t sfloat;
typedef uint64_t sdouble;
#endif


/* ------------------------------------------------------------------------------------------
    opaque access to the static area of the node 
*/
typedef void *stream_handle_t;  

/* ------------------------------------------------------------------------------------------
   APIs : nodes, services, IO controls  
  
    parameters : 
      command STREAM_SET_PARAMETER STREAM_RUN STREAM_STOP STREAM_SET_BUFFER
      *instance pointer
      stream_xdmbuffer_t data pointers 
      "state" 
 */

typedef void    (stream_al_services) (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n); 
typedef void (*p_stream_al_services) (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n); 

typedef void    (stream_node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);
typedef void (*p_stream_node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);

typedef void    (io_function_ctrl) (uint32_t command, uint8_t *data, uint32_t length);   
typedef void (*p_io_function_ctrl) (uint32_t command, uint8_t *data, uint32_t length);  


/* ------------------------------------------------------------------------------------------
    Stream instance memory
*/
typedef struct  
{  
    const intPtr_t *long_offset;        // pointer to "intPtr_t long_offset[MAX_NB_MEMORY_OFFSET];" 
    uint32_t *graph;
    uint32_t *pio;
    uint32_t *all_formats;   
    uint32_t *all_arcs;
    uint32_t *linked_list;   
    uint32_t *script_offsets;   
    const p_io_function_ctrl *platform_io;
    const p_stream_node *node_entry_point_table;

    const p_stream_al_services * application_callbacks;
    const p_stream_al_services * al_services;

    p_stream_node address_swc;
    uint32_t *linked_list_ptr;          // current position of the linked-list read pointer

    uint32_t *swc_header;               // current swc
    stream_handle_t swc_instance_addr;
    uint16_t arcID[MAX_NB_STREAM_PER_SWC];
    uint8_t *pt8b_collision_arc;        // collision
    uint32_t pack_command;              // preset, narc, tag, instanceID, command
    uint32_t iomask;                    // _IOMASK_ fields 

    uint32_t scheduler_control;         // PACK_STREAM_PARAM(..);
    uint32_t whoami_ports;              // _PARCH_ fields 
    uint8_t swc_memory_banks_offset;    // offset in words  
    uint8_t swc_parameters_offset;      // 
    uint8_t main_script;                // debug script
    uint8_t nb_stream_instances;        // stream instances pointers (in words) = &(all_arcs[ -nb_stream_instances]) 
    uint8_t memory_segment_swap;        // bit-field of the memory segments to swap (TO_SWAP_LW2S) 
    uint8_t error_log;                  // bit-field of logged errors 

} arm_stream_instance_t;


#ifdef __cplusplus
}
#endif
#endif /* #ifndef cSTREAM_TYPES_H */

