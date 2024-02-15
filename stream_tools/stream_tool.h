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


struct options
{
    uint32_t nb_option;
    uint32_t default_index;
    float options[MAX_NBOPTIONS];   /* options offered through SW settings and stream converters */
};


enum imu_channel_format /* uint8_t : data interleaving possible combinations */
{
     aXg0m0 = 1,     /* only accelerometer */
     a0gXm0 = 2,     /* only gyroscope */
     a0g0mX = 3,     /* only magnetometer */
     aXgXm0 = 4,     /* A + G */
     aXg0mX = 5,     /* A + M */
     a0gXmX = 6,     /* G + M */
     aXgXmX = 7,     /* A + G + M */
 };

struct stream_interfaces_motion_specific
{   /* 
    * digital format specifics 
    */
        int channel_format;         /* enum imu_channel_format */

    /*
    * mixed-signal data specifics
    */
        struct options acc_sensitivity, acc_averaging;
        float acc_scaling;                      /* factor to apply to reach 1g from RFC8428 */

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
#define NBCHAR_LINE 200
#define NBCHAR_NAME 120
#define MAX_PROC_MEMBANK 32     /* number of physical memory banks of the processor, for the graph processing */


#define MAXCHAR_NAME 80
#define MAXNBARCH 8
#define MAXNBLIBRARY 16
#define MAXPARAMETERSIZE ((int64_t)1e5)  /* in bytes */

struct node_memory_bank
{
    /* Memory Size = A + B x nb_channels_arc(i) + C x sampling_arc(j) + D x frame_size_arc(k) */
    uint64_t size0, DeltaSize64;
    float sizeNchan, sizeFS, sizeFrame, sizeParameter;
    uint32_t iarcChannelI, iarcSamplingJ, iarcFrameK;
    uint32_t alignmentBytes;      /* enum buffer_alignment_type in NUMBER OF BYTES */
    uint32_t usage;              /* enum mem_mapping_type : static, working, pseudo working, backup */
    uint32_t speed;              /* enum mem_speed_type */
    uint32_t relocatable;        /* enum buffer_relocation_type */

    uint32_t VID;                /* optimization during graphTXT node declaration */

    uint64_t graph_mem_size;      /* computed after graph reading */
    uint32_t graph_basePACK;      /* memory base address in 27 bits pack format (offsetID + offset) */
};
typedef struct node_memory_bank node_memory_bank_t;

struct processor_memory_bank
{
    uint32_t offsetID;       /* long_offset64 index addressing the range of the memory bank */
    uint32_t virtualID;      /* used to map memory section in the graph */
    uint32_t speed;          /* mem_speed_type 0:best effort, 1:normal, 2:fast, 3:critical fast */
    uint32_t working;        /* mem_mapping_type 0:static 1:working(shared with the application) 3:retention*/
    uint32_t private_ram;    /* is it a private or shareable memory bank */
    uint32_t hwio;           /* is it reserved for IO and DMA data moves */
    uint32_t data_access;    /* is it a dataRAM/programRAM/Flash memory bank */
    uint64_t size;           /* which size is guaranteed for graph operations */
    uint64_t base32;         /* which offset is it from the offset64b long integer */

    uint32_t ptalloc_static; /* starting from 0 to size, working area follows */
    uint32_t max_working;    /* during init: max size of memreq-working */
};
typedef struct processor_memory_bank processor_memory_bank_t;

/* ===================================================================================================== */

struct common_io_swc
{
    // common between IOstream and SWC
    uint32_t rx0tx1;                     /* direction rx=0 tx=1 parameter=2 (rx never consumed) */
    uint32_t raw_type;
    uint32_t timestamp;
    uint32_t framelength_format;         /* frame length format (0:in milliseconds 1:in samples) */
    uint32_t samplingRate_format;        /* sampling unit (0:Hz, 1:seconds, 2:days)  */
    float percentFSaccuracy;        /* used for the error on the sampling rate, in percentage */
};

struct specific_io
{
    // specific to IOstream 
    char domainName[MAXCHAR_NAME];      /* arc name used in the GUI */
    uint32_t domain;                    /* decoded domain_name among stream_io_domain [64] */

    uint32_t set0_copy1;                /* data move through pointer setting of data copy */
    uint32_t commander0_servant1;       /* selection for polling protocol */
    uint32_t graphalloc_X_bsp_0;        /* buffer declared in BSP (0) or Graph x (multiplication factor of the Frame size, 2 for a ping-pong buffer for example */
    uint32_t sram0_hwdmaram1;           /* buffer in standard RAM=0, in HW IO RAM=1 */
    uint32_t processorBitFieldAffinity; /* indexes of the processor in charge of this stream */

