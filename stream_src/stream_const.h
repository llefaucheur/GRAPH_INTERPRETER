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

#include "stream_common_const.h" 

#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */


/*------ Major-Minor version numbers ------*/
#define GRAPH_INTERPRETER_VERSION 0x0100u

/* 
  *- Graph data format :
    -------------------SHARED FLASH (RAM POSSIBLE)-----------------
    [-1] size of the graph in Words 
    Offset_0 (long_offset = MBANK_GRAPH)
    [0] 28b RAM address of part/all the graph going in RAM, which part is going in RAM
    [1] number of FORMAT, size of SCRIPTS
    [2] size of LINKEDLIST, number of STREAM_INSTANCES
    [3] number of ARCS, number of DEBUG registers
    [4] list of processors (procID for the scheduler in platform_manifest) processing the graph
        The linkedList gives the architecture/procID only allowed to execute a SWC
        This table is made to exclude a group of processor to execute any NODE even if their scheduler is launched.
    [5,6] UQ8 portion of memory consumed on each long_offset[MAX_NB_MEMORY_OFFSET] 
    -------------------
    STREAM_IO_CONTROL (4 words per IO)  size = 4*[NB_IOS_GR1], "ONGOING" flag is in RAM
        depends on the domain of the IO (DOMAIN_FMT1)

  *- SCRIPTS are adressed with a table_int32[128] : offset, ARC, binary format
        ARC descriptor: size regs/stack, parameters/UC, collision Byte, max cycles 
        The first are indexed with the NODE header 7b index (SCRIPT_LW0) 
        Script index #0 means "disabled"  Indexes 1..up to 63 are used for shared subroutines.
     
  *- LINKED-LIST of SWC
       minimum 5 words/SWC
       Word0   : header processor/architecture, nb arcs, SWCID, arc
       Word1+n : arcs * 2  + debug page
       Word2+2n: ADDR + SIZE of memory segments
       Word3   : optional 4 words User key + Platform Key
       Word4+n : 1+data Preset, New param!, Skip length, 
          byte stream: nbparams (ALLPARAM), {tag, nbWord32, params}

       list Ends with the NODE ID 0x03FF 
    
    -----------------SHARED RAM-------------------------------  OFFSET 0

  *-PIO "ONGOING" (1 Byte per IO)  size = [NB_IOS_GR1] (in instance "uint8_t *ongoing")
    
    at PIO + [NB_IOS_GR1] words : collision management (Dekker's algorithm) [COLLISION_IDX_GR2] words

    at PIO + [ARC_DEBUG_IDX_GR2] words : table of debug registers (2 words each) DEBUG REGISTERS from ARC content
        analysis (DEBUG_REG_ARCW4)
        32 memory banks of 16bytes + 64bytes in normal 

  *-FORMAT used by the arcs (4 words each stream_format)  
        Word0: Frame size, interleaving scheme, arithmetics raw data type
        Word1: time-stamp, domain, nchan, physical unit (pixel format, IMU interleaving..)
        Word2: depends on IO Domain
        Word3: depends on IO Domain

    can be used for tunable formats
        This is used the NODE generates variable frame formats (JPG decoder, MP3 decoder..)

  *-ARC descriptors (5 words each)

    ----------------- MEMORY BANKS -------------------------------
    BUFFERS memory banks (internal/external/LLRAM) used for FIFO buffers 
       and used for initializations, list of PACKSWCMEM results with fields
          SIZE, ALIGNMT, SPEED,

    INSTANCE Pointers (28b encoded format + bits used for synchonization at boot time and service activations)
        (indexed by INST_SSRV_CMD of NODE_COMMANDS)
    

    WORKING areas of INSTANCES

*/


/* max number of instances simultaneously reading the graph
   used to synchronize the RESET sequence in platform_al() 
   smaller than 1<< NBINSTAN_SCTRL */
#define MAX_NB_STREAM_INSTANCES 4u 


/* the first word of the graph holds the number of words32 */
#define GRAPH_SIZE_SKIP_WORD0 1u

/* -------- GRAPH[0] 28b RAM address, HW-shared MEM & RAM copy config---- 
                                   2 options :
        IO CONTROL                 RAM  Flash
        SCRIPTS                    RAM  Flash
        LINKED-LIST                RAM  Flash  RAM allows NODE to be desactivated
        ONGOING  1 byte            RAM  RAM
        FORMAT 4 words             RAM  RAM
        ARC descriptors 4 words    RAM  RAM
        Debug registers, Buffers   RAM  RAM
*/

#define VID0    0u
#define GRAPH_START_VID0 0u

#define COPY_CONF_GR0_COPY_ALL_IN_RAM   0u
#define COPY_CONF_GR0_FROM_PIO          1u
#define COPY_CONF_GR0_ALREADY_IN_RAM    2u

#define PACKSHARERAMSPLIT(share,RAMsplit) (((share)<<3u) + (RAMsplit))
#define GR0_INDEX   1u                // graph size + GR(X)
#define   unused_______GR0_MSB U(31) 
#define   unused_______GR0_LSB U(30) // 2
#define       RAMSPLIT_GR0_MSB U(29) //   
#define       RAMSPLIT_GR0_LSB U(28) // 2 COPY_CONF_GR0_COPY_ALL_IN_RAM / _FROM_PIO / _ALREADY_IN_RAM
//#define    GRAPH_RAM_OFFSET_PTR(L,G,X) platform_pack2linaddr_ptr((L),(G)[0],X)

/* -------- GRAPH[1] number of FORMAT, IOs, size of SCRIPTS ---- */
#define GR1_INDEX   2u
#define   SCRIPTSSZW32_GR1_MSB U(31) 
#define   SCRIPTSSZW32_GR1_LSB U(12) /* 20 scripts size */
#define         NB_IOS_GR1_MSB U(11) 
#define         NB_IOS_GR1_LSB U( 5) /*  7 Nb of I/O :  up to 128 IO streams (max 8bits) */
#define      NBFORMATS_GR1_MSB U( 4) 
#define      NBFORMATS_GR1_LSB U( 0) /*  5 formats */
/*
 * Maximum number of IOs used by the graph 
 * This number is lower or equal to the maximum of possible IO connexions of the platform (max {FWIOIDX_IOFMT0})
 */
