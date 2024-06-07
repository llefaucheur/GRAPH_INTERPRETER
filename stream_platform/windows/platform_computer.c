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

/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 0

#define _CRT_SECURE_NO_DEPRECATE 1
#if DATA_FROM_FILES
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "stream_const.h"      
#include "stream_types.h"

#include "platform_computer.h"


extern const uint8_t platform_audio_out_bit_fields[];

uint8_t platform_io_al_idx_to_graph[LAST_IO_FUNCTION_PLATFORM];

#define L S->long_offset
#define G S->graph

/* 
    this table will be extended with pointers to nodes loaded 
    dynamically and compiled with position independent execution options

    it is aligned with the list of SOFTWARE COMPONENTS MANIFESTS in "files_manifests_computer.txt"
*/
/* -----------------------------------
    Full node descriptions given in ./stream_tools/TEMPLATE_GRAPH.txt
*/

extern p_stream_node arm_stream_graph_control;   /*  1*/
extern p_stream_node arm_stream_script;          /*  2*/
extern p_stream_node arm_stream_router;          /*  3*/
extern p_stream_node arm_stream_converter;       /*  4*/
extern p_stream_node arm_stream_amplifier;       /*  5*/
extern p_stream_node arm_stream_mixer;           /*  6*/
extern p_stream_node arm_stream_filter;          /*  7*/
extern p_stream_node arm_stream_detector;        /*  8*/
extern p_stream_node arm_stream_rescaler;        /*  9*/
extern p_stream_node arm_stream_compressor;      /* 10*/
extern p_stream_node arm_stream_decompressor;    /* 11*/
extern p_stream_node arm_stream_modulator;       /* 12*/
extern p_stream_node arm_stream_demodulator;     /* 13*/
extern p_stream_node arm_stream_resampler;       /* 14*/
extern p_stream_node arm_stream_qos;             /* 15*/
extern p_stream_node arm_stream_split;           /* 16*/
extern p_stream_node arm_stream_detector2D;      /* 17*/
extern p_stream_node arm_stream_filter2D;        /* 18*/
extern p_stream_node arm_stream_analysis;        /* 19*/

#define TBD 0

p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS] =
{
    /*--------- ARM ---------*/
    /*  0*/ (void *)0,                          /* node disabled */
    /*  1*/ (void *)&arm_stream_graph_control,  /* scheduler control : lock, bypass, loop, if-then */
    /*  2*/ (void *)&arm_stream_script,         /* byte-code interpreter, index "arm_stream_script_INDEX" */
    /*  3*/ (void *)&arm_stream_router,         /* copy input arcs and subchannel and output arcs and subchannels   */     
    /*  4*/ (void *)&arm_stream_converter,      /* raw data format converter */
    /*  5*/ (void *)&arm_stream_amplifier,      /* amplifier mute and un-mute with ramp and delay control */
    /*  6*/ (void *)&arm_stream_mixer,          /* multichannel mixer with mute/unmute and ramp control */
    /*  7*/ (void *)&arm_stream_filter,         /* cascade of DF1 filters */
    /*  8*/ (void *)&arm_stream_detector,       /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
    /*  9*/ (void *)&arm_stream_rescaler,       /* raw data values remapping using "interp1" */
    /* 10*/ (void *)&arm_stream_compressor,     /* raw data compression with adaptive prediction */
    /* 11*/ (void *)&arm_stream_decompressor,   /* raw data decompression */
    /* 12*/ (void *)&arm_stream_modulator,      /* signal generator with modulation */
    /* 13*/ (void *)&arm_stream_demodulator,    /* signal demodulator, frequency estimator */
    /* 14*/ (void *)&arm_stream_resampler,      /* asynchronous sample-rate converter */
    /* 15*/ (void *)&arm_stream_qos,            /* raw data interpolator with synchronization to one HQoS stream */
    /* 16*/ (void *)&arm_stream_split,          /* let a buffer be used by several nodes */
    /* 17*/ (void *)&arm_stream_detector2D,     /* activity detection, pattern detection */
    /* 18*/ (void *)&arm_stream_filter2D,       /* Filter, rescale, rotate, exposure compensation */
    /* 19*/ (void *)&arm_stream_analysis,       /* arm_stream_analysis, */
};


