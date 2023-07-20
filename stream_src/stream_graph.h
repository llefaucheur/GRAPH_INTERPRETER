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
   

#ifndef cSTREAM_GRAPH_H
#define cSTREAM_GRAPH_H

/* 
    Graph data format :
    -------------------SHARED FLASH (RAM POSSIBLE)-----------------
    - size of the graph in Words
    [0] 27b RAM address of part/all the graph, HW-shared MEM configuration, which part is going in RAM
    [1] number of FORMAT, size of SCRIPTS
    [2] size of LINKEDLIST, number of STREAM_INSTANCES
    [3] number of ARCS, number of DEBUG registers
    -------------------

    IO "stream_format_io" (2 word per IO)
       Word0: ARC ID, domain, io platform index, in/out, command parameter, format
       Word1: default 32bits mixed-signal settings bit-fields
    
    FORMAT used by the arcs (3 words each) 
       Word0: Frame size, interleaving scheme, raw data type
       Word1: Nb Chan, Sampling rate, time-stamp format
       Word2: depends on IO Domain (audio mapping, picture format, IMU interleaving, ..)

    SCRIPTS in Flash (index given as a parmeter of arm_script())
       Word0+n: table of index to the byte-codes (or ASCII strings)
       Word1+n: Byte-codes 
       Scripts in RAM are in the parameter field of arm_script()

    -----------------SHARED FLASH/RAM (FOR NEWPARAM_LW2)----------    
      
    LINKED-LIST of SWC
       minimum 4 words/SWC
       Word0: header processor/architecture, nb arcs, SWCID, arc
       Word1+n: arcs * 2
       Word2+n: nb membanks, relocatable TCM, main instance 27b address
       Word3+n: Preset, New param!, Skip length, 
          byte stream: nbparams (255='all'), {tag, nbbytes, params}
       list Ends with the SWC ID 0x03FF 
    
    -----------------SHARED RAM-------------------------------

    STREAM INSTANCE "stream_local_instance" 3 words + 8 offsets max, 
        total (32bits only) = 4+1=5 words minimum, 4+8=11 words with multi-instances 32bits
        total (mix with 64bits) = 4+2=6 words, and 4+2*8=20 words with multiple-instances
        granularity step is 20 words with multiple instances 32 or 64bits
       Word0: identification "whoami", next SWC to run
       Word1: trace arc, 
       Word2: IO streams to scan (<32)
       Word3: on-going request on IO 
       Word4+n: up to 8 offsets to memory banks
    
    ARC descriptors (4 words each)
       Word0: base offsetm data format, need for flush after write
       Word1: size, debug result registers
       Word2: read index, ready for read, THR size/4, flow error and debug tasks index
       Word3: write index, ready for write, need realignment flag, locking byte
    
    DEBUG REGISTERS and vectors from ARC content analysis 
    
    BUFFERS memory banks (internal/external/LLRAM) used for FIFO buffers 
       and used for initializations, list of PACKSWCMEM results with fields
          SIZE, ALIGNMT, SPEED,
*/

/* max number of instances simultaneously reading the graph
   used to synchronize the RESET sequence in platform_al() */
#define MAX_NB_STREAM_INSTANCES 4 

/* max number of nodes installed at compilation time */
#define NB_NODE_ENTRY_POINTS 20


/* -------- GRAPH[0] 27b RAM address, HW-shared MEM & RAM copy config---- */
#define COPY_CONF_GRAPH0_COPY_ALL_IN_RAM 0
#define COPY_CONF_GRAPH0_FROM_LINKEDLIST 1
#define COPY_CONF_GRAPH0_FROM_ARCDESC 2
#define COPY_CONF_GRAPH0_ALREADY_IN_RAM 3
#define PACKSHARERAMSPLIT(share,RAMsplit) ((share<<3) + RAMsplit)
#define SHAREDRAM_GRAPH0_MSB U(3) 
#define SHAREDRAM_GRAPH0_LSB U(3) /* 1   */
#define  RAMSPLIT_GRAPH0_MSB U(2)
#define  RAMSPLIT_GRAPH0_LSB U(0) /* 3  */
#define GRAPH_RAM_OFFSET(L,G)     pack2linaddr_int(L,G[0])
#define GRAPH_RAM_OFFSET_PTR(L,G) pack2linaddr_ptr(L,G[0])

