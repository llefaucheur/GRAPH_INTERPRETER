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
#ifdef __cplusplus
 extern "C" {
#endif

/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 0


#define _CRT_SECURE_NO_DEPRECATE 1
#if DATA_FROM_FILES
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>


#include "stream_const.h"      

#include "../platform_al.h"
#include "stream_types.h"



#define PLATFORM_AUDIO_OUT_NCHAN    0
#define PLATFORM_AUDIO_OUT_FS       1
#define PLATFORM_AUDIO_OUT_FRAMESIZE 2
#define PLATFORM_AUDIO_OUT_MVRMS    3
extern const uint8_t platform_audio_out_bit_fields[];

extern void platform_io_ack (uint8_t fw_io_idx, uint8_t *data,  uint32_t size);

extern uint8_t gpio_out_0_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t gpio_out_0_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t gpio_out_0_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);
           
extern uint8_t stream_in_0_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t stream_in_0_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t stream_in_0_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);
           
extern uint8_t trace_set (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t trace_start(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t trace_stop(uint32_t *setting, uint8_t *data, uint32_t siz);


/*-------------------PLATFORM MANIFEST-----------------------
                  +-----------------+
                  | static |working |
   +--------------------------------+
   |internal RAM  |        |        |
   +--------------+--------+--------+
*/
#define SIZE_MBANK_DMEM     0x80    /* internal (graph and SWC instances) = 128 words = 512 bytes*/
#define MBANK_GRAPH         U(0)    /* graph base address (shared) */
static uint32_t RAM1[SIZE_MBANK_DMEM];

#define PROC_ID 0 
extern const uint32_t graph_input[];

intPtr_t long_offset[MAX_NB_MEMORY_OFFSET];

uint8_t platform_iocontrol[LAST_IO_FUNCTION_PLATFORM];


/**
  @brief        Memory banks initialization
  @param[in]    none
  @return       none

  @par          Loads the global variable of the platform holding the base addresses 
                to the physical memory banks described in the "platform manifest"

  @remark       
 */

void platform_specific_long_offset(intPtr_t long_offset[])
{
    /* all the addresses can fit in the +/- 8MB range around [0] */
    long_offset[MBANK_GRAPH]    = (intPtr_t)&(RAM1[0]); 
};


/**
  @brief        Read the HW processor ID and its architecture 
  @param[in]    none
  @return       none

  @par          

  @remark       
 */

void platform_specific_processor_arch_iomask(struct HW_params **params)
{
    (*params)->procID = 1; 
    (*params)->archID= 0; 

    /* list of bytes holding the status of the on-going data moves (MP) */
    (*params)->ioctrl= &(platform_iocontrol[0]);

    /* 10 IOs */
    (*params)->iomask= 0x3FF; 
};


#if MULTIPROCESSING != 0

/**
  @brief        Memory banks initialization
  @param[in]    none
  @return       none

  @par          Loads the global variable of the platform holding the base addresses 
                to the physical memory banks described in the "platform manifest"

  @remark       
 */
uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   volatile uint8_t *pt8;
    pt8 = pt8b;

    *pt8 = code;
    INSTRUCTION_SYNC_BARRIER;

    /* no need to use LDREX, don't wait and escape if collision occurs */
    DATA_MEMORY_BARRIER;

    return (*pt8 == code);
}

#else

uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   return 1u;
}

#endif

