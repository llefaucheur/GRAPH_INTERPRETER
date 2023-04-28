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
#ifndef cSTREAM_GRAPH_H
#define cSTREAM_GRAPH_H

/* 
    Graph data format :
    ----------------------SHARED RAM/FLASH-----------------------------
    header of the graph : 
        [0] Offset/LenRAM copyInRAMConfig bits + SharedMem bit + nb-io (8bits)
        [1] offset to RAM
        [2] offset to IO configuration 
        [3] address of format(s)
        [4] address of the linkedlist 
        [5] address of the CMSIS-Stream instances
        [6] address of arc decriptors 
        [7] address of the debug registers
        [8] address of the debug script
    ...
    [2] : start of IO "stream_format_io" (2 word per IO)
             Word0: ARC ID, on-going request, domain, io platform index, in/out, command parameter, format
             Word1: Settings bit-fields
    ...
    [3] : start of FORMAT used by the arcs (3 words each) 
             Word0: Frame size, interleaving scheme, raw data type
             Word1: Nb Chan, Sampling rate, time-stamp format
             Word2: depends on IO Domain
    ...
    [4] : LINKED-LIST of SWC
             minimum 3 words/SWC
             Word0: SWCID, arc0/arc1. arc++, TCM, verbose, new param
             Word1: pack address of instance to nb memreq words, nb memreq, 
             Word2: Preset, Arc/Proc, Skip (0='no param')
                byte stream: nbparams (255='all'), {tag, nbbytes, params}
             Word3: extension for arc2/arc3, in/out, HQoS
             list Ends with SWC ID 0x3FFF 
    ...
    -----------------------SHARED RAM-------------------------------
    [5] : STREAM INSTANCE "stream_local_instance" 3 words 
             Word0: platform streams to scan, identification "whoami"
             Word1: trace arc
             Word2: pointer to the list of executable nodes from this instance
    ...
    [6] : start of ARC descriptors (4 words each)
             Word0: base offsetm data format, need for flush after write
             Word1: size, debug result registers
             Word2: read index, ready for read, THR size/4, flow error and debug tasks index
             Word3: write index, ready for write, need realignment flag, locking byte
    ...
    [7] : debug registers and vectors from ARC content analysis 
    ...
    [8] : script 
             Word1_scripts : script size, global register used + backup MEM, stack size
             Wordn_scripts : stack and byte-codes, see stream_execute_script()
    ... 
    [-] : memory areas used for FIFO buffers 
             and used for initializations, list of PACKSWCMEM results with fields
                SIZE, ALIGNMT, SPEED, zero on last field 
    ...

*/

/* max number of instances simultaneously reading the graph
   used to synchronize the RESET sequence in platform_al() */
#define MAX_NB_STREAM_INSTANCES 4 

#define NB_NODE_ENTRY_POINTS 24
/* special case of short format script */
#define SCRIPT_SW_IDX 0u            /* index of node_entry_point_table[SCRIPT_SW_IDX] = arm_script*/

/* -------- GRAPH[0]--CONFIG---- */
#define COPY_CONF_GRAPH0_COPY_ALL_IN_RAM 0
#define COPY_CONF_GRAPH0_COPY_PARTIALLY  1
#define COPY_CONF_GRAPH0_ALREADY_IN_RAM  2

#define LENGTHRAM_GRAPH0_MSB U(31) /*    4kB of initialized RAM, without the buffers */
#define LENGTHRAM_GRAPH0_LSB U(20) /* 10 nbW32 to copy in RAM */
#define    OFFSET_GRAPH0_MSB U(19) /*    64kB can stay in Flash and 4kB in RAM */
#define    OFFSET_GRAPH0_LSB U( 8) /* 14 nbW32 offset to the area to put int RAM */
#define COPY_CONF_GRAPH0_MSB U( 7) 
#define COPY_CONF_GRAPH0_LSB U( 6) /* 2  total, partial or no copy in RAM */
#define SHAREDMEM_GRAPH0_MSB U( 5) 
#define SHAREDMEM_GRAPH0_LSB U( 5) /* 1  tells if the graph RAM is "shareable" */
#define      NBIO_GRAPH0_MSB U( 4)       
#define      NBIO_GRAPH0_LSB U( 0) /* 5  nb of external boundaries of the graph */

