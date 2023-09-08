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

#ifndef cSTREAM_ARCS_H
#define cSTREAM_ARCS_H


/*================================================== ARC ==================================================================*/
/*
  arc descriptions : 
                             
      - arc_descriptor_ring : R/W are used to manage the alignment of data to the base address and notify the SWC
                              debug pattern, statistics on data, time-stamps of access, 
                              realignment of data to base-address when READ > (SIZE)*THR
                              deinterleave multichannel have the same read/write offset but the base address starts 
                              from the end of the previous channel
                              boundary of the graph, thresholds and flow errors, func_exchange_data
*/

// enum arc_data_operations_threshold
// enum debug_arc_computation_1D { /* 4bits */
// extra_command_id (8 commands) used in EXDTCMD_IOFMT from STREAM_FORMAT_IO[]
// other commands used with ARCs COMPUTCMD_ARCW2 (16 commands)
#define ARC_NO_ACTION 0u                
#define ARC_INCREMENT_REG 1u        /* increment DEBUG_REG_ARCW1 */
#define ARC_SET_ZERO_ADDR 2u        /* set a 0 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to clear */
#define ARC_SET_ONE_ADDR 3u         /* set a 1 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to set */
#define ARC_INCREMENT_REG_ADDR 4u   /* increment *DEBUG_REG_ARCW1 */
#define ARC_PROCESSOR_WAKEUP 5u     /* wake-up processor from DEBUG_REG_ARCW1=[ProcID, command] */
#define ARC_____UNUSED1 6u
#define ARC_____UNUSED2 7u
#define ARC_DATA_RATE 8             /* data rate estimate in DEBUG_REG_ARCW1 */
#define ARC_TIME_STAMP_LAST_ACCESS 9
#define ARC_PEAK_DATA 10            /* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW1 */
#define ARC_MEAN_DATA 11
#define ARC_MIN_DATA 12
#define ARC_ABSMEAN_DATA 13
#define ARC_DATA_TO_OTHER_ARC 14    /* when data is changing the new data is push to another arc DEBUG_REG_ARCW1=[ArcID] */
#define ARC_unused3 15u
//};

//enum underflow_error_service_id{/* 2bits UNDERFLRD_ARCW2, OVERFLRD_ARCW2 */
#define NO_UNDERFLOW_MANAGEMENT 0  /* just RD/WR index alignment on the next frame size */
#define REPEAT_LAST_FRAME 1          /* flow errors management */
#define GENERATE_ZEROES 2
#define INTERPOLATE_FRAME 3          /* MPEG decoder underflow : tell the decoder, mute the output */
//};

//enum overflow_error_service_id {/* 2bits UNDERFLRD_ARCW2, OVERFLRD_ARCW2 */
#define NO_OVERFLOW_MANAGEMENT 0     /* just RD/WR index alignment on the next frame size */
#define SKIP_LAST_FRAME 1            /* flow errors management */
#define DECIMATE_FRAME 2
//};

/*============================================================================================================*/
/* 
    Format 23+4_offsets for buffer BASE ADDRESS
    Frame SIZE and ring indexes are using 22bits linear (0..4MB)

*/

/* for MISRA-2012 compliance to Rule 10.4 */
#define U(x) ((uint32_t)(x)) 
#define U8(x) ((uint8_t)(x)) 
//#define S(x) ((int32_t)(x)) 
#define S8(x) ((int8_t)(x)) 

//------------------------------------------------------------------------------------------------------

//enum time_stamp_format_type {
#define NO_TS 0
#define ABS_TS_64 1                  /* long time reference */
#define REL_TS_32 2                  /* time difference from previous frame packet in stream_time_seconds format */
#define COUNTER_TS 3                 /* counter of data frames */


//enum hashing_type {
#define NO_HASHING 0                 /* cipher protocol under definition */
#define HASHING_ON 1                 /* the stream is ciphered (HMAC-SHA256 / stream_encipher XTEA)*/


//enum frame_format_type {
#define FMT_INTERLEAVED 0           /* "arc_descriptor_interleaved" for example L/R audio or IMU stream..   */
                                    /* the pointer associated to the stream points to data (L/R/L/R/..)     */
