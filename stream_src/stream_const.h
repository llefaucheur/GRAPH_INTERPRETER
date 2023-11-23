/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_const.h
 * Description:  public references for the application using CMSIS-STREAM
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

#ifndef cSTREAM_CONST_H
#define cSTREAM_CONST_H

/*------ Major-Minor version numbers ------*/
#define GRAPH_INTERPRETER_VERSION 0x0100



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
#define ARC_____UNUSED6 6u
#define ARC_____UNUSED7 7u
#define ARC_DATA_RATE 8             /* data rate estimate in DEBUG_REG_ARCW1 */
#define ARC_TIME_STAMP_LAST_ACCESS 9
#define ARC_PEAK_DATA 10            /* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW1 */
#define ARC_MEAN_DATA 11
#define ARC_MIN_DATA 12
#define ARC_ABSMEAN_DATA 13
#define ARC_DATA_TO_OTHER_ARC 14    /* when data is changing the new data is push to another arc DEBUG_REG_ARCW1=[ArcID] */
#define ARC_____UNUSED15 15
//#define ARC_LOOPBACK 15             /* automatic read/write loopback (debug trace, test-pattern, ..) */
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
                                    /*  computed by adding the frame size or buffer size/nchan, also for ring buffers  */
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

/*===================================   FORMATS   =================================================*/
#define STREAM_FORMAT_SIZE_W32 3            // digital, common part of the format 

/*
*   stream_data_stream_data_format (size multiple of 3 x uint32_t)
*       word 0 : common to all domains : frame size, raw format, interleaving
*       word 1 : common to all domains : time-stamp, sampling rate, nchan         
*       word 2 : specific to domains : hashing, direction, channel mapping 
*/

/*--------------- WORD 0 - producer frame size, raw format, interleaving --------------- */
#define SIZSFTRAW_FMT0   0
    //#define MULTIFRME_FMT0_MSB 31 /* 1  allow the scheduler to push multiframes  */
    //#define MULTIFRME_FMT0_LSB 31
    #define       RAW_FMT0_MSB 30
    #define       RAW_FMT0_LSB 25 /* 6  stream_raw_data 6bits (0..63)  */
    #define INTERLEAV_FMT0_MSB 24       
    #define INTERLEAV_FMT0_LSB 24 /* 1  interleaving : frame_format_type */
    #define FRAME_EXT_FMT0_MSB 23 
    #define FRAME_EXT_FMT0_LSB 22 /* 2  _____*/
                                  /*    frame size in bytes for one deinterleaved channel Byte-acurate up to 4MBytes */
    #define FRAMESIZE_FMT0_MSB 21 /*    raw interleaved buffer size is framesize x nb channel, max = 4MB x nchan */
    #define FRAMESIZE_FMT0_LSB  0 /* 22 in swc manifests it gives the minimum input size (grain) before activating the swc
                                        A "frame" is the combination of several channels sampled at the same time 
                                        A value =0 means the size is any or defined by the IO AL.
                                        For sensors delivering burst of data not isochronous, it gives the maximum 
                                        framesize; same comment for the sampling rate. */

    #define PACKSTREAMFORMAT0(RAW,INTERL,FRAMESIZE) ((U(RAW)<<26)|(U(INTERL)<<24)|U(FRAMESIZE))

/*--------------- WORD 1 - sampling rate , nchan  -------------*/
#define   SAMPINGNCHANM1_FMT1  U( 1)
    #define  SAMPLING_FMT1_MSB U(31) /* 23 truncated IEEE-754 Seeeeeeemmmmmmmmmmmmmmm__XXXXXXX  */
    #define  SAMPLING_FMT1_LSB U( 9) /*    FP23_E8_M15        FEDCBA9876543210FEDCBA9876543210  */
    #define    UNUSED_FMT1_MSB U( 8) /* 2   */
    #define    UNUSED_FMT1_LSB U( 7)
    #define  TIMSTAMP_FMT1_MSB U( 6) /* 2  time_stamp_format_type for time-stamped streams for each interleaved frame */
    #define  TIMSTAMP_FMT1_LSB U( 5) 
    #define   NCHANM1_FMT1_MSB U( 4)
    #define   NCHANM1_FMT1_LSB U( 0) /* 5  nb channels-1 [1..32] */


    #define PACKSTREAMFORMAT1(TIMESTAMP,SAMPLING, NCHANM1) ((U(TIMESTAMP)<<26)|(U(SAMPLING)<<5)|U(NCHANM1))
    #define FMT_FS_MAX_EXPONENT 15
    #define FMT_FS_MAX_MANTISSA 65535
    #define FMT_FS_EXPSHIFT 16
    #define HIGH_FMTQ2FS(E,M) ((M*16)/pow(2,(2*E)))
    #define LOW_FMTQ2FS(E,M)  ((M*16)/pow(2,((3*E)-7)))

/*--------------- WORD 2 -  direction, channel mapping (depends on the "stream_io_domain")------*/
#define AUDIO_MAPPING_FMT2 U( 2)
    #define AUDIO_MAPPING_FMT2_MSB U(31) /* 32 mapping of channels example of 7.1 format (8 channels): */
    #define AUDIO_MAPPING_FMT2_LSB U( 0) /*     FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..*/

#define IMU_FMT2 U( 2)
    #define IMU_MAPPING_FMT2_MSB U(31) 
    #define IMU_MAPPING_FMT2_LSB U( 0) 

#define     PICTURE_FMT2 U( 2)
    #define  UNUSED_2D_FMT2_MSB U(31) /* 12b   */
    #define  UNUSED_2D_FMT2_LSB U(20)
    #define  BORDER_2D_FMT2_MSB U(19) /* 2 pixel border 0,1,2,3   */
    #define  BORDER_2D_FMT2_LSB U(18)
    #define   RATIO_2D_FMT2_MSB U(17) /* 4 ratio to height : 1:1 4:3 16:9 16:10 5:4  */
    #define   RATIO_2D_FMT2_LSB U(14)
    #define   WIDTH_2D_FMT2_MSB U(13) /* 14 number of pixel width */
    #define   WIDTH_2D_FMT2_LSB U( 0) 



/*==========================================  ARCS  ===================================================*/

#define SIZEOF_ARCDESC_SHORT_W32 2 /* number of arcdesc words shared at STREAM_RESET time */
#define SIZEOF_ARCDESC_W32 4