/* -------- GRAPH[1] number of FORMAT, size of SCRIPTS ---- */
#define SCRIPTS_SIZE_GR1_MSB U(31) 
#define SCRIPTS_SIZE_GR1_LSB U(10) /*  22 scripts size */
#define       NB_IOS_GR1_MSB U( 9) 
#define       NB_IOS_GR1_LSB U( 5) /*  5 Nb of I/O :  up to 32 IO streams */
#define    NBFORMATS_GR1_MSB U( 4) 
#define    NBFORMATS_GR1_LSB U( 0) /*  5 formats */

#define PACKFORMATIOSSCRIPT(LENscript,nIOs,nFMT) (((LENscript)<<10) | ((nIOs)<<5) | (nFMT))
#define GRAPH_LENSCRIPT(G) (((G[1])>>10) & 0x3FFFFF)
#define GRAPH_NB_IOS(G)    (((G[1])>> 5) & 0x1F)
#define GRAPH_NBFORMAT(G)  (((G[1])>> 0) & 0x1F)

/* -------- GRAPH[2] size of LINKEDLIST, number of STREAM_INSTANCES ---- */
#define LINKEDLIST_SIZE_GR2_MSB U(31) 
#define LINKEDLIST_SIZE_GR2_LSB U( 6) /* 26 size of the linkedList with the parameters */
#define  NB_ST_INSTANCE_GR2_MSB U( 5) /*    number of SW/HW instances using this graph */
#define  NB_ST_INSTANCE_GR2_LSB U( 0) /*  6 nb instance (nb proc x rtos instances) */

#define PACKLINKEDLNBINSTANCE(LinkedList,NbInstance) (((LinkedList)<<6) | (NbInstance))
#define GRAPH_SZ_LINKEDL(G) (((G[2])>> 6) & 0x3FFFFFF)
#define GRAPH_NBINSTANCE(G) (((G[2])>> 0) & 0x3F)

/* -------- GRAPH[3] number of ARCS, number of DEBUG registers ----*/
#define ______________GR3_MSB U(31) 
#define ______________GR3_LSB U(15) /* 17   */
#define DEBUGREG_SIZE_GR3_MSB U(14) 
#define DEBUGREG_SIZE_GR3_LSB U(11) /*  4 size of the debug area addressed with DEBUG_REG_ARCW1*/
#define       NB_ARCS_GR3_MSB U(10) 
#define       NB_ARCS_GR3_LSB U( 0) /* 11 up to 2K ARCs, see ARC0_LW1 */

#define PACKNBARCDEBUG(SizeDebug,NBarc) (((SizeDebug)<<8) | (NBarc))
#define GRAPH_SZDEBUG(G) (((G[3])>> 8) & 0xFFFFFF)
#define GRAPH_NB_ARCS(G) (((G[3])>> 0) & 0xFF)

#define GRAPH_HEADER_NBWORDS 4    /* GRAPH[0 .. 3] */

//#define GRAPH_INSTANCE_OFFSET(L,G)     pack2linaddr_int(L,G[1])
//#define GRAPH_LINKDLIST_ADDR(L,G) pack2linaddr_ptr(L,G[5])
//#define GRAPH_ARC_LIST_ADDR(L,G)  pack2linaddr_ptr(L,G[6])
//#define GRAPH_DEBUG_REGISTER(L,G) pack2linaddr_ptr(L,G[7])
//#define GRAPH_DEBUG_SCRIPT(L,G)   pack2linaddr_ptr(L,G[8])

#define LINKEDLIST_IS_IN_RAM() (MBANK_FLASH != RD(S->S0.graph[4],DATAOFF_ARCW0))

/* 
    ================================= GRAPH LINKED LIST =======================================
*/ 
#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */

/* number of SWC calls in sequence */
#define MAX_SWC_REPEAT 4u

/* ============================================================================== */ 

        /* word 0 - main Header */

