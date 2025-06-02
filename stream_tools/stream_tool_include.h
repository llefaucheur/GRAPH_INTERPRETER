/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_tool_include.h
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
   

#ifndef cSTREAM_TOOL_INCLUDE_H
#define cSTREAM_TOOL_INCLUDE_H

#ifdef _MSC_VER 
#define _CRT_SECURE_NO_DEPRECATE
#endif
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>  /* for void fields_extract(char **pt_line, char *types,  ...) */

#include "../stream_src/stream_const.h"
#include "../stream_platform/platform.h"
#include "../stream_src/stream_types.h"
#include "stream_tool_define.h"
#include "stream_tool.h"
#include "../stream_nodes/stream_common_const.h"
#include "../stream_nodes/stream_common_types.h"
#include "../stream_src/stream_const.h"


extern uint8_t globalEndFile, FoundEndSection;

extern void jump2next_line (char** line);
extern void jump2next_valid_line (char** line);
extern void read_binary_param(char** pt_line, void* X, uint8_t* raw_type, uint32_t* nb_option);
extern void read_common_data_options(char** pt_line, struct options* pt);
extern void read_input_file(char* file_name, char* inputFile);
extern int  fields_extract(char **pt_line, char *types,  ...);
extern int  fields_options_extract(char **pt_line, struct options *o1);
extern int  stream_bitsize_of_raw(uint8_t raw);
extern int  search_word(char line[], char word[]);
extern void stream_tool_read_parameters(char **pt_line, 
        struct stream_platform_manifest *platform, 
        struct stream_graph_linkedlist *graph, 
        uint32_t *ParameterSizeW32,
        uint32_t *PackedParameters);
extern int  vid_malloc(uint32_t VID, uintptr_t size, uint32_t alignment, uint32_t *pack27b, int working, char *comment, struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph);
extern void search_platform_node(char *cstring, struct stream_node_manifest **platform_node, uint32_t *platform_NODE_idx,
            struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph);
extern void search_graph_node(char *cstring, struct stream_node_manifest **graph_node, uint32_t *graph_NODE_idx, struct stream_graph_linkedlist *graph);
extern void compute_memreq(struct node_memory_bank *m, struct formatStruct *all_format, struct stream_node_manifest *node);
extern void stream_tool_read_code(char **pt_line, struct stream_platform_manifest *platform,
                            struct stream_graph_linkedlist *graph, struct stream_script *script);
extern void stream_tool_read_assembler(char **pt_line, struct stream_platform_manifest *platform,
                            struct stream_graph_linkedlist *graph, struct stream_script *script);

//extern void pack_AUDIO_IN_IO_setting(struct arcStruct *arc);
//extern void pack_AUDIO_OUT_IO_setting(struct arcStruct *arc);
//extern void pack_motion_IO_setting(struct arcStruct *arc);
//extern void pack_2D_IN_IO_setting(struct arcStruct *arc);
//extern void pack_2D_OUT_IO_setting(struct arcStruct *arc);
//extern void pack_ANALOG_IN_IO_setting(struct arcStruct *arc);
//extern void pack_ANALOG_OUT_IO_setting(struct arcStruct *arc);

#endif /* #ifndef cSTREAM_TOOL_INCLUDE_H */

#ifdef __cplusplus
}
#endif