#define MAX_GRAPH_NB_IO_STREAM  (1u << (NB_IOS_GR1_MSB - NB_IOS_GR1_LSB + 1u))

/* -------- GRAPH[2] size of LINKEDLIST  ---- */
#define GR2_INDEX   3u
#define   ARC_DEBUG_IDX_GR2_MSB U(31) 
#define   ARC_DEBUG_IDX_GR2_LSB U(22) /* 10 index of PIO to address the arc debug data */
#define LINKEDLISTSZW32_GR2_MSB U(21) 
#define LINKEDLISTSZW32_GR2_LSB U( 0) /* 22 size of the linkedList with the parameters */

/* -------- GRAPH[3] number of ARCS, scheduler control ----*/
#define GR3_INDEX   4u
#define   unused________GR3_MSB U(31) 
#define   unused________GR3_LSB U(20) /* 12  */
#define    SCRIPT_SCTRL_GR3_MSB U(19) /*    SCRIPT_SCTRL_GR3_LSB+SCRIPT_SCTRL_MSB-SCRIPT_SCTRL_LSB+1) */
#define    SCRIPT_SCTRL_GR3_LSB U(14) /* 6  debug script options  */
#define    RETURN_SCTRL_GR3_MSB U(13)
#define    RETURN_SCTRL_GR3_LSB U(11) /* 3  return options (each SWC, each parse, once starving */
#define         NB_ARCS_GR3_MSB U(10) 
#define         NB_ARCS_GR3_LSB U( 0) /* 11 up to 2K ARCs, see ARC0_LW1 */

/* -------- GRAPH[4] bit-field of the processors activated to process this graph ----*/
#define GR4_INDEX   5u
#define             un__GR4_MSB U(31) 
#define             un__GR4_LSB U( 8) /* 24  */
#define    PROC_ALLOWED_GR4_MSB U( 7) 
#define    PROC_ALLOWED_GR4_LSB U( 0) /* 8  up to eight processors selection  (PROCID_PARCH) */
#define PROCID_ALLOWED(proc_id) ((1u<<(proc_id)) & RD(S->graph[4u]))


/* -------- GRAPH[5,6] UQ8(-1) (0xFF = 100%, 0x3F = 25%) portion of memory consumed on each 
    long_offset[MAX_NB_MEMORY_OFFSET] to let the application taking 
    a piece of the preallocated RAM area 
*/
#define BYTE_3_MSB 31u
#define BYTE_3_LSB 24u
#define BYTE_2_MSB 23u
#define BYTE_2_LSB 16u
#define BYTE_1_MSB 15u
#define BYTE_1_LSB  8u
#define BYTE_0_MSB  7u
#define BYTE_0_LSB  0u

#define GRAPH_HEADER_NBWORDS 7    /* GRAPH[0 .. 6] */

 
//enum stream_scheduling_options
//{
#define STREAM_SCHD_RET_NO_ACTION            0u  /* the decision is made by the graph */
#define STREAM_SCHD_RET_END_EACH_NODE        1u  /* return to caller after each NODE calls */
#define STREAM_SCHD_RET_END_ALL_PARSED       2u  /* return to caller once all NODE are parsed */
#define STREAM_SCHD_RET_END_NODE_NODATA      3u  /* return to caller when all NODE are starving */
                                            
#define STREAM_SCHD_NO_SCRIPT                0u
#define STREAM_SCHD_SCRIPT_BEFORE_EACH_NODE  1u  /* script is called before each NODE called */
#define STREAM_SCHD_SCRIPT_AFTER_EACH_NODE   2u  /* script is called after each NODE called */
#define STREAM_SCHD_SCRIPT_END_PARSING       4u  /* script is called at the end of the loop */
#define STREAM_SCHD_SCRIPT_START             8u  /* script is called when starting */
#define STREAM_SCHD_SCRIPT_END              16u /* script is called before return */
#define STREAM_SCHD_SCRIPT_UNUSED           32u /* 6bits are reserved in SCRIPT_SCTRL */
//};

/* number of NODE calls in sequence */
#define MAX_NODE_REPEAT 4u


#define STREAM_MAIN_INSTANCE 1u

/* ----------- instance -> scheduler_control  ------------- */
#define     UNUSED_SCTRL_MSB U(31)   
#define     UNUSED_SCTRL_LSB U(19)  /* 13 ____*/ 
#define   INSTANCE_SCTRL_MSB U(18)
#define   INSTANCE_SCTRL_LSB U(18)  /* 1 flag : 0=instances executed on this processor are disabled  */
#define   MAININST_SCTRL_MSB U(17)   
#define   MAININST_SCTRL_LSB U(17)  /* 1 main instance to set the graph at boot time */
#define   NODEEXEC_SCTRL_MSB U(16)   
#define   NODEEXEC_SCTRL_LSB U(16)  /* 1 node execution flag start=1, done=0 */
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
#define     RETURN_SCTRL_LSB U( 0)  /* 3 return options (each SWC, each parse, once starving, copy of RETURN_SCTRL_GR3 */
#define   PACK_STREAM_PARAM(M,N,B,S,R) (  \
            ((M)<<MAININST_SCTRL_LSB) |   \
            ((N)<<NBINSTAN_SCTRL_LSB) |   \
            ((B)<<    BOOT_SCTRL_LSB) |   \
            ((S)<<  SCRIPT_SCTRL_LSB) |   \
            ((R)<<  RETURN_SCTRL_LSB) )

/* ----------- instance -> whoami_ports  ------------- */
/* identification "whoami", next NODE to run*/
#define    INST_ID_PARCH_MSB U(31)
#define     WHOAMI_PARCH_MSB U(31)
#define   PRIORITY_PARCH_MSB U(31)  /* avoid locking an arc by the same processor, but different RTOS instances*/
#define   PRIORITY_PARCH_LSB U(30)  /* 2 [0..3] up to 4 instances per processors, 0=main instance at boot */
#define     PROCID_PARCH_MSB U(29)  /*   indexes from Manifest(tools) and PLATFORM_PROC_HW */
#define     PROCID_PARCH_LSB U(27)  /* 3 processor index [1..7] for this architecture 0="commander processor" */  
#define     ARCHID_PARCH_MSB U(26)
#define     ARCHID_PARCH_LSB U(24)  /* 3 [1..7] processor architectures 1="commander processor architecture" */
#define     WHOAMI_PARCH_LSB U(24)  /*   whoami used to lock a NODE to specific processor or architecture */
#define    INST_ID_PARCH_LSB U(24)  /*   8 bits identification for locks : !! corresponds with ARCHID_LW0_LSB */
#define NODE_W32OFF_PARCH_MSB U(23)  
#define NODE_W32OFF_PARCH_LSB U( 0) /* 24   offset in words to the NEXT NODE to be executed */  