#define  UNUSED_LW0_MSB U(31)
#define  UNUSED_LW0_LSB U(28) /* 4 -------  */
#define  PROCID_LW0_MSB U(27) /*   same as PROCID_PARCH (stream instance) */
#define  PROCID_LW0_LSB U(25) /* 3 processor index  */  
#define  ARCHID_LW0_MSB U(24)
#define  ARCHID_LW0_LSB U(22) /* 3 processor architectures */
#define  NBARCW_LW0_MSB U(21) 
#define  NBARCW_LW0_LSB U(18) /* 4  total nb arcs */
#define ARCLOCK_LW0_MSB U(17) 
#define ARCLOCK_LW0_LSB U(15) /* 3  arc(tx) index used for locking */
#define ARCSRDY_LW0_MSB U(14) 
#define ARCSRDY_LW0_LSB U(12) /* 3  nb arcs used in streaming and checked by the scheduler */
#define SWC_IDX_LW0_MSB U(11) 
#define SWC_IDX_LW0_LSB U( 0) /* 12 0=nothing, swc index of node_entry_points[] */

        /* word 1 - arcs */

//#if (GRAPH_INTERPRETER_VERSION & 0xFF00) == 0x0100
#define MAX_NB_STREAM_PER_SWC 16
//#endif

#define ARC_HEADER_BYTE_OFFSET 4  /* ARCs hearder starts 4bytes after main header */
#define ARC_RX0TX1_MASK 0x800 /* MSB gives the direction of the arc */
#define ARC_RX0TX1_CLEAR 0x7FF 


#define ______LW1_MSB U(31) 
#define ______LW1_LSB U(28) /*  4   */
#define DBGB1_LW1_MSB U(27) 
#define DBGB1_LW1_LSB U(26) /*  2  debug register bank for ARC1 */
#define DBGB0_LW1_MSB U(25) 
#define DBGB0_LW1_LSB U(24) /*  2  debug register bank for ARC0 */
#define  ARC1_LW1_MSB U(23)
#define  ARC1_LW1_LSB U(12) /* 12  ARC1  11 usefull bits + 1 MSB to tell rx0tx1 */
#define  ARC0_LW1_MSB U(11)
#define  ARC0_LW1_LSB U( 0) /* 12  ARC0, (11+1) up to 2K ARCs */

        /* word 2 - memory banks */
#define   ________LW2_MSB U(31) /*    */
#define   ________LW2_LSB U(31) /*  1 */
#define  TCM_INST_LW2_MSB U(30) /*    SWC has a relocatable scratch bank to TCM, the scheduler reloads the first */
#define  TCM_INST_LW2_LSB U(30) /*  1    intPtr_t address of the instance with the TCM base address*/
#define   NBALLOC_LW2_MSB U(29)
#define   NBALLOC_LW2_LSB U(27) /*  3 number of memory segments to give at RESET */
#define BASEIDXOFFLW2_MSB U(26) 
#define   DATAOFF_LW2_MSB U(26)
#define   DATAOFF_LW2_LSB DATAOFF_ARCW0_LSB /*  3 bits 64bits offset index see idx_memory_base_offset */
#define   BASEIDX_LW2_MSB BASEIDX_ARCW0_MSB /*    buffer address 22 + offset = 25 bits */
#define   BASEIDX_LW2_LSB U( 0)             /* 22 base address in WORD32 */
#define BASEIDXOFFLW2_LSB U( 0) /*    27 bits */

#define MAXNBMEMSEGMENTS (1<<(NBALLOC_LW2_MSB - NBALLOC_LW2_LSB + 1) - 1)

        /* word 3 - parameters
          BOOTPARAMS    : 
            unused      : 1  
            paramtype   : 1  0:all params   1:params sent 1-by-1
            skip        :20  nb of word32 to skip at run time, 0 means no parameter.
            verbose     : 1  level of details in the debug trace
            new param   : 1  a script has updated new parameters
            preset LSB  : 8; preset index (SWC delivery)

            if skip> 0
                sequence of pairs {8b index/tag / 255=ALL; 24b byte length} { parameter(s) }
        */
#define PARAM_MAX_MANTISSA (1<<14)-1 /* SKIP = Mantissa << Exponent */
#define PARAM_MAX_EXPONENT (1<<2)-1  /*  format: EE.MM.MMMM.MMMM.MMMM */
#define FPE2M14TOINT(FP) (((FP) & PARAM_MAX_MANTISSA) << ((unsigned int)FP>>6))
#define MAX_FPE4M6 100000 // to avoid memory overflow

#define  ALLPARAM_ (uint32_t)255 /*  nbparam=255 means "full set of parameters loaded from binary format" */

