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


#ifndef cSTREAM_TOOL_H
#define cSTREAM_TOOL_H

#include "stream_tool_include.h"

/* ===================================================================================================== */

//;--------------------------------------------------------------------------OPTIONS SYNTAX --------------------------
//;   options sets : { index  list } { index  list } 
//;
//;   when the list has one single element "X", this is the value to consider : {X} <=> {1 X} <=> X
//;
//;   when index == 0 it means "any", the list can be empty, the default value is not changed from reset   
//;
//;   when index > 0 the list gives the allowed values the scheduler can select
//;       The Index tells the default "value" to take at reset time and to put in the graph 
//;           the combination of index give the second word of stream_format_io[]
//;        At reset : (*io_func)(STREAM_RESET, (uint8_t *)&stream_format_io_setting, 0);
//;        For tuning : (*io_func)(STREAM_SET_IO_CONFIG, (uint8_t *)&stream_format_io_setting, 0);
//;        Example 2  5 6 7 8 9    ; index 2, default = 6 (index starts at 1)
//;        
//;
//;   when index < 0 a list of triplets follows to describe a combination of data intervals :  A1 B1 C1  A2 B2 C2 ... 
//;       A is starting value, B is the increment step, C is the included maximum value 
//;       The absolute index value selects the default value in this range

struct options
{
    /* default_index = 0 means "any" in the data allowed following values   A1 A2 A3 .. */
    /* default_index = -1 means data range followed by three values   Amin Astep Amax */

    uint32_t nb_option;         // number of "sets" (list of enums or  list of ranges)

    int32_t default_index[MAX_NBOPTIONS_SETS]; 

    float optionRange[MAX_NBOPTIONS_SETS][3];               /* Min, step, Max  (A,B,C) */

    int32_t nbElementsInList[MAX_NBOPTIONS_SETS]; 
    float optionList[MAX_NBOPTIONS_SETS][MAX_NBOPTIONS];    /* list of options offered through SW settings and stream converters */
};


struct stream_interfaces_motion_specific
{   /* 
    * digital format specifics 
    */
        int channel_format;                     /* enum imu_channel_format */
        uint32_t samplingRate_format;           /* sampling unit (0:Hz, 1:seconds, 2:days)  */
        struct options samplingRate;
    /*
    * mixed-signal data specifics
    */
        struct options acc_sensitivity, acc_averaging;
        struct options acc_scaling;             /* factor to apply to reach 1g from RFC8428 */

        struct options gyro_sensitivity, gyro_averaging;
        float gyro_scaling;                     /* factor to apply to reach 1dps */

        struct options mag_sensitivity, mag_averaging;
        float mag_scaling;                      /* factor to apply to reach 1Gauss */
        float   setupTime;                      /* [ms] to avoid this information to being only in the BSP */
};


struct stream_interfaces_audio_specific
{   /*
    * digital format specifics
    */
        /*  Front Left                FL     bit0
            Front Right               FR        1
            Front Center              FC        2
            Low Frequency             LFE       3
            Back Left                 BL        4
            Back Right                BR        5
            Front Left of Center      FLC       6
            Front Right of Center     FRC       7
            Back Center               BC        8
            Side Left                 SL        9
            Side Right                SR        10
            Top Center                TC        11
            Front Left Height         TFL       12
            Front Center Height       TFC       13
            Front Right Height        TFR       14
            Rear Left Height          TBL       15
            Rear Center Height        TBC       16
            Rear Right Height         TBR       17
        */
        long bitFieldChannel;           /* bit0/LSB = front left, bit1 = front right, .. */

    /*
    * mixed-signal data specifics
    */
        /* mixed-signal settings examples : 
            V Vrms;                          RFC8428 units per channel (for information)
            0 default analog gain index; 0; 12; 24;   gain in dB
            0 default digital gain index; 0; -3; 3;   gain in dB
            0 default agc control index; 0; 1;        no AGC
            0 default DCfilter set; 0; 20; 100; 300;  high - pass filter in[Hz]
            1.414;                           scaling to RFC8428 units 1.414Vpeak = 1Vrms
        */

