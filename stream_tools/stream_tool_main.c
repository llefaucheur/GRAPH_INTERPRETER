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

#ifdef _MSC_VER 
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "stream_tool_include.h"

#define GRAPH_ALL_MANIFESTS "../../../stream_tools/stream_tools_files_manifests_computer.txt"
#define GRAPH_GGRAPH        "../../../stream_tools/graph_ggraph.txt"    /* generic graph, with desired IO domains */
#define GRAPH_PGRAPH        "../../../stream_tools/graph_pgraph.txt"    /* platform specific graph, with real IO domains */
#define GRAPH_GMAP          "../../../stream_tools/graph_gmap.txt"      /* almost-binary graph with soft addresses */
 /* "../../../stream_tools/graph_omap.txt" same graph with hand optimized mapping and multi-processor mapping */
#define GRAPH_OMAP          GRAPH_GMAP                                  /* take the default configuration */
#define GRAPH_BIN           "../../../stream_tools/graph_bin.txt"       /* final binary graph file */


char all_files[MAXINPUT];
char ggraph [MAXINPUT];
char pgraph [MAXINPUT];
char output[MAXOUTPUT];

struct stream_platform_manifest platform;
struct stream_node_manifest all_nodes[MAX_NB_NODES];

extern void arm_stream_read_manifests (struct stream_platform_manifest *platform, struct stream_node_manifest *all_nodes, char *all_files);
extern void arm_stream_ggraph2pgraph(struct stream_platform_manifest* platform, struct stream_node_manifest* all_nodes,
            char* ggraph_txt, uint32_t ggraph_length, FILE* ptf_pgraph);
void arm_stream_script_assembler (char *script, uint32_t length, FILE *ptf_graph_script_bytecode, char *script_bytecode, uint32_t *nbByteCodes);
extern void arm_stream_graphTxt2Bin (char *graph_bin, FILE *ptf_graph_txt);

/**
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par              translates the graph intermediate format GraphTxt to GraphBin to be reused
                    in CMSIS-Stream/stream_graph/*.txt
  @remark
 */

void main(void)
{
    /* 
        STEP 1 : read the file names : 
                platform manifest
                stream io manifests
                nodes manifests
    */
    {   FILE* ptf_graph_all_files;
        uint32_t idx;

        if (0 == (ptf_graph_all_files = fopen(GRAPH_ALL_MANIFESTS, "rt"))) exit(-1);
        idx = 0;
        while (1) if (0 == fread(&(all_files[idx++]), 1, 1, ptf_graph_all_files)) break;
        arm_stream_read_manifests(&platform, all_nodes, all_files);
        fclose(ptf_graph_all_files);
    }
    /*
        STEP 2: convert a GRAPH file to a Platform-specific graph 
                cumulate SRAM needs
                list of formats
                insert SWC converters when needed

        graph format structure = 
                list of formats and cros reference to the arcs using them
                IO stream descriptors, filled in step 1
                stream instances : IO mapping and debug arcs
                "IO nodes" from the initial graph TXT = desired domain
                Nodes : instanceID, indexed memory banks, debug info, default processors/arch, 
                    parameter preset/size/tags, list of arcs/streaming-arcs
                Scripts code
                memory banks size accumulators, physical address, speed
                digital platform : processors ID/arch/IO-affinity
                    domain ios, fwioIDX, platform name
                list of arcs: buffer memory bank (base,size), debug, node src/dst +arc idx
    */
    {   FILE *ptf_ggraph, *ptf_pgraph;
        uint32_t idx;

        if (0 == (ptf_pgraph = fopen(GRAPH_PGRAPH, "wt"))) exit(-1);
        if (0 == (ptf_ggraph = fopen(GRAPH_GGRAPH, "rt"))) exit(-1); idx = 0;
        while (1) if (0 == fread(&(ggraph[idx++]), 1, 1, ptf_ggraph)) break;

        arm_stream_ggraph2pgraph(&platform, all_nodes, ggraph, idx, ptf_pgraph);
        fclose(ptf_pgraph); fclose(ptf_ggraph);
        void arm_stream_ggraph2pgraph(struct stream_platform_manifest* platform, struct stream_node_manifest* all_nodes,
            char* ggraph_txt, uint32_t ggraph_length, FILE * ptf_pgraph);
    }

    /* 
        STEP 3 : export the graph in a TXT format used for mapping optimizations
    */

        while (1) if (0 == fread(&(ggraph[idx++]), 1, 1, ptf_ggraph)) break;
    //    uint32_t idx;

    //    if (0 == (ptf_gmap = fopen(GRAPH_GMAP, "wt"))) exit(-1);
    //    if (0 == (ptf_ggraph = fopen(GRAPH_PGRAPH, "rt"))) exit(-1); idx = 0;
    //    while (1) if (0 == fread(&(ggraph[idx++]), 1, 1, ptf_ggraph)) break;
    //    fclose(ptf_ggraph); fclose(ptf_ggraph);
    //}

    /* 
        STEP 4 : convert the manually optimized, graph, to the binary format
            used by the graph interpreter and scheduler
    */
    {   FILE* ptf_graph_omap, * ptf_graph_bin;
        uint32_t idx;

        if (0 == (ptf_graph_bin = fopen(GRAPH_BIN, "wt"))) exit(-1);
        if (0 == (ptf_graph_omap = fopen(GRAPH_OMAP, "rt"))) exit(-1); idx = 0;
        while (1) if (0 == fread(&(pgraph[idx++]), 1, 1, ptf_graph_omap)) break;

        arm_stream_graphTxt2Bin(pgraph, ptf_graph_bin);
        fclose(ptf_graph_bin); fclose(ptf_graph_omap);
    }
    exit (-3); 
}
