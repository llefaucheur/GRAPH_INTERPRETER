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

#ifndef cSTREAM_PLATFORM_MANIFEST_H
#define cSTREAM_PLATFORM_MANIFEST_H

#define NBCHAR_LINE 200
#define NBCHAR_NAME 120
#define NBCHAR_STREAM_NAME 32   /* name of the HW IO streams and graph interfaces */
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

#define MAX_NBOPTIONS 256 // to remove

struct options
{
    uint32_t nb_option;
    uint8_t default_index;
    uint8_t raw_type;                /* to do a type cast the array below */
    uint32_t options[MAX_NBOPTIONS]; /* options offered through SW settings and stream coverters */
};

/* 
    arc description, used for nodes and HW IO streams 
*/
struct arcStruct
{
    uint8_t domain;             /* decoded domain_name among stream_io_domain [64] */

    uint8_t rx0tx1;             /* direction */
    float   setupTime;          /* [ms] to avoid this information to being only in the BSP */
    uint8_t set0_copy1;         /* data move through pointer setting of data copy */
    uint8_t extraCommandID;     /* provision for 8 command extensions */


    uint8_t commander0_servant1;/* selection for polling protocol */
    uint8_t graphalloc0_bsp1;   /* buffer declared from the graph 0 or BSP 1 */
    uint8_t sram0_hwdmaram1 ;   /* buffer in standard RAM=0, in HW IO RAM=1 */
    uint8_t processorBitFieldAffinity; /* indexes of the processor in charge of this stream */

    uint8_t raw;
    uint8_t timestp;
    uint8_t frameSizeFormat_0s_1sample;

    struct options interleaving_option; /* options offered through SW settings and stream coverters */
    struct options nbchannel_option; /* options offered through SW settings and stream coverters */
    struct options frame_size_option;
    struct options sampling_rate_option;

    float FS_accuracy;               /* error on the sampling rate, in percentage */

    uint8_t format_index;           /* when compacted = cross-reference */

    /* arc descriptor */
    uint32_t arcdesc_baseidx, arcdesc_dataoff, arcdesc_producerFormat;
    uint32_t arcdesc_bufferSize, arcdesc_debugReg, arcdesc_MPFLush, arcdesc_consumerFormat;
    uint32_t arcdesc_read, arcdesc_readyW, arcdesc_extend, arcdesc_overflowDebug, arcdesc_UnderflowDebug, arcdesc_computeCommand;
    uint32_t arcdesc_arite, arcdesc_readyR, arcdesc_producerBlocked, arcdesc_collisionByte;

    uint16_t ioarc;                 /* arc index used in the graph description */

    /* 
    *   IO stream arcs
    */
    uint8_t platform_al_fw_io_idx;  /* associated function (platform dependent) */
};


struct stream_node_manifest
{
    char developerName[MAXCHAR_NAME];   /* developer's name */
    char nodeName[MAXCHAR_NAME];        /* node name used in the GUI */
    uint8_t nbInputArc;
    uint8_t nbOutputArc;
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

    uint16_t inPlaceProcessing, arcIDbufferOverlay;
    uint8_t nbArcs;
    struct arcStruct arcs[MAX_NB_STREAM_PER_SWC];
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
};

struct stream_interfaces_2d_in_specific
{   /*
     * digital format specifics
     */

    /*
    * mixed-signal data specifics
    */
    uint8_t wdr;                // wide dynamic range flag (tuya)
    uint8_t watermark;          // watermark insertion flag (tuya)
    uint8_t flip;               // image format (portrait, panoramic)
    uint8_t night_mode;         // motion detection sensitivity (low, medium, high)
    uint8_t detection_zones;    // + {center pixel (in %) radius}, {}, {}
    uint8_t time_stamp;         // detection time-stamp format 
    uint8_t light_detection;    // 
    uint8_t sound_detection;    // sound level 
    uint8_t sensors;            // humudity, battery% 
};