#define GRAPH_RAM_OFFSET()     pack2linaddr_int(arm_stream_global.graph[1])
#define GRAPH_RAM_OFFSET_PTR() pack2linaddr_ptr(arm_stream_global.graph[1])
#define GRAPH_IO_CONFIG_ADDR() pack2linaddr_ptr(arm_stream_global.graph[2])
#define GRAPH_FORMAT_ADDR()    pack2linaddr_ptr(arm_stream_global.graph[3])
#define GRAPH_LINKDLIST_ADDR() pack2linaddr_ptr(arm_stream_global.graph[4])
#define GRAPH_STREAM_INST()    pack2linaddr_ptr(arm_stream_global.graph[5])
#define GRAPH_ARC_LIST_ADDR()  pack2linaddr_ptr(arm_stream_global.graph[6])
#define GRAPH_DEBUG_REGISTER() pack2linaddr_ptr(arm_stream_global.graph[7])
#define GRAPH_DEBUG_SCRIPT()   pack2linaddr_ptr(arm_stream_global.graph[8])

#define LINKEDLIST_IS_IN_RAM() (MBANK_FLASH != RD(arm_stream_global.graph[4],DATAOFF_ARCW0))

/* 
    ================================= GRAPH LINKED LIST =======================================
*/ 
#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */

/* number of SWC calls in sequence */
#define MAX_SWC_REPEAT 4u

#define MAX_NB_ARC_PER_SWC_V0 2

/* ============================================================================== */ 
#define   UNUSED_GI_MSB U(31)
#define   UNUSED_GI_LSB U(29) /* 2 -------  */
#define SEQUENCE_GI_MSB U(31) /*   overlay the buffer when processing is done in sequences */
#define SEQUENCE_GI_LSB U(29) /* 1 save the linked_list_ptr to process next SWC from this point */
#define NEWPARAM_GI_MSB U(28) 
#define NEWPARAM_GI_LSB U(28) /* 1 new param when the list is in RAM */
#define  VERBOSE_GI_MSB U(27)
#define  VERBOSE_GI_LSB U(27) /* 1 verbose debug trace */
#define TCM_INST_GI_MSB U(26) /*   SWC has a relocatable bank to TCM (working area)  the scheduler reloads the first */
#define TCM_INST_GI_LSB U(26) /* 1    intPtr_t address of the instance with the TCM base address*/
#define    ARCX2_GI_MSB U(25) 
#define    ARCX2_GI_LSB U(24) /* 2  additional ARCs {0,1,2,3): total max = 2 + (3 words)x3 arcs = 11 arcs per component */
#define   ARCOUT_GI_MSB U(23) 
#define   ARCOUT_GI_LSB U(16) /* 8  OUT */
#define    ARCIN_GI_MSB U(15) 
#define    ARCIN_GI_LSB U( 8) /* 8  IN */
#define  SWC_IDX_GI_MSB U( 7) 
#define  SWC_IDX_GI_LSB U( 0) /* 8 0<>nothing, swc index of node_entry_points[] */

//#define ARC0(arc) RD(arc,ARCIN_GI)
//#define ARC1(arc) RD(arc,ARCOUT_GI)

/*----------------------------------------
   INSTANCE = arcFormat base address + nb of memory segments 
*/
#define STREAM_LIST_GI_LONG_FORMAT_WORD_1   /* instance */

