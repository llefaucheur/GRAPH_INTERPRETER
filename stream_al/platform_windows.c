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
#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "platform_al.h"

#ifdef _MSC_VER 
#include "../../CMSIS-Stream/stream_al/platform_windows.h"
#include "../../CMSIS-Stream/stream_src/stream_const.h"      
#include "../../CMSIS-Stream/stream_src/stream_types.h"  
#else
#include "platform_windows.h"
#include "stream_const.h"      
#include "stream_types.h"  
#endif

extern uint32_t audio_render_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint32_t audio_render_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint32_t audio_render_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);

extern uint32_t audio_ap_rx_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint32_t audio_ap_rx_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint32_t audio_ap_rx_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);

extern uint32_t trace_set (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint32_t trace_start(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint32_t trace_stop(uint32_t *setting, uint8_t *data, uint32_t siz);


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

static uint32_t MEXT[SIZE_MBANK_DMEM_EXT];
static uint32_t RAM1[SIZE_MBANK_DMEM/4];
static uint32_t RAM2[SIZE_MBANK_DMEM/4];
static uint32_t RAM3[SIZE_MBANK_DMEM/4];
static uint32_t RAM4[SIZE_MBANK_DMEM/4];
static uint32_t TCM1[SIZE_MBANK_DMEMFAST]; 
static uint32_t TCM2[SIZE_MBANK_DMEMFAST]; 
static uint32_t BKUP[SIZE_MBANK_BACKUP]; 
static uint32_t HWIO[SIZE_MBANK_HWIODMEM];
static uint32_t PMEM[SIZE_MBANK_PMEM];

#define PROC_ID 0 
extern const uint32_t graph_input[];

#if PROC_ID == 0
intPtr_t long_offset[NB_MEMINST_OFFSET];
#endif

void platform_specific_long_offset(void)
{
    long_offset[MBANK_DMEM_EXT] = (const intPtr_t)&(MEXT[10]);
    long_offset[MBANK_DMEM    ] = (const intPtr_t)&(RAM1[11]);
    long_offset[MBANK_DMEMPRIV] = (const intPtr_t)&(RAM1[12]);
    long_offset[MBANK_DMEMFAST] = (const intPtr_t)&(TCM1[13]);
    long_offset[MBANK_BACKUP  ] = (const intPtr_t)&(BKUP[14]);
    long_offset[MBANK_HWIODMEM] = (const intPtr_t)&(HWIO[15]);
    long_offset[MBANK_PMEM    ] = (const intPtr_t)&(PMEM[16]);
    long_offset[MBANK_FLASH   ] = (const intPtr_t)graph_input;
};


#if MULTIPROCESS == 1  
uint32_t check_hw_compatibility(uint32_t whoami, uint32_t bootParamsHeader) 
{
    uint8_t match = 1;

    if (RD(bootParamsHeader, ARCHID_BP) > 0u) /* do I care about the architecture ID ? */
    {   match = U8(RD(bootParamsHeader, ARCHID_BP) == RD(whoami, ARCHID_PARCH));
    }

    if (RD(bootParamsHeader, PROCID_BP) > 0u) /* do I care about the processor ID ? */
    {   match = match & U8(RD(bootParamsHeader, PROCID_BP) == RD(whoami, PROCID_PARCH));
    }
    return match;
}

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

#define check_hw_compatibility(whoami, bootParamsHeader) 1u  

//#define WR_BYTE_AND_CHECK_MP_(pt8b, code) 1
uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   return 1u;
}

#endif



extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3);

/*
 * ----------- data access --------------------------------------------------------------
 */

//
//extern uint32_t * platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];
//