struct stream_interfaces_av_codec_specific
{   /*
     * digital format specifics
     */

TUYA_VIDEO_BITRATE_64K	64	The code rate is 64 kbits/s.
TUYA_VIDEO_BITRATE_128K	128,	The code rate is 128 kbits/s.
TUYA_VIDEO_BITRATE_256K	256,	The code rate is 256 kbits/s.
TUYA_VIDEO_BITRATE_512K	512,	The code rate is 512 kbits/s.
TUYA_VIDEO_BITRATE_768K	768,	The code rate is 768 kbits/s.
TUYA_VIDEO_BITRATE_1M	1024,	The code rate is 1024 kbits/s.
TUYA_VIDEO_BITRATE_1_5M	1536	The code rate is 1536 kbits/s.

TUYA_CODEC_VIDEO_MPEG4 = 0,
    TUYA_CODEC_VIDEO_H263,
    TUYA_CODEC_VIDEO_H264,
    TUYA_CODEC_VIDEO_MJPEG,
    TUYA_CODEC_VIDEO_H265,
    TUYA_CODEC_VIDEO_MAX = 99,
    TUYA_CODEC_AUDIO_ADPCM,
    TUYA_CODEC_AUDIO_PCM,
    TUYA_CODEC_AUDIO_AAC_RAW,
    TUYA_CODEC_AUDIO_AAC_ADTS,
    TUYA_CODEC_AUDIO_AAC_LATM,
    TUYA_CODEC_AUDIO_G711U,
    TUYA_CODEC_AUDIO_G711A,
    TUYA_CODEC_AUDIO_G726,
    TUYA_CODEC_AUDIO_SPEEX,
    TUYA_CODEC_AUDIO_MP3,
    TUYA_CODEC_AUDIO_MAX = 199,
    TUYA_CODEC_INVALID

    TUYA_AUDIO_SAMPLE_8K     = 8000,
    TUYA_AUDIO_SAMPLE_11K    = 11000,
    TUYA_AUDIO_SAMPLE_12K    = 12000,
    TUYA_AUDIO_SAMPLE_16K    = 16000,
    TUYA_AUDIO_SAMPLE_22K    = 22000,
    TUYA_AUDIO_SAMPLE_24K    = 24000,
    TUYA_AUDIO_SAMPLE_32K    = 32000,
    TUYA_AUDIO_SAMPLE_44K    = 44000,
    TUYA_AUDIO_SAMPLE_48K    = 48000,
    TUYA_AUDIO_SAMPLE_MAX    = 0xFFFFFFFF

TUYA_AUDIO_DATABITS_8	8	8 bits
TUYA_AUDIO_DATABITS_16 	16	16 bits
TUYA_AUDIO_DATABITS_MAX	0xFF	255 bits

TUYA_AUDIO_CHANNEL_MONO	0	Mono
TUYA_AUDIO_CHANNEL_STERO	1	Stereo

2.5.1 Maximum Number of Video and Audio Stream Channels
typedef enum
{
    E_CHANNEL_VIDEO_MAIN = 0,    
    E_CHANNEL_VIDEO_SUB,
    E_CHANNEL_VIDEO_3RD,
    E_CHANNEL_VIDEO_4TH,
    E_CHANNEL_VIDEO_MAX = 8,    // Maximum number of video stream channels
    E_CHANNEL_AUDIO,
    E_CHANNEL_AUDIO_2RD,
    E_CHANNEL_AUDIO_3RD,
    E_CHANNEL_AUDIO_4TH,
    E_CHANNEL_MAX = 16
}CHANNEL_E;

Parameter	Description
channel_enable	Enable.
video_fps	Frame rate. The default value is 30.
video_gop	I-Frame interval. The default value is 60.
video_bitrate	See definition of TUYA_VIDEO_BITRATE_E.
video_width	Width range, which is associated with the video coding formats SD and HD. The SD resolution is below 1280 x 720 pixels, and the HD resolution is 1280 x 720 pixels to 1920 x 1080 pixels.
video_height	Height range, which is associated with the video coding formats SD and HD. The SD resolution is below 1280 x 720 pixels, and the HD resolution is 1280 x 720 pixels to 1920 x 1080 pixels.
video_freq	Clock frequency. The default value is 90000.
video_codec	See definition of TUYA_CODEC_ID.
audio_codec	See definition of TUYA_CODEC_ID.
audio_fps	Audio frame rate range. The default value is 25.
audio_sample	See definition of TUYA_AUDIO_SAMPLE_E.
audio_databits	See definition of TUYA_AUDIO_DATABITS_E.
audio_channel	See definition of TUYA_AUDIO_CHANNEL_E.

Parameter	Description
pan	Horizontal PTZ. The driver layer parameter value range is not limited and can be defined as needed.
tilt	Vertical PTZ. The driver layer parameter value range is not limited and can be defined as needed.
zoom	Zoom function. The driver layer parameter value range is not limited and can be defined as needed.

Parameter	Description
id	Identifier for each preset point on the server.
tilt	Preset name. The value is a maximum of 31 characters, including Chinese characters.
zoom	Preset point index ID, which must be unique.
Ptz	PTZ position information.

4.7 Audio and Video Interface Definitions
4.7.1 tuya_ipc_ring_buffer_init
Function prototype
OPERATE_RET tuya_ipc_ring_buffer_init(
CHANNEL_E channel, 
UINT_T bitrate, 
UINT_T fps, 
UINT_T max_buffer_seconds, 
FUNC_REQUEST_I_FRAME requestIframeCB
);




////////////////////
    if (0 == strcmp(input, "storage_out"))          *domain_index = PLATFORM_STORAGE_OUT; /* periodic dump of captured data */

4.6 SD Card Storage Interface Definitions
4.6.1 tuya_ipc_ss_init
Function prototype
OPERATE_RET tuya_ipc_ss_init(
IN CHAR_T *base_path, 
IN IPC_MEDIA_INFO_S *media_setting,
 IN UINT_T max_event_per_day
);

Function description
This interface initializes the SD card storage logic.

Parameter description

Parameter	Description	Parameter Type	Mandatory	Setting
base_path	Root directory of the SD card	Input parameter	Yes	
media_setting	Audio and video configuration information	Input parameter	Yes	See section 3.2.1 "IPC_MEDIA_INFO_S."
max_event_per_day	Maximum number of events on a single day	Input parameter	Yes	If the number of events on a single day exceeds the value, playback will fail due to storage exceptions. If this parameter is set to a large value, it will take a long time to respond to the app, causing user experience to deteriorate.

4.6.16 tuya_ipc_ss_pb_start
Function prototype
OPERATE_RET tuya_ipc_ss_pb_start(
IN UINT_T pb_idx, 
IN SS_PB_EVENT_CB event_cb, 
IN SS_PB_GET_MEDIA_CB video_cb, 
IN SS_PB_GET_MEDIA_CBaudio_cb
);

4.6.19 tuya_ipc_ss_pb_seek
Function prototype
OPERATE_RET tuya_ipc_ss_pb_seek(
IN UINT_T pb_idx, 
IN SS_FILE_TIME_TS_S *pb_file_info, 
IN UINT_T play_timestamp
);


