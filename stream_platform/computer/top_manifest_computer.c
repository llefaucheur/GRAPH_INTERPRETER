/* ----------------------------------------------------------------------
 * top_manifest_computer.c.c
 * -------------------------------------------------------------------- */

#include "presets.h"
#ifndef top_manifest_computerPc
#define top_manifest_computerPc
#ifdef PLATFORM_COMPUTER
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "presets.h"

#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      
#include "stream_types.h"

#include "top_manifest_computer.h"

/*------------------------------------------------------------------------------------
    ALL THE NODES OF ALL PROCESSORS, AND THEIR INDEX 
-------------------------------------------------------------------------------------*/
extern p_stream_node arm_stream_null_task;       /*    */
extern p_stream_node arm_stream_script;          /*  1  #define arm_stream_script_index 1 */
extern p_stream_node arm_stream_router;          /*  2 */
extern p_stream_node arm_stream_modulator;       /*  3 */
extern p_stream_node arm_stream_fixedbf;         /*  4 */
extern p_stream_node arm_stream_filter2D;        /*  5 */
extern p_stream_node arm_stream_filter;          /*  6 */
extern p_stream_node arm_stream_demodulator;     /*  7 */
extern p_stream_node arm_stream_anr;             /*  8 */
extern p_stream_node arm_stream_amplifier;       /*  9 */
extern p_stream_node sigp_kws;                   /* 10 */
extern p_stream_node sigp_stream_compressor;     /* 11 */
extern p_stream_node sigp_stream_decompressor;   /* 12 */
extern p_stream_node sigp_stream_detector;       /* 13 */
extern p_stream_node sigp_stream_detector2D;     /* 14 */
extern p_stream_node sigp_stream_resampler;      /* 15 */
extern p_stream_node bitbank_JPEGENC;            /* 16 */
extern p_stream_node TjpgDec;                    /* 17 */

/*------------------------------------------------------------------------------------
    NODES OF PROCESSOR 1 (HE)
-------------------------------------------------------------------------------------*/
#if PLATFORM_PROCESSOR == 1
const p_stream_node node_entry_points[NB_NODE_ENTRY_POINTS] =
{
    /*  0 */ (p_stream_node)&arm_stream_null_task,      /* ID | PROC ARCH    */
    /*  1 */ (p_stream_node)&arm_stream_script,         /*  1 |   0   0      byte-code interpreter, index "arm_stream_script_INDEX" */
    /*  2 */ (p_stream_node)&arm_stream_router,         /*  2 |   0   1      copy input arcs and subchannel and output arcs and subchannels   */
    /*  3 */ (p_stream_node)&arm_stream_modulator,      /*  3 |   0   1      signal generator with modulation */
    /*  4 */ (p_stream_node)&arm_stream_fixedbf,        /*  4 |   0   1
    /*  5 */ 0,                                         /*  5 |   0   1      2D processing on the HP processor PROC_ID=2 */
    /*  6 */ (p_stream_node)&arm_stream_filter,         /*  6 |   0   1      cascade of DF1 filters */
    /*  7 */ (p_stream_node)&arm_stream_demodulator,    /*  7 |   0   1      signal demodulator, frequency estimator */
    /*  8 */ (p_stream_node)&arm_stream_anr,            /*  8 |   0   1
    /*  9 */ (p_stream_node)&arm_stream_amplifier,      /*  9 |   0   1      amplifier mute and un-mute with ramp and delay control */
    /* 10 */ (p_stream_node)&sigp_kws,                  /* 10 |   0   1      YES/NO KWS */
    /* 11 */ (p_stream_node)&sigp_stream_compressor,    /* 11 |   0   1      raw data compression with adaptive prediction */
    /* 12 */ (p_stream_node)&sigp_stream_decompressor,  /* 12 |   0   1      raw data decompression */
    /* 13 */ (p_stream_node)&sigp_stream_detector,      /* 13 |   0   1      estimates peaks/floor of the mono input and triggers a flag on high SNR */
    /* 14 */ 0,                                         /* 14 |   0   1      2D processing on the HP processor PROC_ID=2 */
    /* 15 */ (p_stream_node)&sigp_stream_resampler,     /* 15 |   0   1      asynchronous sample-rate converter */
    /* 16 */ (p_stream_node)&bitbank_JPEGENC,           /* 16 |   0   1      bitbank_JPEGENC */
    /* 17 */ (p_stream_node)&TjpgDec,                   /* 17 |   0   1      TjpgDec */
};
#endif

