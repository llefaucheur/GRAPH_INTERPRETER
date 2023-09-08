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

#include "stream_tool_include.h"

#ifndef cSTREAM_MANIFEST_H
#define cSTREAM_MANIFEST_H

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
    uint32_t sizeA;
    float sizeB, sizeC, sizeD;
    uint16_t iarcChannelI, iarcSamplingJ, iarcFrameK;
    uint8_t alignmentLog2;      /* enum buffer_alignment_type */
    uint8_t usage;              /* enum mem_mapping_type : static, working, pseudo working, backup */
    uint8_t speed;              /* enum mem_speed_type */
    uint8_t relocatable;        /* enum buffer_relocation_type */
};


union anyType
{
    char c;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    float f32;
    double f64;
};

struct options
{
    uint8_t data_format;            /* frame length format (0:in milliseconds 1:in samples) */
                                    /* sampling unit (0:Hz, 1:seconds, 2:days)  */

    uint32_t nb_option;
    uint8_t default_index;
    uint8_t raw_type;              
    float accuracy;                 /* used for the error on the sampling rate, in percentage */

#define MAX_NBOPTIONS 256
    union anyType options[MAX_NBOPTIONS];   /* options offered through SW settings and stream converters */
};

/* 
    arc description, used for nodes and HW IO streams 
*/
struct arcStruct
{
    /* 
    *   data filed during the graph compilation
    */
    uint16_t arcID;                     /* arc index used in the graph description */

    // common between IOstream and SWC
    uint8_t rx0tx1;                     /* direction rx=0 tx=1 parameter=2 (rx never consumed) */
    uint8_t set0_copy1;                 /* data move through pointer setting of data copy */
    uint8_t commander0_servant1;        /* selection for polling protocol */
    uint8_t graphalloc0_bsp1;           /* buffer declared from the graph 0 or BSP 1 */
    uint8_t sram0_hwdmaram1 ;           /* buffer in standard RAM=0, in HW IO RAM=1 */
    uint8_t extraCommandID;             /* provision for 8 command extensions */
    
    // specific to SWC
    uint8_t inPlaceProcessing;          /* SWCONLY flag buffer overlay with another arcID, 0=none*/
    uint8_t arcIDbufferOverlay;         /* SWCONLY   arcID being overlaid */

    // specific to IOstream 
    uint8_t processorBitFieldAffinity;  /* IOSONLY indexes of the processor in charge of this stream */
    uint8_t platform_al_fw_io_idx;      /* IOSONLY associated function (platform dependent) */


    /* common digital format information between arcs of nodes and IO */
    char domainName[MAXCHAR_NAME];      /* arc name used in the GUI */
    uint8_t domain;                     /* decoded domain_name among stream_io_domain [64] */

    uint8_t raw;
    uint8_t timestp;

    struct options interleaving_option; 
    struct options nbchannel_option;    
    struct options frame_size_option;
    struct options sampling_rate_option;
};


struct processor_memory_bank
{
    uint8_t offsetID;       /* index of this bank return by the AL */
    uint64_t offset64b;     /* long offset provided by the AL, for informationm printed in the comments */
    uint8_t speed;          /* */
    uint8_t shareable;      /* is it a private or shareable memory bank */
    uint8_t data_access;    /* is it a data/program access memory bank */
    uint8_t backup;         /* is it a memory with retention capability */
    uint8_t hwio;           /* is it reserved for IO and DMA data moves */
    uint8_t flash;          /* embedded flash, no write access */
    uint32_t base32;        /* which offset is it from the offset64b */
    long size;          /* which size is guaranteed for graph operations */
};
typedef struct processor_memory_bank processor_memory_bank_t;

struct processing_architecture
{   
    uint8_t processorID;
    uint8_t IamTheMainProcessor;
    uint16_t architecture;
    uint32_t libraries_b;   /* bit-field of embedded libraries in the flash of the device */
    uint8_t nbMemoryBank_simple;
    uint8_t nbMemoryBank_detailed;
    processor_memory_bank_t membank_simple[MAX_PROC_MEMBANK];
    processor_memory_bank_t membank_detailed[MAX_PROC_MEMBANK];
};
typedef struct processing_architecture processing_architecture_t;

enum imu_channel_format /* uint8_t : data interleaving possible combinations */
{
    aXg0m0,     /* only accelerometer */
    a0gXm0,     /* only gyroscope */
    a0g0mX,     /* only magnetometer */
    aXgXm0,     /* A + G */
    aXg0mX,     /* A + M */
    a0gXmX,     /* G + M */
    aXgXmX,     /* A + G + M */
 };


struct stream_interfaces_motion_specific
{   /* 
    * digital format specifics 
    */
        uint8_t channel_format;         /* enum imu_channel_format */

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
    uint8_t i;  

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


/*----------------------------------------------------------------------------------------------------*/
struct stream_node_manifest
{
    char developerName[MAXCHAR_NAME];   /* developer's name */
    char nodeName[MAXCHAR_NAME];        /* node name used in the GUI */
    uint8_t nbArch, arch[MAXNBARCH];    /* stream_processor_architectures (max 256) */
    uint8_t nbFpu, fpu[MAXNBARCH];      /* stream_processor_sub_arch_fpu */
    uint8_t nbLib, lib[MAXNBLIBRARY];   /* Up to 16 family of processing extensions "SERVICE_COMMAND_GROUP" */
    uint32_t codeVersion, schedulerVersion; /* version of the scheduler it is compatible with */
    uint8_t deliveryMode;               /* 0:source, 1:binary */

    uint8_t nbMemorySegment;
    struct node_memory_bank mem[MAXNBMEMSEGMENTS]; /* memory allocations */

    uint8_t preset;                     /* default "preset value" */
    uint32_t defaultParameterSize;      /* size of the array below */
    uint8_t defaultParameters[MAXPARAMETERSIZE];

    uint8_t nb_arc;
    struct arcStruct arcs[MAX_NB_STREAM_PER_SWC];
};
/*----------------------------------------------------------------------------------------------------*/

struct stream_IO_interfaces
{
    char IO_name[MAXCHAR_NAME];             /* IO stream name used in the GUI */
    uint8_t instance_index;                 /* 16 selections (several IMU or GPIO for example) */

    uint8_t nb_arc;                         /* number of streams used by this stream interface */
                                            /* format section common to all stream interfaces (digital format) */
    struct arcStruct arc_flow[MAX_NB_ARC_STREAM]; /* rx0tx1, domain, digital format and FS accuracy */

                                            
    union                                   /* format section specific to each domain */
    {   
        struct stream_interfaces_motion_specific imu;
        struct stream_interfaces_audio_specific audio;
        struct stream_interfaces_metadata_specific metadata;
    } U;
};


struct stream_platform_manifest
{   
    uint16_t nb_nodes;                  /* number of software components to build the graph */

    uint8_t nb_processors;              /* number of processors using the graph */
    processing_architecture_t processor[MAX_GRAPH_NB_PROCESSORS];
    
    int nb_hwio_stream;
    struct stream_IO_interfaces stream[MAX_GRAPH_NB_IO_STREAM];
};

typedef struct stream_platform_manifest stream_platform_manifest_t;



#endif /* #ifndef cSTREAM_PLATFORM_MANIFEST_H */
#ifdef __cplusplus
}
#endif