    /*
    * mixed-signal data specifics
    */
 
};

struct stream_interfaces_metadata_specific
{   /*
     * digital format specifics
     */
    /*
    * mixed-signal data specifics
    */
    int i;  /* nothing specific, so far ..*/
};


struct stream_interfaces
{
    char IO_name[NBCHAR_STREAM_NAME];       /* IO stream name used in the GUI */
    uint8_t nb_arc;                         /* number of streams used by this stream interface */
    uint8_t instance_index;                 /* 16 selections (several IMU or GPIO for example) */

                                            /* format section common to all stream interfaces (digital format) */
    struct arcStruct arc_flow[MAX_NB_ARC_STREAM]; /* rx0tx1, domain, digital format and FS accuracy */

    /* desired platform domains mapping to platform capabilities (fw_io_idx) */
    uint8_t domains_to_fw_io_idx[PLATFORM_MAX_NB_DOMAINS];

                                            /* format section specific to each domain */
    union
    {   struct stream_interfaces_motion_specific imu;
        struct stream_interfaces_audio_specific audio;
        struct stream_interfaces_2d_in_specific image;
        struct stream_interfaces_metadata_specific metadata;
    } U;
};


struct stream_platform_manifest
{   
    uint16_t nb_nodes;                  /* number of software components to build the graph */

    uint8_t nb_processors;              /* number of processors using the graph */
    processing_architecture_t processor[MAX_GRAPH_NB_PROCESSORS];
    
    int nb_hwio_stream;
    struct stream_interfaces stream[MAX_GRAPH_NB_IO_STREAM];
};

typedef struct stream_platform_manifest stream_platform_manifest_t;



#endif /* #ifndef cSTREAM_PLATFORM_MANIFEST_H */
#ifdef __cplusplus
}
#endif
