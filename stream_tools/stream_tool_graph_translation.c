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

#include "stream_tool_node_manifests.h"
#include "stream_tool_platform_manifests.h"

#define GRAPH_MANIFESTS "../../../stream_tools/stream_tools_files_computer.txt"
#define GRAPH_YML "../../../stream_tools/graph_yaml.txt"
#define GRAPH_TXT "../../../stream_tools/graph_txt_0.txt"
#define GRAPH_OPT "../../../stream_tools/graph_txt_1.txt"
#define GRAPH_SCRIPT "../../../stream_tools/graph_script.txt"
#define GRAPH_BIN "../../../stream_tools/graph_bin_1.txt"

#define MAXINPUT 100000
#define MAXOUTPUT 100000
#define MAX_NB_NODES 2048
char input [MAXINPUT];
char output[MAXOUTPUT];

extern void arm_stream_graphTxt2Bin (char *graph_bin, FILE *ptf_graph_txt);
extern void arm_stream_yaml2opt (char *yaml_txt, uint32_t ncharyaml, FILE *ptf_graph_opt);
extern void arm_stream_read_manifests (struct stream_platform_manifest *platform, struct stream_node_manifest *all_nodes, FILE *ptf_graph_all_files);
extern void arm_stream_yaml2opt (struct stream_platform_manifest *platform, struct stream_node_manifest *all_nodes, 
                          char *yaml_txt, uint32_t ncharyaml, FILE *ptf_graph_opt);

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
    FILE *ptf_graph_txt, *ptf_graph_bin, *ptf_graph_yaml, *ptf_graph_2opt, *ptf_graph_all_files;
    uint32_t file_length;

    struct stream_platform_manifest platform;
    struct stream_node_manifest all_nodes[MAX_NB_NODES];

    /* 
        STEP 1 : read the file names : platform manifest, all the nodes manifests
    */
    if (0 == (ptf_graph_all_files = fopen (GRAPH_MANIFESTS, "rt"))) exit(-1);
    {   arm_stream_read_manifests (&platform, all_nodes, ptf_graph_all_files);
    }
    fclose (ptf_graph_all_files);

    /* 
        STEP 2 : convert YAML file to a graph ready for an optional 
        memory map manual optimization
    */
    if (0 == (ptf_graph_2opt = fopen (GRAPH_TXT, "wt"))) exit(-1);
    if (0 == (ptf_graph_yaml = fopen (GRAPH_YML, "rt"))) exit(-1);
    { char *p8=input;
        do 
        {   fread(p8, 1, 1, ptf_graph_yaml);
            p8++;
            if (feof(ptf_graph_yaml))
            {   break;
            }
        } while (1);
        file_length = (uint32_t)((uint64_t)p8 - (uint64_t)input);
        fclose (ptf_graph_yaml);
    }
    fprintf (ptf_graph_2opt, "; AUTOMATICALLY GENERATED FILE, DON'T EDIT IT !\n");
    arm_stream_yaml2opt (&platform, all_nodes, input, file_length, ptf_graph_2opt);
    fflush (ptf_graph_2opt);
    fclose (ptf_graph_2opt);


    /* 
        STEP 3 : convert SCRIPT files 
    */
    if (0 == (ptf_graph_2opt = fopen (GRAPH_TXT, "wt"))) exit(-1);
    if (0 == (ptf_graph_yaml = fopen (GRAPH_YML, "rt"))) exit(-1);
    { char *p8=input;
        do 
        {   fread(p8, 1, 1, ptf_graph_yaml);
            p8++;
            if (feof(ptf_graph_yaml))
            {   break;
            }
        } while (1);
        file_length = (uint32_t)((uint64_t)p8 - (uint64_t)input);
        fclose (ptf_graph_yaml);
    }
    fprintf (ptf_graph_2opt, "; AUTOMATICALLY GENERATED FILE, DON'T EDIT IT !\n");
    arm_stream_yaml2opt (&platform, all_nodes, input, file_length, ptf_graph_2opt);
    fflush (ptf_graph_2opt);
    fclose (ptf_graph_2opt);


    /* 
        STEP 4 : convert the readable, manually optimized, graph, to the binary format
            used by the graph interpreter and scheduler
    */
    if (0 == (ptf_graph_bin = fopen (GRAPH_BIN, "wt"))) exit(-1);
    if (0 == (ptf_graph_txt = fopen (GRAPH_OPT, "rt"))) exit(-1);
    { char *p8=input;
        do {
            fread(p8, 1, 1, ptf_graph_txt);
            p8++;
            if (feof(ptf_graph_txt))
            {   break;
            }
        } while (1);
        fclose (ptf_graph_txt);
    }
    
    fprintf (ptf_graph_bin, "// AUTOMATICALLY GENERATED FILE, DON'T EDIT IT !\n");
    arm_stream_graphTxt2Bin (input, ptf_graph_bin);
    fflush (ptf_graph_bin);
    fclose (ptf_graph_bin);
    exit (-3);
}