/* maximum number of processors = nb_proc x nb_arch */
#define MAX_GRAPH_NB_PROCESSORS ((1<<(PROCID_PARCH_MSB-PROCID_PARCH_LSB+1))*(1<<(ARCHID_PARCH_MSB-ARCHID_PARCH_LSB+1)))

/*================================= STREAM_IO_CONTROL (FLASH) ===================
      
      The graph hold a table of uint32_t "stream_io_control" 

        arcID, direction, servant/commander, 
        set pointer/copy, buffer allocation, Domain, index to the AL

        example with a platform using maximum 10 IOs and a graph using "SENSOR_0" and "_DATA_OUT_0"
            #define IO_PLATFORM_DATA_IN_0        0 
            #define IO_PLATFORM_DATA_IN_1        1 
            #define IO_PLATFORM_ANALOG_SENSOR_0  2   X
            #define IO_PLATFORM_MOTION_IN_0      3 
            #define IO_PLATFORM_AUDIO_IN_0       4 
            #define IO_PLATFORM_2D_IN_0          5 
            #define IO_PLATFORM_LINE_OUT_0       6 
            #define IO_PLATFORM_GPIO_OUT_0       7 
            #define IO_PLATFORM_GPIO_OUT_1       8 
            #define IO_PLATFORM_DATA_OUT_0       9   X
        The binary grap will have stream_io_control with 2 indexes "SENSOR_0" and "_DATA_OUT_0"

        The table arm_stream_io_ack[10] makes the translation 
            arm_stream_io_ack [FWIOIDX] = graph_io_idx index in the graph for pio_control[]
        arm_stream_io_ack (arm_stream_io_ack[IO_PLATFORM_ANALOG_SENSOR_0], (uint8_t *)data, size);
*/
#define STREAM_IOFMT_SIZE_W32 4u   /* four word for IO controls : one for the scheduler, three for IO settings */

#define MAX_IO_FUNCTION_PLATFORM 127u /* table of functions : platform_io[MAX_IO_FUNCTION_PLATFORM] */
                              
#define IO_COMMAND_SET_BUFFER 0u  /* arc buffer used ping-pong, big buffer */
#define IO_COMMAND_DATA_COPY  1u  /* copy : the IO has its own buffer */
                              
#define RX0_TO_GRAPH          0u
#define TX1_FROM_GRAPH        1u
                              
#define IO_IS_COMMANDER0      0u
#define IO_IS_SERVANT1        1u

#define IOFMT0 0u                  /* first word used by the scheduler */

#define    FWIOIDX_IOFMT0_MSB 31u  
#define    FWIOIDX_IOFMT0_LSB 14u  /* 18 index to byte array: arm_stream_io_ack[] */
#define  SET0COPY1_IOFMT0_MSB 13u  
#define  SET0COPY1_IOFMT0_LSB 13u  /* 1  command_id IO_COMMAND_SET_BUFFER / IO_COMMAND_DATA_COPY */
#define   SERVANT1_IOFMT0_MSB 12u  
#define   SERVANT1_IOFMT0_LSB 12u  /* 1  1=IO_IS_SERVANT1 */
#define     RX0TX1_IOFMT0_MSB 11u  /*    direction of the stream */
#define     RX0TX1_IOFMT0_LSB 11u  /* 1  0 : to the graph    1 : from the graph */
#define    IOARCID_IOFMT0_MSB 10u 
#define    IOARCID_IOFMT0_LSB  0u  /* 11  ARC*/

#define IOFMT1 1u                  /* domain-specific controls */
#define IOFMT2 2u                  /* domain-specific controls */
#define IOFMT3 3u                  /* domain-specific controls */

/*================================= ONGOING  (RAM) ==============================
      
      The graph hold a table of uint8_t in RAM for the "on-going" flag    
*/

#define     UNUSED_IO_MSB 7u  
#define     UNUSED_IO_LSB 1u  /* 7 */
#define    ONGOING_IO_MSB 0u  
#define    ONGOING_IO_LSB 0u  /* 1 set in scheduler, reset in IO, iomask manages processor access */

/*================================= SCRIPTS ======================================= */

#define     ARC_SCROFF0_MSB U(31) /* 11 associated arc descriptor */
#define     ARC_SCROFF0_LSB U(21) /*                   */
#define  FORMAT_SCROFF0_MSB U(20) /* 3  byte codes format = 0, 7 binary native architecture ARCHID_LW0 */
#define  FORMAT_SCROFF0_LSB U(19) /*       ARMv6-M */
#define  SHARED_SCROFF0_MSB U(18) /* 1  shareable memory for the script with other scripts in mono processor platforms */
#define  SHARED_SCROFF0_LSB U(18) /*                                    */
#define  OFFSET_SCROFF0_MSB U(17) /* 17 offset in the W32 script table */
#define  OFFSET_SCROFF0_LSB U( 0) /*    placed at    */

         
/* 
    arc descriptors used to address the working area : registers and stack
*/
#define      SCRIPT_PTR_SCRARCW0  U( 0) /* Base address + NREGS + new UC */
#define          SCRIPT_SCRARCW1  U( 1) /* LENGTH use case UC0 */
#define          RDFLOW_SCRARCW2  U( 2) /* READ use-case UC1 + ARCEXTEND_ARCW2  */
#define        WRIOCOLL_SCRARCW3  U( 3) /* WRITE + STACK LENGTH + Flag logMaxCycles8b */
#define          DBGFMT_SCRARCW4  U( 4) /*  */

          
#define    unused____SCRARCW0_MSB U(31)    
#define    unused____SCRARCW0_LSB U(29) /*  3    base address of the working memory */
#define NEW_USE_CASE_SCRARCW0_MSB U(28) /*  1  new use-case arrived */ 
#define NEW_USE_CASE_SCRARCW0_LSB U(28) /*     */ 
#define    BASEIDXOFFSCRARCW0_MSB U(27)    
#define    BASEIDXOFFSCRARCW0_LSB U( 0) /* 28  base address of the script memory (regs + state + stack)  */