#define   BUF_PTR_ARCW0    U( 0)
#define PRODUCFMT_ARCW0_MSB U(31) /* 5 bits  PRODUCER format  (intPtr_t) +[ixSTREAM_FORMAT_SIZE_W32]  */ 
#define PRODUCFMT_ARCW0_LSB U(27) /*   Graph generator gives IN/OUT arc's frame size to be the LCM of SWC "grains" */
#define BASEIDXOFFARCW0_MSB U(26) 
#define   DATAOFF_ARCW0_MSB U(26) /*   address = offset[DATAOFF] + 4xBASEIDX [Bytes] */
#define   DATAOFF_ARCW0_LSB U(24) /* 3 32/64bits offset index see idx_memory_base_offset */
#define   ________ARCW0_MSB U(23) /*   We don't know yet if the base will be extended with a 2bits shifter */
#define   ________ARCW0_LSB U(22) /* 2  or if the list of offsets must be increased */
#define   BASEIDX_ARCW0_MSB U(21) /*        */
#define  BASESIGN_ARCW0_MSB U(21) /*   buffer address 21 + sign + offset = 25 bits (+2bits margin) */
#define  BASESIGN_ARCW0_LSB U(21) /*   sign of the address with respect to the offset */
#define   BASEIDX_ARCW0_LSB U( 0) /*22 base address 22bits linear address range in WORD32 */
#define BASEIDXOFFARCW0_LSB U( 0) /*   Signed ase can address +/- 8MBytes around the offset */
                                
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
#define    EXTEND_ARCW2_MSB U(23) /*    Size/Read/Write(x64) are used with 16words chunks => extends to */
#define    EXTEND_ARCW2_LSB U(23) /* 1    256MBytes buffers for arcs used to read NN models, video players, etc */
#define   READY_W_ARCW2_MSB U(22) /*    "Buffer ready for refill" */
#define   READY_W_ARCW2_LSB U(22) /* 1  (size-write)>size/2 (or /4) */
#define      READ_ARCW2_MSB U(21) /*    data read index  Byte-acurate up to 4MBytes starting from base address */
#define      READ_ARCW2_LSB U( 0) /* 22   this is incremented by "frame_size" FRAMESIZE_FMT0  */

#define COLLISION_ARC_OFFSET_BYTE U(3) /* offset in bytes to the collision detection byte */
#define  WRIOCOLL_ARCW3    U( 3) /* write access only from the SWC producer */
#define COLLISION_ARCW3_MSB U(31) /* 8  MSB byte used to lock the SWC, loaded with arch+proc+instance ID */ 
#define COLLISION_ARCW3_LSB U(24) /*       to check node-access collision from an other processor */
#define ALIGNBLCK_ARCW3_MSB U(23) /*    producer blocked */
#define ALIGNBLCK_ARCW3_LSB U(23) /* 1  producer sets "need for data realignement"  */
#define   READY_R_ARCW3_MSB U(22) /*    "Buffer ready for read" */ 
#define   READY_R_ARCW3_LSB U(22) /* 1  (write-read)>size/2 (or /4) */
#define     WRITE_ARCW3_MSB U(21) /*    write pointer is incremented by FRAMESIZE_FMT0 */
#define     WRITE_ARCW3_LSB U( 0) /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */

/* arcs with indexes higher than IDX_ARCS_desc, see enum_arc_index */

//#define PLATFORM_IO 0                   /* 3 bits offets code for arcs external to the graph */



/* 
    Graph data format :
    -------------------SHARED FLASH (RAM POSSIBLE)-----------------
    [-1] size of the graph in Words 
    Offset_0 (idx_memory_base_offset = MBANK_GRAPH)
    [0] 27b RAM address of part/all the graph going in RAM, HW-shared MEM configuration, which part is going in RAM
    [1] number of FORMAT, size of SCRIPTS
    [2] size of LINKEDLIST, number of STREAM_INSTANCES
    [3] number of ARCS, number of DEBUG registers
    [4] list of processors (procID for the scheduler in platform_manifest) processing the graph
        The linkedList gives the architecture/procID only allowed to execute a SWC
        This table is made to exclude a group of processor to execute any SWC even if their scheduler is launched.
    [5,6] UQ8 portion of memory consumed on each long_offset[MAX_NB_MEMORY_OFFSET] 
    -------------------

    IO "stream_format_io" (2 words per IO)
        Word0: ARC ID, domain, io platform index, in/out, command parameter, format
        Word1: default 32bits mixed-signal settings bit-fields
    
    FORMAT used by the arcs (3 words each) 
        Word0: Frame size, interleaving scheme, raw data type
        Word1: Nb Chan, Sampling rate, time-stamp format
        Word2: depends on IO Domain (audio mapping, picture format, IMU interleaving, ..)

    SCRIPTS in Flash 
        Table of 16bits byte-offsets (<127 SCRIPT_LW0) to the table of byte codes
          + Byte code of all the scripts in sequence.
        Indexes 1..7 are used for shared subroutines.
        Indexes 8..127 are assigned to nodes in the header field SCRIPT_LW0
            indexes are provided in the graph 

    -----------------SHARED FLASH/RAM (FOR NEWPARAM_LW2)----------    
      
    LINKED-LIST of SWC
       minimum 4 words/SWC
       Word0: header processor/architecture, nb arcs, SWCID, arc
       Word1+n: arcs * 2
       Word2+n: nb membanks, main instance 27b address
       Word3+n: Preset, New param!, Skip length, 
          byte stream: nbparams (ALLPARAM), {tag, nbbytes, params}
       list Ends with the SWC ID 0x03FF 
    
    -----------------SHARED RAM-------------------------------
offset_descriptor
    ARC descriptors (4 words each)
       Word0: base offsetm data format, need for flush after write
       Word1: size, debug result registers
       Word2: read index, ready for read, THR size/4, flow error and debug tasks index
       Word3: write index, ready for write, need realignment flag, locking byte

offset_buffer   
    BUFFERS memory banks (internal/external/LLRAM) used for FIFO buffers 
       and used for initializations, list of PACKSWCMEM results with fields
          SIZE, ALIGNMT, SPEED,

offset_instance
    INSTANCE Pointers (27bits encoded format + bits used for synchonization at boot time and service activations)
        (indexed by INST_SSRV_CMD of SWC_COMMANDS)
    

    WORKING areas of INSTANCES

    DEBUG REGISTERS and vectors from ARC content analysis (DEBUG_REG_ARCW1)

    SERVICES_RAM shared between all instances (maximum 5kB)
        32 memory banks of 16bytes + 64bytes in normal and critical fast memory when possible
*/

/* max number of instances simultaneously reading the graph
   used to synchronize the RESET sequence in platform_al() 
   smaller than 1<< NBINSTAN_SCTRL */
#define MAX_NB_STREAM_INSTANCES 4 

/* max number of nodes installed at compilation time */
#define NB_NODE_ENTRY_POINTS 20

/* max number of application callbacks used from SWC and scripts */
#define MAX_NB_APP_CALLBACKS 4

/* -------- GRAPH[0] 27b RAM address, HW-shared MEM & RAM copy config---- 
                                   3 options :
        IO 2 words                 RAM  Flash  Flash
        FORMAT 3 words             RAM  Flash  Flash
        SCRIPTS                    RAM  Flash  Flash
        LINKED-LIST                RAM  RAM    Flash  RAM allows SWC to be desactivated
        STREAM INSTANCE 3 words    RAM  RAM    RAM
        ARC descriptors 4 words    RAM  RAM    RAM
        Debug registers, Buffers   RAM  RAM    RAM
*/
#define COPY_CONF_GRAPH0_COPY_ALL_IN_RAM 0
#define COPY_CONF_GRAPH0_FROM_LINKEDLIST 1
#define COPY_CONF_GRAPH0_FROM_STREAM_INST 2
#define COPY_CONF_GRAPH0_ALREADY_IN_RAM 3

