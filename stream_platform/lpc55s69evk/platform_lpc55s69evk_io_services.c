/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        platform_computer_io_services.c
 * Description:  abstraction layer to BSP and streams from the application
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

#include "platform.h"
#if PLATFORM_LPC55S69EVK == 1

#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include "stream_const.h"      
#include "stream_types.h"

#include "platform_lpc55s69evk.h"

extern void arm_graph_interpreter_io_ack (uint8_t graph_io_idx, uint8_t *data,  uint32_t size);
extern uint8_t platform_io_al_idx_to_graph[];


/* --------------------------------------------------------------------------------------- 
    FW IO FUNCTIONS

    const p_io_function_ctrl platform_io [LAST_IO_FUNCTION_PLATFORM] =
    {
        (void *)&data_in_0,        // 0
        (void *)&data_in_1,        // 1
        (void *)&analog_sensor_0,  // 2
        (void *)&motion_in_0,      // 3
        (void *)&audio_in_0,       // 4
        (void *)&d2_in_0,          // 5
        (void *)&line_out_0,       // 6
        (void *)&gpio_out_0,       // 7
        (void *)&gpio_out_1,       // 8
        (void *)&data_out_0,       // 9
    };
*/


void data_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}


void data_in_1 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}


void analog_sensor_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    default:
    case STREAM_STOP:
    case STREAM_SET_BUFFER:
    case STREAM_SET_PARAMETER:
    case STREAM_RESET:
        break;

    case STREAM_RUN:
        arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_ANALOG_SENSOR_0], data, size);
        break;
    }
}


void motion_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}
void audio_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}
void d2_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}
void line_out_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}
void gpio_out_0 (uint32_t command, uint8_t *data, uint32_t size) 
{   
#define FORMAT_CONSUMER_FRAME_SIZE 12
    switch (command)
    {
    default:
    case STREAM_SET_PARAMETER:
        break;

    case STREAM_RUN:
        arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_GPIO_OUT_0], data, size);
        break;

    case STREAM_STOP:
    case STREAM_SET_BUFFER:
        break;
    }
}

void gpio_out_1 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}

void data_out_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
    case STREAM_SET_PARAMETER:  /* presets reloaded */
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
    case STREAM_STOP:           /* stop data moves */
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
    default: break;
    }
}

#endif

/*
 * -----------------------------------------------------------------------
 */