#define     CODESIZE_SCRARCW1_MSB U(31) 
#define     CODESIZE_SCRARCW1_LSB U(22) /* 10 */
#define    BUFF_SIZE_SCRARCW1_MSB U(21) /*    */
#define    BUFF_SIZE_SCRARCW1_LSB U( 0) /* 22 BYTE-acurate up to 4MBytes (up to 128GB with ARCEXTEND_ARCW2 */

//#define          READ_ARCW2_MSB U(21) /*    data read index  Byte-acurate up to 4MBytes starting from base address */
//#define          READ_ARCW2_LSB U( 0) /* 22 this is incremented by "frame_size" FRAMESIZE_FMT0  */

#define    COLLISION_SCRARCW3_MSB U(31) /*  8  */
#define    COLLISION_SCRARCW3_LSB U(24) /*     */
//#define         WRITE_ARCW3_MSB U(21) /*    write pointer is incremented by FRAMESIZE_FMT0 */
//#define         WRITE_ARCW3_LSB U( 0) /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */

#define  RAMTOTALW32_SCRARCW4_MSB U(23) /*  13  Total in words = 2*(regs + stack) + heap size = 8k W32*/
#define  RAMTOTALW32_SCRARCW4_LSB U(11) /*     */
#define        NREGS_SCRARCW4_MSB U(10) /*  4   number of registers used in this script */ 
#define        NREGS_SCRARCW4_LSB U( 7) /*     */
#define       NSTACK_SCRARCW4_MSB U( 6) /*  7   max size of the FIFO/stack in register size:  128 registers */
#define       NSTACK_SCRARCW4_LSB U( 0) /*     */

/* =================
    script (SCRIPT_LW0) used to copy input test-patterns, set-parameters from the global use-case or from information 
    of an application call-back

    "analog user-interface" called (knobs / needles) give controls and visibility on NODE parameters and
    are accessed with scripts 
*/
#define SCRIPT_PRERUN 1u        /* executed before calling the node : the Z flag is set */
#define SCRIPT_POSTRUN 2u       /* executed after */

/* ======================================   NODE   ============================================ */ 
#define arm_stream_script_index 1u     /* arm_stream_script() is the first one in the list node_entry_points[] */

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
#define   SCRIPT_LW0_MSB U(23) /*   script called Before/After (debug, verbose trace control) */
#define   SCRIPT_LW0_LSB U(17) /* 7 script ID to call before and after calling NODE */
#define   NBARCW_LW0_MSB U(16) 
#define   NBARCW_LW0_LSB U(13) /* 4  total nb arcs, streaming and metadata/control {0 .. MAX_NB_STREAM_PER_NODE} */
#define unused___LW0_MSB U(12) 
#define unused___LW0_LSB U(10) /* 3  */
#define NODE_IDX_LW0_MSB U( 9) 
#define NODE_IDX_LW0_LSB U( 0) /* 10 0=nothing, node index of node_entry_points[] */


        /* word 1+n -arcs

            starting with the one used for locking, the streaming arcs, then the metadata arcs 
            arc(tx) used for locking is ARC0_LW1

            input parameter arcs are declared as TX-arcs, 
            preset "empty" and let untouched by the node
        */


#define ARC_RX0TX1_TEST  0x0800u /* MSB gives the direction of the arc */
#define ARC_RX0TX1_CLEAR 0x07FFu 

#define un__1_LW1_MSB 31u 
#define un__1_LW1_LSB 30u /*  2   */
#define DBGB1_LW1_MSB 29u 
#define DBGB1_LW1_LSB 28u /*  2  debug register bank for ARC1 */
#define  ARC1_LW1_MSB 27u
#define ARC1D_LW1_LSB 27u /*  1  ARC1 direction */
#define ARC1D_LW1_MSB 27u
#define  ARC1_LW1_LSB 16u /* 11  ARC1  11 usefull bits + 1 MSB to tell rx0tx1 */
                        
#define un__0_LW1_MSB 15u 
#define un__0_LW1_LSB 15u /*  1   */
#define   KEY_LW1_MSB 14u 
#define   KEY_LW1_LSB 14u /*  1  two 64b KEYs are inserted after the memory pointers (word 2+2n)  */
#define DBGB0_LW1_MSB 13u /*     debug register bank for ARC0 : debug-arc index of the debug data */
#define DBGB0_LW1_LSB 12u /*  2    the banks are extending arc's debug traces for subgraphs */
#define  ARC0_LW1_MSB 11u
#define ARC0D_LW1_LSB 11u /*  1  ARC0 direction */
#define ARC0D_LW1_MSB 11u
#define  ARC0_LW1_LSB  0u /* 11  ARC0, (10 + 1 rx0tx1) up to 2K ARCs */

#if IOARCID_IOFMT0_MSB != (ARC0D_LW1_MSB-1)
#error "IOFORMAT ARC SIZE"
#endif
        /* word 2+2n - FIRST WORD : memory banks address + size */

#define NBW32_MEMREQ_LW2  2u    /* there are two words per memory segments, to help programing the memory protection unit (MPU) */
#define ADDR_LW2 0u             /*      one for the address */ 
#define SIZE_LW2 1u             /*      one for the size of the segment */ 

            /* word 2 first word = base address of the memory segment + control on the first segment */
#define   NALLOCM1_LW2_MSB U(31) /*      number of memory segments (pointer + size) to give at RESET [0..MAX_NB_MEM_REQ_PER_NODE-1] */  
#define   NALLOCM1_LW2_LSB U(29) /*  3   2 words each  */
#define     unused_LW2_MSB U(28) /*     */
#define     unused_LW2_LSB U(28) /*  1  */
#define MEMSEGMENT_LW2_MSB SIZE_EXT_OFF_FMT_MSB /* 28 = offsets(4) + EXT(3) + sign(1) + size(20) */
#define MEMSEGMENT_LW2_LSB SIZE_EXT_OFF_FMT_LSB