#define  __UNUSED_LW3_MSB U(31) /*      */
#define  __UNUSED_LW3_LSB U(31) /* 1    */
#define  SELPARAM_LW3_MSB U(30) /*      */
#define  SELPARAM_LW3_LSB U(30) /* 1 1 mean "send parameters one by one", 0 means "all parameters" */
#define W32LENGTH_LW3_MSB U(29) /*    if >4MB are needed then use an arc to a buffer */
#define W32LENGTH_LW3_LSB U(10) /*20 skip this : number of uint32 to skip the boot parameters */
#define   VERBOSE_LW3_MSB U( 9)
#define   VERBOSE_LW3_LSB U( 9) /* 1 verbose debug trace */
#define  NEWPARAM_LW3_MSB U( 8) /*   when the list is in flash the parameters at set once at reset */
#define  NEWPARAM_LW3_LSB U( 8) /* 1 new param when the list is in RAM */
#define    PRESET_LW3_MSB U( 7)
#define    PRESET_LW3_LSB U( 0) /* 8 preset   precomputed configurations, manifest's uint8_t *parameter_presets; */


/* ================================= */

#define GRAPH_LAST_WORD_MSB SWC_IDX_LW0_MSB
#define GRAPH_LAST_WORD_LSB SWC_IDX_LW0_LSB
#define GRAPH_LAST_WORD U((U(1)<<U(SWC_IDX_LW0_MSB- SWC_IDX_LW0_LSB+1U))-1U)

/*-------------------------------------------------------------------------------------*/

//#define SETARCINLINKLST(arcout,arcin) (((arcout) <<16) | (arcin))
//
//#define PACKARCS_LINKLST(arcout,arcin,word) {(arcin) = (word) & 0xFF; (arcout) = ((word)>>16) & 0xFF; }
//#define PACKARCINLINKLST(arcin,word) {(arcin) = (word) & 0xFF; }
//#define PACKARCOUTLINKLST(arcout,word) {(arcout) = ((word)>>16) & 0xFF; }

/* WORD n+1 = instance pointer if (INSTPT == 1) */

//#define MAX_NB_ARCS_DEBUG_REGS U(4)            /* 4bits reserved in DEBUG_REG_ARCW1_LSB */
//#define MAX_NB_COMMUNICATION_RINGS U(32)       /* 5bits   */
//#define NB_COMMUNICATION_RINGS U(4) /* 4 input / output of the arc, + 0 inter-processor RINGs, index of io_config[] */
//#define MAX_NB_INTERNAL_FORMATS (U(4)*STREAM_FORMAT_SIZE_W32)  /* max 32, see PRODUCFMT_ARCW0_LSB, digital global 
                                            //(data and frame format) to avoid repeating the same information on each arc */
//#define MAX_NB_GRAPH_LIST 200u               /* The graph description, compiled for the TXT input format */
//#define MAX_NB_ARCS 20u                      /* memory allocation for arcs, ranging 2 .. 8 words of 32bits */
//#define MAX_NB_IO_ARCS 10u                   /* IO arcs monitored in the scheduler loop of each STREAM instance  */
//#define MAX_NBW_TRACE_BUFFER 32u            
//#define MAX_NBW_COMMAND_BUFFER 32u           
//#define NB_ARCS_DEBUG 2u                     /* memory allocation for arcs debug */

/*=============================== STREAM_INSTANCES =================================*/
//
//struct stream_local_instance        /* structure allocated to each STREAM instance */

#define STREAM_INSTANCE_SIZE 20         /* 4 words + 16words offset maximum (8 x 64bits addresses) */
#define STREAM_INSTANCE_WHOAMI_PORTS 0  /* _PARCH_ fields */
#define STREAM_INSTANCE_PARAMETERS 1    /* _PARINST_ fields */
#define STREAM_INSTANCE_IOMASK 2        /* _IOMASK_ fields */
#define STREAM_INSTANCE_IOREQ 3         /* _IOREQ_ fields */
#define STREAM_INSTANCE_OFFSETS 4       /* _OFFSET_ fields */

