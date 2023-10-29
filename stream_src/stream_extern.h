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

extern p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS];

/* three entry points */
extern void arm_stream (uint32_t command,  arm_stream_instance_t *inst, uint32_t *data);
extern void arm_stream_io (uint32_t fw_io_idx, 
        arm_stream_instance_t *stream_instance,
        uint8_t *data, uint32_t length);
extern void arm_stream_services (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n);

extern void stream_scan_graph (arm_stream_instance_t *stream_instance, int8_t reset_option);

/* ---- REFERENCES --------------------------------------------*/

extern void platform_specific_processor_arch_iomask(uint8_t *procID, uint8_t *archID, uint32_t *ioMask);

extern void script_processing (uint32_t *instance);
extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3);
extern uint32_t lin2pack (arm_stream_instance_t *stream_instance, uint8_t *buffer);
extern void arc_data_operations (arm_stream_instance_t *stream_instance, uint32_t *arc, uint8_t tag, uint8_t *buffer, uint32_t size);


#ifdef __cplusplus
}
#endif
#endif /* #ifndef cSTREAM_TYPES_H */

