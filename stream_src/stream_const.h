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

#include "../stream_nodes/stream_common_const.h"

#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */


/*------ Major-Minor version numbers ------*/
#define GRAPH_INTERPRETER_VERSION 0x0100

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
    STREAM_IO_CONTROL (4 words per IO)  size = [NB_IOS_GR1], "ONGOING" flag is in RAM
        depends on the domain of the IO

  *- SCRIPTS are adressed with a table_int32[128] : offset, ARC, binary format
        ARC descriptor: size regs/stack, parameters/UC, collision Byte, max cycles 
        The first are indexed with the NODE header 6b index (SCRIPT_LW0) 
        Script index #0 means "disabled"  Indexes 1..up to 63 are used for shared subroutines.
     
  *- LINKED-LIST of SWC
       minimum 5 words/SWC
       Word0  : header processor/architecture, nb arcs, SWCID, arc
       Word1+n: arcs * 2  + debug page
       Word2+n: 2xW32 : ADDR + SIZE + nb of memory segments
       Word3+n: Preset, New param!, Skip length, 
          byte stream: nbparams (ALLPARAM), {tag, nbWord32, params}
       list Ends with the NODE ID 0x03FF 
    
    -----------------SHARED RAM-------------------------------  OFFSET 0

  *-PIO "ONGOING" (1 Byte per IO)  size = [NB_IOS_GR1] (in instance "uint8_t *ongoing")
    
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
#define MAX_NB_STREAM_INSTANCES 4 


/* the first word of the graph holds the number of words32 */
#define GRAPH_SIZE_SKIP_WORD0 1

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

#define VID0    0
#define GRAPH_START_VID0 0

/* for pack2linaddr_ptr () */
//#define LINADDR_UNIT_BYTE   1
#define LOG2ADDR_UNIT_W32   2
#define LINADDR_UNIT_W32    4
#define LINADDR_UNIT_EXTD  64

#define COPY_CONF_GR0_COPY_ALL_IN_RAM   0
#define COPY_CONF_GR0_FROM_PIO          1
#define COPY_CONF_GR0_ALREADY_IN_RAM    2

#define PACKSHARERAMSPLIT(share,RAMsplit) ((share<<3) + RAMsplit)
#define GR0_INDEX   1                // graph size + GR(X)
#define   _____________GR0_MSB U(31) 
#define   _____________GR0_LSB U(30) // 2
#define       RAMSPLIT_GR0_MSB U(29) //   
#define       RAMSPLIT_GR0_LSB U(28) // 2 COPY_CONF_GR0_COPY_ALL_IN_RAM / _FROM_PIO / _ALREADY_IN_RAM
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
/*
 * Maximum number of IOs used by the graph 
 * This number is lower or equal to the maximum of possible IO connexions of the platform (max {FWIOIDX_IOFMT0})
 */
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
#define STREAM_SCHD_RET_END_EACH_NODE        1    /* return to caller after each NODE calls */
#define STREAM_SCHD_RET_END_ALL_PARSED      2    /* return to caller once all NODE are parsed */
#define STREAM_SCHD_RET_END_NODE_NODATA      3    /* return to caller when all NODE are starving */
                                              
#define STREAM_SCHD_NO_SCRIPT               0 
#define STREAM_SCHD_SCRIPT_BEFORE_EACH_NODE  1    /* script is called before each NODE called */
#define STREAM_SCHD_SCRIPT_AFTER_EACH_NODE   2    /* script is called after each NODE called */
#define STREAM_SCHD_SCRIPT_END_PARSING      4    /* script is called at the end of the loop */
#define STREAM_SCHD_SCRIPT_START            8    /* script is called when starting */
#define STREAM_SCHD_SCRIPT_END              16   /* script is called before return */
#define STREAM_SCHD_SCRIPT_UNUSED           32   /* 6bits are reserved in SCRIPT_SCTRL */
//};
#define STREAM_COLD_BOOT 0u
#define STREAM_WARM_BOOT 1u         /* Reset + restore memory banks from retention */

/* number of NODE calls in sequence */
#define MAX_NODE_REPEAT 4u


#define STREAM_MAIN_INSTANCE 1

/* ----------- instance -> scheduler_control  ------------- */
#define     UNUSED_SCTRL_MSB U(31)   
#define     UNUSED_SCTRL_LSB U(19)  /* 12 */ 
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

        The table arm_graph_interpreter_io_ack[10] makes the translation 
            platform_io_al_idx_to_graph [FWIOIDX] = graph_io_idx index in the graph for pio_control[]
        arm_graph_interpreter_io_ack (platform_io_al_idx_to_graph[IO_PLATFORM_ANALOG_SENSOR_0], (uint8_t *)data, size);
*/
#define STREAM_IOFMT_SIZE_W32 4   /* four word for IO controls : one for the scheduler, three for IO settings */

#define MAX_IO_FUNCTION_PLATFORM 127 /* table of functions : platform_io[MAX_IO_FUNCTION_PLATFORM] */
                              
#define IO_COMMAND_SET_BUFFER 0u  /* arc buffer point directly to the IO buffer: ping-pong, big buffer */
#define IO_COMMAND_DATA_MOVE  1u  /* the IO has its own buffer */
                              
#define RX0_TO_GRAPH          0u
#define TX1_FROM_GRAPH        1u
                              
#define IO_IS_COMMANDER0      0u
#define IO_IS_SERVANT1        1u

#define IOFMT0 0                   /* first word used by the scheduler */

