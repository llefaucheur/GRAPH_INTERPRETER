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

//enum arc_data_operations_threshold
//enum debug_arc_computation_1D { /* 4bits */

//#if 0
//#define ARC_NO_COMPUTE 0
//#define ARC_DATA_RATE 1
//#define ARC_TIME_STAMP_LAST_ACCESS 2
//#define ARC_PEAK_DATA 3
//#define ARC_MEAN_DATA 4
//#define ARC_MIN_DATA 5
//#define ARC_ABSMEAN_DATA 6
//#define ARC_METADATA_TO_OTHER_ARC 6 /* when metada is changing (from last DEBUG_REG_ARCW1) the new data is push to another arc (DBG_IDX_ARCW5) */
////};
//
////enum underflow_error_service_id{/* 3bits UNDERFLRD_ARCW2, OVERFLRD_ARCW2 */
//#define NO_UNDERFLOW_MANAGEMENT 0  /* just RD/WR index alignment on the next frame size */
//#define REPEAT_LAST_FRAME 1          /* flow errors management */
//#define GENERATE_ZEROES 2
//#define INTERPOLATE_FRAME 3          /* MPEG decoder underflow : tell the decoder, mute the output */
////};
//
////enum overflow_error_service_id {/* 3bits UNDERFLRD_ARCW2, OVERFLRD_ARCW2 */
//#define NO_OVERFLOW_MANAGEMENT 0     /* just RD/WR index alignment on the next frame size */
//#define SKIP_LAST_FRAME 1            /* flow errors management */
//#define DECIMATE_FRAME 2
////};
//#endif

/*============================================================================================================*/
/* 
    Format 22+2 for buffer BASE ADDRESS
    Frame SIZE and ring indexes are using 24bits linear (0..16MB)

    shifter 0 : linear space with Byte accuracy up to 4MB (1<<22)
    shifter 1 : 16Bytes accuracy 0 .. 4MB<<4 = 64MB 
    shifter 2 : 256Bytes accuracy 0 .. 4MB<<8 = 1GB
    shifter 3 : 4096Bytes accuracy 0 .. 4MB<<12 = 16GB

*/

/* for MISRA-2012 compliance to Rule 10.4 */
#define U(x) ((uint32_t)(x)) 
#define U8(x) ((uint8_t)(x)) 
#define S(x) ((int32_t)(x)) 
#define S8(x) ((int8_t)(x)) 

//------------------------------------------------------------------------------------------------------

//enum time_stamp_format_type {
#define NO_TS 0
//#define ABS_TS_64 1                  /* long time reference */
//#define REL_TS_32 2                  /* time difference from previous frame packet in stream_time_seconds format */
//#define COUNTER_TS 3                 /* counter of data frames */


//enum hashing_type {
//#define NO_HASHING 0                 /* cipher protocol under definition */
//#define HASHING_ON 1                 /* the stream is ciphered (HMAC-SHA256 / stream_encipher XTEA)*/


//enum frame_format_type {
//#define FMT_INTERLEAVED 0           /* "arc_descriptor_interleaved" for example L/R audio or IMU stream..   */
//                                    /* the pointer associated to the stream points to data (L/R/L/R/..)     */
#define FMT_DEINTERLEAVED_1PTR 1    /* single pointer to the first channel, next channel base address is    */
                                    /*  computed by adding the buffer size/nchan, also for ring buffers  */
//
////enum frame_format_synchro {
//#define SYNCHRONOUS 0               /* tells the output buffer size is NOT changing */
//#define ASYNCHRONOUS 1              /* tells the output frame length is variable, input value "Size" tells the maximum value  
//                                       data format : optional time-stamp (stream_time_stamp_format_type)
//                                                    domain [2bits] and sub-domain [6bits rfc8428]
//                                                    payload : [nb samples] [samples]  */
////enum direction_rxtx {
//#define IODIRECTION_RX 0              /* RX from the Graph pont of view */
//#define IODIRECTION_TX 1

/*============================================================================================================*/
#define STREAM_FORMAT_SIZE_W32 3            // digital, common part of the format 