#define PACKSHARERAMSPLIT(share,RAMsplit) ((share<<3) + RAMsplit)   // bits 27..30 (PRODUCFMT_ARCW0_LSB 27
#define ___UNUSED_GRAPH0_MSB U(31) 
#define ___UNUSED_GRAPH0_LSB U(30) // 2
#define SHAREDRAM_GRAPH0_MSB U(29) 
#define SHAREDRAM_GRAPH0_LSB U(29) // 1
#define  RAMSPLIT_GRAPH0_MSB U(28) //   COPY_CONF_GRAPH0_COPY_ALL_IN_RAM / _FROM_LINKEDLIST / _FROM_ARCDESC / _ALREADY_IN_RAM
#define  RAMSPLIT_GRAPH0_LSB U(27) // 2
#define GRAPH_RAM_OFFSET(L,G)     pack2linaddr_int(L,G[0])
#define GRAPH_RAM_OFFSET_PTR(L,G) pack2linaddr_ptr(L,G[0])

/* -------- GRAPH[1] number of FORMAT, size of SCRIPTS ---- */
#define  SCRIPTS_SIZE_GR1_MSB U(31) 
#define  SCRIPTS_SIZE_GR1_LSB U(12) /* 20 scripts size */
#define NB_IOS_MARGIN_GR1_MSB U(11) 
#define NB_IOS_MARGIN_GR1_LSB U(10) /*  2 provision for 128 IOs */
#define        NB_IOS_GR1_MSB U( 9) 
#define        NB_IOS_GR1_LSB U( 5) /*  5 Nb of I/O :  up to 32 IO streams */
#define     NBFORMATS_GR1_MSB U( 4) 
#define     NBFORMATS_GR1_LSB U( 0) /*  5 formats */

#define PACKFORMATIOSSCRIPT(LENscript,nIOs,nFMT) (((LENscript)<<SCRIPTS_SIZE_GR1_LSB) | ((nIOs)<<NB_IOS_GR1_LSB) | ((nFMT)<<NBFORMATS_GR1_LSB))
#define GRAPH_LENSCRIPT(G) (((G[1])>>10) & 0x3FFFFF)
#define GRAPH_NB_IOS(G)    (((G[1])>> 5) & 0x1F)
#define GRAPH_NBFORMAT(G)  (((G[1])>> 0) & 0x1F)

/* -------- GRAPH[2] size of LINKEDLIST, number of STREAM_INSTANCES ---- */
#define LINKEDLIST_SIZE_GR2_MSB U(26) 
#define LINKEDLIST_SIZE_GR2_LSB U( 0) /* 26 size of the linkedList with the parameters */


#define PACKLINKEDLNBINSTANCE(LinkedList,NbInstance) (((LinkedList)<<LINKEDLIST_SIZE_GR2_LSB) | (NbInstance))

/* -------- GRAPH[3] number of ARCS, number of DEBUG registers ----*/
#define ______________GR3_MSB U(31) 
#define ______________GR3_LSB U(21) /* 11   */
#define  SCRIPT_SCTRL_GR3_MSB U(20) /* SCRIPT_SCTRL_GR3_LSB+SCRIPT_SCTRL_MSB-SCRIPT_SCTRL_LSB+1) */
#define  SCRIPT_SCTRL_GR3_LSB U(15) /*  6 debug script options  */
#define DEBUGREG_SIZE_GR3_MSB U(14) 
#define DEBUGREG_SIZE_GR3_LSB U(11) /*  4 size of the debug area addressed with DEBUG_REG_ARCW1 */
#define       NB_ARCS_GR3_MSB U(10) 
#define       NB_ARCS_GR3_LSB U( 0) /* 11 up to 2K ARCs, see ARC0_LW1 */

#define PACKNBARCDEBUG(dbgScript,SizeDebug,NBarc) (((dbgScript)<<SCRIPT_SCTRL_GR3_LSB) | ((SizeDebug)<<DEBUGREG_SIZE_GR3_LSB) | (NBarc) << NB_ARCS_GR3_LSB)

/* -------- GRAPH[4] bit-field of the processors activated to process this graph ----*/
#define PROCID_ALLOWED(proc_id) ((1<<(proc_id)) & RD(S->graph[4]))



/* -------- GRAPH[5,6] UQ8(-1) (0xFF = 100%, 0x3F = 25%) portion of memory consumed on each 
    long_offset[MAX_NB_MEMORY_OFFSET] to let the application taking 
    a piece of the preallocated RAM area 
*/

#define GRAPH_HEADER_NBWORDS 7    /* GRAPH[0 .. 6] */



/*================================= STREAM_FORMAT_IO ================================
      The graph hold a table of uint32_t "stream_format_io" [LAST_IO_FUNCTION_PLATFORM]
*/

//enum input_output_command_id {  SET0COPY1_IOFMT
#define STREAM_IOFMT_SIZE_W32 2   /* one word for settings controls 
        + 1 for instance selection and mixed-signal settings placed in the BUFFER area */

#define IO_COMMAND_SET_BUFFER   0u  /* arc buffer point directly to the IO buffer: ping-pong, big buffer */
#define IO_COMMAND_DATA_MOVE    1u  /* the IO has its own buffer */
//#define IO_COMMAND_UPDATE_PTR   2u  /* intermediate data is moved in the circular buffer, pointers need to be updated */

#define RX0_TO_GRAPH            0u
#define TX1_FROM_GRAPH          1u

#define IO_IS_COMMANDER0         0u
#define IO_IS_SERVANT1           1u

#define ___UNUSED_IOFMT_MSB U(31)  
#define ___UNUSED_IOFMT_LSB U(25)  /* 7 */
#define SET0COPY1_IOFMT_MSB U(24)  
#define SET0COPY1_IOFMT_LSB U(24)  /* 1  command_id IOCOMMAND */
#define  SERVANT1_IOFMT_MSB U(23)  
#define  SERVANT1_IOFMT_LSB U(23)  /* 1  1=IO_IS_SERVANT1 */
#define  ARC0_LW1_IOFMT_MSB U(22)  
#define  ARC0_LW1_IOFMT_LSB U(12)  /* 11 initialization : buffer allocation 0:none, >0:arc descriptor index */
#define    RX0TX1_IOFMT_MSB U(11)  /*    direction of the stream */
#define    RX0TX1_IOFMT_LSB U(11)  /* 1  0 : to the graph    1 : from the graph */
#define   IOARCID_IOFMT_MSB U(10)  
#define   IOARCID_IOFMT_LSB U( 0)  /* 11  Arc */

//#define FW_IO_IDX_IOFMT_MSB U(19)  /*    fw_io_dx : stream_al/platform_computer.h <=> stream_tools/files_manifests_computer.txt */
//#define FW_IO_IDX_IOFMT_LSB U(12)  /* 8  platform_io [fw_io_idx] -> io_start(parameter) */

/* data depending of STREAM_IO_DOMAIN : mixed-signal setting, 
    + bit-field giving the configuration of io_start(setting,*,n)) for variable numbers of frames r/w */
