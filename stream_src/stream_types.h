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


#include "stream_const.h"


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
    uint32_t iomask;
    uint8_t *ioctrl;
    uint8_t procID;
    uint8_t archID;
    PADDING_BYTES(2);
};

struct stream_xdmbuffer
{   
    intPtr_t address; 
    intPtr_t size;
};

typedef struct stream_xdmbuffer stream_xdmbuffer_t;

typedef uint32_t stream_service_command;

#if STREAM_FLOAT_ALLOWED==1
typedef float sfloat;
typedef double sdouble;

#else
union floatb {
    float f;
    uint32_t i;
    struct {
        unsigned m : 23;   /* Mantissa */
        unsigned e : 8;    /* Exponent */
        unsigned s : 1;    /* Sign bit */
    } b;
};

union doubleb {
    double d;
    uint64_t i;
    struct {
        unsigned ml: 32;   /* Mantissa low */
        unsigned mh: 20;   /* Mantissa high */
        unsigned e : 11;   /* Exponent */
        unsigned s : 1;    /* Sign bit */
    } b;
};
typedef uint32_t sfloat;
typedef uint64_t sdouble;
#endif

/*
    opaque access to the static area of the node 
*/
typedef void *stream_handle_t;  

/*==================================================== SWC  ======================================================================*/
/* 
 *  parameters 
 *    command
 *    *instance pointer
 *    stream_xdmbuffer_t data pointers 
 *    "state" 
 *  the SWC updates the (*,n) fields before returning
 * 
 * The availabilty of data is managed from the scheduler/application to avoid the event scheme of CHRE
 *    When a SWC/nanoAppRT is called this is for real 
 *    https://cs.android.com/android/platform/superproject/+/master:system/chre/pal/include/chre/pal/sensor.h
 *    https://github.com/FromLiQg/chre-wasm/blob/wasm/doc/nanoapp_developer_guide.md
 * 
 * We have one entry-point per nanoAppRT instead of 3 for Android
 *    Android’s context hub enables the use of nanoapps which have 3 entry points seen in chre_api/chre/nanoapp.h:
 *  - nanoappStart function used to notify the nanoapp that it is now active.
 *  - nanoappHandleEvent function used to notify the nanoapp tha an event of interest took place.
 *  - nanoappEnd function used to notify the nanoapp that it is now deactivated.
 * 
 * 
 *  case STREAM_SET_PARAMETER:
 *  case STREAM_RUN:
 *  case STREAM_STOP:
 *  case STREAM_SET_BUFFER:
 */

typedef void    (stream_al_services) (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n); 
typedef void (*p_stream_al_services) (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n); 

typedef void    (stream_node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);
typedef void (*p_stream_node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);

typedef void    (io_function_ctrl) (uint32_t command, uint8_t *data, uint32_t length);   
typedef void (*p_io_function_ctrl) (uint32_t command, uint8_t *data, uint32_t length);  






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




/* ----------------------------------------------------------------------------------------------------------------
    STREAM_IO_DOMAIN (s)

    enum stream_io_domain : list of stream "domains" categories, max 31 (DOMAIN_FMT1)
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).
*/
#define IO_DOMAIN_GENERAL               0  /* (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings*/
#define IO_DOMAIN_AUDIO_IN              1  /* microphone, line-in, I2S, PDM RX */
#define IO_DOMAIN_AUDIO_OUT             2  /* line-out, earphone / speaker, PDM TX, I2S, */
#define IO_DOMAIN_GPIO_IN               3  /* generic digital IO , control of relay, */
#define IO_DOMAIN_GPIO_OUT              4  /* generic digital IO , control of relay, */
#define IO_DOMAIN_MOTION                5  /* accelerometer, combined or not with pressure and gyroscope */
#define IO_DOMAIN_2D_IN                 6  /* camera sensor */
#define IO_DOMAIN_2D_OUT                7  /* display, led matrix, */
#define IO_DOMAIN_ANALOG_IN             8  /* analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage */
#define IO_DOMAIN_ANALOG_OUT            9  /* D/A, position piezzo, PWM converter  */
#define IO_DOMAIN_RTC                  10  /* ticks sent from a programmable timer */
#define IO_DOMAIN_USER_INTERFACE_IN    11  /* button, slider, rotary button */
#define IO_DOMAIN_USER_INTERFACE_OUT   12  /* LED, digits, display, */
#define IO_DOMAIN_PLATFORM_3           13  /*  */                             
#define IO_DOMAIN_PLATFORM_2           14  /* platform-specific #2, decoded with callbacks */                             
#define IO_DOMAIN_PLATFORM_1           15  /* platform-specific #1, decoded with callbacks */                             
#define IO_DOMAIN_MAX_NB_DOMAINS       32
                                         
                                         
#define arc_read_address                1 
#define arc_write_address               2 
#define arc_data_amount                 3 
#define arc_free_area                   4
#define arc_buffer_size                 5
#define data_move_to_arc                6
#define data_moved_from_arc             7
#define data_swapped_with_arc           8
#define arc_data_realignment_to_base    9
    //data_moved_from_arc_update,          
    //arc_set_base_address_to_arc,
    //arc_set_base_address_from_arc,
    //consumer_frame_size,
    //producer_frame_size


 