static uint32_t MEXT[SIZE_MBANK_DMEM_EXT];
static uint32_t TCM1[SIZE_MBANK_DMEMFAST]; 
const intPtr_t long_offset[MAX_NB_MEMORY_OFFSET] = 
{   (const intPtr_t) &(MEXT[0]),
    (const intPtr_t) &(TCM1[0]),
};


uint8_t platform_iocontrol[LAST_IO_FUNCTION_PLATFORM];      /* bytes/flags of on-going activity */
arm_stream_instance_t * platform_io_callback_parameter;


extern p_io_function_ctrl data_in_0;      
extern p_io_function_ctrl analog_sensor_0;
extern p_io_function_ctrl motion_in_0;    
extern p_io_function_ctrl audio_in_0;     
extern p_io_function_ctrl d2_in_0;        
extern p_io_function_ctrl line_out_0;     
extern p_io_function_ctrl gpio_out_0;     
extern p_io_function_ctrl gpio_out_1;     
extern p_io_function_ctrl data_out_0;     

/*
    files_manifests_computer.txt :

     stream_in_0.txt        0    data from the application
     analog_sensor_0.txt    1    ADC 
     gpio_out_0.txt         2    PWM
     line_out_0.txt         3    audio out stereo
*/
const p_io_function_ctrl platform_io [LAST_IO_FUNCTION_PLATFORM] =
{
    (void *)&data_in_0,        // 0  ..windows/platform_manifest/io_platform_data_in_0.txt      
    (void *)&analog_sensor_0,  // 1  ..windows/platform_manifest/io_platform_analog_sensor_0.txt
    (void *)&motion_in_0,      // 2  ..windows/platform_manifest/io_platform_motion_in_0.txt    
    (void *)&audio_in_0,       // 3  ..windows/platform_manifest/io_platform_audio_in_0.txt     
    (void *)&d2_in_0,          // 4  ..windows/platform_manifest/io_platform_d2_in_0.txt        
    (void *)&line_out_0,       // 5  ..windows/platform_manifest/io_platform_line_out_0.txt     
    (void *)&gpio_out_0,       // 6  ..windows/platform_manifest/io_platform_gpio_out_0.txt     
    (void *)&gpio_out_1,       // 7  ..windows/platform_manifest/io_platform_gpio_out_1.txt     
    (void *)&data_out_0,       // 8  ..windows/platform_manifest/io_platform_data_out_0.txt     
};


const uint32_t graph_input[] = 
{ 
#include "graph_0_bin.txt"
};


/*
 *  TIME (see stream_time64 definition)
 *
 *  "stream_time64" example of implementation using a global variable
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s]  140 Y + Q28 [s]
 *  systick increment for  1ms =  0x00041893 =  1ms x 2^28
 *  systick increment for 10ms =  0x0028F5C2 = 10ms x 2^28
 * 
 * Other implementation rely on an HW timer (RP2040)
*/

uint64_t global_stream_time64;


