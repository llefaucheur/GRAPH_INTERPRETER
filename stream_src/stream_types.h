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

#ifndef cSTREAM_TYPES__H
#define cSTREAM_TYPES__H

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>


#include "platform_al.h"


/* 
    if 64 bits architectures are reading the graph:
    #define intPtr_t uint64_t
    #define MAX_ADD_OFFSET 0x7FFFFFFFFFFFFFFFL
*/
#ifdef PLATFORM_ARCH_32BIT
#define intPtr_t uint32_t 
#define sintPtr_t int32_t 
#else
#define intPtr_t uint64_t 
#define sintPtr_t int64_t 
#endif


struct HW_params
{
    uint8_t procID;
    uint8_t archID;
    uint8_t *ioctrl;
    uint32_t iomask;
};

struct stream_xdmbuffer
{   
    intPtr_t address; 
    intPtr_t size;
};

typedef struct stream_xdmbuffer stream_xdmbuffer_t;

typedef uint32_t stream_service_command;

/*
    opaque access to the static area of the node 
*/
typedef void *stream_handle_t;  

/*==================================================== SWC  ======================================================================*/
/* 
 * command ; *instance pointer ; two pointers to a temporary area holding : 
 *      (*,input data size)  x N_stream_input
 *      (*,output free_area) x N_stream_output
 *    the SWC updates the (*,n) fields before returning, to  
 * 
 */

typedef void (stream_services_entry) (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n); 
typedef void (*p_stream_node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);
typedef void (stream_node)    (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);

//typedef uint32_t (stream_node) (uint32_t command, void * ptr1, void * ptr2, void * ptr3); 
//typedef void (*io_callback_ptr)      (uint32_t idx, uint8_t *data, uint32_t length);   

typedef uint8_t (*io_function_control_ptr)  (uint32_t *setting, uint8_t *data, uint32_t length);  
typedef uint8_t (io_function_control) (uint32_t *setting, uint8_t *data, uint32_t length);  

// callbacks used from scripts
typedef void (*p_stream_script_callback) (uint32_t command, intPtr_t* ptr1, intPtr_t* ptr2, uint32_t n);

//------------------------------------------------------------------------------------------------------
/*  Time format - 64 bits
 *  Local time in bit-fields : years/../millisecond, WWW=day of the week 
 *  (0=Sunday, 1=Monday..)
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  000______________________.YY.YY.YY.YY.MMM.DDDD.SSSSS.MM.MM.MM.WW
 * 
 *  q42.17 milliseconds from January 1st 1970 UTC (or internal AL reference)
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  001mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmqqqqqqqqqqqqqqqqq q42.17 ms (140 Y +/- 8ns)
 *  
 *  q42.17 seconds "stream_time64"
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  010ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.29 s (140 Y +/- 2ns)
 * 
 *  systick increment of 1ms =  0x00041893 = 1ms x 2^28 = 1.7 ppm resolution, 1minute/year
 *  In each systick ISR, time64 is incremented by 1ms x 2^32 = fraction increment
 */

typedef uint64_t stream_time64;
#define   TYPE_T64_MSB 63
#define   TYPE_T64_LSB 61 
#define SECOND_T64_MSB 60
#define SECOND_T64_LSB 29
#define  FRACT_T64_MSB 28 
#define  FRACT_T64_LSB  0

/*
 * stream_time_seconds in 32bits : "stream_time32"
 *  F_______________________________FEDCBA9876543210FEDCBA9876543210
 *  1_______________________________00ssssssssssssssqqqqqqqqqqqqqqqq q14.16 [s] (4hours 30mn, +/- 15us)
 *  1_______________________________01ssssssssssssssssssssssssssssqq q28.2  [s] (8 years +/- 0.25s)
 */
typedef uint32_t stream_time32;
#define   FORMAT_T32_MSB 31
#define   FORMAT_T32_LSB 30 
#define     TIME_T32_MSB 29
#define     TIME_T32_LSB  0 

/*
 * stream_time_seconds in 16bits : "stream_time16"
 *  F_______________________________F_______________FEDCBA9876543210
 *  1_______________________________1_______________qqqqqqqqqqqqqqqq q15 x 250ms  (1/4 s +/- 15us)
 *  1_______________________________1_______________ssssssssssssssqq q14.2 seconds (4 hours 30mn +/- 0.25s)
 */
typedef uint32_t stream_time16;
#define     TIME_T16_MSB 15
#define     TIME_T16_LSB  0 

