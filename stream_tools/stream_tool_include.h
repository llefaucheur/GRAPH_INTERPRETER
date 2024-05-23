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
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>  /* for void fields_extract(char **pt_line, char *types,  ...) */

//#include "../stream_platform/windows/platform_computer.h"
#include "../stream_src/stream_const.h"
#include "../stream_src/stream_types.h"
#include "stream_tool_define.h"
#include "stream_tool.h"

extern uint8_t globalEndFile, FoundEndSection;

extern void     jump2next_line (char** line);
extern uint32_t quantized_FS (float FS);
extern void     jump2next_valid_line (char** line);
extern void     read_binary_param(char** pt_line, void* X, uint8_t* raw_type, uint32_t* nb_option);
extern void     read_common_data_options(char** pt_line, struct options* pt);
extern void     read_input_file(char* file_name, char* inputFile);
extern int      fields_extract(char **pt_line, char *types,  ...);
extern void     fields_list(char **pt_line, struct options *opt);
extern void     fields_list_subtype(char **pt_line, struct options_subtype *opt);
extern int      stream_bitsize_of_raw(uint8_t raw);
extern int      search_word(char line[], char word[]);
extern void     stream_tool_read_parameters(char **pt_line, struct stream_node_manifest *node);
extern int      vid_malloc(uint32_t VID, uint64_t size, uint32_t alignemnt, uint32_t *pack27b, uint8_t *ptr, 
                struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph);

#endif /* #ifndef cSTREAM_TOOL_INCLUDE_H */

#ifdef __cplusplus
}
#endif
