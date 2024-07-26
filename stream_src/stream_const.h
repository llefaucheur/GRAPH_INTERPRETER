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

#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */

/*------ Floating point allowed ------*/
#define STREAM_FLOAT_ALLOWED 1

/*------ Major-Minor version numbers ------*/
#define GRAPH_INTERPRETER_VERSION 0x0100

/* 
  *- Graph data format :
    -------------------SHARED FLASH (RAM POSSIBLE)-----------------
    [-1] size of the graph in Words 
    Offset_0 (long_offset = MBANK_GRAPH)
    [0] 27b RAM address of part/all the graph going in RAM, which part is going in RAM
    [1] number of FORMAT, size of SCRIPTS
    [2] size of LINKEDLIST, number of STREAM_INSTANCES
    [3] number of ARCS, number of DEBUG registers
    [4] list of processors (procID for the scheduler in platform_manifest) processing the graph
        The linkedList gives the architecture/procID only allowed to execute a SWC
        This table is made to exclude a group of processor to execute any SWC even if their scheduler is launched.
    [5,6] UQ8 portion of memory consumed on each long_offset[MAX_NB_MEMORY_OFFSET] 
    -------------------
    IO settings (1 word per IO)  size = [NB_IOS_GR1]
        depends on the domain of the IO

  *- SCRIPTS are adressed with a table_int32[128] : offset, ARC, binary format
        ARC descriptor: size regs/stack, parameters/UC, collision Byte, max cycles 
        The first are indexed with the SWC header 6b index (SCRIPT_LW0) 

        Script index #0 means "disabled"  Indexes 1..up to 63 are used for shared subroutines.
        The first SWC of the graph is a SWC-script interpreting the use-case command, in cascade
          this script will call other subgraph's script and SWC with SET_PARAM. The graph compilation 
          tool generates the table of index to let SWC be found with name mangling in the graph hierarchy

        script scheduling reset = memreq = instance = raw descriptor address
            there is no XDM data,   nb arc = 1 TX
     
  *- LINKED-LIST of SWC
       minimum 5 words/SWC
       Word0  : header processor/architecture, nb arcs, SWCID, arc
       Word1+n: arcs * 2  + debug page
       Word2+n: 2xW32 : ADDR + SIZE + nb of memory segments
       Word3+n: Preset, New param!, Skip length, 
          byte stream: nbparams (ALLPARAM), {tag, nbWord32, params}
       list Ends with the SWC ID 0x03FF 
    
    -----------------SHARED RAM-------------------------------  OFFSET 0

  *-PIO "stream_format_io" (1 word per IO)  size = [NB_IOS_GR1]
        ARC ID, in/out, command parameter, domain, io platform index, on-going-flag
    
    at PIO + [NB_IOS_GR1] words : collision management (Dekker's algorithm) [COLLISION_IDX_GR2] words

    at PIO + [ARC_DEBUG_IDX_GR2] words : table of debug registers (2 words each) DEBUG REGISTERS from ARC content
        analysis (DEBUG_REG_ARCW1)
        32 memory banks of 16bytes + 64bytes in normal 

  *-FORMAT used by the arcs (4 words each stream_format)  
        Word0: Frame size, interleaving scheme, arithmetics raw data type
        Word1: time-stamp, domain, nchan, physical unit (pixel format, IMU interleaving..)
        Word2: depends on IO Domain
        Word3: depends on IO Domain

    can be used for tunable formats
        This is used the SWC generates variable frame formats (JPG decoder, MP3 decoder..)

  *-ARC descriptors (4 words each)
        Word0: base offsetm data format, need for flush after write
        Word1: size, debug result registers
        Word2: read index, ready for read, flow error and debug tasks index
        Word3: write index, ready for write, need realignment flag, locking byte
    ----------------- MEMORY BANKS -------------------------------
    BUFFERS memory banks (internal/external/LLRAM) used for FIFO buffers 
       and used for initializations, list of PACKSWCMEM results with fields
          SIZE, ALIGNMT, SPEED,

    INSTANCE Pointers (27bits encoded format + bits used for synchonization at boot time and service activations)
        (indexed by INST_SSRV_CMD of SWC_COMMANDS)
    

    WORKING areas of INSTANCES

*/


/* max number of instances simultaneously reading the graph
   used to synchronize the RESET sequence in platform_al() 
   smaller than 1<< NBINSTAN_SCTRL */
#define MAX_NB_STREAM_INSTANCES 4 


/*  AL SERVICE GROUP : TIME ------------------------------- */
#define AL_SERVICE_READ_TIME 0
    #define AL_SERVICE_READ_TIME64 1
    #define AL_SERVICE_READ_TIME32 2
    #define AL_SERVICE_READ_TIME16 3

/*  AL SERVICE GROUP : SLEEP CONTROL ---------------------- */
#define AL_SERVICE_SLEEP_CONTROL 1

/*  AL SERVICE GROUP : READ MEMORY ------------------------ */
#define AL_SERVICE_READ_MEMORY 2
    /* AL SERVICE FUNCTIONS */
    #define AL_SERVICE_READ_MEMORY_FAST_MEM_ADDRESS 1

/*  AL SERVICE GROUP : SERIAL COMMUNICATION --------------- */
#define AL_SERVICE_SERIAL_COMMUNICATION 3

/*  AL SERVICE GROUP : MUTUAL EXCLUSION ------------------- */
#define AL_SERVICE_MUTUAL_EXCLUSION 4
    /* AL SERVICE FUNCTIONS */
    #define AL_SERVICE_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP 1
    #define AL_SERVICE_MUTUAL_EXCLUSION_RD_BYTE_MP 2
    #define AL_SERVICE_MUTUAL_EXCLUSION_WR_BYTE_MP 3
    #define AL_SERVICE_MUTUAL_EXCLUSION_CLEAR_BIT_MP 4

    #ifdef _MSC_VER 
    #define DATA_MEMORY_BARRIER
    #define INSTRUCTION_SYNC_BARRIER
    #else
    #if MULTIPROCESSING == 1
    #define DATA_MEMORY_BARRIER //DMB()
    #define INSTRUCTION_SYNC_BARRIER //ISB()
    #else
    #define DATA_MEMORY_BARRIER 
    #define INSTRUCTION_SYNC_BARRIER
    #endif
    #endif

/*  AL SERVICE GROUP : IO SETTINGS -------------------------*/
#define AL_SERVICE_CHANGE_IO_SETTING 5
#define AL_SERVICE_UNUSED2 6
#define AL_SERVICE_UNUSED3 7
#define MAX_NB_APP_SERVICES 8


/* the first word of the graph holds the number of words32 */
#define GRAPH_SIZE_SKIP_WORD0 1

/* -------- GRAPH[0] 27b RAM address, HW-shared MEM & RAM copy config---- 
                                   2 options :
        IO settings                RAM  Flash
        SCRIPTS                    RAM  Flash
        LINKED-LIST                RAM  Flash  RAM allows SWC to be desactivated
        PIO                        RAM  RAM
        FORMAT 3 words             RAM  RAM
        ARC descriptors 4 words    RAM  RAM
        Debug registers, Buffers   RAM  RAM
*/

#define VID0    0
#define GRAPH_START_VID0 0

/* for pack2linaddr_ptr () */
//#define LINADDR_UNIT_BYTE   1
#define LINADDR_UNIT_W32    4
#define LINADDR_UNIT_EXTD  64

#define COPY_CONF_GR0_COPY_ALL_IN_RAM   0
#define COPY_CONF_GR0_FROM_PIO          1
#define COPY_CONF_GR0_ALREADY_IN_RAM    2

#define PACKSHARERAMSPLIT(share,RAMsplit) ((share<<3) + RAMsplit)   // bits 27..30 (PRODUCFMT_ARCW0_LSB 27
#define GR0_INDEX   1                // graph size + GR(X)
#define   _____________GR0_MSB U(31) 
#define   _____________GR0_LSB U(29) // 3 
#define       RAMSPLIT_GR0_MSB U(28) //   
#define       RAMSPLIT_GR0_LSB U(27) // 2 COPY_CONF_GR0_COPY_ALL_IN_RAM / _FROM_PIO / _ALREADY_IN_RAM
#define    GRAPH_RAM_OFFSET(L,G)     pack2linaddr_int(L,G[0], LINADDR_UNIT_W32)
#define    GRAPH_RAM_OFFSET_PTR(L,G) pack2linaddr_ptr(L,G[0], LINADDR_UNIT_W32)

/* -------- GRAPH[1] number of FORMAT, IOs, size of SCRIPTS ---- */
#define GR1_INDEX   2
#define   SCRIPTSSZW32_GR1_MSB U(31) 
#define   SCRIPTSSZW32_GR1_LSB U(12) /* 20 scripts size */
#define         NB_IOS_GR1_MSB U(11) 
#define         NB_IOS_GR1_LSB U( 5) /*  7 Nb of I/O :  up to 128 IO streams */
#define      NBFORMATS_GR1_MSB U( 4) 
#define      NBFORMATS_GR1_LSB U( 0) /*  5 formats */
#define MAX_GRAPH_NB_IO_STREAM  (1 << (NB_IOS_GR1_MSB - NB_IOS_GR1_LSB + 1))

/* -------- GRAPH[2] size of LINKEDLIST  ---- */
#define GR2_INDEX   3
#define   ARC_DEBUG_IDX_GR2_MSB U(31) 
#define   ARC_DEBUG_IDX_GR2_LSB U(22) /* 10 index of PIO to address the arc debug data */
#define LINKEDLISTSZW32_GR2_MSB U(21) 
#define LINKEDLISTSZW32_GR2_LSB U( 0) /* 22 size of the linkedList with the parameters */

/* -------- GRAPH[3] number of ARCS, scheduler control ----*/
#define GR3_INDEX   4
#define   ______________GR3_MSB U(31) 
#define   ______________GR3_LSB U(20) /* 12  */
#define    SCRIPT_SCTRL_GR3_MSB U(19) /*    SCRIPT_SCTRL_GR3_LSB+SCRIPT_SCTRL_MSB-SCRIPT_SCTRL_LSB+1) */
#define    SCRIPT_SCTRL_GR3_LSB U(14) /* 6  debug script options  */
#define    RETURN_SCTRL_GR3_MSB U(13)
#define    RETURN_SCTRL_GR3_LSB U(11) /* 3  return options (each SWC, each parse, once starving */
#define         NB_ARCS_GR3_MSB U(10) 
#define         NB_ARCS_GR3_LSB U( 0) /* 11 up to 2K ARCs, see ARC0_LW1 */

/* -------- GRAPH[4] bit-field of the processors activated to process this graph ----*/
#define GR4_INDEX   5
#define   ______________GR4_MSB U(31) 
#define   ______________GR4_LSB U( 8) /* 24  */
#define    PROC_ALLOWED_GR4_MSB U( 7) 
#define    PROC_ALLOWED_GR4_LSB U( 0) /* 8  up to eight processors selection  (PROCID_PARCH) */
#define PROCID_ALLOWED(proc_id) ((1<<(proc_id)) & RD(S->graph[4]))


/* -------- GRAPH[5,6] UQ8(-1) (0xFF = 100%, 0x3F = 25%) portion of memory consumed on each 
    long_offset[MAX_NB_MEMORY_OFFSET] to let the application taking 
    a piece of the preallocated RAM area 
*/
#define BYTE_3_MSB 31
#define BYTE_3_LSB 24
#define BYTE_2_MSB 23
#define BYTE_2_LSB 16
#define BYTE_1_MSB 15
#define BYTE_1_LSB  8
#define BYTE_0_MSB  7
#define BYTE_0_LSB  0

#define GRAPH_HEADER_NBWORDS 7    /* GRAPH[0 .. 6] */

 
//enum stream_scheduling_options
//{
#define STREAM_SCHD_RET_NO_ACTION           0    /* the decision is made by the graph */
#define STREAM_SCHD_RET_END_EACH_SWC        1    /* return to caller after each SWC calls */
#define STREAM_SCHD_RET_END_ALL_PARSED      2    /* return to caller once all SWC are parsed */
#define STREAM_SCHD_RET_END_SWC_NODATA      3    /* return to caller when all SWC are starving */
                                              