#define FMT_DEINTERLEAVED_1PTR 1    /* single pointer to the first channel, next channel base address is    */
                                    /*  computed by adding the buffer size/nchan, also for ring buffers  */
//
////enum frame_format_synchro {
#define SYNCHRONOUS 0               /* tells the output buffer size is NOT changing */
#define ASYNCHRONOUS 1              /* tells the output frame length is variable, input value "Size" tells the maximum value  
//                                       data format : optional time-stamp (stream_time_stamp_format_type)
//                                                    domain [2bits] and sub-domain [6bits rfc8428]
//                                                    payload : [nb samples] [samples]  */
////enum direction_rxtx {
#define IODIRECTION_RX 0              /* RX from the Graph pont of view */
#define IODIRECTION_TX 1

/*============================================================================================================*/
#define STREAM_FORMAT_SIZE_W32 3            // digital, common part of the format 

/*
*   stream_data_stream_data_format (size multiple of 3 x uint32_t)
*       word 0 : common to all domains : frame size, raw format, interleaving
*       word 1 : common to all domains : time-stamp, sampling rate, nchan         
*       word 2 : specific to domains : hashing, direction, channel mapping 
*/

/*--------------- WORD 0 - producer frame size, raw format, interleaving --------------- */
#define SIZSFTRAW_FMT0   0
    #define MULTIFRME_FMT0_MSB 31 /*  1 allow the scheduler to push multiframes  */
    #define MULTIFRME_FMT0_LSB 31
    #define       RAW_FMT0_MSB 30
    #define       RAW_FMT0_LSB 25 /*  6 stream_raw_data 6bits (0..63)  */
    #define INTERLEAV_FMT0_MSB 24       
    #define INTERLEAV_FMT0_LSB 24 /*  1 interleaving : frame_format_type */
                                  /*    frame size in bytes for one deinterleaved channel Byte-acurate up to 16MBytes */
    #define FRAMESIZE_FMT0_MSB 23 /*    raw interleaved buffer size is framesize x nb channel, max = 16MB x nchan */
    #define FRAMESIZE_FMT0_LSB  0 /* 24 in swc manifests it gives the minimum input size (grain) before activating the swc
                                        A "frame" is the combination of several channels sampled at the same time 
                                        A value =0 means the size is any or defined by the IO AL.
                                        For sensors delivering burst of data not isochronous, it gives the maximum 
                                        framesize; same comment for the sampling rate. */

    #define PACKSTREAMFORMAT0(RAW,INTERL,FRAMESIZE) ((U(RAW)<<26)|(U(INTERL)<<24)|U(FRAMESIZE))

