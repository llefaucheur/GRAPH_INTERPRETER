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
#ifdef PLATFORM_ALIF1

#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 1

//#define _CRT_SECURE_NO_DEPRECATE 1
#ifdef _MSC_VER 
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)
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

#include "top_manifest_alif1.h"


extern void arm_stream_io_ack (uint8_t HW_io_idx, void *data, uint32_t size);

/* Local IO functions */
extern void data_sink      (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 0   io_alif1_data_in_0.txt   */
extern void data_in_1      (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 1   io_alif1_data_in_1.txt   */
extern void analog_in_0    (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 2   io_alif1_analog_in_0.txt */
extern void motion_in_0    (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 3   io_alif1_motion_in_0.txt */
extern void audio_in_0     (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 4   io_alif1_audio_in_0.txt  */
extern void sensor_2d_in_0 (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 5   io_alif1_2d_in_0.txt  */
extern void line_out_0     (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 6   io_alif1_line_out_0.txt  */
extern void gpio_out_0     (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 7   io_alif1_gpio_out_0.txt  */
extern void gpio_out_1     (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 8   io_alif1_gpio_out_1.txt  */
extern void data_out_0     (uint32_t command, stream_xdmbuffer_t* data);        /*  IO_AL_idx = 9   io_alif1_data_out_0.txt  */


/*
 * NULL TASK
 */
void arm_stream_null_task (int32_t c, stream_handle_t i, void *d, uint32_t *s)  {}


/* --------------------------------------------------------------------------------------- 
    FW IO FUNCTIONS

    1 io_alif1_analog_0.txt        2    size_sensor_0 16
    1 io_alif1_audio_in_0.txt      4    two microphones    320
    1 io_alif1_line_out_0.txt      6    audio out stereo   160
    1 io_alif1_gpio_out_0.txt      7    detection results  4
    1 io_alif1_gpio_out_1.txt      8    LED/display
    1 io_alif1_data_out_0.txt      9    debug trace        1024

    #define IO_PLATFORM_DATA_IN_0        0
    #define IO_PLATFORM_DATA_IN_1        1
    #define IO_PLATFORM_SENSOR_0  2
    #define IO_PLATFORM_MOTION_IN_0      3
    #define IO_PLATFORM_AUDIO_IN_0       4
    #define IO_PLATFORM_2D_IN_0          5
    #define IO_PLATFORM_LINE_OUT_0       6
    #define IO_PLATFORM_GPIO_OUT_0       7
    #define IO_PLATFORM_GPIO_OUT_1       8
    #define IO_PLATFORM_DATA_OUT_0       9
*/

#ifdef _MSC_VER 
FILE* ptf_analog_0;
FILE* ptf_audio_in_0;
FILE* ptf_debug_detector;
FILE* ptf_line_out_0;
FILE* ptf_gpio_out_0;
FILE* ptf_gpio_out_1;
FILE* ptf_data_out_0;
#endif

/*  IO_AL_idx =  0 data_in_0 */
/*  IO_AL_idx =  1 data_in_1 */

/*  IO_AL_idx =  2 analog_in_0 */
#define size_analog_0 16 /* Bytes */
static int16_t buffer_analog_0[size_analog_0 / sizeof(int16_t)];

/*  IO_AL_idx =  3 motion_in_0 */

/*  IO_AL_idx =  4 audio_in_0  buffer declared in the application */
#define size_audio_in_0 640 
    #ifdef _MSC_VER 
        #define audio_buf_size  1280    /* 1280Bytes = 640 stereo samples ping-pong buffer */
        #define audio_block_size 640    /* 640Bytes = 320 stereo samples = 10 ms processing */
        uint8_t stereoBuffer[audio_buf_size];
    #endif

/*  IO_AL_idx =  5 sensor_2d_in_0 */

/*  IO_AL_idx =  6 line_out_0 */
#define size_line_out_0 320
static int16_t buffer_line_out_0[size_line_out_0 / sizeof(int16_t)];

/*  IO_AL_idx =  7  gpio_out_0 */
#define size_gpio_out_0 4                      
static uint32_t buffer_gpio_out_0[size_gpio_out_0];

/*  IO_AL_idx =  8  gpio_out_1  UI/LED  */
#define size_gpio_out_1 4
static int32_t buffer_gpio_out_1[size_gpio_out_1 / sizeof(int32_t)];

/*  IO_AL_idx =  9  data_out_0  debug traces  */
#define size_data_out_0 4                      
static uint32_t buffer_data_out_0[size_data_out_0];    




/*  IO_AL_idx = 0   */
void data_sink(uint32_t command, stream_xdmbuffer_t* data)
{   switch (command)
    {   case STREAM_RESET:          
        case STREAM_SET_PARAMETER:  
        case STREAM_SET_BUFFER:     
        case STREAM_RUN:            
        case STREAM_STOP:           
        case STREAM_READ_PARAMETER: 
        default:break;
    }
}

/*  IO_AL_idx = 1   */
void data_in_1(uint32_t command, stream_xdmbuffer_t* data)
{   switch (command)
    {   case STREAM_RESET:          
        case STREAM_SET_PARAMETER:  
        case STREAM_SET_BUFFER:     
        case STREAM_RUN:            
        case STREAM_STOP:           
        case STREAM_READ_PARAMETER: 
        default:break;
    }
}


/*  IO_AL_idx = 2       1 io_alif1_analog_0.txt 2    size_sensor_0 16 */
void analog_in_0(uint32_t command, stream_xdmbuffer_t* data)
{   switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        #ifdef _MSC_VER 
                #define FILE_ANALOG_0 "..\\stream_test\\analog_0.wav"
                if (NULL == (ptf_analog_0 = fopen(FILE_ANALOG_0, "rb")))
                {   exit(666);  }
        #endif
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    case STREAM_RUN:            /* data moves */
        #ifdef _MSC_VER 
            {   int tmp;
                tmp = (int32_t)fread(buffer_analog_0, 1, size_analog_0, ptf_analog_0);
                if (tmp != size_analog_0)
                {   fclose(ptf_analog_0); exit(-3);   }
            }
        #endif
        arm_stream_io_ack(IO_PLATFORM_SENSOR_0, buffer_analog_0, size_analog_0);
        break;
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}



/*  IO_AL_idx = 3   */
void motion_in_0(uint32_t command, stream_xdmbuffer_t* data)
{   switch (command)
    {   case STREAM_RESET:          
        case STREAM_SET_PARAMETER:  
        case STREAM_SET_BUFFER:     
        case STREAM_RUN:            
        case STREAM_STOP:           
        case STREAM_READ_PARAMETER: 
        default:break;
    }
}


/*  IO_AL_idx = 4       1 io_alif1_audio_in_0.txt      4    two microphones    320  */
void audio_in_0(uint32_t command, stream_xdmbuffer_t* data)
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        #ifdef _MSC_VER 
            #define FILE_AUDIO_IN_0 "..\\stream_test\\audio_in_0.raw"
            //#define FILE_AUDIO_IN_0 "..\\stream_test\\test3.wav"
            if (NULL == (ptf_audio_in_0 = fopen(FILE_AUDIO_IN_0, "rb")))
            {   exit(-1); }
            if (NULL == (ptf_debug_detector = fopen("..\\stream_test\\debug_detector_6x32.raw", "wb")))
            {   exit(-1); }
            
        #endif
        break;
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    case STREAM_RUN:            /* data moves */
        #ifdef _MSC_VER 
            {   int tmp;
                tmp = (int32_t)fread(stereoBuffer, 1, size_audio_in_0, ptf_audio_in_0);
                if (tmp != size_audio_in_0)
                {   arm_stream_io_ack(IO_PLATFORM_AUDIO_IN_0, stereoBuffer, 0);
                    fclose(ptf_audio_in_0);
                    exit(-3);
                }
            }
            arm_stream_io_ack(IO_PLATFORM_AUDIO_IN_0, stereoBuffer, size_audio_in_0);
        #else
            // stereo buffer declared in the application 
            arm_stream_io_ack(IO_PLATFORM_AUDIO_IN_0, stereoBuffer, size_audio_in_0);
        #endif
        break;
    case STREAM_STOP:           /* stop data moves */
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}


/*  IO_AL_idx = 5   */
void sensor_2d_in_0(uint32_t command, stream_xdmbuffer_t* data)
{
    switch (command)
    {
    case STREAM_RESET:
    case STREAM_SET_PARAMETER:
    case STREAM_SET_BUFFER:
    case STREAM_RUN:
    case STREAM_STOP:
    case STREAM_READ_PARAMETER:
    default:break;
    }
}




/*  IO_AL_idx = 6      1 io_alif1_line_out_0.txt      6    audio out stereo   160  */
void line_out_0 (uint32_t command, stream_xdmbuffer_t *data) 
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);
        #ifdef _MSC_VER 
        #define FILE_LINE_OUT_0 "..\\stream_test\\line_out_0.raw"
            if (NULL == (ptf_line_out_0 = fopen(FILE_LINE_OUT_0, "wb")))
            {   exit ( 1);
            }
        #endif
        break;        
    case STREAM_SET_PARAMETER:  /* presets reloaded */
        break;
    case STREAM_SET_BUFFER:     /* if memory allocation is made in the graph */
        break;
    case STREAM_RUN:            /* data moves */
        arm_stream_io_ack(IO_PLATFORM_LINE_OUT_0, buffer_line_out_0, size_line_out_0);

        #ifdef _MSC_VER 
            fwrite(buffer_line_out_0, 1, size_line_out_0, ptf_line_out_0);
            fflush(ptf_line_out_0);
        #endif
        break;
    case STREAM_STOP:           /* stop data moves */
        #ifdef _MSC_VER 
        fclose(ptf_line_out_0);
        #endif  
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;      
    }
}


/*  IO_AL_idx = 7   */
void gpio_out_0(uint32_t command, stream_xdmbuffer_t* data)
{
    switch (command)
    {
    case STREAM_RESET:
    case STREAM_SET_PARAMETER:
    case STREAM_SET_BUFFER:
    case STREAM_RUN:
    case STREAM_STOP:
    case STREAM_READ_PARAMETER:
    default:break;
    }
}


/*  IO_AL_idx = 8  UI/LED  */
void gpio_out_1(uint32_t command, stream_xdmbuffer_t* data)
{
    uint32_t tmp;

    switch (command)
    {
    case STREAM_RESET:
        #ifdef _MSC_VER 
        #define FILE_gpio_out_1 "..\\stream_test\\trace_0.raw"
                if (NULL == (ptf_gpio_out_1 = fopen(FILE_gpio_out_1, "wb")))
                {
                    exit(1);
                }
        #endif  
        break;
    case STREAM_SET_PARAMETER:
        //stream_format_io_setting = *(uint32_t *)(data->address);          
        break;
    case STREAM_SET_BUFFER:
    {
        stream_xdmbuffer_t* pt_pt;
        pt_pt = (stream_xdmbuffer_t*)data;
        pt_pt->address = (intptr_t)buffer_gpio_out_1;
        pt_pt->size = size_gpio_out_1;
    }
    break;
    case STREAM_RUN:
        /* "io_platform_stream_in_0," frame_size option in samples + FORMAT-0 in the example graph */
        arm_stream_io_ack(IO_PLATFORM_GPIO_OUT_1, (uint8_t*)data, size_gpio_out_1);
        #ifdef _MSC_VER 
                fwrite(data, 1, size_gpio_out_1, ptf_gpio_out_1);
                fflush(ptf_gpio_out_1);
        #else
        tmp = *(uint32_t*)data;
        if (tmp == 0)
        {
            vioSetSignal(vioLED0, vioLEDoff);  // Switch LED0 off
            vioSetSignal(vioLED1, vioLEDoff);  // Switch LED1 off
        }
        if (tmp == 1)
        {
            vioSetSignal(vioLED0, vioLEDon);   // Switch LED0 on
        }
        if (tmp == 2)
        {
            vioSetSignal(vioLED1, vioLEDon);   // Switch LED1 on
        }
        #endif  
        break;
    case STREAM_STOP:
    #ifdef _MSC_VER 
            fclose(ptf_gpio_out_1);
    #endif  
        break;
    case STREAM_READ_PARAMETER: /* setting done ? device is ready ? calibrated ? */
        break;
    default:
        break;
    }
}


/*  IO_AL_idx = 9       1 io_alif1_data_out_0.txt         9    debug trace        1024  */
void data_out_0(uint32_t command, stream_xdmbuffer_t* data)
{
    switch (command)
    {
    case STREAM_RESET:
        //stream_format_io_setting = *(uint32_t *)(data->address);          
        break;
    case STREAM_SET_PARAMETER:
        #ifdef _MSC_VER 
        #define FILE_data_out_0 "..\\stream_test\\trace_0.raw"
        if (NULL == (ptf_data_out_0 = fopen(FILE_data_out_0, "wb")))
        {
            exit(1);
        }
        #endif  
        break;
    case STREAM_SET_BUFFER:
    {
        stream_xdmbuffer_t* pt_pt;
        pt_pt = (stream_xdmbuffer_t*)data;
        pt_pt->address = (intptr_t)buffer_data_out_0;
        pt_pt->size = size_data_out_0;
    }
    break;
    case STREAM_RUN:
        /* "io_platform_stream_in_0," frame_size option in samples + FORMAT-0 in the example graph */
        arm_stream_io_ack(IO_PLATFORM_DATA_OUT_0, (uint8_t*)data, size_data_out_0);
        #ifdef _MSC_VER 
        fwrite(data, 1, size_data_out_0, ptf_data_out_0);
        fflush(ptf_data_out_0);
        #endif  
        break;
    case STREAM_STOP:
        #ifdef _MSC_VER 
        fclose(ptf_data_out_0);
        #endif  
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