        struct options analog_gain, digital_gain, AGC, DC_filter;
        float audio_scaling;   /* factor to apply to reach 90dBSPL */
        float   setupTime;     /* [ms] to avoid this information to being only in the BSP */
};


struct stream_interfaces_metadata_specific
{   /*
     * digital format specifics
     */
    /*
    * mixed-signal data specifics
    */
    int i;  

    /* Environmental Sensing Service, Bluetooth Service Specification
      0x00: Unspecified 0x01: Air 0x02: Water 0x03: Barometric 0x04: Soil 0x05: Infrared 
      0x06: Map Database 0x07: Barometric Elevation Source 0x08: GPS only Elevation Source 
      0x09: GPS and Map database Elevation Source 0x0A: Vertical datum Elevation Source 
      0x0B: Onshore 0x0C: Onboard vessel or vehicle 0x0D: Front 0x0E: Back/Rear 0x0F: Upper 
      0x10: Lower 0x11: Primary 0x12: Secondary 0x13: Outdoor 0x14: Indoor 0x15: Top 0x16: Bottom 
      0x17: Main 0x18: Backup 0x19: Auxiliary 0x1A: Supplementary 0x1B: Inside 0x1C: Outside 
      0x1D: Left 0x1E: Right 0x1F: Internal 0x20: External 0x21: Solar 0x22-0xFF:Reserved for future use
    */
};


struct stream_interfaces_2d_in_specific
{   /*
     * digital format specifics
     */

    /*
    * mixed-signal data specifics
    */
    int wdr;                // wide dynamic range flag (tuya)
    int watermark;          // watermark insertion flag (tuya)
    int flip;               // image format (portrait, panoramic)
    int night_mode;         // motion detection sensitivity (low, medium, high)
    int detection_zones;    // + {center pixel (in %) radius}, {}, {}
    int time_stamp;         // detection time-stamp format 
    int light_detection;    // 
    int sound_detection;    // sound level 
    int sensors;            // humidity, battery% 
};


struct stream_interfaces_av_codec_specific
{   /*
     * digital format specifics
     */
     int i;
};

/* ===================================================================================================== */
/* 
    digital descriptions
*/


struct node_memory_bank
{
    /* Memory Size =  A(32b) + 
            B x nb_channels_arc(i) + C x samplingHz_arc(j) + D x frame_size_arc(k) 
            + E x maxParameterSize 
    */

    intPtr_t size0;             /* A */
    uint32_t sizeFromGraph;     /* E */
    uint32_t sizeNchan;
    float sizeFS;
    uint32_t sizeFrame, sizeParameter;
    uint32_t iarcChannelI, iarcSamplingJ, iarcFrameK;
    uint32_t alignmentBytes;    /* enum buffer_alignment_type in NUMBER OF BYTES */
    uint32_t stat0work1ret2;    /* enum mem_mapping_type : static, working, pseudo working, backup */
    uint32_t speed;             /* enum mem_speed_type */
    uint32_t relocatable;       /* enum buffer_relocation_type */
    uint32_t data0prog1;        /* data / program memory */
    uint32_t toSwap, swapVID;   /* @@@@ memory bank to swap to swapVID */

    uint32_t mem_VID;           /* optimization during graphTXT node declaration */

    intPtr_t graph_memreq_size; /* computed after graph reading */
    uint32_t graph_base27b;     /* memory base address in 27 bits pack format (offsetID + offset) */
};
typedef struct node_memory_bank node_memory_bank_t;

struct processor_memory_bank
{
    uint32_t offsetID;          /* long_offset64 index addressing the range of the memory bank */
    uint32_t virtualID;         /* used to map memory section in the graph */
    uint32_t speed;             /* mem_speed_type 0:best effort, 1:normal, 2:fast, 3:critical fast */
    uint32_t stat0work1ret2;    /* mem_mapping_type 0:static 1:working(shared with the application) 3:retention*/
    uint32_t private_ram;       /* is it a private (processor ID) or shareable memory bank (= 0) */
    uint32_t hwio;              /* is it reserved for IO and DMA data moves */
    uint32_t data0prog1;        /* data / program memory */
    intPtr_t size;              /* which size is guaranteed for graph operations */
    intPtr_t base64;            /* which offset is it from the offset64b long integer */