#define    UNUSED_INST_MSB U(31)
#define    UNUSED_INST_LSB U(30) /*  2      */
#define   NBALLOC_INST_MSB U(29)
#define   NBALLOC_INST_LSB U(27) /*  3 number of memory segments to give at RESET */
#define   DATAOFF_INST_MSB U(26) /*    arcs are using offset=0/1, same format for SWC instances: 0..4 */
#define   DATAOFF_INST_LSB U(24) /*  3 bits 64bits offset index see idx_memory_base_offset */
#define BASESHIFT_INST_MSB U(23) /*    check physical_to_offset() before changing data */
#define BASESHIFT_INST_LSB U(22) /*  2 bits shifter */
#define   BASEIDX_INST_MSB U(21) /*    buffer address (22 + 2) + offset = 27 bits */
#define   BASEIDX_INST_LSB U( 0) /* 22 base address 22bits + 2bits exponent ((base) << ((shift) << 2)) */
#define BASEIDXOFFINST_LSB U( 0) 

/*----------------------------------------
  BOOTPARAMS Word0 : 
    preset LSB  : 8; preset index (SWC delivery)
    unused      : 4; 
    skip        :12  nb of word32 to skip at run time
    architecture: 3
    processor id: 3

  if Skip == 0 
    only preset is used for compatibility with the short format
  else

  BOOTPARAMS Word1 : byte stream
    nbparams    : 8; nb of parameters changed, on top of the preset ones
    format if nbparams = ALLPARAM_ (255)
        { all parameter(s) }
    else
        sequence of pairs {8b index/tag ; 8b nb of bytes } 
            { parameter(s) }
*/
#define STREAM_LIST_GI_LONG_FORMAT_WORD_2   /* parameters */
#define  ALLPARAM_    (uint32_t)255 //   nbparam=255 means "full set of parameters loaded from binary format"
#define PRESET_0 0
#define PRESET_1 1
#define PRESET_2 2
#define PRESET_3 3
#define  PACKPARAMTAG(tag,preset) U((((uint32_t)(tag))<<16) | (preset))

#define    UNUSED_BP_MSB U(31)
#define    UNUSED_BP_LSB U(30) /*  2  */
#define    PROCID_BP_MSB U(29) /*    same as PROCID_PARCH (stream instance) */
#define    PROCID_BP_LSB U(27) /*  3 processor index [0..7] for this architecture 0="master processor" */  
#define    ARCHID_BP_MSB U(26)
#define    ARCHID_BP_LSB U(24) /*  3 [1..7] processor architectures 0="any" 1="master processor architecture" */
#define N32LENGTH_BP_MSB U(23)  
#define N32LENGTH_BP_LSB U(12) /* 12 skip this : number of uint32 to skip the boot parameters */
#define    PRESET_BP_MSB U(11)
#define    PRESET_BP_LSB U( 0) /* 12 preset   precomputed configurations, manifest's uint8_t *parameter_presets; */

/*----------------------------------------
   ARC extension  +3 arcs with QoS on the most important one
   Up to 11 arcs can be managed streams of different domain
     example: a motor control subsystem receiving several commands
     and position sensors, and delivering several current controls
     (from https://skill-lync.com/student-projects/final-project-design-of-an-electric-vehicle-308)

*/
#define STREAM_LIST_GI_LONG_FORMAT_WORD_3   
#define STREAM_LIST_GI_LONG_FORMAT_WORD_4   /* 3 ARCs per word */
#define STREAM_LIST_GI_LONG_FORMAT_WORD_5   

//#define      UNUSED_H_MSB U(31)
//#define      UNUSED_H_LSB U(31) /* 1  */
//#define  HIGHQOSOUT_H_MSB U(30) 
//#define  HIGHQOSOUT_H_LSB U(29) /* 2  */
//#define   HIGHQOSIN_H_MSB U(28) 
//#define   HIGHQOSIN_H_LSB U(27) /* 2  */
//#define ARC4IN0OUT1_H_MSB U(26)
//#define ARC4IN0OUT1_H_LSB U(26) /* 1  */
//#define        ARC4_H_MSB U(26)
//#define        ARC4_H_LSB U(18) /* 8  */
//#define ARC3IN0OUT1_H_MSB U(17)
//#define ARC3IN0OUT1_H_LSB U(17) /* 1  */
//#define        ARC3_H_MSB U(16)
//#define        ARC3_H_LSB U( 9) /* 8  */
//#define ARC2IN0OUT1_H_MSB U( 8)
//#define ARC2IN0OUT1_H_LSB U( 8) /* 1  */
//#define        ARC2_H_MSB U( 7)
//#define        ARC2_H_LSB U( 0) /* 8  */



