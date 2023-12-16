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


#include "stream_tool_include.h"

#define GRAPH_ALL_MANIFESTS "../../../stream_al/platform_computer/files_manifests_computer.txt"
#define GRAPH_TXT           "../../../stream_graph/graph_0.txt"     /* almost-binary graph with soft addresses hand optimized mapping and multi-processor mapping */
#define GRAPH_LST           "../../../stream_graph/graph_0_listing.txt"     /* almost-binary graph with soft addresses hand optimized mapping and multi-processor mapping */
#define GRAPH_BIN           "../../../stream_graph/graph_0_bin.txt"   /* final binary graph file */


char all_files[MAXINPUT];
char ggraph [MAXINPUT];
char listing [MAXINPUT];
char output[MAXOUTPUT];

FILE *ptf_listing;

struct stream_platform_manifest platform;
struct stream_graph_linkedlist graph;

extern void arm_stream_read_manifests (struct stream_platform_manifest *platform, char *all_files);
extern void arm_stream_read_graph(struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph, char* ggraph_txt);
extern void arm_stream_script_assembler (char *script, FILE *ptf_graph_script_bytecode, char *script_bytecode, uint32_t *nbByteCodes);
extern void arm_stream_graphFormatChecks (struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph);
extern void arm_stream_graphTxt2Bin (struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph, FILE *ptf_graph_bin);

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
        Read the file names : 
        platform manifest
        stream io manifests
        nodes manifests
    */
    memset(&platform, 0, sizeof(struct stream_platform_manifest));
    memset(&graph, 0, sizeof(struct stream_graph_linkedlist));

    read_input_file (GRAPH_ALL_MANIFESTS, all_files);
    arm_stream_read_manifests(&platform,  all_files);
    

    /*
        read the GRAPH, compile the scripts
        cumulate SRAM needs (static/working/graph size) 
        list of formats, warn for the need SWC converters

        graph format structure = 
            IO mapping and debug arcs
            Nodes : instanceID, indexed memory banks, debug info, default processors/arch, 
                parameter preset/size/tags, list of arcs/streaming-arcs
            Scripts code
            memory banks size accumulators, physical address, speed
            digital platform : processors ID/arch/IO-affinity
                domain ios, fwioIDX, platform name
            list of arcs: buffer memory bank (base,size), debug, node src/dst +arc idx

            PACK data to 32bits
    */
    

    if (0 == (ptf_listing = fopen(GRAPH_LST, "wt"))) exit(-1);

    read_input_file (GRAPH_TXT, ggraph);

    arm_stream_read_graph(&platform, &graph, ggraph);

    fclose(ptf_listing); 
    

    /* 
        check format compatibility and insert conversion nodes
        remove formats declared twice

    */

    arm_stream_graphFormatChecks(&platform, &graph);

    /* 
        Convert the structure to the binary format
            used by the graph interpreter and scheduler
    */
    {   FILE * ptf_graph_bin;

        if (0 == (ptf_graph_bin = fopen(GRAPH_BIN, "wt"))) exit(-1);

        arm_stream_graphTxt2Bin(&platform, &graph, ptf_graph_bin);
        fclose(ptf_graph_bin); 
    }
    exit (-3); 
}
