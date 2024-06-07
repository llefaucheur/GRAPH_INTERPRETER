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
uint32_t addrRAM;
uint32_t addrBytes;
FILE *ptf_graph_dbg;
struct stream_graph_linkedlist *G;

void PRINTF(uint32_t d, char *c) 
{
    //int32_t iRAM;
    //uint32_t i;
    //char cplus[NBCHAR_LINE];
    //char line[NBCHAR_LINE];

    //if (addrBytes/4 >= 0x74 && addrBytes <0x80)
    //{   i = 0;
    //}

    //strcpy(cplus, c);

    //for (i = 0; i < G->idbg; i++)
    //{
    //    if (addrBytes == G->dbg[i].address)
    //    {   strcat(cplus, G->dbg[i].toprint);
    //    }
    //}

    //iRAM = (int)addrBytes - (int)addrRAM;
    //if (iRAM < 0) iRAM = 0;
    //sprintf (line, "0x%08X, // %03X %03X %03X %s \n", (d), addrBytes, iRAM, iRAM/4, cplus);
    //fprintf(ptf_graph_dbg, "%s", line); 
    //addrBytes += 4;
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
    uint32_t FMT0, FMT1, FMT2, FMT3, ARCW0, ARCW1, ARCW2, ARCW3;
    static uint32_t SC1, SC2, LK1, LK2;
    static fpos_t pos_NWords, pos_end;
    static uint32_t nFMT, LENscript, LinkedList, LinkedList0, NbInstance, nIOs, NBarc, SizeDebug, dbgScript;
    uint32_t i, j, i1, i2, fw_io_idx;
    char tmpstring[200];
    time_t rawtime;
    struct tm * timeinfo;
    //union {uint8_t indexes_fw_io_idx_u8[256]; 
    //       uint32_t indexes_fw_io_idx_u32[32]; } UIO;
    struct arcStruct *arcIO;
    struct arcStruct *arc;

    addrBytes = 0;
    ptf_graph_dbg = ptf_graph_bin;
    G = graph;
    addrRAM = graph->offsetToRam;

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
        sprintf(tmpstring, "2 size of the linked-list of nanoAppRT + parameters"); 
        PRINTF(FMT0, tmpstring);   // [2] size of LINKEDLIST

        FMT0 = 0; ST(FMT0, NB_ARCS_GR3, graph->nb_arcs);
        ST(FMT0, SCRIPT_SCTRL_GR3, 0 /*graph->script_sctrl */);
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

    for (fw_io_idx = 0; fw_io_idx < platform->nb_hwio_stream ; fw_io_idx++) 
    {   
        uint8_t arcBuffer_shared_with_IO;
        FMT0 = FMT1 = 0xFFFFFFFFL;
        arcIO = &(graph->arc[fw_io_idx]);
        sprintf(tmpstring, "");

        if (arcIO->idx_arc_in_graph != 0xFFFF)
        {
            ST(FMT0, QUICKSKIP_IOFMT, 0);
            ST(FMT0,   IOARCID_IOFMT, arcIO->idx_arc_in_graph);
            ST(FMT0,    RX0TX1_IOFMT, arcIO->format.rx0tx1);
            ST(FMT0,  SERVANT1_IOFMT, arcIO->format.commander0_servant1);
            ST(FMT0, SET0COPY1_IOFMT, arcIO->format.set0_copy1);
            arcBuffer_shared_with_IO = (0 != arcIO->format.graphalloc_X_bsp_0)? 1: 0;
            ST(FMT0, SHAREBUFF_IOFMT, arcBuffer_shared_with_IO);
            FMT1 = arcIO->settings;
            sprintf(tmpstring, "IO-Interface(%d) set0_copy1=%d arc(%d) direction_rx0tx1=%d", fw_io_idx, arcIO->format.set0_copy1, arcIO->idx_arc_in_graph, arcIO->format.rx0tx1); 
        }
        else
        {
            ST(FMT0, QUICKSKIP_IOFMT, 1);
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
        
        FMT0 = 0;
        format = &(graph->arcFormat[i]);
        ST(FMT0, FRAMESIZE_FMT0, format->frame_length);

        FMT1 = 0;
        ST(FMT1, RAW_FMT1, format->raw_data);
        ST(FMT1, SUBTYPE_FMT1, 0);  //@@@@@@@@
        ST(FMT1, DOMAIN_FMT1,  0);
        ST(FMT1, TSTPSIZE_FMT1,0);
        ST(FMT1, TIMSTAMP_FMT1, 0);
        ST(FMT1, INTERLEAV_FMT1, format->deinterleaved);
        ST(FMT1, NCHANM1_FMT1, format->nchan -1);  

        FMT2 = 0;
        FMT3 = 0;
  
        
        //sprintf(tmpstring, "Data and frame format %d   frame size %d   nb chan %d   sampling %6.2f", i,
        //    RD(FMT0,  FRAMESIZE_FMT0),
        //    RD(FMT1,  NCHANM1_FMT1) +1,
        //    f);

        PRINTF(FMT0, tmpstring);
        PRINTF(FMT1, "");
        PRINTF(FMT2, "");
        PRINTF(FMT3, "");

    }
    
    /* 
        SCRIPTS in Flash     table of 2x W32      word0 : offset, word1:length + byteCode Format
    */
    //SC1 = addrBytes;
   
    //for (j = i = 0; i < graph->nb_scripts; i++)
    //{   FMT0 = graph->script_offset[j]; 
    //    PRINTF(FMT0, "script offset");
    //    FMT0 = 0; 
    //    PRINTF(FMT0, "script length");
    //}

    //i1 = graph->nb_byte_code;
    //i1 = (3+i1)>>2;             // round to W32
    //for (j = i = 0; i < i1; i++)
    //{   uint8_t *pt8 = &(graph->script_bytecode[j]);
    //    FMT0 = pt8[0]; 
    //    FMT1 = ((long)(pt8[1]))<<8; 
    //    FMT0 |= FMT1; 
    //    FMT1 = ((long)(pt8[2]))<<16;
    //    FMT0 |= FMT1; 
    //    FMT1 = ((long)(pt8[3]))<<24;
    //    FMT0 |= FMT1; 
    //    j = j+4;
    //    PRINTF(FMT0, "byte codes");
    //}
    //SC2 = addrBytes;
        
    /* 
        LINKED-LIST of SWC
    */
    LK1 = addrBytes;
    for (i = 0; i < graph->nb_nodes; i++)
    {   uint32_t iarc, imem;
        struct stream_node_manifest *n;
        struct arcStruct *arc;

        n = &(graph->all_nodes[i]);

        /* word 0 - main Header */
        strcpy(tmpstring, "-------------------- nanoAppRT : "); strcat(tmpstring, graph->all_nodes[i].nodeName);
        PRINTF(n->headerPACK, tmpstring);

        /* word 1 - arcs */
        for (iarc = 0; iarc < n->nbInputArc + n->nbOutputArc; iarc+=2)
        {
            arc = &(n->arc[iarc]);
            FMT0 = 0;
            ST(FMT0, ARC1_LW1, arc[iarc+1].arcID);
            ST(FMT0, ARC0_LW1, arc[iarc].arcID);

            ST(FMT0, ARC0D_LW1, (arc[iarc  ].format.rx0tx1 >0));    /* set the direction_rx0tx1 arc direction bit */
            ST(FMT0, ARC1D_LW1, (arc[iarc+1].format.rx0tx1 >0));    

            ST(FMT0, DBGB1_LW1, arc[iarc+1].debug_page_DBGB0_LW1);  
            ST(FMT0, DBGB0_LW1, arc[iarc].debug_page_DBGB0_LW1);

            PRINTF(FMT0, "Indexes of ARC 0 and 1 of the nanoAppRT");
        }

        /* word 2 - memory banks */
        for (imem = 0; imem < n->nbMemorySegment; imem++) 
        {
            FMT0 = 0;
            if (imem == 0) 
            {   ST(FMT0, NBALLOC_LW2,  n->nbMemorySegment);  // instance pointer has the number of memory banks on MSBs
                if (iarc == 0)                               // the first memreq word holds the XDM11 flag
                {   int xdm11;
                    xdm11 = platform->all_nodes[n->swc_idx].RWinSWC;
                    ST(FMT0, XDM11_LW2, xdm11); 
                }
            }
            //if (n->ParameterSizeW32 > 1) ST(FMT0, PARAMETER_LW2, 1);
            ST(FMT0, BASEIDXOFFLW2, n->memreq[imem].graph_basePACK);    // address 
            sprintf(tmpstring, "Reserved memory bank(%d)", imem); 
            PRINTF(FMT0, tmpstring);

            FMT0 = 0;   //@@@@@
            sprintf(tmpstring, "size of the memory bank(%d)", imem); 
            PRINTF(FMT0, tmpstring);
        }

        /* word 4 - parameters */
        for (j = 0; j < n->ParameterSizeW32; j++)
        {   
            if (j==0) sprintf(tmpstring, "(%d) boot parameters", j); 
            else sprintf(tmpstring, "(%d)", j); 
            PRINTF(n->PackedParameters[j], tmpstring);
        }
    }

    /* LAST WORD */
    PRINTF(GRAPH_LAST_WORD, "-------------------  vvvvvvvvv RAM vvvvvvvvv");
    LK2 = addrBytes;

    /*
        ARC descriptors (4 words each)
    */
//        minFrameSize = (int)(0.5 + (IOSizeMulfac * minFrameSize));      /* buffer size rescaling, Byte accurate */
//        /* check it is larger than what the IO-Interface needs */
//
//        arcIO = &(graph->arcIO[fw_io_idx]);
//        if (minFrameSize < arcIO->si.graphalloc_X_bsp_0 * minFrameSize)   /* @@@ to check with a test graphTxt */
//        {   minFrameSize = arcIO->si.graphalloc_X_bsp_0 * minFrameSize;
//        }
//
//        arcBufferSizeByte = minFrameSize;               /* in Bytes */           
//
//        // @@@@ arc sequence starting with the one used for locking, the streaming arcs, then the metadata arcs 
//        // @@@@ arc(tx) used for locking is ARC0_LW1
//
//        ST(arc->ARCW0, PRODUCFMT_ARCW0, ProdFMT);
//        ST(arc->ARCW0,   DATAOFF_ARCW0, 0);
//        ST(arc->ARCW0,   BASEIDX_ARCW0, 0);         /* fill the base address after all arcs are read */
//
//        arc->memVID = VID;
//
//        ST(arc->ARCW1, CONSUMFMT_ARCW1, ConsFMT);
//        ST(arc->ARCW1,   MPFLUSH_ARCW1, FLUSH);
//        ST(arc->ARCW1, DEBUG_REG_ARCW1, REG);
//        ST(arc->ARCW1, BUFF_SIZE_ARCW1, minFrameSize);
//
//        ST(arc->ARCW2, COMPUTCMD_ARCW2, CMD);
//        ST(arc->ARCW2, FLOWERROR_ARCW2, OVF);
//        ST(arc->ARCW2,    EXTEND_ARCW2, EXTD);
//        ST(arc->ARCW2,      READ_ARCW2, 0);
//
//        ST(arc->ARCW3, COLLISION_ARCW3, 0);
//        ST(arc->ARCW3, ALIGNBLCK_ARCW3, 0);
//        ST(arc->ARCW3,     WRITE_ARCW3, 0);
    for (i = 0; i < graph->nb_arcs; i++)
    {   
        sprintf(tmpstring, "Circular buffer ARC descriptor(%d) ", i); 
        i2 = 0;
        for (j = 0; j < G->idbg; j++)
        {   i1 = RD(graph->arc[i].ARCW0, BASEIDX_ARCW0);
            i1 = i1 * 4; /* BASEIDX_ARCW0 is in WORD32 */
            if (i1 == G->dbg[j].address)
            {   strcat(tmpstring, G->dbg[j].toprint);
                if (0 == strncmp(G->dbg[j].toprint,"SC",2)) 
                    i2 = 1;
                if (0 == strncmp(G->dbg[j].toprint,"IO",2)) 
                    i2 = 1;
            }
        }

        arc = &(graph->arc[i]);

        ST(ARCW0, PRODUCFMT_ARCW0, 0);
        ST(ARCW0,   DATAOFF_ARCW0, 0);
        ST(ARCW0,   BASEIDX_ARCW0, 0);         /* fill the base address after all arcs are read */


        ST(ARCW1, CONSUMFMT_ARCW1, 0);
        ST(ARCW1,   MPFLUSH_ARCW1, 0);
        ST(ARCW1, DEBUG_REG_ARCW1, 0);
        ST(ARCW1, BUFF_SIZE_ARCW1, arc->format.frame_length);

        ST(ARCW2, COMPUTCMD_ARCW2, 0);
        ST(ARCW2, FLOWERROR_ARCW2, 0);
        ST(ARCW2,    EXTEND_ARCW2, 0);
        ST(ARCW2,      READ_ARCW2, 0);

        ST(ARCW3, COLLISION_ARCW3, 0);
        ST(ARCW3, ALIGNBLCK_ARCW3, 0);
        ST(ARCW3,     WRITE_ARCW3, 0);



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
    ST(FMT0, RAMSPLIT_GR0, graph->PackedFormat);
    sprintf(tmpstring, "0 destination of the graph in RAM"); 
    PRINTF(FMT0, tmpstring);      // [0] 27b RAM address of part/all the graph going in RAM, 

    FMT0 = 0; ST(FMT0, NBFORMATS_GR1, graph->nb_formats);
    ST(FMT0, NB_IOS_GR1, platform->nb_hwio_stream);
    ST(FMT0, SCRIPTS_SIZE_GR1, (SC2-SC1)/4);
    sprintf(tmpstring, "1 number of IO-Interfaces, size of the script section"); 
    PRINTF(FMT0, tmpstring);     // [1] number of FORMAT, size of SCRIPTS

    FMT0 = 0; ST(FMT0, LINKEDLIST_SIZE_GR2, (LK2-LK1)/4);
    sprintf(tmpstring, "2 size of the linked-list of nanoAppRT + parameters"); 
    PRINTF(FMT0, tmpstring);   // [2] size of LINKEDLIST

    FMT0 = 0; ST(FMT0, NB_ARCS_GR3, graph->nb_arcs);
    ST(FMT0, SCRIPT_SCTRL_GR3, 0 /* graph->script_sctrl*/);
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

    // fsetpos (ptf_graph_bin, &pos_end);  
}




/**=================================================================================================
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par              compute the graph memory map

  reserve memory for 
    1) arc descriptors for debug pages 
    2) arc descriptors of scripts and streams
    3) rescan the arc and malloc the buffers => transate in 27b addresses
    4) malloc the node instances => transate in 27b addresses

  @remark
    ARC descriptors (4 words each) start after other data :
        [0] byte array of  LSB(fw_io_idx) = ioctrl [graph_io_idx], MSB="on=going"
            data to copy from Graph[GRAPH_HEADER_NBWORDS]   to S-> all_arcs[0], size = [NB16IOSTREAM_GR3 x 4]
        [COLLISION_IDX_GR2] reserved for collision management (Dekker's algorithm) 
        [ARC_DEBUG_IDX_GR2] debug registers (2 words each) DEBUG REGISTERS and vectors from ARC content 
            analysis (DEBUG_REG_ARCW1)
            32 memory banks of 16bytes + 64bytes in normal and critical fast memory when possible
        [SERVICE_RAM_IDX_GR2] shared between all instances (maximum 5kB)

 */

void arm_stream_memory_map (struct stream_platform_manifest *platform, 
                            struct stream_graph_linkedlist *graph)
{
    uint32_t first_arc = 1;
    intPtr_t RamW32;
    intPtr_t arcBufferSizeByte;
    uint32_t pack27b, N, iscripts, iAllArcs, inode, sharedArc;
    intPtr_t size;
    uint8_t ptr8, dbgcmd, dbgreg, ovf, unf, first_script_sharedRAM, imemreq;
    struct arcStruct *arc;
    struct stream_node_manifest *node;

    size = RamW32 = 0; 

    /* ======= memory for FIFO descriptors ======= */
    if (graph->nb_debug_pages > 0)                      /* are there debug data to save ? */
    {                                                   /* arc0 is used to address the 16x64b debug registers pages */
        size = 4 *(ARC_DBG_REGISTER_SIZE_W32 * graph->nb_debug_registers);
        vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, MEM_TYPE_STATIC, &pack27b, &ptr8, platform, graph);
    }
    RamW32 += (size +3)>>2;

    N = graph->nb_scripts + graph->nb_arcs;
    size = SIZEOF_ARCDESC_W32 * N;                      /* reserve the descriptor area (all the arcs) */
    vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, MEM_TYPE_STATIC, &pack27b, &ptr8, platform, graph);
    RamW32 += (size +3)>>2;

    /* ======= memory for FIFO buffers ======= */


    /* ======= memory for scripts ======= */
    N = graph->max_shared_stackW32; // + graph->max_nregs;
    size = ((SCRIPT_REGSIZE +1) * N);                /* reserve the shared area of scripts */
    vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, MEM_TYPE_STATIC, &pack27b, &ptr8, platform, graph);


    iAllArcs = 0;
    first_script_sharedRAM = 1;
    //for (iscripts = 0; iscripts < graph->nb_scripts; iscripts++)
    //{   uint32_t RegAndStackW32, stackdepthW32;
    //    dbgcmd = dbgreg = ovf = unf = 0;

    //    if (0 == graph->scriptRAMshared[iscripts] || first_script_sharedRAM)
    //    {   if (first_script_sharedRAM && (0 != graph->scriptRAMshared[iscripts]))
    //        {   first_script_sharedRAM = 0;
    //            sharedArc = graph->nb_arcs;
    //        }

    //        RegAndStackW32 = graph->script_nregs[iscripts];
    //        stackdepthW32 = graph->script_stackdepthW32[iscripts];
    //        arcBufferSizeByte = RegAndStackW32 * (SCRIPT_REGSIZE +1);   /* register type = 1 byte*/
    //        arcBufferSizeByte+= stackdepthW32 * (SCRIPT_REGSIZE +1);    /* stack element on 9 bytes too */
    //        size = ((arcBufferSizeByte +3)>>2)<<2;         /* round to the closest W32 */
    //        vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, &pack27b, &ptr8, platform, graph);

    //        arc = &(graph->arc[iAllArcs]);
    //        fill_arc(arc, pack27b, (uint32_t)size,   0, 0, 0,  dbgcmd, dbgreg, ovf, "");
    //        iAllArcs ++;
    //        /* second sharedRAM script : reuse the RAM of the first shared script */
    //    }
    //}

    /* ======= memory for node instances ======= */
    for (inode = 0; inode < graph->nb_arcs; inode++)
    {   
        node = &(graph->all_nodes[inode]);
        for (imemreq = 0; imemreq < node->nbMemorySegment; imemreq++)
        {
            compute_memreq(node, graph->arcFormat);

            size = node->memreq[imemreq].graph_memreq_size;
            vid_malloc(node->memreq[imemreq].VID, size, MEM_REQ_4BYTES_ALIGNMENT, MEM_TYPE_STATIC, &(node->memreq[imemreq].graph_basePACK), 
                    &ptr8, platform, graph);
        }
    }

}


#ifdef __cplusplus
}
#endif