/* word 0 , identification "whoami", next SWC to run*/
#define    INST_ID_PARCH_MSB U(31)
#define     WHOAMI_PARCH_MSB U(31)
#define   INSTANCE_PARCH_MSB U(31)  /* avoid locking an arc by the same processor, but different RTOS instances*/
#define   INSTANCE_PARCH_LSB U(30)  /* 2 [0..3] up to 4 instances per processors */
#define     PROCID_PARCH_MSB U(29)  /*   indexes from Manifest(tools) and PLATFORM_PROC_ID */
#define     PROCID_PARCH_LSB U(27)  /* 3 processor index [0..7] for this architecture 0="commander processor" */  
#define     ARCHID_PARCH_MSB U(26)
#define     ARCHID_PARCH_LSB U(24)  /* 3 [1..7] processor architectures 1="commander processor architecture" */
#define     WHOAMI_PARCH_LSB U(24)  /*   whoami used to lock a SWC to specific processor or architecture */
#define    INST_ID_PARCH_LSB U(24)  /*   8 bits identification for locks */
#define SWC_W32OFF_PARCH_MSB U(23)  
#define SWC_W32OFF_PARCH_LSB U( 0) /* 24   offset in words to the NEXT SWC to be executed */  

#define PACKWHOAMI(INST,PROCIDX,ARCH,BOUNDARIES) (((INST)<<30)|((PROCIDX)<<27)|((ARCH)<<24)|(BOUNDARIES))
/* maximum number of processors = nb_proc x nb_arch */
#define MAX_GRAPH_NB_PROCESSORS ((1<<(PROCID_PARCH_MSB-PROCID_PARCH_LSB+1))*(1<<(ARCHID_PARCH_MSB-ARCHID_PARCH_LSB+1)))

/* word 1 trace arc */
#define __________PARINST_MSB U(31)
#define __________PARINST_LSB U( 8)  /*   */
#define TRACE_ARC_PARINST_MSB U( 7)
#define TRACE_ARC_PARINST_LSB U( 0)  /*  8 index of the arc used for debug trace / instance */

/* word 2 IO streams to scan , max = 32 */
// 32 = 1 << (NB_IOS_GR1_MSB - NB_IOS_GR1_LSB + 1)
#define MAX_GRAPH_NB_IO_STREAM 32

// #define STREAM_INSTANCE_IOMASK 2 /* _IOMASK_ fields */
#define BOUNDARY_IOMASK_MSB U(31)  
#define BOUNDARY_IOMASK_LSB U( 0)   /* 32 boundary ports in STREAM_FORMAT_IO to scan */  

/* word 3 on-going request on IO */
// #define STREAM_INSTANCE_IOREQ 3  /* _IOREQ_ fields */
#define REQMADE_IO_MSB U(31)  
#define REQMADE_IO_LSB U( 0)        /* 32 boundary ports data transfer on-going */  

/* word 4 long offsets */
#define WORD4_DINST_OFFSET_ 4     /* table of offsets (32/64bits) to memory banks */

/* ----------- for stream_param_t : intPtr_t[2] -----------------*/
//#define NB_STREAM_PARAM 1
//#define STREAM_PARAM_CTRL 0     /* index of the Stream instance */
//
//#define STREAM_PARAM_BITFIELDS 
//#define  UNUSED_PARAM_MSB U(31)
//#define  UNUSED_PARAM_LSB U(18) /* 14      */
//#define LASTSWC_PARAM_MSB U(17)
//#define LASTSWC_PARAM_LSB U( 6) /* 12 index to the last position scanned by the scheduler */
//#define INSTIDX_PARAM_MSB U( 5) 
//#define INSTIDX_PARAM_LSB U( 0) /*  6 Stream instance index of the graph readers */


/*================================= STREAM_FORMAT_IO ================================
      The graph hold a table of uint32_t "stream_format_io" [LAST_IO_FUNCTION_PLATFORM]
*/

//enum input_output_command_id {  IOCOMMAND_IOFMT
#define IO_COMMAND_SET_BUFFER    U(0)
#define IO_COMMAND_DATA_MOVE     U(1)


#define STREAM_IOFMT_SIZE_W32 2   /* one word for settings controls + 1 for instance selection and mixed-signal settings */

#define RX0_TO_GRAPH 0u
#define TX1_FROM_GRAPH 1u