    intPtr_t ptalloc_static;    /* starting from 0 to size, working area follows */
    intPtr_t max_working;       /* during init: max size of memreq-working */
    intPtr_t max_working_alignement; /* during init: worst-case of memory alignement */
    intPtr_t max_working_booking;

    char comments[MAXCHAR_NAME]; /* comments used when printing the memory map listing */
};
typedef struct processor_memory_bank processor_memory_bank_t;

/* ===================================================================================================== */
struct formatStruct 
{
    uint32_t idx_arc_in_graph;          /* "format_new new" index field , versus the real index after format merge @@@@ */

    /* FMT0 */
    uint32_t raw_data;          /* FMT1    raw data format */
    uint32_t deinterleaved;     /* FMT0    interleaving */
    uint32_t domain;            /* FMT1    IO_DOMAIN */ 
    uint32_t physical_type;     /* FMT1    stream_unit_physical */
    uint32_t nchan;                     /*  default 1 */

#define FORMAT_MAXNB_DOMAIN_SPECIFIC_FIELDS 10
    float percentFSaccuracy;            /* used for the error on the sampling rate, in percentage */
    float samplingRate;
    uint32_t timestamp;         /* FMT1                 */
    uint32_t timeformat;        /* FMT1                 */
    float frame_length;         /* FMT0    frame length format (0:in milliseconds 1:in samples) */
    uint32_t framelength_format;   /* frame length format (0:in milliseconds 1:in samples) */

    /* format section specific to each domain */
    union
    {   struct stream_interfaces_motion_specific imu;
        struct stream_interfaces_audio_specific audio;
        struct stream_interfaces_2d_in_specific image;
        struct stream_interfaces_metadata_specific metadata;
    } U;
};

typedef struct formatStruct formatStruct_t;


/* ----------------------------------------------------------------------
    ARC description, used for nodes and HW IO streams 
* ---------------------------------------------------------------------- */
struct arcStruct
{
    /* IO arcs */
    //uint32_t top_graph_index;         // index used during the declarion of IO-arcs, allows easy redirection to other IOs  
    uint32_t rx0tx1;                    // ARC0_LW1 and RX0TX1_IOFMT : direction rx=0 tx=1 parameter=2 (rx never consumed) 
    uint8_t  ioarc_flag;                // boolean this arc connects to IOs 
    float    setupTime;                 // [ms] to avoid this information to being only in the BSP 
    uint32_t settings;                  // pack format of digital + MSIC options (SETTINGS_IOFMT2), the format depends on the IO domain 
    uint32_t idx_arc_in_graph;          // "stream_io new" index field 
    uint32_t domain;                    // domain of operation
    struct formatStruct format;         // stream_format 
    uint32_t fw_io_idx;                 // ID of the interface given in "files_manifests_computer" associated function (platform dependent) 
    uint32_t format_idx;                // stream format index
    uint32_t initialized_from_platform; // manifest IO data from platform have been copied
    char IO_name[MAXCHAR_NAME];         // IO stream name used in the GUI
    char domainName[MAXCHAR_NAME];      // arc name used in the GUI 

