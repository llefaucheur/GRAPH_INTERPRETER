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


#include <stdint.h>
#include <time.h>

#include "../stream_src/stream_types.h"
#include "stream_tool_include.h"
#include "../stream_nodes/arm/script/arm_stream_script_instructions.h"


/**
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par              translates the graph intermediate format GraphTxt to GraphBin to be reused
                    in CMSIS-Stream/stream_graph/*.txt
  @remark

*/

void arm_stream_graphTxt2Bin (struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph, FILE *ptf_graph_bin, char* ggraph_source)
{
    uint32_t FMT0, FMT1, FMT2, FMT3, FMT4, FMT5; 
    uint32_t ARCW[SIZEOF_ARCDESC_W32];
    static uint32_t SC1, SC2, LK0, LK1, LK2, LKscripts, LK_lkSize, LKalloc, LK_PIO, ENDARCS, addrW32s_backup;
    static fpos_t pos_NWords, pos_end;
    static uint32_t nFMT, LENscript, LinkedList, LinkedList0, NbInstance, nIOs, NBarc, SizeDebug, dbgScript;
    uint32_t j, m;
    char tmpstring[NBCHAR_LINE], tmpstring2[NBCHAR_LINE], tmpstring3[NBCHAR_LINE];
    struct arcStruct *arc;
    struct stream_node_manifest *node;
    uint32_t addrW32s, last_addrW32s;
    uint32_t packxxb;
    time_t rawtime;
    struct tm * timeinfo;
    uint32_t all_buffersW32, iarc, inode, iscript, iformat, clearswap_lw2s;
    stream_script_t *pscript;

    packxxb = FMT4 = FMT5 = all_buffersW32 = addrW32s = 0;

#define GTEXT(T) sprintf(graph->binary_graph_comments[addrW32s], "%s", T);
#define GTEXTINC(T) GTEXT(T) addrW32s += 1;
#define GWORD(W) graph->binary_graph[addrW32s] = W;
#define GWORDINC(W) GWORD(W) addrW32s += 1;
#define GINC    addrW32s += 1;
#define HCDEF_SSDC(N,N2,D,C) fprintf(graph->ptf_header,"#define %s_%s 0x%X //%s\n", N,N2,D,C);
#define HCNEWLINE() fprintf(graph->ptf_header,"\n");
#define HCTEXT(T) fprintf(graph->ptf_header,"// %s \n",T);// fprintf(graph->ptf_header,"\n");

#define MAP_TO_SPECIFIC_VID_BANK(VID) ((unsigned)(graph->option_graph_locations[VID]) < MAX_VID_FOR_MALLOC)

    fprintf(graph->ptf_graph_bin, "//--------------------------------------\n"); 
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    fprintf(graph->ptf_graph_bin, "//  DATE %s", asctime(timeinfo));
    fprintf(graph->ptf_graph_bin, "//  AUTOMATICALLY GENERATED CODES\n");
    fprintf(graph->ptf_graph_bin, "//  DO NOT MODIFY !\n"); 
    fprintf(graph->ptf_graph_bin, "//--------------------------------------\n");
    fprintf(graph->ptf_graph_bin, "//  Source %s \n", ggraph_source);
    fprintf(graph->ptf_graph_bin, "//--------------------------------------\n");

    fprintf(graph->ptf_header, "//--------------------------------------\n"); 
    fprintf(graph->ptf_header, "//  DATE %s", asctime (timeinfo)); 
    fprintf(graph->ptf_header, "//  AUTOMATICALLY GENERATED CODES\n"); 
    fprintf(graph->ptf_header, "//  DO NOT MODIFY !\n"); 
    fprintf(graph->ptf_header, "//--------------------------------------\n");
    fprintf(graph->ptf_header, "//  Source %s\n", ggraph_source);
    fprintf(graph->ptf_header, "//--------------------------------------\n");


    /* ------------------------------------------------------------------------------------------------------------------------------
       HEADER
     */
  

    LKalloc = addrW32s + 2;      // graph memory consumption computed at the end 
    
    sprintf(tmpstring, "header : size of the graph, compression used");             GTEXT(tmpstring); GWORDINC(0);
    sprintf(tmpstring, "interpreter version");                                      GTEXT(tmpstring); GWORDINC(GRAPH_INTERPRETER_VERSION);
    sprintf(tmpstring, "memory consumption in bank 0-3 (0xFF = 100%%, 0x3F = 25%%)"); GTEXT(tmpstring); GWORDINC(0);
    sprintf(tmpstring, "bank 4-7  (banks of long_offset[4-7])");                    GTEXT(tmpstring); GWORDINC(0);
    sprintf(tmpstring, "bank 8-11 ");                                               GTEXT(tmpstring); GWORDINC(0);
    sprintf(tmpstring, "bank 12-15");                                               GTEXT(tmpstring); GWORDINC(0);
    addrW32s = GRAPH_HEADER_POINTERS_NBWORDS;   // 20
    /*  ----------------------------------------------------------------------------------------------------------------------------------   
        [0] PIO HW decoding table
            [1] PIO Graph table, STREAM_IO_CONTROL (4 words per IO)
            [2] Scripts (when used with indexes)
            [3] Graph Linked-list of nodes
            [4] "on going" flags
            [5] stream formats
            [6] arc descriptors
        
        translation table HW->graph (platform_io_al_idx_to_stream)
        size in NBHWIOIDX_GR0
        ----------------------------------------------------------------------------------------------------------------------------------
    */
    m = platform->max_io_al_idx;
    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_PIO_HW))
    {   sprintf(tmpstring3, "GRAPH_PIO_HW to MEMID %d", graph->option_graph_locations[GRAPH_PIO_HW]);
        vid_malloc(graph->option_graph_locations[GRAPH_PIO_HW],     // MEMID
                   4 * m,                                           // one word per HW IO
                   MEM_REQ_4BYTES_ALIGNMENT, 
                   &packxxb, MEM_TYPE_STATIC, 
                   tmpstring3, platform, graph);
        #define AFTER_INDEXES 0
        sprintf(tmpstring2, " position %08X", packxxb + AFTER_INDEXES);
        strcat(tmpstring3, tmpstring2);
    }
    else
    {   sprintf(tmpstring3, "GRAPH_PIO_HW used from here");
        ST(packxxb, COPY_IN_RAM_FMT0, INPLACE_ACCESS_TAG); ST(packxxb, SIZE_EXT_OFF_FMT0, addrW32s); // read from here
    }
    addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_PIO_HW;
    GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
    GWORDINC(m);                                                    // size in W32
    addrW32s = addrW32s_backup;  

    for (j = 0; j < platform->max_io_al_idx; j++)
    {   
        // ;IO_AL_idx=0 is used for sinking (+rewinding script) without consumer node 
        if (j == 0)
        {   platform->IO_arc[j].arc_graph_ID = NOT_CONNECTED_TO_GRAPH;
        }
        FMT0 = 0;
        ST(FMT0,      PRIORITY_IO_CONTROL, 0);
        ST(FMT0,        PROCID_IO_CONTROL, platform->IO_arc[j].procID);
        ST(FMT0,        ARCHID_IO_CONTROL, platform->IO_arc[j].archID);
        ST(FMT0, IDX_TO_STREAM_IO_CONTROL, platform->IO_arc[j].arc_graph_ID);
        if (platform->IO_arc[j].arc_graph_ID == NOT_CONNECTED_TO_GRAPH)
        {   sprintf(tmpstring, "IO(HW%d) Not Connected ProcID_%d archID_%d", j, platform->IO_arc[j].procID, platform->IO_arc[j].archID);
        }
        else
        {   sprintf(tmpstring, "IO(HW%d) GraphID(%d)    ProcID_%d archID_%d %s", j, platform->IO_arc[j].arc_graph_ID,
                platform->IO_arc[j].procID, platform->IO_arc[j].archID, platform->IO_arc[j].manifest_file);
        }
        GTEXT(tmpstring); GWORDINC(FMT0);
    }   

    /*  ----------------------------------------------------------------------------------------------------------------------------------   
        PIO settings (4 words per IO) 
            depends on the domain of the IO
   
            [0] PIO HW decoding table
        [1] PIO Graph table, STREAM_IO_CONTROL (4 words per IO)
            [2] Scripts (when used with indexes)
            [3] Graph Linked-list of nodes
            [4] "on going" flags
            [5] stream formats
            [6] arc descriptors
        ----------------------------------------------------------------------------------------------------------------------------------
    */
    for (NBarc = iarc = 0; iarc < graph->nb_arcs; iarc++)
    {   arc = &(graph->arc[iarc]);
        if (arc->ioarc_flag) NBarc++;
    }

    m = STREAM_IOFMT_SIZE_W32 * NBarc;
    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_PIO_GRAPH))
    {   sprintf(tmpstring3, "GRAPH_PIO_GRAPH to MEMID %d", graph->option_graph_locations[GRAPH_PIO_GRAPH]);
        vid_malloc(graph->option_graph_locations[GRAPH_PIO_GRAPH],     // MEMID
                   4 * m,                  // four W32 per graph IO
                   MEM_REQ_4BYTES_ALIGNMENT, 
                   &packxxb, MEM_TYPE_STATIC, 
                   tmpstring3, platform, graph);
        sprintf(tmpstring2, " position %08X", packxxb);
        strcat(tmpstring3, tmpstring2);
    }
    else
    {   sprintf(tmpstring3, "GRAPH_PIO_GRAPH used from here");
        ST(packxxb, COPY_IN_RAM_FMT0, INPLACE_ACCESS_TAG); ST(packxxb, SIZE_EXT_OFF_FMT0, addrW32s); // read from here
    }
    addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_PIO_GRAPH;
    GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
    GWORDINC(m);                                                    // size in W32
    addrW32s = addrW32s_backup; 

    for (j = iarc = 0; iarc < graph->nb_arcs; iarc++)
    {   
        arc = &(graph->arc[iarc]);
        if (arc->ioarc_flag)
        {   
            FMT0 = FMT1 = FMT2 = FMT3 = 0;

            ST(FMT0,   FWIOIDX_IOFMT0, arc->fw_io_idx);
            ST(FMT0, SET0COPY1_IOFMT0, arc->set0copy1);
            ST(FMT0, BUFFALLOC_IOFMT0, arc->buffalloc);
            ST(FMT0,  SERVANT1_IOFMT0, arc->commander0_servant1);
            ST(FMT0,    RX0TX1_IOFMT0, arc->rx0tx1);
            ST(FMT0,   IOARCID_IOFMT0, arc->idx_arc_in_graph);

            sprintf(tmpstring, "IO(graph%d) %d arc %d set0copy1=%d rx0tx1=%d servant1 %d buffer allocation %d", 
                j++, arc->fw_io_idx,arc->idx_arc_in_graph, arc->set0copy1, arc->rx0tx1, arc->commander0_servant1, arc->buffalloc);
            GTEXT(tmpstring); GWORDINC(FMT0);

            sprintf(tmpstring, "IO(settings %d, fmtProd %d (L=%d) fmtCons %d (L=%d)", FMT1, graph->arc[iarc].fmtProd, (int)(graph->arcFormat[graph->arc[iarc].fmtProd].frame_length_bytes), 
                graph->arc[iarc].fmtCons, (int)(graph->arcFormat[graph->arc[iarc].fmtCons].frame_length_bytes));

            //switch(arc->domain)
            //{   case IO_DOMAIN_GENERAL:break;
            //    case IO_DOMAIN_AUDIO_IN:    pack_AUDIO_IN_IO_setting(arc);break;
            //    case IO_DOMAIN_AUDIO_OUT:   pack_AUDIO_OUT_IO_setting(arc);break;
            //    case IO_DOMAIN_GPIO_IN:break;
            //    case IO_DOMAIN_GPIO_OUT:break;
            //    case IO_DOMAIN_MOTION:      pack_motion_IO_setting(arc);break;
            //    case IO_DOMAIN_2D_IN:       pack_2D_IN_IO_setting(arc);break;
            //    case IO_DOMAIN_2D_OUT:      pack_2D_OUT_IO_setting(arc);break;
            //    case IO_DOMAIN_ANALOG_IN:   pack_ANALOG_IN_IO_setting(arc);break;
            //    case IO_DOMAIN_ANALOG_OUT:  pack_ANALOG_OUT_IO_setting(arc);break;
            //    case IO_DOMAIN_RTC:break;
            //    case IO_DOMAIN_USER_INTERFACE_IN:break;
            //    case IO_DOMAIN_USER_INTERFACE_OUT:break;
            //}

            FMT1 = arc->settings[0]; 
            FMT2 = arc->settings[1]; 
            FMT3 = arc->settings[2]; 
            GTEXT(tmpstring); GWORDINC(FMT1);   strcpy(tmpstring, "");
            GTEXT(tmpstring); GWORDINC(FMT2);   strcpy(tmpstring, "");
            GTEXT(tmpstring); GWORDINC(FMT3);   strcpy(tmpstring, "");
        }
    }

    /*  ----------------------------------------------------------------------------------------------------------------------------------
        SUBROUTINE- SCRIPTS : indexed with a table before the codes
   
            [0] PIO HW decoding table
            [1] PIO Graph table, STREAM_IO_CONTROL (4 words per IO)
        [2] Scripts (when used with indexes)
            [3] Graph Linked-list of nodes
            [4] "on going" flags
            [5] stream formats
            [6] arc descriptors
        ----------------------------------------------------------------------------------------------------------------------------------
    */
    for (m = iscript = 0; iscript < graph->nb_scripts; iscript++)
    {   m = m + graph->all_scripts[iscript].script_nb_instruction;
    }

    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_SCRIPTS))
    {   sprintf(tmpstring3, "GRAPH_SCRIPTS to MEMID %d", graph->option_graph_locations[GRAPH_SCRIPTS]);
        vid_malloc(graph->option_graph_locations[GRAPH_SCRIPTS],    // MEMID
                   4 * m,                                           // script cumulated size
                   MEM_REQ_4BYTES_ALIGNMENT, 
                   &packxxb, MEM_TYPE_STATIC, 
                   tmpstring3, platform, graph);
        sprintf(tmpstring2, " position %08X", packxxb);
        strcat(tmpstring3, tmpstring2);
    }
    else
    {   sprintf(tmpstring3, "GRAPH_SCRIPTS used from here");
        ST(packxxb, COPY_IN_RAM_FMT0, INPLACE_ACCESS_TAG); ST(packxxb, SIZE_EXT_OFF_FMT0, addrW32s); // read from here
    }
    addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_SCRIPTS;
    GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
    GWORDINC(m);                                                    // size in W32
    addrW32s = addrW32s_backup; 

    {   uint32_t offsetW = 0, Base = addrW32s, PreviousEnd;

        /* table of nb_scripts W32
        *    code[0]
        * .. code[nb_scripts-1]
        */
        PreviousEnd = Base + graph->nb_scripts;

        for (iscript = 0; iscript < graph->nb_scripts; iscript++)
        {   iarc = graph->all_scripts[iscript].arc_script;
            arc = &(graph->arc[iarc]);
            pscript = &(graph->all_scripts[iscript]);

            FMT0 = 0;   
            ST(FMT0, OFFSET_SCROFF0, FMT1 = PreviousEnd - Base);
            ST(FMT0, SHARED_SCROFF0, FMT2 = pscript->stack_memory_shared);
            ST(FMT0, FORMAT_SCROFF0, FMT3 = pscript->script_format);
            ST(FMT0, ARC_SCROFF0,    FMT4 = iarc);
            
            sprintf(tmpstring, "Script %d CodeSize %d (%Xh) shared%d format%d arc %d", 
                iscript, pscript->script_nb_instruction, PreviousEnd, FMT2, FMT3, FMT4); 
            GTEXT(tmpstring); GWORDINC(FMT0);

            PreviousEnd = PreviousEnd + graph->all_scripts[iscript].script_nb_instruction;

            graph->all_scripts[iscript].script_offset = offsetW;                  // prepare the static memory allocation
            offsetW = offsetW + graph->all_scripts[iscript].script_nb_instruction;

            pscript->nbw32_allocated  = pscript->nb_reg * SCRIPT_REGSIZE /4;
            pscript->nbw32_allocated += pscript->nb_stack * SCRIPT_REGSIZE /4;
            pscript->nbw32_allocated += pscript->ram_heap_size * 1;    // parameter in word32
        }

        /*  
            codes in sequence without solving the label position yet
        */
        for (iscript = 0; iscript < graph->nb_scripts; iscript++)
        {   FMT0 = 0;
            ST(FMT0, FORMAT_SCROFF0, graph->all_scripts[iscript].script_format);
            pscript = &(graph->all_scripts[iscript]);

            for (j = 0; j < graph->all_scripts[iscript].script_nb_instruction; j++)
            {   int32_t  cond, opcode, opar, dst, src1, src2, K;
                //FMT0 = graph->all_scripts[iscript].script_program[j];
                //cond =  RD(FMT0, OP_COND_INST);   dst  =  RD(FMT0, OP_DST_INST);
                //opcode= RD(FMT0, OP_INST);        src1 =  RD(FMT0, OP_SRC1_INST);
                //opar =  RD(FMT0, OP_OPAR_INST);   src2 =  RD(FMT0, OP_SRC2_INST);
                //K    =  RD(FMT0, OP_K_INST); K = K - UNSIGNED_K_OFFSET; 

                //sprintf(tmpstring, "IF%d %1d_%2d D%2d S1 %2d S2 %2d K %5d ", 
                //    cond, opcode, opar, dst, src1, src2, K);
                //strcat(tmpstring, graph->all_scripts[iscript].script_comments[j]); /* comments extracted from the code */

                GTEXT(tmpstring); GWORDINC(FMT0);
            }
        }
    }

    /* 
        LINKED-LIST of SWC , First pass without memory allocation 

        minimum 5 words/SWC
        Word0  : header processor/architecture, nb arcs, SWCID, arc
        Word1+n: arcs * 2  + debug page
        Word2+n: 2xW32 : ADDR + SIZE + nb of memory segments
        Word3+n: Preset, New param!, Skip length, 
           byte stream: nbparams (ALLPARAM), {tag, nbbytes, params}
        list Ends with the SWC ID 0x03FF 
    
        ----------------------------------------------------------------------------------------------------------------------------------
        SUBROUTINE- SCRIPTS : indexed with a table before the codes
   
            [0] PIO HW decoding table
            [1] PIO Graph table, STREAM_IO_CONTROL (4 words per IO)
            [2] Scripts (when used with indexes)
        [3] Graph Linked-list of nodes
            [4] "on going" flags
            [5] stream formats
            [6] arc descriptors
        ----------------------------------------------------------------------------------------------------------------------------------
    */
    LK0 = addrW32s;                                                 // linked-list header position
    m = 0;
    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_LINKED_LIST))
    {   sprintf(tmpstring3, "GRAPH_LINKED_LIST to MEMID %d", graph->option_graph_locations[GRAPH_LINKED_LIST]);
    }
    else
    {   sprintf(tmpstring3, "GRAPH_LINKED_LIST used from here");
        ST(packxxb, COPY_IN_RAM_FMT0, INPLACE_ACCESS_TAG); ST(packxxb, SIZE_EXT_OFF_FMT0, addrW32s); // read from here
    }
    addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_LINKED_LIST;
    GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
    GWORDINC(m);                                                    // size in W32
    addrW32s = addrW32s_backup; 

    LK1 = addrW32s;                                                 // linked-list position used in 3 pass 

    for (inode = 0; inode < graph->nb_nodes; inode++)
    {   uint32_t iarc, imem, iword32_arcs, nword32_arcs, nb_arcs, Lin, Lout;

        HCNEWLINE()
        node = &(graph->all_nodes[inode]);


        /* word 0 - main Header */
        FMT0 = 0;
        ST(FMT0, PRIORITY_LW0,  node->node_assigned_priority);
        ST(FMT0, PROCID_LW0,    node->node_assigned_proc);
        ST(FMT0, ARCHID_LW0,    node->node_assigned_arch);
        ST(FMT0, SCRIPT_LW0,    node->local_script_index);
        ST(FMT0, NBARCW_LW0,    node->nbParamArc + node->nbInputArc + node->nbOutputArc);
        ST(FMT0, NALLOCM1_LW0,  node->nbMemorySegment - 1); 
        if (node->connected_to_the_graph == 0)
        {   ST(FMT0, NODE_IDX_LW0,  0 );    /* dummy node */ 
            sprintf(tmpstring2, "%s (BYPASSED !) ",  node->nodeName);
        } else 
        {   ST(FMT0, NODE_IDX_LW0,  node->platform_node_idx);
            strcpy(tmpstring2, node->nodeName);
        }


        sprintf(tmpstring, "-----  %s(%d) idx:%d nRX %d nTX %d lockArc %d Who%d/%d/%d script%d", 
            tmpstring2, node->graph_instance, node->platform_node_idx, node->nbInputArc, node->nbOutputArc, node->locking_arc,
            node->node_assigned_priority, node->node_assigned_proc, node->node_assigned_arch, node->local_script_index);
        GTEXT(tmpstring); GWORDINC(FMT0);
        
        node->node_position_in_graph = addrW32s - 1;
        sprintf(tmpstring, "_%d      ",node->graph_instance); 
        HCDEF_SSDC(node->nodeName, tmpstring, node->node_position_in_graph, " node position in the graph");  

        /* word 1 - arcs */
        nb_arcs = node->nbInputArc + node->nbOutputArc;
        nword32_arcs = ((1 + nb_arcs) / 2);
        for (iword32_arcs = 0; iword32_arcs < nword32_arcs; iword32_arcs++)
        {   iarc = nword32_arcs/2;
            arc = &(node->arc[iarc]);
            FMT0 = 0;
            if (iarc +1 < nb_arcs)
            {   sprintf(tmpstring, "_%d_arc_%d",node->graph_instance, arc[iarc  ].arcID); HCDEF_SSDC(node->nodeName, tmpstring, addrW32s, " node arc offset in linkedList");  
                sprintf(tmpstring, "_%d_arc_%d",node->graph_instance, arc[iarc+1].arcID); HCDEF_SSDC(node->nodeName, tmpstring, addrW32s, " node arc offset in linkedList");  
            } else
            {   sprintf(tmpstring, "_%d_arc_%d",node->graph_instance, arc[iarc].arcID); HCDEF_SSDC(node->nodeName, tmpstring, addrW32s, " node arc offset in linkedList");  
            }

            ST(FMT0, KEY_LW1,   node->use_user_key);       
            ST(FMT0, ARC0_LW1,  arc[iarc].arcID);           ST(FMT0, ARC1_LW1,  arc[iarc+1].arcID);          
            ST(FMT0, ARC0D_LW1, arc[iarc].rx0tx1);          ST(FMT0, ARC1D_LW1, arc[iarc+1].rx0tx1);    

            Lin  = (int)(graph->arcFormat[graph->arc[arc[iarc  ].arcID].fmtProd].frame_length_bytes);
            Lout = (int)(graph->arcFormat[graph->arc[arc[iarc+1].arcID].fmtCons].frame_length_bytes);
            if (Lin>Lout)
            {   strcpy(tmpstring2, ">>>");
            } else
            {   strcpy(tmpstring2, "   ");
            }
            if (iarc +1 < nb_arcs)
            {   sprintf(tmpstring, "ARC %d Rx0Tx1 %d L=%d  -- ARC %d Rx0Tx1 %d L=%d KEY%lld %s",
                    arc[iarc  ].arcID, arc[iarc  ].rx0tx1, Lin, 
                    arc[iarc+1].arcID, arc[iarc+1].rx0tx1, Lout, node->use_user_key, tmpstring2); /* check production < consumption capability */
            }   else
            {   sprintf(tmpstring, "ARC %d Rx0Tx1 %d L=%d KEY%lld %s",
                    arc[iarc  ].arcID, arc[iarc  ].rx0tx1, Lin,  node->use_user_key, tmpstring2);
            }
            GTEXT(tmpstring); GWORDINC(FMT0);
        }

        /* word 2 - memory banks  ---  allocations and comments are made in the second pass */
        for (imem = 0; imem < node->nbMemorySegment; imem++) 
        {   GINC
            GINC
        }

        if (node->use_user_key == 1)
        {   GINC
            GINC
        }

        /* word 3 - parameters */
        pscript = &(graph->all_nodes[inode].node_script);
        if (pscript->script_nb_instruction)
        {
            FMT0 = 0;                   // Param header
            ST(FMT0, PARAM_TAG_LW4, node->TAG); 
            ST(FMT0,    PRESET_LW4, node->preset); 
            ST(FMT0,   TRACEID_LW4, node->trace_ID); 
            ST(FMT0, W32LENGTH_LW4, pscript->script_nb_instruction +1); 
            sprintf(tmpstring, "ParamLen %d+1 Preset %d Tag0ALL %d", pscript->script_nb_instruction +1, node->preset, node->TAG);
            GTEXT(tmpstring); 
            GWORDINC(FMT0);

            node->script_position_in_graph = addrW32s;

            node->ParameterSizeW32 = pscript->script_nb_instruction;
            for (j = 0; j < node->ParameterSizeW32; j++)
            {   int32_t  cond, opcode, opar, dst, src1, src2, K;
            
                FMT0 = pscript->script_program[j];
                //
                //cond =  RD(FMT0, OP_COND_INST);  dst  =  RD(FMT0, OP_DST_INST);
                //opcode= RD(FMT0, OP_INST);       src1 =  RD(FMT0, OP_SRC1_INST);
                //opar =  RD(FMT0, OP_OPAR_INST);  src2 =  RD(FMT0, OP_SRC2_INST);
                //K    =  RD(FMT0, OP_K_INST); K = K - UNSIGNED_K_OFFSET; 

                //sprintf(tmpstring, "IF%d %1d_%2d D%2d S1 %2d S2 %2d K %5d ", 
                //    cond, opcode, opar, dst, src1, src2, K);
                //strcat(tmpstring,  pscript->script_comments[j]);
                //GTEXT(tmpstring); GWORDINC(FMT0);
            }
            strcpy(tmpstring, "");
        }
        else
        {
            FMT0 = 0;                   // Param header
            ST(FMT0, PARAM_TAG_LW4, node->TAG); 
            ST(FMT0,    PRESET_LW4, node->preset); 
            ST(FMT0,   TRACEID_LW4, node->trace_ID); 
            ST(FMT0, W32LENGTH_LW4, node->ParameterSizeW32  + 1); /* the parameter section is Header + PackedParameters[]  */
            sprintf(tmpstring, "ParamLen %d+1 Preset %d Tag0ALL %d", node->ParameterSizeW32, node->preset, node->TAG);
            GTEXT(tmpstring); 
            GWORDINC(FMT0);

            /* parameters */
            for (j = 0; j < node->ParameterSizeW32; j++)
            {   
                sprintf(tmpstring, "(%d)", j); 
                FMT0 = node->PackedParameters[j];
                GTEXT(tmpstring); GWORDINC(FMT0);
            }
        }
    }


    /* LAST WORD */
    GTEXT("vvvvvvvvvvv RAM vvvvvvvvvvv ^^^^^^^^^ END OF LINKED-LIST ^^^^^^^^^ "); GWORDINC(GRAPH_LAST_WORD);


    m = addrW32s - LK1;                                             // all the linked-list words
    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_LINKED_LIST))
    {   vid_malloc(graph->option_graph_locations[GRAPH_LINKED_LIST], // MEMID
                   4 * m,                                           // LinkedList cumulated size
                   MEM_REQ_4BYTES_ALIGNMENT, 
                   &packxxb, MEM_TYPE_STATIC, 
                   tmpstring3, platform, graph);
        sprintf(tmpstring3, "GRAPH_LINKED_LIST to MEMID %d", graph->option_graph_locations[GRAPH_LINKED_LIST]);
        sprintf(tmpstring2, " position %08X", packxxb);
        strcat(tmpstring3, tmpstring2);

        addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_LINKED_LIST;
        GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
        GWORDINC(m);                                                    // size in W32
        addrW32s = addrW32s_backup; 
    }
    else
    {
        addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_LINKED_LIST;
        GTEXT(tmpstring3); GINC;                                        // destination address unchanged
        GWORDINC(m);                                                    // size in W32
        addrW32s = addrW32s_backup; 
    }

    /*================= TODO change the position of the RAM ===================*/
    LK_PIO = addrW32s;


    /*------------------------------------------------------------------------------------------------------------------------------
        PIO "ONGOING" (1 Byte per IO)  
   
            [0] PIO HW decoding table
            [1] PIO Graph table, STREAM_IO_CONTROL (4 words per IO)
            [2] Scripts (when used with indexes)
            [3] Graph Linked-list of nodes
        [4] "on going" flags
            [5] stream formats
            [6] arc descriptors
        ----------------------------------------------------------------------------------------------------------------------------------

        In the AL : graph_io_idx = platform_io_al_idx_to_graph[HWIO]
        In the scheduler instance : pointer to the table of PIOs  S->ioctrl[graph_io_idx]
        At init : all_arcs[0..] descriptors hold packs of 16 bytes of IOs used in this graph (xx IOCTRL_LSB)
    */
    sprintf(tmpstring, "->ongoing iomask bytes"); 
    m = (NBarc + 3) >> 2;                               // on going flag bytes, rounding go W32

    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_ONGOING))
    {   sprintf(tmpstring3, "GRAPH_ONGOING to MEMID %d", graph->option_graph_locations[GRAPH_ONGOING]);
        vid_malloc(graph->option_graph_locations[GRAPH_ONGOING],    // MEMID
                   4 * m,                                           // on going flag bytes cumulated size
                   MEM_REQ_4BYTES_ALIGNMENT, 
                   &packxxb, MEM_TYPE_STATIC, 
                   tmpstring3, platform, graph);
        sprintf(tmpstring2, " position %08X", packxxb);
        strcat(tmpstring3, tmpstring2);
    }
    else
    {   sprintf(tmpstring3, "GRAPH_ONGOING used from here");
        ST(packxxb, COPY_IN_RAM_FMT0, INPLACE_ACCESS_TAG); ST(packxxb, SIZE_EXT_OFF_FMT0, addrW32s); // read from here
    }
    addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_ONGOING;
    GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
    GWORDINC(m);                                                    // size in W32
    addrW32s = addrW32s_backup; 


    {   union 
        {   uint32_t W32;
            uint8_t W8[4];
        } pio_ongoing;

        m = 0; /* byte index */
        pio_ongoing.W32 = 0xFFFFFFFF;
        for (j = iarc = 0; iarc < graph->nb_arcs; iarc++)
        {
            arc = &(graph->arc[iarc]);
            if (arc->ioarc_flag) 
            {   
                pio_ongoing.W8[m & 3] = 0xFF;           
                ST(pio_ongoing.W8[m & 3], ONGOING_IO, 0);  /* not "ongoing" */
                m++;
                if ((m & 3) == 0) /* we have 4 bytes "ongoing" */
                {
                    GTEXT(tmpstring); GWORDINC(pio_ongoing.W32);
                    pio_ongoing.W32 = 0xFFFFFFFF;
                }
            }
        }
        if ((m & 3) != 0) /* remaining bytes ongoing */
        {
            GTEXT(tmpstring); GWORDINC(pio_ongoing.W32);
        }
    }

    /* 
        ARC DEBUG TABLE  (@@@@ TODO) 

        [ARC_DEBUG_IDX_GR2] words
    */



    /*------------------------------------------------------------------------------------------------------------------------------
        FORMAT used by the arcs (4 words each stream_format)  
        Word0: Frame size, interleaving scheme, arithmetics raw data type
        Word1: time-stamp, domain, nchan, physical unit (pixel format, IMU interleaving..)
        Word2: depends on IO Domain
        Word3: depends on IO Domain

        can be used for tunable formats
            This is used the SWC generates variable frame formats (JPG decoder, MP3 decoder..)

  
            [0] PIO HW decoding table
            [1] PIO Graph table, STREAM_IO_CONTROL (4 words per IO)
            [2] Scripts (when used with indexes)
            [3] Graph Linked-list of nodes
            [4] "on going" flags
        [5] stream formats
            [6] arc descriptors
      -------------------------------------------------------------------------------------------------------------------------------
    */
    graph->nb_formats ++;
    m = graph->nb_formats * STREAM_FORMAT_SIZE_W32;                 // number of Formats (nb_formats is an index => +1)

    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_FORMATS))
    {   sprintf(tmpstring3, "GRAPH_FORMATS to MEMID %d", graph->option_graph_locations[GRAPH_FORMATS]);
        vid_malloc(graph->option_graph_locations[GRAPH_FORMATS],    // MEMID
                   4 * m,                                           // Formats bytes  size
                   MEM_REQ_4BYTES_ALIGNMENT, 
                   &packxxb, MEM_TYPE_STATIC, 
                   tmpstring3, platform, graph);
        sprintf(tmpstring2, " position %08X", packxxb);
        strcat(tmpstring3, tmpstring2);
    }
    else
    {   sprintf(tmpstring3, "GRAPH_FORMATS used from here");
        ST(packxxb, COPY_IN_RAM_FMT0, INPLACE_ACCESS_TAG); ST(packxxb, SIZE_EXT_OFF_FMT0, addrW32s); // read from here
    }
    addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_FORMATS;
    GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
    GWORDINC(m);                                                    // size in W32
    addrW32s = addrW32s_backup; 

    for (iformat = 0; iformat < graph->nb_formats; iformat++)
    {   struct formatStruct *format;
        union 
        {   uint32_t W32;
            uint8_t W8[4];
            float F32;
        } TranslateIntFloat;
        
        FMT0 = 0;
        format = &(graph->arcFormat[iformat]);
        ST(FMT0, FRAMESIZE_FMT0, (uint32_t)(0.5+format->frame_length_bytes));

        FMT1 = 0;
        ST(FMT1, SUBTYPE_FMT1, 0);                  // @@@ todo
        ST(FMT1, DOMAIN_FMT1, 0);                   // arc->domain @@@@
        ST(FMT1, RAW_FMT1, format->raw_data);
        ST(FMT1, TSTPSIZE_FMT1,0);
        ST(FMT1, TIMSTAMP_FMT1, 0);
        ST(FMT1, INTERLEAV_FMT1, format->deinterleaved);
        ST(FMT1, NCHANM1_FMT1, format->nchan -1);  

        TranslateIntFloat.F32 = format->samplingRate;
        FMT2 = TranslateIntFloat.W32;
        FMT3 = 0;
        sprintf(tmpstring, "Format %2d frameSize %d ", iformat, (uint32_t)(0.5+format->frame_length_bytes)); GTEXT(tmpstring);  GWORDINC(FMT0);
        sprintf(tmpstring, "          nchan %d raw %d", format->nchan, format->raw_data); GTEXT(tmpstring); GWORDINC(FMT1);
        sprintf(tmpstring, "          FS[Hz]=%f", (float)(format->samplingRate));  GTEXT(tmpstring); GWORDINC(FMT2);
        sprintf(tmpstring, "          domain-dependent");  GTEXT(tmpstring); GWORDINC(FMT3);
    }


    /* ------------------------------------------------------------------------------------------------------------------------------
        ARC descriptors (4 words each)  + buffer memory allocation
        Word0: base offsetm data format, need for flush after write
        Word1: size, debug result registers
        Word2: read index, ready for read, flow error and debug tasks index
        Word3: write index, ready for write, need realignment flag, locking byte 

            [0] PIO HW decoding table
            [1] PIO Graph table, STREAM_IO_CONTROL (4 words per IO)
            [2] Scripts (when used with indexes)
            [3] Graph Linked-list of nodes
            [4] "on going" flags
            [5] stream formats
        [6] arc descriptors
      -------------------------------------------------------------------------------------------------------------------------------
    */
    m = graph->nb_arcs* SIZEOF_ARCDESC_W32;                     // number of arcs

    if (MAP_TO_SPECIFIC_VID_BANK(GRAPH_ARCS))
    {   sprintf(tmpstring3, "GRAPH_ARCS to MEMID %d", graph->option_graph_locations[GRAPH_ARCS]);
        vid_malloc(graph->option_graph_locations[GRAPH_ARCS],   // MEMID
                   4 * m,                                       // arcs descriptors  size
                   MEM_REQ_4BYTES_ALIGNMENT, 
                   &packxxb, MEM_TYPE_STATIC, 
                   tmpstring3, platform, graph);
        sprintf(tmpstring2, " position %08X", packxxb);
        strcat(tmpstring3, tmpstring2);
    }
    else
    {   sprintf(tmpstring3, "GRAPH_ARCS used from here");
        ST(packxxb, COPY_IN_RAM_FMT0, INPLACE_ACCESS_TAG); ST(packxxb, SIZE_EXT_OFF_FMT0, addrW32s); // read from here
    }
    addrW32s_backup = addrW32s; addrW32s = GRAPH_HEADER_NBWORDS + 2*GRAPH_ARCS;
    GTEXT(tmpstring3); GWORDINC(packxxb);                           // destination address
    GWORDINC(m);                                                    // size in W32
    addrW32s = addrW32s_backup; 


    HCNEWLINE()
    for (iarc = 0; iarc < graph->nb_arcs; iarc++)
    {   float sizeProd, sizeCons, jitterFactor, size, tmpSize, tmpflag;
        struct stream_script *pscript;

        arc = &(graph->arc[iarc]);
        memset(ARCW, 0, sizeof(ARCW));
 
        /*-------------------------------- ARC FOR SCRIPTS-------------------------------------------------------------*/
        /* is it the arc from a script ? */
        FMT0 = FMT1 = 0; 
        pscript = &(graph->all_nodes[0].node_script); // for the compiler
        for (iscript = 0; iscript < graph->nb_scripts; iscript++)
        {   if (iarc == graph->all_scripts[iscript].arc_script)
            {   FMT0 = 1;
                pscript = &(graph->all_scripts[iscript]);
                break;
            }
        }
        /* or is it from arm_stream_script ? */
        for (inode = 0; inode < graph->nb_nodes; inode++)
        {   
            if (graph->all_nodes[inode].platform_node_idx == arm_stream_script_index
                && iarc == graph->all_nodes[inode].arc[0].arcID)
            {   FMT0 = 1;
                pscript = &(graph->all_nodes[inode].node_script);
                pscript->nbw32_allocated  = pscript->nb_reg * SCRIPT_REGSIZE /4;
                pscript->nbw32_allocated += pscript->nb_stack * SCRIPT_REGSIZE /4;
                pscript->nbw32_allocated += pscript->ram_heap_size * 1;    // heap size in word32
                iscript = 0xFFFF;
                break;
            }
        }

        if (FMT0) 
        {   m = 4 * pscript->nbw32_allocated;

            /* memory pre-allocation in working / static areas */
            if (pscript->stack_memory_shared)
            {   j = pscript->mem_VID;
                if (platform->membank[j].max_working < m)
                {   platform->membank[j].max_working = m;
                }
            }
            else /* memory allocation in static areas */
            {   
                if (iscript == 0xFFFF)
                {   sprintf(tmpstring, "arm_stream_script  format %d w32length %d", pscript->script_format, pscript->script_nb_instruction); 
                } else
                {   sprintf(tmpstring, "Script %d format %d w32length %d", iscript, pscript->script_format, pscript->script_nb_instruction); 
                }

                vid_malloc (pscript->mem_VID,    /* VID */
                    m,          /* size */
                    MEM_REQ_4BYTES_ALIGNMENT, 
                    &(ARCW[SCRIPT_PTR_SCRARCW0]), 
                    MEM_TYPE_STATIC,                                /* working 0 static 1 */
                    tmpstring,                                      /* comment */
                    platform, graph);

                HCTEXT(tmpstring);
            }

            ST(ARCW[    SCRIPT_SCRARCW1], BUFF_SIZE_SCRARCW1, m);        
            ST(ARCW[    SCRIPT_SCRARCW1], CODESIZE_SCRARCW1, pscript->script_nb_instruction);        
            ST(ARCW[    DBGFMT_SCRARCW4], RAMTOTALW32_SCRARCW4, 2*(pscript->nb_reg + pscript->nb_stack) + pscript->ram_heap_size);        
            ST(ARCW[    DBGFMT_SCRARCW4], NREGS_SCRARCW4, pscript->nb_reg);        
            ST(ARCW[    DBGFMT_SCRARCW4], NSTACK_SCRARCW4, pscript->nb_stack);        
        
            addrW32s_backup = addrW32s; // save the arc descriptor address while filling the comments

            if (iscript == 0xFFFF)
            {   sprintf(tmpstring, "ARC%d  from arm_stream_script   sizeW32 %Xh (%Xh)", iarc, m/4, m);  GTEXTINC(tmpstring); 
            } else
            {   sprintf(tmpstring, "ARC%d  from script%d   sizeW32 %Xh (%Xh)", iarc, iscript, m/4, m);  GTEXTINC(tmpstring); 
            }
           
            sprintf(tmpstring, "      nb instructions 0x%x", pscript->script_nb_instruction);           GTEXTINC(tmpstring); 
            sprintf(tmpstring, "      nregs+r12 %d x2   stack %d x2  heap %xh", RD(ARCW[DBGFMT_SCRARCW4], NREGS_SCRARCW4), 
                RD(ARCW[DBGFMT_SCRARCW4], NSTACK_SCRARCW4), pscript->ram_heap_size); HCTEXT(tmpstring); GTEXTINC(tmpstring); 
            
            sprintf(tmpstring, "arc_buf_%d ",iarc); 
            HCDEF_SSDC("", tmpstring, graph->arc[iarc].graph_base, " arc buffer address");

            addrW32s = addrW32s_backup;         // rewind and print the arc descriptor
            for (j=0; j<SIZEOF_ARCDESC_W32; j++) { GWORDINC(ARCW[j]); }

            continue;
            /*--------------------------------END ARC FOR SCRIPTS-------------------------------------------------------------*/
        }
            /*--------------------------------ARC FOR SWAP--------------------------------------------------------------------*/
        /* is it an arc used for memory swap of a node memory bank ? */
        tmpflag = 0;
        for (inode = 0; inode < graph->nb_nodes; inode++)
        {   uint32_t imem_swap;
            struct node_memory_bank *membank = 0;

            node = &(graph->all_nodes[inode]);
            for (imem_swap = 0; imem_swap < node->nbMemorySegment; imem_swap++)    
            {   membank = &(node->memreq[imem_swap]);
                if (membank->toSwap) 
                {   if (iarc == membank->swap_arc)  /* current arc is the one of this node/memreq */
                    {   membank->addrW32s = addrW32s;   /*  the address of the descriptor and reserve its place : */
                        for (j=0; j<SIZEOF_ARCDESC_W32; j++) { GWORDINC(ARCW[j]); }
                        tmpflag = 1;
                    }
                }
            }
        }
        if (tmpflag)    /* current arc will be managed during node memeory allocation */
        {   continue;
        }


        /*--------------------------------ARC BUFFER --------------------------------------------------------------------*/

        sprintf(tmpstring, "arc_%d     ",iarc); HCDEF_SSDC("", tmpstring, addrW32s, " arc descriptor position W32 in the graph");  

        jitterFactor = graph->arc[iarc].sizeFactor;
        sizeProd = (float)(graph->arcFormat[graph->arc[iarc].fmtProd].frame_length_bytes);
        sizeCons = (float)(graph->arcFormat[graph->arc[iarc].fmtCons].frame_length_bytes);

        if (sizeProd > sizeCons) 
        {   size = sizeProd;
        } else 
        {   size = sizeCons;
        }

        FMT5 = (uint32_t)(0.5f + (jitterFactor * size));
        FMT5 = (3 + FMT5)>>2;                   /* buffer size rounding to the next W32 */
        FMT5 = FMT5 << 2;                       /* in Bytes */
        tmpSize = (float)FMT5;
        size = (float)FMT5;

        if (arc->ioarc_flag &&                  /* the arc buffer is external => no memory allocation */
            0 == arc->set0copy1)     
        {   size  = 0;
        }


        /* =====================DEBUG_MAPPING =====================*/
        if (arc->memVID == 0)
        {   char tmpstring4[NBCHAR_LINE];
            uint64_t imem, memstart;

            memstart = LK_PIO + platform->membank[0].ptalloc_static/4;
            strcpy(tmpstring4, "S "); strcat(tmpstring4, tmpstring);
            
            for (imem = 0; imem < size/4; imem++)
            {   strcpy (graph->binary_graph_comments[memstart + imem], tmpstring4);
            }

            last_addrW32s = memstart + imem;
        } 


        /* memory allocation of BUFFERS */
        sprintf(tmpstring, "iarc %d ",iarc);  
        vid_malloc(arc->memVID,                 /* VID */
                (uint32_t)size,                 /* size */
                MEM_REQ_4BYTES_ALIGNMENT, 
                &(graph->arc[iarc].graph_base), /* address of the Base to be filled */
                MEM_TYPE_STATIC,                /* working 0 static 1 */
                tmpstring,                      /* comment */
                platform, graph);


        addrW32s_backup = addrW32s; // save the arc descriptor address while filling the comments            

        ARCW[0] = arc->graph_base;  
        if (arc->ioarc_flag)
        {
        sprintf(tmpstring, "ARC%d -IO- Base %Xh (%Xh words) fmtProd_%d (frameL %d)", iarc, 
            arc->graph_base, (int)(size/4), 
            arc->fmtProd, graph->arcFormat[arc->fmtProd].frame_length_bytes); 
        } else
        {
        sprintf(tmpstring, "ARC%d  Base %Xh (%Xh words) fmtProd_%d (frameL %d)", iarc, 
            graph->arc[iarc].graph_base, (int)(size/4), 
            graph->arc[iarc].fmtProd, graph->arcFormat[graph->arc[iarc].fmtProd].frame_length_bytes); 
        }
        GTEXTINC(tmpstring); 

        ST(ARCW[1], BUFF_SIZE_ARCW1, (uint32_t)tmpSize);
        all_buffersW32 += (uint32_t)tmpSize;

        if (size == 0)
        {   sprintf(tmpstring, "IO buffer with base address redirection, no memory allocation, the Size is from the producer");  
        } else
        {   sprintf(tmpstring, "      Size %Xh[B] fmtCons_%d FrameL %d jitterScaling%4.1f", (uint32_t)size, graph->arc[iarc].fmtCons, 
                graph->arcFormat[graph->arc[iarc].fmtCons].frame_length_bytes, jitterFactor); 
        }
        GTEXTINC(tmpstring);
        sprintf(tmpstring, "arc_buf_%d ",iarc); HCDEF_SSDC("", tmpstring, graph->arc[iarc].graph_base, " arc buffer address");

        //ST(ARCW2, ARCEXTEND_ARCW2, 0); @@@
        ST(ARCW[2],   MPFLUSH_ARCW2, graph->arc[iarc].flush);
        ST(ARCW[2], FLOW_WR_ARCW2, graph->arc[iarc].flow_error_wr);
        ST(ARCW[2], FLOW_RD_ARCW2, graph->arc[iarc].flow_error_rd);
        ST(ARCW[2], ALIGNBLCK_ARCW2, 0);
        ST(ARCW[2],  HIGH_QOS_ARCW2, graph->arc[iarc].HQoS);            
        ST(ARCW[2],      READ_ARCW2, 0);
        {   char source_name[NBCHAR_LINE], destination_name[NBCHAR_LINE];
            node = &(graph->all_nodes[arc->SwcProdGraphIdx]);
            strcpy(source_name, node->nodeName);
            node = &(graph->all_nodes[arc->SwcConsGraphIdx]);
            strcpy(destination_name, node->nodeName);
            if (arc->ioarc_flag && 0 == arc->rx0tx1)  sprintf(source_name, "IO %d", arc->fw_io_idx);
            if (arc->ioarc_flag && 1 == arc->rx0tx1)  sprintf(destination_name, "IO %d", arc->fw_io_idx);
            sprintf(tmpstring, "      %s ===> %s", source_name, destination_name);
        }
        GTEXTINC(tmpstring); 
        strcpy(tmpstring, "");

        ST(ARCW[3], COLLISION_ARCW3, 0);
        ST(ARCW[3],     WRITE_ARCW3, 0);

        sprintf(tmpstring, "      fmtCons %d fmtProd %d dbgreg %d dbgcmd %d %s", graph->arc[iarc].fmtCons, graph->arc[iarc].fmtProd, 
            graph->arc[iarc].debug_reg, graph->arc[iarc].debug_cmd, (graph->arc[iarc].HQoS==0)?" ":"HQoS"); 
        GTEXTINC(tmpstring); 

        ST(ARCW[4], COMPUTCMD_ARCW4, graph->arc[iarc].debug_cmd);
        ST(ARCW[4], DEBUG_REG_ARCW4, graph->arc[iarc].debug_reg);
        ST(ARCW[4], PRODUCFMT_ARCW4, graph->arc[iarc].fmtProd);
        ST(ARCW[4], CONSUMFMT_ARCW4, graph->arc[iarc].fmtCons);

        sprintf(tmpstring, "      log producer/consumer flow errors"); 
        GTEXTINC(tmpstring); 

        addrW32s = addrW32s_backup;         // rewind and print the arc descriptor
        for (j=0; j<SIZEOF_ARCDESC_W32; j++) { GWORDINC(ARCW[j]); }
    }
    HCNEWLINE()   


    /* ------------------------------------------------------------------------------------------------------------------------------
        ==== debug filling pattern ====
    */
    for (j = 0; j < graph->debug_pattern_size; j++)
    {   
        sprintf(tmpstring, "    padding "); GTEXT(tmpstring);GWORDINC(graph->debug_pattern);
    }
    HCNEWLINE()   



    /* ------------------------------------------------------------------------------------------------------------------------------
        LINKING SCRIPT : ==== looking for OFFSET in the graph, search in small scripts and nodes 
    */
    for (iscript = 0; iscript < graph->nb_scripts; iscript++)
    {   
        labelPos_t *Labels;
        uint32_t ilabel, label_position, script_offset_in_graph, *instruction;
        char node_to_compare[NBCHAR_STREAM_NAME];

        pscript = &(graph->all_scripts[iscript]);
        Labels = pscript->Label_positions;
        script_offset_in_graph = graph->all_scripts[iscript].script_offset;

        /* replacement of Label of set r label/heap , instruction using 2 words */
        for (ilabel = 0; ilabel < pscript->idx_label; ilabel++)
        {   if (Labels[ilabel].label_type == LABEL_GRAPH_USE) 
            {   for (inode = 0; inode < graph->nb_nodes; inode++)
                {   node = &(graph->all_nodes[inode]);
                    sprintf(node_to_compare, "%s_%d", node->nodeName, node->graph_instance);

                    // does the node_name is the one from the script source code 
                    if (0 == strcmp(Labels[ilabel].symbol, node_to_compare))
                    {   
                        label_position = Labels[ilabel].offset;
                        instruction = &(graph->binary_graph[label_position + script_offset_in_graph]);
                        *instruction = node->node_position_in_graph;      // 32bits address
                    }
                }
            }
        }
    }

    /* solves the labels of scripts in nodes */
    for (inode = 0; inode < graph->nb_nodes; inode++)
    {   
        node = &(graph->all_nodes[inode]);
        pscript = &(graph->all_nodes[inode].node_script);
        if (pscript->script_nb_instruction)
        {
        labelPos_t *Labels;
        uint32_t ilabel, label_position, script_offset_in_graph, *instruction;
        char node_to_compare[NBCHAR_STREAM_NAME];

        Labels = pscript->Label_positions;
        script_offset_in_graph = node->script_position_in_graph;

        /* replacement of Label of set r label/heap , instruction using 2 words */
        for (ilabel = 0; ilabel < pscript->idx_label; ilabel++)
        {   if (Labels[ilabel].label_type == LABEL_GRAPH_USE) 
            {   for (inode = 0; inode < graph->nb_nodes; inode++)
                {   node = &(graph->all_nodes[inode]);
                    sprintf(node_to_compare, "%s_%d", node->nodeName, node->graph_instance);

                    // does the node_name is the one from the script source code 
                    if (0 == strcmp(Labels[ilabel].symbol, node_to_compare))
                    {   
                        label_position = Labels[ilabel].offset;
                        instruction = &(graph->binary_graph[label_position + script_offset_in_graph]);
                        *instruction = node->node_position_in_graph;      // 32bits address
                    }
                }
            }
        }
        }
    }

    /* ------------------------------------------------------------------------------------------------------------------------------
        LINKED-LIST of SWC , second pass with ONLY static memory allocation 
    */

    ENDARCS = addrW32s;
    addrW32s = LK1;
    for (inode = 0; inode < graph->nb_nodes; inode++)
    {   uint32_t imem, iword32_arcs, nword32_arcs;
        uint64_t extend;

        /* word 0 - main Header */
        node = &(graph->all_nodes[inode]); GINC 

        /* word 1 - arcs */
        nword32_arcs = ((1 + node->nbInputArc + node->nbOutputArc) / 2);
        for (iword32_arcs = 0; iword32_arcs < nword32_arcs; iword32_arcs++)
        {   GINC
        }

        clearswap_lw2s = 0;
        /* word 2 - memory banks  compute the global swap/clear flag  */
        for (imem = 0; imem < node->nbMemorySegment; imem++)    
        {   struct node_memory_bank *membank = &(node->memreq[imem]);
            clearswap_lw2s |=  membank->toClear;
            clearswap_lw2s |=  membank->toSwap;
        }

        /* word 2 - memory banks  compute the static area  */
        for (imem = 0; imem < node->nbMemorySegment; imem++)    
        {   struct node_memory_bank *membank = &(node->memreq[imem]);

            FMT0 = FMT1 = 0;
            ST(FMT1, CLEARSWAP_LW2S, clearswap_lw2s); 
            ST(FMT1, CLEAR_LW2S, membank->toClear); 
            ST(FMT1, SWAP_LW2S, membank->toSwap); 
            extend = (membank->graph_memreq_size) >> (SIZE_SIGN_FMT0_MSB+1);
            extend = extend >> 1;       /* extension is computed as : address << (extend *2) */
            ST(FMT1, EXT_SIZE_LW2S, extend); 

            if (membank->stat0work1ret2 != MEM_TYPE_WORKING)
            {
                if (imem == 0) 
                {   sprintf(tmpstring, "Nb Memreq %d  ClearSwap %d -Static memory bank ", 
                    (node->nbMemorySegment), clearswap_lw2s);
                } 
                else
                {   sprintf(tmpstring, "Static memory bank (Swap %d) ", membank->toSwap);
                }
                
                compute_memreq (membank, graph->arcFormat, node);
            
                sprintf(tmpstring3, "inode %d %s imem %d Size %lld h%llx", inode, node->nodeName, imem, membank->graph_memreq_size, membank->graph_memreq_size);

                /* =====================DEBUG_MAPPING =====================*/
                if (membank->mem_VID == 0)
                {   char tmpstring4[NBCHAR_LINE];
                    uint64_t imem, size, memstart;

                    size = membank->graph_memreq_size;
                    memstart = LK_PIO + platform->membank[0].ptalloc_static/4;
                    strcpy(tmpstring4, "S "); strcat(tmpstring4, tmpstring3);
                    
                    for (imem = 0; imem < size/4; imem++)
                    {   strcpy (graph->binary_graph_comments[memstart+imem], tmpstring4);
                    }

                    last_addrW32s = memstart + imem;
                }     

                vid_malloc (membank->mem_VID, membank->graph_memreq_size, membank->alignmentBytes,
                    &(membank->graph_base), MEM_TYPE_STATIC, tmpstring3, platform, graph);
                HCDEF_SSDC(node->nodeName, tmpstring3, membank->graph_base, " node static memory address");        

                if(membank->toSwap)
                {   ST(FMT0, SWAPBUFID_LW2S, membank->swap_arc);    // arcID used to address the swap buffer
                    ST(FMT1, BUFF_SIZE_LW2S, membank->graph_memreq_size); 

                    sprintf(tmpstring, "swapped memory bank arcID %d ", membank->swap_arc);
                    sprintf(tmpstring2, " bank %ld stat0work1ret2 = %d size %lld(h%llx) ", 
                        imem, membank->stat0work1ret2, membank->graph_memreq_size, membank->graph_memreq_size); 
                }
                else
                {   ST(FMT0, SIGNED_SIZE_FMT0, membank->graph_base);    // address of the memory bank
                    ST(FMT1, BUFF_SIZE_LW2S, membank->graph_memreq_size); 

                    sprintf(tmpstring2, " bank %ld stat0work1ret2 = %d size %lld(h%llx) ", 
                        imem, membank->stat0work1ret2, membank->graph_memreq_size, membank->graph_memreq_size); 
                }
                GTEXT(tmpstring); GWORDINC(FMT0); GTEXT(tmpstring2);GWORDINC(FMT1);

                /* does this static memory area is reloaded from a swap area ? declared with an arc  */
                if (membank->toSwap) 
                {   
                    addrW32s_backup = addrW32s;
                    addrW32s = membank->addrW32s;   /*  the address of the descriptor */

                    memset(ARCW, 0, 4*SIZEOF_ARCDESC_W32);

                    sprintf(tmpstring, "ARC%d swap-buffer of node %d %s membank %d size %lld", membank->swap_arc, inode, node->nodeName, imem, membank->graph_memreq_size); 
                    //size32 = packed_address_24b(membank->graph_memreq_size);
                    ARCW[BUFSIZE_ARCW1] = (uint32_t)(membank->graph_memreq_size);
                    vid_malloc(membank->swapVID,            /* VID memory used for swap */
                            ARCW[BUFSIZE_ARCW1],            /* size */
                            MEM_REQ_4BYTES_ALIGNMENT, 
                            &(ARCW[BUF_PTR_ARCW0]),         /* address of the Base to be filled */
                            MEM_TYPE_STATIC,                /* working 0 static 1 */
                            tmpstring,                      /* comment */
                            platform, graph);

                    sprintf(graph->binary_graph_comments[addrW32s], "%s", tmpstring);
                    for (j=0; j<SIZEOF_ARCDESC_W32; j++) { GWORDINC(ARCW[j]); }
                    addrW32s = addrW32s_backup; 
                }
            }
            else
            {   GINC    /* address */
                GINC    /* size */
            }
        }

        if (node->use_user_key == 1)
        {   GINC
            GINC
        }

        /* word 3 - parameters  + header */
        for (j = 0; j < node->ParameterSizeW32  +1; j++)
        {   GINC    
        }
    }   
    

    /* ------------------------------------------------------------------------------------------------------------------------------
        LINKED-LIST of SWC , third pass with ONLY scratch / working memory allocation 
    */
    addrW32s = LK1;
    for (inode = 0; inode < graph->nb_nodes; inode++)
    {   uint32_t imem, iword32_arcs, nword32_arcs;

        /* word 0 - main Header */
        node = &(graph->all_nodes[inode]); GINC 

        /* word 1 - arcs */
        nword32_arcs = ((1 + node->nbInputArc + node->nbOutputArc) / 2);
        for (iword32_arcs = 0; iword32_arcs < nword32_arcs; iword32_arcs++)
        {   GINC
        }

        /* word 2 - memory banks  ---  Working allocations are made in THIS second pass */
        for (imem = 0; imem < node->nbMemorySegment; imem++)    
        {   struct node_memory_bank *membank = &(node->memreq[imem]);

            if (membank->stat0work1ret2 == MEM_TYPE_WORKING)
            {
                if (imem == 0) 
                {   sprintf(tmpstring, "Nb Memreq %d  ClearSwap %d -Scratch memory bank  Swap %d ", 
                    (node->nbMemorySegment), clearswap_lw2s, (membank->toSwap));
                } else
                {   sprintf(tmpstring, "Scratch memory bank Swap %d ", membank->toSwap);
                }

                compute_memreq (membank, graph->arcFormat, node);
            
                sprintf(tmpstring3, "inode %d imem %d (Scratch) Size %lld  h%llx", inode, imem, membank->graph_memreq_size, membank->graph_memreq_size);

                /* =====================DEBUG_MAPPING =====================*/
                if (membank->mem_VID == 0)
                {   char tmpstring4[NBCHAR_LINE];
                    uint64_t imem, size, memstart;

                    size = membank->graph_memreq_size;
                    memstart = LK_PIO + platform->membank[0].ptalloc_static/4;
                    strcpy(tmpstring4, "W "); strcat(tmpstring4, tmpstring3);
                    
                    for (imem = 0; imem < size/4; imem++)
                    {   strcpy (graph->binary_graph_comments[memstart + imem], tmpstring4);
                    }

                    last_addrW32s = memstart + imem;
                }   

                vid_malloc(membank->mem_VID, (uint32_t)(membank->graph_memreq_size), membank->alignmentBytes, 
                    &(membank->graph_base), MEM_TYPE_WORKING, tmpstring3, platform, graph);
                sprintf(tmpstring2, "%s offset %d index %d", " node scratch memory address", RD(membank->graph_base, DATAOFF_ARCW0), RD(membank->graph_base, SIGNED_SIZE_FMT0));  
                HCDEF_SSDC(node->nodeName, tmpstring2, membank->graph_base, tmpstring3);

                ST(FMT0, SIGNED_SIZE_FMT0, membank->graph_base);       // address of the memory bank
                ST(FMT1, BUFF_SIZE_LW2S, membank->graph_memreq_size); 

                sprintf(tmpstring2, " bank %ld stat0work1ret2 = %d size d%lld(h%llx) ", 
                    imem, membank->stat0work1ret2, membank->graph_memreq_size, membank->graph_memreq_size); 

                GTEXT(tmpstring); 
                GWORDINC(FMT0);
                GTEXT(tmpstring2); 
                GWORDINC(FMT1);
            }
            else
            {   GINC    /* address */
                GINC    /* size */
            }
        }

        if (node->use_user_key == 1)
        {   FMT0 = (uint32_t)(node->user_key[0]);
            sprintf(tmpstring, " user Key0 %ld %08X", FMT0, FMT0);
            GTEXT(tmpstring);  GWORDINC(FMT0);
            FMT0 = (uint32_t)((node->user_key[1])>>32);
            sprintf(tmpstring, " user Key1 %ld %08X", FMT0, FMT0);
            GTEXT(tmpstring);  GWORDINC(FMT0);
        }

        /* word 3 - parameters + header */
        for (j = 0; j < node->ParameterSizeW32  +1; j++)
        {   GINC    
        }
    }   
    addrW32s = ENDARCS;                    /* restore the original address */


    /* ------------------------------------------------------------------------------------------------------------------------------
        HEADER UPDATE : memory consumed 
    */

    {   int i;
        addrW32s_backup = addrW32s; addrW32s = LKalloc; 
        
#define NOFF (1<<(DATAOFF_ARCW0_MSB - DATAOFF_ARCW0_LSB +1))
        uint64_t maxSizes[NOFF], used[NOFF], percent[NOFF];
        struct processor_memory_bank *mem;
        for (i = 0; i < NOFF; i++)
        {   maxSizes[i] = 1; used[i] = 0;
        }

        for (i = 0; i < MAX_PROC_MEMBANK; i++)
        {   mem = &(platform->membank[i]); 
            maxSizes[mem->offsetID] += mem->size;
            used[mem->offsetID] = (uint32_t)(used[mem->offsetID] + mem->ptalloc_static + mem->max_working);
        }
        for (i = 0; i < NOFF; i++)
        {   percent[i] = (255 * used[i]) / maxSizes[i];
        }
        
        /* there are 16 data offsets DATAOFF_ARCW0 */
        ST(FMT0, BYTE_0, percent[0]);
        ST(FMT0, BYTE_1, percent[1]);
        ST(FMT0, BYTE_2, percent[2]);
        ST(FMT0, BYTE_3, percent[3]);
        GWORDINC(FMT0);
        ST(FMT0, BYTE_0, percent[4]);
        ST(FMT0, BYTE_1, percent[5]);
        ST(FMT0, BYTE_2, percent[6]);
        ST(FMT0, BYTE_3, percent[7]);
        GWORDINC(FMT0);
        ST(FMT0, BYTE_0, percent[8]);
        ST(FMT0, BYTE_1, percent[9]);
        ST(FMT0, BYTE_2, percent[10]);
        ST(FMT0, BYTE_3, percent[11]);
        GWORDINC(FMT0);
        ST(FMT0, BYTE_0, percent[12]);
        ST(FMT0, BYTE_1, percent[13]);
        ST(FMT0, BYTE_2, percent[14]);
        ST(FMT0, BYTE_3, percent[15]);
        GWORDINC(FMT0);
        addrW32s = addrW32s_backup;
    }


    FMT1 = (uint32_t)(platform->membank[0].ptalloc_static + platform->membank[0].max_working_booking);
    FMT1 = FMT1/4;
    FMT1 += LK_PIO;

    sprintf(tmpstring, "Size, Flash=h%X +RAM=h%X +Buffers=h%X, all W32, not compressed", 
            LK_PIO, addrW32s-LK_PIO, FMT1-(addrW32s-LK_PIO) );
    sprintf(graph->binary_graph_comments[0], "%s", tmpstring);
    FMT0 = 0;
    ST(FMT0,  COMPRESSION_HW0, 0);              // no compression
    ST(FMT0,  GRAPH_SIZE_HW0, last_addrW32s);   // graph size (last word)
    graph->binary_graph[0] = FMT0;              // address 0 gives the size


    /* ------------------------------------------------------------------------------------------------------------------------------
        FINAL PRINT 
            data (binary_graph)   
            comments (binary_graph_comments)
    */
    for (j = 0; j < FMT1; j ++)
    {
        if (j < LK_PIO)
        {
            fprintf (graph->ptf_graph_bin, "0x%08X, // %03X %03X %s \n", 
                graph->binary_graph[j], j*4, j, graph->binary_graph_comments[j]);
        } else
        {
            fprintf (graph->ptf_graph_bin, "0x%08X, // %03X %03X %03X %03X %s \n", 
                graph->binary_graph[j], j*4, j, (j-LK_PIO)*4, (j-LK_PIO), graph->binary_graph_comments[j]);
        }
    }


    {   uint32_t ibank, a, b, c, d, sumAlloc = 0; 
        sprintf(tmpstring,    "// bank       size            consumed        static   +    working");
        fprintf (graph->ptf_graph_bin, "%s\n", tmpstring);
        for (ibank = 0; ibank < platform->nb_shared_memory_banks; ibank++)
        {   
            a = (uint32_t)(platform->membank[ibank].size);
            b = (uint32_t)(platform->membank[ibank].ptalloc_static + platform->membank[ibank].max_working_booking); 
            c = (uint32_t)(platform->membank[ibank].ptalloc_static);
            d = (uint32_t)(platform->membank[ibank].max_working_booking);
            sprintf(tmpstring, "// %2d   %7d (%6X) %6d (%5X) %6d (%5X) %6d (%5X)", ibank, a,a, b,b, c,c, d,d);
            fprintf (graph->ptf_graph_bin, "%s\n", tmpstring);

            sumAlloc += b;

            if (sumAlloc > MAXBINARYGRAPHW32)
            {   fprintf(stderr, "\n\n graph overflow %d > %d  \n\n", sumAlloc, MAXBINARYGRAPHW32);
                exit(6);
            }
        }
    }


}

#ifdef __cplusplus
}
#endif