#define ____UNUSED_IOFMT0_MSB 31u  
#define ____UNUSED_IOFMT0_LSB 26u  /* 6 */
#define    FWIOIDX_IOFMT0_MSB 25u  
#define    FWIOIDX_IOFMT0_LSB 19u  /* 7 */
#define  IO_DOMAIN_IOFMT0_MSB 18u  /*    the domain should match with the arc prod/cons format */
#define  IO_DOMAIN_IOFMT0_LSB 15u  /* 4  16 Domains, to select the format of the tuning  */
#define FROMIOBUFF_IOFMT0_MSB 14u   
#define FROMIOBUFF_IOFMT0_LSB 14u  /* 1  share the arc buffer with the IO BSP "io_buffer_allocation" */
#define  SET0COPY1_IOFMT0_MSB 13u  
#define  SET0COPY1_IOFMT0_LSB 13u  /* 1  command_id IO_COMMAND_SET_BUFFER / IO_COMMAND_DATA_MOVE */
#define   SERVANT1_IOFMT0_MSB 12u  
#define   SERVANT1_IOFMT0_LSB 12u  /* 1  1=IO_IS_SERVANT1 */
#define     RX0TX1_IOFMT0_MSB 11u  /*    direction of the stream */
#define     RX0TX1_IOFMT0_LSB 11u  /* 1  0 : to the graph    1 : from the graph */
#define    IOARCID_IOFMT0_MSB 10u 
#define    IOARCID_IOFMT0_LSB  0u  /* 11  ARC*/

#define IOFMT1 1                   /* domain-specific controls */
#define IOFMT2 2                   /* domain-specific controls */
#define IOFMT3 3                   /* domain-specific controls */

/*================================= ONGOING  (RAM) ==============================
      
      The graph hold a table of uint8_t in RAM for the "on-going" flag    
*/

#define    ONGOING_IO_MSB 7u  
#define    ONGOING_IO_LSB 7u  /* 1 set in scheduler, reset in IO, iomask manages processor access */
#define ____UNUSED_IO_MSB 6u  
#define ____UNUSED_IO_LSB 0u  /* 7 */