    uint32_t set0_copy1;                // SET0COPY1_IOFMT data move through pointer setting of data copy 
    uint32_t commander0_servant1;       // SERVANT1_IOFMT selection for polling protocol 
    uint32_t graphalloc_X_bsp_0;        // buffer declared in BSP (0) or Graph x (multiplication factor of the Frame size, 2 for a ping-pong buffer for example 
    uint32_t sram0_hwdmaram1;           // buffer in standard RAM=0, in HW IO RAM=1 
    uint32_t processorBitFieldAffinity; // indexes of the processor in charge of this stream 
    uint32_t clockDomain;               // indication for the need of ASRC insertion 
    uint32_t flow_error;                // #1 do something depending on domain when a flow error occurs, default #0 (no interpolation)
    uint32_t debug_cmd;                 // debug action "ARC_INCREMENT_REG", default = #0 (no debug)
    uint32_t debug_reg;                 // index of the 64bits result, default = #0
    uint32_t debug_page;                // debug registers base address + 64bits x 16 registers = 32 word32 / page, default = #0
    uint32_t flush;                     // control of register "MPFLUSH_ARCW1" : forced flush of data in MProcessing and shared tasks
    uint32_t extend_addr;               // address range extension-mode of the arc descriptor "EXTEND_ARCW2" for large NN models, default = #0 (no extension)
    uint32_t map_hwblock;               // mapping VID index from "procmap_manifest_xxxx.txt" to map the buffer, default = #0 (VID0)
    uint32_t jitter_ctrl;               // factor to apply to the minimum size between the producer and the consumer, default = 1.0 (no jitter)
    uint32_t SwcProdGraphIdx, fmtProd;  // index to the producer node graph->all_nodes[IDX], index format of the producer
    uint32_t SwcConsGraphIdx, fmtCons;  // index to the consumer node graph->all_nodes[IDX], index format of the consumer

    /* ----- from platform manifest data ----- */
    struct options raw_format_options;
    struct options nb_channels_option;    
    struct options sampling_rate_option;
    struct options sampling_period_s;
    struct options sampling_period_day;
    struct options frame_length_option;
    struct options frame_duration_option;
    float sampling_accuracy;

    /* ----- graph data ----- */    
    uint32_t memVID;
    uint32_t graph_base27b;

    /* ------ COMPILATION RESULT-- */
    //uint32_t ioarc;                    /* arc index used in the graph description */
    //uint32_t ioarc_buffer;
    uint32_t arcID;                      /* arc index used in the graph description */
    float sizeFactor;

    // specific to SWC
    uint32_t format_idx_src, format_idx_dst; // stream format index
    uint32_t inPlaceProcessing;          /* SWCONLY flag buffer overlay with another arcID, 0=none*/
    uint32_t arcIDbufferOverlay;         /* SWCONLY   arcID being overlaid */
    uint32_t src_instanceid, dst_instanceid; /* for information during graph reading */
    //uint32_t platform_swc_idx_src, platform_swc_idx_dst;    /* SWC */
};

/*----------------------------------------------------------------------------------------------------*/


/* ----------------------------------------------------------------------
    NODES
* ---------------------------------------------------------------------- */
struct stream_node_manifest
{
    char developerName[MAXCHAR_NAME];   /* developer's name */
    char nodeName[MAXCHAR_NAME];        /* node name used in the GUI */
       
    uint32_t nbInputArc, nbOutputArc, nbParamArc;
    uint32_t nbArch, arch[MAXNBARCH];   /* stream_processor_architectures (max 256) */
    uint32_t fpu[MAXNBARCH];            /* stream_processor_sub_arch_fpu */
    uint32_t swc_assigned_arch, swc_assigned_proc, swc_verbose, swc_assigned_priority;
    uint32_t RWinSWC;                   /* XDM11 read/write index is managed in SWC, for variable buffer consumption */
    uint32_t formatUsed;                /* buffer format is used by the component */
    uint32_t masklib;                   /* mask of up to 16 family of processing extensions "SERVICE_COMMAND_GROUP" */
    uint32_t codeVersion, schedulerVersion;  /* version of the scheduler it is compatible with */
    uint32_t deliveryMode;               /* 0:source, 1:binary 2:2 binaries (fat binary)*/

    uint32_t arc_parameter;             // SWC with extra-large amount of parameters (NN models) will declare it with extra arcs
    uint32_t steady_stream;             // (0) the data flow is variable (or constant, default value :1) on all input and output arcs
    uint32_t same_data_rate;            // (0) the arcs have different data rates, (1) all arcs have the same data rate 
    uint32_t use_dtcm;                  // default 0 (no MP DTCM_LW2), 1: fast memory pointer placed after the arc format
    uint32_t use_arc_format;            // default 0 : the scheduler must push each arc format (LOADFMT_LW0_LSB)
    uint32_t mask_library;              // default 0 bit-field of dependencies to computing libraries
    uint32_t subtype_units;             // triggers the need for rescaling and data conversion
    uint32_t architecture;              // arch compatible with (default: 0 = source code) to merge and sort for ARCHID_LW0
    uint32_t fpu_used;                  // fpu option used (default 0: none, no FPU assembly or intrinsic)
    uint32_t node_version;              // version of the computing node
    uint32_t stream_version;            // version of the stream scheduler it is compatible with
    uint32_t graph_instance;            // occurence of the same node
    