#define SETTINGS_IOFMT2_MSB U(31)  
#define SETTINGS_IOFMT2_LSB U( 0) /* 32  second word : common (8b MSB) and mixed-signal (24b LSB) settings */

/* address of the buffer */
#define BASEIDXOFFIOFMT3_MSB BASEIDXOFFARCW0_MSB
#define   DATAOFF_IOFMT3_MSB   DATAOFF_ARCW0_MSB
#define   DATAOFF_IOFMT3_LSB   DATAOFF_ARCW0_LSB
#define   ________IOFMT3_MSB   ________ARCW0_MSB
#define   ________IOFMT3_LSB   ________ARCW0_LSB
#define   BASEIDX_IOFMT3_MSB   BASEIDX_ARCW0_MSB
#define  BASESIGN_IOFMT3_MSB  BASESIGN_ARCW0_MSB
#define  BASESIGN_IOFMT3_LSB  BASESIGN_ARCW0_LSB
#define   BASEIDX_IOFMT3_LSB   BASEIDX_ARCW0_LSB
#define BASEIDXOFFIOFMT3_LSB BASEIDXOFFARCW0_LSB

/* 
   ================================= GRAPH FORMATS =======================================
*/ 
        // #define STREAM_FORMAT_SIZE_W32 3   (see stream_arcs.h)


/* 
   ================================= GRAPH LINKED LIST =======================================
*/ 
#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */

/* number of SWC calls in sequence */
#define MAX_SWC_REPEAT 4u

#define SWC_TASK_COMPLETED 0
#define SWC_TASK_NOT_COMPLETED 1
/* =========================================================================================== */ 

        /* word 0 - main Header */

#define PRIORITY_LW0_MSB U(31) /*   RTOS instances. 1:low-latency tasks, 2:heavy background tasks */
#define PRIORITY_LW0_LSB U(30) /* 2 up to 4 instances per processors */
#define   PROCID_LW0_MSB U(29) /*   same as PROCID_PARCH (stream instance) */
#define   PROCID_LW0_LSB U(27) /* 3 execution reserved to this processor index  */  
#define   ARCHID_LW0_MSB U(26) /*   SWC_IDX=6 (filter) selection to different arch (see PLATFORM_NODE_ADDRESS) */
#define   ARCHID_LW0_LSB U(24) /* 3 execution reserved to this processor architectures */
#define   SCRIPT_LW0_MSB U(23) /*   script parameter (SWC_IDX + Before/After + verbose trace control) */
#define   SCRIPT_LW0_LSB U(17) /* 7 script ID to call before and after calling SWC */
#define   NBARCW_LW0_MSB U(16) 
#define   NBARCW_LW0_LSB U(14) /* 3  total nb arcs, streaming and metadata/control */
#define  ARCLOCK_LW0_MSB U(13) 
#define  ARCLOCK_LW0_LSB U(12) /* 2  index of the arc used to lock SWC */
#define  ARCSRDY_LW0_MSB U(11) 
#define  ARCSRDY_LW0_LSB U(10) /* 2  first arcs, data availability checked before RUN */
#define  SWC_IDX_LW0_MSB U( 9) 
#define  SWC_IDX_LW0_LSB U( 0) /* 10 0=nothing, swc index of node_entry_points[] */

///* 16 arcs per nodes, and per HW IO stream interface (streams and metadata)
//        struct stream_IO_interfaces io_stream[MAX_GRAPH_NB_IO_STREAM]; */
//#define MAX_NB_ARC_STREAM (1<<(NBARCW_LW0_MSB-NBARCW_LW0_LSB+1))


        /* word 2+n -arcs

            starting with the one used for locking, the streaming arcs, then the metadata arcs 
            arc(tx) used for locking is ARC0_LW1
        */

// #define MAX_NB_STREAM_PER_SWC 8
#define MAX_NB_STREAM_PER_SWC 4

#define ARC_RX0TX1_MASK 0x800 /* MSB gives the direction of the arc */
#define ARC_RX0TX1_CLEAR 0x7FF 

#define  DTCM_LW1_MSB U(31) /*     for relocatable scratch DTCM usage with SMP, address is changing : */
#define  DTCM_LW1_LSB U(31) /*  1  arc_index_update() pushed the DTCM address after XDM buffers */
#define XDM11_LW1_MSB U(30) 
#define XDM11_LW1_LSB U(30) /*  1  the input and output frame size of all arcs are identical */
#define DBGB1_LW1_MSB U(29) 
#define DBGB1_LW1_LSB U(28) /*  2  debug register bank for ARC1 */
#define  ARC1_LW1_MSB U(27)
#define ARC1D_LW1_LSB U(27) /*     ARC0 direction */
#define ARC1D_LW1_MSB U(27)
#define  ARC1_LW1_LSB U(16) /* 12  ARC1  11 usefull bits + 1 MSB to tell rx0tx1 */

#define __UN0_LW1_MSB U(15) 
#define __UN0_LW1_LSB U(14) /*  2   */
#define DBGB0_LW1_MSB U(13) 
#define DBGB0_LW1_LSB U(12) /*  2  debug register bank for ARC0 */
#define  ARC0_LW1_MSB U(11)
#define ARC0D_LW1_LSB U(11) /*     ARC0 direction */
#define ARC0D_LW1_MSB U(11)
#define  ARC0_LW1_LSB U( 0) /* 12  ARC0, (11 + 1 rx0tx1) up to 2K ARCs */


        /* word 2+n - memory banks */

#define PARAMETER_LW2_MSB U(31) /*      */
#define PARAMETER_LW2_LSB U(31) /*  1  '1' parameter field to read */
#define   ________LW2_MSB U(30) /*      */
#define   ________LW2_LSB U(30) /*  1   */
#define   NBALLOC_LW2_MSB U(29)
#define   NBALLOC_LW2_LSB U(27) /*  3 number of memory segments to give at RESET */
#define BASEIDXOFFLW2_MSB U(26) 
#define   DATAOFF_LW2_MSB U(26)
#define   DATAOFF_LW2_LSB DATAOFF_ARCW0_LSB /*  3 bits 64bits offset index see idx_memory_base_offset */
#define   BASEIDX_LW2_MSB BASEIDX_ARCW0_MSB /*    buffer address 21 + sign + offset = 25 bits (2bits margin) */
#define   BASEIDX_LW2_LSB U( 0)             /* 21 base address in WORD32 + 1 sign bit*/
#define BASEIDXOFFLW2_LSB U( 0) /*    27 bits */

#define MAXNBMEMSEGMENTS ((1<<(NBALLOC_LW2_MSB - NBALLOC_LW2_LSB + 1)) - 1)

        /* word 3+n - parameters */