#define STREAM_SCHD_NO_SCRIPT               0 
#define STREAM_SCHD_SCRIPT_BEFORE_EACH_SWC  1    /* script is called before each SWC called */
#define STREAM_SCHD_SCRIPT_AFTER_EACH_SWC   2    /* script is called after each SWC called */
#define STREAM_SCHD_SCRIPT_END_PARSING      4    /* script is called at the end of the loop */
#define STREAM_SCHD_SCRIPT_START            8    /* script is called when starting */
#define STREAM_SCHD_SCRIPT_END              16   /* script is called before return */
#define STREAM_SCHD_SCRIPT_UNUSED           32   /* 6bits are reserved in SCRIPT_SCTRL */
//};
#define STREAM_COLD_BOOT 0u
#define STREAM_WARM_BOOT 1u         /* Reset + restore memory banks from retention */

/* number of SWC calls in sequence */
#define MAX_SWC_REPEAT 4u

#define TASKS_COMPLETED 0
#define TASKS_NOT_COMPLETED 1

#define STREAM_MAIN_INSTANCE 1

/* ----------- instance -> scheduler_control  ------------- */
#define     UNUSED_SCTRL_MSB U(31)   
#define     UNUSED_SCTRL_LSB U(18)  /* 14 */ 
#define   INSTANCE_SCTRL_MSB U(11)
#define   INSTANCE_SCTRL_LSB U(11)  /* 1 flag : 0=instances executed on this processor are disabled  */
#define   MAININST_SCTRL_MSB U(17)   
#define   MAININST_SCTRL_LSB U(17)  /* 1 main instance to set the graph at boot time */
#define   NODEEXEC_SCTRL_MSB U(16)   
#define   NODEEXEC_SCTRL_LSB U(16)  /* 1 */
#define   ENDLLIST_SCTRL_MSB U(15)   
#define   ENDLLIST_SCTRL_LSB U(15)  /* 1 endLinkedList detected */
#define   STILDATA_SCTRL_MSB U(14)   
#define   STILDATA_SCTRL_LSB U(14)  /* 1 still some_components_have_data*/
#define   NBINSTAN_SCTRL_MSB U(13)  /*   NB_SCHD_INSTANCE_GR4 */ 
#define   NBINSTAN_SCTRL_LSB U(10)  /* 4 used at boot time to reset the synchronization flags */
#define       BOOT_SCTRL_MSB U( 9)   
#define       BOOT_SCTRL_LSB U( 9)  /* 1 cold0/warm1 boot : Reset + restore memory banks from retention */
#define     SCRIPT_SCTRL_MSB U( 8)   
#define     SCRIPT_SCTRL_LSB U( 3)  /* 6 script call options bit-field (before/after SWC/loop/full) */
#define     RETURN_SCTRL_MSB U( 2)
#define     RETURN_SCTRL_LSB U( 0)  /* 3 return options (each SWC, each parse, once starving */
#define   PACK_STREAM_PARAM(M,N,B,S,R) (  \
            ((M)<<MAININST_SCTRL_LSB) |   \
            ((N)<<NBINSTAN_SCTRL_LSB) |   \
            ((B)<<    BOOT_SCTRL_LSB) |   \
            ((S)<<  SCRIPT_SCTRL_LSB) |   \
            ((R)<<  RETURN_SCTRL_LSB) )

/* ----------- instance -> whoami_ports  ------------- */
/* identification "whoami", next SWC to run*/
#define    INST_ID_PARCH_MSB U(31)
#define     WHOAMI_PARCH_MSB U(31)
#define   PRIORITY_PARCH_MSB U(31)  /* avoid locking an arc by the same processor, but different RTOS instances*/
#define   PRIORITY_PARCH_LSB U(30)  /* 2 [0..3] up to 4 instances per processors, 0=main instance at boot */
#define     PROCID_PARCH_MSB U(29)  /*   indexes from Manifest(tools) and PLATFORM_PROC_HW */
#define     PROCID_PARCH_LSB U(27)  /* 3 processor index [1..7] for this architecture 0="commander processor" */  
#define     ARCHID_PARCH_MSB U(26)
#define     ARCHID_PARCH_LSB U(24)  /* 3 [1..7] processor architectures 1="commander processor architecture" */
#define     WHOAMI_PARCH_LSB U(24)  /*   whoami used to lock a SWC to specific processor or architecture */
#define    INST_ID_PARCH_LSB U(24)  /*   8 bits identification for locks : !! corresponds with ARCHID_LW0_LSB */
#define SWC_W32OFF_PARCH_MSB U(23)  
#define SWC_W32OFF_PARCH_LSB U( 0) /* 24   offset in words to the NEXT SWC to be executed */  

/* maximum number of processors = nb_proc x nb_arch */
#define MAX_GRAPH_NB_PROCESSORS ((1<<(PROCID_PARCH_MSB-PROCID_PARCH_LSB+1))*(1<<(ARCHID_PARCH_MSB-ARCHID_PARCH_LSB+1)))

/* ----------------------------------------------------------------------------------------------------------------
    STREAM_IO_DOMAIN (s)

    enum stream_io_domain : list of stream "domains" categories, max 31 (DOMAIN_FMT1)
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).
*/
#define IO_DOMAIN_GENERAL             0  /* (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings*/
#define IO_DOMAIN_AUDIO_IN            1  /* microphone, line-in, I2S, PDM RX */
#define IO_DOMAIN_AUDIO_OUT           2  /* line-out, earphone / speaker, PDM TX, I2S, */
#define IO_DOMAIN_GPIO_IN             3  /* generic digital IO , control of relay, */
#define IO_DOMAIN_GPIO_OUT            4  /* generic digital IO , control of relay, */
#define IO_DOMAIN_MOTION              5  /* accelerometer, combined or not with pressure and gyroscope */
#define IO_DOMAIN_2D_IN               6  /* camera sensor */
#define IO_DOMAIN_2D_OUT              7  /* display, led matrix, */
#define IO_DOMAIN_ANALOG_IN           8  /* analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage */
#define IO_DOMAIN_ANALOG_OUT          9  /* D/A, position piezzo, PWM converter  */
#define IO_DOMAIN_RTC                10  /* ticks sent from a programmable timer */
#define IO_DOMAIN_USER_INTERFACE_IN  11  /* button, slider, rotary button */
#define IO_DOMAIN_USER_INTERFACE_OUT 12  /* LED, digits, display, */
#define IO_DOMAIN_PLATFORM_3         13  /*  */                             
#define IO_DOMAIN_PLATFORM_2         14  /* platform-specific #2, decoded with callbacks */                             
#define IO_DOMAIN_PLATFORM_1         15  /* platform-specific #1, decoded with callbacks */                             
#define IO_DOMAIN_MAX_NB_DOMAINS     32
                                         

/*================================= STREAM_FORMAT_IO  _PIO_  (RAM) ===================
      
      The graph hold a table of uint32_t "stream_format_io" 
        in RAM for the "on-going" flag    
*/
#define STREAM_IOFMT_SIZE_W32 1   /* one word for controls  */

#define MAX_IO_FUNCTION_PLATFORM 127 /* table of functions : platform_io[MAX_IO_FUNCTION_PLATFORM] */
                              
#define IO_COMMAND_SET_BUFFER 0u  /* arc buffer point directly to the IO buffer: ping-pong, big buffer */
#define IO_COMMAND_DATA_MOVE  1u  /* the IO has its own buffer */
                              
#define RX0_TO_GRAPH          0u
#define TX1_FROM_GRAPH        1u
                              
#define IO_IS_COMMANDER0      0u
#define IO_IS_SERVANT1        1u

#define    ONGOING_IOFMT_MSB 31u  
#define    ONGOING_IOFMT_LSB 31u  /* 1 set in scheduler, reset in IO, iomask manages processor access */
#define ____UNUSED_IOFMT_MSB 30u  
#define ____UNUSED_IOFMT_LSB 27u  /* 4 */
#define    FWIOIDX_IOFMT_MSB 26u  
#define    FWIOIDX_IOFMT_LSB 20u  /* 7 */
#define  IO_DOMAIN_IOFMT_MSB 19u  /*    the domain should match with the arc prod/cons format */
#define  IO_DOMAIN_IOFMT_LSB 15u  /* 5  32 Domains, to select the format of the tuning word2 */
#define FROMIOBUFF_IOFMT_MSB 14u   
#define FROMIOBUFF_IOFMT_LSB 14u  /* 1  share the arc buffer with the IO BSP "io_buffer_allocation" */
#define  SET0COPY1_IOFMT_MSB 13u  
#define  SET0COPY1_IOFMT_LSB 13u  /* 1  command_id IO_COMMAND_SET_BUFFER / IO_COMMAND_DATA_MOVE */
#define   SERVANT1_IOFMT_MSB 12u  
#define   SERVANT1_IOFMT_LSB 12u  /* 1  1=IO_IS_SERVANT1 */
#define     RX0TX1_IOFMT_MSB 11u  /*    direction of the stream */
#define     RX0TX1_IOFMT_LSB 11u  /* 1  0 : to the graph    1 : from the graph */
#define    IOARCID_IOFMT_MSB 10u 
#define    IOARCID_IOFMT_LSB  0u  /* 11  ARC*/


/* ==========================================================================================

    IO_DOMAIN physical types and tuning : used to insert extra conversion nodes 
                                          during the graph compilation 

   ==========================================================================================
*/

/* IO_DOMAIN_GENERAL           : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2  */
    #define STREAM_SUBT_GENERAL          0
    #define STREAM_SUBT_GENERAL_COMP195X 1  /* compressed byte stream following RFC1950 / RFC1951 ("deflate") */
    #define STREAM_SUBT_GENERAL_DPCM     2  /* compressed byte stream */
    #define STREAM_SUBT_GENERAL_JSON     3  /* JSON */
    #define STREAM_SUBT_GENERAL_XFORMAT  4  /* SensorThings MultiDatastream extension */

/* IO_DOMAIN_AUDIO_IN          : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
    #define STREAM_SUBT_AUDIO_IN        0   /* no subtype_units : integer/ADC format  */
    #define STREAM_SUBT_AUDIO_MPG       0   /* compressed byte stream */

/* IO_DOMAIN_AUDIO_OUT         : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
    #define STREAM_SUBT_AUDIO_OUT       0   /* no subtype_units : integer/DAC format  */

/* IO_DOMAIN_GPIO_IN           : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
/* IO_DOMAIN_GPIO_OUT          : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */

   #define STREAM_SUBT_GPIO_IN     0   /* no subtype_units  */
   #define STREAM_SUBT_GPIO_OUT    0   /* no subtype_units  */

