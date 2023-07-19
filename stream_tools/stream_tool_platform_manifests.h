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
   

#ifndef cSTREAM_PLATFORM_MANIFEST_H
#define cSTREAM_PLATFORM_MANIFEST_H

#include <stdint.h>


#define MAX_PROC_MEMBANK 32 /* number of physical memory banks of the processor, for the graph processing */

struct memory_bank
{
    uint8_t offsetID;       /* index of this bank return by the AL */
};

struct processing_architecture
{
    uint8_t IamTheMainProcessor;
    uint8_t architecture;
    uint8_t nbMemoryBank;
    struct memory_bank membank[MAX_PROC_MEMBANK];
};

struct stream_interfaces
{
};


struct stream_platform_manifest
{       
    uint16_t nb_nodes;                  /* number of software components to build the graph */
    struct processing_architecture processor[MAX_GRAPH_NB_PROCESSORS];
    struct stream_interfaces stream[MAX_GRAPH_NB_IO_STREAM];
};

typedef struct stream_platform_manifest stream_platform_manifest_t;



#endif /* #ifndef cSTREAM_PLATFORM_MANIFEST_H */
#ifdef __cplusplus
}
#endif
