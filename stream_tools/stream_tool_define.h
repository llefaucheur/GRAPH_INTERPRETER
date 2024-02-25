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


#ifndef cSTREAM_TOOL_DEFINE_H
#define cSTREAM_TOOL_DEFINE_H

#define HEADER "top_header"
#define PATHS "paths"
#define IO_INTERFACE "top_graph_interface"
#define FORMAT "format"
#define PARAMETER_START "parameters"
#define NODE "node"
#define ARM_SCRIPT "arm_script"
#define SCRIPT "SCRIPT"
#define ARC "arc"
#define INCLUDE "include"
//#define _INTERFACE_NODE_ID 0
#define SECTION_END "_end_"

#define MAX_NB_NODES 100
#define MAX_NB_FORMAT 32

#define MAXINPUT 100000
#define MAXOUTPUT 100000
#define MAXBINARYGRAPHW32 100000
#define MAXDBGTRACEGRAPH 10000

#define MAXNBCHAR_LINE 200
#define NOT_YET_END_OF_FILE 1
#define FOUND_END_OF_FILE 2
#define MAX_NB_PATH 32          /* number of file paths in the graph */
#define MAX_NB_IO 100           /* number of IO streams of the graph */

#define NBCHAR_LINE 200
#define NBCHAR_NAME 120
#define NBCHAR_STREAM_NAME 32   /* name of the HW IO streams and graph interfaces */
#define MAX_PROC_MEMBANK 32     /* number of physical memory banks of the processor, for the graph processing */


#define MAXCHAR_NAME 80
#define MAXNBARCH 8
#define MAXNBLIBRARY 16
#define MAXPARAMETERSIZE ((int64_t)1e5)  /* in word32 */

#define MAX_NBOPTIONS 256 


#endif /* #ifndef cSTREAM_TOOL_DEFINE_H */
#ifdef __cplusplus
}
#endif