/* IO_DOMAIN_MOTION_IN         : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
   #define STREAM_SUBT_MOTION_A     1
   #define STREAM_SUBT_MOTION_G     2
   #define STREAM_SUBT_MOTION_B     3
   #define STREAM_SUBT_MOTION_AG    4
   #define STREAM_SUBT_MOTION_AB    5
   #define STREAM_SUBT_MOTION_GB    6
   #define STREAM_SUBT_MOTION_AGB   7

/* IO_DOMAIN_2D_IN             : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
/* IO_DOMAIN_2D_OUT            : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
   #define STREAM_SUBT_2D_YUV420P   1  /* Luminance, Blue projection, Red projection, 6 bytes per 4 pixels, reordered */
   #define STREAM_SUBT_2D_YUV422P   2  /* 8 bytes per 4 pixels, or 16bpp, Y0 Cb Y1 Cr (1 Cr & Cb sample per 2x1 Y samples) */
   #define STREAM_SUBT_2D_YUV444P   3  /* 12 bytes per 4 pixels, or 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
   #define STREAM_SUBT_2D_CYM24     4  /* cyan yellow magenta */
   #define STREAM_SUBT_2D_CYMK32    5  /* cyan yellow magenta black */
   #define STREAM_SUBT_2D_RGB8      6  /* RGB  3:3:2,  8bpp, (msb)2B 3G 3R(lsb) */
   #define STREAM_SUBT_2D_RGB16     7  /* RGB  5:6:5, 16bpp, (msb)5R 6G 5B(lsb) */
   #define STREAM_SUBT_2D_RGBA16    8  /* RGBA 4:4:4:4 32bpp (msb)4R */
   #define STREAM_SUBT_2D_RGB24     9  /* BBGGRR 24bpp (msb)8B */
   #define STREAM_SUBT_2D_RGBA32   10  /* BBGGRRAA 32bpp (msb)8B */
   #define STREAM_SUBT_2D_RGBA8888 11  /* AABBRRGG OpenGL/PNG format R=lsb A=MSB ("ABGR32" little endian) */
   #define STREAM_SUBT_2D_BW1B     12  /* Y, 1bpp, 0 is black, 1 is white */
   #define STREAM_SUBT_2D_GREY2B   13  /* Y, 2bpp, 0 is black, 3 is white, ordered from lsb to msb  */
   #define STREAM_SUBT_2D_GREY4B   14  /* Y, 4bpp, 0 is black, 15 is white, ordered from lsb to msb */
   #define STREAM_SUBT_2D_GREY8B   15  /* Grey 8b, 0 is black, 255 is white */


/* IO_DOMAIN_ANALOG_IN     : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
/* IO_DOMAIN_ANALOG_OUT : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
   #define STREAM_SUBT_ANA_ANY             0 /*        any                        */        
   #define STREAM_SUBT_ANA_METER           1 /* m         meter                   */
   #define STREAM_SUBT_ANA_KGRAM           2 /* kg        kilogram                */
   #define STREAM_SUBT_ANA_GRAM            3 /* g         gram*                   */
   #define STREAM_SUBT_ANA_SECOND          4 /* s         second                  */
   #define STREAM_SUBT_ANA_AMPERE          5 /* A         ampere                  */
   #define STREAM_SUBT_ANA_KELVIB          6 /* K         kelvin                  */
   #define STREAM_SUBT_ANA_CANDELA         7 /* cd        candela                 */
   #define STREAM_SUBT_ANA_MOLE            8 /* mol       mole                    */
   #define STREAM_SUBT_ANA_HERTZ           9 /* Hz        hertz                   */
   #define STREAM_SUBT_ANA_RADIAN         10 /* rad       radian                  */
   #define STREAM_SUBT_ANA_STERADIAN      11 /* sr        steradian               */
   #define STREAM_SUBT_ANA_NEWTON         12 /* N         newton                  */
   #define STREAM_SUBT_ANA_PASCAL         13 /* Pa        pascal                  */
   #define STREAM_SUBT_ANA_JOULE          14 /* J         joule                   */
   #define STREAM_SUBT_ANA_WATT           15 /* W         watt                    */
   #define STREAM_SUBT_ANA_COULOMB        16 /* C         coulomb                 */
   #define STREAM_SUBT_ANA_VOLT           17 /* V         volt                    */
   #define STREAM_SUBT_ANA_FARAD          18 /* F         farad                   */
   #define STREAM_SUBT_ANA_OHM            19 /* Ohm       ohm                     */
   #define STREAM_SUBT_ANA_SIEMENS        20 /* S         siemens                 */
   #define STREAM_SUBT_ANA_WEBER          21 /* Wb        weber                   */
   #define STREAM_SUBT_ANA_TESLA          22 /* T         tesla                   */
   #define STREAM_SUBT_ANA_HENRY          23 /* H         henry                   */
   #define STREAM_SUBT_ANA_CELSIUSDEG     24 /* Cel       degrees Celsius         */
   #define STREAM_SUBT_ANA_LUMEN          25 /* lm        lumen                   */
   #define STREAM_SUBT_ANA_LUX            26 /* lx        lux                     */
   #define STREAM_SUBT_ANA_BQ             27 /* Bq        becquerel               */
   #define STREAM_SUBT_ANA_GRAY           28 /* Gy        gray                    */
   #define STREAM_SUBT_ANA_SIVERT         29 /* Sv        sievert                 */
   #define STREAM_SUBT_ANA_KATAL          30 /* kat       katal                   */
   #define STREAM_SUBT_ANA_METERSQUARE    31 /* m2        square meter (area)     */
   #define STREAM_SUBT_ANA_CUBICMETER     32 /* m3        cubic meter (volume)    */
   #define STREAM_SUBT_ANA_LITER          33 /* l         liter (volume)                               */
   #define STREAM_SUBT_ANA_M_PER_S        34 /* m/s       meter per second (velocity)                  */
   #define STREAM_SUBT_ANA_M_PER_S2       35 /* m/s2      meter per square second (acceleration)       */
   #define STREAM_SUBT_ANA_M3_PER_S       36 /* m3/s      cubic meter per second (flow rate)           */
   #define STREAM_SUBT_ANA_L_PER_S        37 /* l/s       liter per second (flow rate)*                */
   #define STREAM_SUBT_ANA_W_PER_M2       38 /* W/m2      watt per square meter (irradiance)           */
   #define STREAM_SUBT_ANA_CD_PER_M2      39 /* cd/m2     candela per square meter (luminance)         */
   #define STREAM_SUBT_ANA_BIT            40 /* bit       bit (information content)                    */
   #define STREAM_SUBT_ANA_BIT_PER_S      41 /* bit/s     bit per second (data rate)                   */
   #define STREAM_SUBT_ANA_LATITUDE       42 /* lat       degrees latitude[1]                          */
   #define STREAM_SUBT_ANA_LONGITUDE      43 /* lon       degrees longitude[1]                         */
   #define STREAM_SUBT_ANA_PH             44 /* pH        pH value (acidity; logarithmic quantity)     */
   #define STREAM_SUBT_ANA_DB             45 /* dB        decibel (logarithmic quantity)               */
   #define STREAM_SUBT_ANA_DBW            46 /* dBW       decibel relative to 1 W (power level)        */
   #define STREAM_SUBT_ANA_BSPL           47 /* Bspl      bel (sound pressure level; log quantity)     */
   #define STREAM_SUBT_ANA_COUNT          48 /* count     1 (counter value)                            */
   #define STREAM_SUBT_ANA_PER            49 /* /         1 (ratio e.g., value of a switch; [2])       */
   #define STREAM_SUBT_ANA_PERCENT        50 /* %         1 (ratio e.g., value of a switch; [2])*      */
   #define STREAM_SUBT_ANA_PERCENTRH      51 /* %RH       Percentage (Relative Humidity)               */
   #define STREAM_SUBT_ANA_PERCENTEL      52 /* %EL       Percentage (remaining battery energy level)  */
   #define STREAM_SUBT_ANA_ENERGYLEVEL    53 /* EL        seconds (remaining battery energy level)     */
   #define STREAM_SUBT_ANA_1_PER_S        54 /* 1/s       1 per second (event rate)                    */
   #define STREAM_SUBT_ANA_1_PER_MIN      55 /* 1/min     1 per minute (event rate, "rpm")*            */
   #define STREAM_SUBT_ANA_BEAT_PER_MIN   56 /* beat/min  1 per minute (heart rate in beats per minute)*/
   #define STREAM_SUBT_ANA_BEATS          57 /* beats     1 (Cumulative number of heart beats)*        */
   #define STREAM_SUBT_ANA_SIEMPERMETER   58 /* S/m       Siemens per meter (conductivity)             */
   #define STREAM_SUBT_ANA_BYTE           59 /* B         Byte (information content)                   */
   #define STREAM_SUBT_ANA_VOLTAMPERE     60 /* VA        volt-ampere (Apparent Power)                 */
   #define STREAM_SUBT_ANA_VOLTAMPERESEC  61 /* VAs       volt-ampere second (Apparent Energy)         */
   #define STREAM_SUBT_ANA_VAREACTIVE     62 /* var       volt-ampere reactive (Reactive Power)        */
   #define STREAM_SUBT_ANA_VAREACTIVESEC  63 /* vars      volt-ampere-reactive second (Reactive Energy)*/
   #define STREAM_SUBT_ANA_JOULE_PER_M    64 /* J/m       joule per meter (Energy per distance)        */
   #define STREAM_SUBT_ANA_KG_PER_M3      65 /* kg/m3     kg/m3 (mass density, mass concentration)     */
   #define STREAM_SUBT_ANA_DEGREE         66 /* deg       degree (angle)*                              */
   #define STREAM_SUBT_ANA_NTU            67 /* NTU       Nephelometric Turbidity Unit                 */

   // Secondary Unit (rfc8798)           Description          SenML Unit     Scale     Offset 
   #define STREAM_SUBT_ANA_MS             68 /* millisecond                  s      1/1000    0       1ms = 1s x [1/1000] */
   #define STREAM_SUBT_ANA_MIN            69 /* minute                       s      60        0        */
   #define STREAM_SUBT_ANA_H              70 /* hour                         s      3600      0        */
   #define STREAM_SUBT_ANA_MHZ            71 /* megahertz                    Hz     1000000   0        */
   #define STREAM_SUBT_ANA_KW             72 /* kilowatt                     W      1000      0        */
   #define STREAM_SUBT_ANA_KVA            73 /* kilovolt-ampere              VA     1000      0        */
   #define STREAM_SUBT_ANA_KVAR           74 /* kilovar                      var    1000      0        */
   #define STREAM_SUBT_ANA_AH             75 /* ampere-hour                  C      3600      0        */
   #define STREAM_SUBT_ANA_WH             76 /* watt-hour                    J      3600      0        */
   #define STREAM_SUBT_ANA_KWH            77 /* kilowatt-hour                J      3600000   0        */
   #define STREAM_SUBT_ANA_VARH           78 /* var-hour                     vars   3600      0        */
   #define STREAM_SUBT_ANA_KVARH          79 /* kilovar-hour                 vars   3600000   0        */
   #define STREAM_SUBT_ANA_KVAH           80 /* kilovolt-ampere-hour         VAs    3600000   0        */
   #define STREAM_SUBT_ANA_WH_PER_KM      81 /* watt-hour per kilometer      J/m    3.6       0        */
   #define STREAM_SUBT_ANA_KIB            82 /* kibibyte                     B      1024      0        */
   #define STREAM_SUBT_ANA_GB             83 /* gigabyte                     B      1e9       0        */
   #define STREAM_SUBT_ANA_MBIT_PER_S     84 /* megabit per second           bit/s  1000000   0        */
   #define STREAM_SUBT_ANA_B_PER_S        85 /* byteper second               bit/s  8         0        */
   #define STREAM_SUBT_ANA_MB_PER_S       86 /* megabyte per second          bit/s  8000000   0        */
   #define STREAM_SUBT_ANA_MV             87 /* millivolt                    V      1/1000    0        */
   #define STREAM_SUBT_ANA_MA             88 /* milliampere                  A      1/1000    0        */
   #define STREAM_SUBT_ANA_DBM            89 /* decibel rel. to 1 milliwatt  dBW    1       -30     0 dBm = -30 dBW       */
   #define STREAM_SUBT_ANA_UG_PER_M3      90 /* microgram per cubic meter    kg/m3  1e-9      0        */
   #define STREAM_SUBT_ANA_MM_PER_H       91 /* millimeter per hour          m/s    1/3600000 0        */
   #define STREAM_SUBT_ANA_M_PER_H        92 /* meterper hour                m/s    1/3600    0        */
   #define STREAM_SUBT_ANA_PPM            93 /* partsper million             /      1e-6      0        */
   #define STREAM_SUBT_ANA_PER_100        94 /* percent                      /      1/100     0        */
   #define STREAM_SUBT_ANA_PER_1000       95 /* permille                     /      1/1000    0        */
   #define STREAM_SUBT_ANA_HPA            96 /* hectopascal                  Pa     100       0        */
   #define STREAM_SUBT_ANA_MM             97 /* millimeter                   m      1/1000    0        */
   #define STREAM_SUBT_ANA_CM             98 /* centimeter                   m      1/100     0        */
   #define STREAM_SUBT_ANA_KM             99 /* kilometer                    m      1000      0        */
   #define STREAM_SUBT_ANA_KM_PER_H      100 /* kilometer per hour           m/s    1/3.6     0        */
                                                                                                 
   #define STREAM_SUBT_ANA_GRAVITY       101 /* earth gravity                m/s2   9.81      0       1g = m/s2 x 9.81     */
   #define STREAM_SUBT_ANA_DPS           102 /* degrees per second           1/s    360       0     1dps = 1/s x 1/360     */   
   #define STREAM_SUBT_ANA_GAUSS         103 /* Gauss                        Tesla  10-4      0       1G = Tesla x 1/10000 */
   #define STREAM_SUBT_ANA_VRMS          104 /* Volt rms                     Volt   0.707     0    1Vrms = 1Volt (peak) x 0.707 */
   #define STREAM_SUBT_ANA_MVPGAUSS      105 /* Hall effect, mV/Gauss        millivolt 1      0    1mV/Gauss                    */