/*      
        BOOTPARAMS (if PARAMETER_LW2=1)

        PARAM_TAG : 8  index to paramters (255='all parameters')
        unused    : 4  (extension of W32LENGTH?)
        PRESET    : 4  preset index (SWC delivery)
        W32LENGTH :16  nb of WORD32 to skip at run time, 0 means no parameter, max=256kB

        Example with 
        2  u8;  0 255                       preset, TAG = "all parameters"
        1  u8;  2                           Two biquads
        1  u8;  0                           postShift
        5 h16; 5678 2E5B 71DD 2166 70B0     b0/b1/b2/a1/a2 
        5 h16; 5678 2E5B 71DD 2166 70B0     second biquad

        0xFF000007, // 054 015 parameters
        0x56780002, // 058 016 parameters
        0x71DD2E5B, // 05C 017 parameters
        0x70B02166, // 060 018 parameters
        0x2E5B5678, // 064 019 parameters
        0x216671DD, // 068 01A parameters
        0x000070B0, // 06C 01B parameters

        SWC can use an input arc to receive a huge set of parameters, for example a NN model
        The arc read pointer is never incremented during the execution of the node.
*/
//#define MAX_TMP_PARAMETERS 30   /* temporary buffer (words32) of parameters to send to the Node */


#define PARAM_TAG_LW3_MSB U(31) 
#define PARAM_TAG_LW3_LSB U(24) /* 8  for PARAM_TAG_CMD (255='all parameters')  */
#define   _UNUSED_LW3_MSB U(23)       
#define   _UNUSED_LW3_LSB U(20) /* 4  _______ */
#define    PRESET_LW3_MSB U(19)
#define    PRESET_LW3_LSB U(16) /* 4  preset   16 precomputed configurations */
#define W32LENGTH_LW3_MSB U(15) /*    if >4MB are needed then use an arc to a buffer */
#define W32LENGTH_LW3_LSB U( 0) /* 16 skip this : number of uint32 to skip the boot parameters */

/*  nbparam = FF means "full set of parameters loaded from binary format" */
#define ALLPARAM_ ((1<<(1+ PARAM_TAG_LW3_MSB- PARAM_TAG_LW3_LSB))-1)  

/* ================================= */

/* last word has SWC index 0b11111..111 */
#define GRAPH_LAST_WORD_MSB SWC_IDX_LW0_MSB
#define GRAPH_LAST_WORD_LSB SWC_IDX_LW0_LSB
#define GRAPH_LAST_WORD U((U(1)<<U(SWC_IDX_LW0_MSB- SWC_IDX_LW0_LSB+1U))-1U)

/*=====================================================================================*/                          

/*
    commands from the application, and from Stream to the SWC

    SWC_COMMANDS 
*/

#define  _UNUSED2_CMD_MSB U(31)       
#define  _UNUSED2_CMD_LSB U(24) /* 8 _______ */
#define   SWC_TAG_CMD_MSB U(23)       
#define   SWC_TAG_CMD_LSB U(16) /* 8 parameter, function selection / debug arc index */
#define    PRESET_CMD_MSB U(15)       
#define    PRESET_CMD_LSB U(12) /* 4  #16 presets */
#define      NARC_CMD_MSB U(11)       
#define      NARC_CMD_LSB U( 8) /* 4 number of arcs */
#define  __UNUSED_CMD_MSB U( 7)       
#define  __UNUSED_CMD_LSB U( 4) /* 4 _______ */
#define   COMMAND_CMD_MSB U( 3)       
#define   COMMAND_CMD_LSB U( 0) /* 4 command */

#define PACK_COMMAND(SWCTAG,PRESET,NARC,CMD) (((PRESET)<<12)|((NARC)<<8)|((SWCTAG)<<16)|(CMD))

/*=====================================================================================*/    
/*
    "stream_service_command"  from the nodes, to "arm_stream_services"
*/

#define   OPTION_SSRV_MSB U(31)       
#define   OPTION_SSRV_LSB U(14) /* 18   compute accuracy, in-place processing, frame size .. */
#define INST_CMD_SSRV_MSB U(13)       
#define INST_CMD_SSRV_LSB U(10) /* 4    stream instance index RD(S->scheduler_control, INSTANCE_SCTRL) */
#define FUNCTION_SSRV_MSB U( 9)       
#define FUNCTION_SSRV_LSB U( 4) /* 6    64 functions/group  */
#define    GROUP_SSRV_MSB U( 3)       
#define    GROUP_SSRV_LSB U( 0) /* 4    16 groups */

/* clears the MALLOC_SSRV field */
#define PACK_SERVICE(OPTION,INST,FUNC,GROUP) (((OPTION)<<OPTION_SSRV_LSB)|((INST)<<INST_CMD_SSRV_LSB)|((FUNC)<<FUNCTION_SSRV_LSB)|(GROUP)<<GROUP_SSRV_LSB)
#define PACK_SERVICE_S(FUNC,GROUP) (((FUNC)<<FUNCTION_SSRV_LSB)|(GROUP)<<GROUP_SSRV_LSB)



//enum stream_command (8bits LSB)
//{
#define STREAM_RESET 1u             /* func(STREAM_RESET, *instance, * memory_results) */
#define STREAM_SET_PARAMETER 2u     /* swc instances are protected by multithread effects when changing parmeters on the fly */
#define STREAM_READ_PARAMETER 3u    
#define STREAM_RUN 4u               /* func(STREAM_RUN, instance, *in_out) */
#define STREAM_STOP 5u              /* func(STREAM_STOP, instance, 0)  swc calls free() if it used stdlib's malloc */


/*
    commands from the SWC to Stream
    16 family of commands:
    - 0 : internal to Stream, reset, debug trace, report errors
    - 1 : arc access for scripts : pointer, last data, debug fields, format changes
    - 2 : format converters (time, raw data)
    - 3 : stdlib.h subset (time, stdio)
    - 4 : math.h subset
    - 5 : Interface to CMSIS-DSP / CMSIS-NN
    - 6 : Multimedia audio library
    - 7 : Image processing library
    - 8..15 : reserved

    each family can define 256 operations (TAG_CMD_LSB)
*/
//enum stream_service_group
#define STREAM_SERVICE_INTERNAL 0
#define STREAM_SERVICE_FLOW 1
#define STREAM_SERVICE_CONVERSION 2
#define STREAM_SERVICE_STDLIB 3
#define STREAM_SERVICE_MATH 4
#define STREAM_SERVICE_DSP_ML 5
#define STREAM_SERVICE_MM_AUDIO 6
#define STREAM_SERVICE_MM_IMAGE 7

//{
    /* 0/STREAM_SERVICE_INTERNAL ------------------------------------------------ */

#define STREAM_SERVICE_INTERNAL_RESET 1u
#define STREAM_SERVICE_INTERNAL_NODE_REGISTER 2u

#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_FS 3u       /* SWC information for a change of stream format, sampling, nb of channel */
#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_NCHAN 4u     /* raw data sample, mapping of channels, (web radio use-case) */
#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_RAW 5u
#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_MAP 6u

#define STREAM_SERVICE_INTERNAL_AUDIO_ERROR 7u        /* PLC applied, Bad frame (no header, no synchro, bad data format), bad parameter */
    
#define STREAM_SERVICE_INTERNAL_DEBUG_TRACE 8u
#define STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STAMPS 9u

#define STREAM_SERVICE_INTERNAL_AVAILABLE 10u