/*
    Callback for SWC and scripts (CALS  system call (0..7) and application callbacks (8..15))
        and ARC debug activities (ARC_APP_CALLBACK1) 
    Use-case : 
        deep-sleep proposal from the scheduler
        event detection trigger, software timers 
        metadata sharing from script
        allow execution of arm_graph_interpreter(STREAM_RESET..) after a graph remote reload (check IOs are ok)
*/
const p_stream_al_services application_callbacks[MAX_NB_APP_CALLBACKS] =
{   (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0,
    (void*)0
};


//extern const p_stream_al_services al_services;

/* Forward declare al_services defined below. */
stream_al_services al_services;
const p_stream_al_services al_service[1] =
{   (void *)&al_services
};


/**
  @brief         Translation of physical address to packed address in 27-bits
  @param[in]     instance   Global registers of the current instance
  @param[in]     address    Address to be translated to 
  @param[in]     data3      3rd parameter integer
  @return        none

  @par           Checks which of the 8 memory bank is suitable for the address
                 Translate the offset to te base address in an integer and shift
  @remark
 */

/*----------------------------------------------------------------------------
    convert a physical address to a portable multiprocessor address 
 *----------------------------------------------------------------------------*/
uint32_t lin2pack (arm_stream_instance_t *S, uint8_t *buffer)
{
    sintPtr_t distance;
    uint8_t i;

    /* packed address range is [ long_offset[IDX]  +/- 8MB ]*/
#define MAX_PACK_ADDR_RANGE (((1<<(BASEIDX_ARCW0_MSB - BASEIDX_ARCW0_LSB+1))-1))

    /* find the base offset */

    for (i = 0; i < (uint8_t)MAX_NB_MEMORY_OFFSET; i++)
    {
        distance = (sintPtr_t)buffer - (sintPtr_t)(S->long_offset[i]);
        if (ABS(distance) < MAX_PACK_ADDR_RANGE) 
        {   break;
        }
    }

    return (uint32_t)(distance | ((uint64_t)i << (uint8_t)DATAOFF_ARCW0_LSB));
}

/*----------------------------------------------------------*/
static intPtr_t pack2linaddr_int(const intPtr_t *long_offset, uint32_t x)
{
    intPtr_t dbg1, dbg2, dbg3;

    dbg1 = long_offset[RD(x,DATAOFF_ARCW0)];    
    dbg2 = (intPtr_t)(BASEINWORD32 * (intPtr_t)RD((x),BASEIDX_ARCW0));

    if (RD(x,BAS_SIGN_ARCW0)) 
        dbg3 = dbg1 + ~(dbg2) +1;   // dbg1-dbg2 using unsigned integers
     else
        dbg3 = dbg1 + dbg2;


//#define PACK2LINADDR(o,x) (o[RD(x,DATAOFF_ARCW0)] + 
//        (RD(x,BAS_SIGN_ARCW0))? 
//            (1 + ~(((intPtr_t)RD((x),BASEIDX_ARCW0))<<LOG2BASEINWORD32)):
//            ( ((intPtr_t)RD((x),BASEIDX_ARCW0))<<LOG2BASEINWORD32))


    return dbg3;   // PACK2LINADDR(long_offset,x);
}

void * pack2linaddr_ptr(const intPtr_t *long_offset, uint32_t data)
{
    return (void *) (pack2linaddr_int(long_offset, data));
}

/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int

  @par              initialize a Graph interpreter  instance : share the address of the graph and
                    and the index of this instance within the application (other processors
                    or RTOS processes are using different index). 
  @remark
 */

void platform_init_stream_instance(arm_stream_instance_t *instance)
{

extern p_stream_node node_entry_point_table[];
extern void platform_specific_long_offset(intPtr_t long_offset[]);
extern const uint32_t graph_input[];
extern const p_stream_al_services application_callbacks[];


#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 1

    /* reset instance */
    memset (instance, 0, sizeof(arm_stream_instance_t));

    instance->graph = (uint32_t *) &(graph_input[1]);

    instance->scheduler_control = PACK_STREAM_PARAM(
            STREAM_MAIN_INSTANCE,
            STREAM_NB_INSTANCE,
            STREAM_COLD_BOOT,
            STREAM_SCHD_NO_SCRIPT, 
            STREAM_SCHD_RET_END_ALL_PARSED
            );

    ST(instance->whoami_ports, ARCHID_PARCH, ARCH_ID);  /* 3 fields used for SWC locking */
    ST(instance->whoami_ports, PROCID_PARCH, PROC_ID);
    ST(instance->whoami_ports, PRIORITY_PARCH, STREAM_INSTANCE_LOWLATENCYTASKS);
    
    instance->iomask = 0x03FF;                      /* 10 IOs */
    instance->ioctrl= &(platform_iocontrol[0]);     /* list of bytes holding the status of the on-going data moves (MP) */

    instance->al_services = al_service;
    instance->application_callbacks = application_callbacks;
    instance->platform_io = platform_io;
    instance->node_entry_point_table = node_entry_point_table;
    instance->long_offset = long_offset;
}



/**
  @brief           
  @param[in/out]    
  @return           
  @par              
  @remark
 */

 
/**
  @brief        Graph copy in RAM (STREAM_RESET phase)
  @param[in]    instance   global data of this instance
  @param[in]    graph      input graph
  @return       none

  @par          Initialization of the Graph interpreter instance 
                   long offsets, address of nodes, arcs, stream formats

                The Graph is a table using uint32 :
                - size of the graph in Words
                [0] 27b RAM address of part/all the graph, HW-shared MEM configuration, which part is going in RAM
                [1] number of FORMAT, size of SCRIPTS
                [2] size of LINKEDLIST, number of STREAM_INSTANCES
                [3] number of ARCS, number of DEBUG registers

                The application shares a pointer, the register [0] tells the graph is either:
                - already in RAM : nothing to do
                - to move in RAM (internal / external) partially or totally
                
                platform_init_copy_graph() implements this data move and calls platform AL to synchronize the other processor 
                to let them initialize the node instances in parallel.
  @remark
 */
/*---------------------------------------------------------------------------
 * Application execution
 *---------------------------------------------------------------------------*/
void platform_init_copy_graph(arm_stream_instance_t *S)
{
    uint8_t RAMsplit;
    uint32_t graph_words_in_ram, graph_size_word32, offsetWords;
    uint32_t *graph_src, *graph0;
    uint32_t *graph_dst;

    /* 
        start copying the graph in RAM 
        initialize the current Graph interpreter instance (->node_entry_points, ->graph)
        initialize the IO boundaries of the graph
     realign "graph0" */
    graph_size_word32 = S->graph[-1];
    graph0 = S->graph;

    /* default : 
        COPY_CONF_GR0_COPY_ALL_IN_RAM    graph0 in flash -> GRAPH_RAM_OFFSET_PTR()
        COPY_CONF_GR0_ALREADY_IN_RAM     graph0 in RAM   -> GRAPH_RAM_OFFSET_PTR()  
     */
    RAMsplit = (uint8_t)RD((graph0[0]),RAMSPLIT_GR0);
    graph_words_in_ram = graph_size_word32;
    graph_src = graph0;
    graph_dst = S->graph = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,graph0);

    /* Graph is read from Flash and a small part is copied in RAM
       to save RAM (for example Cortex-M0 with 2kB of internal SRAM)

                   RAMsplit Option 0/3    1     2 
        IO x2 words                RAM   Flash Flash  
        FORMAT x3 words            RAM   Flash Flash  
        SCRIPTS                    RAM   Flash Flash  
        LINKED-LIST                RAM   RAM   Flash   RAM allows SWC to be desactivated
        ARC descriptors 4 words    RAM   RAM   RAM    
        Debug registers, Buffers   RAM   RAM   RAM    
    */

    offsetWords = GRAPH_HEADER_NBWORDS;
    offsetWords += (4 * RD((S->graph)[3], NB16IOSTREAM_GR3));   /* IO bit-fields */

    switch (RAMsplit)
    {  
    /*
        Graph is already in RAM, No data move, initialize the instance 
    */
    default:
    case COPY_CONF_GR0_COPY_ALL_IN_RAM:
    case COPY_CONF_GR0_ALREADY_IN_RAM:
        S->pio = &(graph_dst[offsetWords]);   /* pio / io_al_idx */

        offsetWords += (RD(graph0[1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32);
        S->all_formats = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32);
        S->script_offsets = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[1], SCRIPTS_SIZE_GR1));
        S->linked_list = &(graph_dst[offsetWords]);

        offsetWords += (RD(graph0[2], LINKEDLIST_SIZE_GR2));
        S->all_arcs = &(graph_dst[offsetWords]);
        break;

    case COPY_CONF_GR0_FROM_ARC_DESCS:   /* option 2 */
        S->pio = &(graph0[offsetWords]); 

        offsetWords += (RD(graph0[1], NB_IOS_GR1) * STREAM_IOFMT_SIZE_W32);
        S->all_formats = &(graph0[offsetWords]);

        offsetWords += (RD(graph0[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32);
        S->script_offsets = &(graph0[offsetWords]);

        S->graph = graph0;                  /* graph base address is in Flash */
        graph_dst = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,G);

        offsetWords += (RD(graph0[1], SCRIPTS_SIZE_GR1));
        S->linked_list = &(graph0[offsetWords]);/*  */

        offsetWords += (RD(graph0[2], LINKEDLIST_SIZE_GR2));
        graph_src = &(graph0[offsetWords]);     /* linked list in Flash */
        S->all_arcs = graph_dst;                /* arc in RAM */
        graph_words_in_ram -= offsetWords;      /* copy from arcs */
        
        break;
    } /* switch (RAMsplit) */

    /* finalize the copy by telling the other processors */   
    {   uint32_t i; 
        for (i=0;i<graph_words_in_ram;i++)
        {   graph_dst[i]=graph_src[i];
        }
    }    


    /*          depends on the platform
        platform AL decide to set the RAM Sshared 
        set MPU_RASR[18] = 1 
     */


    /* copy the graph data to uint8_t platform_io_al_idx_to_graph[LAST_IO_FUNCTION_PLATFORM]; 
        to ease the translation from graph index to graph_io_idx used in arm_graph_interpreter_io_ack()
    */
    {   int i_graph_io_idx, n;
        uint8_t *graph_io, *graph_io_RAM, io_byte;

        n = 16 * RD(graph0[3], NB16IOSTREAM_GR3);           /* 16 IO bytes */
        graph_io = (uint8_t *) &(graph0[GRAPH_HEADER_NBWORDS]); /* list of IO bytes */
        graph_io_RAM = (uint8_t *)(S->all_arcs);            /* S-> all_arcs[0] */

        for (i_graph_io_idx = 0; i_graph_io_idx < n; i_graph_io_idx++)
        {   io_byte = *graph_io++;
            platform_io_al_idx_to_graph[RD(io_byte, IOALIDX_IOCTRL)] = i_graph_io_idx;
            *graph_io_RAM++ = io_byte;
        }
    }
}