#define    DOMAIN_IOFMT_MSB U(31)  
#define    DOMAIN_IOFMT_LSB U(26)  /* 6  64 stream_io_domain  */
#define  INSTANCE_IOFMT_MSB U(25)  
#define  INSTANCE_IOFMT_LSB U(22)  /* 4  selection of 16 GPIO for example */
#define   EXDTCMD_IOFMT_MSB U(21)  
#define   EXDTCMD_IOFMT_LSB U(19)  /* 3  extra_command_id */
#define IOCOMMAND_IOFMT_MSB U(18)  
#define IOCOMMAND_IOFMT_LSB U(18)  /* 1  command_id 0_set_1_copy */
#define   SERVANT_IOFMT_MSB U(17)  
#define   SERVANT_IOFMT_LSB U(17)  /* 1  1=IO_IS_SERVANT */
#define    RX0TX1_IOFMT_MSB U(16)  /*    direction of the stream */
#define    RX0TX1_IOFMT_LSB U(16)  /* 1  0 : to the graph    1 : from the graph */
#define FW_IO_IDX_IOFMT_MSB U(15)  /*    enum codes to address platform_io_functions[] */
#define FW_IO_IDX_IOFMT_LSB U(11)  /* 5  platform_io [fw_io_idx] -> io_start(parameter) */
#define   IOARCID_IOFMT_MSB U(10)  
#define   IOARCID_IOFMT_LSB U( 0)  /* 11  Arc */


#define SETTINGS_IOFMT2_MSB U(31)  
#define SETTINGS_IOFMT2_LSB U( 0) /* 32  second word : mixed-signal settings */
/*=====================================================================================*/                          

/*
    commands from the application, and from Stream to the SWC

    SWC_COMMANDS 
*/

#define ________CMD_MSB U(31)
#define ________CMD_LSB U(28) /* 4 */
#define  PRESET_CMD_MSB U(27)       
#define  PRESET_CMD_LSB U(20) /* 8 preset */
#define    NARC_CMD_MSB U(19)       
#define    NARC_CMD_LSB U(16) /* 4 number of arcs */
#define     TAG_CMD_MSB U(15)       
#define     TAG_CMD_LSB U( 8) /* 8 selection / debug arc index */
#define    INST_CMD_MSB U( 7)       
#define    INST_CMD_LSB U( 4) /* 4 instance index RD(S->scheduler_control, INSTANCE_SCTRL) */
#define COMMAND_CMD_MSB U( 3)       
#define COMMAND_CMD_LSB U( 0) /* 4 command */
#define PACK_COMMAND(PRESET,NARC,TAG,INST,CMD) (((PRESET)<<20)|((NARC)<<16)|((TAG)<<8)|((INST)<<4)|(CMD))

//enum stream_command (8bits LSB)
//{
#define STREAM_RESET 1u             /* func(STREAM_RESET, *instance, * memory_results) */
#define STREAM_SET_PARAMETER 2u     /* swc instances are protected by multithread effects when changing parmeters on the fly */
#define STREAM_READ_PARAMETER 3u    
#define STREAM_RUN 4u               /* func(STREAM_RUN, instance, *in_out) */
#define STREAM_STOP 5u              /* func(STREAM_STOP, instance, 0)  swc calls free() if it used stdlib's malloc */
#define STREAM_INTERPRET_COMMANDS 6u /* arm_stream (STREAM_INTERPRET_COMMANDS, byte stream, 0, 0)*/
//#define STREAM_DELETE_NODE 
//#define STREAM_INSERT_NODE
//};

/*
    commands from the SWC to Stream
*/
//enum stream_services
//{
#define STREAM_SERVICE_RESET 10u
#define STREAM_NODE_REGISTER 11u

#define STREAM_FORMAT_UPDATE_FS 12u       /* SWC information for a change of stream format, sampling, nb of channel */
#define STREAM_FORMAT_UPDATE_NCHAN 13u     /* raw data sample, mapping of channels, (web radio use-case) */
#define STREAM_FORMAT_UPDATE_RAW 14u
#define STREAM_FORMAT_UPDATE_MAP 15u

#define STREAM_AUDIO_ERROR 16u        /* PLC applied, Bad frame (no header, no synchro, bad data format), bad parameter */
    
#define STREAM_DEBUG_TRACE 17u
#define STREAM_DEBUG_TRACE_STAMPS 18u

