/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
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

#ifndef cSTREAM_EXTERN__H
#define cSTREAM_EXTERN__H

#ifdef __cplusplus
 extern "C" {
#endif

extern p_stream_node node_entry_points[];

/* entry point from the application  */
extern void arm_graph_interpreter (uint32_t command,  arm_stream_instance_t *S, uintptr_t ptr1, uintptr_t ptr2);

/* entry point from the device drivers */
extern void arm_stream_io_ack (uint8_t io_al_idx, void *data,  uint32_t size);

/* entry point from the computing nodes */
extern void arm_stream_services(uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n);

/* check the need for memory swaps, to let the graph memory being overlaid by the application data */
void arm_memory_swap(arm_stream_instance_t* S);

extern void platform_init_stream_instance(arm_stream_instance_t* S);

extern void pack2lin(uintptr_t* R, uint32_t x, uint8_t** LL);

/* ---- REFERENCES --------------------------------------------*/

extern int32_t stream_bitsize_of_raw(uint8_t raw);

extern void graph_interpreter_process (arm_stream_instance_t *stream_instance, int8_t command, uintptr_t data);

#ifdef __cplusplus
}
#endif
#endif /* #ifndef cSTREAM_TYPES_H */