/* IO_DOMAIN_RTC               : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */

/* IO_DOMAIN_USER_INTERFACE_IN    11 : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */

/* IO_DOMAIN_USER_INTERFACE_OUT   12 : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */


/* 
   ================================= stream_format  FORMATS =======================================
    
    Format 23+4_offsets for buffer BASE ADDRESS
    Frame SIZE and ring indexes are using 22bits linear (0..4MB)
*/

#define STREAM_FORMAT_SIZE_W32 4     /*  digital, common part of the format  */
/*
*   STREAM_DATA_START_data_format (size multiple of 3 x uint32_t)
*   _FMT0 word 0 : common to all domains : frame size, raw format, interleaving
*   _FMT1 word 1 : common to all domains : time-stamp, sampling rate, nchan         
*   _FMT2 word 2 : specific to domains : hashing, channel mapping 
*   _FMT3 word 3 : specific to domains : hashing, channel mapping 
*/

/* for MISRA-2012 compliance to Rule 10.4 */
#define U(x) ((uint32_t)(x)) 
#define U8(x) ((uint8_t)(x)) 
#define S8(x) ((int8_t)(x)) 

//enum time_stamp_format_type {
#define NO_TS 0
#define ABS_TS 1                     /* long time reference stream_time64 */
#define REL_TS 2                     /* time difference from previous frame packet in stream_time_seconds format */
#define COUNTER_TS 3                 /* 32bits counter of data frames */

//enum hashing_type {
#define NO_HASHING 0                 /* cipher protocol under definition */
#define HASHING_ON 1                 /* the stream is ciphered (HMAC-SHA256 / stream_encipher XTEA)*/

//enum frame_format_type {
#define FMT_INTERLEAVED 0           /* "arc_descriptor_interleaved" for example L/R audio or IMU stream..   */
                                    /* the pointer associated to the stream points to data (L/R/L/R/..)     */
#define FMT_DEINTERLEAVED_1PTR 1    /* single pointer to the first channel, next channel base address is    */
                                    /*  computed by adding the frame size or buffer size/nchan, also for ring buffers  */
////+enum frame_format_synchro {
#define SYNCHRONOUS 0               /* tells the output buffer size is NOT changing */
#define ASYNCHRONOUS 1              /* tells the output frame length is variable, input value "Size" tells the maximum value  
//                                       data format : optional time-stamp (stream_time_stamp_format_type)
//                                                    domain [2bits] and sub-domain [6bits rfc8428]
//                                                    payload : [nb samples] [samples]  */
////enum direction_rxtx {
#define IODIRECTION_RX 0              /* RX from the Graph pont of view */
#define IODIRECTION_TX 1


/*--------------- WORD 0 - frame size, --------------- */
#define SIZSFT_FMT0   0           /*    frame size in bytes for one deinterleaved channel Byte-acurate up to 4MBytes       */
                                  /*    raw interleaved buffer size is framesize x nb channel, max = 4MB x nchan           */
    #define __________FMT0_MSB 31 /*    in swc manifests it gives the minimum input size (grain) before activating the swc */
    #define __________FMT0_LSB 22 /*    A "frame" is the combination of several channels sampled at the same time          */
    #define FRAMESIZE_FMT0_MSB 21 /*    A value =0 means the size is any or defined by the IO AL.                          */
    #define FRAMESIZE_FMT0_LSB  0 /* 22 For sensors delivering burst of data not isochronous, it gives the maximum         */
                                  /*    framesize; same comment for the sampling rate.                                     */
                                  /*    The frameSize is including the time-stamp field                                    */
                                  /*    The ARCs are extending the frame size based on EXTEND_ARCW2 up to 256MB            */

/*--------------- WORD 1 - time-stamp, raw format, interleaving, nchan  -------------*/
#define   NCHANM1DOMAIN_FMT1  U( 1)
    #define    _______FMT1_MSB  31 /*     */  
    #define    _______FMT1_LSB  16 /* 16  */
    #define       RAW_FMT1_MSB  15
    #define       RAW_FMT1_LSB  10 /* 6  arithmetics stream_raw_data 6bits (0..63)  */
    #define  TSTPSIZE_FMT1_MSB   9 
    #define  TSTPSIZE_FMT1_LSB   8 /* 2  16/32/64/64TXT time-stamp time format */
    #define  TIMSTAMP_FMT1_MSB   7 
    #define  TIMSTAMP_FMT1_LSB   6 /* 2  time_stamp_format_type for time-stamped streams for each interleaved frame */
    #define INTERLEAV_FMT1_MSB   5       
    #define INTERLEAV_FMT1_LSB   5 /* 1  interleaving : frame_format_type */
    #define   NCHANM1_FMT1_MSB   4 
    #define   NCHANM1_FMT1_LSB   0 /* 5  nb channels-1 [1..32] */

/*  WORD2 content is domain-dependent */
    //  for format with FS (except 2D)
    /*--------------- WORD 2 -------------*/
    #define      FS1D_FMT2_MSB  31 /* 24 truncated IEEE-754 Seee.eeee.mmmmmmmmmmmmmmmm.XXXX.XXXX , 0 means "asynchronous" or "any" */
    #define      FS1D_FMT2_LSB   8 /*    FP24_E8_M16        FEDC.BA98.76543210FEDCBA98.7654.3210  */
    #define _______1D_FMT2_MSB   7
    #define _______1D_FMT2_LSB   0 /* 8   */
    /*--------------- WORD 3 -------*/
    #define   ________FMT3_MSB U(31) /* 8b   */
    #define   ________FMT3_LSB U(24) 
    #define   MAPPING_FMT3_MSB U(23) /* 24 mapping of channels example of 7.1 format (8 channels): */
    #define   MAPPING_FMT3_LSB U( 0) /*     FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..*/

    
   
    // IO_DOMAIN_2D_IN              camera sensor */
    // IO_DOMAIN_2D_OUT             display, led matrix, */
    /*--------------- WORD 2 -------------*/
    #define      FS2D_FMT2_MSB  31 /* 16 truncated IEEE-754 Seee.eeee.mmmm.mmmm.XXXX.XXXX.XXXX.XXXX , 0 means "asynchronous" or "any" */
    #define      FS2D_FMT2_LSB  16 /*    FP16_E8_M8         FEDC.BA98.7654.3210.FEDC.BA98.7654.3210  */
    #define _______2D_FMT2_MSB  15
    #define _______2D_FMT2_LSB   4 /* 12   */
    #define  RATIO_2D_FMT2_MSB   3
    #define  RATIO_2D_FMT2_LSB   0 /* 4  image aspect ratio (see below) */
    enum ratio_2d_fmt2 { R2D_TBD=0, R2D_1_1=1, R2D_4_3=2, R2D_3_4=3, R2D_16_9=4, R2D_9_16=5, R2D_3_2=6, R2D_2_3=7 };

    /*--------------- WORD 3 -------------*/
    #define  I2D_IN_BORDER_FMT3_MSB U(25) /* 2 pixel border 0,1,2,3   */
    #define  I2D_IN_BORDER_FMT3_LSB U(24)
    #define  I2D_IN_HEIGHT_FMT3_MSB U(23) /* 12 pixel height */
    #define  I2D_IN_HEIGHT_FMT3_LSB U(12)
    #define   I2D_IN_WIDTH_FMT3_MSB U(11) /* 12 pixel width */
    #define   I2D_IN_WIDTH_FMT3_LSB U( 0) 

    #define I2D_OUT_HEIGHT_FMT3_MSB U(23) /* 12 pixel height */
    #define I2D_OUT_HEIGHT_FMT3_LSB U(12)
    #define  I2D_OUT_WIDTH_FMT3_MSB U(11) /* 12 pixel width */
    #define  I2D_OUT_WIDTH_FMT3_LSB U( 0) 


    
    /*  DOMAIN_FMT1 = (MixedFormats)  IO_DOMAIN_MOTION_IN  */
    /*--------------- WORD 2 -------------*/
    /*--------------- WORD 3 -------------*/
    //enum imu_channel_format /* uint8_t : data interleaving possible combinations */
    #define aXg0m0 1                            /* only accelerometer */
    #define a0gXm0 2                            /* only gyroscope */
    #define a0g0mX 3                            /* only magnetometer */
    #define aXgXm0 4                            /* A + G */
    #define aXg0mX 5                            /* A + M */
    #define a0gXmX 6                            /* G + M */
    #define aXgXmX 7                            /* A + G + M */

    #define MOTION_IN_MAPPING_FMT3_MSB U(31) 
    #define MOTION_IN_MAPPING_FMT3_LSB U( 3) 
    #define MOTION_IN_DATAFMT_FMT3_MSB U( 2)    /* imu_channel_format : A, A+G, A+G+M, ..*/
    #define MOTION_IN_DATAFMT_FMT3_LSB U( 0) 



/* 
   =================================     SCRIPTS     =======================================

    script offset table[7b] to the byte codes : one W32 per script 
    Max 128 indexes : offset, byteCode Format
*/

#define SCRIPT_REGSIZE 8            /* 8 bytes per register */

#define INDEX_SCRIPT_STRUCT_SIZE 1
#define INDEX_SCRIPT_OFFSET 0
#define INDEX_SCRIPT_SIZE 1

#define     ARC_SCROFF0_MSB U(31) /* 11 arc descriptor */
#define     ARC_SCROFF0_LSB U(21) /*                   */
#define  FORMAT_SCROFF0_MSB U(20) /* 3  byte codes format = 0, 7 binary native architecture ARCHID_LW0 */
#define  FORMAT_SCROFF0_LSB U(18) /*       ARMv6-M */
#define  SHARED_SCROFF0_MSB U(23) /* 1  shareable memory for the script with other scripts in mono processor platforms */
#define  SHARED_SCROFF0_LSB U(23) /*                                    */
#define  OFFSET_SCROFF0_MSB U(17) /* 17 offset to the W32 script table */
#define  OFFSET_SCROFF0_LSB U( 0) /*                                   */
         
/* =================
    arc descriptors used to address the working area : registers and stack
*/
#define      SCRIPT_PTR_SCRARCW0  U( 0) /* Base address + NREGS + new UC*/
#define      SCRIPT_UC0_SCRARCW1  U( 1) /* use case UC0 */
#define      SCRIPT_UC1_SCRARCW2  U( 2) /* use-case UC1 */
#define        WRIOCOLL_SCRARCW3  U( 3) /* synchro byte + STACK LENGTH + Flag logMaxCycles8b */

          
#define    __________SCRARCW0_MSB U(31)    
#define    __________SCRARCW0_LSB U(28) /*  4    base address of the working memory */
#define   NEW_USE_CASE_SCRIPT_MSB U(27) /*  1  new use-case arrived */ 
#define   NEW_USE_CASE_SCRIPT_LSB U(27) /*     */ 
#define    BASEIDXOFFSCRARCW0_MSB U(26)    
#define    BASEIDXOFFSCRARCW0_LSB U( 0) /* 27  base address of the script memory (regs + state + stack)  */