//enum stream_scheduling_options
//{
#define STREAM_SCHD_RET_NO_ACTION           0    /* the decision is made by the graph */
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
    const intPtr_t *long_offset;          /* pointer to "intPtr_t long_offset[MAX_NB_MEMORY_OFFSET];" */
    uint32_t *graph;
    uint32_t *pio;
    uint32_t *all_formats;   
    uint32_t *all_arcs;
    uint32_t *linked_list;   
    uint32_t *script_offsets;   
    const p_io_function_ctrl *platform_io;
    const p_stream_node *node_entry_point_table;

    const p_stream_al_services * application_callbacks;
    const p_stream_al_services * al_services;

    p_stream_node address_swc;
    uint32_t *linked_list_ptr;      // current position of the linked-list read pointer

    uint32_t *swc_header;           // current swc
    stream_handle_t swc_instance_addr;
    uint16_t arcID[MAX_NB_STREAM_PER_SWC];
    uint8_t *pt8b_collision_arc;    // in mono-processor mono-thread there is only no collision
    uint32_t pack_command;          // preset, narc, tag, instanceID, command

/* */
#define STREAM_COLD_BOOT 0u
#define STREAM_WARM_BOOT 1u         /* Reset + restore memory banks from retention */

#define STREAM_INSTANCE_ANY_PRIORITY    0u      /* PRIORITY_SCTRL_LSB */
#define STREAM_INSTANCE_LOWLATENCYTASKS 1u
#define STREAM_INSTANCE_MIDLATENCYTASKS 2u
#define STREAM_INSTANCE_BACKGROUNDTASKS 3u

#define STREAM_MAIN_INSTANCE 1

#define   UNUSED_SCTRL_MSB U(31)   
#define   UNUSED_SCTRL_LSB U(18)  /* 14 */ 
#define MAININST_SCTRL_MSB U(17)   
#define MAININST_SCTRL_LSB U(17)  /* 1 main instance to set the graph at boot time */
#define NODEEXEC_SCTRL_MSB U(16)   
#define NODEEXEC_SCTRL_LSB U(16)  /* 1 */
#define ENDLLIST_SCTRL_MSB U(15)   
#define ENDLLIST_SCTRL_LSB U(15)  /* 1 endLinkedList detected */
#define STILDATA_SCTRL_MSB U(14)   
#define STILDATA_SCTRL_LSB U(14)  /* 1 still some_components_have_data*/
#define NBINSTAN_SCTRL_MSB U(13)   
#define NBINSTAN_SCTRL_LSB U(10)  /* 4 used at boot time to reset the synchronization flags */
#define     BOOT_SCTRL_MSB U( 9)   
#define     BOOT_SCTRL_LSB U( 9)  /* 1 cold0/warm1 boot : Reset + restore memory banks from retention */
#define   SCRIPT_SCTRL_MSB U( 8)   
#define   SCRIPT_SCTRL_LSB U( 3)  /* 6 script call options bit-field (before/after SWC/loop/full) */
#define   RETURN_SCTRL_MSB U( 2)
#define   RETURN_SCTRL_LSB U( 0)  /* 3 return options (each SWC, each parse, once starving */
#define PACK_STREAM_PARAM(M,N,B,S,R) (  \
            ((M)<<MAININST_SCTRL_LSB) |   \
            ((N)<<NBINSTAN_SCTRL_LSB) |   \
            ((B)<<    BOOT_SCTRL_LSB) |   \
            ((S)<<  SCRIPT_SCTRL_LSB) |   \
            ((R)<<  RETURN_SCTRL_LSB) )
    uint32_t scheduler_control;     // PACK_STREAM_PARAM(..);

    /* identification "whoami", next SWC to run*/
    #define    INST_ID_PARCH_MSB U(31)
    #define     WHOAMI_PARCH_MSB U(31)
    #define   PRIORITY_PARCH_MSB U(31)  /* avoid locking an arc by the same processor, but different RTOS instances*/
    #define   PRIORITY_PARCH_LSB U(30)  /* 2 [0..3] up to 4 instances per processors, 0=main instance at boot */
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
    #define ONGOING_IOCTRL_MSB U(31)  
    #define ONGOING_IOCRTL_LSB U(31)   /* on-going request on IO */
    #define ONGOING_IOSIZE_MSB U(30)  
    #define ONGOING_IOSIZE_LSB U( 0)   /* on-going request on IO */

    uint32_t *ioctrl;                    /* byte array of request fields */

    /* word 2 IO streams to scan , max = 128 */
    #define MAX_GRAPH_NB_IO_STREAM  (1 << (NB_IOS_GR1_MSB - NB_IOS_GR1_LSB + 1))
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

    uint8_t swc_memory_banks_offset;    /* offset in words  */

    uint8_t swc_parameters_offset;

    uint8_t nb_stream_instances;    /* stream instances pointers (in words) = &(all_arcs[ -nb_stream_instances]) */

    uint8_t script_arctx;           /* arc buffer for the static area of the script */

    uint8_t memory_segment_swap;    /* bit-field of the memory segments to swap (TO_SWAP_LW2S) */

    uint8_t error_log;              /* bit-field of logged errors */

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