    uint32_t platform_al_fw_io_idx;     /* associated function (platform dependent) */
    uint8_t ioarc_flag;
    float setupTime;                    /* [ms] to avoid this information to being only in the BSP */
    uint32_t settings;                  /* pack format of digital + MSIC options */
};


struct formatStruct 
{
    uint32_t FMT0, FMT1, FMT2;
    struct common_io_swc sc;
};
typedef struct formatStruct formatStruct_t;


/* 
    arc description, used for nodes and HW IO streams 
*/
struct arcStruct
{
    /* IO arcs */
    char IO_name[MAXCHAR_NAME];             /* IO stream name used in the GUI */
    uint32_t domain;                        /* domain of operation */
    uint32_t nb_arc;                        /* number of streams used by this stream interface */

    uint32_t top_graph_index; 
    uint32_t format_idx;
    int fw_io_idx;
    int arcIDstream;
    
    /* format section specific to each domain */
    union
    {   struct stream_interfaces_motion_specific imu;
        struct stream_interfaces_audio_specific audio;
        struct stream_interfaces_2d_in_specific image;
        struct stream_interfaces_metadata_specific metadata;
    } U;

    /* common with internal arcs */
    uint32_t memVID;
    struct common_io_swc sc;
    struct specific_io si;

    /* 
    *   data filed during the graph compilation
    */
    //uint32_t ioarc;                      /* arc index used in the graph description */
    //uint32_t ioarc_buffer;
    uint32_t arcID;                      /* arc index used in the graph description */
    float sizeFactor;

    // specific to SWC
    uint32_t inPlaceProcessing;          /* SWCONLY flag buffer overlay with another arcID, 0=none*/
    uint32_t arcIDbufferOverlay;         /* SWCONLY   arcID being overlaid */
    uint32_t src_instanceid, dst_instanceid; /* for information during graph reading */
    uint32_t swc_idx_src, swc_idx_dst;   /* SWC */

    /* common digital format information between arcs of nodes and IO */
    struct options interleaving_option; 
    struct options nbchannel_option;    
    struct options frame_size_option;
    struct options sampling_rate_option;

    /* arc descriptor */
    uint32_t ARCW0, ARCW1, ARCW2, ARCW3;
};

/*----------------------------------------------------------------------------------------------------*/


/* ----------------------------------------------------------------------
    NODES
* ---------------------------------------------------------------------- */
struct stream_node_manifest
{
    char developerName[MAXCHAR_NAME];   /* developer's name */
    char nodeName[MAXCHAR_NAME];        /* node name used in the GUI */
    uint32_t instance_idx;                   /* inversion version of theis node */
    uint32_t nbInputArc, nbOutputArc, nbParameArc, idxStreamingArcSync, nbInputOutputArc;
    uint32_t nbArch, arch[MAXNBARCH];        /* stream_processor_architectures (max 256) */
    uint32_t fpu[MAXNBARCH];                 /* stream_processor_sub_arch_fpu */
    uint32_t swc_assigned_arch, swc_assigned_proc, swc_verbose, swc_assigned_priority;
    uint32_t RWinSWC;                        /* XDM11 read/write index is managed in SWC, for variable buffer consumption */
    uint32_t formatUsed;                     /* buffer format is used by the component */
    uint32_t masklib;                        /* mask of up to 16 family of processing extensions "SERVICE_COMMAND_GROUP" */
    uint32_t codeVersion, schedulerVersion;  /* version of the scheduler it is compatible with */
    uint32_t deliveryMode;               /* 0:source, 1:binary 2:2 binaries (fat binary)*/

    /* to build the header */
    uint32_t swc_idx, arcsrdy, narc_streamed, locking_arc, lock_proc, lock_arch;
    uint32_t headerPACK;

    uint32_t nbMemorySegment;
    node_memory_bank_t memreq[MAX_NB_MEM_REQ_PER_NODE]; /* memory allocations request */
    uint64_t sum_of_working_simplemap;   /* cumulate the multiple working segment memory requirements */


    uint32_t preset;                     /* default "preset value" */
    uint32_t local_script_index;
    uint32_t defaultParameterSizeW32;    /* size of the array below */
    uint32_t PackedParameters[MAXPARAMETERSIZE]; /* words32 PACK */

    uint32_t inPlaceProcessing, arcIDbufferOverlay;
    uint32_t nb_arcs;
    struct arcStruct arc[MAX_NB_STREAM_PER_SWC];
};

typedef struct stream_node_manifest stream_node_manifest_t;


/* ----------------------------------------------------------------------
    PROCESSOR
* ---------------------------------------------------------------------- */
struct processing_architecture
{   
    uint32_t processorID;
    uint32_t IamTheMainProcessor;
    uint64_t architecture;
    uint64_t libraries_b;   /* bit-field of embedded libraries in the flash of the device */

