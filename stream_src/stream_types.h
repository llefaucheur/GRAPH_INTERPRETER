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

typedef struct {

    intPtr_t address;
    intPtr_t size;

} data_buffer_t;


//typedef uint32_t (stream_node) (uint32_t command, void * ptr1, void * ptr2, void * ptr3); 

//typedef void (*io_callback_ptr)      (uint32_t idx, uint8_t *data, uint32_t length);   
typedef uint32_t (*io_function_control_ptr)  (uint32_t *setting, uint8_t *data, uint32_t length);  


/* 
    MANIFEST of IO functions and capabilities
    used to interpret stream_setting in platform_xx.h
*/
    //enum stream_io_domain                 
    //#define PLATFORM_DATA_IN                1u
    //#define PLATFORM_DATA_OUT               2u
    //#define PLATFORM_APPLICATION_DATA_IN    3u
    //#define PLATFORM_APPLICATION_DATA_OUT   4u
    //#define PLATFORM_AUDIO_IN               5u
    //#define PLATFORM_AUDIO_OUT              6u
    //#define PLATFORM_GPIO_IN                7u
    //#define PLATFORM_GPIO_OUT               8u
    //#define PLATFORM_MOTION_CAPTURE         9u
    //#define PLATFORM_PICTURE_IN            10u
    //#define PLATFORM_PICTURE_OUT           11u  
    //#define PLATFORM_USER_INTERFACE_IN     12u 
    //#define PLATFORM_USER_INTERFACE_OUT    13u 
    //#define PLATFORM_COMMAND_IN            14u
    //#define PLATFORM_COMMAND_OUT           15u
    //#define PLATFORM_LOW_DATA_RATE_IN      16u 
    //#define PLATFORM_LOW_DATA_RATE_OUT     17u 
    //#define PLATFORM_RTC_IN                18u 
    //#define PLATFORM_RTC_OUT               19u




struct platform_io_control           
{   io_function_control_ptr io_set;     
    io_function_control_ptr io_start; 
    io_function_control_ptr io_stop; 
    const int32_t *stream_setting;
};


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



enum {
    arc_base_address,
    arc_ceil_address,
    arc_read_address,
    arc_write_address,
    arc_data_amount,
    arc_free_area,
    arc_buffer_size,
    arc_move_to_arc,
    arc_moved_from_arc,
    arc_set_base_address_to_arc,
    arc_set_base_address_from_arc,
    arc_data_realignment_to_base,
};


 
/*============================ STREAM DATA/TYPE =================================*/
/* types fit in 6bits, arrays start with 0, stream_bitsize_of_raw() is identical */

enum stream_raw_data {
    STREAM_DATA_ARRAY = 0,         /* 0 see stream_array: [0NNNTT00] 0, type, nb */

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



struct platform_proc_identification{    /* who am I ? */
    uint16_t architecture;
    uint16_t arch_id;
    uint16_t proc_id;
};


struct platform_control_stream
{   
    uint32_t *graph;        
    uint32_t domain_settings;
    uint8_t fw_idx;
    data_buffer_t buffer;    // [{void *; int}]
};


/*
    global data of a processor, independent of the Stream instances
*/
typedef struct  
{   
    uint32_t *graph;
    intPtr_t *long_offset;
} arm_global_data_t;

/* stream parameters */
//struct stream_control 
//{   const io_function_control_ptr io_set;     
//    const io_function_control_ptr io_start; 
//    const io_function_control_ptr io_stop; 
//};

extern p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS];

/* three entry points */
extern void arm_stream (uint32_t command, 
        void *ptr1, void *ptr2, void *ptr3);
extern void arm_stream_io (uint32_t fw_io_idx, 
        uint32_t *graph,
        uint8_t *data, uint32_t length);
extern void arm_stream_services (uint32_t command, 
        uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3);

extern void stream_scan_graph (uint8_t stream_instance, int8_t return_option, 
    int8_t script_option, int8_t reset_option);

/* ---- REFERENCES --------------------------------------------*/

extern void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3);

extern arm_global_data_t arm_stream_global;

extern intPtr_t convert_ptr_to_int (void *in);
extern void * convert_int_to_ptr (intPtr_t in);

extern intPtr_t * pack2linaddr_ptr(uint32_t data);
extern intPtr_t pack2linaddr_int(uint32_t data);

extern intPtr_t arc_extract_info_int (uint32_t *arc, uint8_t tag);
extern intPtr_t * arc_extract_info_pt (uint32_t *arc, uint8_t tag);
extern int stream_execute_script(void);
extern uint32_t physical_to_offset (uint8_t *buffer);
extern void arc_data_operations (intPtr_t *arc, uint8_t tag, uint8_t *buffer, uint32_t size);

#endif /* #ifndef cSTREAM_TYPES_H */