//#define     RELOC_LW2_MSB *      relocatable memory segment to update with STREAM_UPDATE_RELOCATABLE */
//#define     RELOC_LW2_LSB *  1   @@@ TODO */
/* 
    Memory Protection Unit (MPU)  has 8 memory segments : 4 memory segments per NODE (1 instance + 
        3 segments) + 1 code + 1 stack + 1 IRQ + 1 Stream/services/script
*/
#define MAX_NB_MEM_REQ_PER_NODE U(6)    /* TO_SWAP_LW2S limits to 6 MAX and 4 MAX if memory protection is used */

            /* word 2 SECOND WORD = size of the memory segment  + control on the first segment 
                            if the segment is swapped, the 12-LSB bits give the ARC ID of the buffer 
                            and the memory size is given by the FIFO descriptor (BUFF_SIZE_ARCW1) */
#define LW2S_NOSWAP 0u
#define LW2S_SWAP 1u

#define    unused_LW2S_MSB U(31) /*      */
#define    unused_LW2S_LSB U(25) /*  7   */
#define CLEARSWAP_LW2S_MSB U(24) /*      used on the instance : "at least one memory bank is swapped or cleared */
#define CLEARSWAP_LW2S_LSB U(24) /*  1   */
#define     CLEAR_LW2S_MSB U(23) /*      clear the memory before calling the node */
#define     CLEAR_LW2S_LSB U(23) /*  1   */
#define      SWAP_LW2S_MSB U(22) /*      0= normal memory segment, 1 = swap before execute */
#define      SWAP_LW2S_LSB U(22) /*  1    */
#define  EXT_SIZE_LW2S_MSB U(21) /*      extend SizeMax = 1M / 16M / 256M / 4G with EXT=0/1/2/3 */
#define  EXT_SIZE_LW2S_LSB U(20) /*  2    */
#define BUFF_SIZE_LW2S_MSB U(19) /* ###  overlaid with SWAPBUFID_LW2S in case of COPY / SWAP */
#define BUFF_SIZE_LW2S_LSB U( 0) /* 20   Wrd32-acurate up to 1MBwords x (1 << (4 * EXT_SIZE)) */

#define SWAPBUFID_LW2S_MSB ARC0_LW1_MSB /*     ARC => swap source address in slow memory + swap length */
#define SWAPBUFID_LW2S_LSB ARC0_LW1_LSB /* 12  ARC0, (11 + 1) up to 2K FIFO */

  /* Word3   : optional 4 words User key + Platform Key
                if KEY_LW1 == 1

     0x11223344 User key word 0
     0x11223344 User key word 1

  */
#define NBWORDS_KEY_USER_PLATFORM 2

  /* word 4+n - parameters 
    NODE header can be in RAM (to patch the parameter area, cancel the component..)

  BOOTPARAMS (if W32LENGTH_LW4>0 )

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

  NODE can declare an extra input arc to receive a huge set of parameters (when >256kB), for example a 
  NN model. This is a fake arc and the read pointer is never incremented during the execution of the node.
*/
//#define MAX_TMP_PARAMETERS 30   /* temporary buffer (words32) of parameters to send to the Node */


#define PARAM_TAG_LW4_MSB U(31) 
#define PARAM_TAG_LW4_LSB U(28) /* 4  for PARAM_TAG_CMD (15='all parameters')  */
#define    PRESET_LW4_MSB U(27)
#define    PRESET_LW4_LSB U(24) /* 4  preset   16 precomputed configurations */
#define   TRACEID_LW4_MSB U(23)       
#define   TRACEID_LW4_LSB U(16) /* 8  TraceID used to route the trace to the corresponding peripheral/display-line */
#define W32LENGTH_LW4_MSB U(15) /*    if >256kB are needed then use an arc to a buffer */
#define W32LENGTH_LW4_LSB U( 0) /* 16 skip this : number of uint32 to skip the boot parameters */


/* ================================= */

/* last word has NODE index 0b11111..111 */
#define GRAPH_LAST_WORD_MSB NODE_IDX_LW0_MSB
#define GRAPH_LAST_WORD_LSB NODE_IDX_LW0_LSB
#define GRAPH_LAST_WORD U((U(1)<<U(NODE_IDX_LW0_MSB- NODE_IDX_LW0_LSB+1U))-1U)

/*
 *  NODE manifest :
 */ 

//enum stream_node_status {
#define NODE_BUFFERS_PROCESSED 0u
#define NODE_NEED_RUN_AGAIN 1u        /* NODE completion type */

