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

#include "presets.h"
#ifdef PLATFORM_COMPUTER

#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 1

//#define _CRT_SECURE_NO_DEPRECATE 1
#ifdef _MSC_VER 
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if DATA_FROM_FILES
#include <stdio.h>
#define CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <string.h>


#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      
#include "stream_types.h"

extern void arm_stream_io_ack (uint8_t HW_io_idx, void *data, uint32_t size);

/*
 * NULL TASK
 */
void arm_stream_null_task (int32_t c, stream_handle_t i, void *d, uint32_t *s)  {}


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


FILE *ptf_data_in_1;
FILE *ptf_analog_sensor_0;
FILE *ptf_gpio_out_0;
FILE *ptf_data_out_0;


#define size_data_in_1 32
static int16_t buffer_data_in_1[size_data_in_1/2];

#define size_data_out_0 32
static int16_t buffer_data_out_0[size_data_out_0/2];

#define size_gpio_out_0 32
static uint32_t buffer_gpio_out_0[size_gpio_out_0/4];


void data_in_0 (uint32_t command, stream_xdmbuffer_t *data) 
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
void data_in_1 (uint32_t command, stream_xdmbuffer_t *data) 
{   int32_t tmp, stream_format_io_setting, count;

    switch (command)
    {
    case STREAM_RESET:
        if (NULL == (ptf_data_in_1 = fopen("stream_test\\test3.wav", "rb")))
        {   exit (-1); 
        }
        else 
        {   int i, c; 
            for(i=0;i<64;i++) 
            {   fread(&c,1,1,ptf_data_in_1); // skip WAV header
            }
        }
        stream_format_io_setting = *(uint32_t *)(data->address);
        break;
    case STREAM_SET_PARAMETER:
        break;
    case STREAM_SET_BUFFER:
        {   stream_xdmbuffer_t *pt_pt;
            pt_pt = (stream_xdmbuffer_t *)data;
            pt_pt->address = (intptr_t)buffer_data_in_1;
            pt_pt->size = size_data_in_1;
        }
        break;
    case STREAM_RUN:
        count = size_data_in_1/2;
        tmp = fread(buffer_data_in_1, 2, count, ptf_data_in_1);
        if (tmp != count)
        {   arm_stream_io_ack (IO_PLATFORM_DATA_IN_1, buffer_data_in_1, 0);
            fclose (ptf_data_in_1);
        }
        else
        {   arm_stream_io_ack (IO_PLATFORM_DATA_IN_1, buffer_data_in_1, size_data_in_1);
        }
        break;
    case STREAM_STOP:
            fclose (ptf_data_in_1);
        break;
    default:
        break;
    }
}

/*
*  Manifest declaration : io_set0copy1 1
*/
void analog_sensor_0 (uint32_t command, stream_xdmbuffer_t *data) 
{   int32_t tmp, stream_format_io_setting;
#define FORMAT_PRODUCER_FRAME_SIZE 8

    switch (command)
    {
    case STREAM_RESET:
        //if (NULL == (ptf_in_stream_in_0_data = fopen("stream_test\\sine_noise_offset.wav", "rb"))) 
        if (NULL == (ptf_analog_sensor_0 = fopen("stream_test\\chirp_M6dB.wav", "rb")))
        {   exit (-1); 
        }
        else 
        {   int i, c; 
            for(i=0;i<48;i++) 
            { fread(&c,1,1,ptf_analog_sensor_0); // skip WAV header
            }
        }
        stream_format_io_setting = *(uint32_t *)(data->address);          
        break;
    case STREAM_SET_PARAMETER:
        break;
    case STREAM_SET_BUFFER:
        break;
    case STREAM_RUN:
        /* "io_platform_stream_in_0," frame_size option in samples + FORMAT-0 in the example graph */
        tmp = fread(data, 1, FORMAT_PRODUCER_FRAME_SIZE, ptf_analog_sensor_0);

        if (FORMAT_PRODUCER_FRAME_SIZE != tmp)
        {   arm_stream_io_ack (IO_PLATFORM_ANALOG_SENSOR_0, data, 0);
            fclose (ptf_analog_sensor_0);
        }
        else
        {   arm_stream_io_ack (IO_PLATFORM_ANALOG_SENSOR_0, (uint8_t *)data, FORMAT_PRODUCER_FRAME_SIZE);
        }
        break;
    case STREAM_STOP:
            fclose (ptf_analog_sensor_0);
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
        #define FILE_GPIO_OUT_0 "stream_test\\gpio_out_0.raw"
            if (NULL == (ptf_gpio_out_0 = fopen(FILE_GPIO_OUT_0, "wb")))
            {   exit (-1);
            }
        break; 
    case STREAM_SET_BUFFER:
        {   stream_xdmbuffer_t *pt_pt;
            pt_pt = (stream_xdmbuffer_t *)data;
            pt_pt->address = (intptr_t)buffer_gpio_out_0;
            pt_pt->size = size_gpio_out_0;
        }
        break;
    case STREAM_RUN:
         /* "io_platform_stream_in_0," frame_size option in samples + FORMAT-0 in the example graph */ 
         arm_stream_io_ack (IO_PLATFORM_GPIO_OUT_0, (uint8_t *)data, size_gpio_out_0);
         fwrite(data, 1, size_gpio_out_0, ptf_gpio_out_0);
         fflush(ptf_gpio_out_0);
        break;
    case STREAM_STOP:
            fclose (ptf_gpio_out_0);
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}

void gpio_out_1 (uint32_t command, stream_xdmbuffer_t *data) 
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

void data_out_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
    switch (command)
    {
    case STREAM_RESET:
        #define FILE_DATA_OUT_0 "stream_test\\data_out_0.raw"
            if (NULL == (ptf_data_out_0 = fopen(FILE_DATA_OUT_0, "wb")))
            {   exit (-1);
            }
        break;      
    case STREAM_SET_PARAMETER:
        //stream_format_io_setting = *(uint32_t *)(data->address);          
        break;
    case STREAM_SET_BUFFER:
        {   stream_xdmbuffer_t *pt_pt;
            pt_pt = (stream_xdmbuffer_t *)data;
            pt_pt->address = (intptr_t)buffer_data_out_0;
            pt_pt->size = size_data_out_0;
        }
        break;
    case STREAM_RUN:
            arm_stream_io_ack(IO_PLATFORM_DATA_OUT_0, buffer_data_out_0, size_data_out_0);
            fwrite(buffer_data_out_0, 1, size_data_out_0, ptf_data_out_0);
            fflush(ptf_data_out_0);
        break;
    case STREAM_STOP:
            fclose (ptf_data_out_0);
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