/**
  @brief        Initialization and start of the IOs 
  @param[in]    instance   global data of this instance
  @return       none

  @par          Read the IOs descriptor the instance is in charge of
                Interface to platform-specific stream controls (set, start)
  @remark
 */
void platform_init_io(arm_stream_instance_t *S)
{
    uint32_t nio;
    uint8_t graph_io_idx;
    uint32_t *pio, *pio_base;
    uint32_t stream_format_io_setting;
    uint32_t io_mask;
    uint8_t *ioctrl;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    const p_io_function_ctrl *io_func;
    const p_stream_al_services *al_func;

    extern arm_stream_instance_t * platform_io_callback_parameter;
    /*-------------------------------------------*/

    /* if cold start : clear the backup area */
    if (TEST_BIT(S->scheduler_control, BOOT_SCTRL_LSB) == STREAM_COLD_BOOT)
    {   al_func = &(S->al_services[0]);
        (*al_func)(PACK_AL_SERVICE(0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
    }

    /* wait all the process have initialized the graph */
    {   //uint8_t wait; 
        //do {
        //    platform_al (PLATFORM_MP_RESET_WAIT, &wait, 0, RD(S->scheduler_control, NBINSTAN_SCTRL));
        //} while (wait == 0);
    }

    /*-------------------------------------------*/
    /* 
        initialization of the graph IO ports 
    */     
    io_mask = S->iomask;
    pio_base = S->pio;
    nio = RD(S->graph[1],NB_IOS_GR1);
    all_arcs = S->all_arcs;


    /* the Graph interpreter instance holds the memory pointers */
    platform_io_callback_parameter = S;

    for (graph_io_idx = 0; graph_io_idx < nio; graph_io_idx++)
    {
        uint8_t *address;
        uint32_t size;

        pio = &(pio_base[graph_io_idx * STREAM_IOFMT_SIZE_W32]);

        /* does this port is managed by the Graph interpreter instance ? */
        if (0 == (io_mask & (1U << graph_io_idx))) 
            continue; 

        /* no init for the unset IO */
        if (0 != TEST_BIT(*pio, QUICKSKIP_IOFMT_LSB))
            continue;

        ioctrl = &(S->ioctrl[graph_io_idx]);

        /* default value settings */
        stream_format_io_setting = pio[1];
        io_func = &(S->platform_io[RD(ioctrl[graph_io_idx], IOALIDX_IOCTRL)]);
        if (*io_func == 0) 
        {   continue;
        }
        (*io_func)(STREAM_RESET, (uint8_t *)&stream_format_io_setting, 0);

        /* 
            IO-Interface expects the buffer to be declared by the graph 
        */
        if (0 == TEST_BIT(*pio, SHAREBUFF_IOFMT_LSB))
        {
            iarc = RD(*pio, IOARCID_IOFMT);
            iarc = SIZEOF_ARCDESC_W32 * iarc;
            address = (uint8_t *)pack2linaddr_ptr(S->long_offset, all_arcs[iarc + BUF_PTR_ARCW0]);
            size = (uint32_t)RD(all_arcs[iarc + BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);

            io_func = &(S->platform_io[RD(ioctrl[graph_io_idx], IOALIDX_IOCTRL)]);
            (*io_func)(STREAM_SET_BUFFER, address, size);
        }
    } 
}



#define DATA_MEMORY_BARRIER
#define INSTRUCTION_SYNC_BARRIER


/**
  @brief        time functions
  @param[in]    none
  @return       none

  @par          

  @remark       
 */
void al_service_time_functions (uint32_t service_command, uint8_t *pt8b, uint8_t *data, uint8_t *flag, uint32_t n)
{   
    volatile uint8_t *pt8 = pt8b;

    switch (service_command)
    {
        case AL_SERVICE_READ_TIME64:
        {
            break;
        }
    }
}

/**
  @brief        
  @param[in]    none
  @return       none

  @par          Usage:
            al_service_mutual_exclusion (AL_SERVICE_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP, 
                S->pt8b_collision_arc, (*data) &check, &process_ID, 0);
                
  @remark       
 */
void al_service_mutual_exclusion(uint32_t service_command, uint8_t *pt8b, uint8_t *data, uint8_t *flag, uint32_t n)
{   
    volatile uint8_t *pt8 = pt8b;

    switch (service_command)
    {
        case AL_SERVICE_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP:
        {
            /* attempt to reserve the node */
            *pt8 = *data;    

            /* check collision with all the running processes
              using the equivalent of lock() and unlock() 
              Oyama Lock, "Towards more scalable mutual exclusion for multicore architectures"
                by Jean-Pierre Lozi */
            //  INSTRUCTION_SYNC_BARRIER;
            //  /* check mutual-exclusion */
            //  DATA_MEMORY_BARRIER;

            *data = (*pt8 == *flag);
            break;
        }

        case AL_SERVICE_MUTUAL_EXCLUSION_WR_BYTE_MP:
        {   *(volatile uint8_t *)(pt8) = (*data); 
            DATA_MEMORY_BARRIER; 
            break;
        }

        default:
        case AL_SERVICE_MUTUAL_EXCLUSION_RD_BYTE_MP:
        {   DATA_MEMORY_BARRIER; 
            (*data) = *(volatile uint8_t *)(pt8);
            break;
        }

        case AL_SERVICE_MUTUAL_EXCLUSION_CLEAR_BIT_MP:
        {   ((*pt8b) = U(*pt8b) & U(~(U(1) << U(n)))); 
            DATA_MEMORY_BARRIER;
            break;
        }
    }
}

void al_services (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n)
{   
    //arm_stream_instance_t *pinst;

    /* max 16 groups of commands */
	switch (RD(service_command, GROUP_AL_SRV))
    {
    case AL_SERVICE_READ_TIME:
        al_service_time_functions(RD(service_command, FUNCTION_SSRV), ptr1, ptr2, ptr3, n);
        break;
    case AL_SERVICE_SLEEP_CONTROL:
        break;
    case AL_SERVICE_READ_MEMORY:
        break;
    case AL_SERVICE_SERIAL_COMMUNICATION:
        break;
    //enum stream_service_group
    case AL_SERVICE_MUTUAL_EXCLUSION:
        al_service_mutual_exclusion(RD(service_command, FUNCTION_SSRV), ptr1, ptr2, ptr3, n);
        break;
    case AL_SERVICE_CHANGE_IO_SETTING:
        break;
    }
}


#if MULTIPROCESSING != 0
#if 0
/**
  @brief        Memory banks initialization
  @param[in]    none
  @return       none

  @par          Loads the global variable of the platform holding the base addresses 
                to the physical memory banks described in the "platform manifest"

  @remark       
 */
uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   volatile uint8_t *pt8;
    pt8 = pt8b;

    *pt8 = code;
    INSTRUCTION_SYNC_BARRIER;

    /* no need to use LDREX, don't wait and escape if collision occurs */
    DATA_MEMORY_BARRIER;

    return (*pt8 == code);
}

#else

uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   return 1u;
}
#endif
#endif



/*
 * -----------------------------------------------------------------------
 */
