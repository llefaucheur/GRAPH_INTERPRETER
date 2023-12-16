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

#ifdef __cplusplus
 extern "C" {
#endif
    
#include "stream_tool_include.h"
#include <time.h>


/*  
    first letters of the lines in the input file 
*/
#define _comments    'c'
#define _offsets     'o'
#define _header      'h'
#define _format      'f'
#define _stream_inst 'i'
#define _linked_list 'l'
#define _script      's'
#define _arc         'a'
#define _RAM         'r'
#define _debug       'd'

char * dbgplus;
uint32_t addrBytes;
FILE *ptf_graph_dbg;
struct stream_graph_linkedlist *G;

void PRINTF(uint32_t d, char *c) 
{
    uint32_t i;
    char cplus[MAXNBCHAR_LINE];
    char line[MAXNBCHAR_LINE];

    if (addrBytes/4 >= 0x74 && addrBytes <0x80)
    {   i = 0;
    }

    strcpy(cplus, c);

    for (i = 0; i < G->idbg; i++)
    {
        if (addrBytes/4 == G->dbg[i].address)
        {   strcat(cplus, G->dbg[i].toprint);
        }
    }

    sprintf (line, "0x%08X, // %03X %03X %s \n", (d), addrBytes, addrBytes/4, cplus);
    fprintf(ptf_graph_dbg, "%s", line); 
    addrBytes += 4;
}

//
///* 
//    the number of coefficients per nodes can be large
//*/
//static uint32_t parameters[MAX_FPE4M6];


/* 
    list of all the memory banks, for debug
*/
#define MAX_MEMORY_BANK_DBG 1000
static uint32_t memory_banks[MAX_MEMORY_BANK_DBG], nb_memory_banks;


/**
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par              translates the graph intermediate format GraphTxt to GraphBin to be reused
                    in CMSIS-Stream/stream_graph/*.txt
  @remark

*/

void arm_stream_graphTxt2Bin (struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph, FILE *ptf_graph_bin)
{
    uint32_t FMT0, FMT1, FMT2;
    static uint32_t SC1, SC2, LK1, LK2;
    static fpos_t pos_NWords, pos_end;
    static uint32_t nFMT, LENscript, LinkedList, LinkedList0, NbInstance, nIOs, NBarc, SizeDebug, dbgScript;
    uint32_t i, j, i1, i2, fw_io_idx;
    char tmpstring[200];
    time_t rawtime;
    struct tm * timeinfo;
    //union {uint8_t indexes_fw_io_idx_u8[256]; 
    //       uint32_t indexes_fw_io_idx_u32[32]; } UIO;
    struct io_arcstruct *arcIO;

    addrBytes = 0;
    ptf_graph_dbg = ptf_graph_bin;
    G = graph;

    /*
        Graph data format :
        Header 6+1 words
        IO "stream_format_io" (2 words per IO)
        FORMAT used by the arcs (3 words each) 
        SCRIPTS in Flash 
        LINKED-LIST of SWC
        INSTANCE Pointers (27bits encoded format + bits used for synchonization at boot time and service activations)
        ARC descriptors (4 words each)
        DEBUG REGISTERS and vectors from ARC content analysis 
        BUFFERS memory banks (internal/external/LLRAM) used for FIFO buffers 
    */
    fprintf(ptf_graph_dbg, "//--------------------------------------\n"); 
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    fprintf(ptf_graph_dbg, "//  date/time: %s", asctime (timeinfo)); 
    fprintf(ptf_graph_dbg, "//  AUTOMATICALLY GENERATED CODES\n"); 
    fprintf(ptf_graph_dbg, "//  DO NOT MODIFY IT !\n"); 
    fprintf(ptf_graph_dbg, "//--------------------------------------\n"); 

    fgetpos (ptf_graph_bin, &pos_NWords);
    // @@@@ code badly duplicated at the end of this file 
        fprintf(ptf_graph_bin, "0x%08X, // number of words\n", addrBytes/4); // [-1] size of the graph in Words 
        FMT0 = 0;
        addrBytes = 0;
        sprintf(tmpstring, "0 destination of the graph in RAM"); 
        PRINTF(FMT0, tmpstring);      // [0] 27b RAM address of part/all the graph going in RAM, 

        FMT0 = 0; ST(FMT0, NBFORMATS_GR1, graph->nb_formats);
        ST(FMT0, NB_IOS_GR1, platform->nb_hwio_stream);
        ST(FMT0, SCRIPTS_SIZE_GR1, (SC2-SC1)/4);
        sprintf(tmpstring, "1 number of IO-Interfaces, size of the script section"); 
        PRINTF(FMT0, tmpstring);     // [1] number of FORMAT, size of SCRIPTS

        FMT0 = 0; ST(FMT0, LINKEDLIST_SIZE_GR2, (LK2-LK1)/4);
        sprintf(tmpstring, "2 size of the linked-list of nanoApp + parameters"); 
        PRINTF(FMT0, tmpstring);   // [2] size of LINKEDLIST

        FMT0 = 0; ST(FMT0, NB_ARCS_GR3, graph->nb_arcs);
        ST(FMT0, SCRIPT_SCTRL_GR3, graph->script_sctrl);
        sprintf(tmpstring, "3 number of arcs and debug/trace registers"); 
        PRINTF(FMT0, tmpstring);   // [3] number of ARCS, number of DEBUG registers

        FMT0 = 1; //platform->procid_allowed_gr4;
        sprintf(tmpstring, "4 list of processors allowed to execute the graph"); 
        PRINTF(FMT0, tmpstring); // [4] list of processors (procID for the scheduler in platform_manifest) processing the graph

        FMT0 = 0x000000FF;      // all the memory of the bank 0 is used, banks 1,2,3 are not used
        sprintf(tmpstring, "5 percentage of 8 memory banks consumed by the graph"); 
        PRINTF(FMT0, tmpstring); //[5,6] UQ8 portion of memory consumed on each long_offset[MAX_NB_MEMORY_OFFSET] 

        FMT0 = 0x00000000;      // banks 4,5,6,7 are not used
        PRINTF(FMT0, "6 '''''");      //


    /*
        STREAM IO   start from 1, fw_io_idx=0 is reserved for script controls
    */
    //nUIO8 = 4*((platform->nb_hwio_stream +3)/4);      /* indexes to FW IO IDX */
    //for (fw_io_idx = 1; fw_io_idx <= platform->nb_hwio_stream ; fw_io_idx++) 
    //{   
    //}


    for (fw_io_idx = 1; fw_io_idx <= platform->nb_hwio_stream ; fw_io_idx++) 
    {   
        uint8_t arcBuffer_shared_with_IO;
        FMT0 = FMT1 = 0xFFFFFFFFL;
        arcIO = &(graph->arcIO[fw_io_idx]);
        sprintf(tmpstring, "");

        if (arcIO->top_graph_index != 0xFFFF)
        {
            ST(FMT0,   IOARCID_IOFMT, arcIO->arcIDstream);
            ST(FMT0,    RX0TX1_IOFMT, arcIO->sc.rx0tx1);
            ST(FMT0,  SERVANT1_IOFMT, arcIO->si.commander0_servant1);
            ST(FMT0, SET0COPY1_IOFMT, arcIO->si.set0_copy1);
            arcBuffer_shared_with_IO = (0 != arcIO->si.graphalloc_X_bsp_0)? 1: 0;
            ST(FMT0, SHAREBUFF_IOFMT, arcBuffer_shared_with_IO);
            FMT1 = arcIO->si.settings;
            sprintf(tmpstring, "IO-Interface_%d", fw_io_idx); 
        }

        PRINTF(FMT0, tmpstring);
        PRINTF(FMT1, "-");
    }

    /*
        FORMAT used by the arcs (3 words each) 
    */
    for (i = 0; i < graph->nb_formats; i++)
    {   union 
        {   uint32_t u32;
            float f32;
        } U;
        float f;
        struct formatStruct *format;
        
        format = &(graph->arcFormat[i]);

        FMT0 = format->FMT0;
        FMT1 = format->FMT1;
        FMT2 = format->FMT2;

        U.u32 = FMT1;
        f = U.f32;
        
        sprintf(tmpstring, "Data and frame format %d   frame size %d   nb chan %d   sampling %6.2f", i,
            RD(FMT0,  FRAMESIZE_FMT0),
            RD(FMT1,  NCHANM1_FMT1) +1,
            f);

        PRINTF(FMT0, tmpstring);
        PRINTF(FMT1, "");
        PRINTF(FMT2, "");

    }
    
    /* 
        SCRIPTS in Flash 
    */
    SC1 = addrBytes;
    i1 = graph->nb_scripts;
    i1 = (1+i1)>>1;             // table of W16 round to W32
    for (j = i = 0; i < i1; i++)
    {   FMT0 = graph->script_indirect[j]; 
        FMT1 = ((long)(graph->script_indirect[j+1]))<<16; 
        FMT0 |= FMT1;
        j = j+2;
        if (i == 0) PRINTF(FMT0, "nb of scripts, indexes");
        else PRINTF(FMT0, "script index");
    }

    i1 = graph->nb_byte_code;
    i1 = (3+i1)>>2;             // round to W32
    for (j = i = 0; i < i1; i++)
    {   uint8_t *pt8 = &(graph->script_bytecode[j]);
        FMT0 = pt8[0]; 
        FMT1 = ((long)(pt8[1]))<<8; 
        FMT0 |= FMT1; 
        FMT1 = ((long)(pt8[2]))<<16;
        FMT0 |= FMT1; 
        FMT1 = ((long)(pt8[3]))<<24;
        FMT0 |= FMT1; 
        j = j+4;
        PRINTF(FMT0, "byte codes");
    }
    SC2 = addrBytes;
        
    /* 
        LINKED-LIST of SWC
    */
    LK1 = addrBytes;
    for (i = 0; i < graph->nb_nodes; i++)
    {   uint32_t iarc, imem;
        struct stream_node_manifest *n;
        struct arcStruct *arc;

        n = &(graph->all_nodes[i]);

        /* word 1 - main Header */
        strcpy(tmpstring, "-------------------- nanoApp : "); strcat(tmpstring, graph->all_nodes[i].nodeName);
        PRINTF(n->headerPACK, tmpstring);

        /* word 2 - arcs */
        for (iarc = 0; iarc < n->nbInputArc + n->nbOutputArc; iarc+=2)
        {
            arc = &(n->arc[iarc]);
            FMT0 = 0;
            ST(FMT0, ARC1_LW1, arc[iarc+1].arcID);
            ST(FMT0, ARC0_LW1, arc[iarc].arcID);

            if (iarc == 0) // the first ARC word holds the XDM11 flag
            {   int xdm11;
                xdm11 = platform->all_nodes[n->swc_idx].RWinSWC;
                ST(FMT0, XDM11_LW1, xdm11); 
            }
            ST(FMT0, ARC0D_LW1, (arc[iarc  ].sc.rx0tx1 >0));    /* set the rx0tx1 arc direction bit */
            ST(FMT0, ARC1D_LW1, (arc[iarc+1].sc.rx0tx1 >0));    

            PRINTF(FMT0, "Indexes of ARC 0 and 1 of the nanoApp");
        }

        /* word 3 - memory banks */
        for (imem = 0; imem < n->nbMemorySegment; imem++) 
        {
            FMT0 = 0;
            if (imem == 0) ST(FMT0, NBALLOC_LW2,  n->nbMemorySegment);  // instance pointer has the number of memory banks on MSBs
            if (n->defaultParameterSizeW32 > 1) ST(FMT0, PARAMETER_LW2, 1);
            ST(FMT0, BASEIDXOFFLW2, n->memreq[imem].graph_basePACK);    // address 
            sprintf(tmpstring, "Reserved memory bank(%d)", imem); 
            PRINTF(FMT0, tmpstring);
        }

        /* word 4 - parameters */
        for (j = 0; j < n->defaultParameterSizeW32; j++)
        {   
            if (j==0) sprintf(tmpstring, "(%d) boot parameters", j); 
            else sprintf(tmpstring, "(%d)", j); 
            PRINTF(n->PackedParameters[j], tmpstring);
        }
    }

    /* LAST WORD */
    PRINTF(GRAPH_LAST_WORD, "^^^^^^^^ FLASH ^^^^^^^^  vvvvvvvvv RAM vvvvvvvvv");
    LK2 = addrBytes;

    /*
        ARC descriptors (4 words each)
    */
    for (i = 0; i < graph->nb_arcs; i++)
    {   
        sprintf(tmpstring, "Circular buffer ARC descriptor(%d) ", i); 
        i2 = 0;
        for (j = 0; j < G->idbg; j++)
        {   i1 = RD(graph->arc[i].ARCW0, BASEIDX_ARCW0);
            if (i1 == G->dbg[j].address)
            {   strcat(tmpstring, G->dbg[j].toprint);
                if (0 == strncmp(G->dbg[j].toprint,"SC",2)) 
                    i2 = 1;
                if (0 == strncmp(G->dbg[j].toprint,"IO",2)) 
                    i2 = 1;
            }
        }

        if (i2==0) 
        {   sprintf(tmpstring, "Circular buffer ARC descriptor(%d) Producer %d Consumer %d", i,
                RD(graph->arc[i].ARCW0, PRODUCFMT_ARCW0),
                RD(graph->arc[i].ARCW1, CONSUMFMT_ARCW1)); 
        }

        PRINTF(graph->arc[i].ARCW0, tmpstring);
        PRINTF(graph->arc[i].ARCW1, "");
        PRINTF(graph->arc[i].ARCW2, "");
        PRINTF(graph->arc[i].ARCW3, "");
    }

    i1 = addrBytes/4;
    i2 = graph->end_binary_graph;
    i = i1;
    do
    {   PRINTF(graph->binary_graph[i], "");
        i++;
    } while (i < i2);
    fgetpos (ptf_graph_bin, &pos_end);


    fsetpos (ptf_graph_bin, &pos_NWords);   

    fprintf(ptf_graph_bin, "0x%08X, // number of words\n", addrBytes/4); // [-1] size of the graph in Words 
    addrBytes = 0;
    FMT0 = 0;
    ST(FMT0, RAMSPLIT_GRAPH0, graph->PackedFormat);
    sprintf(tmpstring, "0 destination of the graph in RAM"); 
    PRINTF(FMT0, tmpstring);      // [0] 27b RAM address of part/all the graph going in RAM, 

    FMT0 = 0; ST(FMT0, NBFORMATS_GR1, graph->nb_formats);
    ST(FMT0, NB_IOS_GR1, platform->nb_hwio_stream);
    ST(FMT0, SCRIPTS_SIZE_GR1, (SC2-SC1)/4);
    sprintf(tmpstring, "1 number of IO-Interfaces, size of the script section"); 
    PRINTF(FMT0, tmpstring);     // [1] number of FORMAT, size of SCRIPTS

    FMT0 = 0; ST(FMT0, LINKEDLIST_SIZE_GR2, (LK2-LK1)/4);
    sprintf(tmpstring, "2 size of the linked-list of nanoApp + parameters"); 
    PRINTF(FMT0, tmpstring);   // [2] size of LINKEDLIST

    FMT0 = 0; ST(FMT0, NB_ARCS_GR3, graph->nb_arcs);
    ST(FMT0, SCRIPT_SCTRL_GR3, graph->script_sctrl);
    sprintf(tmpstring, "3 number of arcs and debug/trace registers"); 
    PRINTF(FMT0, tmpstring);   // [3] number of ARCS, number of DEBUG registers

    FMT0 = 1; //platform->procid_allowed_gr4;
    sprintf(tmpstring, "4 list of processors allowed to execute the graph"); 
    PRINTF(FMT0, tmpstring); // [4] list of processors (procID for the scheduler in platform_manifest) processing the graph

    FMT0 = 0x000000FF;      // all the memory of the bank 0 is used, banks 1,2,3 are not used
    sprintf(tmpstring, "5 percentage of 8 memory banks consumed by the graph"); 
    PRINTF(FMT0, tmpstring); //[5,6] UQ8 portion of memory consumed on each long_offset[MAX_NB_MEMORY_OFFSET] 

    FMT0 = 0x00000000;      // banks 4,5,6,7 are not used
    PRINTF(FMT0, "6 '''''");      //

    fsetpos (ptf_graph_bin, &pos_end);  
}

#ifdef __cplusplus
}
#endif