    /* to build the header */
    uint32_t platform_swc_idx; 
    uint32_t arcsrdy;                   
    uint32_t narc_streamed;
    uint32_t locking_arc;               /* from the SWC in ROM */    
    uint32_t lock_proc;
    uint32_t lock_arch;                 
    uint32_t headerPACK;

    /* memory */
    uint32_t nbMemorySegment;
    node_memory_bank_t memreq[MAX_NB_MEM_REQ_PER_NODE]; /* memory allocations request */
    intPtr_t sum_of_working_simplemap;   /* cumulate the multiple working segment memory requirements */
    processor_memory_bank_t usedmem[MAX_PROC_MEMBANK];

    /* parameters */
    uint32_t preset;                     /* default "preset value" */
    uint32_t TAG;                     
    uint32_t trace_ID;                     
    uint32_t local_script_index;
    uint32_t ParameterSizeW32;          /* size of the array below */
    uint32_t PackedParameters[MAXPARAMETERSIZE]; /* words32 PACK */
    float malloc_E; 

    /* arcs and buffers */
    uint32_t initialized_from_platform;
    uint32_t inPlaceProcessing, arcIDbufferOverlay;
    //uint16_t nb_arcs;
    struct arcStruct arc[MAX_NB_STREAM_PER_SWC]; /* loaded with default data */
};

typedef struct stream_node_manifest stream_node_manifest_t;


/* ----------------------------------------------------------------------
    PROCESSOR
* ---------------------------------------------------------------------- */
struct processing_architecture
{   
    uint32_t processorID;
    uint32_t IamTheMainProcessor;
    intPtr_t architecture;
    intPtr_t libraries_b;   /* bit-field of embedded libraries in the flash of the device */

    uint32_t nb_threads;    /* RTOS threads (0..3) have decreasing priority (see PRIORITY_LW0) */
    uint32_t nb_long_offset;
};
typedef struct processing_architecture processing_architecture_t;

/* ----------------------------------------------------------------------
    PLATFORM = PROCESSOR + IO + NODES
* ---------------------------------------------------------------------- */
struct stream_script 
{
    #define AVG_SCRIPT_LEN 80
    char script_bytecode[AVG_SCRIPT_LEN];
    uint32_t script_instance27b, nbw32_allocated;           // static address, size
    uint32_t stack_memory_shared;                           // flag
    uint32_t script_nb_byte_code, nb_reg, nb_ptr, nb_stack; // memory allocation parameters
    uint32_t stack_memory_script, mem_VID;
    uint32_t script_offset;
    uint32_t script_format;     /* 0 : interpreted byte code, or architecture native code */
};

typedef struct stream_script stream_script_t;

/* ----------------------------------------------------------------------
    PLATFORM = PROCESSOR + IO + NODES
* ---------------------------------------------------------------------- */
struct stream_platform_manifest
{   
    uint32_t nb_nodes;                   /* number of software components to build the graph */
    uint32_t nb_processors;              /* number of processors */
    uint32_t nb_architectures;           /* number of architecture */
    processing_architecture_t processor[MAX_GRAPH_NB_PROCESSORS];

    uint32_t nbOffset;                  /* Nb Base addresses :  MAX_NB_MEMORY_OFFSET
                                            static uint32_t MEXT[SIZE_MBANK_DMEM_EXT];
                                            static uint32_t TCM1[SIZE_MBANK_DMEMFAST]; 
                                            const intPtr_t long_offset[MAX_NB_MEMORY_OFFSET] = 
                                            {   (const intPtr_t) &(MEXT[0]),
                                                (const intPtr_t) &(TCM1[0]),
            */
    uint32_t nbMemoryBank_detailed;
    processor_memory_bank_t membank[MAX_PROC_MEMBANK];