/*--------------- WORD 1 - sampling rate , nchan  -------------*/
#define   SAMPINGNCHANM1_FMT1  U( 1)
    #define    UNUSED_FMT1_MSB U(31) /* 5   */
    #define    UNUSED_FMT1_LSB U(27)
    #define  TIMSTAMP_FMT1_MSB U(26) /* 2  time_stamp_format_type for time-stamped streams for each interleaved frame */
    #define  TIMSTAMP_FMT1_LSB U(25) 
    #define  SAMPLING_FMT1_MSB U(24) /* 20 mantissa 16b + exponent 4b, FS= 0 <=> ASYNCHRONOUS/SERVANT */
    #define  SAMPLING_FMT1_LSB U( 5) /*    FS = (Mx16)/2^(2xE) (High FS) FS = (Mx16)/2^(3xE) (Low FS) */
    #define   NCHANM1_FMT1_MSB U( 4)
    #define   NCHANM1_FMT1_LSB U( 0) /* 5  nb channels-1 [1..32] */

    /* E=0, 1x16/2^0    16Hz ..  1MHz    48 kHz =  3000 x16 for example */
    /* E=1, 1x16/2^2     4Hz ..262kHz   44100Hz = 11025 x 4, 88100 = 22050 x 4, 176400Hz = 44100Hz x 4 */
    /* E=2, 1x16/2^4     1Hz .. 65kHz  */
    /* E=3, 1x16/2^6   0.2Hz .. 16kHz  */
    /* E=4, 1x16/2^8  0.06Hz ..  4kHz  */
    /* E=5  1x16/2^10  15mHz ..  1kHz  */
    /* E=6  1x16/2^12   4mHz .. 250Hz  */
    /* E=7  1x16/2^14   1mHz ..  64Hz  */

    /* E=8  1x16/2^17 122uHz ..   8Hz  */
    /* E=9  1x16/2^20  15uHz ..   1Hz  */
    /* E=10 1x16/2^23   2uHz ..125mHz  */
    /* E=11 1x16/2^26 238uHz .. 16mHz  */
    /* E=12 1x16/2^29  30nHz ..  2mHz  */
    /* E=13 1x16/2^32   4nHz ..244uHz  1 day = 11.57407uHz = 12427 x 16/2^34, drift error of ~3 seconds/day */
    /* E=14 1x16/2^35 0.5nHz .. 30uHz  1 week = 1.65344uHz = 28406 x 16/2^38, drift error of ~3 seconds/week */
    /* E=15 1x16/2^38  58pHz ..  4uHz  1 month (30d) =386nHz, 3314 x 16/2^38, drift error of ~14 seconds/month */

    #define PACKSTREAMFORMAT1(TIMESTAMP,SAMPLING, NCHANM1) ((U(TIMESTAMP)<<26)|(U(SAMPLING)<<5)|U(NCHANM1))
    #define FMT_FS_MAX_EXPONENT 15
    #define FMT_FS_MAX_MANTISSA 65535
    #define FMT_FS_EXPSHIFT 16
    #define HIGH_FMTQ2FS(E,M) ((M*16)/pow(2,(2*E)))
    #define LOW_FMTQ2FS(E,M)  ((M*16)/pow(2,((3*E)-7)))

/*--------------- WORD 2 -  direction, channel mapping (depends on the "stream_io_domain")------*/
#define AUDIO_MAPPING_FMT2 U( 2)
    #define MAPPING_FMT2_MSB U(31) /* 32 mapping of channels example of 7.1 format (8 channels): */
    #define MAPPING_FMT2_LSB U( 0) /*     FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..*/

#define IMU_FMT2 U( 2)
    #define MAPPING_FMT2_MSB U(31) /* 32 mapping of channels example of 7.1 format (8 channels): */
    #define MAPPING_FMT2_LSB U( 0) /*     FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..*/


#define     PICTURE_FMT2 U( 2)
    #define  UNUSED_FMT2_MSB U(31) /* 12b   */
    #define  UNUSED_FMT2_LSB U(20)
    #define  BORDER_FMT2_MSB U(19) /* 2 pixel border 0,1,2,3   */
    #define  BORDER_FMT2_LSB U(18)
    #define   RATIO_FMT2_MSB U(17) /* 4 ratio to height : 1:1 4:3 16:9 16:10 5:4  */
    #define   RATIO_FMT2_LSB U(14)
    #define   WIDTH_FMT2_MSB U(13) /* 14 number of pixel width */
    #define   WIDTH_FMT2_LSB U( 0) 



/*------------------------------------------------------------------------------------*/

#define SIZEOF_ARCDESC_SHORT_W32 U(2)
#define SIZEOF_ARCDESC_W32 U(4)