#define    COLLISION_SCRARCW3_MSB U(31) /*  8  */
#define    COLLISION_SCRARCW3_LSB U(24) /*     */
#define LOG2MAXCYCLE_SCRARCW3_MSB U(23) /*  8  minifloat reset and return when reaching this number of decoded instructions */ 
#define LOG2MAXCYCLE_SCRARCW3_LSB U(16) /*     */ 
#define    __________SCRARCW3_MSB U(15) /*  1  */
#define    __________SCRARCW3_LSB U(15) /*     */
#define       NBREGS_SCRARCW3_MSB U(14) /*  4    number of registers used in this script */ 
#define       NBREGS_SCRARCW3_LSB U(11) /*     */
#define       NSTACK_SCRARCW3_MSB U(10) /* 11  max size of the FIFO/stack in W32 */
#define       NSTACK_SCRARCW3_LSB U( 0) /*     */


/* ======================================   SWC   ============================================ */ 
#define arm_stream_script_index 1      /* arm_stream_script() is the first one in the list node_entry_point_table[] */

        /* word 0 - main Header */

        /* bit-field 24-31 cannot be null: this used to lock */

#define STREAM_INSTANCE_ANY_PRIORITY    0u      /* PRIORITY_LW0 bit field */
#define STREAM_INSTANCE_LOWLATENCYTASKS 1u
#define STREAM_INSTANCE_MIDLATENCYTASKS 2u
#define STREAM_INSTANCE_BACKGROUNDTASKS 3u

                               /*   corresponds to "WHOAMI_PARCH_LSB" */ 
#define PRIORITY_LW0_MSB U(31) /*   RTOS instances. 0=STREAM_INSTANCE_ANY_PRIORITY */
#define PRIORITY_LW0_LSB U(30) /* 2 up to 3 instances per processors, see PRIORITY_SCTRL  */
#define   PROCID_LW0_MSB U(29) /*   same as PROCID_PARCH (stream instance) (1..3) */
#define   PROCID_LW0_LSB U(27) /* 3 execution reserved to this processor index  (1..7) */  
#define   ARCHID_LW0_MSB U(26) /*   same as ARCHID_PARCH */
#define   ARCHID_LW0_LSB U(24) /* 3 execution reserved to this processor architecture (1..7)  */
/*---------------------------*/
#define  LOADFMT_LW0_MSB U(23) /* 1 optional arcs format loading in reset_component() */
#define  LOADFMT_LW0_LSB U(23) /*   controled by "node_use_arc_format" in the graph */
#define  ARCLOCK_LW0_MSB U(22) /* 1 0/1 the first/second arc locks the components */
#define  ARCLOCK_LW0_LSB U(22) /*   */
#define   SCRIPT_LW0_MSB U(21) /*   script called Before/After (debug, verbose trace control) */
#define   SCRIPT_LW0_LSB U(15) /* 7 script ID for script_SWC or to call before and after calling SWC */
#define  ARCSRDY_LW0_MSB U(14) 
#define  ARCSRDY_LW0_LSB U(13) /* 2  first arcs, nb arcs used to check data availability before RUN */
#define   NBARCW_LW0_MSB U(12) 
#define   NBARCW_LW0_LSB U(10) /* 3  total nb arcs, streaming and metadata/control */
#define  SWC_IDX_LW0_MSB U( 9) 
#define  SWC_IDX_LW0_LSB U( 0) /* 10 0=nothing, swc index of node_entry_points[] */


        /* word 1+n -arcs

            starting with the one used for locking, the streaming arcs, then the metadata arcs 
            arc(tx) used for locking is ARC0_LW1
        */

#define MAX_NB_STREAM_PER_SWC 4 /* no limit in practice */

#define ARC_RX0TX1_TEST  0x0800 /* MSB gives the direction of the arc */
#define ARC_RX0TX1_CLEAR 0x07FF 

#define DBGB1_LW1_MSB 31 
#define DBGB1_LW1_LSB 28 /*  4  debug register bank for ARC1 */
#define  ARC1_LW1_MSB 27
#define ARC1D_LW1_LSB 27 /*  1  ARC1 direction */
#define ARC1D_LW1_MSB 27
#define  ARC1_LW1_LSB 16 /* 11  ARC1  11 usefull bits + 1 MSB to tell rx0tx1 */

#define DBGB0_LW1_MSB 15 
#define DBGB0_LW1_LSB 12 /*  4  debug register bank for ARC0 : debug-arc index of the debug data */
#define  ARC0_LW1_MSB 11
#define ARC0D_LW1_LSB 11 /*  1  ARC0 direction */
#define ARC0D_LW1_MSB 11
#define  ARC0_LW1_LSB  0 /* 11  ARC0, (11 + 1 rx0tx1) up to 2K ARCs */

#if IOARCID_IOFMT_MSB != (ARC0D_LW1_MSB-1)
#error "IOFORMAT ARC SIZE"
#endif
        /* word 2+n - FIRST WORD : memory banks address + size */

#define NBW32_MEMREQ_LW2  2     /* there are two words per memory segments, to help programing the memory protection unit (MPU) */
#define ADDR_LW2 0              /*      one for the address */ 
#define SIZE_LW2 1              /*      one for the size of the segment */ 

            /* word 2 first word = base address of the memory segment + control on the first segment */
#define      DTCM_LW2_MSB U(31) /*      for relocatable scratch DTCM usage with SMP, address is changing : */
#define      DTCM_LW2_LSB U(31) /*  1   arc_index_update() pushed the DTCM address after XDM buffers */
#define     XDM11_LW2_MSB U(30) /*      0: Rx/Tx flow is asynchronous  1: same consumption on Rx/Tx */   
#define     XDM11_LW2_LSB U(30) /*  1   the input and output frame size of all arcs are identical (manifest: node_same_rxtx_data_rate)*/ 
#define   ________LW2_MSB U(29) /*      */  
#define   ________LW2_LSB U(29) /*  1   */
#define  NALLOCM1_LW2_MSB U(28) /*      number of memory segments to give at RESET [0..MAX_NB_MEM_REQ_PER_NODE-1] */  
#define  NALLOCM1_LW2_LSB U(27) /*  2    2 words each : pointer + size */
#define BASEIDXOFFLW2_MSB U(26) 
#define   DATAOFF_LW2_MSB DATAOFF_ARCW0_MSB
#define   DATAOFF_LW2_LSB DATAOFF_ARCW0_LSB /*  3 bits 64bits offset index see long_offset */
#define   BASEIDX_LW2_MSB BASEIDX_ARCW0_MSB /*    buffer address 21 + sign + offset = 25 bits (2bits margin) */
#define   BASEIDX_LW2_LSB BASEIDX_ARCW0_LSB /* 21 base address in Word32 + 1 sign bit*/
#define BASEIDXOFFLW2_LSB U( 0) /* 27  */

/* 
    table of PACKSWCMEM requirements (speed, size, alignment..) see MAXNB_MEMORY_BASE_MALLOC 
    MPU has 8 memory segments : 4 memory segments per SWC (1 instance + 3 segments) + 1 code + 1 stack + 1 IRQ + 1 Stream/services/script
*/
#define MAX_NB_MEM_REQ_PER_NODE U(4)    /* TO_SWAP_LW2S limits to 4 MAX */

            /* word 2 SECOND WORD = size of the memory segment  + control on the first segment 
                            if the segment is swapped, the 12-LSB bits give the ARC ID of the buffer 
                            and the memory size is given by the FIFO descriptor (BUFF_SIZE_ARCW1) */
#define LW2S_NOSWAP 0
#define LW2S_SWAP 1
#define LW2S_COPY 2

#define   ________LW2S_MSB U(31) /*      */
#define   ________LW2S_LSB U(30) /*  2   */
#define       KEY_LW2S_MSB U(29) /*      */
#define       KEY_LW2S_LSB U(28) /*  2   protocol for key exchanges (boot and graph/user) */
#define   TO_SWAP_LW2S_MSB U(27) /*      */
#define   TO_SWAP_LW2S_LSB U(24) /*  4   one bit per MAX_NB_MEM_REQ_PER_NODE segment to consider for swapping */
#define      SWAP_LW2S_MSB U(23) /*      0= normal memory segment, 2 = copy before execute */
#define      SWAP_LW2S_LSB U(22) /*  2   1= swap before/after execute */
#define BUFF_SIZE_LW2S_MSB U(21) /* ###  overlaid with SWAPBUFID_LW2S in case of COPY / SWAP */
#define BUFF_SIZE_LW2S_LSB U( 0) /* 22   Wrd32-acurate up to 16MBytes */

#define SWAPBUFID_LW2S_MSB  ARC0_LW1_MSB /*     ARC => swap source address in slow memory + swap length */
#define SWAPBUFID_LW2S_LSB  ARC0_LW1_LSB /* 12  ARC0, (11 + 1) up to 2K FIFO */


  /* word 3+n - parameters 
    SWC header can be in RAM (to patch the parameter area, cancel the component..)

  BOOTPARAMS (if W32LENGTH_LW3>0 )

  PARAM_TAG : 4  index to parameter (0='all parameters')
  PRESET    : 4  preset index (SWC delivery)
  TRACEID   : 8  
  W32LENGTH :16  nb of WORD32 to skip at run time, 0 means NO PARAMETER, max=256kB

  Example with 
  1  u8:  0                           trace ID
  1  h8;  01                          TAG = "all parameters"_0 + preset_1
                parameters 
  1  u8;  2                           Two biquads
  1  u8;  0                           postShift
  5 h16; 5678 2E5B 71DD 2166 70B0     b0/b1/b2/a1/a2 
  5 h16; 5678 2E5B 71DD 2166 70B0     second biquad

  SWC can declare an extra input arc to receive a huge set of parameters (when >256kB), for example a 
  NN model. This is a fake arc and the read pointer is never incremented during the execution of the node.
*/
//#define MAX_TMP_PARAMETERS 30   /* temporary buffer (words32) of parameters to send to the Node */


#define PARAM_TAG_LW3_MSB U(31) 
#define PARAM_TAG_LW3_LSB U(28) /* 4  for PARAM_TAG_CMD (15='all parameters')  */
#define    PRESET_LW3_MSB U(27)
#define    PRESET_LW3_LSB U(24) /* 4  preset   16 precomputed configurations */
#define   TRACEID_LW3_MSB U(23)       
#define   TRACEID_LW3_LSB U(16) /* 8  TraceID used to route the trace to the corresponding peripheral/display-line */
#define W32LENGTH_LW3_MSB U(15) /*    if >256kB are needed then use an arc to a buffer */
#define W32LENGTH_LW3_LSB U( 0) /* 16 skip this : number of uint32 to skip the boot parameters */

/*  nbparam = 0 means any or "full set of parameters loaded from binary format" 
    W32LENGTH_LW3 == 0 means no parameter to read
*/
#define ALLPARAM_ 0 

/* ================================= */

/* last word has SWC index 0b11111..111 */
#define GRAPH_LAST_WORD_MSB SWC_IDX_LW0_MSB
#define GRAPH_LAST_WORD_LSB SWC_IDX_LW0_LSB
#define GRAPH_LAST_WORD U((U(1)<<U(SWC_IDX_LW0_MSB- SWC_IDX_LW0_LSB+1U))-1U)

