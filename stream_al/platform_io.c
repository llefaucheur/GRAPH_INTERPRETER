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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stream_const.h"      /* graph list */

#include "platform_const.h"

#include "stream_types.h"  
#include "platform_types.h"


/*
 * --- IO HW and board interfaces -------------------------------------------------------
 */
/* tuning of AUDIO_RENDER_STREAM_SETTING */
const int32_t audio_render_settings [] = { 
    /* nb options nbbits */
    /*  8  3  nchan */         3,   1, 2, 8,
    /* 16  4  FS */            2,   16000, 48000, 
    /*  4  2  framesize [ms] */2,   10, 16, 
    /*  8  3  mVrms max */     2,   100, 700,
    /* 16  4  PGA gain */      0,
    /*  4  2  bass gain dB */  4,   0, -3, 3, 6,
    /*  2  1  bass frequency */2,   80, 200,       
    /*  4  2  mid gain */      4,   0, -3, 3, 6,
    /*  2  1  mid frequency */ 2,   500, 2000,       
    /*  4  2  high gain */     4,   0, -3, 3, 6,
    /*  2  1  high frequency */2,   4000, 8000,       
    /*  2  1  agc gain */      0,
    /*     6 bits remains */ 
    };
 
 /*  
    data in Flash :
 */
struct platform_io_manifest platform_io [LAST_IO_FUNCTION_PLATFORM] = 
{
    {   //  PLATFORM_APPLICATION_DATA_IN
    .io_set = audio_ap_rx_set_stream,
    .io_start = audio_ap_rx_start_data_move,
    .io_stop = audio_ap_rx_stop_stream,
    .io_domain = PACK_IOMEMDOMAIN(1,0,0,0,0,IO_IS_FOLLOWER,PLATFORM_APPLICATION_DATA_IN),
    .stream_settings_default = { 
        PACKSTREAMFORMAT0(STREAM_Q15,FMT_DEINTERLEAVED_1PTR,32),
        PACKSTREAMFORMAT1(NO_TS,16000,0)},
    .stream_setting_extension = 0,
    },

    {   //  PLATFORM_AUDIO_OUT
    .io_set = audio_render_set_stream,
    .io_start = audio_render_start_data_move,
    .io_stop = audio_render_stop_stream,
    .io_domain = PACK_IOMEMDOMAIN(2,0,0,0,0,IO_IS_FOLLOWER,PLATFORM_AUDIO_OUT),
    .stream_settings_default = { 
        PACKSTREAMFORMAT0(STREAM_Q15,FMT_DEINTERLEAVED_1PTR,32),
        PACKSTREAMFORMAT1(NO_TS,16000,0)},
    .stream_setting_extension = audio_render_settings,
    }
};
 
 /*  
    data in RAM : a single Stream instance is in charge of one io
    the only extra SRAM area needed to interface with Stream is the address of the Stream instance

 */
intPtr_t platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];


/* --------------------------------------------------------------------------------------- */
int16_t *audio_ap_rx_data;
uint32_t audio_ap_rx_size;
/* --------------------------------------------------------------------------------------- */

void audio_ap_rx_transfer_done (uint8_t *data, uint32_t size) 
{   
    platform_al(PLATFORM_IO_ACK, (uint8_t *)PLATFORM_APPLICATION_DATA_IN, data, (uint8_t *)(uint64_t)size);
}



uint32_t audio_ap_rx_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 
    #define NSAMP 16
    int8_t i; 
    int8_t n;
    int8_t *ptr;
    static int16_t iptr;
    static int16_t samples[NSAMP] = { 
     0, 12539, 23170, 30273, 32767, 30273, 23170, 12539,
     0, -12539, -23170, -30273, -32767, -30273, -23170, -12539, };

    extern int16_t *audio_ap_rx_data;
    ptr = (int8_t *)audio_ap_rx_data;
    n = size;

    for (i = 0; i < n; i++)
    {   
        *ptr = (int8_t)((samples[15u & iptr])>>8u); ptr++; iptr++;
    }

    audio_ap_rx_transfer_done ((uint8_t *)(uint64_t)data, size);
    return 1u; 
}



uint32_t audio_ap_rx_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{ 
    uint32_t tmp;
    uint8_t *tmp8;

    tmp = *setting; tmp8 = data; tmp = size;
    return 1u;
}


uint32_t audio_ap_rx_set_stream (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 
    uint32_t tmp;
    tmp = *setting; 

    audio_ap_rx_data = (int16_t *)data;
    audio_ap_rx_size = size;
    return 1u;
}

/* --------------------------------------------------------------------------------------- */
int16_t *audio_render_data;
uint32_t audio_render_size;
/* --------------------------------------------------------------------------------------- */
void audio_render_transfer_done (uint8_t *data, uint32_t size) 
{   
    platform_al(PLATFORM_IO_ACK, (uint8_t *)PLATFORM_AUDIO_OUT, data, (uint8_t *)size);
}



uint32_t audio_render_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{   
    #define NSAMP 16
    static uint16_t i;
    static uint16_t iptr;
    static int16_t *ptr;
    static int16_t samples[NSAMP];
    uint32_t *tmp;

    tmp = setting;

    ptr = audio_render_data;

    for (i = 0; i < audio_render_size; i++)
    {   
        samples[15u & iptr] = *ptr; ptr++; iptr++;
    }

    audio_render_transfer_done ((uint8_t *)(uint64_t)audio_render_data, size);
    return 1u; 
}



uint32_t audio_render_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{   
    uint32_t tmp;
    uint8_t *tmp8;

    tmp = *setting; tmp8 = data; tmp = size;

    return 1u;
}



uint32_t audio_render_set_stream (uint32_t *setting, uint8_t *data, uint32_t size)
{   
    uint32_t tmp;
    tmp = *setting; 

    audio_render_data = (int16_t *)data;
    audio_render_size = size;
    return 1u;
}


/*
 * -----------------------------------------------------------------------
 */