/*================================= SCRIPTS =======================================

    W32 script offset table[6b = 64 SCRIPT_LW0] to the byte codes 
        [SCRIPTSSZW32_GR1] = 
        |    nb_script x { word offset, byteCode Format, shared RAM, ARC } 
        |
        |    Flash at the offset position :
        |        { nb Label W32, nb data index W32, nregs }
        |        [ Labels offset in the code and parameters ]
        |        [ Data index in the RAM buffer ]
        |        Byte codes
        |        Parameters 

        RAM ARC descriptor 5 words
            
        ARC buffer 
        |    Registers 8 Bytes : R0 (state) .. R(nregs)
        |    Stack 8 Bytes 
        |    Heap 4 Bytes

    before/after nodes (can be reused for several nodes)



    Script instance = 
       
    
    
    BYTECODE XXXXXXXXXXXXXXX
    
    INSTANCE (arc descriptor address = *script_instance
           |   
           v                    <--- nStack + 1 ------->
           R0 R1 R2 ..  nregs   R13  R14 R15             
           <--- registers--->   RegK SP  SP+1
    STACK                            [.................]
                                     SP init = nregs+2                  
                               
    HEAP / PARAM (4bytes/words)                                [............]

    III_____________________________ if yes, if not, no test, break-point (+ margin)
    ___yyyyy________________________ 32 op-code 
    ________-OPAR___________________ 32 arithmetic op
    _____________DST.SRC1SRC2_______ 10 Registers(r1..r10) + sp(r14) + sp1(r15) + ctrl(r0) + rK(r13)
    _____________________xxxxxxxxxxx 11-bits Constant = +/- 0 .. 1000 / Bit field for {1000 < K < 1024}
    _________________xxxxxxxxxxxxxxx 15-bits Constant 
    
    
    registers format :                      DTYPE
                <---- MSB word ----------------> <---- LSB word ---------------->  
                FEDCBA9876543210FEDCBA987654____ FEDCBA9876543210FEDCBA987654321|  
    int32       ____________________________   0 <------------------------------>  used for R0 = 0
    uint32      ____________________________   1 <------------------------------>  
    q15         ____________________________   2 <------------------------------>  
    fp32        ____________________________   3 <------------------------------>  
    TIME64      <-------------------------->   4 <------------------------------>    
    fp64        <-------------------------->   5 <------------------------------>  mantissa is patched
    int64-4bits <-------------------------->   6 <------------------------------>  LSB are patched
    TIME16      ____________________________   7 ________________<-------------->  
    TIME32      ____________________________   8 <------------------------------>  
    28b + DTYPE ---BASE------SIZE-18------__   9 DTYPE<------ 28bits address---->  typed pointer + circular option (10+18)
    char /0     ____________________________  10 ________________________<------>  
    Control     <-------------------------->  11 <------------------------------>   
    
    
    
                              
     Encoded instructions : 
     COND   INST    DST  LD-TST/ JMP  / MOV     SRC1  SRC2/#K
     0      1       2     ------- 3 -------     4     5  : field index
     ifyes  ld/k    0     add    jmp    wr2bf   0     0   + #iufdptc and constant 
     ifno   test/k  r1    min    cal    ptrinc  r1    r1
            jmp     r10   set                   r10   r10
            mov     sp           ret            sp    sp
            label   sp1                         sp1   sp1
                                                rk    rk

    Assembler                          Binary encoding 
    ---------                          ----------------
    OP_LD/OP_LDK family
      testleq r6 sp                      OP_TESTLEQ  R6 NOP R14 R0        decode OP_TEST
      testleq r2 mul { r4 r6 }	         OP_TESTLEQ  R2 MUL R4 R6 
      testleq r2 add { r4 #int 15}	     OP_TESTLEQK R2 ADD R4 #15        decode RK=15
      testleq r2 max { r4 #float 3.14}	 OP_TESTLEQK R2 MAX R4 DTYPE_W32  >RK=float on extra word
      testleq r2 max { r4 #double 3.141} OP_TESTLEQK R2 MAX R4 DTYPE_W64  >RK=double on two extra words

    OP_LD/OP_LDK family
      ld r2 add { r4 r6 }                OP_LD R2 ADD R4 R6          R2_ptr = R4_ptr [R6]  DTYPE of R4
      ld r6 sp                           OP_LD R6 NOP R14 R0         decode OP_LD + decode NOP
      ld r2 mul { r4 r6 }	             OP_LD R2 MUL R4 R6          R2 = R4 + R6
      ld r2 add { r4 #int 15}	         OP_LDK R2 ADD R4 #15        decode OP_LDK : RK=15 + decode MUL
      ld r2 max { r4 #float 3.14}	     OP_LDK R2 MAX R4 DTYPE_W32  OP_LDK1 : RK=float on extra word
      ld r2 max { r4 #double 3.14159}    OP_LDK R2 MAX R4 DTYPE_W64  OP_LDK1 : RK=double on two extra words


    OP_MOV family
      cast    r2 #float         OP_MOV R0 OPLD_CAST     R2 #DTYPE   DST_ptr = (DTYPE) 
      castptr r2 #float         OP_MOV R0 OPLD_CASTPTR  R2 #DTYPE   DST_ptr = (POINTER DTYPE) 
      base    r4 r5             OP_MOV R4 OPLD_BASE     R5 R0       R4_PTR.base = R5, base for cicular addressing
      size    r4 r5             OP_MOV R4 OPLD_SIZE     R5 R0       R4_PTR.size = R5, size for cicular addressing
      ptrinc  r4, r5, #inc      OP_MOV R4 OPLD_PTRINC   R5 #INCTYPE R4_PTR = R5_PTR +/-1 +/-inc +/-inc_modulo 
      scatter r2 r3 r4          OP_MOV R2 OPLD_SCATTER  R3 R4       R2[R4] = R3    indirect, write with indexes 
      scatter r2 r3 #k          OP_MOV R2 OPLD_SCATTERK R3 #K       R2[K] = R3     indirect, write with indexes 
      gather  r2 r3 r4          OP_MOV R2 OPLD_GATHER   R3 R4       R2 = R3[R4]    indirect, read with indexes 
      gather  r2 r3 #k          OP_MOV R2 OPLD_GATHERK  R3 #K       R2 = R3[K]     indirect, read with indexes 
      wr2bf   r2 r3 LEN.POS     OP_MOV R2 OPLD_WR2BF    R3 #LEN.POS R2(bitfield) = R3 
      rdbf    r2 r3 LEN.POS     OP_MOV R2 OPLD_RDBF     R3 #LEN.POS R2 = R3(bitfield)
      norm    r2 r3             OP_MOV R2 OPLD_NORM     R3 R4       R2 = normed on MSB(R3), applied shift in R4
      swap    r2 r3             OP_MOV R0 OPLD_SWAP     R2 R3       SWAP SRC1, DST 

    OP_JMP family
      jump L15              OP_JUMP R0 OPBR_JUMP       R0 #15      JMP LABEL K11 and PUSH SRC1/SRC2
      jump r4               OP_JUMP R4 OPBR_JUMPA      R0 R0       JMPA  jump to computed address DST, PUSH SRC1/SRC2
      jump -14              OP_JUMP R0 OPBR_JUMPOFF    R0 #-14     JMP signed_K11  signed word32 offset  and push 3 registers 
      banz r4 L15           OP_JUMP R4 OPBR_BANZ       R0 #15      BANZ SRC1   Label_K11    
      call L15              OP_JUMP R0 OPBR_CALL       R0 #15      CALL LABEL K11 and PUSH SRC1/SRC2
      cala r4               OP_JUMP R4 OPBR_CALA       R0 R0       CALA  call computed address DST, PUSH SRC1/SRC2
      callsys 15            OP_JUMP R0 OPBR_CALLSYS    R0 #15      CALLSYS   {K11} system calls (FIFO, TIME, debug, SetParam, DSP/ML, IO/HW, Pointers)  
      script 15             OP_JUMP R0 OPBR_CALLSCRIPT R0 #15      CALLSCRIPT{K11} common scripts and node control   
      callapp 15            OP_JUMP R0 OPBR_CALLAPP    R0 #15      CALLAPP   {K11} 0K6=64 local callbacks 1K6= 64 global callbacks    
      save 3,4,5            OP_JUMP R0 OPBR_SAVEREG    R0 #1Ch     push K11 registers
      restore 3,4,5         OP_JUMP R0 OPBR_RESTOREREG R0 #1Ch     pop k11 registers
      return                OP_JUMP R0 OPBR_RETURN     R0 R0       RETURN {keep registers 
      label R4 L11          OP_JUMP R4 OPBR_LABEL      R0 #K       DST = Label_K15 (offset in code/parameter area (flash) or heap (RAM))

    Labels = L_<symbol> (no instruction)    

    SYSCALLS
    --------
  SYSCALL : 
    - Node Set/ReadParam/Run, patch a field in the header
    - FIFO read/set, data amount/free, debug reg read/set, time-stamp of last access (ASRC use-case)
    - Script TX buffer is starting with a table of indexes (Labels) to the data to address (SYSCALL return the address)
    - Update Format framesize, FS
    - Read Time 16/32/64 from arc/AL, compute differences,
    - Jump to +/- N nodes in the list,      => read HEADER + test fields + jump
      Un/Lock a section of the graph
    - Select new IO settings (set timer period, change gains)
    - AL decides deep sleep with the help of parameters (wake-up in X[s])
    - Trace "string" + data
    - Callbacks (cmd (cmd+LIbName+Subfunction , X, n) for I2C/Radio .. Libraries 
    - Callback returning the current use-case to set parameters
    - Call a relocatable binary section in the Param area
    - Share Physical HW addresses
    - Compute Median from data in a circular buffer + mean/STD

        reprogram the setting of stream_format_io[]
        //  format 6b control + 2b register
        //  load "t", from stack(i) = R(i) or #const
        //  move R(i) to/from arc FIFOdata / debugReg / with/without read index update
        //  compare R(i) with "t" : <> = != and skip next instruction
        //  jump to #label
        //  dsz decrement and jump on non-zero
        //  arithmetic add,sub,(AND,shift), #const/R(j)
        //  Basic DSP: moving average, median(5), Max(using VAD's forgetting factors).
        //  time difference, time comparison, change the setting of the timer (stop/restart)
        //  time elapsed from today, from a reference, from reset, UTC/local time
        //  computations on time-stamps
        //  default implementation with SYSTICK
        //  convert in ASCII format ISO 8601
        //  Modulo 60 function for the translation to mn. Wake me at 5AM.
        //  Activate timer 0.1s , 1s, 10s 1h 1D 1M
        //  Registers : 64bits(addressable in int8/16/32) + 8bits (type: time, temperature, pressure, 4xint16, counter-current-max)
        //  if {data arrived from the button queue}
        //  Registered callback for low-level operations for one or all instances
        //    fixed format f(cmd,ptr,x,n)
        //    example specific : IP address, password to share, Ping IP to blink the LED, read RSSI, read IP@
        //  Default callbacks: sleep/deep-sleep activation, timer control, who am I
        //    DAC/PWM/GPIO controlled with standard stream Arcs
        //  Low-level interface : Fill the I2C control string and callback
        //  Minimum services : average, timer, data formating/rescale/Interp, polling IOs
        //  Power meter process is using 3 phases x voltage, current, reactive power
        //  Save the state of a button (shutter button)
        //Command from arm_stream_command_interpreter() : return the code version number, ..
*/

