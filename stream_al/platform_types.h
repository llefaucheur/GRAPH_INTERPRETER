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
#ifndef PLATFORM_IO_H
#define PLATFORM_IO_H

extern uint16_t available_architectures[STREAM_NB_ARCHITECTURES];
extern uint8_t proc_state[STREAM_NB_PROCESSES];



extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3);

/*
 * ----------- data access --------------------------------------------------------------
 */


extern uint32_t * platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];

extern uint32_t audio_render_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_render_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_render_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);

extern uint32_t audio_ap_rx_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_ap_rx_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_ap_rx_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);

#endif /* #ifndef PLATFORM_IO_H */
