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
#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 1

#define _CRT_SECURE_NO_DEPRECATE 1
#if DATA_FROM_FILES
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "stream_const.h"      
#include "stream_types.h"

#include "platform_computer.h"

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




#if DATA_FROM_FILES
    FILE *ptf_trace;
    FILE *ptf_in_stream_in_0_data;
    FILE *ptf_in_gpio_out_data;
#else
    const int16_t ptf_in_stream_in_0_data[] = { 
        #include "..//stream_test//sine_noise_offset.txt"
    };
    uint16_t ptf_in_gpio_out_data[sizeof(ptf_in_stream_in_0_data)/sizeof(int16_t)];
    uint16_t ptf_trace[6*sizeof(ptf_in_stream_in_0_data)/sizeof(int16_t)];

    uint32_t ptr_in_stream_in_0_data;
    uint32_t ptr_in_gpio_out_data;
    uint32_t ptr_trace;
#endif


/* the IO manifest declares graphalloc_X_bsp_0 = 0 : the buffers are declared in BSP */
#define graphalloc_X_bsp_0 0
static int16_t tx_buffer[16];
static int16_t rx_buffer[16];


void data_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    default:
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    }
}


void data_in_1 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    default:
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    }
}


void analog_sensor_0 (uint32_t command, uint8_t *data, uint32_t size) 
{   int32_t tmp, stream_format_io_setting;
    uint32_t cumulated_data = 0;
    int16_t *data16 = rx_buffer;
    switch (command)
    {
    default:
    case STREAM_SET_PARAMETER:
    case STREAM_RESET:
        #if DATA_FROM_FILES
            if (NULL == (ptf_in_stream_in_0_data = fopen("..\\stream_test\\sine_noise_offset.wav", "rb")))
            {   exit (-1); 
            }
            else 
            {   int i, c; 
                for(i=0;i<512;i++) 
                { fread(&c,1,1,ptf_in_stream_in_0_data); // skip WAV header
                }
            }
        #else
            ptr_in_stream_in_0_data = 0;
        #endif
        stream_format_io_setting = *data;            
        break;

    case STREAM_RUN:
         /* "io_platform_stream_in_0," frame_size option in samples + FORMAT-0 in the example graph */ 
        #define FORMAT_PRODUCER_FRAME_SIZE 8
            size  = (FORMAT_PRODUCER_FRAME_SIZE/2);

            //for (j = i = 0; i < size; i++) 
            //{   j = fscanf(ptf_in_stream_in_0_data, "%d,", &tmp); 
            //    data16[i] = (int16_t)tmp; 
            //    cumulated_data += sizeof(int16_t);
            //}
            //if (j > 0) tmp = size; else tmp = 0;
        #if DATA_FROM_FILES
            tmp = fread(data16, 2, size, ptf_in_stream_in_0_data);

            //for (i = 0; i < size; i++) 
            //{   data16[i] = (int16_t)0x1000; 
            //}
        #else
            memcpy(data16, &(ptf_in_stream_in_0_data[ptr_in_stream_in_0_data]), 2 * size);
            ptr_in_stream_in_0_data += size;
            if (ptr_in_stream_in_0_data*2 >= sizeof(ptf_in_stream_in_0_data))
                tmp = size+1;
            else tmp = size;
        #endif
            cumulated_data = 2 * size;

            if (size != tmp)
            {   arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_ANALOG_SENSOR_0], data, 0);
        #if DATA_FROM_FILES
                fclose (ptf_in_stream_in_0_data);
        #endif
            }
            else
            {   arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_ANALOG_SENSOR_0], (uint8_t *)data16, cumulated_data);
            }
        break;

    case STREAM_STOP:
        #if DATA_FROM_FILES
            fclose (ptf_in_stream_in_0_data);
        #endif
        break;

    case STREAM_SET_BUFFER:
        #if graphalloc_X_bsp_0 != 0
        rx_buffer = (int16_t *)data;
        #endif
        break;
    }
}


void motion_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    default:
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    }
}
void audio_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    default:
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    }
}
void d2_in_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    default:
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    }
}
void line_out_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    default:
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    }
}
void gpio_out_0 (uint32_t command, uint8_t *data, uint32_t size) 
{   
#define FORMAT_CONSUMER_FRAME_SIZE 12
    switch (command)
    {
    default:
    case STREAM_SET_PARAMETER:
        #if DATA_FROM_FILES
        #define FILE_OUT "..\\stream_test\\audio_out.raw"
            if (NULL == (ptf_in_gpio_out_data = fopen(FILE_OUT, "wb")))
            {   exit (-1);
            }
        #else
            ptr_in_gpio_out_data = 0;
        #endif
        break;

    case STREAM_RUN:
         /* "io_platform_stream_in_0," frame_size option in samples + FORMAT-0 in the example graph */ 
        #define FORMAT_CONSUMER_FRAME_SIZE 12
            size  = (FORMAT_CONSUMER_FRAME_SIZE/2);
        #if DATA_FROM_FILES
            fwrite(tx_buffer, 2, size, ptf_in_gpio_out_data);
            fflush(ptf_in_gpio_out_data);
        #else
            memcpy(&(ptf_in_gpio_out_data[ptr_in_gpio_out_data]), tx_buffer, 2 * size);
            ptr_in_gpio_out_data += size;
            {int i = sizeof(ptf_in_stream_in_0_data);
                 i = sizeof(ptf_in_gpio_out_data);
                 i=0;
            }
        #endif
            arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_GPIO_OUT_0], (uint8_t *)tx_buffer, FORMAT_CONSUMER_FRAME_SIZE);

        break;

    case STREAM_STOP:
        #if DATA_FROM_FILES
            fclose (ptf_in_gpio_out_data);
        #endif
        break;

    case STREAM_SET_BUFFER:
        #if graphalloc_X_bsp_0 != 0
        tx_buffer = (int16_t *)data; 
        #endif
        memset(tx_buffer, 0, FORMAT_CONSUMER_FRAME_SIZE);

        break;
    }
}

void gpio_out_1 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    case STREAM_RUN:            /* data moves */
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    default:
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    }
}

void data_out_0 (uint32_t command, uint8_t *data, uint32_t size) 
{
    switch (command)
    {
    default:
    case STREAM_SET_PARAMETER:
        #if DATA_FROM_FILES
        #define FILE_TRACE "..\\stream_test\\trace.raw"
            if (NULL == (ptf_trace = fopen(FILE_TRACE, "wb")))
            {   exit (-1);
            }
        #else
            ptr_trace = 0;
        #endif
        break;
    case STREAM_RUN:
        #if DATA_FROM_FILES
            //fprintf(ptf_trace, "%s\n", data);
            fwrite(data, 1, size, ptf_trace);
            fflush(ptf_trace);
            arm_graph_interpreter_io_ack(platform_io_al_idx_to_graph[IO_PLATFORM_DATA_OUT_0], data,size);
        #else
            ptr_trace = 0;  // no trace 
        #endif
        break;
    case STREAM_STOP:
        #if DATA_FROM_FILES
            fclose (ptf_trace);
        #endif
        break;

    case STREAM_SET_BUFFER:
        break;
    }
}


/*
 * -----------------------------------------------------------------------
 */
