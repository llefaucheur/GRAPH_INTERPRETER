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


#ifndef cSTREAM_TOOL_H
#define cSTREAM_TOOL_H

#include "stream_tool_include.h"


/* ----------------------------------------------------------------------
    NODES
* ---------------------------------------------------------------------- */
struct stream_node_manifest
{
    char developerName[MAXCHAR_NAME];   /* developer's name */
    char nodeName[MAXCHAR_NAME];        /* node name used in the GUI */
    uint32_t instance_idx;                   /* inversion version of theis node */
    uint32_t nbInputArc, nbOutputArc, nbParameArc, idxStreamingArcSync;
    uint32_t nbArch, arch[MAXNBARCH];        /* stream_processor_architectures (max 256) */
    uint32_t fpu[MAXNBARCH];                 /* stream_processor_sub_arch_fpu */
    uint32_t swc_assigned_arch, swc_assigned_proc, swc_verbose;
    uint32_t RWinSWC;                        /* XDM11 read/write index is managed in SWC, for variable buffer consumption */
    uint32_t formatUsed;                     /* buffer format is used by the component */
    uint32_t masklib;                        /* mask of up to 16 family of processing extensions "SERVICE_COMMAND_GROUP" */
    uint32_t codeVersion, schedulerVersion;  /* version of the scheduler it is compatible with */
    uint32_t deliveryMode;               /* 0:source, 1:binary 2:2 binaries (fat binary)*/

    /* to build the header */
    uint32_t swc_idx, arcsrdy, narc_streamed, locking_arc, lock_proc, lock_arch;
    uint32_t headerPACK;

    uint32_t nbMemorySegment;
    node_memory_bank_t memreq[MAXNBMEMSEGMENTS]; /* memory allocations request */
    uint64_t sum_of_working_simplemap;   /* cumulate the multiple working segment memory requirements */


    uint32_t preset;                     /* default "preset value" */
    uint32_t local_script_index;
    uint32_t defaultParameterSizeW32;    /* size of the array below */
    uint32_t PackedParameters[MAXPARAMETERSIZE]; /* words32 PACK */

    uint32_t inPlaceProcessing, arcIDbufferOverlay;
    uint32_t nb_arcs;
    struct arcStruct arc[MAX_NB_STREAM_PER_SWC];
};

typedef struct stream_node_manifest stream_node_manifest_t;


/* ----------------------------------------------------------------------
    IO
* ---------------------------------------------------------------------- */
struct stream_IO_interfaces
{
    char IO_name[MAXCHAR_NAME];             /* IO stream name used in the GUI */
    uint32_t domain;                        /* domain of operation */
    uint32_t nb_arc;                        /* number of streams used by this stream interface */

                                            /* format section common to all stream interfaces (digital format) */
                                            /* rx0tx1, domain, digital format and FS accuracy */
    struct arcStruct arc_flow;

    uint32_t format_idx;

    /* desired platform domains mapping to platform capabilities (fw_io_idx) */
    int domains_to_fw_io_idx[IO_PLATFORM_MAX_NB_DOMAINS];

                                            /* format section specific to each domain */
    union
    {   struct stream_interfaces_motion_specific imu;
        struct stream_interfaces_audio_specific audio;
        struct stream_interfaces_2d_in_specific image;
        struct stream_interfaces_metadata_specific metadata;
    } U;
};


/* ----------------------------------------------------------------------
    PROCESSOR
* ---------------------------------------------------------------------- */
struct processing_architecture
{   
    uint32_t processorID;
    uint32_t IamTheMainProcessor;
    uint64_t architecture;
    uint64_t libraries_b;   /* bit-field of embedded libraries in the flash of the device */

    uint32_t nb_long_offset;
    uint32_t nbMemoryBank_detailed;
    processor_memory_bank_t membank[MAX_PROC_MEMBANK];

    uint32_t offset_ID[MAX_NB_MEMORY_OFFSET];
    uint64_t offset_base[MAX_NB_MEMORY_OFFSET];
};
typedef struct processing_architecture processing_architecture_t;




/* ----------------------------------------------------------------------
    PLATFORM = PROCESSOR + IO + NODES
* ---------------------------------------------------------------------- */
struct stream_platform_manifest
{   
    uint32_t nb_nodes;                   /* number of software components to build the graph */
    uint32_t nb_processors;              /* number of processors */
    uint32_t nb_architectures;           /* number of architecture */
    processing_architecture_t processor[MAX_GRAPH_NB_PROCESSORS];

    uint32_t procid_allowed_gr4;    /* graph[4] bit-field of allowed processors */

    uint32_t nb_hwio_stream;
    struct stream_IO_interfaces io_stream[MAX_GRAPH_NB_IO_STREAM];

    struct stream_node_manifest all_nodes[MAX_NB_NODES];    // SWC_IDX + memreq
};

typedef struct stream_platform_manifest stream_platform_manifest_t;


struct dbgtrace
{
    uint32_t address;
    char toprint[MAXNBCHAR_LINE];
};
typedef struct dbgtrace dbgtrace_t;

/* ----------------------------------------------------------------------
    GRAPH = FORMATS + ARCS + NODES + SCRIPTS
* ---------------------------------------------------------------------- */
struct stream_graph_linkedlist
{   
    /*  consolidated formats per arc, inter-nodes and at boundaries */
                                                    /* format section common to all stream interfaces (digital format) */
    struct arcStruct arc[MAX_NB_NODES];             /* rx0tx1, domain, digital format and FS accuracy */
    uint32_t nb_arcs;                               /*  consolidated formats per arc, inter-nodes and at boundaries */
    uint32_t nb_ioarcs;
                                           
    struct formatStruct arcFormat[MAX_NB_FORMAT];   /* merged formats */
    uint32_t nb_formats;

    struct io_arcstruct arcIO[MAX_NB_IO];

    /* script byte-codes */
    uint32_t nb_byte_code;
    uint32_t nb_scripts;
    uint32_t script_indirect[1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1)];
    #define AVG_SCRIPT_LEN 100
    char script_bytecode[AVG_SCRIPT_LEN * (1<<(SCRIPT_LW0_MSB-SCRIPT_LW0_LSB-1))];

    uint32_t script_sctrl;

    uint32_t mem_consumption[MAX_NB_MEMORY_OFFSET];      /* for the listing */

    /* nodes + parameters + computed memory allocations */
    struct stream_node_manifest all_nodes[MAX_NB_NODES];    /* share the same type as manifests */
    uint32_t nb_nodes;
    uint64_t max_of_sum_of_working_simplemap;

    /* memory allocation : incremented per bank, until reaching "processor_memory_bank_t.size" */
    uint64_t mbank_1_Byte;
    uint64_t membank_detailed_malloc[MAX_PROC_MEMBANK];

    uint32_t end_binary_graph;
    uint32_t binary_graph[MAXBINARYGRAPHW32];

    uint32_t idbg;
    dbgtrace_t dbg[MAXDBGTRACEGRAPH];
};

#endif /* #ifndef cSTREAM_TOOL_H */
#ifdef __cplusplus
}
#endif