/* --------------------------------------------------------------------------------------- 
    replicated fw_io_dx : platform_computer.h <=> manifest_computer.txt 

    #define IO_PLATFORM_STREAM_IN_0      1  interface to the application processor see stream_al\io_platform_stream_in_0 
    #define IO_PLATFORM_IMU_0            2  3D motion sensor see stream_al\io_platform_imu_0 
    #define IO_PLATFORM_MICROPHONE_0     3  audio in mono see stream_al\io_platform_microphone_0.txt  
    #define IO_PLATFORM_LINE_IN_0        4  audio in stereo  stream_al\io_platform_line_in_0.txt     
    #define IO_PLATFORM_LINE_OUT_0       5  audio out stereo stream_al\io_platform_line_out_0.txt    
    #define IO_PLATFORM_ANALOG_SENSOR_0  6  analog converter stream_al\io_platform_analog_sensor_0.txt 
    #define IO_PLATFORM_GPIO_OUT_0       7  PWM              stream_al\io_platform_gpio_out_0.txt    
    #define IO_PLATFORM_GPIO_OUT_1       8  LED              stream_al\io_platform_gpio_out_1.txt    
    #define IO_PLATFORM_COMMAND_IN_0     9  UART command     stream_al\io_platform_command_in_0.txt  
    #define IO_PLATFORM_COMMAND_OUT_0   10  UART trace       stream_al\io_platform_command_out_0.txt 

    functions corresponding to the platform capabilities, indexed with fw_io_idx :
 */
struct platform_io_control platform_io [LAST_IO_FUNCTION_PLATFORM] = 
{
    { /* INDEX 0 IS NOT USED */ 
    .io_set = 0, 
    .io_start = 0, 
    .io_stop = 0, 
    },

    {   /* stream_al\platform_stream_in_0.txt      1 interface to the application processor */
    .io_set = stream_in_0_set_stream,
    .io_start = stream_in_0_start_data_move,
    .io_stop = stream_in_0_stop_stream,
    },

    {   /* stream_al\platform_imu_0.txt            2 3D motion sensor */
    .io_set = 0,
    .io_start = 0,
    .io_stop = 0,
    },

    {   /* stream_al\platform_microphone_0.txt     3 audio in mono    */
    .io_set = 0,
    .io_start = 0,
    .io_stop = 0,
    },

    {   /* stream_al\platform_line_in_0.txt        4 audio in stereo  */
    .io_set = 0,
    .io_start = 0,
    .io_stop = 0,
    },

    {   /* stream_al\platform_line_out_0.txt       5 audio out stereo */
    .io_set = 0,
    .io_start = 0,
    .io_stop = 0,
    },

    {   /* stream_al\platform_analog_sensor_0.txt  6 ADC              */
    .io_set = 0,
    .io_start = 0,
    .io_stop = 0,
    },

    {   /* stream_al\platform_gpio_out_0.txt       7 PWM              */
    .io_set = gpio_out_0_set_stream,
    .io_start = gpio_out_0_start_data_move,
    .io_stop = gpio_out_0_stop_stream,
    },

    {   /* stream_al\platform_gpio_out_1.txt       8 LED              */
    .io_set = 0,
    .io_start = 0,
    .io_stop = 0,
    },

    {   /* stream_al\platform_command_out_0.txt    9  shock detector  */
    .io_set = trace_set,
    .io_start = trace_start,
    .io_stop = trace_stop,
    },

    {   /* stream_al\platform_command_out_1.txt   10  temp. sensor   */
    .io_set = 0,
    .io_start = 0,
    .io_stop = 0,
    },
};


/* --------------------------------------------------------------------------------------- 
    global variables of this platform 
*/


#if DATA_FROM_FILES
FILE *ptf_trace;
FILE *ptf_in_stream_in_0_data;
FILE *ptf_in_gpio_out_data;
#else
const int16_t ptf_in_stream_in_0_data[]={ 
    //
    0,0,0
    // #include "sine_noise_offset.txt"
};
uint16_t ptf_in_gpio_out_data[sizeof(ptf_in_stream_in_0_data)/sizeof(int16_t)];
uint16_t ptf_trace[6*sizeof(ptf_in_stream_in_0_data)/sizeof(int16_t)];

uint32_t ptr_in_stream_in_0_data;
uint32_t ptr_in_gpio_out_data;
uint32_t ptr_trace;
#endif
uint32_t frame_size_audio_render;