#define SCRIPT_REGSIZE 8            /* 8 bytes per register */

#define INDEX_SCRIPT_STRUCT_SIZE 2
#define INDEX_SCRIPT_OFFSET 0
#define INDEX_SCRIPT_LABELS 1

#define     ARC_SCROFF0_MSB U(31) /* 11 arc descriptor */
#define     ARC_SCROFF0_LSB U(21) /*                   */
#define  FORMAT_SCROFF0_MSB U(20) /* 3  byte codes format = 0, 7 binary native architecture ARCHID_LW0 */
#define  FORMAT_SCROFF0_LSB U(18) /*       ARMv6-M */
#define  SHARED_SCROFF0_MSB U(23) /* 1  shareable memory for the script with other scripts in mono processor platforms */
#define  SHARED_SCROFF0_LSB U(23) /*                                    */
#define  OFFSET_SCROFF0_MSB U(17) /* 17 offset to the W32 script table */
#define  OFFSET_SCROFF0_LSB U( 0) /*                                   */

#define     ARC_SCROFF1_MSB U(31) /* 11 arc descriptor */
#define     ARC_SCROFF1_LSB U(21) /*                   */
#define  FORMAT_SCROFF1_MSB U(20) /* 3  byte codes format = 0, 7 binary native architecture ARCHID_LW0 */
#define  FORMAT_SCROFF1_LSB U(18) /*       ARMv6-M */
#define  SHARED_SCROFF1_MSB U(23) /* 1  shareable memory for the script with other scripts in mono processor platforms */
#define  SHARED_SCROFF1_LSB U(23) /*                                    */
#define  OFFSET_SCROFF1_MSB U(17) /* 17 offset to the W32 script table */
#define  OFFSET_SCROFF1_LSB U( 0) /*                                   */
         
/* =================
    arc descriptors used to address the working area : registers and stack
*/
#define      SCRIPT_PTR_SCRARCW0  U( 0) /* Base address + NREGS + new UC */
#define          SCRIPT_SCRARCW1  U( 1) /* use case UC0 */
#define          RDFLOW_SCRARCW2  U( 2) /* use-case UC1 + ARCEXTEND_ARCW2  */
#define        WRIOCOLL_SCRARCW3  U( 3) /* synchro byte + STACK LENGTH + Flag logMaxCycles8b */
#define          DBGFMT_SCRARCW4  U( 4) /*  */

          
#define    __________SCRARCW0_MSB U(31)    
#define    __________SCRARCW0_LSB U(29) /*  3    base address of the working memory */
#define NEW_USE_CASE_SCRARCW0_MSB U(28) /*  1  new use-case arrived */ 
#define NEW_USE_CASE_SCRARCW0_LSB U(28) /*     */ 
#define    BASEIDXOFFSCRARCW0_MSB U(27)    
#define    BASEIDXOFFSCRARCW0_LSB U( 0) /* 28  base address of the script memory (regs + state + stack)  */

