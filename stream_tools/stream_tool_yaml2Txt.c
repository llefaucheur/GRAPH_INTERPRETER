/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Tools
 * Title:        main.c
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
    

#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

#include "stream_const.h"
#include "stream_types.h"

#define SZ_LINE 200
char current_line[SZ_LINE];

/* 
    enum stream_io_domain : list of stream "domains" categories 
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).

    a change in this list 
*/
#define CHARLENDOMAINS 80

char raw_type_name[LAST_RAW_TYPE][CHARLENDOMAINS] = 
{
    "int8",     /* STREAM_S8,  */
    "uint8",    /* STREAM_U8,  */
    "int16",    /* STREAM_S16, */
    "uint16",   /* STREAM_U16, */
    "int32"     /* STREAM_S32, */
    "uint32",   /* STREAM_U32, */
    "float",    /* STREAM_Q31, */
    "double",   /* STREAM_FP64,*/
};

/* mem_speed_type */
char memory_speed_name[][CHARLENDOMAINS] = 
{
    "bestEffort",   /* MEM_SPEED_REQ_ANY           0    /* best effort */
    "normal",       /* MEM_SPEED_REQ_NORMAL        1    /* can be external memory */
    "fast",         /* MEM_SPEED_REQ_FAST          2    /* will be internal SRAM when possible */
    "criticalFast", /* MEM_SPEED_REQ_CRITICAL_FAST 3    /* will be TCM when possible */
};

/* mem_mapping_type */
char memory_usage_name[][CHARLENDOMAINS] = 
{
    "static",       /* MEM_TYPE_STATIC          0  (LSB) memory content is preserved (default ) */
    "worging",      /* MEM_TYPE_WORKING         1  scratch memory content is not preserved between two calls */
    "psudoWorking", /* MEM_TYPE_PSEUDO_WORKING  2  static only during the uncompleted execution state of the SWC, see “NODE_RUN” */
    "backup",       /* MEM_TYPE_PERIODIC_BACKUP 3  static parameters to reload for warm boot after a crash, holding for example ..*/
};

char domain_name[PLATFORM_MAX_NB_DOMAINS][CHARLENDOMAINS] = 
{
    "",
    "data_in",                 // #define PLATFORM_DATA_IN                1     not streamed
    "data_out",                // #define PLATFORM_DATA_OUT               2   
    "data_stream_in",          // #define PLATFORM_DATA_STREAM_IN         3     MPEG, temperature, 
    "data_stream_out",         // #define PLATFORM_DATA_STREAM_OUT        4
    "audio_in",                // #define PLATFORM_AUDIO_IN               5     
    "audio_out",               // #define PLATFORM_AUDIO_OUT              6
    "gpio_in",                 // #define PLATFORM_GPIO_IN                7
    "gpio_out",                // #define PLATFORM_GPIO_OUT               8
    "motion_in",               // #define PLATFORM_MOTION_IN              9
    "2d_in",                   // #define PLATFORM_2D_IN                 10
    "2d_out",                  // #define PLATFORM_2D_OUT                11
    "user_interface_in",       // #define PLATFORM_USER_INTERFACE_IN     12  
    "user_interface_out",      // #define PLATFORM_USER_INTERFACE_OUT    13
    "command_in",              // #define PLATFORM_COMMAND_IN            14
    "command_out",             // #define PLATFORM_COMMAND_OUT           15
    "low_data_rate_in",        // #define PLATFORM_LOW_DATA_RATE_IN      16
    "low_data_rate_out",       // #define PLATFORM_LOW_DATA_RATE_OUT     17
    "rtc_in",                  // #define PLATFORM_RTC_IN                18
    "rtc_out",                 // #define PLATFORM_RTC_OUT               19
    "storage_out"              // #define PLATFORM_STORAGE_OUT           20
};


/**
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par              translates the graph intermediate format GraphTxt to GraphBin to be reused
                    in CMSIS-Stream/stream_graph/*.txt
  @remark
 */

void arm_stream_yaml2txt (struct stream_platform_manifest *platform, struct stream_node_manifest *all_nodes, 
                          char *yaml_txt, uint32_t yaml_length, FILE *ptf_graph_opt)
{
#if 0
    char *pt_line;
    uint32_t state, substate;

    enum read_state
    {   YMLINIT,
        YMLNODES, 
        YMLARCS,
    };

    pt_line = yaml_txt;
    state = YMLINIT;

    while (1)
    {
        /* check end of file */
        read new line 
        (pt_line - yaml_txt >= ncharyaml)
        break;

        /* wait for "nodes:" or "arcs:" and change state*/
        if search "nodes:"  and "arcs:" 
            => change state , loop

        /* state nodes : read one line */
        if (state == YMLNODES)
        {   if "node:" : Ptr declare new node ; and loop 
            if "inputs" :  fill new node fields; loop 
            if "outputs" :  fill new node fields;loop 
        }
            /* */
        /* state arcs */
        if (state == YMLARCS)
        {   if "src:" : substate = SRC declare a new arc; and loop 
            if "node" :     prepare index of node ; loop 
            if "inputs" :   fill fields of arc ; loop 
            if "outputs" :  fill fields of arc ; loop 
        }
#endif
}


#ifdef __cplusplus
}
#endif