/*
*   stream_data_stream_data_format (size multiple of 4 x uint32_t)
*   format (4 x uint32_t) 
*       word 0 : common to all domains : frame size, raw format, interleaving
*       word 1 : common to all domains : sampling rate , nchan                  << SWC flexibility 
*       word 2 : specific to domains : hashing, direction, channel mapping 
*       word 3 : mixed-signal interface setting (sampling, gain, filters, ..), codec setting
*/

/*--------------- WORD 0 - frame size, raw format, interleaving --------------- */
//#define SIZSFTRAW_FMT0   0
    //#define       RAW_FMT0_MSB 31
    //#define       RAW_FMT0_LSB 26 /* stream_raw_data 6bits (0..63)  */
    //#define INTERLEAV_FMT0_MSB 25 
    //#define INTERLEAV_FMT0_LSB 24 /* interleaving : frame_format_type */
    //                              /* frame size in bytes for one deinterleaved channel Byte-acurate up to 16MBytes */
    //#define FRAMESIZE_FMT0_MSB 23 /* raw interleaved buffer size is framesize x nb channel, max = 16MB x nchan */
    //#define FRAMESIZE_FMT0_LSB  0 /* in swc manifests it gives the minimum input size (grain) before activating the swc
    //                                 A "frame" is the combination of several channels sampled at the same time 
    //                                 A value =0 means the size is any or defined by the IO AL  */

    #define PACKSTREAMFORMAT0(RAW,INTERL,FRAMESIZE) ((U(RAW)<<26)|(U(INTERL)<<24)|U(FRAMESIZE))

/*--------------- WORD 1 - sampling rate , nchan  -------------*/
//#define   SAMPINGNCHANM1_FMT1  U( 1)
    //#define    UNUSED_FMT1_MSB U(31) /* 4b   */
    //#define    UNUSED_FMT1_LSB U(28)
    //#define  TIMSTAMP_FMT1_MSB U(27)
    //#define  TIMSTAMP_FMT1_LSB U(26) /* time_stamp_format_type for time-stamped streams for each interleaved frame */
    //#define  SAMPLING_FMT1_MSB U(25) /* 21bits : mantissa [U19], exponent [U2], FS = M x 2^(1-(8xE)), 0<=>ASYNCHRONOUS/SLAVE */
    //#define  SAMPLING_FMT1_LSB U( 5) /* range = (E=0,1,2,3) 0x3FFFF x 2^(1-4x0) .. 1 x 2^(1-8*3)) [524kHz .. 3 months] */
    //#define   NCHANM1_FMT1_MSB U( 4)
    //#define   NCHANM1_FMT1_LSB U( 0) /* nb channels-1 [1..32] */

    #define PACKSTREAMFORMAT1(TIMESTAMP,SAMPLING, NCHANM1) ((U(TIMESTAMP)<<26)|(U(SAMPLING)<<5)|U(NCHANM1))


/*--------------- WORD 2 -  direction, channel mapping (depends on the "domain")------*/
//#define          MAPPING_FMT2  U( 2)
    //#define    UNUSED_FMT2_MSB U(31)
    //#define    UNUSED_FMT2_LSB U(27) /* 5   unused*/
    //#define   HASHIDX_FMT2_MSB U(26)
    //#define   HASHIDX_FMT2_LSB U(25) /* 2  index (1,2,3) of the hashing key structure */
    //#define DIRECTION_FMT2_MSB U(24)
    //#define DIRECTION_FMT2_LSB U(24) /* 1  0 = RX (data flows to the graph), 1 = TX */
    //#define   MAPPING_FMT2_MSB U(23) /* 24 mapping of <24 channels example of 7.1 format (8 channels): */
    //#define   MAPPING_FMT2_LSB U( 0) /*     FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..*/

    #define PACKSTREAMFORMAT2(HASH,DIRECTION,MAP) ((U(HASH)<<25)|(U(DIRECTION)<<24)|U(MAP))

#define SELECT_BASE_ARCW0   U(27) /*   at graph boot time: select the Flash or the long_offsets[] */

#define SIZEOF_ARCDESC_W32 U(4)