    uint32_t nb_threads;    /* RTOS threads (0..3) have decreasing priority (see PRIORITY_LW0) */
    uint32_t nb_long_offset;
    uint32_t nbMemoryBank_detailed;
    processor_memory_bank_t membank[MAX_PROC_MEMBANK];
#define MAX_NB_MEMORY_OFFSET 8
    uint32_t offset_ID[MAX_NB_MEMORY_OFFSET];
    uint64_t offset_base[MAX_NB_MEMORY_OFFSET];
};
typedef struct processing_architecture processing_architecture_t;




/* ----------------------------------------------------------------------
    PLATFORM = PROCESSOR + IO + NODES
* ---------------------------------------------------------------------- */
struct stream_platform_manifest
{   
    uint32_t nb_nodes;                   /* number of software components to build the graph */
    uint32_t nb_processors;              /* number of processors */
    uint32_t nb_architectures;           /* number of architecture */
    processing_architecture_t processor[MAX_GRAPH_NB_PROCESSORS];

    uint32_t nb_hwio_stream;
    struct arcStruct io_stream[MAX_GRAPH_NB_IO_STREAM];

    struct stream_node_manifest all_nodes[MAX_NB_NODES];    // SWC_IDX + memreq

    /* desired platform domains mapping to platform capabilities (fw_io_idx) */
    int domains_to_fw_io_idx[IO_DOMAIN_MAX_NB_DOMAINS];
};

typedef struct stream_platform_manifest stream_platform_manifest_t;


struct dbgtrace
{
    uint32_t address;
    char toprint[MAXNBCHAR_LINE];
};
typedef struct dbgtrace dbgtrace_t;

/* ----------------------------------------------------------------------
    GRAPH = FORMATS + ARCS + NODES + SCRIPTS
* ---------------------------------------------------------------------- */
struct stream_graph_linkedlist
{   
    uint32_t offsetToRam;                           /* position of the graph words going to RAM */
    uint32_t PackedFormat;                          /* RAMSPLIT_GRAPH0 = COPY_CONF_GRAPH0_COPY_ALL_IN_RAM /FROM_LINKEDLIST /FROM_STREAM_INST ..*/
                                                    /*  consolidated formats per arc, inter-nodes and at boundaries */
                                                    /* format section common to all stream interfaces (digital format) */
    struct arcStruct arc[MAX_NB_NODES];             /* rx0tx1, domain, digital format and FS accuracy */
    uint32_t nb_arcs;                               /*  consolidated formats per arc, inter-nodes and at boundaries */
 
    struct arcStruct arcIO[MAX_NB_IO];
    uint32_t nbio_interfaces, nbio_interfaces_with_arcBuffer;

    uint32_t procid_allowed_gr4;                    /* graph[4] bit-field of allowed processors */
    
    struct formatStruct arcFormat[MAX_NB_FORMAT];   /* merged formats */
    uint32_t nb_formats;

    /* script byte-codes */
    uint32_t nb_byte_code;
    uint32_t nb_scripts, nb_scriptsARC, atleastOneScriptShared; 
    uint32_t scriptRAMshared  [1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    uint32_t script_offset    [1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    uint32_t script_stackLengthW32[1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    uint16_t script_indirect  [1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    uint32_t script_nregs     [1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    uint32_t script_stackdepthW32[1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    uint32_t script_param_length[1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    uint32_t max_nregs, max_stack;
    #define AVG_SCRIPT_LEN 100          // list of byte-codes
    char script_bytecode      [(1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)) * AVG_SCRIPT_LEN];
    #define AVG_SCRIPT_PARAM_LEN 1000   // parameters set by scripts
    char script_embedded_param[1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)][(1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)) * AVG_SCRIPT_LEN];

    uint32_t script_sctrl;

    uint32_t mem_consumption[MAX_NB_MEMORY_OFFSET];      /* for the listing */

    /* nodes + parameters + computed memory allocations */
    struct stream_node_manifest all_nodes[MAX_NB_NODES];    /* share the same type as manifests */
    uint32_t nb_nodes;
    uint64_t max_of_sum_of_working_simplemap;

    /* memory allocation : incremented per bank, until reaching "processor_memory_bank_t.size" */
    uint64_t mbank_1_Byte;
    uint64_t membank_detailed_malloc[MAX_PROC_MEMBANK];

    uint32_t end_binary_graph;
    uint32_t binary_graph[MAXBINARYGRAPHW32];

    uint32_t idbg;
    dbgtrace_t dbg[MAXDBGTRACEGRAPH];
};

#endif /* #ifndef cSTREAM_TOOL_H */
#ifdef __cplusplus
}
#endif