/*=====================================================================================*/                          
/*
    commands 
        from the application to the graph scheduler         arm_graph_interpreter(command,  arm_stream_instance_t *S, uint8_t *, uint32_t);
        from the graph scheduler to the nanoApps            S->address_swc (command, stream_handle_t instance, stream_xdmbuffer_t *, uint32_t *);
        from the Scripts to the IO configuration setting    arm_stream_services (command, uint8_t *, uint8_t *, uint8_t *, uint32_t)
*/

    /*  FROM APP TO SCHEDULER :     arm_graph_interpreter (STREAM_RESET, &stream_instance, 0, 0); 
        FROM SCHEDULER to SWC :     devxx_fyyyy (STREAM_RESET, &swc_instance, &memreq, &status); 
            Command + nb arcs, preset 0..15m TAG 0..255
        -  (STREAM_RESET, ptr1, ptr2, ptr3); 
            ptr1 = instance pointer, memory banks
            ptr2 = stream_al_services function address, followed by all the arc format (node manifest node_use_arc_format =1)
        -  (STREAM_SET_PARAMETER, ptr1, ptr2, ptr3); 
            ptr1 = instance
            ptr2 = byte pointer to parameters, depends on the TAG 
        -  (STREAM_READ_PARAMETER, ptr1, ptr2, ptr3); 
            ptr1 = instance
            ptr2 = parameter data destination, depends on the TAG
        -  (STREAM_RUN, ptr1, ptr2, ptr3); 
            ptr1 = instance
            ptr2 = list of XDM arc buffers (X,n) , the size field means :
                 rx arc . size = amount of data available for processing
                 tx arc . size = amount of free area in the buffer 
                when XDM11=0 (node_same_rxtx_data_rate=0) SWC updates the XDM size fields with :
                 rx arc . size = amount of data consumed
                 tx arc . size = amount of data produced
        -  (STREAM_STOP, ptr1, ptr2, ptr3); 
    */
    #define STREAM_RESET            1   /* arm_graph_interpreter(STREAM_RESET, *instance, * memory_results) */
    #define STREAM_SET_PARAMETER    2   /* APP sets SWC parameters swc instances are protected by multithread effects when 
                                          changing parmeters on the fly, used to exchange the unlock key */
    #define STREAM_READ_PARAMETER   3   /* used from script */
    #define STREAM_RUN              4   /* arm_graph_interpreter(STREAM_RUN, instance, *in_out) */
    #define STREAM_STOP             5   /* arm_graph_interpreter(STREAM_STOP, instance, 0)  swc calls free() if it used stdlib's malloc */
    #define STREAM_UPDATE_RELOCATABLE 6 /* update the nanoAppRT pointers to relocatable memory segments */
    #define STREAM_SET_IO_CONFIG STREAM_SET_PARAMETER /* 
            reconfigure the IO : p_io_function_ctrl(STREAM_SET_IO_CONFIG + (FIFO_ID<<SWC_TAG_CMD), 0, new_configuration_index) */
    #define STREAM_SET_BUFFER       7   /* platform_IO(STREAM_SET_BUFFER, *data, size)  */


/*  FROM THE GRAPH SCHEDULER TO THE NANOAPPS   SWC_COMMANDS  */
    #define  _UNUSED2_CMD_MSB U(31)       
    #define  _UNUSED2_CMD_LSB U(24) /* 8 _______ */
    #define   SWC_TAG_CMD_MSB U(23) /*    parameter, function selection / debug arc index / .. */      
    #define   SWC_TAG_CMD_LSB U(16) /* 8  instanceID for the trace / FIFO_ID for status checks */
    #define    PRESET_CMD_MSB U(15)       
    #define    PRESET_CMD_LSB U(12) /* 4  #16 presets */
    #define      NARC_CMD_MSB U(11)       
    #define      NARC_CMD_LSB U( 8) /* 4 number of arcs */
    #define  __UNUSED_CMD_MSB U( 7)       
    #define  __UNUSED_CMD_LSB U( 5) /* 3 _______ */
    #define  COMMDEXT_CMD_MSB U( 4)       
    #define  COMMDEXT_CMD_LSB U( 4) /* 1 command option (RESET + warmboot)  */
    #define   COMMAND_CMD_MSB U( 3)       
    #define   COMMAND_CMD_LSB U( 0) /* 4 command */

    #define PACK_COMMAND(SWCTAG,PRESET,NARC,EXT,CMD) (((SWCTAG)<<SWC_TAG_CMD_LSB)|((PRESET)<<PRESET_CMD_LSB)|((NARC)<<NARC_CMD_LSB)|((EXT)<<COMMDEXT_CMD_LSB)|(CMD))

/*================================================================================================================*/    
/*
    "stream_service_command"  from the nodes, to "arm_stream_services"

    void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n)
*/

#define NOOPTION_SSRV 0
#define NOCONTROL_SSRV 0


/* arm_stream_services COMMAND */
#define  _UNUSED_SSRV_MSB U(31)       
#define  _UNUSED_SSRV_LSB U(29) /* 3 _______ */
#define  CONTROL_SSRV_MSB U(28)       
#define  CONTROL_SSRV_LSB U(26) /* 3    set/init/wait completion, in case of coprocessor usage */
#define   OPTION_SSRV_MSB U(25)       
#define   OPTION_SSRV_LSB U(10) /* 16   compute accuracy, in-place processing, frame size .. */
#define FUNCTION_SSRV_MSB U( 9)       
#define FUNCTION_SSRV_LSB U( 4) /* 6    64 functions/group  */
#define    GROUP_SSRV_MSB U( 3)       
#define    GROUP_SSRV_LSB U( 0) /* 4    16 groups */

/* clears the MALLOC_SSRV field */
#define PACK_SERVICE(CTRL,OPTION,FUNC,GROUP) (((CTRL)<<CONTROL_SSRV_LSB)|((OPTION)<<OPTION_SSRV_LSB)|((FUNC)<<FUNCTION_SSRV_LSB)|(GROUP)<<GROUP_SSRV_LSB)

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

                /* needs a registered return address (Y/N)  (TBD @@@) */
#define STREAM_SERVICE_INTERNAL     1   /* Y */
#define STREAM_SERVICE_FLOW         2   /* Y */
#define STREAM_SERVICE_CONVERSION   3   /* N */
#define STREAM_SERVICE_STDLIB       4   /* Y */
#define STREAM_SERVICE_MATH         5   /* N */
#define STREAM_SERVICE_DSP_ML       6   /* N */
#define STREAM_SERVICE_MM_AUDIO     7   /* Y */
#define STREAM_SERVICE_MM_IMAGE     8   /* Y */

//{
/* 0/STREAM_SERVICE_INTERNAL ------------------------------------------------ */

#define STREAM_SERVICE_INTERNAL_RESET 1u
#define STREAM_SERVICE_INTERNAL_NODE_REGISTER 2u

/* change stream format from SWC media decoder, script applying change of use-case (IO_format, vocoder frame-size..): sampling, nb of channel, 2D frame size */
#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE 3u      

//#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_FS 3u       /* SWC information for a change of stream format, sampling, nb of channel */
//#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_NCHAN 4u     /* raw data sample, mapping of channels, (web radio use-case) */
//#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_RAW 5u

#define STREAM_SERVICE_INTERNAL_SECURE_ADDRESS 6u       /* this call is made from the secured address */
#define STREAM_SERVICE_INTERNAL_AUDIO_ERROR 7u          /* PLC applied, Bad frame (no header, no synchro, bad data format), bad parameter */
#define STREAM_SERVICE_INTERNAL_DEBUG_TRACE 8u          /* 1b, 1B, 16char */
#define STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STAMPS 9u
#define STREAM_SERVICE_INTERNAL_AVAILABLE 10u
#define STREAM_SERVICE_INTERNAL_SETARCDESC 11u          /* buffers holding MP3 songs.. rewind from script, 
                                                            switch a NN model to another, change a parameter-set using arcs */
#define STREAM_SERVICE_INTERNAL_KEYEXCHANGE 12          /* at reset time : key exchanges */


//STREAM_SERVICE_INTERNAL_DEBUG_TRACE, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_1B, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_DIGIT, 
//STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STAMPS, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STRING,
//STREAM_SAVE_HOT_PARAMETER, 
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
    /* stdlib.h */
    /* string.h */
    //STREAM_MEMSET, STREAM_STRCHR, STREAM_STRLEN,
    //STREAM_STRNCAT, STREAM_STRNCMP, STREAM_STRNCPY, STREAM_STRSTR, STREAM_STRTOK,
    #define STREAM_RAND     1 /* (STREAM_RAND + OPTION_SSRV(seed), *ptr1, 0, 0, n) */
    #define STREAM_SRAND    2
    #define STREAM_ATOF     3
    #define STREAM_ATOI     4
    #define STREAM_MEMSET   5
    #define STREAM_STRCHR   6
    #define STREAM_STRLEN   7
    #define STREAM_STRNCAT  8
    #define STREAM_STRNCMP  9
    #define STREAM_STRNCPY  10
    #define STREAM_STRSTR   11
    #define STREAM_STRTOK   12
    //STREAM_RAND, STREAM_SRAND, STREAM_ATOF, STREAM_ATOI
    #define STREAM_FREE     13
    #define STREAM_MALLOC   14


/* 4/STREAM_SERVICE_MATH ------------------------------------------------ */

    /* time.h */
    //STREAM_ASCTIMECLOCK, STREAM_DIFFTIME, STREAM_SYS_CLOCK (ms since reset), STREAM_TIME (linux seconds)
    //STREAM_READ_TIME (high-resolution timer), STREAM_READ_TIME_FROM_START, 
    //STREAM_TIME_DIFFERENCE, STREAM_TIME_CONVERSION,  
    // 
    //STREAM_TEA,

    /* From Android CHRE  https://source.android.com/docs/core/interaction/contexthub
    String/array utilities: memcmp, memcpy, memmove, memset, strlen
    Math library: Commonly used single-precision floating-point functions:
    Basic operations: ceilf, fabsf, floorf, fmaxf, fminf, fmodf, roundf, lroundf, remainderf
    Exponential/power functions: expf, log2f, powf, sqrtf
    Trigonometric/hyperbolic functions: sinf, cosf, tanf, asinf, acosf, atan2f, tanhf
    */

/* 5/STREAM_SERVICE_DSP_ML ------------------------------------------------ */

#define STREAM_SERVICE_NO_INIT         0    /* CONTROL_SSRV_LSB field*/
#define STREAM_SERVICE_INIT_RETASAP    1    /* return even when computation is not finished */
#define STREAM_SERVICE_INIT_WAIT_COMP  2    /* tell to return when processing completed */
#define STREAM_SERVICE_CHECK_COPROCESSOR 3  /* check for services()*/
#define STREAM_SERVICE_CHECK_END_COMP  4    /* check completion for the caller */


#define STREAM_SERVICE_CASCADE_DF1_Q15 1    /* IIR filters, use STREAM_SERVICE_CHECK_COPROCESSOR */
#define STREAM_SERVICE_CASCADE_DF1_Q15_CHECK_COMPLETION 2    
#define STREAM_SERVICE_CASCADE_DF1_F32 3         
#define STREAM_SERVICE_CASCADE_DF1_F32_CHECK_COMPLETION 4

#define STREAM_SERVICE_rFFT_Q15        5    /* RFFT windowing, module, dB , use STREAM_SERVICE_CHECK_COPROCESSOR */
#define STREAM_SERVICE_rFFT_Q15_CHECK_COMPLETION        6
#define STREAM_SERVICE_rFFT_F32        7
#define STREAM_SERVICE_rFFT_F32_CHECK_COMPLETION        8
                                       
#define STREAM_SERVICE_cFFT_Q15        8    /* cFFT windowing, module, dB */
#define STREAM_SERVICE_cFFT_F32       10
                                       
#define STREAM_SERVICE_DFT_Q15        12    /* DFT/Goertzel windowing, module, dB */
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

#define STREAM_SERVICE_SORT 3

#define STREAM_WINDOWS                /* windowing for spectral estimations */
#define STREAM_FIR                    
#define STREAM_FC                     /* fully connected layer Mat x Vec */
#define STREAM_CNN                    /* convolutional NN : 3x3 5x5 fixed-weights */
#define STREAM_CONV2D                 /* Sobel */