/* 
    MANIFEST of IO functions and capabilities
    used to interpret stream_setting in platform_xx.h
*/
#define   UNUSED_PLATFORM_DOMAIN_MSB U(31)  
#define   UNUSED_PLATFORM_DOMAIN_LSB U(10)  /* 22   */
#define INSTANCE_PLATFORM_DOMAIN_MSB U( 9)  
#define INSTANCE_PLATFORM_DOMAIN_LSB U( 6)  /* 4  16 instances of the same domain */
#define          PLATFORM_DOMAIN_MSB U( 5)  
#define          PLATFORM_DOMAIN_LSB U( 0)  /* 6  64 different domains */
#define PACK_PLATFORM_DOMAIN(INST,DOMAIN) (((INST)<<INSTANCE_PLATFORM_DOMAIN_LSB)|(DOMAIN))

/* 
    STREAM_IO_DOMAIN(s)

    enum stream_io_domain : list of stream "domains" categories, max 64
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).
*/
#define IO_PLATFORM_DATA_IN                 1
#define IO_PLATFORM_DATA_OUT                2
#define IO_PLATFORM_DATA_STREAM_IN          3
#define IO_PLATFORM_DATA_STREAM_OUT         4
#define IO_PLATFORM_AUDIO_IN                5
#define IO_PLATFORM_AUDIO_OUT               6
#define IO_PLATFORM_GPIO_IN                 7
#define IO_PLATFORM_GPIO_OUT                8
#define IO_PLATFORM_MOTION_IN               9
#define IO_PLATFORM_2D_IN                  10
#define IO_PLATFORM_2D_OUT                 11  
#define IO_PLATFORM_USER_INTERFACE_IN      12  /* slider, rotary button */
#define IO_PLATFORM_USER_INTERFACE_OUT     13 
#define IO_PLATFORM_COMMAND_IN             14  /* UART/I2C serial interface */
#define IO_PLATFORM_COMMAND_OUT            15
#define IO_PLATFORM_ANALOG_SENSOR          16 
#define IO_PLATFORM_ANALOG_TRANSDUCER      17 
#define IO_PLATFORM_RTC_IN                 18 
#define IO_PLATFORM_RTC_OUT                19
#define IO_PLATFORM_STORAGE_OUT            20 
#define IO_PLATFORM_AV_CODEC               21 
#define IO_PLATFORM_UNUSED_1               22 
#define IO_PLATFORM_UNUSED_2               23
#define IO_PLATFORM_UNUSED_3               24
#define IO_PLATFORM_UNUSED_4               25
#define IO_PLATFORM_UNUSED_5               26
#define IO_PLATFORM_UNUSED_6               27
#define IO_PLATFORM_UNUSED_7               28
#define IO_PLATFORM_UNUSED_8               29
#define IO_PLATFORM_UNUSED_9               30
#define IO_PLATFORM_MAX_NB_DOMAINS         (IO_PLATFORM_UNUSED_9+1)


/*
    each stream is controled by 3 functions and presets
    domain presets are defined with bitfields (see example platform_audio_out_bit_fields[]).
*/
struct platform_io_control           
{   io_function_control *io_set;
    io_function_control *io_start;
    io_function_control *io_stop;
};
typedef struct platform_io_control platform_io_control_t;


enum 
{   arc_read_address,
    arc_write_address,
    arc_data_amount,
    arc_free_area,
    arc_buffer_size,
    arc_IO_move_to_arc,
    arc_IO_moved_from_arc,
    //arc_IO_moved_from_arc_update,
    //arc_set_base_address_to_arc,
    //arc_set_base_address_from_arc,
    arc_data_realignment_to_base,
    //consumer_frame_size,
    //producer_frame_size
};


 
/*============================ STREAM DATA/TYPE =================================*/
/* types fit in 6bits, arrays start with 0, stream_bitsize_of_raw() is identical */

enum stream_raw_data 
{   STREAM_DATA_ARRAY = 0,         /* 0 see stream_array: [0NNNTT00] 0, type, nb */

    /* one bit per data */
    STREAM_S1,                     /*   S, one signed bit, "0" = +1 */
    STREAM_U1,                     /*   one bit unsigned */
                                      
    /* two bits per data */           
    STREAM_S2,                     /*   SX  */
    STREAM_U2,                     /*   XX  */
    STREAM_Q1,                     /*   Sx ~stream_s2 with saturation management*/
                                      
    /* four bits per data */          
    STREAM_S4,                     /* 6 Sxxx  */
    STREAM_U4,                     /*   xxxx  */
    STREAM_Q3,                     /*   Sxxx  */
    STREAM_FP4_E2M1,               /*   Seem  micro-float [8 .. 64] */
    STREAM_FP4_E3M0,               /*   Seee  [8 .. 512] */
                                      