/* ================================= */

#define GRAPH_LAST_WORD_MSB SWC_IDX_GI_MSB
#define GRAPH_LAST_WORD_LSB SWC_IDX_GI_LSB
#define GRAPH_LAST_WORD U((U(1)<<U(SWC_IDX_GI_MSB- SWC_IDX_GI_LSB+1U))-1U)

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
//#define MAX_NB_INTERNAL_FORMATS (U(4)*STREAM_FORMAT_SIZE_W32)  /* max 32, see FORMATIDX_ARCW0_LSB, digital global 
                                            //(data and frame format) to avoid repeating the same information on each arc */
//#define MAX_NB_GRAPH_LIST 200u               /* The graph description, compiled for the TXT input format */
//#define MAX_NB_ARCS 20u                      /* memory allocation for arcs, ranging 2 .. 8 words of 32bits */
//#define MAX_NB_IO_ARCS 10u                   /* IO arcs monitored in the scheduler loop of each STREAM instance  */
//#define MAX_NBW_TRACE_BUFFER 32u            
//#define MAX_NBW_COMMAND_BUFFER 32u           
//#define NB_ARCS_DEBUG 2u                     /* memory allocation for arcs debug */

/*=============================== STREAM INSTANCES =================================*/

/* PROCESSORS FORMAT packed_info in 32bits : -------------------------  */
/* synchronization BYTE, used for locking SWC on arc1 */
#define  INST_ID_PARCH_MSB U(31)
#define   WHOAMI_PARCH_MSB U(31)
#define INSTANCE_PARCH_MSB U(31)  /* avoid locking an arc by the same processor, but different RTOS instances*/
#define INSTANCE_PARCH_LSB U(30)  /* 2 [0..3] up to 4 instances per processors */
#define   PROCID_PARCH_MSB U(29)  /*   indexes from Manifest(tools) and PLATFORM_PROC_ID */
#define   PROCID_PARCH_LSB U(27)  /* 3 processor index [0..7] for this architecture 0="master processor" */  
#define   ARCHID_PARCH_MSB U(26)
#define   ARCHID_PARCH_LSB U(24)  /* 3 [1..7] processor architectures 0="any" 1="master processor architecture" */
#define   WHOAMI_PARCH_LSB U(24)  /*   whoami used to lock a SWC to specific processor or architecture */
#define  INST_ID_PARCH_LSB U(24)  /*   8 bits identification for locks */
#define BOUNDARY_PARCH_MSB U(23)  
#define BOUNDARY_PARCH_LSB U( 0) /* 24 boundary ports in STREAM_FORMAT_IO to scan */  

#define PACKWHOAMI(INST,PROCIDX,ARCH,BOUNDARIES) (((INST)<<30)|((PROCIDX)<<27)|((ARCH)<<24)|(BOUNDARIES))

#define    UNUSED_PARINST_MSB U(31)  /* 12 */ 
#define    UNUSED_PARINST_LSB U(20)    
#define   LASTSWC_PARINST_MSB U(19)  /* 12 last linkedList index used*/ 
#define   LASTSWC_PARINST_LSB U( 8)    
#define TRACE_ARC_PARINST_MSB U( 7)
#define TRACE_ARC_PARINST_LSB U( 0)  /*  8 index of the arc used for debug trace / instance */