// duplicate : 
//#define       ARCEXTEND_ARCW2_MSB U(31) /*    Size/Read/Write are used with <<(2x{0..7}) to extend base/size/read/write arc */
//#define       ARCEXTEND_ARCW2_LSB U(29) /* 3  to  256MB, 4GB, 64GB , for use-cases with NN models, video players, etc */


#define    COLLISION_SCRARCW3_MSB U(31) /*  8  */
#define    COLLISION_SCRARCW3_LSB U(24) /*     */
#define LOG2MAXCYCLE_SCRARCW3_MSB U(23) /*  8  minifloat: reset and return when reaching this number of decoded instructions */ 
#define LOG2MAXCYCLE_SCRARCW3_LSB U(16) /*     */ 
#define    __________SCRARCW3_MSB U(15) /*  1  */
#define    __________SCRARCW3_LSB U(15) /*     */
#define       NBREGS_SCRARCW3_MSB U(14) /*  4   number of registers used in this script */ 
#define       NBREGS_SCRARCW3_LSB U(11) /*     */
#define       NSTACK_SCRARCW3_MSB U(10) /* 11   max size of the FIFO/stack in W32 */
#define       NSTACK_SCRARCW3_LSB U( 0) /*     */

/* =================
    script (SCRIPT_LW0) used to copy input test-patterns, set-parameters from the global use-case or from information 
    of an application call-back

    "analog user-interface" called (knobs / needles) give controls and visibility on NODE parameters and
    are accessed with scripts 
*/
#define SCRIPT_PRERUN 1         /* executed before calling the node : the Z flag is set */
#define SCRIPT_POSTRUN 2        /* executed after */

/* ======================================   NODE   ============================================ */ 
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
#define  LOADFMT_LW0_LSB U(23) /*   controled by "node_using_arc_format" in the graph, default = NO /0 */
#define  ARCLOCK_LW0_MSB U(22) /* 1 0/1 the first/second arc locks the components */
#define  ARCLOCK_LW0_LSB U(22) /*   */
#define   SCRIPT_LW0_MSB U(21) /*   script called Before/After (debug, verbose trace control) */
#define   SCRIPT_LW0_LSB U(15) /* 7 script ID to call before and after calling NODE */
#define  ARCSRDY_LW0_MSB U(14) 
#define  ARCSRDY_LW0_LSB U(13) /* 2  first arcs, nb arcs used to check data availability before RUN */
#define   NBARCW_LW0_MSB U(12) 
#define   NBARCW_LW0_LSB U(10) /* 3  total nb arcs, streaming and metadata/control {0 .. MAX_NB_STREAM_PER_NODE} */
#define  NODE_IDX_LW0_MSB U( 9) 
#define  NODE_IDX_LW0_LSB U( 0) /* 10 0=nothing, node index of node_entry_points[] */


        /* word 1+n -arcs

            starting with the one used for locking, the streaming arcs, then the metadata arcs 
            arc(tx) used for locking is ARC0_LW1
        */

#define MAX_NB_STREAM_PER_NODE 4 /* no limit in practice */

#define ARC_RX0TX1_TEST  0x0800 /* MSB gives the direction of the arc */
#define ARC_RX0TX1_CLEAR 0x07FF 

#define ____1_LW1_MSB 31 
#define ____1_LW1_LSB 30 /*  2   */
#define DBGB1_LW1_MSB 29 
#define DBGB1_LW1_LSB 28 /*  2  debug register bank for ARC1 */
#define  ARC1_LW1_MSB 27
#define ARC1D_LW1_LSB 27 /*  1  ARC1 direction */
#define ARC1D_LW1_MSB 27
#define  ARC1_LW1_LSB 16 /* 11  ARC1  11 usefull bits + 1 MSB to tell rx0tx1 */

#define ____0_LW1_MSB 15 
#define ____0_LW1_LSB 14 /*  2   */
#define DBGB0_LW1_MSB 13 
#define DBGB0_LW1_LSB 12 /*  2  debug register bank for ARC0 : debug-arc index of the debug data */
#define  ARC0_LW1_MSB 11
#define ARC0D_LW1_LSB 11 /*  1  ARC0 direction */
#define ARC0D_LW1_MSB 11
#define  ARC0_LW1_LSB  0 /* 11  ARC0, (11 + 1 rx0tx1) up to 2K ARCs */

#if IOARCID_IOFMT0_MSB != (ARC0D_LW1_MSB-1)
#error "IOFORMAT ARC SIZE"
#endif
        /* word 2+n - FIRST WORD : memory banks address + size */

#define NBW32_MEMREQ_LW2  2     /* there are two words per memory segments, to help programing the memory protection unit (MPU) */
#define ADDR_LW2 0              /*      one for the address */ 
#define SIZE_LW2 1              /*      one for the size of the segment */ 

            /* word 2 first word = base address of the memory segment + control on the first segment */