    uint32_t nb_hwio_stream;
    struct arcStruct arc[MAX_GRAPH_NB_IO_STREAM];

    struct stream_node_manifest all_nodes[MAX_NB_NODES];    // SWC_IDX + memreq

    /* desired platform domains mapping to platform capabilities (fw_io_idx) */
    int domains_to_fw_io_idx[IO_DOMAIN_MAX_NB_DOMAINS];
};

typedef struct stream_platform_manifest stream_platform_manifest_t;


struct dbgtrace
{
    uint32_t address;
    char toprint[NBCHAR_LINE];
};
typedef struct dbgtrace dbgtrace_t;

/* ----------------------------------------------------------------------
    GRAPH = FORMATS + ARCS + NODES + SCRIPTS
* ---------------------------------------------------------------------- */
struct stream_graph_linkedlist
{   
    uint32_t subg_depth;                            
    char mangling[SUBGMAXDEPTH][MAXCHAR_NAME];     /* name of subgraph, to concatenate with local labels */
    char toConcatenate[SUBGMAXDEPTH * MAXCHAR_NAME];
    uint32_t arcIO_redirection[SUBGMAXDEPTH][MAX_NB_IO]; /* 
    
    /* ------- HEADER ------- */                    /* graph[4] bit-field of allowed processors */
    uint32_t procid_allowed_gr4;                    /* position of the graph words going to RAM */  
    uint32_t offsetToRam;                           /* RAMSPLIT_GRAPH0 = COPY_CONF_GRAPH0_COPY_ALL_IN_RAM /FROM_LINKEDLIST /FROM_STREAM_INST ..*/  
    uint32_t PackedFormat;                          /*  consolidated formats per arc, inter-nodes and at boundaries */  
                                                    /* format section common to all stream interfaces (digital format) */  
    /* ------- FORMAT ------ */
    uint32_t nb_formats;
    struct formatStruct arcFormat[MAX_NB_FORMAT];   /* merged formats */

    /* ------- ARCS ------- */
    struct arcStruct arc[MAX_NB_NODES];             /* rx0tx1, domain, digital format and FS accuracy => merge data */
    uint16_t nb_arcs, nb_io_arcs;                   /*  consolidated formats per arc, inter-nodes and at boundaries */
    uint32_t nb_debug_pages;                        /* arcs self-debug : number of 16x64b debug pages, in the buffer of arc0 */ 
    uint32_t nb_debug_registers;                    /* arcs self-debug : number of 16x64b debug registers, in the buffer of arc0 */ 
    
    /* ------- NODES ------- */
    struct stream_node_manifest all_nodes[MAX_NB_NODES]; /* share the same type as manifests => load arc data */
    uint16_t nb_nodes;                              /* nodes + parameters + computed memory allocations */

    /* ------- SCRIPTS ------- */
    uint32_t nb_scripts, idx_script, nb_scriptsARC; /* 128 common scripts, others are in the parameter section of arm_stream_script() */
    uint32_t  max_shared_stackW32;                  // nb of arcs, including the shared one
    struct stream_script all_scripts[1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    //uint32_t all_scripts_byte_codes[50 * (1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1))];

    /* ------- OUTPUT ------- */
    uint32_t end_binary_graph, memVID;              /* compilation result, location of the graph in memory */
    uint32_t option_graph_location;
    uint32_t binary_graph[MAXBINARYGRAPHW32];   
    char  binary_graph_comments[MAXBINARYGRAPHW32][NBCHAR_LINE];
    uint32_t sched_return, dbg_script_fields; 
    uint32_t idbg;
    dbgtrace_t dbg[MAXDBGTRACEGRAPH];

    FILE *ptf_graph_bin;/* graph conversion  */
    FILE *ptf_debug;    /* comments during graph compilation */
    FILE *ptf_header;   /* list of labels to do "set_parameter" from scripts (script compilation step)
                           address of NODEs on the binary graph, location of debug result of ARCs  */
};

#endif /* #ifndef cSTREAM_TOOL_H */
#ifdef __cplusplus
}
#endif
