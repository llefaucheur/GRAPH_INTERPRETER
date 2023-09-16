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
#include "platform_sensor.h"

#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  



/*
     8  3  nchan          3,   1, 2, 8,
    16  4  FS             2,   16000, 48000, 
     4  2  framesize [ms] 2,   10, 16, 
*/
#define PLATFORM_AUDIO_OUT_NCHAN    0
#define PLATFORM_AUDIO_OUT_FS       1
#define PLATFORM_AUDIO_OUT_FRAMESIZE 2
#define PLATFORM_AUDIO_OUT_MVRMS    3
extern const uint8_t platform_audio_out_bit_fields[];


extern uint8_t audio_render_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t audio_render_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t audio_render_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);
           
extern uint8_t audio_ap_rx_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t audio_ap_rx_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t audio_ap_rx_set_stream(uint32_t *setting, uint8_t *data, uint32_t siz);
           
extern uint8_t trace_set (uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t trace_start(uint32_t *setting, uint8_t *data, uint32_t size);
extern uint8_t trace_stop(uint32_t *setting, uint8_t *data, uint32_t siz);


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
#define SIZE_MBANK_DMEM_EXT   0x100     /* external (buffers) */
#define SIZE_MBANK_DMEM       0x100     /* internal (graph and SWC instances) */
#define SIZE_MBANK_DMEMFAST   0x20      /* TCM (fast RAM) */
#define SIZE_MBANK_BACKUP     0x10      /* BACKUP */
#define SIZE_MBANK_HWIODMEM   0x100     /* DMA buffer */
#define SIZE_MBANK_PMEM       0x100     /* patch */


// MEMORY BANKS
//#define MBANK_GRAPH     U(0)    /* graph base address (shared) */
//#define MBANK_DMEM      U(1)    /* shared internal/external memory */
//#define MBANK_DMEMPRIV  U(2)    /* not shared memory space */
//#define MBANK_DMEMFAST  U(3)    /* not shared DTCM Cortex-M/LLRAM Cortex-R, swapped between SWC calls if static */
//#define MBANK_BACKUP    U(4)    /* shared backup SRAM addressed only by STREAM */
//#define MBANK_HWIODMEM  U(5)    /* shared memory space for I/O and DMA buffers */
//#define MBANK_PMEM      U(6)    /* shared program RAM */
//#define MBANK_FLASH     U(7)    /* shared internal Flash */

static uint64_t MEXT[SIZE_MBANK_DMEM_EXT];
static uint64_t RAM1[SIZE_MBANK_DMEM];
static uint64_t RAM2[SIZE_MBANK_DMEM];
static uint64_t RAM3[SIZE_MBANK_DMEM];
static uint64_t RAM4[SIZE_MBANK_DMEM];
static uint64_t TCM1[SIZE_MBANK_DMEMFAST]; 
static uint64_t TCM2[SIZE_MBANK_DMEMFAST]; 
static uint64_t BKUP[SIZE_MBANK_BACKUP]; 
static uint64_t HWIO[SIZE_MBANK_HWIODMEM];
static uint64_t PMEM[SIZE_MBANK_PMEM];

#define PROC_ID 0 
extern const uint32_t graph_input[];

intPtr_t long_offset[NB_MEMINST_OFFSET];


/**
  @brief        Memory banks initialization
  @param[in]    none
  @return       none

  @par          Loads the global variable of the platform holding the base addresses 
                to the physical memory banks described in the "platform manifest"

  @remark       
 */

void platform_specific_long_offset(void)
{
    long_offset[MBANK_GRAPH   ] = (const intPtr_t)&(MEXT[10]);  
    long_offset[MBANK_DMEM    ] = (const intPtr_t)&(RAM1[10]);
    long_offset[MBANK_DMEMPRIV] = (const intPtr_t)&(RAM1[10]); /* random offsets */
    long_offset[MBANK_DMEMFAST] = (const intPtr_t)&(TCM1[10]);
    long_offset[MBANK_BACKUP  ] = (const intPtr_t)&(BKUP[10]);
    long_offset[MBANK_HWIODMEM] = (const intPtr_t)&(HWIO[10]);
    long_offset[MBANK_PMEM    ] = (const intPtr_t)&(PMEM[10]);
    long_offset[MBANK_FLASH   ] = (const intPtr_t)graph_input;

    //long_offset[MBANK_SRAM1   ] = (const intPtr_t)&(RAM1[10]);
    //long_offset[MBANK_SRAM2   ] = (const intPtr_t)&(RAM1[10]);
    //long_offset[MBANK_SRAM3   ] = (const intPtr_t)&(RAM1[10]);
    //long_offset[MBANK_SRAM4   ] = (const intPtr_t)&(RAM1[10]);
    //long_offset[MBANK_SRAM5   ] = (const intPtr_t)&(RAM1[10]);
    //long_offset[MBANK_SRAM6   ] = (const intPtr_t)&(RAM1[10]);
    //long_offset[MBANK_SRAM7   ] = (const intPtr_t)&(RAM1[10]);
    //long_offset[MBANK_SRAM8   ] = (const intPtr_t)&(RAM1[10]);
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


extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3);

/*
 * ----------- data access --------------------------------------------------------------
 */

//
//extern uint32_t * platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];
//


/*
 * --- IO HW and board Manifest -------------------------------------------------------

   Declaration of commander/servant stream control 

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




/* --------------------------------------------------------------------------------------- 
    global variables of this platform 
*/

FILE *ptf_in_audio_ap_rx_data;
FILE *ptf_in_audio_render_data;
FILE *ptf_trace;
uint32_t frame_size_audio_render;




/**
  @brief         Callbacks used after a data transfer is done
  @param[in]     data       pointer to the data being exchanged
  @param[in]     size       size in bytes of the exchanged
  @return        none

  @par           Audio rendering buffer transfer callback
  @remark
 */

void audio_render_transfer_done (uint8_t *data, uint32_t size) 
{   platform_al(PLATFORM_IO_ACK, data, 0, PACK_PARAM_AL3(PLATFORM_AUDIO_OUT_INSTANCE_0, size));
}


/**
  @brief         Callbacks used after a data transfer is done
  @param[in]     data       pointer to the data being exchanged
  @param[in]     size       size in bytes of the exchanged
  @return        none

  @par           Audio reception buffer transfer callback
  @remark
 */

void audio_ap_rx_transfer_done (uint8_t *data, uint32_t size) 
{   platform_al(PLATFORM_IO_ACK, data, 0, PACK_PARAM_AL3(PLATFORM_DATA_STREAM_IN_INSTANCE_0, size));
}


/**
  @brief         Callbacks used after a data transfer is done
  @param[in]     data       pointer to the data being exchanged
  @param[in]     size       size in bytes of the exchanged
  @return        none

  @par           Debug trace end of transfer callback
  @remark
 */

void trace_ap_rx_transfer_done (uint8_t *data, uint32_t size) 
{   platform_al(PLATFORM_IO_ACK, data, 0, PACK_PARAM_AL3(PLATFORM_COMMAND_OUT_INSTANCE_0, size));
}



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
    //fprintf(ptf_trace, "%s\n", data);
    fwrite(data, 1, size, ptf_trace);
    fflush(ptf_trace);
    trace_ap_rx_transfer_done ((uint8_t *)data, size);
    return 1u; 
}
/* --------------------------------------------------------------------------------------- */
uint8_t trace_stop (uint32_t *setting, uint8_t *data, uint32_t size) 
{   fclose (ptf_trace);
    return 1u;
}
/* --------------------------------------------------------------------------------------- */
uint8_t trace_set (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 
//#define FILE_TRACE "..\\trace.txt"
//    if (NULL == (ptf_trace = fopen(FILE_TRACE, "wt")))
#define FILE_TRACE "..\\trace.raw"
    if (NULL == (ptf_trace = fopen(FILE_TRACE, "wb")))
    {   exit (-1);
    }
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
/* --------------------------------------------------------------------------------------- */
uint8_t audio_ap_rx_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{   int32_t j, tmp;
    uint32_t i;
    int16_t *data16;

    data16 = (int16_t *)data;
    for (i = 0; i < (size/2); i++) 
    {   j = fscanf(ptf_in_audio_ap_rx_data, "%d,", &tmp); 
        data16[i] = (int16_t)tmp; 
    }
    if (j > 0) tmp = size; else tmp = 0;

    //tmp = fread(data, 1, size, ptf_in_audio_ap_rx_data);
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
uint8_t audio_ap_rx_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{   fclose (ptf_in_audio_ap_rx_data);
    return 1u;
}

/* --------------------------------------------------------------------------------------- */
uint8_t audio_ap_rx_set_stream (uint32_t *setting, uint8_t *data, uint32_t size) 
{ 

#define FILE_IN "..\\TestPattern.txt"

    if (NULL == (ptf_in_audio_ap_rx_data = fopen(FILE_IN, "rt")))
//    if (NULL == (ptf_in_audio_ap_rx_data = fopen(FILE_IN, "rb")))
    {   exit (-1);
    }
    return 1u;
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
                    platform_al(PLATFORM_IO_DATA, XDM buffer, 0u, 0u);  translated to :
                        io_start(settings, buffer->address, buffer->size);
  @remark       
 */
/* --------------------------------------------------------------------------------------- */
uint8_t audio_render_start_data_move (uint32_t *setting, uint8_t *data, uint32_t size) 
{   
    audio_render_transfer_done ((uint8_t *)data, size);
    
    fwrite(data, 1, size, ptf_in_audio_render_data);
    fflush(ptf_in_audio_render_data);

    return 1u; 
}

/* --------------------------------------------------------------------------------------- */
uint8_t audio_render_stop_stream(uint32_t *setting, uint8_t *data, uint32_t size) 
{   fclose (ptf_in_audio_render_data);
    return 1u;
}

/* --------------------------------------------------------------------------------------- */
uint8_t audio_render_set_stream (uint32_t *setting, uint8_t *data, uint32_t size)
{   uint8_t index_frame_size = 0;
#define FILE_OUT "..\\audio_out.raw"
    if (NULL == (ptf_in_audio_render_data = fopen(FILE_OUT, "wb")))
    {   exit (-1);
    }

    /* simulate IO commander port with a fixed frame size */
    frame_size_audio_render = extract_sensor_field 
        (platform_audio_out_bit_fields, audio_render_settings, PLATFORM_AUDIO_OUT_FRAMESIZE, index_frame_size);

    return 1u;
}

/*
 * -----------------------------------------------------------------------
 */
   