/**
  @brief        IO functions "set" "start" "stop" for the debug trace
  @param[in]    settings    the specific setting of this IO interface
  @param[in]    data        pointer to the data to transfer
  @param[in]    size        size of the buffer
  @return       int8        flag, always 1 except when error occurs

  @par          specific to the platform, 

  @remark       
 */
/* --------------------------------------------------------------------------------------- */
uint8_t trace_start (uint32_t *setting, uint8_t *data, uint32_t size) 
{   //data[size] = 0;     /* end of string */
#if DATA_FROM_FILES
    //fprintf(ptf_trace, "%s\n", data);
    fwrite(data, 1, size, ptf_trace);
    fflush(ptf_trace);
    platform_io_ack(IO_PLATFORM_COMMAND_OUT, data,size);
#else
    ptr_trace = 0;  // no trace 
#endif
    return 1u; 
}
/* --------------------------------------------------------------------------------------- */
uint8_t trace_stop (uint32_t *setting, uint8_t *data, uint32_t size) 
{   
#if DATA_FROM_FILES
    fclose (ptf_trace);
#endif
    return 1u;
}
/* --------------------------------------------------------------------------------------- */
uint8_t trace_set (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 
#if DATA_FROM_FILES

#define FILE_TRACE "..\\..\\..\\stream_test\\trace.raw"
    if (NULL == (ptf_trace = fopen(FILE_TRACE, "wb")))
    {   exit (-1);
    }
#else
    ptr_trace = 0;
#endif
    return 1u;
}





/**
  @brief        IO functions "set" "start" "stop" for data streams from the application
  @param[in]    settings    the specific setting of this IO interface
  @param[in]    data        pointer to the data to transfer
  @param[in]    size        size of the buffer
  @return       int8        flag, always 1 except when error occurs

  @par          specific to the platform, 

  @remark       
 */

static int16_t *rx_buffer;
/* --------------------------------------------------------------------------------------- 
    data = final destination in the ARC
    size = free space for copy

    FORMAT 0
    I F   R N    FS T I S
    0 8  17 1 16000 0 0 3    

    "io_platform_stream_in_0.txt" :

    1   commander=0 servant=1                               
    0   buffer declared from the graph 0 or BSP 1           
    0   in standard RAM=0, in HW IO RAM=1                   
    1   processor affinity bit-field                        

    0   rx0tx1  
    6   raw format                                          
    0   time-stamp                                          
    1   frame length format (0:in milliseconds 1:in samples)
    1   sampling rate format (0:Hz, 1:seconds, 2:days)
    3   percent FS accuracy                                 

    ;   Lists of options, starting with the default index 
    0   0                   ;f [1bits] interleaving options list
    0   1                   ;f [2bits] nb channels options list 
    2   4 5 6               ;f [2bits] frame_size option in samples default=4samples = 8Bytes/frame
    1   101 202 404         ;f [3bits] sampling rate options in Hz
*/

uint8_t stream_in_0_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{   int32_t tmp;
    uint32_t cumulated_data = 0;
    int16_t *data16 = rx_buffer;

 /* "io_platform_stream_in_0.txt" frame_size option in samples + FORMAT-0 in the example graph */ 
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
#else
    memcpy(data16, &(ptf_in_stream_in_0_data[ptr_in_stream_in_0_data]), 2 * size);
    ptr_in_stream_in_0_data += size;
    if (ptr_in_stream_in_0_data*2 >= sizeof(ptf_in_stream_in_0_data))
        tmp = size+1;
    else tmp = size;
#endif
    cumulated_data = 2 * size;

    if (size != tmp)
    {   platform_io_ack (IO_PLATFORM_DATA_IN, data, 0);
#if DATA_FROM_FILES
        fclose (ptf_in_stream_in_0_data);
#endif
        exit(-1);
    }
    else
    {   platform_io_ack (IO_PLATFORM_DATA_IN, (uint8_t *)data16, cumulated_data);
    }
    return 1u; 
}
/* --------------------------------------------------------------------------------------- */
uint8_t stream_in_0_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{   
#if DATA_FROM_FILES
    fclose (ptf_in_stream_in_0_data);
#endif
    return 1u;
}
/* --------------------------------------------------------------------------------------- */
uint8_t stream_in_0_set_stream (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 

    //@@@ DECODE SETTINGS => frame size, sampling, ..
#if DATA_FROM_FILES
    if (NULL == (ptf_in_stream_in_0_data = fopen("..\\..\\..\\stream_test\\sine_noise_offset.wav", "rb")))
    {   exit (-1); }
    else 
    {   int i, c; for(i=0;i<512;i++)fread(&c,1,1,ptf_in_stream_in_0_data); }// skeep WAV header

#else
    ptr_in_stream_in_0_data = 0;
#endif

    rx_buffer = (int16_t *)data;
    return 1u;
}