#define  NALLOCM1_LW2_MSB U(31) /*      number of memory segments to give at RESET [0..MAX_NB_MEM_REQ_PER_NODE-1] */  
#define  NALLOCM1_LW2_LSB U(29) /*  3   2 words each : pointer + size */
#define     XDM11_LW2_MSB U(28) /*      0: Rx/Tx flow is asynchronous  1: same consumption on RSx/Tx */   
#define     XDM11_LW2_LSB U(28) /*  1   the input and output frame size of all arcs are identical (manifest: 0=node_variable_rxtx_data_rate)*/ 
#define BASEIDXOFFLW2_MSB U(27) 
#define   DATAOFF_LW2_MSB DATAOFF_ARCW0_MSB
#define   DATAOFF_LW2_LSB DATAOFF_ARCW0_LSB
#define   BASEIDX_LW2_MSB BASEIDX_ARCW0_MSB
#define   BASEIDX_LW2_LSB BASEIDX_ARCW0_LSB
#define BASEIDXOFFLW2_LSB U( 0) /* 28  */

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
#define LW2S_NOSWAP 0
#define LW2S_SWAP 1
#define LW2S_COPY 2


#define       KEY_LW2S_MSB U(31) /*      */
#define       KEY_LW2S_LSB U(31) /*  1   protocol for key exchanges (boot and graph/user) */
#define CLEARSWAP_LW2S_MSB U(29) /*      bit used on the first memory bank */
#define CLEARSWAP_LW2S_LSB U(24) /*  1   at least one memory segment to clear or to swap */
#define     CLEAR_LW2S_MSB U(24) /*      */
#define     CLEAR_LW2S_LSB U(24) /*  1   protocol for key exchanges (boot and graph/user) */
#define      SWAP_LW2S_MSB U(23) /*      0= normal memory segment, 2 = copy before execute */
#define      SWAP_LW2S_LSB U(22) /*  2   1= swap before/after execute */
#define  EXT_SIZE_LW2S_MSB U(21) /*      SizeMax = 1M / 16M / 256M / 4G with EXT=0/1/2/3 */
#define  EXT_SIZE_LW2S_LSB U(20) /*  2    */
#define BUFF_SIZE_LW2S_MSB U(19) /* ###  overlaid with SWAPBUFID_LW2S in case of COPY / SWAP */
#define BUFF_SIZE_LW2S_LSB U( 0) /* 20   Wrd32-acurate up to 1MBwords x (1 << (4 * EXT_SIZE)) */

#define SWAPBUFID_LW2S_MSB  ARC0_LW1_MSB /*     ARC => swap source address in slow memory + swap length */
#define SWAPBUFID_LW2S_LSB  ARC0_LW1_LSB /* 12  ARC0, (11 + 1) up to 2K FIFO */


  /* word 3+n - parameters 
    NODE header can be in RAM (to patch the parameter area, cancel the component..)

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

  NODE can declare an extra input arc to receive a huge set of parameters (when >256kB), for example a 
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


/* ================================= */

/* last word has NODE index 0b11111..111 */
#define GRAPH_LAST_WORD_MSB NODE_IDX_LW0_MSB
#define GRAPH_LAST_WORD_LSB NODE_IDX_LW0_LSB
#define GRAPH_LAST_WORD U((U(1)<<U(NODE_IDX_LW0_MSB- NODE_IDX_LW0_LSB+1U))-1U)

/*
 *  NODE manifest :
 */ 

//enum stream_node_status {
#define NODE_BUFFERS_PROCESSED 0
#define NODE_NEED_RUN_AGAIN 1         /* NODE completion type */

