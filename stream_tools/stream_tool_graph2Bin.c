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
    uint32_t FMT0, FMT1, FMT2, FMT3, ARCW0, ARCW1, ARCW2, ARCW3, ARCW4;
    static uint32_t SC1, SC2, LK1, LK2, LKscripts, LK_lkSize, LKalloc, LK_PIO, ENDARCS, addrW32s_backup;
    static fpos_t pos_NWords, pos_end;
    static uint32_t nFMT, LENscript, LinkedList, LinkedList0, NbInstance, nIOs, NBarc, SizeDebug, dbgScript;
    uint32_t i, j, m;
    char tmpstring[NBCHAR_LINE], tmpstring2[NBCHAR_LINE];
    struct arcStruct *arc;
    struct stream_node_manifest *node;
    uint32_t addrW32s;
    time_t rawtime;
    struct tm * timeinfo;
    addrW32s = 0;
    uint32_t all_buffers = 0;

#define GTEXT(T) sprintf(graph->binary_graph_comments[addrW32s], "%s", T);
#define GWORD(W) graph->binary_graph[addrW32s] = W;
#define GWORDINC(W) GWORD(W) addrW32s += 1;
#define GINC    addrW32s += 1;
#define HCDEF_SSDC(N,N2,D,C) fprintf(graph->ptf_header,"#define %s%s 0x%X //%s\n", N,N2,D,C);
#define HCNEWLINE() fprintf(graph->ptf_header,"\n");
#define HCTEXT(T) fprintf(graph->ptf_header,T);fprintf(graph->ptf_header,"\n");

    fprintf(graph->ptf_graph_bin, "//--------------------------------------\n"); 
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    fprintf(graph->ptf_graph_bin, "//  DATE %s", asctime (timeinfo)); 
    fprintf(graph->ptf_graph_bin, "//  AUTOMATICALLY GENERATED CODES\n"); 
    fprintf(graph->ptf_graph_bin, "//  DO NOT MODIFY !\n"); 
    fprintf(graph->ptf_graph_bin, "//--------------------------------------\n"); 

    fprintf(graph->ptf_header, "//--------------------------------------\n"); 
    fprintf(graph->ptf_header, "//  DATE %s", asctime (timeinfo)); 
    fprintf(graph->ptf_header, "//  AUTOMATICALLY GENERATED CODES\n"); 
    fprintf(graph->ptf_header, "//  DO NOT MODIFY !\n"); 
    fprintf(graph->ptf_header, "//--------------------------------------\n"); 

    FMT0 = graph->graph_location_from_0; 
    ST(FMT0, RAMSPLIT_GR0, graph->option_graph_location);
    sprintf(tmpstring, "[0] Destination in RAM %d, and RAM split %d", 0, graph->option_graph_location);
    GTEXT(tmpstring);          GWORDINC(FMT0);

    LKscripts = addrW32s;               // GR1 filled later , number of FORMAT, IOs, size of SCRIPTS(..)
    GINC;

    LK_lkSize = addrW32s;               // GR2 filled later, size of LINKEDLIST (..)
    GINC;

    FMT0 = 0;                           // GR3 number of ARCS, scheduler control  
    ST(FMT0, SCRIPT_SCTRL_GR3,  graph->dbg_script_fields);
    ST(FMT0, RETURN_SCTRL_GR3,  graph->sched_return);
    ST(FMT0, NB_ARCS_GR3,       graph->nb_arcs);
    sprintf(tmpstring, "[3] Nb arcs %d  SchedCtrl %d ScriptCtrl %d  ", graph->nb_arcs, graph->sched_return, graph->dbg_script_fields);
    GTEXT(tmpstring);           GWORDINC(FMT0);

    FMT0;
    ST(FMT0, PROC_ALLOWED_GR4, graph->procid_allowed_gr4);
    GTEXT("[4] Processors allowed");                                 GWORDINC(FMT0);

    LKalloc = addrW32s;                 // graph memory consumption computed at the end 
    GINC;
    GINC;

    /*     
        PIO settings (1 word per IO)  size = [NB_IOS_GR1]
        depends on the domain of the IO
    */
    for (j = i = 0; i < graph->nb_arcs; i++)
    {  
        arc = &(graph->arc[i]);
        if (arc->ioarc_flag)
        {   
            FMT0 = FMT1 = FMT2 = FMT3 = 0;
            FMT1 = arc->settings[0]; 
            FMT2 = arc->settings[1]; 
            FMT3 = arc->settings[2]; 

            ST(FMT0,   FWIOIDX_IOFMT0, arc->fw_io_idx);
            ST(FMT0, IO_DOMAIN_IOFMT0, arc->domain);
            ST(FMT0,FROMIOBUFF_IOFMT0, (arc->graphalloc_X_bsp_0) > 0);
            ST(FMT0, SET0COPY1_IOFMT0, arc->set0copy1);
            ST(FMT0,  SERVANT1_IOFMT0, arc->commander0_servant1);
            ST(FMT0,    RX0TX1_IOFMT0, arc->rx0tx1);
            ST(FMT0,   IOARCID_IOFMT0, arc->idx_arc_in_graph);

            sprintf(tmpstring, "IO(graph%d) %d arc %d set0copy1=%d rx0tx1=%d servant1 %d shared %d domain %d", 
                j++, arc->fw_io_idx,arc->idx_arc_in_graph, arc->set0copy1, arc->rx0tx1, arc->commander0_servant1, (arc->graphalloc_X_bsp_0) > 0, arc->domain); 
            GTEXT(tmpstring); GWORDINC(FMT0);

            sprintf(tmpstring, "IO(settings %d, fmtProd %d (L=%d) fmtCons %d (L=%d)", FMT1, graph->arc[i].fmtProd, (int)(graph->arcFormat[graph->arc[i].fmtProd].frame_length), 
                graph->arc[i].fmtCons, (int)(graph->arcFormat[graph->arc[i].fmtCons].frame_length));
            GTEXT(tmpstring); GWORDINC(FMT1);   strcpy(tmpstring, "");
            GTEXT(tmpstring); GWORDINC(FMT2);   strcpy(tmpstring, "");
            GTEXT(tmpstring); GWORDINC(FMT3);   strcpy(tmpstring, "");
        }
    }

    /*  
        SCRIPTS 
    */
    {   uint32_t offsetW = 0;
    for (i = 0; i < graph->nb_scripts; i++)
    {   graph->all_scripts[i].script_offset = offsetW;                  // prepare the static memory allocation
        script_memory_consumption(&(graph->all_scripts[i]));            // fill nbw32_allocated

        sprintf(tmpstring, "Script %d format %d w32length %d, w32offsetW %d", 
            i, graph->all_scripts[i].script_format, graph->all_scripts[i].script_nb_instruction/4, offsetW); 
            
        /* memory pre-allocation in working areas */
        if (graph->all_scripts[i].stack_memory_shared)
        {   j = graph->all_scripts[i].mem_VID;
            m = graph->all_scripts[i].nbw32_allocated;
            if (platform->membank[j].max_working < m)
            {   platform->membank[j].max_working = m;
            }
        }
        else /* memory allocation in static areas */
        {   vid_malloc (graph->all_scripts[i].mem_VID, 
                graph->all_scripts[i].nbw32_allocated, MEM_REQ_4BYTES_ALIGNMENT, 
                &(graph->all_scripts[i].script_instance27b),  0, tmpstring,
                platform, graph);
        }

        FMT0 = 0;
        ST(FMT0, FORMAT_SCROFF0, graph->all_scripts[i].script_format);
        ST(FMT0, OFFSET_SCROFF0, offsetW);

        GTEXT(tmpstring); GWORDINC(FMT0);
        
        //@@@@ copy GWORDINC script codes 

        offsetW = offsetW + (3 + graph->all_scripts[i].script_nb_instruction)/4;
    }
    
    FMT0 = graph->binary_graph[GR1_INDEX];
    ST(FMT0, SCRIPTSSZW32_GR1, offsetW);
    graph->nb_formats ++ ; // nb_formats is an index = MAX(current_format_index, nb_format)

    {   uint32_t backupLK = addrW32s; addrW32s = LKscripts; FMT0 = 0;
        ST(FMT0, SCRIPTSSZW32_GR1,      offsetW);
        ST(FMT0, NB_IOS_GR1,            graph->nb_io_arcs);
        ST(FMT0, NBFORMATS_GR1,         graph->nb_formats); 
        sprintf(tmpstring, "[1] Number of IOs %d, Formats %d, Scripts %d", graph->nb_io_arcs, graph->nb_formats, offsetW);
        GTEXT(tmpstring);          GWORDINC(FMT0);
        addrW32s = backupLK;
    }
    }

    /* LINKED-LIST of SWC
       minimum 5 words/SWC
       Word0  : header processor/architecture, nb arcs, SWCID, arc
       Word1+n: arcs * 2  + debug page
       Word2+n: 2xW32 : ADDR + SIZE + nb of memory segments
       Word3+n: Preset, New param!, Skip length, 
          byte stream: nbparams (ALLPARAM), {tag, nbbytes, params}
       list Ends with the SWC ID 0x03FF 
    
    /* 
        LINKED-LIST of SWC , First pass without memory allocation 
    */
    LK1 = addrW32s;

    for (i = 0; i < graph->nb_nodes; i++)
    {   uint32_t iarc, imem, iword32_arcs, nword32_arcs, Lin, Lout;

        HCNEWLINE()
        node = &(graph->all_nodes[i]);

        /* word 0 - main Header */
        FMT0 = 0;
        ST(FMT0, PRIORITY_LW0,  node->NODE_assigned_priority);
        ST(FMT0, PROCID_LW0,    node->NODE_assigned_proc);
        ST(FMT0, ARCHID_LW0,    node->NODE_assigned_arch);
        ST(FMT0, LOADFMT_LW0,   node->using_arc_format);
        ST(FMT0, ARCLOCK_LW0,   node->locking_arc);
        ST(FMT0, SCRIPT_LW0,    node->local_script_index);
        ST(FMT0, ARCSRDY_LW0,   node->nbInputArc + node->nbOutputArc);
        ST(FMT0, NBARCW_LW0,    node->nbParamArc + node->nbInputArc + node->nbOutputArc);
        ST(FMT0, NODE_IDX_LW0,   node->platform_NODE_idx);

        sprintf(tmpstring, "-----  %s(%d) idx:%d Nrx %d Ntx %d ArcFmt %d lockArc %d", 
            node->nodeName, node->graph_instance, node->platform_NODE_idx, node->nbInputArc, node->nbOutputArc, node->using_arc_format, node->locking_arc);
        GTEXT(tmpstring); GWORDINC(FMT0);
        
        sprintf(tmpstring, "_%d      ",node->graph_instance); HCDEF_SSDC(node->nodeName, tmpstring, addrW32s - 1, " node position in the graph");  

        /* word 1 - arcs */
        nword32_arcs = ((1 + node->nbInputArc + node->nbOutputArc) / 2);
        for (iword32_arcs = 0; iword32_arcs < nword32_arcs; iword32_arcs++)
        {   iarc = nword32_arcs/2;
            arc = &(node->arc[iarc]);
            FMT0 = 0;
            sprintf(tmpstring, "_%d_arc_%d",node->graph_instance, arc[iarc  ].arcID); HCDEF_SSDC(node->nodeName, tmpstring, addrW32s, " node arc");  
            sprintf(tmpstring, "_%d_arc_%d",node->graph_instance, arc[iarc+1].arcID); HCDEF_SSDC(node->nodeName, tmpstring, addrW32s, " node arc");  

            ST(FMT0, ARC0_LW1,  arc[iarc].arcID);           ST(FMT0, ARC1_LW1,  arc[iarc+1].arcID);          
            ST(FMT0, ARC0D_LW1, arc[iarc].rx0tx1);          ST(FMT0, ARC1D_LW1, arc[iarc+1].rx0tx1);    
            ST(FMT0, DBGB0_LW1, arc[iarc].debug_page);      ST(FMT0, DBGB1_LW1, arc[iarc+1].debug_page);    

            Lin  = (int)(graph->arcFormat[graph->arc[arc[iarc  ].arcID].fmtProd].frame_length);
            Lout = (int)(graph->arcFormat[graph->arc[arc[iarc+1].arcID].fmtCons].frame_length);
            if (Lin>Lout)
            {   strcpy(tmpstring2, "!?!");
            } else
            {   strcpy(tmpstring2, "   ");
            }
            sprintf(tmpstring, "ARC %d Rx0Tx1 %d L=%d dbgpage%d -- ARC %d Rx0Tx1 %d L=%d dbgpage%d %s",
                arc[iarc  ].arcID, arc[iarc  ].rx0tx1, Lin, arc[iarc  ].debug_page, 
                arc[iarc+1].arcID, arc[iarc+1].rx0tx1, Lout, arc[iarc+1].debug_page, tmpstring2); /* check production < consumption capability */
            GTEXT(tmpstring); GWORDINC(FMT0);
        }

        /* word 2 - memory banks  ---  allocations are made in the second pass */
        for (imem = 0; imem < node->nbMemorySegment; imem++) 
        {   FMT0 = 0; 
            ST(FMT0, NALLOCM1_LW2, (node->nbMemorySegment) - 1); 
            ST(FMT0, XDM11_LW2, (0 == node->variable_data_rate));      /* same data rate on Rx and Tx, invert the flag */
            if (imem == 0) 
            {   sprintf(tmpstring, "Nb Memreq-1 %d  XDM11_same_rate %d", (node->nbMemorySegment)-1, (0 == node->variable_data_rate));
            } else
            {   sprintf(tmpstring, "");
            }
            GTEXT(tmpstring); GWORDINC(FMT0);
            FMT0 = 0; /* size of the segment */  GTEXT(""); GWORDINC(FMT0);
        }

        /* word 3 - parameters */
        FMT0 = 0;                   // Param header
        ST(FMT0, PARAM_TAG_LW3, node->TAG); 
        ST(FMT0,    PRESET_LW3, node->preset); 
        ST(FMT0,   TRACEID_LW3, node->trace_ID); 
        ST(FMT0, W32LENGTH_LW3, node->ParameterSizeW32  + 1); /* the parameter section is Header + PackedParameters[]  */
        sprintf(tmpstring, "ParamLen %d+1 Preset %d Tag0ALL %d", node->ParameterSizeW32, node->preset, node->TAG);
        GTEXT(tmpstring); 
        GWORDINC(FMT0);

        for (j = 0; j < node->ParameterSizeW32; j++)
        {   
            sprintf(tmpstring, "(%d)", j); 
            FMT0 = node->PackedParameters[j];
            GTEXT(tmpstring); GWORDINC(FMT0);
        }
    }


    /* LAST WORD */
    GTEXT("^^^^^^^^^ LINKED-LIST END ^^^^^^^^^ vvvvvvvvvvv RAM vvvvvvvvvvv"); GWORDINC(GRAPH_LAST_WORD);


    LK_PIO = addrW32s;

    addrW32s_backup = addrW32s;     FMT0 = 0;                       // write GR2 bit-field 
    ST(FMT0, ARC_DEBUG_IDX_GR2,     0);
    ST(FMT0, LINKEDLISTSZW32_GR2, addrW32s-LK1);                    // size of the LinkedList section 
    sprintf(tmpstring, "LinkedList size = %d, ongoing IO bytes, Arc debug table size %d", addrW32s-LK1, 0); 
    addrW32s = LK_lkSize; GTEXT(tmpstring); GWORDINC(FMT0); 
    addrW32s = addrW32s_backup;                                     // restore the index to the graph


    /*     
        PIO "ONGOING" (1 Byte per IO)  size = [NB_IOS_GR1] (in instance "uint8_t *ongoing")

        In the AL : graph_io_idx = platform_io_al_idx_to_graph[HWIO]
        In the scheduler instance : pointer to the table of PIOs  S->ioctrl[graph_io_idx]
        At init : all_arcs[0..] descriptors hold packs of 16 bytes of IOs used in this graph (xx IOCTRL_LSB)
    */
    {   union 
        {   uint32_t W32;
            uint8_t W8[4];
        } pio_ongoing;

        m = 0; /* byte index */
        pio_ongoing.W32 = 0xFFFFFFFF;
        for (j = i = 0; i < graph->nb_arcs; i++)
        {
            arc = &(graph->arc[i]);
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



    /* FORMAT used by the arcs (4 words each stream_format)  
        Word0: Frame size, interleaving scheme, arithmetics raw data type
        Word1: time-stamp, domain, nchan, physical unit (pixel format, IMU interleaving..)
        Word2: depends on IO Domain
        Word3: depends on IO Domain

        can be used for tunable formats
            This is used the SWC generates variable frame formats (JPG decoder, MP3 decoder..)
    */
    for (i = 0; i < graph->nb_formats; i++)
    {   struct formatStruct *format;
        
        FMT0 = 0;
        format = &(graph->arcFormat[i]);
        ST(FMT0, FRAMESIZE_FMT0, (uint32_t)(0.5+format->frame_length));

        FMT1 = 0;
        ST(FMT1, RAW_FMT1, format->raw_data);
        ST(FMT1, TSTPSIZE_FMT1,0);
        ST(FMT1, TIMSTAMP_FMT1, 0);
        ST(FMT1, INTERLEAV_FMT1, format->deinterleaved);
        ST(FMT1, NCHANM1_FMT1, format->nchan -1);  

        FMT2 = 0;
        FMT3 = 0;
        sprintf(tmpstring, "Format %2d frameSize %d ", i, (uint32_t)(0.5+format->frame_length)); GTEXT(tmpstring);  GWORDINC(FMT0);
        sprintf(tmpstring, "          nchan %d raw %d", format->nchan, format->raw_data); GTEXT(tmpstring); GWORDINC(FMT1);
        sprintf(tmpstring, "          domain-dependent");  GTEXT(tmpstring); GWORDINC(FMT2);
        sprintf(tmpstring, "          domain-dependent");  GTEXT(tmpstring); GWORDINC(FMT3);
    }


    /*
        Memory consumption adjusted to the graph consumption in VID0
    */
    if (graph->option_graph_location == COPY_CONF_GR0_COPY_ALL_IN_RAM || 
        graph->option_graph_location == COPY_CONF_GR0_ALREADY_IN_RAM )
    {   
        vid_malloc(VID0, 4 * (addrW32s + (SIZEOF_ARCDESC_W32 * graph->nb_arcs) + graph->debug_pattern_size), 
            MEM_REQ_4BYTES_ALIGNMENT, &i, 0, "COPY_CONF_GR0", platform, graph);
    }

    if (graph->option_graph_location == COPY_CONF_GR0_FROM_PIO)
    {
        vid_malloc(VID0, 4 * (addrW32s + (SIZEOF_ARCDESC_W32 * graph->nb_arcs) - LK_PIO  + graph->debug_pattern_size), 
           MEM_REQ_4BYTES_ALIGNMENT,  &i, 0, "COPY_CONF_GR0", platform, graph);
    }



    /*
        ARC descriptors (4 words each)  + buffer memory allocation
        Word0: base offsetm data format, need for flush after write
        Word1: size, debug result registers
        Word2: read index, ready for read, flow error and debug tasks index
        Word3: write index, ready for write, need realignment flag, locking byte 
    */
    HCNEWLINE()
    for (i = 0; i < graph->nb_arcs; i++)
    {   float sizeProd, sizeCons, jitterFactor, size, tmpSize;

        sprintf(tmpstring, "arc_%d     ",i); HCDEF_SSDC("", tmpstring, addrW32s, " arc descriptor position in the graph");  

        arc = &(graph->arc[i]);

        ARCW0 = ARCW1 = ARCW2 = ARCW3 = ARCW4 = 0;


        jitterFactor = graph->arc[i].sizeFactor;
        sizeProd = graph->arcFormat[graph->arc[i].fmtProd].frame_length;
        sizeCons = graph->arcFormat[graph->arc[i].fmtCons].frame_length;

        if (sizeProd > sizeCons) 
        {   size = sizeProd; 
        } else 
        {   size = sizeCons;
        }
        tmpSize = size = 0.5f + (jitterFactor * size);

        /* if the IO buffer is declared in the BSP: no malloc */
        if (arc->ioarc_flag)
        {   if (arc->set0copy1 == 0)
            {   tmpSize = size = 0;
            }
        }

        /* memory allocation of BUFFERS */
        sprintf(tmpstring, "_%d_buff",i);  
        vid_malloc(arc->memVID, 
                (uint32_t)size, 
                MEM_REQ_4BYTES_ALIGNMENT, 
                &(graph->arc[i].graph_base),  0, tmpstring, 
                platform, graph);


        ARCW0 = arc->graph_base;  
        if (arc->ioarc_flag)
        {
        sprintf(tmpstring, "IO-ARC descriptor(%d) Base %Xh (%Xh words) fmtProd_%d frameL %2.1f", i, 
            graph->arc[i].graph_base, RD((graph->arc[i].graph_base), BASEIDX_ARCW0)/4, graph->arc[i].fmtProd, graph->arcFormat[graph->arc[i].fmtProd].frame_length); 
        } else
        {
        sprintf(tmpstring, "ARC descriptor(%d) Base %Xh (%Xh words) fmtProd_%d frameL %2.1f", i, 
            graph->arc[i].graph_base, RD((graph->arc[i].graph_base), BASEIDX_ARCW0)/4, graph->arc[i].fmtProd, graph->arcFormat[graph->arc[i].fmtProd].frame_length); 
        }
        GTEXT(tmpstring); GWORDINC(ARCW0); 

        ST(ARCW1, BUFF_SIZE_ARCW1, (uint32_t)tmpSize);
        all_buffers += (uint32_t)tmpSize;

        if (size == 0)
        {   sprintf(tmpstring, "IO buffer with base address redirection, no memory allocation, the Size is from the producer");  
        } else
        {   sprintf(tmpstring, "    Size %Xh[B] fmtCons_%d FrameL %2.1f jitterScaling%4.1f", (uint32_t)size, graph->arc[i].fmtCons, 
                graph->arcFormat[graph->arc[i].fmtCons].frame_length, jitterFactor); 
        }
        GTEXT(tmpstring);GWORDINC(ARCW1);
        sprintf(tmpstring, "arc_buf_%d ",i); HCDEF_SSDC("", tmpstring, graph->arc[i].graph_base, " arc buffer address");

        ST(ARCW2, ARCEXTEND_ARCW2, 0);
        ST(ARCW2,   MPFLUSH_ARCW2, graph->arc[i].flush);
        ST(ARCW2, FLOWERROR_ARCW2, graph->arc[i].flow_error);
        ST(ARCW2,      READ_ARCW2, 0);
        GWORDINC(ARCW2);

        ST(ARCW3, COLLISION_ARCW3, 0);
        ST(ARCW3, ALIGNBLCK_ARCW3, 0);
        ST(ARCW3,     WRITE_ARCW3, 0);
        GWORDINC(ARCW3);

        ST(ARCW4, COMPUTCMD_ARCW4, graph->arc[i].debug_cmd);
        ST(ARCW4, DEBUG_REG_ARCW4, graph->arc[i].debug_reg);
        ST(ARCW4, PRODUCFMT_ARCW4, graph->arc[i].fmtProd);
        ST(ARCW4, CONSUMFMT_ARCW4, graph->arc[i].fmtCons);
        sprintf(tmpstring, "    fmtCons %d fmtProd %d dbgreg %d dbgcmd %d", graph->arc[i].fmtCons, graph->arc[i].fmtProd, graph->arc[i].debug_reg, graph->arc[i].debug_cmd); 
        GTEXT(tmpstring); GWORDINC(ARCW4);

        //sprintf(tmpstring, "    debug register"); 
        //GTEXT(tmpstring); GWORDINC(ARCW5);

    }
    HCNEWLINE()   


    /*
        ==== debug filling pattern ====
    */
    for (i = 0; i < graph->debug_pattern_size; i++)
    {   
        sprintf(tmpstring, "    padding "); GTEXT(tmpstring);GWORDINC(graph->debug_pattern);
    }
    HCNEWLINE()   


    /* 
        LINKED-LIST of SWC , second pass with ONLY static memory allocation 
    */

    ENDARCS = addrW32s;
    addrW32s = LK1;
    for (i = 0; i < graph->nb_nodes; i++)
    {   uint32_t imem, iword32_arcs, nword32_arcs;

        /* word 0 - main Header */
        node = &(graph->all_nodes[i]); GINC 

        /* word 1 - arcs */
        nword32_arcs = ((1 + node->nbInputArc + node->nbOutputArc) / 2);
        for (iword32_arcs = 0; iword32_arcs < nword32_arcs; iword32_arcs++)
        {   GINC
        }

        /* word 2 - memory banks  ---  Static allocations are made in THIS second pass */
        for (imem = 0; imem < node->nbMemorySegment; imem++)    
        {   struct node_memory_bank *membank = &(node->memreq[imem]);
            uint32_t graph_malloc;

            if (membank->stat0work1ret2 != MEM_TYPE_WORKING)
            {
                compute_memreq (membank, graph->arcFormat);
                graph_malloc = node->memreq[imem].malloc_add;
                membank->graph_memreq_size += graph_malloc; 

                sprintf(tmpstring, "_%d_mem_%d",node->graph_instance, imem);
            
                vid_malloc (membank->mem_VID, membank->graph_memreq_size, membank->alignmentBytes,
                    &(membank->graph_base), MEM_TYPE_STATIC, tmpstring, platform, graph);

                HCDEF_SSDC(node->nodeName, tmpstring, membank->graph_base, " node static memory address"); 

                FMT0 = graph->binary_graph[addrW32s];
                ST(FMT0, BASEIDXOFFLW2, node->memreq[imem].graph_base);    // address 
                GWORDINC(FMT0);

                sprintf(tmpstring, "Reserved static memory bank(%d) = bank %ld stat0work1ret2 = %d size %ld ", 
                    imem, RD(membank->graph_base, DATAOFF_ARCW0), membank->stat0work1ret2, membank->graph_memreq_size); 
                GTEXT(tmpstring); FMT0 = membank->graph_memreq_size; GWORDINC(FMT0);
            }
            else
            {   GINC    /* address */
                GINC    /* size */
            }
        }


        /* word 3 - parameters  + header */
        for (j = 0; j < node->ParameterSizeW32  +1; j++)
        {   GINC    
        }
    }   
    

    /* 
        LINKED-LIST of SWC , third pass with ONLY scratch / working memory allocation 
    */
    addrW32s = LK1;
    for (i = 0; i < graph->nb_nodes; i++)
    {   uint32_t imem, iword32_arcs, nword32_arcs;

        /* word 0 - main Header */
        node = &(graph->all_nodes[i]); GINC 

        /* word 1 - arcs */
        nword32_arcs = ((1 + node->nbInputArc + node->nbOutputArc) / 2);
        for (iword32_arcs = 0; iword32_arcs < nword32_arcs; iword32_arcs++)
        {   GINC
        }

        /* word 2 - memory banks  ---  Working allocations are made in THIS second pass */
        for (imem = 0; imem < node->nbMemorySegment; imem++)    
        {   struct node_memory_bank *membank = &(node->memreq[imem]);
            uint32_t graph_malloc;

            if (membank->stat0work1ret2 == MEM_TYPE_WORKING)
            {
                compute_memreq (membank, graph->arcFormat);
                graph_malloc = node->memreq[imem].malloc_add;
                membank->graph_memreq_size += graph_malloc; 
            
                sprintf(tmpstring, "_%d_mem_%d",node->graph_instance, imem);  

                vid_malloc(membank->mem_VID, membank->graph_memreq_size, membank->alignmentBytes, 
                    &(membank->graph_base), MEM_TYPE_WORKING, tmpstring, platform, graph);

                sprintf(tmpstring2, "%s offset %d index %d", " node scratch memory address", RD(membank->graph_base, DATAOFF_ARCW0), RD(membank->graph_base, BASEIDX_ARCW0));  
                HCDEF_SSDC(node->nodeName, tmpstring, membank->graph_base, tmpstring2);

                FMT0 = graph->binary_graph[addrW32s];
                ST(FMT0, BASEIDXOFFLW2, node->memreq[imem].graph_base);    // address 
                GWORDINC(FMT0);

                sprintf(tmpstring, "Scratch memory bank(%d) = bank %ld stat0work1ret2 = %d  size %ld ",
                    imem, RD(membank->graph_base, DATAOFF_ARCW0), membank->stat0work1ret2,  membank->graph_memreq_size); 
                GTEXT(tmpstring); FMT0 = membank->graph_memreq_size; GWORDINC(FMT0);
            }
            else
            {   GINC    /* address */
                GINC    /* size */
            }
        }

        /* word 3 - parameters + header */
        for (j = 0; j < node->ParameterSizeW32  +1; j++)
        {   GINC    
        }
    }   
    addrW32s = ENDARCS;                    /* restore the original address */


    /* memory consumed */
    {   addrW32s_backup = addrW32s; addrW32s = LKalloc; 
#define NOFF (1<<(DATAOFF_ARCW0_MSB - DATAOFF_ARCW0_LSB +1))
        uint64_t maxSizes[NOFF], used[NOFF], percent[NOFF];
        struct processor_memory_bank *mem;
        for (i = 0; i < NOFF; i++)
        {   maxSizes[i] = 1; used[i] = 0;
        }

        for (i = 0; i < MAX_PROC_MEMBANK; i++)
        {   mem = &(platform->membank[i]); 
            maxSizes[mem->offsetID] += mem->size;
            used[mem->offsetID] += mem->ptalloc_static + mem->max_working;
        }
        for (i = 0; i < NOFF; i++)
        {   percent[i] = (255 * used[i]) / maxSizes[i];
        }
        ST(FMT0, BYTE_0, percent[0]);
        ST(FMT0, BYTE_1, percent[1]);
        ST(FMT0, BYTE_2, percent[2]);
        ST(FMT0, BYTE_3, percent[3]);
        GTEXT("[5] memory consumed 0,1,2,3"); GWORDINC(FMT0);
        ST(FMT0, BYTE_0, percent[4]);
        ST(FMT0, BYTE_1, percent[5]);
        ST(FMT0, BYTE_2, percent[6]);
        ST(FMT0, BYTE_3, percent[7]);
        GTEXT("[6] memory consumed 4,5,6,7 ... "); GWORDINC(FMT0);
        addrW32s = addrW32s_backup;
    }


    sprintf(tmpstring, "Graph size = Flash=%d[W]+RAM%d[W]  +Buffers=%d[B] %d[W]", LK_PIO, addrW32s-LK_PIO, all_buffers, (all_buffers + 3)>>2);
    fprintf (graph->ptf_graph_bin, "0x%08X, // ------- %s \n", addrW32s, tmpstring);

    for (i = 0; i < addrW32s; i ++)
    {
        fprintf (graph->ptf_graph_bin, "0x%08X, // %03X %03X %s \n", 
            graph->binary_graph[i], i*4, i, graph->binary_graph_comments[i]);
    }

    //{   uint32_t ibank; 
    //    for (ibank = 0; ibank < platform->nbMemoryBank_detailed; ibank++)
    //    {   if (platform->membank[ibank].ptalloc_static > 0)
    //        {  fprintf (graph->ptf_graph_bin, "// %s \n", platform->membank[ibank].comments);
    //        }
    //    }
    //}
}


#ifdef __cplusplus
}
#endif