#define STREAM_SERVICE_INTERNAL_SETARCDESC 11u  /* buffers holding MP3 songs.. rewind from script, switch a NN model to another, change a parameter-set using arcs */


    //STREAM_SERVICE_INTERNAL_DEBUG_TRACE, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_1B, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_DIGIT, 
    //STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STAMPS, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STRING,


    //STREAM_SAVE_HOT_PARAMETER, 
    //STREAM_READ_TIME, STREAM_READ_TIME_FROM_START, STREAM_TIME_DIFFERENCE, 
    //STREAM_TIME_CONVERSION,  
    //STREAM_LOW_POWER,     /* interface to low-power platform settings, "wake-me in 24h with deep-sleep in-between" */
    //STREAM_PROC_ARCH,     /* returns the processor architecture details, used before executing specific assembly codes */



    /* 1/STREAM_SERVICE_FLOW ------------------------------------------------ */
    //for scripts:
        #define STREAM_SERVICE_FLOW_ARC_RWPTR 1 
        #define STREAM_SERVICE_FLOW_ARC_DATA 2 
        #define STREAM_SERVICE_FLOW_ARC_FILLING 3

    //for scripts/Nodes: fast data moves
        #define STREAM_SERVICE_FLOW_DMA_SET 4       /* set src/dst/length */
        #define STREAM_SERVICE_FLOW_DMA_START 5
        #define STREAM_SERVICE_FLOW_DMA_STOP 6
        #define STREAM_SERVICE_FLOW_DMA_CHECK 7

    /* 2/STREAM_SERVICE_CONVERSION ------------------------------------------------ */
        #define STREAM_SERVICE_CONVERSION_INT16_FP32 1


    /* 3/STREAM_SERVICE_STDLIB ------------------------------------------------ */
        /* stdio.h */
        //STREAM_FEOF, STREAM_FFLUSH, STREAM_FGETC, STREAM_FGETS, STREAM_FOPEN, STREAM_FPRINTF, STREAM_FPUTC,
        //STREAM_FPUTS, STREAM_FREAD, STREAM_FSCANF, STREAM_FWRITE,

        /* stdlib.h */
        //STREAM_ABS, STREAM_ATOF, STREAM_ATOI, STREAM_ATOL, STREAM_ATOLL, STREAM_CALLOC, STREAM_FREE, STREAM_MALLOC, 
        //STREAM_RAND, STREAM_SRAND, STREAM_STRTOF, STREAM_STRTOL,

        /* string.h */
        //STREAM_MEMCHR, STREAM_MEMCMP, STREAM_MEMCPY, STREAM_MEMMOVE, STREAM_MEMSET, STREAM_STRCHR, STREAM_STRLEN,
        //STREAM_STRNCAT, STREAM_STRNCMP, STREAM_STRNCPY, STREAM_STRSTR, STREAM_STRTOK,


    /* 4/STREAM_SERVICE_MATH ------------------------------------------------ */

        //STREAM_SIN_FP32,  STREAM_COS_FP32, STREAM_ASIN_FP32, STREAM_ACOS_FP32, 
        //STREAM_TAN_FP32,  STREAM_ATAN_FP32, STREAM_ATAN2_FP32, 
        //STREAM_LOG10_FP32,STREAM_LOG2_FP32, STREAM_POW_FP32, STREAM_SQRT_FP32, 

        /* time.h */
        //STREAM_ASCTIMECLOCK, STREAM_DIFFTIME,
        //STREAM_TEA,

    /* 5/STREAM_SERVICE_DSP_ML ------------------------------------------------ */

#define STREAM_SERVICE_CASCADE_DF1_Q15 1    /* IIR filters */
#define STREAM_SERVICE_CASCADE_DF1_F32 2         

#define STREAM_SERVICE_INIT_rFFT_Q15   3    /* RFFT windowing, module, dB */
#define STREAM_SERVICE_rFFT_Q15        4
#define STREAM_SERVICE_INIT_rFFT_F32   5    
#define STREAM_SERVICE_rFFT_F32        6
                                       
#define STREAM_SERVICE_INIT_cFFT_Q15   7    /* cFFT windowing, module, dB */
#define STREAM_SERVICE_cFFT_Q15        8
#define STREAM_SERVICE_INIT_cFFT_F32   9         
#define STREAM_SERVICE_cFFT_F32       10
                                       
#define STREAM_SERVICE_INIT_DFT_Q15   11    /* DFT/Goertzel windowing, module, dB */
#define STREAM_SERVICE_DFT_Q15        12
#define STREAM_SERVICE_INIT_DFT_F32   13        
#define STREAM_SERVICE_DFT_F32        14

#define STREAM_SERVICE_SQRT_Q15       15
#define STREAM_SERVICE_SQRT_F32       16
#define STREAM_SERVICE_LOG_Q15        17
#define STREAM_SERVICE_LOG_F32        18

#define STREAM_SERVICE_SINE_Q15       19
#define STREAM_SERVICE_SINE_F32       20
#define STREAM_SERVICE_COS_Q15        21
#define STREAM_SERVICE_COS_F32        22
#define STREAM_SERVICE_ATAN2_Q15      23
#define STREAM_SERVICE_ATAN2_F32      24

        //#define STREAM_SERVICE_SORT 3

        //STREAM_WINDOWS,                       /* windowing for spectral estimations */
        //STREAM_FIR,
        //STREAM_MATRIX_MULT,


        //STREAM_CNORM,                         
        //STREAM_MAX,                          
        //         
        ////STREAM_CONJ,                          /* Conjugate */
        //STREAM_RFFT, STREAM_CFFT, STREAM_CIFFT,   /* FFT - there is a state and associated instance common for fft and ifft */

        //STREAM_RADD, STREAM_CADD,
        //STREAM_RSUB, STREAM_CSUB,
        //STREAM_RMUL, STREAM_CMUL,               /* element-wise real/complex vector multiply, with conjugate */
        //STREAM_RDIV, STREAM_CDIV,     
        //STREAM_RABS,                          /* absolute values of real vectors, norm-1/2/Inf of complex numbers */
        //STREAM_MAX,
        //STREAM_CNORM,                         
        //STREAM_RMAX,                          /* max between two vectors */

        //STREAM_FIR,
        //STREAM_MATRIX_MULT,
        //STREAM_2D_FILTER, 
        //STREAM_ASRC, 
        //STREAM_2D_DECIMATE, 

        //STREAM_APPLY_GAIN, 
        //STREAM_RATE_CONVERTER, 


    /* 6/STREAM_SERVICE_MM_AUDIO ------------------------------------------------ */
        /* audio Codecs */
        //STREAM_ALAW_ENC, STREAM_ALAW_DEC, STREAM_MULAW_ENC, STREAM_MULAW_DEC, STREAM_IMADPCM_ENC, STREAM_IMADPCM_DEC,
        //STREAM_LPC_ENC, STREAM_LPC_DEC,


    /* 7/STREAM_SERVICE_MM_IMAGE ------------------------------------------------ */
        /* image                   */
        //STREAM_JPEG_ENC, STREAM_JPEG_DEC, STREAM_PNG_ENC, STREAM_PNG_DEC,