/*------------------------------------------------------------------------------------
    NODES OF PROCESSOR 2 (HP)
-------------------------------------------------------------------------------------*/
#if PLATFORM_PROCESSOR == 2
const p_stream_node node_entry_points[NB_NODE_ENTRY_POINTS] =
{
    /*  0 */ (p_stream_node)&arm_stream_null_task,      /* ID | PROC ARCH    */
    /*  1 */ (p_stream_node)&arm_stream_script,         /*  1 |   0   0      byte-code interpreter, index "arm_stream_script_INDEX" */
    /*  2 */ (p_stream_node)&arm_stream_router,         /*  2 |   0   1      copy input arcs and subchannel and output arcs and subchannels   */
    /*  3 */ (p_stream_node)&arm_stream_modulator,      /*  3 |   0   1      signal generator with modulation */
    /*  4 */ (p_stream_node)&arm_stream_kws,            /*  4 |   0   1
    /*  5 */ (p_stream_node)&arm_stream_fixedbf,        /*  5 |   0   1
    /*  6 */ (p_stream_node)arm_stream_filter2D,        /*  6 |   0   1  2D processing on the HP processor */
    /*  7 */ (p_stream_node)&arm_stream_filter,         /*  7 |   0   1      cascade of DF1 filters */
    /*  8 */ (p_stream_node)&arm_stream_demodulator,    /*  8 |   0   1      signal demodulator, frequency estimator */
    /*  9 */ (p_stream_node)&arm_stream_anr,            /*  9 |   0   1
    /* 10 */ (p_stream_node)&arm_stream_amplifier,      /* 10 |   0   1      amplifier mute and un-mute with ramp and delay control */
    /* 11 */ (p_stream_node)&sigp_stream_compressor,    /* 11 |   0   1      raw data compression with adaptive prediction */
    /* 12 */ (p_stream_node)&sigp_stream_decompressor,  /* 12 |   0   1      raw data decompression */
    /* 13 */ (p_stream_node)&sigp_stream_detector,      /* 13 |   0   1      estimates peaks/floor of the mono input and triggers a flag on high SNR */
    /* 14 */ (p_stream_node)&sigp_stream_detector2D,    /* 14 |   0   2  activity detection, pattern detection */
    /* 15 */ (p_stream_node)&sigp_stream_resampler,     /* 15 |   0   1      asynchronous sample-rate converter */
    /* 16 */ (p_stream_node)&bitbank_JPEGENC,           /* 16 |   0   1      bitbank_JPEGENC */
    /* 17 */ (p_stream_node)&TjpgDec,                   /* 17 |   0   1      TjpgDec */
};
#endif


extern void data_sink     (uint32_t, stream_xdmbuffer_t *);  // 0
extern void data_in_1     (uint32_t, stream_xdmbuffer_t *);  // 1
extern void sensor_in_0   (uint32_t, stream_xdmbuffer_t *);  // 2
extern void motion_in_0   (uint32_t, stream_xdmbuffer_t *);  // 3
extern void audio_in_0    (uint32_t, stream_xdmbuffer_t *);  // 4 
extern void sensor_2d_in_0(uint32_t, stream_xdmbuffer_t *);  // 5
extern void line_out_0    (uint32_t, stream_xdmbuffer_t *);  // 6 
extern void gpio_out_0    (uint32_t, stream_xdmbuffer_t *);  // 7
extern void gpio_out_1    (uint32_t, stream_xdmbuffer_t *);  // 8
extern void data_out_0    (uint32_t, stream_xdmbuffer_t *);  // 9 

/*  pointer to the current graph interpreter instance  */
arm_stream_instance_t* platform_io_callback_parameter;




#if PLATFORM_PROCESSOR == 1
/* ------------------------------------------------
                TOP_MANIFEST_computer.TXT
  ------------------------------------------------ */
uint8_t MEXT[SIZE_MBANK_DMEM_EXT];  
uint8_t DTCM[SIZE_MBANK_DTCM];
uint8_t ITCM[SIZE_MBANK_ITCM];
uint8_t BACKUP[SIZE_MBANK_RETENTION];

const uint8_t* long_offset[MAX_NB_MEMORY_OFFSET] =
{ &(MEXT[0]), &(DTCM[0]), &(ITCM[0]), &(BACKUP[0]) };

/*---------------------------------------------------------
  IO AFFINITY WITH PROCESSOR 1 = ALL IOs EXCEPT THE CAMERA  
  ---------------------------------------------------------*/
#define LAST_IO_FUNCTION_PLATFORM (IO_PLATFORM_DATA_OUT_0+1)  /* table of platform_io[io_al_idx] */