/*
 *  SWC manifest :
 */ 

//enum stream_node_status {
#define SWC_BUFFERS_PROCESSED 0
#define SWC_NEED_RUN_AGAIN 1         /* SWC completion type */

//enum mem_mapping_type {
#define MEM_TYPE_STATIC          0    /* (LSB) memory content is preserved (default ) */
#define MEM_TYPE_WORKING         1    /* scratch memory content is not preserved between two calls */
#define MEM_TYPE_PERIODIC_BACKUP 2    /* static parameters to reload for warm boot after a crash */
#define MEM_TYPE_PSEUDO_WORKING  3    /* static only during the uncompleted execution state of the SWC, see NODE_RUN 

                periodic backup use-case : long-term estimators. This memory area is cleared at cold NODE_RESET and 
                refreshed for warm NODE_RESET. The SWC should not reset it (there is 
                no "warm-boot reset" entry point. The period of backup depends on platform capabilities 
                When MBANK_BACKUP is a retention-RAM there is nothing to do, when it is standard RAM area then on periodic
                basis the AL will be call to transfer data to Flash 
            */
              

//enum mem_speed_type                         /* memory requirements associated to enum memory_banks */
#define MEM_SPEED_REQ_ANY           0    /* best effort */
#define MEM_SPEED_REQ_FAST          1    /* will be internal SRAM when possible */
#define MEM_SPEED_REQ_CRITICAL_FAST 2    /* will be TCM when possible
           When a SWC is declaring this segment as relocatable ("RELOC_MEMREQ") it will use 
           physical address different from one TCM to an other depending on the processor running the SWC.
           The scheduler shares the TCM address dynamically before calling the SWC. 
           This TCM address is provided as a pointer after the XDM in/out pointer
           The TCM address is placed at the end (CRITICAL_FAST_SEGMENT_IDX) of long_offset[] 
           */
                                

//enum buffer_alignment_type            
//#define MEM_REQ_NOALIGNMENT_REQ    0    /* address binary mask : */
//#define MEM_REQ_2BYTES_ALIGNMENT   1   /*   mask = ~((1 << (7 & mem_req_2bytes_alignment) -1) */
#define MEM_REQ_4BYTES_ALIGNMENT   2     
#define MEM_REQ_8BYTES_ALIGNMENT   3
#define MEM_REQ_16BYTES_ALIGNMENT  4
#define MEM_REQ_32BYTES_ALIGNMENT  5
#define MEM_REQ_64BYTES_ALIGNMENT  6
#define MEM_REQ_128BYTES_ALIGNMENT 7


#define SWC_CONTROLS U(4)
#define SWC_CONTROLS_NAME U(8)

/*================================================== ARC ==================================================================*/
/*
  arc descriptions : 
                             
      - arc_descriptor_ring : R/W are used to manage the alignment of data to the base address and notify the SWC
                              debug pattern, statistics on data, time-stamps of access, 
                              realignment of data to base-address when READ > (SIZE) - consumer frame-size
                              deinterleave multichannel have the same read/write offset but the base address starts 
                              from the end of the previous channel boundary of the graph
*/

// enum debug_arc_computation_1D { COMPUTCMD_ARCW2 /* 5bits */

#define COMPUTCMD_ARCW2_NO_ACTION               0       
#define COMPUTCMD_ARCW2_INCREMENT_REG           1   /* increment DEBUG_REG_ARCW1 with the number of RAW samples */      
#define COMPUTCMD_ARCW2_SET_ZERO_ADDR           2   /* set a 0 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to clear */      
#define COMPUTCMD_ARCW2_SET_ONE_ADDR            3   /* set a 1 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to set */       
#define COMPUTCMD_ARCW2_INCREMENT_REG_ADDR      4   /* increment *DEBUG_REG_ARCW1 */ 
#define COMPUTCMD_ARCW2__5                      5
#define COMPUTCMD_ARCW2_APP_CALLBACK1           6   /* call-back in the application side, data rate estimate in DEBUG_REG_ARCW1 */      
#define COMPUTCMD_ARCW2_APP_CALLBACK2           7   /* second call-back : wake-up processor from DEBUG_REG_ARCW1=[ProcID, command]  */      
#define COMPUTCMD_ARCW2__8                      8
#define COMPUTCMD_ARCW2_TIME_STAMP_LAST_ACCESS  9
#define COMPUTCMD_ARCW2_PEAK_DATA               10  /* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW1 */          
#define COMPUTCMD_ARCW2_MEAN_DATA               11
#define COMPUTCMD_ARCW2_MIN_DATA                12
#define COMPUTCMD_ARCW2_ABSMEAN_DATA            13
//#define COMPUTCMD_ARCW2_DATA_TO_OTHER_ARC     14  /* when data is changing the new data is push to another arc DEBUG_REG_ARCW1=[ArcID] */
#define COMPUTCMD_ARCW2_LOOPBACK                15  /* automatic rewind read/write */           
#define COMPUTCMD_ARCW2_LAST                31

#define ARC_DBG_REGISTER_SIZE_W32 2                 /* debug registers on 64 bits */


/* increment DEBUG_REG_ARCW1 with the number of RAW samples */
/* set a 0 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to clear */
/* set a 1 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to set */
/* increment *DEBUG_REG_ARCW1 */

/* call-back in the application side, data rate estimate in DEBUG_REG_ARCW1 */
/* second call-back : wake-up processor from DEBUG_REG_ARCW1=[ProcID, command]  */


/* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW1 */



  /* when data is changing the new data is push to another arc DEBUG_REG_ARCW1=[ArcID] */
/* automatic rewind read/write */


/* debug registers on 64 bits */
















/*==========================================  ARCS  ===================================================*/
                                         
#define arc_read_address                1 
#define arc_write_address               2 
#define arc_data_amount                 3 
#define arc_free_area                   4
#define arc_buffer_size                 5
#define data_move_to_arc                6
#define data_moved_from_arc             7
#define data_swapped_with_arc           8
#define arc_data_realignment_to_base    9



#define SIZEOF_ARCDESC_W32 4

#define   BUF_PTR_ARCW0    U( 0)
#define PRODUCFMT_ARCW0_MSB U(31) /*  5 bits  PRODUCER format  (intPtr_t) +[i x STREAM_FORMAT_SIZE_W32]  */ 
#define PRODUCFMT_ARCW0_LSB U(27) /*    Graph generator gives IN/OUT arc's frame size to be the LCM of SWC "grains" */
#define BASEIDXOFFARCW0_MSB U(26)    
#define   DATAOFF_ARCW0_MSB U(26) /*    address = offset[DATAOFF] + 4x BASEIDX[Bytes] */
#define   DATAOFF_ARCW0_LSB U(24) /*  3 32/64bits offset index see long_offset */
#define   BASEIDX_ARCW0_MSB U(23) /*    +/- 0x0080.0000 = 8MW=2^23    0x2000.0000 = 512MW */
#define  BAS_SIGN_ARCW0_MSB U(23) /*    buffer address 23 + sign + offset = 27 bits */
#define  BAS_SIGN_ARCW0_LSB U(23) /*    sign of the address with respect to the offset */
#define   BASEIDX_ARCW0_LSB U( 0) /* 24 base address 24bits linear address range in Word32 */
#define BASEIDXOFFARCW0_LSB U( 0) /*    +/- 0x7F.FFFF(W32) =  +/- 8MW/32MBytes (+/-2GB EXTEND_ARCW2=1) */
                                
#define BUFSIZDBG_ARCW1    U( 1)
#define CONSUMFMT_ARCW1_MSB U(31) /*    */
#define CONSUMFMT_ARCW1_LSB U(27) /* 5  CONSUMER format  */ 
#define   MPFLUSH_ARCW1_MSB U(26) 
#define   MPFLUSH_ARCW1_LSB U(26) /* 1  flush data used after processing */
#define DEBUG_REG_ARCW1_MSB U(25) /*    debug registers have 64bits and are stored in the first arc descriptors  */
#define DEBUG_REG_ARCW1_LSB U(22) /* 4  2x32bits debug result index [0..15][page: DBGB0_LW1] = data + STREAM_TIMESTMP */
#define BUFF_SIZE_ARCW1_MSB U(21) /*     */
#define BUFF_SIZE_ARCW1_LSB U( 0) /* 22 BYTE-acurate up to 4MBytes (4 x64 = 256MB with EXTEND_ARCW2=1*/

#define    RDFLOW_ARCW2    U( 2)  /* write access only from the SWC consumer */
#define COMPUTCMD_ARCW2_MSB U(31)       
#define COMPUTCMD_ARCW2_LSB U(27) /* 5  gives the debug task to proceed  (enum debug_arc_computation_1D) */
#define FLOWERROR_ARCW2_MSB U(26)
#define FLOWERROR_ARCW2_LSB U(26) /* 1  under/overflow 0=nothing or best effort from IO_DOMAIN_IOFMT */
#define    EXTEND_ARCW2_MSB U(25) /*    Size/Read/Write are used with x64 (EXT=2 applies x1024, EXT=3 x16k) to extend to */
#define    EXTEND_ARCW2_LSB U(24) /* 2  256MB (EXT=2 4GB, EXT=3 64GB) size for arcs used to read NN models, video players, etc */
#define    _______ARCW2_MSB U(23) /*    */
#define    _______ARCW2_LSB U(22) /* 2  */
#define      READ_ARCW2_MSB U(21) /*    data read index  Byte-acurate up to 4MBytes starting from base address */
#define      READ_ARCW2_LSB U( 0) /* 22 this is incremented by "frame_size" FRAMESIZE_FMT0  */

#define COLLISION_ARC_OFFSET_BYTE U(3) /* offset in bytes to the collision detection byte */
#define  WRIOCOLL_ARCW3    U( 3) /* write access only from the SWC producer */
#define COLLISION_ARCW3_MSB U(31) /* 8  MSB byte used to lock the SWC, loaded with arch+proc+instance ID */ 
#define COLLISION_ARCW3_LSB U(24) /*       to check node-access collision from an other processor */
#define ALIGNBLCK_ARCW3_MSB U(23) /*    producer blocked */
#define ALIGNBLCK_ARCW3_LSB U(23) /* 1  producer sets "need for data realignement"  */
#define   ________ARCW3_MSB U(22) /*     */
#define   ________ARCW3_LSB U(22) /* 1   */
#define     WRITE_ARCW3_MSB U(21) /*    write pointer is incremented by FRAMESIZE_FMT0 */
#define     WRITE_ARCW3_LSB U( 0) /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */

/* arcs with indexes higher than IDX_ARCS_desc, see enum_arc_index */

//#define PLATFORM_IO 0                   /* 3 bits offets code for arcs external to the graph */



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

#define PLATFORM_CLEAR_BACKUP_MEM  0x0D   /* cold start : clear backup memory */
#define PLATFORM_ERROR             0x10   /* error to report to the application */

#define   OPTION_AL_SRV_MSB U(25)       
#define   OPTION_AL_SRV_LSB U(10) /* 16   .. */
#define FUNCTION_AL_SRV_MSB U( 9)       
#define FUNCTION_AL_SRV_LSB U( 4) /* 6    64 functions/group  */
#define    GROUP_AL_SRV_MSB U( 3)       
#define    GROUP_AL_SRV_LSB U( 0) /* 4    16 groups */  
#define PACK_AL_SERVICE(OPTION,FUNC,GROUP) (((OPTION)<<OPTION_AL_SRV_LSB)|((FUNC)<<FUNCTION_AL_SRV_LSB)|(GROUP)<<GROUP_AL_SRV_LSB)


/*
* system subroutines : 
* - IO settings : 
* - Get Time, in different formats, and conversion, extract time-stamps
* - Get Peripheral data : RSSI, MAC/IP address
* - Low-level : I2C string of commands, GPIO, physical address to perpherals
*/
#define PLATFORM_DEEPSLEEP_ENABLED 0x20   /* deep-sleep activation is possible when returning from arm_graph_interpreter(STREAM_RUN..) */
#define PLATFORM_TIME_SET          0x21
#define PLATFORM_RTC_SET           0x22
#define PLATFORM_TIME_READ         0x23
#define PLATFORM_HW_WORD_READ      0x24  
#define PLATFORM_HW_WORD_WRITE     0x25  
#define PLATFORM_HW_BYTE_READ      0x26  
#define PLATFORM_HW_BYTE_WRITE     0x27  

//enum error_codes 
#define ERROR_MEMORY_ALLOCATION     1u


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



//------------------------------------------------------------------------------------------------------
/*  Time format - 64 bits
 * 
 *  140 years ~2^32 ~ 4.4 G = 0x1.0000.0000 seconds 
 *  1 year = 31.56 M seconds
 *  1 day = 86.400 seconds
 * 
 *  "stream_time64" 
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s]  140 Y + Q28 [s]
 *  systick increment for  1ms =  0x00041893 =  1ms x 2^28
 *  systick increment for 10ms =  0x0028F5C2 = 10ms x 2^28
 *
 *  140 years in ms = 4400G ms = 0x400.0000.0000 ms 42bits
 *  milliseconds from January 1st 1970 UTC (or internal AL reference)
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ______________________mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm ms 
 *
 *
 *  Local time in BINARY bit-fields : years/../millisecond, WWW=day of the week 
 *  (0=Sunday, 1=Monday..)
 *      COVESA allowed formats : ['YYYY_MM_DD', 'DD_MM_YYYY', 'MM_DD_YYYY', 'YY_MM_DD', 'DD_MM_YY', 'MM_DD_YY']
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  _________________________.YY.YY.YY.YY.MMM.DDDD.SSSSS.MM.MM.MM.WW
 * 
 */

//typedef uint64_t stream_time64;
#define   TYPE_T64_MSB 63
#define   TYPE_T64_LSB 61 
#define SECOND_T64_MSB 60
#define SECOND_T64_LSB 29
#define  FRACT_T64_MSB 28 
#define  FRACT_T64_LSB  0

/*
 * stream_time_seconds in 32bits : "stream_time32"
 *  FEDCBA9876543210FEDCBA9876543210
 *  ssssssssssssssssssssssssssssqqqq q28.4  [s] (8.5 years +/- 0.0625s)
 *  stream_time32 = (stream_time64 >> 24) 
 *
 * delta_stream_time_seconds in 32bits : "stream_time32_diff"
 *  FEDCBA9876543210FEDCBA9876543210
 *  sssssssssssssssssqqqqqqqqqqqqqqq q17.15 [s] (36h, +/- 30us)
 *  stream_time32_diff = (stream_time64 >> 13) 

 * ARC time-stamp in 32bits : "stream_timestamp32" 
 *  FEDCBA9876543210FEDCBA9876543210
 *  ssssssssssssssssssssqqqqqqqqqqqq q20.12 [s] (12 days, +/- 0.25ms)
 *  stream_time32_diff = (stream_time64 >> 10) 
 *
 */
//typedef uint32_t stream_time32;
#define   FORMAT_T32_MSB 31
#define   FORMAT_T32_LSB 30 
#define     TIME_T32_MSB 29
#define     TIME_T32_LSB  0 

/*
 *  stream_time_seconds in 16bits : "stream_time16"
 *  FEDCBA9876543210
 *  ssssssssssssqqqq q14.2   1 hour + 8mn +/- 0.0625s
 *
 *  FEDCBA9876543210
 *  stream_time_seconds differencein 16bits : "stream_time16diff"
 *  qqqqqqqqqqqqqqqq q15 [s] time difference +/- 15us
 */
//typedef uint32_t stream_time16;
#define     TIME_T16_MSB 15
#define     TIME_T16_LSB  0 


/*================================ STREAM ARITHMETICS DATA/TYPE ====================================================*/
/* types fit in 6bits, arrays start with 0, stream_bitsize_of_raw() is identical */


#define STREAM_DATA_ARRAY 0 /* see stream_array: [0NNNTT00] 0, type, nb */
#define STREAM_S1         1 /* S, one signed bit, "0" = +1 */                           /* one bit per data */
#define STREAM_U1         2 /* one bit unsigned, boolean */
#define STREAM_S2         3 /* SX  */                                                   /* two bits per data */
#define STREAM_U2         4 /* XX  */
#define STREAM_Q1         5 /* Sx ~stream_s2 with saturation management*/
#define STREAM_S4         6 /* Sxxx  */                                                 /* four bits per data */
#define STREAM_U4         7 /* xxxx  */
#define STREAM_Q3         8 /* Sxxx  */
#define STREAM_FP4_E2M1   9 /* Seem  micro-float [8 .. 64] */
#define STREAM_FP4_E3M0  10 /* Seee  [8 .. 512] */
#define STREAM_S8        11 /* Sxxxxxxx  */                                             /* eight bits per data */
#define STREAM_U8        12 /* xxxxxxxx  ASCII char, numbers.. */
#define STREAM_Q7        13 /* Sxxxxxxx  arithmetic saturation */
#define STREAM_CHAR      14 /* xxxxxxxx  */
#define STREAM_FP8_E4M3  15 /* Seeeemmm  NV tiny-float [0.02 .. 448] */                 
#define STREAM_FP8_E5M2  16 /* Seeeeemm  IEEE-754 [0.0001 .. 57344] */
#define STREAM_S16       17 /* Sxxxxxxx.xxxxxxxx  */                                    /* 2 bytes per data */
#define STREAM_U16       18 /* xxxxxxxx.xxxxxxxx  Numbers, UTF-16 characters */
#define STREAM_Q15       19 /* Sxxxxxxx.xxxxxxxx  arithmetic saturation */
#define STREAM_FP16      20 /* Seeeeemm.mmmmmmmm  half-precision float */
#define STREAM_BF16      21 /* Seeeeeee.mmmmmmmm  bfloat */
#define STREAM_Q23       22 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx  24bits */                    /* 3 bytes per data */ 
#define STREAM_Q23_32    23 /* SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxxx  */                  /* 4 bytes per data */
#define STREAM_S32       24 /* one long word  */
#define STREAM_U32       25 /* xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx UTF-32, .. */
#define STREAM_Q31       26 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx  */
#define STREAM_FP32      27 /* Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm  FP32 */             
#define STREAM_CQ15      28 /* Sxxxxxxx.xxxxxxxx Sxxxxxxx.xxxxxxxx (I Q) */             
#define STREAM_CFP16     29 /* Seeeeemm.mmmmmmmm Seeeeemm.mmmmmmmm (I Q) */             
#define STREAM_S64       30 /* long long */                                             /* 8 bytes per data */
#define STREAM_U64       31 /* unsigned  64 bits */
#define STREAM_Q63       32 /* Sxxxxxxx.xxxxxx ....... xxxxx.xxxxxxxx  */
#define STREAM_CQ31      33 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxx..*/
#define STREAM_FP64      34 /* Seeeeeee.eeemmmmm.mmmmmmm ... double  */
#define STREAM_CFP32     35 /* Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm Seee.. (I Q)  */
#define STREAM_FP128     36 /* Seeeeeee.eeeeeeee.mmmmmmm ... quadruple precision */     /* 16 bytes per data */
#define STREAM_CFP64     37 /* fp64 fp64 (I Q)  */
#define STREAM_FP256     38 /* Seeeeeee.eeeeeeee.eeeeemm ... octuple precision  */      /* 32 bytes per data */
#define STREAM_TIME16    39 /* ssssssssssssqqqq q14.2   1 hour + 8mn +/- 0.0625 */
#define STREAM_TIME16D   40 /* qqqqqqqqqqqqqqqq q15 [s] time difference +/- 15us */
#define STREAM_TIME32    41 /* ssssssssssssssssssssssssssssqqqq q28.4  [s] (8.5 years +/- 0.0625s) */ 
#define STREAM_TIME32D   42 /* ssssssssssssssssqqqqqqqqqqqqqqqq q17.15 [s] (36h, +/- 30us) time difference */   
#define STREAM_TIMESTMP  43 /* ssssssssssssssssssssqqqqqqqqqqqq q20.12 [s] (12 days, +/- 0.25ms) */   
#define STREAM_TIME64    44 /* ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s] 140 Y +Q28 [s] */   
#define STREAM_TIME64MS  45 /* ______________________mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm u42 [ms] 140 years */   
#define STREAM_TIME64ISO 46 /* ___..YY..YY..YY..YY..MM..MM..DD..DD..SS..SS.....offs..MM..MM..MM ISO8601 signed offset 2024-05-04T21:12:02+07:00  */   
#define STREAM_WGS84     47 /* <--LATITUDE 32B--><--LONGITUDE 32B-->  lat="52.518611" 0x4252130f   lon="13.376111" 0x4156048d - dual IEEE754 */   
#define STREAM_HEXBINARY 48 /* UTF-8 lower case hexadecimal byte stream */
#define STREAM_BASE64    49 /* RFC-2045 base64 for xsd:base64Binary XML data */
#define STREAM_STRING8   50 /* UTF-8 string of char terminated by 0 */
#define STREAM_STRING16  51 /* UTF-16 string of char terminated by 0 */

#define LAST_RAW_TYPE    64 /* coded on 6bits RAW_FMT0_LSB */



#define STREAM_PTRPHY      4    
#define STREAM_PTR27B      5    


/* constants for uint8_t itoa(char *s, int32_t n, int base) 
* string conversions to decimal and hexadecimal */

#define C_BASE2 2
#define C_BASE10 10
#define C_BASE16 16



/* ========================== MINIFLOAT 8bits ======================================*/

// Time constants for algorithm
// MiniFloat 76543210
//           MMMEEEEE x= MMM(0..7) << EEEEE(0..31) =[0..15e9] +/-1
// just for information: OFP8_E4M3 SEEEEMMM x= (sign).(1 + M/8).(2<<(E-7)) =[-8..+8] +/-1e-6
#define MINIF(m,exp) ((uint8_t)((m)<<5 | (exp)))
#define MINIFLOAT2Q31(x) ((((x) & 0xE0)>>5) << ((x) & 0x1F))
#define MULTIPLIER_MSB 7     
#define MULTIPLIER_LSB 5
#define   EXPONENT_MSB 4     
#define   EXPONENT_LSB 0

/*============================ BIT-FIELDS MANIPULATIONS ============================*/
/*
 *  stream constants / Macros.
 */
 
// We define a preprocessor macro that will allow us to add padding
// to a data structure in a way that helps communicate our intent.
// Example : 
//   struct alignas(4) Pixel {
//       char R, G, B;
//       PADDING_BYTES(1);
//   };
#define CONCATENATE_(a, b) a##b
#define CONCATENATE(a, b) CONCATENATE_(a, b)
#define PADDING_BYTES(N) char CONCATENATE(PADDING_MACRO__, __COUNTER__)[N]

#define SHIFT_SIZE(base,shift) ((base) << ((shift) << 2));           

#define MIN(a, b) (((a) > (b))?(b):(a))
#define MAX(a, b) (((a) < (b))?(b):(a))
#define ABS(a) (((a)>0)? (a):-(a))

#define MAXINT32 0x7FFFFFFFL
#define MEMCPY(dst,src,n) {uint32_t i; for(i=0;i<n;i++){dst[i]=src[i];}}
#define MEMSWAP(dst,src,n) {uint32_t i, x; for(i=0;i<n;i++){x=dst[i];dst[i]=src[i];src[i]=x;}}
#define MEMSET(dst,c,n) {uint32_t i; uint8_t *pt8=(uint8_t *)dst; for(i=0;i<n;i++){pt8[i]=c;} }


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

//#define LOG2BASEINWORD32 2 
//#define BASEINWORD32 (1<<LOG2BASEINWORD32)
// replaced #define LINADDR_UNIT_BYTE   1
//      #define LINADDR_UNIT_W32    4
//      #define LINADDR_UNIT_EXTD  64

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
    