//enum mem_mapping_type {
#define MEM_TYPE_STATIC          0u   /* (LSB) memory content is preserved (default ) */
#define MEM_TYPE_WORKING         1u   /* scratch memory content is not preserved between two calls */
#define MEM_TYPE_PERIODIC_BACKUP 2u   /* persistent static parameters to reload for warm boot after a crash */
#define MEM_TYPE_PSEUDO_WORKING  3u   /* static only during the uncompleted execution state of the SWC, see NODE_RUN 

                periodic backup use-case : long-term estimators. This memory area is cleared at cold NODE_RESET and 
                refreshed for warm NODE_RESET. The NODE should not reset it (there is 
                no "warm-boot reset" entry point. The period of backup depends on platform capabilities 
                When MBANK_BACKUP is a retention-RAM there is nothing to do, when it is standard RAM area then on periodic
                basis the AL will be call to transfer data to Flash 
            */
              

//enum mem_speed_type                         /* memory requirements associated to enum memory_banks */
#define MEM_SPEED_REQ_ANY           0u   /* best effort */
#define MEM_SPEED_REQ_FAST          1u   /* will be internal SRAM when possible */
#define MEM_SPEED_REQ_CRITICAL_FAST 2u   /* will be TCM when possible
           When a NODE is declaring this segment as relocatable ("RELOC_MEMREQ") it will use 
           physical address different from one TCM to an other depending on the processor running the SWC.
           The scheduler shares the TCM address dynamically before calling the SWC. 
           This TCM address is provided as a pointer after the XDM in/out pointer
           The TCM address is placed at the end (CRITICAL_FAST_SEGMENT_IDX) of long_offset[] 
           */
                                

//enum buffer_alignment_type            
//#define MEM_REQ_NOALIGNMENT_REQ    0    /* address binary mask : */
//#define MEM_REQ_2BYTES_ALIGNMENT   1   /*   mask = ~((1 << (7 & mem_req_2bytes_alignment) -1) */
#define MEM_REQ_4BYTES_ALIGNMENT   2u    
#define MEM_REQ_8BYTES_ALIGNMENT   3u
#define MEM_REQ_16BYTES_ALIGNMENT  4u
#define MEM_REQ_32BYTES_ALIGNMENT  5u
#define MEM_REQ_64BYTES_ALIGNMENT  6u
#define MEM_REQ_128BYTES_ALIGNMENT 7u


#define NODE_CONTROLS U(4u)
#define NODE_CONTROLS_NAME U(8u)

/*================================================== ARC ==================================================================*/
/*
  arc descriptions : 
                             
      - arc_descriptor_ring : R/W are used to manage the alignment of data to the base address and notify the SWC
                              debug pattern, statistics on data, time-stamps of access, 
                              realignment of data to base-address when READ > (SIZE) - consumer frame-size
                              deinterleave multichannel have the same read/write offset but the base address starts 
                              from the end of the previous channel boundary of the graph
*/

// enum debug_arc_computation_1D { COMPUTCMD_ARCW4 /* 5bits */

#define COMPUTCMD_ARCW4_NO_ACTION                0u      
#define COMPUTCMD_ARCW4_INCREMENT_REG            1u  /* increment DEBUG_REG_ARCW4 with the number of RAW samples */      
#define COMPUTCMD_ARCW4_SET_ZERO_ADDR            2u  /* set a 0 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to clear */      
#define COMPUTCMD_ARCW4_SET_ONE_ADDR             3u  /* set a 1 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to set */       
#define COMPUTCMD_ARCW4_INCREMENT_REG_ADDR       4u  /* increment *DEBUG_REG_ARCW4 */ 
#define COMPUTCMD_ARCW4__5                       5u 
#define COMPUTCMD_ARCW4_APP_CALLBACK1            6u  /* call-back in the application side, data rate estimate in DEBUG_REG_ARCW4 */      
#define COMPUTCMD_ARCW4_APP_CALLBACK2            7u  /* second call-back : wake-up processor from DEBUG_REG_ARCW4=[ProcID, command]  */      
#define COMPUTCMD_ARCW4_TIME_STAMP_READ          8u  /* log a time-stamp of the last read access to FIFO */
#define COMPUTCMD_ARCW4_TIME_STAMP_WRITE         9u  /* log a time-stamp of the last write access to FIFO */
#define COMPUTCMD_ARCW4_PEAK_DATA               10u  /* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW4 */          
#define COMPUTCMD_ARCW4_MEAN_DATA               11u 
#define COMPUTCMD_ARCW4_MIN_DATA                12u 
#define COMPUTCMD_ARCW4_ABSMEAN_DATA            13u 
//#define COMPUTCMD_ARCW4_DATA_TO_OTHER_ARC     14u  /* when data is changing the new data is push to another arc DEBUG_REG_ARCW4=[ArcID] */
#define COMPUTCMD_ARCW4_LOOPBACK                15u  /* automatic rewind read/write */           

/* 
    scripts associated to arcs : 
*/
#define COMPUTCMD_ARCW4_SCRIPT_INDEX_OFFSET     20u  /* index of the scripts = (CMD - xx_OFFSET) */           
#define COMPUTCMD_ARCW4_LAST ((1<<(COMPUTCMD_ARCW4_MSB-COMPUTCMD_ARCW4_LSB+1))-1) /* 7bits CMD bit-field */

#define ARC_DBG_REGISTER_SIZE_W32 2u                 /* debug registers on 64 bits */


/* increment DEBUG_REG_ARCW4 with the number of RAW samples */
/* set a 0 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to clear */
/* set a 1 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to set */
/* increment *DEBUG_REG_ARCW4 */

/* call-back in the application side, data rate estimate in DEBUG_REG_ARCW4 */
/* second call-back : wake-up processor from DEBUG_REG_ARCW4=[ProcID, command]  */

/* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW4 */

/* when data is changing the new data is push to another arc DEBUG_REG_ARCW4=[ArcID] */
/* automatic rewind read/write */

/* debug registers on 64 bits */

/* Arcs used for synchronized streams : for example I/V capture must be synchronized     */
/*  the arc CMD is used to instert a time-stamp before before raw data is pushed in the FIFO    */
/*  the stream consumer check the time-stamps and rejects the data outside of a predefined time-window */

/*
*   Flow error management with FLOW_RD_ARCW2 / FLOW_WR_ARCW2 = let an arc stay with 25% .. 75% of data
*       process done on "router" node when using HQOS arc and IO master interfaces
* 
*  The arc is initialized with 50% of null data
*  The processing is frame-based, there are minimum 3 frames in the buffer

*   When a IO-master writes in an arc with FLOW_WR_ARCW2=1 and the arc is full at +75%, the new data 
*       is extrapolated and the arc stays at 75% full 
*      Buff  xxxxxxxx|xxxx|xxxx|xxxx|bbbb|aaaa|  buffer full after NewData was push by the IO-master
*              R_ptr                      W_ptr
*            The previous frame (bbb) is filled (bbb x win_rampDown) + (newData_aaa x win_rampUp)
*            W_ptr steps back 
*            xxxxxxxx|xxxx|xxxx|xxxx|bbaa|----|  buffer full
*              R_ptr                      W_ptr
* 
*   When a IO-master read from an arc with FLOW_RD_ARCW2=1 and the arc is empty at -25%, the new data 
*       is extrapolated and the arc stays at 25% empty
* 
*      Buff  |bbbb| is read by the IO-master
*            |bbbb|aaaa|----|  buffer hold only ONE frame |aaaa| after the previous read
*                  R_ptr W_ptr
*            The previous frame (bbb) is filled (aaa x win_rampDown) + (bbb x win_rampUp)
*               and R_ptr steps back
*            |aabb|aaaa|----|  bbbb
*             R_ptr      W_ptr
*/

#define arc_read_address                1u
#define arc_write_address               2u
#define arc_data_amount                 3u
#define arc_free_area                   4u
#define arc_buffer_size                 5u
#define data_move_to_arc                6u
#define data_moved_from_arc             7u
#define data_swapped_with_arc           8u
#define arc_data_realignment_to_base    9u

#define SIZEOF_ARCDESC_W32 8u


/*          SIZE_EXT_FMT0 / COLLISION_ARC / NODE memory bank

    -8 bits-|--------24 bits -------
  
    1_987654321_987654321_987654321_
    III_____________________________  NALLOCM1 node memory banks
    cccccccc________________________  collision byte in the WRITE word32
    ____OOOO________________________  long offset of the buffer base address
    ________EXT_____________________  extension 3bits shifter
    ___________sbbbbbbbbbbbbbbbbbbbb  signed length in 20 bits BYTES x (1 << (EXT x 2))
    ___________s44443333222211110000  5 hex signed digits
   
   max = 0x000FFFFF << (2x7) = +/- 1M x 16k = +/- 16G
*/

#define         BUF_PTR_ARCW0    U( 0)  
#define   unused__ARCW0_MSB U(31) /*  4   */
#define   unused__ARCW0_LSB U(28) /*      */    
#define BASEIDXOFFARCW0_MSB U(27) /*      */
#define   DATAOFF_ARCW0_MSB U(27) /*  4   address = offset[DATAOFF] + 4x BASEIDX[Bytes] */
#define   DATAOFF_ARCW0_LSB U(24) /*      16 long offsets  */
#define  BUFFBASE_ARCW0_MSB SIZE_EXT_FMT0_MSB  /* 24 bits   */
#define  BUFFBASE_ARCW0_LSB SIZE_EXT_FMT0_LSB
#define BASEIDXOFFARCW0_LSB U( 0) /*                        */
                 
                 
#define       BUFSIZDBG_ARCW1    U( 1)
#define   unused__ARCW1_MSB U(31) 
#define   unused__ARCW1_LSB U(24) /* 8 ____*/
#define BUFF_SIZE_ARCW1_MSB SIZE_EXT_FMT0_MSB /*     */
#define BUFF_SIZE_ARCW1_LSB SIZE_EXT_FMT0_LSB /* 24  */


#define          RDFLOW_ARCW2    U( 2)  
#define   unused__ARCW2_MSB U(31)       
#define   unused__ARCW2_LSB U(29) /*  3  ____*/
#define   FLOW_WR_ARCW2_MSB U(28)   
#define   FLOW_WR_ARCW2_LSB U(28) /*  1  overflow control on writes 0=nothing  1= best effort from domain */
#define   FLOW_RD_ARCW2_MSB U(27)   
#define   FLOW_RD_ARCW2_LSB U(27) /*  1  underflow control on reads 0=nothing  1= best effort from domain */
#define  HIGH_QOS_ARCW2_MSB U(26) /*     arc with high QoS */
#define  HIGH_QOS_ARCW2_LSB U(26) /*  1  data in the arc is processed whatever the content of the other arcs */
#define ALIGNBLCK_ARCW2_MSB U(25) /*     producer blocked sets "I need data realignement from the consumer because the buffer is full" */
#define ALIGNBLCK_ARCW2_LSB U(25) /*  1   a full buffer can have the Write index = BUFF_SIZE, there is no space lost */
#define   MPFLUSH_ARCW2_MSB U(24) /*     Multiprocessing buffer usage : conditional call of DATA_MEMORY_BARRIER */
#define   MPFLUSH_ARCW2_LSB U(24) /*  1  flush data used after processing or update of the descriptor */
#define      READ_ARCW2_MSB SIZE_EXT_FMT0_MSB /*     */
#define      READ_ARCW2_LSB SIZE_EXT_FMT0_LSB /* 24  */


#define       COLLISION_ARC_BYTE U(3) 
#define  WRIOCOLL_ARCW3    U( 3) 
#define COLLISION_ARCW3_MSB U(31) /*  8 MSB byte used to lock the SWC, loaded with arch+proc+instance ID */ 
#define COLLISION_ARCW3_LSB U(24) /*       to check node-access collision from an other processor */
#define     WRITE_ARCW3_MSB FRAMESIZE_FMT0_MSB /*    write pointer is incremented by FRAMESIZE_FMT0 */
#define     WRITE_ARCW3_LSB FRAMESIZE_FMT0_LSB /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */


#define         DBGFMT_ARCW4    U( 4) 
#define  TRACECMD_ARCW4_MSB U(31) /*  1 default tracing data estimation = data-rate (see Margo observability reports) */ 
#define  TRACECMD_ARCW4_LSB U(31) /*      Margo (Latin word for ‘edge’) open standard for industrial automation */
#define COMPUTCMD_ARCW4_MSB U(31)       
#define COMPUTCMD_ARCW4_LSB U(27) /*  7 gives the debug task to proceed / script to run (enum debug_arc_computation_1D) */
#define DEBUG_REG_ARCW4_MSB U(23) /*    debug registers have 64bits and are stored in the first arc descriptors  */
#define DEBUG_REG_ARCW4_LSB U(16) /*  8 2x32bits debug result index [0..256][page: DBGB0_LW1] = data + STREAM_TIMESTMP for ex. */
#define CONSUMFMT_ARCW4_MSB U(15) /*    */
#define CONSUMFMT_ARCW4_LSB U( 8) /*  7 bits CONSUMER format  */ 
#define PRODUCFMT_ARCW4_MSB U( 7) /*  7 bits PRODUCER format  (intptr_t) +[i x STREAM_FORMAT_SIZE_W32]  */ 
#define PRODUCFMT_ARCW4_LSB U( 0) /*    Graph generator gives IN/OUT arc's frame size to be the LCM of NODE "grains" */


#define         LOGFMT_ARCW5    U( 5) 
#define   LOGCONS_ARCW5_MSB U(31) 
#define   LOGCONS_ARCW5_LSB U(24) /*  8 consumer logs of flow errors  */ 
#define   LOGPROD_ARCW5_MSB U(23) 
#define   LOGPROD_ARCW5_LSB U(16) /*  8 producer logs of flow errors */ 
#define  LOGTRACE_ARCW5_MSB U(15) 
#define  LOGTRACE_ARCW5_LSB U( 0) /* 16  */ 

#define         LOGFMT_ARCW6    U( 6) 
#define LOGTMESTP_ARCW6_MSB U(31) 
#define LOGTMESTP_ARCW6_LSB U( 0) 

#define         LOGFMT_ARCW7    U( 7) 
#define LOGTMESTP_ARCW7_MSB U(31) 
#define LOGTMESTP_ARCW7_LSB U( 0) 

//#define PLATFORM_IO 0                   /* 3 bits offets code for arcs external to the graph */


/*================================================================================================================*/    
/*
    "SERV_command"  from the nodes, to "arm_stream_services"

    void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n)

    commands from the NODE to Stream
    16 family of commands:
    - 1 : internal to Stream, reset, debug trace, report errors , described here
            SYSCALL_INTERNAL         TODO
            - Un/Lock a section of the graph
            - Jump to +/- N nodes in the list,      => read HEADER + test fields + jump
            - system regsters access: who am I ?

    described in stream_common_const.h:
    - 2 : arc access for SCRIPT : pointer, last data, debug fields, format changes
    - 3 : format converters (time, raw data)
    - 4 : stdlib.h subset (time, stdio)
    - 5 : math.h subset
    - 6 : Interface to CMSIS-DSP
    - 7 : Interface to CMSIS-NN 
    - 8 : Multimedia audio library
    - 9 : Image processing library
    - 10..15 : reserved

*/


/* ---------------------------------------------------------------- */
/* 1/SERV_INTERNAL                                                 */
/* ---------------------------------------------------------------- */


    /*  FUNCTION_SSRV GROUP : HW AND SLEEP CONTROL --------------- */
#define SERV_INTERNAL_SLEEP_CONTROL                             0x001
#define SERV_INTERNAL_PLATFORM_CLEAR_BACKUP_MEM                 0x002   /* cold start : clear backup memory */
#define SERV_INTERNAL_CPU_CLOCK_UPDATE   0x30 /* notification from the application of the CPU clock setting (TBD @@@) */



    /*  FUNCTION_SSRV GROUP : READ MEMORY ------------------------ */
#define SERV_INTERNAL_READ_MEMORY                               0x101
#define SERV_INTERNAL_READ_MEMORY_FAST_MEM_ADDRESS              0x102


    /*  FUNCTION_SSRV GROUP : SERIAL COMMUNICATION --------------- */
#define SERV_INTERNAL_SERIAL_COMMUNICATION                      0x201


    /*  FUNCTION_SSRV GROUP : MUTUAL EXCLUSION ------------------- */
#define SERV_MUTUAL_EXCLUSION 4u
    /* SERVICE FUNCTIONS */
    #define SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP 0x301
    #define SERV_INTERNAL_MUTUAL_EXCLUSION_RD_BYTE_MP           0x302
    #define SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP           0x303
    #define SERV_INTERNAL_MUTUAL_EXCLUSION_CLEAR_BIT_MP         0x304

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


    /*  FUNCTION_SSRV GROUP : IO SETTINGS -------------------------*/
#define SERV_INTERNAL_CHANGE_IO_SETTING                         0x401

    /* used to call p_io_function_ctrl functions with STREAM_READ_PARAMETER : the arm_stream_converter checks
    *   if consumer-node (of the associated producer-node for IO TX) is compatible with the format
    *   to rescale the data stream to the good amplitude/scale 
    *   SERV_READ_IO_SCALING receives the IO ID
    *       it reads the setting in the graph STREAM_IO_CONTROL table
    *       calls the p_io_function_ctrl with STREAM_READ_PARAMETER 
    */
#define SERV_INTERNAL_READ_IO_SCALING 0u       



    /*  FUNCTION_SSRV GROUP : TIME ------------------------------- */
#define SERV_INTERNAL_READ_TIME 6u
    #define SERV_INTERNAL_READ_TIME64                           0x601
    #define SERV_INTERNAL_READ_TIME32                           0x602
    #define SERV_INTERNAL_READ_TIME16                           0x603



    /*  FUNCTION_SSRV GROUP : BOOT ------------------------------- */
#define SERV_INTERNAL_KEYEXCHANGE       0x701 /* at reset time : key exchanges */

    // see platform_init_copy_graph() 
#define PLATFORM_INIT_AL                0x700   /* set the graph pointer */
#define PLATFORM_MP_GRAPH_SHARED        0x701   /* need to declare the graph area as "sharable" in S = MPU_RASR[18] */
#define PLATFORM_MP_BOOT_WAIT           0x702   /* wait commander processor copies the graph */
#define PLATFORM_MP_BOOT_DONE           0x703   /* to confirm the graph was copied in RAM */
#define PLATFORM_MP_RESET_WAIT          0x704   /* wait the graph is initialized */
#define PLATFORM_MP_RESET_DONE          0x705   /* tell the reset sequence was executed for that Stream instance */




    /*  FUNCTION_SSRV GROUP : ERRORS ------------------------------- */
#define PLATFORM_ERROR                  0x901   /* error to report to the application */



/*================================================================================================================*/    

#define U(x) ((uint32_t)(x))
#define U8(x) ((uint8_t)(x))


#define STREAM_PTRPHY      4u    
#define STREAM_PTR27B      5u    


/* constants for uint8_t itoa(char *s, int32_t n, int base) 
* string conversions to decimal and hexadecimal */

#define C_BASE2 2u
#define C_BASE10 10u
#define C_BASE16 16u

/*================================================================================================================*/    
/*                                            FLOAT                                                               */
/*================================================================================================================*/    

#define FADD(tmp, src1, src2)  tmp=src1+src2
#define FSUB(tmp, src1, src2)  
#define FMUL(tmp, src1, src2)  
#define FDIV(tmp, src1, src2)  
#define FMAX(tmp, src2, src1)  
#define FMIN(tmp, src2, src1)  
#define FAMAX(tmp, src2, src1) 
#define FAMAX(tmp, src2, src1) 
#define FTESTEQU(tmp, dst) 0
#define FTESTLEQ(tmp, dst) 0
#define FTESTLT(tmp, dst)  0
#define FTESTNEQ(tmp, dst) 0
#define FTESTGEQ(tmp, dst) 0
#define FTESTGT(tmp, dst)  0

#define F2I(src) ((uint32_t)(src))
#define I2F(src) ((float_t)(src))

/*================================================================================================================*/    

#endif /* cSTREAM_CONST_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
    

