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

#include "platform_al.h"

#include "platform_windows.h"
#include "stream_const.h"      
#include "stream_types.h"  

/*-------------------PLATFORM MANIFEST-----------------------
                  +-----------------+
                  | static |working |
   +--------------------------------+
   |external RAM  |        |        |
   +--------------------------------+
   |internal RAM  |        |        |
   +--------------------------------+
   |fast TCM      |  N/A   |        |
   +--------------+--------+--------+
*/
#define SIZE_MBANK_DMEM_EXT   0x20000  /* external */
#define SIZE_MBANK_DMEM       0x8000    /* internal */
#define SIZE_MBANK_DMEMFAST   0x4000    /* TCM */
#define SIZE_MBANK_BACKUP     0x10      /* BACKUP */
#define SIZE_MBANK_HWIODMEM   0x1000    /* DMA buffer */
#define SIZE_MBANK_PMEM       0x100     /* patch */

uint32_t MEXT[SIZE_MBANK_DMEM_EXT];
uint32_t RAM1[SIZE_MBANK_DMEM/4];
uint32_t RAM2[SIZE_MBANK_DMEM/4];
uint32_t RAM3[SIZE_MBANK_DMEM/4];
uint32_t RAM4[SIZE_MBANK_DMEM/4];
uint32_t TCM1[SIZE_MBANK_DMEMFAST]; 
uint32_t TCM2[SIZE_MBANK_DMEMFAST]; 
uint32_t BKUP[SIZE_MBANK_BACKUP]; 
uint32_t HWIO[SIZE_MBANK_HWIODMEM];
uint32_t PMEM[SIZE_MBANK_PMEM];

#define PROC_ID 0 
extern const uint32_t graph_input[];

#if PROC_ID == 0
intPtr_t long_offset[NB_MEMINST_OFFSET] = 
{
    (intPtr_t)&(MEXT[10]), // MBANK_DMEM_EXT
    (intPtr_t)&(RAM1[11]), // MBANK_DMEM    
    (intPtr_t)&(RAM1[12]), // MBANK_DMEMPRIV
    (intPtr_t)&(TCM1[13]), // MBANK_DMEMFAST
    (intPtr_t)&(BKUP[14]), // MBANK_BACKUP  
    (intPtr_t)&(HWIO[15]), // MBANK_HWIODMEM
    (intPtr_t)&(PMEM[16]), // MBANK_PMEM    
    (intPtr_t)graph_input, // MBANK_FLASH   ideally 16Bytes-aligned for arc shifter 1
};
#endif


#if MULTIPROCESS == 1
static uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   volatile uint8_t *pt8 = pt8b;
    *pt8 = code;
    INSTRUCTION_SYNC_BARRIER;

    /* no need to use LDREX, don't wait and escape if collision occurs */
    DATA_MEMORY_BARRIER;

    return (*pt8 == code);
}
#else
#define WR_BYTE_AND_CHECK_MP_(pt8b, code) 1
#endif




extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3);

/*
 * ----------- data access --------------------------------------------------------------
 */

//
//extern uint32_t * platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];
//
extern uint32_t audio_render_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_render_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_render_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);

extern uint32_t audio_ap_rx_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_ap_rx_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) ;
extern uint32_t audio_ap_rx_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);

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
