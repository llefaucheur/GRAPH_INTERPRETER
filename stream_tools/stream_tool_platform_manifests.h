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

#ifndef cSTREAM_PLATFORM_MANIFESTS_H
#define cSTREAM_PLATFORM_MANIFESTS_H

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
    int sensors;            // humudity, battery% 
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
    uint64_t size0;
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
    uint32_t private;        /* is it a private or shareable memory bank */
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
    uint32_t domain;                     /* decoded domain_name among stream_io_domain [64] */

    uint32_t set0_copy1;                 /* data move through pointer setting of data copy */
    uint32_t commander0_servant1;        /* selection for polling protocol */
    uint32_t graphalloc0_bsp1;           /* buffer declared from the graph 0 or BSP 1 */
    uint32_t sram0_hwdmaram1;            /* buffer in standard RAM=0, in HW IO RAM=1 */
    uint32_t processorBitFieldAffinity;  /* indexes of the processor in charge of this stream */

    uint32_t platform_al_fw_io_idx;      /* associated function (platform dependent) */
    float setupTime;                /* [ms] to avoid this information to being only in the BSP */
    uint32_t settings;              /* pack format of digital + MSIC options */
};


struct formatStruct 
{
    uint32_t FMT0, FMT1, FMT2;
    struct common_io_swc sc;
};
typedef struct formatStruct formatStruct_t;

/*
    stream_format_io
*/
struct io_arcstruct
{
    uint32_t format_idx;
    struct specific_io si;
    struct common_io_swc sc;
};


/* 
    arc description, used for nodes and HW IO streams 
*/
struct arcStruct
{
    /* 
    *   data filed during the graph compilation
    */
    uint32_t ioarc;                      /* arc index used in the graph description */
    uint32_t arcID;                      /* arc index used in the graph description */

    // specific to SWC
    uint32_t inPlaceProcessing;          /* SWCONLY flag buffer overlay with another arcID, 0=none*/
    uint32_t arcIDbufferOverlay;         /* SWCONLY   arcID being overlaid */
    uint32_t src_instanceid, dst_instanceid; /* for information during graph reading */
    uint32_t swc_idx_src, swc_idx_dst;   /* SWC */

    struct specific_io si;
    struct common_io_swc sc;

    /* common digital format information between arcs of nodes and IO */
    struct options interleaving_option; 
    struct options nbchannel_option;    
    struct options frame_size_option;
    struct options sampling_rate_option;

    /* arc descriptor */
    uint32_t ARCW0, ARCW1, ARCW2, ARCW3;
    //uint64_t arcdesc_baseidx, arcdesc_dataoff, arcdesc_producerFormat;
    //uint64_t arcdesc_bufferSize, arcdesc_debugReg, arcdesc_MPFLush, arcdesc_consumerFormat;
    //uint64_t arcdesc_read, arcdesc_readyW, arcdesc_extend, arcdesc_overflowDebug, arcdesc_UnderflowDebug, arcdesc_computeCommand;
    //uint64_t arcdesc_write, arcdesc_readyR, arcdesc_producerBlocked, arcdesc_collisionByte;
};

/*----------------------------------------------------------------------------------------------------*/



#endif /* #ifndef cSTREAM_PLATFORM_MANIFESTS_H */
#ifdef __cplusplus
}
#endif
