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
   

#ifndef cSTREAM_NODE_MANIFESTS_H
#define cSTREAM_NODE_MANIFESTS_H

#include "stream_const.h"

#define MAXCHAR_NAME 80
#define MAXNBARCH 8
#define MAXNBLIBRARY 16
#define MAXPARAMETERSIZE ((int64_t)1e5)  /* in bytes */

struct memory_bank
{
    /* Memory Size = A + B x nb_channels_arc(i) + C x sampling_arc(j) + D x frame_size_arc(k) */
    uint32_t sizeA;
    float sizeB, sizeC, sizeD;
    uint16_t iarcChannelI, iarcSamplingJ, iarcFrameK;
    uint8_t alignmentLog2;      /* enum buffer_alignment_type */
    uint8_t usage;              /* enum mem_mapping_type : static, working, pseudo working, backup */
    uint8_t speed;              /* enum mem_speed_type */
    uint8_t relocatable;        /* enum buffer_relocation_type */
};


struct arcStruct
{
    uint8_t rx0tx1;             /* direction */
    uint8_t domain;             /* domain_name[][] enum stream_io_domain */
    /*
    *   stream_data_stream_data_format (size multiple of 3 x uint32_t)
    *       word 0 : common to all domains : frame size, raw format, interleaving
    *       word 1 : common to all domains : time-stamp, sampling rate , nchan
    *       word 2 : specific to domains : hashing, direction, channel mapping, image width/borders
    */
    uint32_t format[STREAM_FORMAT_SIZE_W32];
};


struct stream_node_manifest
{   
    char developerName[MAXCHAR_NAME];   /* developer's name */
    char nodeName[MAXCHAR_NAME];        /* node name used in the GUI */
    uint8_t nbInputArc;
    uint8_t nbOutputArc;
    uint8_t nbArch, arch[MAXNBARCH];    /* stream_processor_architectures (max 256) */
    uint8_t nbFpu, fpu[MAXNBARCH];      /* stream_processor_sub_arch_fpu */
    uint8_t nbLib, lib[MAXNBLIBRARY];   /* Up to 16 family of processing extensions "SERVICE_COMMAND_GROUP" */
    uint32_t codeVersion, schedulerVersion; /* version of the scheduler it is compatible with */
    uint8_t deliveryMode;               /* 0:source, 1:binary */

    uint8_t nbMemorySegment;            
    struct memory_bank mem[MAXNBMEMSEGMENTS]; /* memory allocations */

    uint8_t preset;                     /* default "preset value" */
    uint32_t defaultParameterSize;      /* size of the array below */
    uint8_t defaultParameters[MAXPARAMETERSIZE];

    uint16_t inPlaceProcessing, arcIDbufferOverlay;
    uint8_t nbArcs;
    struct arcStruct arcs[MAX_NB_STREAM_PER_SWC];
};

#endif /* #ifndef cSTREAM_NODE_MANIFESTS_H */

#ifdef __cplusplus
}
#endif