///* struct stream_local_instance */        /* structure allocated to each STREAM instance */
//{     uint32_t whoami_ports;              /* PACKWHOAMI */
//      uint32 parameters;
//      p_stream_node *node_entry_points;   /* all the nodes visible from this processor */

#define STREAM_INSTANCE_SIZE 3
#define STREAM_INSTANCE_WHOAMI_PORTS 0          /* _PARCH_ fields */
#define STREAM_INSTANCE_PARAMETERS 1            /* _PARINST_ fields */
#define STREAM_INSTANCE_NODE_ENTRY_POINTS 2     /* 27 bits */




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


/*================================= STREAM CONTROL ================================*/
/* 
    Platform control (RAM) => HW IO
*/
//enum input_output_command_id {  /*    */
#define IO_COMMAND_NONE          U(0)  /* not a ring buffer located on graph ports, or slave protocol */
#define IO_COMMAND_SET_BUFFER_RX U(1)
#define IO_COMMAND_SET_BUFFER_TX U(2)
#define IO_COMMAND_DATA_MOVE_RX  U(3)
#define IO_COMMAND_DATA_MOVE_TX  U(4)
//
#define SET_ZERO_REG 5u               /* set a 0 in to *DEBUG_REG_ARCW1 */
#define SET_ONE_REG 6u                /* set a 1 in to *DEBUG_REG_ARCW1 */
#define INCREMENT_REG 7u              /* increment *DEBUG_REG_ARCW1 */
#define SET_ZERO_NOTIFICATION_ADDR 8u /* set a 0 in to *DBGADDR_ARCW4 */
#define SET_ONE_NOTIFICATION_ADDR 9u  /* set a 1 in to *DBGADDR_ARCW4 */
#define INCREMENT_NOTIFICATION_REG 10u /* increment the arc notification register in *DBGADDR_ARCW4 */
#define SET_AND_INCREMENT 11u        /* set a 1 in to *DBGADDR_ARCW4 and increment register *DEBUG_REG_ARCW1 */
#define PROCESSOR_WAKEUP 12u         /* MP use-case WAKEUP_IOFMT */
//};    


/*=====================================================================================*/                          
/*                  STREAM_FORMAT_IO 
    The graph hold a table of uint32_t "stream_format_io" [LAST_IO_FUNCTION_PLATFORM]
*/

#define STREAM_IOFMT_SIZE_W32 2   /* one word for settings controls + 1 for instance selection and mixed-signal settings */

#define RX0_TO_GRAPH 0u
#define TX1_FROM_GRAPH 1u

#define    UNUSED_IOFMT_MSB U(31)  
#define    UNUSED_IOFMT_LSB U(25)  /* 7   */
#define NOTISOCHR_IOFMT_MSB U(24)  /*    used to size the FIFO:  FRAMESIZE_FMT0 x SAMPLING_FMT1 */
#define NOTISOCHR_IOFMT_LSB U(24)  /* 1  1:bursty asynchronous : frame size and FS give the peak data-rate */
#define  INSTANCE_IOFMT_MSB U(23)  
#define  INSTANCE_IOFMT_LSB U(20)  /* 4  selection of 16 GPIO for example */
#define IOCOMMAND_IOFMT_MSB U(19)  
#define IOCOMMAND_IOFMT_LSB U(16)  /* 4  input_output_command_id */
#define  FOLLOWER_IOFMT_MSB U(15)  
#define  FOLLOWER_IOFMT_LSB U(15)  /* 1  1=IO_IS_FOLLOWER */
#define    RX0TX1_IOFMT_MSB U(14)  /*    direction of the stream */
#define    RX0TX1_IOFMT_LSB U(14)  /* 1  0 : to the graph    1 : from the graph */
#define   REQMADE_IOFMT_MSB U(13)  
#define   REQMADE_IOFMT_LSB U(13)  /* 1  request on going, no need to ask again  */
#define FW_IO_IDX_IOFMT_MSB U(12)  /*    enum codes to address platform_io_functions[] */
#define FW_IO_IDX_IOFMT_LSB U( 8)  /* 5  platform_io [fw_io_idx] -> io_start(parameter) */
#define   IOARCID_IOFMT_MSB U( 7)  
#define   IOARCID_IOFMT_LSB U( 0)  /* 8  Arc */
/*=====================================================================================*/                          