/* 
    See platform_sensor.h for the bit-field meaning per domain
*/
const uint8_t platform_audio_out_bit_fields[] = { 3,4,2,3,4,2,1,2,1,2,1,2,1 };

static int16_t *tx_buffer;
/**
  @brief        Extract setting fields
  @param[in]    *bit_field list of the bit-fields used for this IO domain
  @param[in]    settings   the specific setting of this IO interface
  @param[in]    line       the setting to have access to
  @param[in]    index      the selected index 
  @return       int32      the extracted field

  @par          Each IO interface is associated to a "domain" of operation. Each domain
                is set with default values at reset, or a list of proposed options. 

  @remark       
 */
int32_t extract_sensor_field (const uint8_t *platform_bit_fields, 
                              const int32_t *settings,
                              uint8_t setting_line,
                              uint8_t index)
{
    uint8_t i, j, i_field, nb_fields;

    i_field = 0;

    for (i = 0; i < setting_line; i++)
    {   nb_fields = settings[i_field];
        for (j = 0; j < nb_fields; j++)
        {
            /* TBC */
        }
        i_field = i_field + nb_fields;
    }
    return 3;
}

/**
  @brief        IO functions "set" "start" "stop" for data stream to the emulated audio interface
  @param[in]    settings    the specific setting of this IO interface
  @param[in]    data        pointer to the data to transfer
  @param[in]    size        size of the buffer
  @return       int8        flag, always 1 except when error occurs

  @par          specific to the platform, 
                When the stream is commander the "start" function is called by the device 
                    driver in the DMA interrupt
                Otherwise the "start" function is called from the AL
                    platform_al(PLATFORM_IO_DATA_START, XDM buffer, 0u, 0u);  translated to :
                        io_start(settings, buffer->address, buffer->size);
  @remark       
 */
