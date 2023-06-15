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

#define GRAPH_TXT "../../../stream_tools/graph_txt_1.h"
#define GRAPH_BIN "../../../stream_tools/graph_bin_1.h"

#define MAXINPUT 100000
#define MAXOUTPUT 100000

char input [MAXINPUT];
char output[MAXOUTPUT];

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
    FILE *ptf_graph_txt, *ptf_graph_bin;

    if (0 == (ptf_graph_bin = fopen (GRAPH_BIN, "wt"))) exit(-1);
    if (0 == (ptf_graph_txt = fopen (GRAPH_TXT, "rt"))) exit(-1);

    { char *p8=input;
        do {
            fread(p8, 1, 1, ptf_graph_txt);
            p8++;
            if (feof(ptf_graph_txt))
            {   break;
            }
        } while (1);
    }
    
    fprintf (ptf_graph_bin, "// AUTOMATICALLY GENERATED FILE, DON'T EDIT IT !\n");

    arm_stream_graphTxt2Bin (input, ptf_graph_bin);

    fflush (ptf_graph_bin);
    fclose (ptf_graph_bin);
    fclose (ptf_graph_txt);
    exit (-3);
}