//enum mem_mapping_type {
#define MEM_TYPE_STATIC          0    /* (LSB) memory content is preserved (default ) */
#define MEM_TYPE_WORKING         1    /* scratch memory content is not preserved between two calls */
#define MEM_TYPE_PERIODIC_BACKUP 2    /* persistent static parameters to reload for warm boot after a crash */
#define MEM_TYPE_PSEUDO_WORKING  3    /* static only during the uncompleted execution state of the SWC, see NODE_RUN 

                periodic backup use-case : long-term estimators. This memory area is cleared at cold NODE_RESET and 
                refreshed for warm NODE_RESET. The NODE should not reset it (there is 
                no "warm-boot reset" entry point. The period of backup depends on platform capabilities 
                When MBANK_BACKUP is a retention-RAM there is nothing to do, when it is standard RAM area then on periodic
                basis the AL will be call to transfer data to Flash 
            */
              

//enum mem_speed_type                         /* memory requirements associated to enum memory_banks */
#define MEM_SPEED_REQ_ANY           0    /* best effort */
#define MEM_SPEED_REQ_FAST          1    /* will be internal SRAM when possible */
#define MEM_SPEED_REQ_CRITICAL_FAST 2    /* will be TCM when possible
           When a NODE is declaring this segment as relocatable ("RELOC_MEMREQ") it will use 
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


#define NODE_CONTROLS U(4)
#define NODE_CONTROLS_NAME U(8)

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

#define SIZEOF_ARCDESC_W32 5

#define   BUF_PTR_ARCW0    U( 0)
#define    _______ARCW0_MSB U(31) 
#define    _______ARCW0_LSB U(28) /*  4 */
#define BASEIDXOFFARCW0_MSB U(27) /*    */
#define   DATAOFF_ARCW0_MSB U(27) /*    address = offset[DATAOFF] + 4x BASEIDX[Bytes] */
#define   DATAOFF_ARCW0_LSB U(22) /*  6 64 x 64bits offset indexes  */
#define   BASEIDX_ARCW0_MSB U(21) /*    base address 22bits linear address range in Word32 */
#define   BASEIDX_ARCW0_LSB U( 0) /* 22 0x3F.FFFF(W32) =  4MW/16MBytes extended with ARCEXTEND_ARCW4 */
#define BASEIDXOFFARCW0_LSB U( 0) /*    base + offset */
                                
#define BUFSIZDBG_ARCW1    U( 1)
#define    _______ARCW1_MSB U(31) 
#define    _______ARCW1_LSB U(21) /* 10 */
#define BUFF_SIZE_ARCW1_MSB U(21) /*    */
#define BUFF_SIZE_ARCW1_LSB U( 0) /* 22 BYTE-acurate up to 4MBytes (up to 128GB with ARCEXTEND_ARCW2 */

#define    RDFLOW_ARCW2    U( 2)  
#define ARCEXTEND_ARCW2_MSB U(31) /*    Size/Read/Write are used with <<(2x{0..7}) to extend base/size/read/write arc */
#define ARCEXTEND_ARCW2_LSB U(29) /* 3  to  256MB, 4GB, 64GB , for use-cases with NN models, video players, etc */
#define    _______ARCW2_MSB U(28) 
#define    _______ARCW2_LSB U(24) /* 5  */
#define   MPFLUSH_ARCW2_MSB U(23) 
#define   MPFLUSH_ARCW2_LSB U(23) /* 1  flush data used after processing */
#define FLOWERROR_ARCW2_MSB U(22)
#define FLOWERROR_ARCW2_LSB U(22) /* 1  under/overflow 0=nothing or best effort from IO_DOMAIN_IOFMT0 */
#define      READ_ARCW2_MSB U(21) /*    data read index  Byte-acurate up to 4MBytes starting from base address */
#define      READ_ARCW2_LSB U( 0) /* 22 this is incremented by "frame_size" FRAMESIZE_FMT0  */

#define COLLISION_ARC_OFFSET_BYTE U(3) 
#define  WRIOCOLL_ARCW3    U( 3) 
#define COLLISION_ARCW3_MSB U(31) /*  8 MSB byte used to lock the SWC, loaded with arch+proc+instance ID */ 
#define COLLISION_ARCW3_LSB U(24) /*       to check node-access collision from an other processor */
#define    _______ARCW3_MSB U(23) 
#define    _______ARCW3_LSB U(23) /*  1  */
#define ALIGNBLCK_ARCW3_MSB U(22) /*    producer blocked */
#define ALIGNBLCK_ARCW3_LSB U(22) /*  1 producer sets "need for data realignement"  */
#define     WRITE_ARCW3_MSB U(21) /*    write pointer is incremented by FRAMESIZE_FMT0 */
#define     WRITE_ARCW3_LSB U( 0) /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */

#define         DBGFMT_ARCW4    U( 4) 
#define  TRACECMD_ARCW4_MSB U(31) 
#define  TRACECMD_ARCW4_LSB U(31) /*  1 default tracing data estimation = data-rate (see MARGO observability reports) */ 
#define COMPUTCMD_ARCW4_MSB U(31)       
#define COMPUTCMD_ARCW4_LSB U(27) /*  7 gives the debug task to proceed  (enum debug_arc_computation_1D) */
#define DEBUG_REG_ARCW4_MSB U(23) /*    debug registers have 64bits and are stored in the first arc descriptors  */
#define DEBUG_REG_ARCW4_LSB U(16) /*  8 2x32bits debug result index [0..256][page: DBGB0_LW1] = data + STREAM_TIMESTMP */
#define CONSUMFMT_ARCW4_MSB U(15) /*    */
#define CONSUMFMT_ARCW4_LSB U( 8) /*  8 bits CONSUMER format  */ 
#define PRODUCFMT_ARCW4_MSB U( 7) /*  8 bits PRODUCER format  (intPtr_t) +[i x STREAM_FORMAT_SIZE_W32]  */ 
#define PRODUCFMT_ARCW4_LSB U( 0) /*    Graph generator gives IN/OUT arc's frame size to be the LCM of NODE "grains" */

#define         LOGFMT_ARCW4    U( 5) 
#define  LOGTRACE_ARCW5_MSB U(31) 
#define  LOGTRACE_ARCW5_LSB U( 0) /* 32 default debug/trace location is in the descriptor (TRACECMD / DEBUG_REG) */ 

/* arcs with indexes higher than IDX_ARCS_desc, see enum_arc_index */

//#define PLATFORM_IO 0                   /* 3 bits offets code for arcs external to the graph */


/*================================================================================================================*/    
/*
    "SERV_command"  from the nodes, to "arm_stream_services"

    void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n)

    commands from the NODE to Stream
    16 family of commands:
    - 1 : internal to Stream, reset, debug trace, report errors , described here

            described in stream_common_const.h:
    - 2 : arc access for scripts : pointer, last data, debug fields, format changes
    - 3 : format converters (time, raw data)
    - 4 : stdlib.h subset (time, stdio)
    - 5 : math.h subset
    - 6 : Interface to CMSIS-DSP
    - 7 : Interface to CMSIS-NN 
    - 8 : Multimedia audio library
    - 9 : Image processing library
    - 10..15 : reserved

*/


//#define NOOPTION_SSRV 0
//#define NOCONTROL_SSRV 0
//
///* arm_stream_services COMMAND */
//#define  CONTROL_SSRV_MSB U(31)       
//#define  CONTROL_SSRV_LSB U(28) /* 4   set/init/wait completion, in case of coprocessor usage */
//#define   OPTION_SSRV_MSB U(27)       
//#define   OPTION_SSRV_LSB U(24) /* 4   compute accuracy, in-place processing, frame size .. */
//#define FUNCTION_SSRV_MSB U(23)       
//#define FUNCTION_SSRV_LSB U( 4) /* 20   64K functions/group  */
//#define    GROUP_SSRV_MSB U( 3)       
//#define    GROUP_SSRV_LSB U( 0) /* 4    16 groups */
//
//#define PACK_SERVICE(CTRL,OPTION,FUNC,GROUP) (((CTRL)<<CONTROL_SSRV_LSB)|((OPTION)<<OPTION_SSRV_LSB)|((FUNC)<<FUNCTION_SSRV_LSB)|(GROUP)<<GROUP_SSRV_LSB)


/* 1/SERV_INTERNAL ------------------------------------------------ */

/*  FUNCTION_SSRV GROUP : SLEEP CONTROL ---------------------- */
#define AL_SERVICE_SLEEP_CONTROL 1

/*  FUNCTION_SSRV GROUP : READ MEMORY ------------------------ */
#define AL_SERVICE_READ_MEMORY 2
    /* SERVICE FUNCTIONS */
    #define AL_SERVICE_READ_MEMORY_FAST_MEM_ADDRESS 1

/*  FUNCTION_SSRV GROUP : SERIAL COMMUNICATION --------------- */
#define AL_SERVICE_SERIAL_COMMUNICATION 3

/*  FUNCTION_SSRV GROUP : MUTUAL EXCLUSION ------------------- */
#define AL_SERVICE_MUTUAL_EXCLUSION 4
    /* SERVICE FUNCTIONS */
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

/*  FUNCTION_SSRV GROUP : IO SETTINGS -------------------------*/
#define AL_SERVICE_CHANGE_IO_SETTING 5

/*  FUNCTION_SSRV GROUP : TIME ------------------------------- */
#define AL_SERVICE_READ_TIME 6
    #define AL_SERVICE_READ_TIME64 1
    #define AL_SERVICE_READ_TIME32 2
    #define AL_SERVICE_READ_TIME16 3

#define AL_SERVICE_UNUSED3 7
#define MAX_NB_APP_SERVICES 8



//enum platform_al_services       
#define PLATFORM_INIT_AL           0x00   /* set the graph pointer */
#define PLATFORM_MP_GRAPH_SHARED   0x01   /* need to declare the graph area as "sharable" in S = MPU_RASR[18] */
#define PLATFORM_MP_BOOT_WAIT      0x02   /* wait commander processor copies the graph */
#define PLATFORM_MP_BOOT_DONE      0x03   /* to confirm the graph was copied in RAM */
#define PLATFORM_MP_RESET_WAIT     0x04   /* wait the graph is initialized */
#define PLATFORM_MP_RESET_DONE     0x05   /* tell the reset sequence was executed for that Stream instance */

#define PLATFORM_CLEAR_BACKUP_MEM  0x0D   /* cold start : clear backup memory */
#define PLATFORM_ERROR             0x10   /* error to report to the application */


#define SERV_INTERNAL_RESET 1u
#define SERV_INTERNAL_NODE_REGISTER 2u

/* change stream format from NODE media decoder, script applying change of use-case (IO_format, vocoder frame-size..): sampling, nb of channel, 2D frame size */
#define SERV_INTERNAL_FORMAT_UPDATE 3u      

//#define SERV_INTERNAL_FORMAT_UPDATE_FS 3u       /* NODE information for a change of stream format, sampling, nb of channel */
//#define SERV_INTERNAL_FORMAT_UPDATE_NCHAN 4u     /* raw data sample, mapping of channels, (web radio use-case) */
//#define SERV_INTERNAL_FORMAT_UPDATE_RAW 5u

#define SERV_INTERNAL_SECURE_ADDRESS 6u       /* this call is made from the secured address */
#define SERV_INTERNAL_AUDIO_ERROR 7u          /* PLC applied, Bad frame (no header, no synchro, bad data format), bad parameter */
#define SERV_INTERNAL_DEBUG_TRACE 8u          /* 1b, 1B, 16char */
#define SERV_INTERNAL_DEBUG_TRACE_STAMPS 9u
#define SERV_INTERNAL_AVAILABLE 10u
#define SERV_INTERNAL_SETARCDESC 11u          /* buffers holding MP3 songs.. rewind from script, 
                                                            switch a NN model to another, change a parameter-set using arcs */
#define SERV_INTERNAL_KEYEXCHANGE 12          /* at reset time : key exchanges */

#define SERV_CPU_CLOCK_UPDATE 13              /* notification from the application of the CPU clock setting (TBD @@@) */

//SERV_INTERNAL_DEBUG_TRACE, SERV_INTERNAL_DEBUG_TRACE_1B, SERV_INTERNAL_DEBUG_TRACE_DIGIT, 
// 
//SERV_INTERNAL_DEBUG_TRACE_STAMPS, SERV_INTERNAL_DEBUG_TRACE_STRING,
// 
//STREAM_SAVE_HOT_PARAMETER, 

//STREAM_LOW_POWER,     /* interface to low-power platform settings, "wake-me in 24h with deep-sleep in-between" */
//                          " I have nothing to do most probably for the next 100ms, do what is necessary "

 
//STREAM_PROC_ARCH,     /* returns the processor architecture details, used before executing specific assembly codes */

/*================================================================================================================*/    

#define U(x) ((uint32_t)(x))
#define U8(x) ((uint8_t)(x))


#define STREAM_PTRPHY      4    
#define STREAM_PTR27B      5    


/* constants for uint8_t itoa(char *s, int32_t n, int base) 
* string conversions to decimal and hexadecimal */

#define C_BASE2 2
#define C_BASE10 10
#define C_BASE16 16



#endif /* cSTREAM_CONST_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
    

