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

#ifndef cSTREAM_TYPES_H
#define cSTREAM_TYPES_H

/* 
    if 64 bits architectures are reading the graph:
    #define intPtr_t uint64_t
    #define MAX_ADD_OFFSET 0x7FFFFFFFFFFFFFFFL
*/
#ifdef PLATFORM_ARCH_32BIT
#define intPtr_t uint32_t 
#else
#define intPtr_t uint64_t 
#endif

#define MAX_ADD_OFFSET 0x7FFFFFFFL


struct data_buffer
{   intPtr_t address; 
    intPtr_t size;
};

typedef struct data_buffer data_buffer_t;

/*==================================================== SWC  ======================================================================*/
/* 
 * command ; *instance pointer ; two pointers to a temporary area holding : 
 *      (*,input data size)  x N_stream_input
 *      (*,output free_area) x N_stream_output
 *    the SWC updates the (*,n) fields before returning, to  
 * 
 */

typedef void (stream_entrance) (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3); 
typedef void (*p_stream_node) (uint32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *state);
typedef void (stream_node) (uint32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *state);

//typedef uint32_t (stream_node) (uint32_t command, void * ptr1, void * ptr2, void * ptr3); 

//typedef void (*io_callback_ptr)      (uint32_t idx, uint8_t *data, uint32_t length);   
typedef uint8_t (*io_function_control_ptr)  (uint32_t *setting, uint8_t *data, uint32_t length);  
typedef uint8_t (io_function_control) (uint32_t *setting, uint8_t *data, uint32_t length);  


//------------------------------------------------------------------------------------------------------
/*  Time format - 64 bits
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  00ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq seconds and q14 seconds from reset (resolution ~50us)
 *  10ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq seconds and q14 seconds from 1st of January, 1970 UTC (overflow in 2108)
 *  systick increment of 1ms =      00000000000100000110001001001101 0x0010624D = 1ms/4 x 2^32 = 0.275 ppm resolution, 10 seconds/year
 *  systick increment of 10ms =     00000000101000111101011100001010 0x00A3D70A = 10ms/4 x 2^32
 */

 /* at each systick ISR the time64 is incremented by 1ms x 2^32 = fraction increment */
typedef uint64_t stream_time64;
#define ABSOLUTE_TIM_MSB 63
#define ABSOLUTE_TIM_LSB 63 /* 0: from reset, 1: from Jan-1st-2000 */
#define   UNUSED_TIM_MSB 62
#define   UNUSED_TIM_LSB 62 
#define   SECOND_TIM_MSB 61
#define   SECOND_TIM_LSB 32
#define    FRACT_TIM_MSB 31 
#define    FRACT_TIM_LSB  0

#define LINK_NB_SOFTWARE_TIMERS 1

/*
 * stream_time_seconds in 32bits :
 *  FEDCBA987654321 FEDCBA9876543210
 *  00sssssssssssssssssqqqqqqqqqqqqq seconds and q13 seconds (8h +/-50us)
 *  10ssssssssssssssssssssssssssssqq seconds and q2 seconds 
 * 
 * bits 31-30 = format   0: seconds from reset, 1:format U14.16 for IMU time-stamps (4h30mn)
 * bit  29- 0 = time     (30bits seconds = 68years)
 */
typedef uint32_t stream_time32;
#define ABSOLUTE_T32_MSB 31          
#define ABSOLUTE_T32_LSB 31 /* 0: from reset, 1: from link_local_instance reftime32 (usually set to Jan-1st-2022) */
#define   FORMAT_T32_MSB 30
#define   FORMAT_T32_LSB 30 /* 0: Q28.2 (1/4 second) (28bits=8.5years)  1: U18.12 (0.250us) (18bits=3days)  */     
#define     TIME_T32_MSB 29
#define     TIME_T32_LSB  0 


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
#define PACK_PLATFORM_DOMAIN(INST,DOMAIN) (((INST)<<6)|(DOMAIN))