//enum stream_scheduling_options
//{
#define STREAM_SCHD_RET_END_EACH_SWC        S8(1)      /* return to caller after each SWC calls */
#define STREAM_SCHD_RET_END_ALL_PARSED      S8(2)    /* return to caller once all SWC are parsed */
#define STREAM_SCHD_RET_END_SWC_NODATA      S8(3)    /* return to caller when all SWC are starving */
                            
#define STREAM_SCHD_NO_SCRIPT               S8(4)             /* script is not called */
#define STREAM_SCHD_SCRIPT_BEFORE_EACH_SWC  S8(5)/* script is called after each SWC called */
#define STREAM_SCHD_SCRIPT_AFTER_EACH_SWC   S8(6) /* script is called after each SWC called */
#define STREAM_SCHD_SCRIPT_END_PARSING      S8(7)    /* script is called at the end of the loop */
#define STREAM_SCHD_SCRIPT_START            S8(8)          /* script is called when starting */
#define STREAM_SCHD_SCRIPT_END              S8(9)            /* script is called before return */
//};

/*
    commands from the application, and from Stream to the SWC
*/
#define     NIN_CMD_MSB U(31)
#define     NIN_CMD_LSB U(28) /* 4 number of input arcs */
#define    NOUT_CMD_MSB U(27)       
#define    NOUT_CMD_LSB U(24) /* 4 number of output arcs */
#define     TAG_CMD_MSB U(23)       
#define     TAG_CMD_LSB U(16) /* 8 selection / debug arc index */
#define  PRESET_CMD_MSB U(15)       
#define  PRESET_CMD_LSB U( 8) /* 8 preset */
#define    INST_CMD_MSB U( 7)       
#define    INST_CMD_LSB U( 4) /* 4 instance index */
#define COMMAND_CMD_MSB U( 3)       
#define COMMAND_CMD_LSB U( 0) /* 4 command */
#define PACK_COMMAND(I,O,T,P,INST,CMD) (((I)<<28)|((O)<<24)|((T)<<16)|((P)<<8)|((INST)<<4)|(CMD))

//enum stream_command (8bits LSB)
//{
#define STREAM_RESET 1u             /* func(STREAM_RESET, *instance, * memory_results) */
#define STREAM_SET_PARAMETER 2u     /* swc instances are protected by multithread effects when changing parmeters on the fly */
#define STREAM_READ_PARAMETER 3u    
#define STREAM_RUN 4u               /* func(STREAM_RUN, instance, *in_out) */
#define STREAM_END 5u               /* func(STREAM_END, instance, 0)   called at STREAM_APP_STOP_GRAPH, swc calls free() if it used stdlib's malloc */
#define STREAM_APP_SET_PARAMETER 6u /* arm_stream (STREAM_INTERPRET_COMMANDS, byte stream, 0, 0)*/
//#define STREAM_DELETE_NODE 
//#define STREAM_INSERT_NODE
//};

/*
    commands from the SWC to Stream
*/
//enum stream_services
//{
#define STREAM_NODE_REGISTER 1u

#define STREAM_FORMAT_UPDATE_FS 2u       /* SWC information for a change of stream format, sampling, nb of channel */
#define STREAM_FORMAT_UPDATE_NCHAN 3u     /* raw data sample, mapping of channels, (web radio use-case) */
#define STREAM_FORMAT_UPDATE_RAW 4u
#define STREAM_FORMAT_UPDATE_MAP 5u

#define STREAM_AUDIO_ERROR 6u        /* PLC applied, Bad frame (no header, no synchro, bad data format), bad parameter */
    
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