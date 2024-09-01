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
#include "platform.h"

#ifdef PLATFORM_COMPUTER 
#define GRAPH_ALL_MANIFESTS "../../stream_platform/computer/manifest/top_manifest_computer.txt"
#define GRAPH_TXT           "../../stream_platform/computer/graph_computer.txt"     /* graph */
#define GRAPH_BIN           "../../stream_platform/computer/graph_computer_bin.txt" /* binary graph file */
#define GRAPH_HEADER        "../../stream_platform/computer/graph_computer_header.h"  /* list of labels to do "set_parameter" from scripts */
#define GRAPH_DEBUG         "../../stream_platform/computer/graph_computer_debug.txt" /* comments made during graph conversion  */
#endif

#ifdef PLATFORM_LPC55S69EVK 
#define GRAPH_ALL_MANIFESTS "../../stream_platform/lpc55s69evk/manifest/top_manifest_lpc55s69evk.txt"
#define GRAPH_TXT           "../../stream_platform/lpc55s69evk/graph_lpc55s69evk.txt"     /* graph */
#define GRAPH_BIN           "../../stream_platform/lpc55s69evk/graph_lpc55s69evk_bin.txt" /* binary graph file */
#define GRAPH_HEADER        "../../stream_platform/lpc55s69evk/graph_lpc55s69evk_header.h"  /* list of labels to do "set_parameter" from scripts */
#define GRAPH_DEBUG         "../../stream_platform/lpc55s69evk/graph_lpc55s69evk_debug.txt" /* comments made during graph conversion  */
#endif

#ifdef PLATFORM_MSPM0L1306 
#define GRAPH_ALL_MANIFESTS "../../stream_platform/lp-mspm0l1306/manifest/top_manifest_lp-mspm0l1306.txt"
#define GRAPH_TXT           "../../stream_platform/lp-mspm0l1306/graph_lp-mspm0l1306.txt"     /* graph */
#define GRAPH_BIN           "../../stream_platform/lp-mspm0l1306/graph_lp-mspm0l1306_bin.txt" /* binary graph file */
#define GRAPH_HEADER        "../../stream_platform/lp-mspm0l1306/graph_lp-mspm0l1306_header.h"  /* list of labels to do "set_parameter" from scripts */
#define GRAPH_DEBUG         "../../stream_platform/lp-mspm0l1306/graph_lp-mspm0l1306_debug.txt" /* comments made during graph conversion  */
#endif

extern void arm_stream_read_manifests (struct stream_platform_manifest *platform, char *all_files);
extern void arm_stream_read_graph(struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph, char* ggraph_txt);
extern void arm_stream_memory_map(struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph);
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
    char *all_files, *ggraph;

    struct stream_platform_manifest *platform;
    struct stream_graph_linkedlist *graph;

    if (0 == (all_files = calloc (MAXINPUT, 1))) {  fprintf (stderr, "\n init error \n"); {  fprintf (stderr, "\n init error \n"); exit(-1); } }
    if (0 == (ggraph = calloc (MAXINPUT, 1))) {  fprintf (stderr, "\n init error \n"); exit(-1); }
    if (0 == (platform = calloc (sizeof(struct stream_platform_manifest), 1))) {  fprintf (stderr, "\n init error \n"); exit(-1); }
    if (0 == (graph = calloc (sizeof(struct stream_graph_linkedlist), 1))) {  fprintf (stderr, "\n init error \n"); exit(-1); }

    /* 
        Read the file names : 
        platform manifest
        stream io manifests
        nodes manifests
    */
    memset(platform, 0, sizeof(struct stream_platform_manifest));
    memset(graph, 0, sizeof(struct stream_graph_linkedlist));


    if (0 == (graph->ptf_graph_bin  = fopen(GRAPH_BIN,  "wt"))) {  fprintf (stderr, "\n init error \n"); exit(-1); }
    if (0 == (graph->ptf_header = fopen(GRAPH_HEADER, "wt"))) {  fprintf (stderr, "\n init error \n"); exit(-1); }
    if (0 == (graph->ptf_debug = fopen(GRAPH_DEBUG, "wt"))) {  fprintf (stderr, "\n init error \n"); exit(-1); }


    read_input_file (GRAPH_ALL_MANIFESTS, all_files);
    arm_stream_read_manifests(platform, all_files);
    

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
    

    read_input_file (GRAPH_TXT, ggraph);
    arm_stream_read_graph(platform, graph, ggraph);

    /*@@@  TODO
       check consistency : formats between nodes/arcs 
       to help the graph designer insert conversion nodes
        - ARCS : does { struct options raw_format_options } matches with { struct formatStruct format; }
        - NODES : does { struct options raw_format_options } matches with { struct arcStruct arc[MAX_NB_STREAM_PER_NODE].format; }
         arm_stream_check_graph(platform, graph); 
       
      @@@  TODO 
       generate the debug TXT file and header used to address nodes in the binary graph,
       header with a declaration arc ID and arc names
       
         arm_stream_print_graph(platform, graph); 
      
        check format compatibility to tell the graph programmer to insert conversion nodes
        remove formats declared twice 
      
        Convert the structure to the binary format
            used by the graph interpreter and scheduler
    */
    {   FILE * ptf_graph_bin;

        if (0 == (ptf_graph_bin = fopen(GRAPH_BIN, "wt"))) exit(-1);

        arm_stream_graphTxt2Bin(platform, graph, ptf_graph_bin);

        fclose(ptf_graph_bin); 

    }

    fprintf (stderr, "\n graph compilation done \n");
    exit (-3); 
}
