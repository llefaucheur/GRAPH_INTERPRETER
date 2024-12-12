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
#ifdef  PLATFORM_MSPM0L1306

#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      
#include "stream_types.h"

#include "platform_lp-mspm0l1306.h"

extern void arm_graph_interpreter_io_ack (uint8_t graph_io_idx, void *data,  uint32_t size);
extern uint8_t platform_io_al_idx_to_graph[];

/*
 * NULL TASK
 */
void arm_stream_null_task (int32_t c, stream_handle_t i, void *d, uint32_t *s)  {}


/* --------------------------------------------------------------------------------------- 
    FW IO FUNCTIONS

    const p_io_function_ctrl platform_io [LAST_IO_FUNCTION_PLATFORM] =
    {
        (void *)&data_in_0,        // 0 <= ADC connexion
        (void *)&data_in_1,        // 1
        (void *)&analog_sensor_0,  // 2
        (void *)&motion_in_0,      // 3
        (void *)&audio_in_0,       // 4
        (void *)&d2_in_0,          // 5
        (void *)&line_out_0,       // 6
        (void *)&gpio_out_0,       // 7
        (void *)&gpio_out_1,       // 8 => LED 
        (void *)&data_out_0,       // 9
    };
*/

#define size_gpio_out_1 2
uint16_t buffer_gpio_out_1[size_gpio_out_1 / 2 /* sizeof(uint16_t) */];

void data_in_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
extern uint16_t gADCSamplesPing[64];
#define SIZEOF_ADC_BUFFER 128   // ADC_SAMPLE_SIZE * sizeof(uint16_t);
    
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        break;        
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        {   stream_xdmbuffer_t *pt_pt;
            pt_pt = (stream_xdmbuffer_t *)data;
            pt_pt->address = (intPtr_t)(&(gADCSamplesPing[0]));
            pt_pt->size = SIZEOF_ADC_BUFFER;
        }    
        break;
    case STREAM_RUN:            /* data moves */
        {   stream_xdmbuffer_t *pt_pt;
            pt_pt = (stream_xdmbuffer_t *)data;
            arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_DATA_IN_0], (void *)(pt_pt->address), pt_pt->size);
        }
        break;
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}

/*
*  Manifest declaration : io_set0copy1 0
*/
void data_in_1 (uint32_t command, stream_xdmbuffer_t *data) 
{   
    switch (command)
    {
    case STREAM_RESET:
        break;
    case STREAM_SET_PARAMETER:
        break;
    case STREAM_SET_BUFFER:
        break;
    case STREAM_RUN:
        break;
    case STREAM_STOP:
        break;
    default:
        break;
    }
}

/*
*  Manifest declaration : io_set0copy1 1
*   This is a RX servant port : there is no need to return a buffer address (STREAM_SET_BUFFER)
*       the servant driver will provide it when calling for ACK of data transfers
*/
void analog_sensor_0 (uint32_t command, stream_xdmbuffer_t *data) 
{   
    switch (command)
    {
    case STREAM_RESET:
        break;
    case STREAM_SET_PARAMETER:
        break;
    case STREAM_SET_BUFFER:
        break;
    case STREAM_RUN:
        break;
    case STREAM_STOP:
        break;
    default:
        break;        
    }
}


void motion_in_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        break;        
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;       
    }
}
void audio_in_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        break;        
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;      
    }
}
void d2_in_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        break;        
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;        
    }
}
void line_out_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        break;        
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;      
    }
}
/*
*  Manifest declaration : io_set0copy1 0
*/
void gpio_out_0 (uint32_t command, stream_xdmbuffer_t *data) 
{   
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);          
        break;    
    case STREAM_SET_PARAMETER:
        break; 
    case STREAM_SET_BUFFER:
        break;
    case STREAM_RUN:
        //arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_GPIO_OUT_0], (uint8_t *)data, size_gpio_out_0);
        break;
    case STREAM_STOP:
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}

/*      #include "ti_msp_dl_config.h"
*       SYSCFG_DL_init();
*     
*       DL_GPIO_setPins  (GPIOA, GPIO_ALERT_A1_PIN);  
*       DL_GPIO_clearPins(GPIOA, GPIO_ALERT_A1_PIN);  
*       DL_GPIO_togglePins(GPIO_LEDS_PORT, DL_GPIO_PIN_0);
*       DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
* 
*       DL_GPIO_clearPins(RGB_PORT, (RGB_RED_PIN | RGB_GREEN_PIN | RGB_BLUE_PIN));
*       DL_GPIO_clearPins(RGB_PORT, (RGB_GREEN_PIN | RGB_BLUE_PIN));  DL_GPIO_setPins(RGB_PORT, RGB_RED_PIN);
*       DL_GPIO_clearPins(RGB_PORT, (RGB_RED_PIN   | RGB_BLUE_PIN));  DL_GPIO_setPins(RGB_PORT, RGB_BLUE_PIN);
*       DL_GPIO_clearPins(RGB_PORT, (RGB_RED_PIN   | RGB_BLUE_PIN));  DL_GPIO_setPins(RGB_PORT, RGB_GREEN_PIN);
* 
        if (DL_GPIO_readPins(INPUT_PORT, INPUT_CHANNEL_0_PIN)){ DL_GPIO_setPins  (OUTPUT_PORT, OUTPUT_LED_PIN);
        } else {                                                DL_GPIO_clearPins(OUTPUT_PORT, OUTPUT_LED_PIN);
*/
void gpio_out_1 (uint32_t command, stream_xdmbuffer_t *data) 
{   
    extern void platform_gpio_out_1(uint8_t bit);
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        break;        
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        data = (stream_xdmbuffer_t *)data;
        data->address = (intPtr_t)buffer_gpio_out_1;
        data->size = size_gpio_out_1;
        break;
    case STREAM_RUN:            /* data moves */
        {   uint16_t x;
            x = (*(uint16_t *)(data->address));

            if (0 == x) platform_gpio_out_1(0);
        else platform_gpio_out_1(1);

            arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_GPIO_OUT_1], buffer_gpio_out_1, 2);
        break;
        }
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}

void data_out_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);          
        break;      
    case STREAM_SET_PARAMETER:
        //stream_format_io_setting = *(uint32_t *)(data->address);          
        break;
    case STREAM_SET_BUFFER:
        break;
    case STREAM_RUN:
        //arm_graph_interpreter_io_ack(platform_io_al_idx_to_graph[IO_PLATFORM_DATA_OUT_0], buffer_data_out_0, size_data_out_0);
        break;
    case STREAM_STOP:
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}

#endif

/*
 * -----------------------------------------------------------------------
 */