#define STREAM_SERVICE_ENUMERATE 19u

    //STREAM_DEBUG_TRACE, STREAM_DEBUG_TRACE_1B, STREAM_DEBUG_TRACE_DIGIT, 
    //STREAM_DEBUG_TRACE_STAMPS, STREAM_DEBUG_TRACE_STRING,

    //STREAM_SAVE_HOT_PARAMETER, 
    //STREAM_READ_TIME, STREAM_READ_TIME_FROM_START, STREAM_TIME_DIFFERENCE, 
    //STREAM_TIME_CONVERSION,  
    //STREAM_LOW_POWER,     /* interface to low-power platform settings, "wake-me in 24h with deep-sleep in-between" */
    //STREAM_PROC_ARCH,     /* returns the processor architecture details, used before executing specific assembly codes */

    /* STREAM_STANDARD_LIBRARIES LSB4 = 1 ------------------------------------------------------------------- */
//#define STREAM_STANDARD_LIBRARIES 0x00000011u

    /* stdio.h */
    //STREAM_FEOF, STREAM_FFLUSH, STREAM_FGETC, STREAM_FGETS, STREAM_FOPEN, STREAM_FPRINTF, STREAM_FPUTC,
    //STREAM_FPUTS, STREAM_FREAD, STREAM_FSCANF, STREAM_FWRITE,

    /* stdlib.h */
    //STREAM_ABS, STREAM_ATOF, STREAM_ATOI, STREAM_ATOL, STREAM_ATOLL, STREAM_CALLOC, STREAM_FREE, STREAM_MALLOC, 
    //STREAM_RAND, STREAM_SRAND, STREAM_STRTOF, STREAM_STRTOL,

    /* string.h */
    //STREAM_MEMCHR, STREAM_MEMCMP, STREAM_MEMCPY, STREAM_MEMMOVE, STREAM_MEMSET, STREAM_STRCHR, STREAM_STRLEN,
    //STREAM_STRNCAT, STREAM_STRNCMP, STREAM_STRNCPY, STREAM_STRSTR, STREAM_STRTOK,

    /* STREAM_MATH_LIBRARIES LSB4 = 2 ------------------------------------------------------------------- */
    //STREAM_MATH_LIBRARIES = 0x00000012,           

    /* time.h */
    //STREAM_ASCTIMECLOCK, STREAM_DIFFTIME,

    /* CMSIS-DSP/ML */
    //STREAM_SET_ACCURACY, STREAM_SET_MEMORY_CONSTRAINT, STREAM_FREE_COMPUTE_INSTANCE, 

    //STREAM_TFLITE                         /* cascade of TFLite {commands,pointers} */
    //STREAM_WINDOWS,                       /* windowing for spectral estimations */
    //STREAM_BIQUAD_CASCADE,                /* filters - there a state and associated instance */
    //STREAM_FIR,
    //STREAM_MATRIX_MULT,
    //STREAM_2D_FILTER, 
    //STREAM_INTERP, 
    //STREAM_2D_INTERP, 
    //STREAM_MATH_TABLE, 

    //#define STREAM_RFFT 0x00010012u 
    //#define STREAM_CFFT 0x00020012u
    //#define STREAM_CIFFT 0x00030012u
    //STREAM_CNORM,                         
    //STREAM_RMAX,                          
    //STREAM_RSORT,                          

    /* STREAM_MULTIMEDIA_LIBRARIES LSB4 = 3 ------------------------------------------------------------------- */
//#define STREAM_MULTIMEDIA_LIBRARIES 0x00010013u

    /* audio Codecs */
    //STREAM_ALAW_ENC, STREAM_ALAW_DEC, STREAM_MULAW_ENC, STREAM_MULAW_DEC, STREAM_IMADPCM_ENC, STREAM_IMADPCM_DEC,
    //STREAM_LPC_ENC, STREAM_LPC_DEC,

    /* image                   */
    //STREAM_JPEG_ENC, STREAM_JPEG_DEC, STREAM_PNG_ENC, STREAM_PNG_DEC,

    /* STREAM_APPLICATION_LIBRARIES LSB4 = 4 ------------------------------------------------------------------- */
    //STREAM_TEA,

//};



#endif /* #ifndef cSTREAM_GRAPH_H */

#ifdef __cplusplus
}
#endif
 