#define   BUF_PTR_ARCW0    U( 0)
#define PRODUCFMT_ARCW0_MSB U(31) /* 5 bits  PRODUCER format  (intPtr_t) +[ixSTREAM_FORMAT_SIZE_W32]  */ 
#define PRODUCFMT_ARCW0_LSB U(27) /*   Graph generator gives IN/OUT arc's frame size to be the LCM of SWC "grains" */
#define BASEIDXOFFARCW0_MSB U(26) 
#define   DATAOFF_ARCW0_MSB U(26) /*   address = offset[DATAOFF] + 4xBASEIDX [Bytes] */
#define   DATAOFF_ARCW0_LSB U(24) /* 3 32/64bits offset index see idx_memory_base_offset */
#define   ________ARCW0_MSB U(23) /*   We don't know yet if the base will be extended with a 2bits shifter */
#define   ________ARCW0_LSB U(22) /* 2  or if the list of offsets must be increased */
#define  BASESIGN_ARCW0_MSB U(21) /*   buffer address 21 + sign + offset = 25 bits (2bits margin) */
#define  BASESIGN_ARCW0_LSB U( 0) /* 1 sign of the address with respect to the offset */
#define   BASEIDX_ARCW0_MSB U(21) /*        */
#define   BASEIDX_ARCW0_LSB U( 0) /*21 base address 22bits linear address range in WORD32 */
#define BASEIDXOFFARCW0_LSB U( 0) /*   Base can address +/- 8MBytes around the offset */
                                
#define BUFSIZDBG_ARCW1    U( 1)
#define CONSUMFMT_ARCW1_MSB U(31) 
#define CONSUMFMT_ARCW1_LSB U(27) /* 5 bits  CONSUMER format  */ 
#define   MPFLUSH_ARCW1_MSB U(26) 
#define   MPFLUSH_ARCW1_LSB U(26) /* 1  flush data used after processing */
#define DEBUG_REG_ARCW1_MSB U(25)
#define DEBUG_REG_ARCW1_LSB U(22) /* 4  debug result index for debug_arcs[0..15] debug_arc_computation_1D */
#define BUFF_SIZE_ARCW1_MSB U(21) 
#define BUFF_SIZE_ARCW1_LSB U( 0) /* 22 Byte-acurate up to 4MBytes */

#define    RDFLOW_ARCW2    U( 2)  /* write access only from the SWC consumer */
#define COMPUTCMD_ARCW2_MSB U(31)       
#define COMPUTCMD_ARCW2_LSB U(28) /* 4  gives the debug task to proceed  (enum debug_arc_computation_1D) */
#define UNDERFLRD_ARCW2_MSB U(27)
#define UNDERFLRD_ARCW2_LSB U(26) /* 2  overflow task id 0=nothing , underflow_error_service_id */
#define  OVERFLRD_ARCW2_MSB U(25)
#define  OVERFLRD_ARCW2_LSB U(24) /* 2  underflow task id 0=nothing, overflow_error_service_id */
#define    EXTEND_ARCW2_MSB U(23) /*    Size/Read/Write are used with 256 bytes chunks => 1GBytes buffers for */
#define    EXTEND_ARCW2_LSB U(23) /* 1    arcs used to read NN models, video players, etc */
#define   READY_W_ARCW2_MSB U(22) /*    "Buffer ready for refill" */
#define   READY_W_ARCW2_LSB U(22) /* 1  (size-write)>size/2 (or /4) */
#define      READ_ARCW2_MSB U(21)
#define      READ_ARCW2_LSB U( 0) /* 22     data read index  Byte-acurate up to 4MBytes starting from base address */

#define COLLISION_ARC_OFFSET_BYTE U(3) /* offset in bytes to the collision detection byte */
#define  WRIOCOLL_ARCW3    U( 3) /* write access only from the SWC producer */
#define COLLISION_ARCW3_MSB U(31) /* 8  MSB byte used to lock the SWC  */ 
#define COLLISION_ARCW3_LSB U(24) 
#define ALIGNBLCK_ARCW3_MSB U(23) /*    producer blocked */
#define ALIGNBLCK_ARCW3_LSB U(23) /* 1  producer sets "need for data realignement"  */
#define   READY_R_ARCW3_MSB U(22) /*    "Buffer ready for read" */ 
#define   READY_R_ARCW3_LSB U(22) /* 1  (write-read)>size/2 (or /4) */
#define     WRITE_ARCW3_MSB U(21)
#define     WRITE_ARCW3_LSB U( 0) /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */

/* arcs with indexes higher than IDX_ARCS_desc, see enum_arc_index */

//#define PLATFORM_IO 0                   /* 3 bits offets code for arcs external to the graph */


#endif /* #ifndef cSTREAM_ARCS_H */

#ifdef __cplusplus
}
#endif
    