    /* eight bits per data */         
    STREAM_S8,                     /*11 Sxxxxxxx  */
    STREAM_U8,                     /*   xxxxxxxx  */
    STREAM_Q7,                     /*   Sxxxxxxx  */
    STREAM_CHAR,                   /*   xxxxxxxx  */
    STREAM_FP8_E4M3,               /*   Seeeemmm  NV tiny-float [0.02 .. 448] */
    STREAM_FP8_E5M2,               /*   Seeeeemm  IEEE-754 [0.0001 .. 57344] */
                                     
    /* 2 bytes per data */            
    STREAM_S16,                    /*   Sxxxxxxx.xxxxxxxx  */
    STREAM_U16,                    /*   xxxxxxxx.xxxxxxxx  */
    STREAM_Q15,                    /*19 Sxxxxxxx.xxxxxxxx  */
    STREAM_FP16,                   /*   Seeeeemm.mmmmmmmm  half-precision float */
    STREAM_BF16,                   /*   Seeeeeee.mmmmmmmm  bfloat */

    /* 3 bytes per data */
    STREAM_Q23,                    /*   Sxxxxxxx.xxxxxxxx.xxxxxxxx in a 24bits container ! */
                                      
    /* 4 bytes per data */            
    STREAM_Q23_32,                 /*   SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxxx  */
    STREAM_S32,                    /*   one long word  */
    STREAM_U32,                    /*   xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx  */
    STREAM_Q31,                    /*   Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx  */
    STREAM_FP32,                   /*27 Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm  float */
    STREAM_CQ15,                   /*   Sxxxxxxx.xxxxxxxx Sxxxxxxx.xxxxxxxx (I Q) */
    STREAM_CFP16,                  /*   Seeeeemm.mmmmmmmm Seeeeemm.mmmmmmmm (I Q)  */
                                      
    /* 8 bytes per data */            
    STREAM_S64,                    /*   long long */
    STREAM_U64,                    /*   unsigned  64 bits */
    STREAM_Q63,                    /*   Sxxxxxxx.xxxxxx ....... xxxxx.xxxxxxxx  */
    STREAM_CQ31,                   /*   Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxxxxx.xxxxxxxx.. */
    STREAM_FP64,                   /*   Seeeeeee.eeemmmmm.mmmmmmm ... double  */
    STREAM_CFP32,                  /*   Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm Seeeeeee.mmm.. (I Q)  */
                                      
    /* 16 bytes per data */           
    STREAM_FP128,                  /*   Seeeeeee.eeeeeeee.mmmmmmm ... quadruple precision */
    STREAM_CFP64,                  /*   fp64 fp64 (I Q)  */
                                      
    /* 32 bytes per data */           
    STREAM_FP256,                  /*   Seeeeeee.eeeeeeee.eeeeemm ... octuple precision  */
                                      
    /* 2D formats  */                 
    STREAM_YUV420P,                /*39 Luminance, Blue projection, Red projection, 6 bytes per 4 pixels, reordered */
    STREAM_YUV422P,                /*   8 bytes per 4 pixels, or 16bpp, Y0 Cb Y1 Cr (1 Cr & Cb sample per 2x1 Y samples) */
    STREAM_YUV444P,                /*   12 bytes per 4 pixels, or 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
    STREAM_CYM24,                  /*   cyan yellow magenta */
    STREAM_CYMK32,                 /*   cyan yellow magenta black */
    STREAM_RGB8,                   /*   RGB  3:3:2,  8bpp, (msb)2B 3G 3R(lsb) */
    STREAM_RGB16,                  /*   RGB  5:6:5, 16bpp, (msb)5R 6G 5B(lsb) */
    STREAM_RGBA16,                 /*   RGBA 4:4:4:4 32bpp (msb)4R */
    STREAM_RGB24,                  /*   BBGGRR 24bpp (msb)8B */
    STREAM_RGBA32,                 /*   BBGGRRAA 32bpp (msb)8B */
    STREAM_RGBA8888,               /*   AABBRRGG OpenGL/PNG format R=lsb A=MSB ("ABGR32" little endian) */
    STREAM_BW1B,                   /*   Y, 1bpp, 0 is black, 1 is white, ordered from the msb to the lsb.  */
    STREAM_BW1BINV,                /*   Y, 1bpp, 0 is white, 1 is black */
    STREAM_GREY8B,                 /*   Grey 8b */
    STREAM_GREY8BINV,              /*53 Grey 8b inverted 0 is white */
    
    LAST_RAW_TYPE  = 64         /* coded on 6bits RAW_FMT0_LSB */
};