/*
 * --- IO HW and board Manifest -------------------------------------------------------

   Declaration of master/follower stream control 

   Declaration of scaling factor to reach the RFC8428 (sensiml) unit
    used during the graph creation for the insertion of gain compensation
*/
    #if 0
    /* physical units rfc8428 rfc8798 */
    enum stream_units_physical_t {   
        unused_unit=0,      /* int16_t format */            /* int32_t format */           
        unit_linear,        /* PCM and default format */    /* PCM and default format */
        unit_dBm0, 
        unit_decibel,       /* Q11.4 :   1dB <> 0x0010      Q19.12 :   1dB <> 0x0000 1000  */
        unit_percentage,    /* Q11.4 :   1 % <> 0x0010      Q19.12 :   1 % <> 0x0000 1000 */
        unit_meter,         /* Q11.4 :  10 m <> 0x00A0      Q19.12 :  10 m <> 0x0000 A000 */
        . . . 
    #endif
/*
 * --- Digital HW Manifest ------------------------------------------------------------

   Declaration of the minimum guaranteed amount of memory pre-allocated per
    types (idx_memory_base_offset)

   Additional information on MBANK_DMEM when there are several physical memory 
    banks and several processors : to let the nodes dedicated to one processor 
    have their processing area isolated to the ones of the other processor.
    => manual manual memory mapping optimization step 
    GUI => YML file => TXT graph generation, verbose editable =>
    optional manual optimization process, mapping to HW platform specific =>
    last step  binary file (hashed) graph generation.
*/


/* --------------------------------------------------------------------------------------- 
 * --- IO HW and board interfaces --------------------------------------------------------
 */
/*  tuning of AUDIO_RENDER_STREAM_SETTING 
    const uint8_t platform_audio_out_bit_fields[] = { 3,4,2,3,4,2,1,2,1,2,1,2,1 };
*/
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
 


/* --------------------------------------------------------------------------------------- 
    data in Flash :
 */
struct platform_io_control platform_io [LAST_IO_FUNCTION_PLATFORM] = 
{
    {   /* PLATFORM_APPLICATION_DATA_IN_INSTANCE_0 */
    .io_set = audio_ap_rx_set_stream,
    .io_start = audio_ap_rx_start_data_move,
    .io_stop = audio_ap_rx_stop_stream,
    .stream_setting = 0, 
    },

    {   /* PLATFORM_AUDIO_OUT_INSTANCE_0 */
    .io_set = audio_render_set_stream,
    .io_start = audio_render_start_data_move,
    .io_stop = audio_render_stop_stream,
    .stream_setting = audio_render_settings,
    },

    {   /* PLATFORM_COMMAND_OUT_INSTANCE_0 */
    .io_set = trace_set,
    .io_start = trace_start,
    .io_stop = trace_stop,
    .stream_setting = 0,
    },
};




/* --------------------------------------------------------------------------------------- */
FILE *ptf_in_audio_ap_rx_data;
FILE *ptf_in_audio_render_data;
FILE *ptf_trace;
uint32_t frame_size_audio_render;





/* --------------------------------------------------------------------------------------- */
void audio_render_transfer_done (uint8_t *data, uint32_t size) 
{   platform_al(PLATFORM_IO_ACK, (uint8_t *)PLATFORM_AUDIO_OUT_INSTANCE_0, 
        data, (uint8_t *)(uint64_t)size);
}
/* --------------------------------------------------------------------------------------- */
void audio_ap_rx_transfer_done (uint8_t *data, uint32_t size) 
{   platform_al(PLATFORM_IO_ACK, (uint8_t *)PLATFORM_APPLICATION_DATA_IN_INSTANCE_0,
        data, (uint8_t *)(uint64_t)size);
}
/* --------------------------------------------------------------------------------------- */
void trace_ap_rx_transfer_done (uint8_t *data, uint32_t size) 
{   platform_al(PLATFORM_IO_ACK, (uint8_t *)PLATFORM_COMMAND_OUT_INSTANCE_0,
        data, (uint8_t *)(uint64_t)size);
}




/* --------------------------------------------------------------------------------------- */
uint32_t trace_start (uint32_t *setting, uint8_t *data, uint32_t size) 
{   //data[size] = 0;     /* end of string */
    //fprintf(ptf_trace, "%s\n", data);
    fwrite(data, 1, size, ptf_trace);
    fflush(ptf_trace);
    trace_ap_rx_transfer_done ((uint8_t *)data, size);
    return 1u; 
}
/* --------------------------------------------------------------------------------------- */
uint32_t trace_stop (uint32_t *setting, uint8_t *data, uint32_t size) 
{   fclose (ptf_trace);
    return 1u;
}
/* --------------------------------------------------------------------------------------- */
uint32_t trace_set (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 
//#define FILE_TRACE "..\\trace.txt"
//    if (NULL == (ptf_trace = fopen(FILE_TRACE, "wt")))
#define FILE_TRACE "..\\trace.raw"
    if (NULL == (ptf_trace = fopen(FILE_TRACE, "wb")))
    {   exit (-1);
    }
    return 1u;
}






/* --------------------------------------------------------------------------------------- */
uint32_t audio_ap_rx_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{   size_t tmp;
    int32_t i, s, j;
    int16_t *data16;

     data16 = (int16_t *)data;
    for (i = 0; i < (size/2); i++)
    { j = fscanf(ptf_in_audio_ap_rx_data, "%d,", &s);
 data16[i] = s;
    }
     if (j > 0) tmp = size; else tmp = 0;

    // tmp = fread(data, 1, size, ptf_in_audio_ap_rx_data);
    if (size != tmp)
    {   audio_ap_rx_transfer_done ((uint8_t *)data, 0);
        fclose (ptf_in_audio_ap_rx_data);
        exit(-1);
    }
    else
    {   audio_ap_rx_transfer_done ((uint8_t *)data, size);
    }
    return 1u; 
}

/* --------------------------------------------------------------------------------------- */
uint32_t audio_ap_rx_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{   fclose (ptf_in_audio_ap_rx_data);
    return 1u;
}

/* --------------------------------------------------------------------------------------- */
uint32_t audio_ap_rx_set_stream (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 
// #define FILE_IN "VAD_TEST.raw"
#define FILE_IN "VoiceTestPattern.txt"
//define FILE_IN "..\\sine_noise.raw"

    if (NULL == (ptf_in_audio_ap_rx_data = fopen(FILE_IN, "rt")))
    {   
        printf("Exiting \n");
        exit (-1);
    }
    return 1u;
}




/* --------------------------------------------------------------------------------------- */
uint32_t audio_render_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{   fwrite(data, 1, size, ptf_in_audio_render_data);
    fflush(ptf_in_audio_render_data);

    audio_render_transfer_done ((uint8_t *)data, size);
    return 1u; 
}

/* --------------------------------------------------------------------------------------- */
uint32_t audio_render_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{   fclose (ptf_in_audio_render_data);
    return 1u;
}

/* --------------------------------------------------------------------------------------- */
uint32_t audio_render_set_stream (uint32_t *setting, uint8_t *data, uint32_t size)
{   uint8_t index_frame_size = 0;
#define FILE_OUT "..\\audio_out.raw"
    if (NULL == (ptf_in_audio_render_data = fopen(FILE_OUT, "wb")))
    {   exit (-1);
    }

    /* simulate IO master port with a fixed frame size */
    frame_size_audio_render = extract_sensor_field 
        (platform_audio_out_bit_fields, audio_render_settings, PLATFORM_AUDIO_OUT_FRAMESIZE, index_frame_size);

    return 1u;
}

/*
 * -----------------------------------------------------------------------
 */
   