const p_io_function_ctrl platform_io[] =
{   data_sink     ,     // 0
    data_in_1     ,     // 1
    sensor_in_0   ,     // 2
    motion_in_0   ,     // 3
    audio_in_0    ,     // 4   
    0             ,     // 5  reserved to the HP processor
    line_out_0    ,     // 6   
    gpio_out_0    ,     // 7
    gpio_out_1    ,     // 8
    data_out_0    ,     // 9   
};
#endif


#if PLATFORM_PROCESSOR == 2
/*  ;-----IO AFFINITY WITH PROCESSOR 2-------------------------------
    ;Path      Manifest       IO_AL_idx    Comments  (frame size in Bytes)
    1 io_computer_2d_in_0.txt         5    camera
*/
const p_io_function_ctrl platform_io[] =
{   0 ,                 // 0
    0 ,                 // 1
    0 ,                 // 2
    0 ,                 // 3
    0 ,                 // 4     
    sensor_2d_in_0,     // 5
    0  ,                // 6 
    0  ,                // 7
    0  ,                // 8
    0  ,                // 9 
};
#endif



/*
    Callback for NODE and scripts (SYSCALL)  system call (0..7) and application callbacks (8..15))
        and ARC debug activities (ARC_APP_CALLBACK1)
    Use-case :
        deep-sleep proposal from the scheduler
        event detection trigger, software timers
        metadata sharing from script
        allow execution of arm_graph_interpreter(STREAM_RESET..) after a graph remote reload (check IOs are ok)
*/
const p_stream_services application_callbacks[MAX_NB_APP_CALLBACKS] =
{   (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0
};

/*==========================================================================================================================*/

extern const uint8_t platform_audio_out_bit_fields[];


/*
    this table will be extended with pointers to nodes loaded
    dynamically and compiled with position independent execution options

    it is aligned with the list of SOFTWARE COMPONENTS MANIFESTS in "stream_platform\computer\manifest\top_manifest_computer.txt"
*/
/* -----------------------------------
    Full node descriptions given in ./stream_tools/TEMPLATE_GRAPH.txt
*/


/*
*   the graph to be executed
*/
const uint32_t graph_computer_audio[] =
{
#include "./graphs/graph_computer_filter_detector_bin.txt"
};



/*
 *  TIME (see stream_time64 definition)
 *
 *  "stream_time64" example of implementation using a global variable
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s]  140 Y + Q28 [s]
 *  systick increment for  1ms =  0x00041893 =  1ms x 2^28
 *  systick increment for 10ms =  0x0028F5C2 = 10ms x 2^28
 *
 * Other implementation rely on an HW timer (RP2040)
*/

uint64_t global_stream_time64;


/*
    Table of pairs {node + parameter address} for parameter updates from the application
*/
uintptr_t new_node_parameters[(1 + MAX_NB_PENDING_PARAM_UPDATES) * 2] =
{ 0, 0,  // [node idx in the graph ; physical address to the parameters in "boot" format]
    0, 0,  // [node idx; parameter address]
    // .. 
    0, 0,  // end of the list 
};


#define GRAPH_SDS_DEMO_AUDIO 0

uint32_t* get_graph_address(uint32_t graph_idx)
{
    if (graph_idx == GRAPH_SDS_DEMO_AUDIO)
    {
        return (uint32_t*)graph_computer_audio;
    }
    return 0;
}

/**
  @brief            Shares the platform-specific data for the initialization of the interpreter instance
  @param[in/out]    none
  @return           int
 */

void platform_init_specific(arm_stream_init_t* data)
{
#if GRAPH_FROM_PLATFORM == 2
    data->graph = get_graph_address(GRAPH_SDS_DEMO_AUDIO);
#endif

    data->long_offset = (uint8_t**)long_offset;           // pointer to "long_offset[MAX_NB_MEMORY_OFFSET]"

    data->application_callbacks = (p_stream_services)application_callbacks;  // callbacks used by scripts
    data->node_entry_points = (p_stream_node)node_entry_points;             // list of nodes
    data->platform_io = (p_io_function_ctrl)platform_io;                     // list of IO functions
    data->new_parameters = (uintptr_t)new_node_parameters;                   // list of pairs [offset; parameter address]

    data->procID = PLATFORM_PROCESSOR;
    data->archID = PLATFORM_ARCHITECTURE;
}


/* here: test the need for memory recovery/swap
    does the application modified the memory banks used by the graph ? */
void arm_memory_swap(arm_stream_instance_t* S)
{
    static int i;
    i = 1;
    return;
    //uint8_t memBankBitFields[(7 + MAX_PROC_MEMBANK) / 8];
}

/*==========================================================================================================================*/
#endif
#endif  //#ifndef top_manifest_computerPc