/* 
    enum stream_io_domain : list of stream "domains" categories 
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).

    a change in this list 
*/
#define PLATFORM_DATA_IN                1
#define PLATFORM_DATA_OUT               2
#define PLATFORM_DATA_STREAM_IN         3
#define PLATFORM_DATA_STREAM_OUT        4
#define PLATFORM_AUDIO_IN               5
#define PLATFORM_AUDIO_OUT              6
#define PLATFORM_GPIO_IN                7
#define PLATFORM_GPIO_OUT               8
#define PLATFORM_MOTION_IN              9
#define PLATFORM_2D_IN                 10
#define PLATFORM_2D_OUT                11  
#define PLATFORM_USER_INTERFACE_IN     12 
#define PLATFORM_USER_INTERFACE_OUT    13 
#define PLATFORM_COMMAND_IN            14
#define PLATFORM_COMMAND_OUT           15
#define PLATFORM_LOW_DATA_RATE_IN      16 
#define PLATFORM_LOW_DATA_RATE_OUT     17 
#define PLATFORM_RTC_IN                18 
#define PLATFORM_RTC_OUT               19
#define PLATFORM_STORAGE_OUT           20 
#define PLATFORM_MAX_NB_DOMAINS        (PLATFORM_STORAGE_OUT+1)

/*
    each stream is controled by 3 functions and presets
    domain presets are defined with bitfields (see example platform_audio_out_bit_fields[]).
*/
struct platform_io_control           
{   io_function_control *io_set;
    io_function_control *io_start;
    io_function_control *io_stop;
    const int32_t *stream_setting;
};

enum 
{   arc_read_address,
    arc_write_address,
    arc_data_amount,
    arc_free_area,
    arc_buffer_size,
    arc_IO_move_to_arc,
    arc_IO_moved_from_arc,
    arc_IO_moved_from_arc_update,
    arc_set_base_address_to_arc,
    arc_set_base_address_from_arc,
    arc_data_realignment_to_base,
    consumer_frame_size,
    producer_frame_size
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
    STREAM_FP8_E4M3,               /*   Seeeemmm  tiny-float [0.02 .. 448] */
    STREAM_FP8_E5M2,               /*   Seeeeemm  [0.0001 .. 57344] for NN training */
                                     
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
#define STREAM_SCHD_RET_END_EACH_SWC        S8(1)   /* return to caller after each SWC calls */
#define STREAM_SCHD_RET_END_ALL_PARSED      S8(2)   /* return to caller once all SWC are parsed */
#define STREAM_SCHD_RET_END_SWC_NODATA      S8(3)   /* return to caller when all SWC are starving */
                            
#define STREAM_SCHD_NO_SCRIPT               S8(0)
#define STREAM_SCHD_SCRIPT_BEFORE_EACH_SWC  S8(1)   /* script is called after each SWC called */
#define STREAM_SCHD_SCRIPT_AFTER_EACH_SWC   S8(2)   /* script is called after each SWC called */
#define STREAM_SCHD_SCRIPT_END_PARSING      S8(4)   /* script is called at the end of the loop */
#define STREAM_SCHD_SCRIPT_START            S8(8)   /* script is called when starting */
#define STREAM_SCHD_SCRIPT_END              S8(16)  /* script is called before return */
//};

/*
    stream instance memory
*/
#define STREAM_COLD_BOOT 0u
#define STREAM_WARM_BOOT 1u

#define   UNUSED_SCTRL_MSB U(31)   
#define   UNUSED_SCTRL_LSB U(24)  /* 8 */
#define ENDLLIST_SCTRL_MSB U(23)   
#define ENDLLIST_SCTRL_LSB U(23)  /* 1 endLinkedList detected */
#define STILDATA_SCTRL_MSB U(22)   
#define STILDATA_SCTRL_LSB U(22)  /* 1 still some_components_have_data*/
#define NBINSTAN_SCTRL_MSB U(21)   
#define NBINSTAN_SCTRL_LSB U(16)  /* 6 */
#define     BOOT_SCTRL_MSB U(15)   
#define     BOOT_SCTRL_LSB U(15)  /* 1 cold0/warm1 boot */
#define   SCRIPT_SCTRL_MSB U(14)   
#define   SCRIPT_SCTRL_LSB U( 9)  /* 6 script call options bit-field (before/after SWC/loop/full) */
#define   RETURN_SCTRL_MSB U( 8)
#define   RETURN_SCTRL_LSB U( 6)  /* 3 return options (each SWC, each parse, once starving */
#define INSTANCE_SCTRL_MSB U( 5)
#define INSTANCE_SCTRL_LSB U( 0)  /* 6 instance index */
#define PACK_STREAM_PARAM(N,B,S,R,I) (      \
            ((N)<<NBINSTAN_SCTRL_LSB) |     \
            ((B)<<BOOT_SCTRL_LSB) |         \
            ((S)<<SCRIPT_SCTRL_LSB) |       \
            ((R)<<RETURN_SCTRL_LSB) |       \
             (I))