/* --------------------------------------------------------------------------------------- 

    FORMAT 1
    I F   R N    FS T I S
    1 12 17 1 16000 0 0 4    formatID1; frameSize; raw; nchan; FS(float); timestp; intlv; specific(Word2);

    io_platform_line_out_0  name for the GUI                            
    audio_out               domain name 
    1   commander=0 servant=1                               
    0   buffer declared from the graph 0 or BSP 1           
    0   in standard RAM=0, in HW IO RAM=1                   
    1   processor affinity bit-field                        

    1   rx0tx1  
    6   raw format                                          
    0   time-stamp                                          
    1   frame length format (0:in milliseconds 1:in samples)
    1   sampling rate format (0:Hz, 1:seconds, 2:days)
    3   percent FS accuracy                                 

    ;   Lists of options, starting with the default index 
    0   0                   ;f [1bits] interleaving options list
    0   1                   ;f [2bits] nb channels options list 
    2   4 5 6               ;f [2bits] frame_size option in samples  (or millis) 
    1   16000 44100 48000   ;f [3bits] sampling rate options in Hz

    #define IO_PLATFORM_STREAM_IN_0      1        interface to the application processor see stream_al\io_platform_stream_in_0 
    #define IO_PLATFORM_IMU_0            2        3D motion sensor see stream_al\io_platform_imu_0 
    #define IO_PLATFORM_MICROPHONE_0     3        audio in mono see stream_al\io_platform_microphone_0.txt  
    #define IO_PLATFORM_LINE_IN_0        4        audio in stereo  stream_al\io_platform_line_in_0.txt     
    #define IO_PLATFORM_LINE_OUT_0       5        audio out stereo stream_al\io_platform_line_out_0.txt    
    #define IO_PLATFORM_ANALOG_SENSOR_0  6        analog converter stream_al\io_platform_analog_sensor_0.txt 
    #define IO_PLATFORM_GPIO_OUT_0       7        PWM              stream_al\io_platform_gpio_out_0.txt    
    #define IO_PLATFORM_GPIO_OUT_1       8        LED              stream_al\io_platform_gpio_out_1.txt    
    #define IO_PLATFORM_DATA_IN_0        9        shock detector   stream_al\io_platform_imu_metadata_0.txt 
    #define IO_PLATFORM_DATA_IN_1       10        temp. sensor     stream_al\io_platform_imu_temperature_0.txt 

*/
uint8_t gpio_out_0_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{   
 /* "io_platform_stream_in_0.txt" frame_size option in samples + FORMAT-0 in the example graph */ 
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
    platform_io_ack (IO_PLATFORM_GPIO_OUT_0, (uint8_t *)tx_buffer, FORMAT_CONSUMER_FRAME_SIZE);
    return 1u; 
}
/* --------------------------------------------------------------------------------------- */
uint8_t gpio_out_0_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{   
#if DATA_FROM_FILES
    fclose (ptf_in_gpio_out_data);
#endif
    return 1u;
}
/* --------------------------------------------------------------------------------------- */
uint8_t gpio_out_0_set_stream (uint32_t *setting, uint8_t *data, uint32_t size)
{   uint8_t index_frame_size = 0;

    //@@@ DECODE SETTINGS => frame size, sampling, ..

    const uint8_t platform_audio_out_bit_fields[] = { 3,4,2,3,4,2,1,2,1,2,1,2,1 };
    /* device driver data */
    //const int32_t gpio_out_0_settings [] = { 
    //    /* nb options nbbits */
    //    /*  8  3  nchan */         3,   1, 2, 8,
    //    /* 16  4  FS */            2,   16000, 48000, 
    //    /*  4  2  framesize [ms] */2,   10, 16, 
    //    /*  8  3  mVrms max */     2,   100, 700,
    //    /* 16  4  PGA gain */      0,
    //    /*  4  2  bass gain dB */  4,   0, -3, 3, 6,
    //    /*  2  1  bass frequency */2,   80, 200,       
    //    /*  4  2  mid gain */      4,   0, -3, 3, 6,
    //    /*  2  1  mid frequency */ 2,   500, 2000,       
    //    /*  4  2  high gain */     4,   0, -3, 3, 6,
    //    /*  2  1  high frequency */2,   4000, 8000,       
    //    /*  2  1  agc gain */      0,
    //    /*     6 bits remains */ 
    //    };

#if DATA_FROM_FILES
#define FILE_OUT "..\\..\\..\\stream_test\\audio_out.raw"
    if (NULL == (ptf_in_gpio_out_data = fopen(FILE_OUT, "wb")))
    {   exit (-1);
    }
#else
    ptr_in_gpio_out_data = 0;
#endif

    tx_buffer = (int16_t *)data; 
    memset(tx_buffer, 0, FORMAT_CONSUMER_FRAME_SIZE);

    /* simulate IO commander port with a fixed frame size */
    //frame_size_audio_render = extract_sensor_field 
    //    (platform_audio_out_bit_fields, 0 /*gpio_out_0_settings */, PLATFORM_AUDIO_OUT_FRAMESIZE, index_frame_size);

    return 1u;
}

/*
 * -----------------------------------------------------------------------
 */
   