#define   BUF_PTR_ARCW0     U( 0)
#define FORMATIDX_ARCW0_MSB U(31) /* 5 bits  32 global formats in the graph, graph (intPtr_t) +[ixSTREAM_FORMAT_SIZE_W32]  */ 
#define FORMATIDX_ARCW0_LSB U(27) /*   Graph generator gives IN/OUT arc's frame size to be the LCM of SWC "grains" */
#define BASEIDXOFFARCW0_MSB U(26) 
#define   DATAOFF_ARCW0_MSB U(26) /*    arcs are using offset=0/1, same format for SWC instances: 0..4 */
#define   DATAOFF_ARCW0_LSB U(24) /* 3 bits 64bits offset index see idx_memory_base_offset */
#define BASESHIFT_ARCW0_MSB U(23) /*   check physical_to_offset() before changing data */
#define BASESHIFT_ARCW0_LSB U(22) /* 2 bits shifter */
#define   BASEIDX_ARCW0_MSB U(21) /*   buffer address (22 + 2) + offset = 27 bits */
#define   BASEIDX_ARCW0_LSB U( 0) /*22   base address 22bits + 2bits exponent ((base) << ((shift) << 2)) */
#define BASEIDXOFFARCW0_LSB U( 0) 
                                
#define  BUFSIZDBG_ARCW1    U( 1)
#define    UNUSED_ARCW1_MSB U(31) 
#define    UNUSED_ARCW1_LSB U(28) /* 4  */
#define SHAREDARC_ARCW1_MSB U(27) 
#define SHAREDARC_ARCW1_LSB U(27) /* 1  null source, null sink  (no overflow, for metadata) */
#define   MPFLUSH_ARCW1_MSB U(26) /* 1  if "1" then flush the arc's buffer after processing (MProcessing) */ 
#define   MPFLUSH_ARCW1_LSB U(26) 
#define DEBUG_REG_ARCW1_MSB U(25)
#define DEBUG_REG_ARCW1_LSB U(22) /* 4  debug result index for debug_arcs[0..15] debug_arc_computation_1D */
#define BUFF_SIZE_ARCW1_MSB U(21) 
#define BUFF_SIZE_ARCW1_LSB U( 0)  /* 22 Byte-acurate up to 4MBytes */

#define RDFLOW_ARCW2        U( 2)  /* write access only from the SWC consumer */
#define COMPUTCMD_ARCW2_MSB U(31)       
#define COMPUTCMD_ARCW2_LSB U(28) /* 4  gives the debug task to proceed  (enum debug_arc_computation_1D) */
#define UNDERFLRD_ARCW2_MSB U(27)
#define UNDERFLRD_ARCW2_LSB U(26) /* 2  overflow task id 0=nothing , underflow_error_service_id */
#define  OVERFLRD_ARCW2_MSB U(25)
#define  OVERFLRD_ARCW2_LSB U(24) /* 2  underflow task id 0=nothing, overflow_error_service_id */
#define THRESHOLD_ARCW2_MSB U(23) 
#define THRESHOLD_ARCW2_LSB U(23) /* 1  0:filling at size/2,  1:at size/4 */
#define   READY_W_ARCW2_MSB U(22) /*    "Buffer ready for refill" */
#define   READY_W_ARCW2_LSB U(22) /* 1  (size-write)>size/2 (or /4) */
#define      READ_ARCW2_MSB U(21)
#define      READ_ARCW2_LSB U( 0) /* 22     data read index  Byte-acurate up to 4MBytes starting from base address */

#define WRIOCOLL_ARCW3      U( 3) /* write access only from the SWC producer */
#define COLLISION_ARCW3_MSB U(31) /* 8  MSB byte used to lock the SWC  */ 
#define COLLISION_ARCW3_LSB U(24) 
#define COLLISION_ARC_OFFSET_BYTE U(3)
#define ALIGNBLCK_ARCW3_MSB U(23)
#define ALIGNBLCK_ARCW3_LSB U(23) /* 1  Producer asking to realign the buffer */
#define   READY_R_ARCW3_MSB U(22) /*    "Buffer ready for read" */ 
#define   READY_R_ARCW3_LSB U(22) /* 1  (write-read)>size/2 (or /4) */
#define     WRITE_ARCW3_MSB U(21)
#define     WRITE_ARCW3_LSB U( 0) /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */

/* arcs with indexes higher than IDX_ARCS_desc, see enum_arc_index */

//#define PLATFORM_IO 0                   /* 3 bits offets code for arcs external to the graph */


#endif /* #ifndef cSTREAM_ARCS_H */