typedef struct
{   stream_entrance *stream_entry;
    uint32_t *registers;
    uint32_t *stack;
    uint32_t *code;
} arm_script_instance;


typedef struct  
{   /* stream instance overlay */
    intPtr_t *long_offset;      
    uint32_t *graph;
    uint32_t *pio;
    uint32_t *all_formats;   
    uint32_t *scripts;   
    uint32_t *linked_list;   
    uint32_t *pinst;         
    uint32_t *all_arcs;
} stream_instance_short_t;

typedef struct  
{  
    stream_instance_short_t S0;
    p_stream_node *all_nodes;
    uint32_t scheduler_control;     // PACK_STREAM_PARAM(..);

    p_stream_node address_swc;
    uint32_t *linked_list_ptr;      // current position of the linked-list read pointer

    uint32_t *swc_header;           // current swc
    intPtr_t swc_instance_addr;
    uint16_t arcID[MAX_NB_STREAM_PER_SWC];
    uint8_t *pt8b_collision_arc;
    uint32_t pack_command;          // preset, narc, tag, instanceID, command

    arm_script_instance *main_script;

    uint8_t swc_memory_banks_offset;// offset in words
    uint8_t swc_parameters_offset;

} arm_stream_instance_t;





struct stream_local_instance        /* structure allocated to each STREAM instance */
{     uint32_t whoami_ports;        /* PACKWHOAMI */
      uint32_t parameters;
      stream_node **node_entry_points;   /* all the nodes visible from this processor */
};



struct platform_proc_identification
{    /* who am I ? */
    uint16_t architecture;
    uint16_t arch_id;
    uint16_t proc_id;
};


struct platform_control_stream
{   //uint32_t *graph;        
    arm_stream_instance_t *instance;
    uint32_t domain_settings;
    uint8_t fw_idx;
    data_buffer_t buffer;    // [{uint8_t *; int}]
};


/* stream parameters */
//struct stream_control 
//{   const io_function_control_ptr io_set;     
//    const io_function_control_ptr io_start; 
//    const io_function_control_ptr io_stop; 
//};

extern p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS];

/* three entry points */
extern void arm_stream (uint32_t command,  arm_stream_instance_t *inst, uint32_t *data);
extern void arm_stream_io (uint32_t fw_io_idx, 
        arm_stream_instance_t *stream_instance,
        uint8_t *data, uint32_t length);
extern void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3);

extern void stream_scan_graph (arm_stream_instance_t *stream_instance, int8_t reset_option);

/* ---- REFERENCES --------------------------------------------*/

extern void script_processing (arm_script_instance *instance);
extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3);
extern uint32_t physical_to_offset (arm_stream_instance_t *stream_instance, uint8_t *buffer);
extern void arc_data_operations (arm_stream_instance_t *stream_instance, uint32_t *arc, uint8_t tag, uint8_t *buffer, uint32_t size);

#endif /* #ifndef cSTREAM_TYPES_H */

#ifdef __cplusplus
}
#endif