//enum stream_scheduling_options
//{
#define STREAM_SCHD_RET_END_EACH_SWC        1    /* return to caller after each SWC calls */
#define STREAM_SCHD_RET_END_ALL_PARSED      2    /* return to caller once all SWC are parsed */
#define STREAM_SCHD_RET_END_SWC_NODATA      3    /* return to caller when all SWC are starving */
                                              
#define STREAM_SCHD_NO_SCRIPT               0 
#define STREAM_SCHD_SCRIPT_BEFORE_EACH_SWC  1    /* script is called after each SWC called */
#define STREAM_SCHD_SCRIPT_AFTER_EACH_SWC   2    /* script is called after each SWC called */
#define STREAM_SCHD_SCRIPT_END_PARSING      4    /* script is called at the end of the loop */
#define STREAM_SCHD_SCRIPT_START            8    /* script is called when starting */
#define STREAM_SCHD_SCRIPT_END              16   /* script is called before return */
#define STREAM_SCHD_SCRIPT_UNUSED           32   /* 6bits are reserved in SCRIPT_SCTRL */
//};

/*
    stream instance memory
*/


/* "registers / computing stack" of the Stream instance */
typedef struct  
{  
    intPtr_t *long_offset;          /* pointer to "intPtr_t long_offset[MAX_NB_MEMORY_OFFSET];" */
    uint32_t *graph;
    uint32_t *pio;
    uint32_t *all_formats;   
    uint32_t *all_arcs;
    uint32_t *linked_list;   
    uint32_t *main_script;   
    struct platform_io_control *platform_io;

    p_stream_script_callback* application_callbacks;

    p_stream_node address_swc;
    uint32_t *linked_list_ptr;      // current position of the linked-list read pointer

    uint32_t *swc_header;           // current swc
    stream_handle_t swc_instance_addr;
    uint16_t arcID[MAX_NB_STREAM_PER_SWC];
    uint8_t *pt8b_collision_arc;
    uint32_t pack_command;          // preset, narc, tag, instanceID, command

    //uint32_t *stack_services;       // memory reserved for stream_services.c
    //uint32_t stack_services_size;

    uint8_t swc_memory_banks_offset;// offset in words
    uint8_t swc_parameters_offset;

    uint8_t nb_stream_instances;    // stream instances pointers (in words) = &(all_arcs[ -nb_stream_instances])

/* */
#define STREAM_COLD_BOOT 0u
#define STREAM_WARM_BOOT 1u         /* Reset + restore memory banks from retention */

#define STREAM_ANY_PRIORITY 0u      /* PRIORITY_SCTRL */
#define STREAM_LOWLATENCYTASKS 1u
#define STREAM_MIDLATENCYTASKS 2u
#define STREAM_BACKGROUNDTASKS 3u

#define STREAM_MAIN_INSTANCE 1


#define   UNUSED_SCTRL_MSB U(31)   
#define   UNUSED_SCTRL_LSB U(25)  /* 7 */ 
#define PRIORITY_SCTRL_MSB U(24)   
#define PRIORITY_SCTRL_LSB U(23)  /* 2 */ 
#define MAININST_SCTRL_MSB U(22)   
#define MAININST_SCTRL_LSB U(22)  /* 1 main instance to set the graph at boot time */
#define NODEEXEC_SCTRL_MSB U(21)   
#define NODEEXEC_SCTRL_LSB U(20)  /* 2 */
#define ENDLLIST_SCTRL_MSB U(19)   
#define ENDLLIST_SCTRL_LSB U(19)  /* 1 endLinkedList detected */
#define STILDATA_SCTRL_MSB U(18)   
#define STILDATA_SCTRL_LSB U(18)  /* 1 still some_components_have_data*/
#define NBINSTAN_SCTRL_MSB U(17)   
#define NBINSTAN_SCTRL_LSB U(14)  /* 4 used at boot time to reset the synchronization flags */
#define     BOOT_SCTRL_MSB U(13)   
#define     BOOT_SCTRL_LSB U(13)  /* 1 cold0/warm1 boot : Reset + restore memory banks from retention */
#define   SCRIPT_SCTRL_MSB U(12)   
#define   SCRIPT_SCTRL_LSB U( 7)  /* 6 script call options bit-field (before/after SWC/loop/full) */
#define   RETURN_SCTRL_MSB U( 6)
#define   RETURN_SCTRL_LSB U( 4)  /* 3 return options (each SWC, each parse, once starving */
#define INSTANCE_SCTRL_MSB U( 3)
#define INSTANCE_SCTRL_LSB U( 0)  /* 4 instance index */
#define PACK_STREAM_PARAM(M,P,N,B,S,R,I) (  \
            ((M)<<MAININST_SCTRL_LSB) |     \
            ((P)<<PRIORITY_SCTRL_LSB) |     \
            ((N)<<NBINSTAN_SCTRL_LSB) |     \
            ((B)<<    BOOT_SCTRL_LSB) |     \
            ((S)<<  SCRIPT_SCTRL_LSB) |     \
            ((R)<<  RETURN_SCTRL_LSB) |     \
             (I))
    uint32_t scheduler_control;     // PACK_STREAM_PARAM(..);

    /* identification "whoami", next SWC to run*/
    #define    INST_ID_PARCH_MSB U(31)
    #define     WHOAMI_PARCH_MSB U(31)
    #define   INSTANCE_PARCH_MSB U(31)  /* avoid locking an arc by the same processor, but different RTOS instances*/
    #define   INSTANCE_PARCH_LSB U(30)  /* 2 [0..3] up to 4 instances per processors, 0=main instance at boot */
    #define     PROCID_PARCH_MSB U(29)  /*   indexes from Manifest(tools) and PLATFORM_PROC_HW */
    #define     PROCID_PARCH_LSB U(27)  /* 3 processor index [0..7] for this architecture 0="commander processor" */  
    #define     ARCHID_PARCH_MSB U(26)
    #define     ARCHID_PARCH_LSB U(24)  /* 3 [1..7] processor architectures 1="commander processor architecture" */
    #define     WHOAMI_PARCH_LSB U(24)  /*   whoami used to lock a SWC to specific processor or architecture */
    #define    INST_ID_PARCH_LSB U(24)  /*   8 bits identification for locks */
    #define SWC_W32OFF_PARCH_MSB U(23)  
    #define SWC_W32OFF_PARCH_LSB U( 0) /* 24   offset in words to the NEXT SWC to be executed */  

    /* maximum number of processors = nb_proc x nb_arch */
    #define MAX_GRAPH_NB_PROCESSORS ((1<<(PROCID_PARCH_MSB-PROCID_PARCH_LSB+1))*(1<<(ARCHID_PARCH_MSB-ARCHID_PARCH_LSB+1)))

    uint32_t whoami_ports;          /* _PARCH_ fields */

    /* list of bytes holding the status of the on-going data moves (MP) */
    #define MAX_GRAPH_NB_IO_STREAM 32
    #define ONGOING_IOCTRL_MSB U( 0)  
    #define ONGOING_IOCRTL_LSB U( 0)   /* on-going request on IO */

    uint8_t *ioctrl;                    /* byte array of request fields */

    /* word 2 IO streams to scan , max = 32  = 1 << (NB_IOS_GR1_MSB - NB_IOS_GR1_LSB + 1) */
    #define MAX_GRAPH_NB_IO_STREAM 32
    #define BOUNDARY_IOMASK_MSB U(31)  
    #define BOUNDARY_IOMASK_LSB U( 0)   /* 32 boundary ports in STREAM_FORMAT_IO to scan */ 

    uint32_t iomask;                    /* _IOMASK_ fields */

    /* word 1 trace arc */
    #define   __________PARINST_MSB U(31)
    #define   __________PARINST_LSB U(12) /* 20 */
    #define INSTANCE_ON_PARINST_MSB U(11)
    #define INSTANCE_ON_PARINST_LSB U(11) /*  1 flag : 0=instances executed on this processor are disabled  */
    #define   TRACE_ARC_PARINST_MSB U(10)
    #define   TRACE_ARC_PARINST_LSB U( 0) /* 11 (ARC0_LW1 / NB_ARCS_GR3 2K arcs) index of the arc used for debug trace / instance */

    uint32_t parameters;                /* _PARINST_ fields */

} arm_stream_instance_t;





//struct stream_local_instance        /* structure allocated to each STREAM instance */
//{     uint32_t whoami_ports;        /* PACKWHOAMI */
//      uint32_t parameters;
//      stream_node **node_entry_points;   /* all the nodes visible from this processor */
//};



//struct platform_proc_identification
//{    /* who am I ? */
//    uint16_t architecture;
//    uint16_t arch_id;
//    uint16_t proc_id;
//};


//struct platform_control_stream
//{   //uint32_t *graph;        
//    arm_stream_instance_t *instance;
//    uint32_t domain_settings;
//    uint8_t fw_io_idx;
//    stream_xdmbuffer_t buffer;    // [{uint8_t *; int}]
//};


#ifdef __cplusplus
}
#endif
#endif /* #ifndef cSTREAM_TYPES_H */