//};




/*
 *  SWC manifest :
 */ 

//enum stream_node_status {
#define SWC_BUFFERS_PROCESSED 0
#define SWC_NEED_RUN_AGAIN 1         /* SWC completion type */

//enum mem_mapping_type {
#define MEM_TYPE_STATIC          0    /* (LSB) memory content is preserved (default ) */
#define MEM_TYPE_WORKING         1    /* scratch memory content is not preserved between two calls */
#define MEM_TYPE_PSEUDO_WORKING  2    /* static only during the uncompleted execution state of the SWC, see “NODE_RUN” */
#define MEM_TYPE_PERIODIC_BACKUP 3    /* static parameters to reload for warm boot after a crash, holding for example 
           long-term estimators. This memory area is cleared at cold NODE_RESET and 
           refreshed for warm NODE_RESET. The SWC should not reset it (there is 
           no "warm-boot reset" entry point. The period of backup depends on platform capabilities 
           When MBANK_BACKUP is a retention-RAM there is nothing to do, when it is standard RAM area then on periodic
            basis the AL will be call to transfer data to Flash */
  

//enum mem_speed_type                         /* memory requirements associated to enum memory_banks */
#define MEM_SPEED_REQ_ANY           0    /* best effort */
#define MEM_SPEED_REQ_NORMAL        1    /* can be external memory */
#define MEM_SPEED_REQ_FAST          2    /* will be internal SRAM when possible */
#define MEM_SPEED_REQ_CRITICAL_FAST 3    /* will be TCM when possible
           When a SWC is declaring this segment as relocatable ("RELOC_MEMREQ") it will use 
           physical address different from one TCM to an other depending on the processor running the SWC.
           The design constraint is to have ONE segment and a pointer to this segment placed 
           at the beginning of the SWC instance. To let the scheduler modify the TCM address dynamically
           before calling the SWC. 
           In all other cases the TCM area will be allocated to one processor designated in 
           the linked-list parameter "PROCID_GI".
           */
                                

//enum buffer_alignment_type            
#define MEM_REQ_NOALIGNMENT_REQ    0    /* address binary mask : */
#define MEM_REQ_2BYTES_ALIGNMENT   1    /*   mask = ~((1 << (7&mem_req_2bytes_alignment) -1) */
#define MEM_REQ_4BYTES_ALIGNMENT   2    
#define MEM_REQ_8BYTES_ALIGNMENT   3
#define MEM_REQ_16BYTES_ALIGNMENT  4
#define MEM_REQ_32BYTES_ALIGNMENT  5
#define MEM_REQ_64BYTES_ALIGNMENT  6
#define MEM_REQ_128BYTES_ALIGNMENT 7


#define SWC_CONTROLS U(4)
#define SWC_CONTROLS_NAME U(8)


//enum buffer_relocation_type
#define NOT_RELOCATABLE U(0)
#define RELOCATABLE U(1)

/*---------------------------------------------------------------------------------------------------*/
//#define  UNUSED_SWCID_MSB U(31)
//#define  UNUSED_SWCID_LSB U(31) /* 1 */
//#define      ID_SWCID_MSB U(30)
//#define      ID_SWCID_LSB U(19) /* 12 SWC ID : developer can produce 4K SWC   */
//#define   IDVER_SWCID_MSB U(18)
//#define   IDVER_SWCID_LSB U(15) /* 4 version    */
//#define   MARCH_SWCID_MSB U(14)
//#define   MARCH_SWCID_LSB U(14) /* 1 multi architecture fat binary delivery, offsets in fatbin_offsets[]    */
//#define SUBARCH_SWCID_MSB U(13)
//#define SUBARCH_SWCID_LSB U( 8) /* 6 (stream_processor_sub_arch_fpu) mapped on 4b from platform_manifest */
//#define   UARCH_SWCID_MSB U( 7)
//#define   UARCH_SWCID_LSB U( 0) /* 8 (stream_processor_architectures) mapped on 6b from platform_manifest */

/*---------------------------------------------------------------------------------------------------*/
/*
    SWC manifest
    Memory consumption estimated by interpolation 
        a.nb_channels + b.FS + c.frameSize + d.rawDataSize

    in-place processing translates to same frame size, or a copy to an extra buffer
*/

/* SWC memory allocation constraint 32bits : memory allocation description */
#define   OFFSET_MEMREQ_MSB U(31)
#define   OFFSET_MEMREQ_LSB U(31) /* 1  followed by an offset (working memory in SMP) */
#define    RELOC_MEMREQ_MSB U(30)
#define    RELOC_MEMREQ_LSB U(30) /* 1  memory segement is relocatble */
#define  INPLACE_MEMREQ_MSB U(29)
#define  INPLACE_MEMREQ_LSB U(29) /* 1  in-place processing : output buffer can be mapped on input buffer  */
#define    INMOD_MEMREQ_MSB U(28)     
#define    INMOD_MEMREQ_LSB U(28) /* 1  warning : "1" means input buffer can be modified by the component  */
#define     TYPE_MEMREQ_MSB U(27)     
#define     TYPE_MEMREQ_LSB U(26) /* 2  mem_mapping_type  static, working, pseudo_working, hot_static      */
#define    SPEED_MEMREQ_MSB U(25)     
#define    SPEED_MEMREQ_LSB U(23) /* 3  mem_speed_type "idx_memory_base_offset"   */
#define  ALIGNMT_MEMREQ_MSB U(22)     
#define  ALIGNMT_MEMREQ_LSB U(20) /* 3  buffer_alignment_type */
#define    SHIFT_MEMREQ_MSB U(19)     
#define    SHIFT_MEMREQ_LSB U(18) /* 2  size shifter : 0=0, 1=4, 2=8, 3=12 => max size 256k<<12 = 1G */
#define     SIZE_MEMREQ_MSB U(17)     
#define     SIZE_MEMREQ_LSB U( 0) /*18 size in bytes, max = 256kB */

/*---------------------------------------------------------------------------------------------------*/
#define   UNUSED_SWPARAM_MSB U(31)
#define   UNUSED_SWPARAM_LSB U(24) /* 7 */
#define   PARMSZ_SWPARAM_MSB U(23)
#define   PARMSZ_SWPARAM_LSB U( 6) /*18 parameter array size <256kB */
#define NBPRESET_SWPARAM_MSB U( 5)
#define NBPRESET_SWPARAM_LSB U( 0) /* 6 nb parameters presets (max 64) */

#define PACKSWCPARAM(PSZ,NBPST) (((PSZ)<<6)|(NBPST))

/* upon STREAM_MEMREQ commands the swc can return up to 6 memory requests for its instance 
    scratch fast, DTCM, static fast, internal L2, external */

#define MAX_NB_MEM_REQ_PER_NODE U(6)  /* table of PACKSWCMEM requirements (speed, size, alignment..) see MAXNB_MEMORY_BASE_MALLOC */

#define GRAPH_MEM_REQ (U(MEM_WORKING_INTERNAL)+U(1)) 

/*
 * -----------------------------------------------------------------------
 */

#define U(x) ((uint32_t)(x))
#define U8(x) ((uint8_t)(x))

//enum platform_al_services       
#define PLATFORM_INIT_AL           0x00   /* set the graph pointer */
#define PLATFORM_MP_GRAPH_SHARED   0x01   /* need to declare the graph area as "sharable" in S = MPU_RASR[18] */
#define PLATFORM_MP_BOOT_WAIT      0x02   /* wait commander processor copies the graph */
#define PLATFORM_MP_BOOT_DONE      0x03   /* to confirm the graph was copied in RAM */
#define PLATFORM_MP_RESET_WAIT     0x04   /* wait the graph is initialized */
#define PLATFORM_MP_RESET_DONE     0x05   /* tell the reset sequence was executed for that Stream instance */
//#define PLATFORM_MP_SERVICE_LOCK   0x06   /* collission of access to the graph at boot time */
//#define PLATFORM_MP_SERVICE_UNLOCK 0x07   /* wait commander processor copies the graph */
#define PLATFORM_IO_SET_STREAM_ALL 0x08   /* launch all the graph interfaces */
#define PLATFORM_IO_SET_STREAM     0x09   /* share &platform_io(), buffer address, *selection of setting EXTENSION/option, data format */
#define PLATFORM_IO_DATA_START           0x0A   /* "data exchanges */ 
#define PLATFORM_IO_STOP_STREAM    0x0B   /*  */
#define PLATFORM_IO_ACK            0x0C   /* interface callback to arm_stream_io */
#define PLATFORM_CLEAR_BACKUP_MEM  0x0D   /* cold start : clear backup memory */
#define PLATFORM_REMOTE_DATA       0x0E
#define PLATFORM_EXEC_TIME         0x0F   /* time counter since last call */
#define PLATFORM_ERROR             0x10   /* error to report to the application */
#define PLATFORM_OFFSETS           0x11   /* returns the pointer to the (long) platform offsets */
#define PLATFORM_NODE_ADDRESS      0x12   /* returns the physical address of a node */
#define PLATFORM_PROC_HW           0x13   /* who am i ? and which ISR is connected to NVIC (iomask) */
#define PLATFORM_IO_STRUCT         0x14   /* share the struct platform_io_control table  */

/*
* system subroutines : 
* - IO settings : 
* - Get Time, in different formats, and conversion, extract time-stamps
* - Get Peripheral data : RSSI, MAC/IP address
* - Low-level : I2C string of commands, GPIO, physical address to perpherals
*/
#define PLATFORM_DEEPSLEEP_ENABLED 0x20   /* deep-sleep activation is possible when returning from arm_stream(STREAM_RUN..) */
#define PLATFORM_TIME_SET          0x21
#define PLATFORM_RTC_SET           0x22
#define PLATFORM_TIME_READ         0x23
#define PLATFORM_HW_WORD_READ      0x24  
#define PLATFORM_HW_WORD_WRITE     0x25  
#define PLATFORM_HW_BYTE_READ      0x26  
#define PLATFORM_HW_BYTE_WRITE     0x27  

//enum error_codes 
#define ERROR_MEMORY_ALLOCATION     1u


/* ----------------- PLATFORM_IO_MANIFEST ------------------------*/


/*
    STREAM SERVICES

*/

#define  UNUSED_SRV_MSB U(31)
#define  UNUSED_SRV_LSB U(16) /* 16 reserved */
#define    INST_SRV_MSB U(15)       
#define    INST_SRV_LSB U(12) /* 4  instance */
#define   GROUP_SRV_MSB U(11)       
#define   GROUP_SRV_LSB U( 8) /* 4  command family groups under compilation options (DSP, Codec, Stdlib, ..) */
#define COMMAND_SRV_MSB U( 7)       
#define COMMAND_SRV_LSB U( 0) /* 8  256 service IDs */


/*
    Up to 16 family of processing extensions "SERVICE_COMMAND_GROUP"
    EXTDSPML EXTMATH EXTAUDIO EXTIMAGE EXTSTDLIB
*/

#define EXT_SERVICE_MATH   1
#define EXT_SERVICE_DSPML  2
#define EXT_SERVICE_AUDIO  3
#define EXT_SERVICE_IMAGE  4 
#define EXT_SERVICE_STDLIB 5




/*============================ BIT-FIELDS MANIPULATIONS ============================*/
/*
 *  stream constants / Macros.
 */
 
#define SHIFT_SIZE(base,shift) ((base) << ((shift) << 2));           


#define MIN(a, b) (((a) > (b))?(b):(a))
#define MAX(a, b) (((a) < (b))?(b):(a))
#define ABS(a) (((a)>0)? (a):-(a))

#define MAXINT32 0x7FFFFFFFL
//#define MEMCPY(dst,src,n) {void *x; uint32_t i; x=memcpy((dst),(src),(n));}
#define MEMCPY(dst,src,n) {uint32_t i; for(i=0;i<n;i++){dst[i]=src[i];}}


/* bit-field manipulations */
#define CREATE_MASK(msb, lsb)               (((U(1) << ((msb) - (lsb) + U(1))) - U(1)) << (lsb))
#define MASK_BITS(arg, msb, lsb)            ((arg) & CREATE_MASK(msb, lsb))
#define EXTRACT_BITS(arg, msb, lsb)         (MASK_BITS(arg, msb, lsb) >> (lsb))
#define INSERT_BITS(arg, msb, lsb, value) \
    ((arg) = ((arg) & ~CREATE_MASK(msb, lsb)) | (((value) << (lsb)) & CREATE_MASK(msb, lsb)))
#define MASK_FIELD(arg, field)              MASK_BITS((arg), field##_MSB, field##_LSB)

#define EXTRACT_FIELD(arg, field)           U(EXTRACT_BITS((U(arg)), field##_MSB, field##_LSB))
#define RD(arg, field) U(EXTRACT_FIELD(arg, field))

#define INSERT_FIELD(arg, field, value)     INSERT_BITS((arg), field##_MSB, field##_LSB, value)
#define ST(arg, field, value) INSERT_FIELD((arg), field, U(value)) 

#define LOG2BASEINWORD32 2 
#define BASEINWORD32 (1<<LOG2BASEINWORD32)
#define PACK2LINADDR(o,x) (o[RD(x,DATAOFF_ARCW0)] + \
        (RD(x,BASESIGN_ARCW0))? \
            (1 + ~(((intPtr_t)RD((x),BASEIDX_ARCW0))<<LOG2BASEINWORD32)):\
            ( ((intPtr_t)RD((x),BASEIDX_ARCW0))<<LOG2BASEINWORD32))

#define SET_BIT(arg, bit)   ((arg) |= (U(1) << U(bit)))
#define CLEAR_BIT(arg, bit) ((arg) = U(arg) & U(~(U(1) << U(bit))))
#define TEST_BIT(arg, bit)  (U(arg) & (U(1) << U(bit)))

#define FLOAT_TO_INT(x) ((x)>=0.0f?(int)((x)+0.5f):(int)((x)-0.5f))




#endif /* cSTREAM_CONST_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
    

