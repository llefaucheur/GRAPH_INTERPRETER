/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Tools
 * Title:        stream_tool_script_assembler.c
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
    
#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "stream_tool_include.h"

/**
  @brief            (stream_tool_script_assembler) 
  @param[in/out]    none
  @return           int

  @par              translates the script assembly in a table of byte codes
  @remark


     if (0 == (ptf_graph_script_bytecode = fopen(GRAPH_TXT, "wt"))) exit(-1);
     if (0 == (ptf_graph_script = fopen(GRAPH_YML, "rt"))) exit(-1); idx = 0;
     while (1) if (0 == fread(script[idx++], 1, 1, ptf_graph_script)) break;

     arm_stream_script_assembler(script, idx, ptf_graph_script_bytecode, script_bytecode, &nbByteCodes);
     fclose(ptf_graph_script); fclose(ptf_graph_script_bytecode);

 */

void arm_stream_script_assembler (char *script, uint32_t length, FILE *ptf_graph_script_bytecode, char *script_bytecode, uint32_t *nbByteCodes)
{
 
}


#ifdef __cplusplus
}
#endif
