/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Tools
 * Title:        generic graph translation to platform specific graph
 * Description:  translates the IO domain to specifics of the platform
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

struct stream_graph_linkedlist *dbggraph;
#define DBGG(mem,c) {dbggraph->dbg[dbggraph->idbg].address=mem; strcpy(dbggraph->dbg[dbggraph->idbg].toprint, c);(dbggraph->idbg)++;}
char DBG[200];

char current_line[MAXNBCHAR_LINE];

/* 
    enum stream_io_domain : list of stream "domains" categories 
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).

    a change in this list 
*/
#define CHARLENDOMAINS 80

char raw_type_name[LAST_RAW_TYPE][CHARLENDOMAINS] = 
{
    "int8",             /* STREAM_S8,  */
    "uint8",            /* STREAM_U8,  */
    "int16",            /* STREAM_S16, */
    "uint16",           /* STREAM_U16, */
    "int32"             /* STREAM_S32, */
    "uint32",           /* STREAM_U32, */
    "float",            /* STREAM_Q31, */
    "double",           /* STREAM_FP64,*/
};

/* mem_speed_type */
char memory_speed_name[][CHARLENDOMAINS] = 
{
    "bestEffort",       /* MEM_SPEED_REQ_ANY           0    /* no requirement  */
    "fast",             /* MEM_SPEED_REQ_FAST          1    /* will be internal SRAM when possible */
    "criticalFast",     /* MEM_SPEED_REQ_CRITICAL_FAST 2    /* will be TCM when possible */
};

/* mem_mapping_type */
char memory_usage_name[][CHARLENDOMAINS] = 
{
    "static",           /* MEM_TYPE_STATIC          0  (LSB) memory content is preserved (default ) */
    "worging",          /* MEM_TYPE_WORKING         1  scratch memory content is not preserved between two calls */
    "pseudoWorking",    /* MEM_TYPE_PSEUDO_WORKING  2  static only during the uncompleted execution state of the SWC, see “NODE_RUN” */
    "backup",           /* MEM_TYPE_PERIODIC_BACKUP 3  static parameters to reload for warm boot after a crash, holding for example ..*/
};

char domain_name[IO_DOMAIN_MAX_NB_DOMAINS][CHARLENDOMAINS] = 
{
    "general",              // #define IO_DOMAIN_GENERAL           
    "audio_in",             // #define IO_DOMAIN_AUDIO_IN           
    "audio_out",            // #define IO_DOMAIN_AUDIO_OUT          
    "gpio_in",              // #define IO_DOMAIN_GPIO_IN            
    "gpio_out",             // #define IO_DOMAIN_GPIO_OUT           
    "motion",               // #define IO_DOMAIN_MOTION            
    "2d_in",                // #define IO_DOMAIN_2D_IN              
    "2d_out",               // #define IO_DOMAIN_2D_OUT             
    "analog_in",            // #define IO_DOMAIN_ANALOG_IN          
    "analog_out",           // #define IO_DOMAIN_ANALOG_OUT        
    "rtc",                  // #define IO_DOMAIN_RTC                
    "user_interface_in",    // #define IO_DOMAIN_USER_INTERFACE_IN  
    "user_interface_out",   // #define IO_DOMAIN_USER_INTERFACE_OUT 
};                                     


/* 
    ====================================================================================    
*/
void read_top_graph_interface (char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{
    char *pt_line2;
    char shortFormat[2];
    uint32_t fw_io_idx, iformat, settings, idx, memVID_buffer, i;
    struct arcStruct *arcIO;

    for (i=0; i<=platform->nb_hwio_stream; i++)
    {   graph->arcIO[i].top_graph_index = 0xFFFF;   /* reset the index for later search the good ones */
        graph->arcIO[i].arcIDstream = 0xFFFF;
    }
    
    graph->nbio_interfaces = 0;
    graph->nbio_interfaces_with_arcBuffer = 0;
    
    while (1)
    {   
        pt_line2 = *pt_line;
        if (fields_extract(pt_line, "CIII", shortFormat, &idx, &iformat, &fw_io_idx) < 0) break;
        if (shortFormat[0] != 'S' && shortFormat[0] != 's')   /* is it a long format ? */
        {   *pt_line = pt_line2;
            if (fields_extract(pt_line, "CIIIII", shortFormat, &idx, &iformat, &fw_io_idx, &settings, &memVID_buffer) <0) break;
        }
    
        arcIO = &(graph->arcIO[fw_io_idx]);
        graph->nbio_interfaces ++;
    
        if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
        {   arcIO->memVID = 0;
            arcIO->si.settings= 0;
        }
        else
        {   arcIO->memVID = memVID_buffer;
            arcIO->si.settings= settings;
        }
    
        arcIO->top_graph_index = idx;
        arcIO->format_idx = iformat;
        arcIO->fw_io_idx = fw_io_idx;
        arcIO->arcIDstream = 0xFFFF;
    
        /* copy data from platform_io */
        graph->arcFormat[iformat].sc = platform->io_stream[fw_io_idx].sc;
        arcIO->sc                    = platform->io_stream[fw_io_idx].sc;
        arcIO->si                    = platform->io_stream[fw_io_idx].si;
    
        //@@@@@  depending on settings, change frame_size_option, sampling_rate_option
        //@@@@@  change nbchannel_option interleaving_option 
        //@@@@@  update the format and create a WARNING in case of mismatch => need format converter
    
        if (arcIO->si.graphalloc_X_bsp_0 > 0)
        {   graph->nbio_interfaces_with_arcBuffer++;
        }
    
        if (FoundEndSection)
        {   break;
        }
    }
    
     if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
         jump2next_valid_line(pt_line);     /* remove trailing _end_ (s)  */
}


/* ====================================================================================   
    Read and pack the script until finding "_end_" / SECTION_END

    script_byte_codes
    ....
    _end_               end of byte codes  
*/
void stream_tool_read_script(char **pt_line, char *script_bytecode, uint32_t *nbBytesCodes)
{
    uint8_t raw_type;
    uint32_t nb_raw, nbytes, nbits;

    *nbBytesCodes = 0;

    while (1)
    {
        read_binary_param(pt_line, script_bytecode, &raw_type, &nb_raw);
        if (nb_raw == 0)
            break;
        nbits = stream_bitsize_of_raw(raw_type);
        nbytes = (nbits * nb_raw)/8;
        script_bytecode = &(script_bytecode[nbytes]);
        *nbBytesCodes += nbytes;
    }
}


/*
====================================================================================   
*/
void stream_tool_script(char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{
    char node_name[MAXINPUT];
    struct stream_node_manifest *node, *platform_node;
    char shortFormat[2];
    uint32_t nreg, stackdepthW32, nbByteCodes=0, nbBytesCodes, privateRAM, iscripts;
    uint32_t scriptFormat;  /* select byte-code format / binary code of the processor */
  
    //FILE *ptf_graph_script_bytecode;
    //char filename[120];

    jump2next_valid_line(pt_line);  // skip "script" 

    node = &(graph->all_nodes[graph->nb_nodes]); 
    platform_node = 0;
    if (fields_extract(pt_line, "ci", node_name, &(node->instance_idx)) < 0) exit (-4);  
    node_name[MAXINPUT-1] = '\0';

    

  
    iscripts = node->instance_idx;     // @@@ warning this index must fit with the SWC declaration &(node->local_script_index)
    fields_extract(pt_line, "ciiii", &shortFormat, &nreg, &stackdepthW32, &privateRAM, &scriptFormat);  
  
    //sprintf (filename, "SCRIPT_%d.txt", iscripts); 
    //ptf_graph_script_bytecode = fopen(filename, "wt");
  
    if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
    {   privateRAM = 0;
        scriptFormat = 0;
    }
  
    // current BYTECODE offset in script_bytecode[]
    graph->script_indirect[INDEX_SCRIPT_STRUCT_SIZE*iscripts+INDEX_SCRIPT_OFFSET] = graph->nb_byte_code; 
    graph->script_nregs[iscripts] = nreg;
    graph->script_stackdepthW32[iscripts] = stackdepthW32;
    graph->nb_scriptsARC ++;                                // one arc descriptor per script
  
    if (privateRAM == 0)                                    // the script is using the shared memory section
    {   graph->max_nregs = MAX(graph->max_nregs, nreg); 
        graph->max_shared_stackW32 = MAX(graph->max_shared_stackW32, stackdepthW32); 
        graph->scriptRAMshared[iscripts] = 1;
        graph->atleastOneScriptShared = 1;
    } 
    
    stream_tool_read_script(pt_line, &(graph->script_bytecode[graph->nb_byte_code]), &nbBytesCodes);
    nbBytesCodes = (3+nbBytesCodes) & 0xFFFFFFFC;   // round it to W32
    
    graph->nb_byte_code += nbBytesCodes;
    graph->nb_scripts ++;
  
    /* extra parameters in FLASH following the code */
    if (0 == strncmp (*pt_line, PARAMETER_START, strlen(PARAMETER_START)))
    {   stream_tool_read_script(pt_line, &(graph->script_bytecode[graph->nb_byte_code]), &nbBytesCodes);
        nbBytesCodes = (3+nbBytesCodes) & 0xFFFFFFFC;   // round it to W32
        graph->nb_byte_code += nbBytesCodes;
    }
}




/*
====================================================================================   
*/
void stream_tool_read_node(char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{
    char *pt_line2;
    char node_name[MAXINPUT];
    struct stream_node_manifest *node, *platform_node;
    uint32_t inode, tmp;
    char *pt_line_saved;
    char shortFormat[2];

    jump2next_valid_line(pt_line);  // skip "node" 

    node = &(graph->all_nodes[graph->nb_nodes]); 
    platform_node = 0;
    if (fields_extract(pt_line, "ci", node_name, &(node->instance_idx)) < 0) exit (-4);  
    node_name[MAXINPUT-1] = '\0';

    //if (0 != strcmp(node_name, "arm_stream_script"))        /* is it NOT a script ? */
    for (inode = 1; inode < platform->nb_nodes+1; inode++)
    {   platform_node = &(platform->all_nodes[inode]);
        if (0 == strncmp(node_name, platform_node->nodeName,strlen(platform_node->nodeName)))
        {   break;
        }
    }

    /* COPY DEFAULT DATA FROM MANIFEST in graph->all_node[] */
    *node = platform->all_nodes[inode];
    node->swc_idx = inode;

    pt_line2 = *pt_line;
    if (fields_extract(pt_line, "CI", shortFormat, &(node->preset)) < 0) 
        exit(-4);

    if (shortFormat[0] != 'S' && shortFormat[0] != 's')   /* is it a long format ? */
    {   *pt_line = pt_line2;
        if (fields_extract(pt_line, "CIIIIII", shortFormat,
            &(node->preset),
            &(node->local_script_index),
            &(node->swc_assigned_arch),
            &(node->swc_assigned_proc),
            &(node->swc_assigned_priority),
            &(node->swc_verbose)) <0) exit(-4);
    }

    if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
    {   node->local_script_index = 0;
        node->swc_assigned_arch = 0;
        node->swc_assigned_proc = 0;
        node->swc_assigned_priority = 0;
        node->swc_verbose = 0;
    }
    else /* read the VID of the node memory segments, finishing with the one for code */
    {   int Pseg;
        switch (node->nbMemorySegment)
        {   case 4: fields_extract(pt_line, "IIIII", &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID), &(node->memreq[2].VID), &(node->memreq[0].VID)); break;
            case 3: fields_extract(pt_line, "IIII",  &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID), &(node->memreq[2].VID)); break;
            case 2: fields_extract(pt_line, "III",   &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID)); break;
            case 1: fields_extract(pt_line, "II",    &Pseg, &(node->memreq[0].VID)); break;
            default: case 0: break;
        }
    }
  
     ST(node->headerPACK, SWC_IDX_LW0, node->swc_idx);
     ST(node->headerPACK, ARCSRDY_LW0, platform_node->nbInputOutputArc);
     ST(node->headerPACK, NBARCW_LW0, platform_node->nbParameArc + platform_node->nbInputOutputArc);
     ST(node->headerPACK, PRIORITY_LW0, node->swc_assigned_priority);
     ST(node->headerPACK, ARCHID_LW0, node->swc_assigned_arch);
     ST(node->headerPACK, PROCID_LW0, node->swc_assigned_proc);
     ST(node->headerPACK, SCRIPT_LW0, node->local_script_index);
  
     pt_line_saved = *pt_line;
  
     if (0 == strncmp (*pt_line, PARAMETER_START, strlen(PARAMETER_START)))
     {  
         jump2next_valid_line(pt_line);
         stream_tool_read_parameters(pt_line, node); /*  returns on "_end_" SECTION_END */
     }
     else
     {   node->PackedParameters[0] = 0;
         node->defaultParameterSizeW32 = 0;
         ST(node->PackedParameters[0], W32LENGTH_LW3, 1);
         ST(node->PackedParameters[0], PRESET_LW3, node->preset);
         *pt_line = pt_line_saved;
     }
  
     ST(node->PackedParameters[0], TRACEID_LW3, node->swc_verbose);
  
     /* keep the parameter header for the PRESET set at reset */
     if (node->defaultParameterSizeW32 < 1) node->defaultParameterSizeW32 =1;
  
     graph->cumulFlashW32 += 1;                                /* SWC header */
     tmp = node->nbInputArc + node->nbOutputArc + node->nbParameArc;
     graph->cumulFlashW32 += (tmp+1)/2;                        /* SWC arcs */
     graph->cumulFlashW32 += 2*(node->nbMemorySegment);        /* two words per memory bank */
     graph->cumulFlashW32 += node->defaultParameterSizeW32;    /* parameters */
  
     graph->nb_nodes++;
}



/*
==================================   
        arc
            0 0  0 0 0 0 0   0 0       ProdFMT ConsFMT   OVF, DBGCMD, REG, FLUSH, EXTD     VID IOSizeMulfac 
            graph_interface    0  0        0= IO "interface" from application processor, second parameter is unused
            arm_stream_filter   0  0        [0]:filter instance       [0]:RX arc of the node
        ;    parameters
        ;       _include    2   TestPattern.txt filepathID and fileName (test-pattern, NN model, ...)
        ;       parameter_end    
            arc_end
           
*/
void stream_tool_read_arcs (char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{   uint32_t arcID, ProdFMT, ConsFMT, VID,   OVF, CMD, REG, FLUSH, EXTD;
    uint32_t inode, instSrc, top_graph_idx, set0_copy1, iarcSrc, instDst, iarcDst, node_inst, minFrameSize;
    float IOSizeMulfac;
    struct stream_node_manifest *node, *platform_node;
    char nodeNameSrc[120], nodeNameDst[120], shortFormat[2], *node_name;
    struct formatStruct *format;
    uint32_t tmp, arcBufferSizeByte;
    struct arcStruct *arc;
    struct arcStruct *arcIO;
    uint32_t fw_io_idx, PAGE;
    char *pt_line2;

    jump2next_valid_line(pt_line);

    node = &(graph->all_nodes[0]);  // init
    arcID = 0;

    while (1)
    {
        arc = &(graph->arc[arcID]); 
        arc->arcID = arcID;

        pt_line2 = *pt_line;
        if (fields_extract(pt_line, "cii",  shortFormat, &ProdFMT, &ConsFMT) < 0) 
            break;

        if (shortFormat[0] != 'S' && shortFormat[0] != 's')   /* is it a long format ? */
        {   *pt_line = pt_line2;
            if (fields_extract(pt_line, "ciiiiiiiiif",  shortFormat,
            &ProdFMT, &ConsFMT, &OVF, &CMD, &REG, &PAGE, &FLUSH, &EXTD, &VID, &IOSizeMulfac
            ) < 0) exit (-5);
        }
    
        if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
        {   OVF = 0; CMD = 0; REG = 0; PAGE = 0;
            FLUSH = 0; EXTD = 0; VID = 0; IOSizeMulfac = 1.0;
        }
        graph->nb_arcs++;

        fields_extract(pt_line, "cii", nodeNameSrc, &instSrc, &iarcSrc);  
        if (0 == strncmp(nodeNameSrc, "arm_stream_script",NBCHAR_LINE))
        {
        }
        else
        {
            fields_extract(pt_line, "cii", nodeNameDst, &instDst, &iarcDst);  
        }


        //minFrameSize=0;
     
        if (0 == strncmp(nodeNameSrc, "graph_interface",NBCHAR_LINE))
        {   
            top_graph_idx = instSrc;
            set0_copy1 = iarcSrc;
            for (fw_io_idx=0; fw_io_idx<platform->nb_hwio_stream; fw_io_idx++)
            {   if (top_graph_idx == graph->arcIO[fw_io_idx].top_graph_index)
                    break;
            }
            arcIO = &(graph->arcIO[fw_io_idx]);
            arcIO->arcIDstream = arcID;             /* arcID associated to this IO */
            arcIO->si.set0_copy1 = set0_copy1;
        }
        else
        {   fw_io_idx = 0;  /* this is not an IO arc */
            /* search SRC node , assign the arcID*/
            node_name = nodeNameSrc;
            node_inst = instSrc;
            for (inode = 0; inode < platform->nb_nodes+1/* io node */; inode++)
            {   
                node = &(graph->all_nodes[inode]);  // scan the node of the graph, not the platform manifest
                                                    // node->idx point to the node in flash
                platform_node = &(platform->all_nodes[node->swc_idx]); // platform manifest, to read the name

                if (0 == strncmp(node_name, platform_node->nodeName,NBCHAR_LINE))
                {   node = &(graph->all_nodes[inode]);  // inode=0 will have dummy arcs
                    if (node_inst == node->instance_idx)    // is this the good instance of the source node ?
                    {   
                        node->arc[iarcSrc].arcID = arcID;   // save the arc src index to the node structure
                        break;
                    }
                }
            } 
        }
        format = &(graph->arcFormat[ProdFMT]);
        minFrameSize = RD(format->FMT0, FRAMESIZE_FMT0);

        /* ------------------------------------------------------- */

        if (0 == strncmp(nodeNameDst, "graph_interface", NBCHAR_LINE))
        {   uint32_t fw_io_idx;
            top_graph_idx = instDst;
            set0_copy1 = iarcSrc;
            for (fw_io_idx=0; fw_io_idx<platform->nb_hwio_stream; fw_io_idx++)
            {   if (top_graph_idx == graph->arcIO[fw_io_idx].top_graph_index)
                    break;
            }
            arcIO = &(graph->arcIO[fw_io_idx]);
            arcIO->arcIDstream = arcID;             /* arcID associated to this IO */
            arcIO->si.set0_copy1 = set0_copy1;

        }
        else
        {            /* search DST node , assign the arcID*/
            node_name = nodeNameDst;
            node_inst = instDst;
            for (inode = 0; inode < platform->nb_nodes+1 /* io node */; inode++)
            {   
                node = &(graph->all_nodes[inode]);  // scan the node of the graph, not the platform manifest
                                                    // node->idx point to the node in flash
                platform_node = &(platform->all_nodes[node->swc_idx]); // platform manifest, to read the name
        
                if (0 == strncmp(node_name, platform_node->nodeName,NBCHAR_LINE))
                {   node = &(graph->all_nodes[inode]);

                    if (node_inst == node->instance_idx)
                    {   
                        node->arc[iarcDst].arcID = arcID;  // save the arc src index to the node structure 
                        break;
                    }
                } 
            }             
        }

        node->arc[iarcSrc].debug_page_DBGB0_LW1 = PAGE; // DBGB0_LW1
        node->arc[iarcDst].debug_page_DBGB0_LW1 = PAGE; // must be the page of the producer node


        {
            extern uint32_t gcd(uint32_t a, uint32_t b);
            extern uint32_t lcm(uint32_t a, uint32_t b);
            uint32_t a, b, minprodcons, maxprodcons, ratio;

            format = &(graph->arcFormat[ConsFMT]);
            tmp = RD(format->FMT0, FRAMESIZE_FMT0);
            a = minFrameSize;
            b = RD(format->FMT0, FRAMESIZE_FMT0);
            //minFrameSize = lcm(a,b);    // take LCM of consumer,producer frame sizes

            if (a == b)
            {   minFrameSize = a;
            }
            else
            {   minprodcons = MAX(1, MIN(a,b));
                maxprodcons = MAX(a,b);
                ratio = maxprodcons / minprodcons;
                minFrameSize = minprodcons * (ratio + 1);
            }
        }



        minFrameSize = (int)(0.5 + (IOSizeMulfac * minFrameSize));      /* buffer size rescaling, Byte accurate */
        /* check it is larger than what the IO-Interface needs */

        arcIO = &(graph->arcIO[fw_io_idx]);
        if (minFrameSize < arcIO->si.graphalloc_X_bsp_0 * minFrameSize)   /* @@@ to check with a test graphTxt */
        {   minFrameSize = arcIO->si.graphalloc_X_bsp_0 * minFrameSize;
        }

        arcBufferSizeByte = minFrameSize;               /* in Bytes */           

        // @@@@ arc sequence starting with the one used for locking, the streaming arcs, then the metadata arcs 
        // @@@@ arc(tx) used for locking is ARC0_LW1

        ST(arc->ARCW0, PRODUCFMT_ARCW0, ProdFMT);
        ST(arc->ARCW0,   DATAOFF_ARCW0, 0);
        ST(arc->ARCW0,   BASEIDX_ARCW0, 0);         /* fill the base address after all arcs are read */

        arc->memVID = VID;

        ST(arc->ARCW1, CONSUMFMT_ARCW1, ConsFMT);
        ST(arc->ARCW1,   MPFLUSH_ARCW1, FLUSH);
        ST(arc->ARCW1, DEBUG_REG_ARCW1, REG);
        ST(arc->ARCW1, BUFF_SIZE_ARCW1, minFrameSize);

        ST(arc->ARCW2, COMPUTCMD_ARCW2, CMD);
        ST(arc->ARCW2, FLOWERROR_ARCW2, OVF);
        ST(arc->ARCW2,    EXTEND_ARCW2, EXTD);
        ST(arc->ARCW2,      READ_ARCW2, 0);

        ST(arc->ARCW3, COLLISION_ARCW3, 0);
        ST(arc->ARCW3, ALIGNBLCK_ARCW3, 0);
        ST(arc->ARCW3,     WRITE_ARCW3, 0);

        if (FoundEndSection || (0 == strncmp (*pt_line,GRAPH_END,strlen(GRAPH_END))))
        {   break;
        }          
    } /* while 1 on ARCS */
}
/*
    ==================================   
    read the formats 
    merge with existing ones  in arcFormat[]
    prepare a table (arcFormatIndexConversion[]) for renaming the current graph format indexes to the final merge one
*/
void stream_tool_read_format(char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{
    uint32_t frameSize, raw, nchan, timestp, intlv;
    uint32_t formatID, short_syntax, extended_syntax;
    char shortFormat[2], *pt_line2;
    uint32_t timeFormat;
    uint32_t domain;
    uint32_t physicalType;
    uint32_t sequentialIndex;
    float X1, X2, X3;
    struct formatStruct *format;
    float FS;
    union 
    {   uint32_t u32;
        float f32;
    } U;

    jump2next_valid_line(pt_line);         // skip "format"
    graph->nb_formats = 0;
    sequentialIndex = 0;

    while (1)
    {   
      /*;----------------------------------------------------------------------------------------
        ; FORMATS
        ;----------------------------------------------------------------------------------------
        ;   the data stream format used in the (sub)graph :
        ;   "S" simple format = 
        ;       ID  : index later used in the graph
        ;       F   : frame size in Bytes for one deinterleaved channel, the arc buffer size is at least = framesize x nb channel 
        ;       R   : raw arithmetic data format (see "stream_raw_data")
        ;       N   : number of channels in the stream
        ;       FS  : sampling rate in Hz, null value means "asynchronous" or "any sampling rate"
        ;       I   : 0 means interleaved data, 1 means deinterleaved data by packets of "frame size"
        ;   "X" eXtended format = 
        ;       simple  format + ..
        ;       TS  : time-stamp format 0:none, 1:absolute time-stamp, 2:relative time, 3:simple counter
        ;       TF  : time format  0:16bits 1:32bits 2:64bits  (see "STREAM_TIME16D" for example)
        ;       DO  : IO_DOMAIN defined in the platform IO manifest (0 means "any")
        ;       PT  : physical data type (for example see stream_unit_physical used for analof sensors)
        ;       X   : several fields provisioned depending on the domain

        format
        ;     I F   R N    FS I 
            S 0 8  17 1     0 0    
            S 1 8  17 1 16000 0     
            S 2 12 17 1 16000 0    

        ;     I F   R N    FS I  TS TF DO PT   X
            X 3 8  17 1     0 0   0  0  0  0 
        _end_
      */

        pt_line2 = *pt_line;
        if (fields_extract(pt_line, "CIIIIFI", shortFormat,  
            &formatID, 
            &frameSize, 
            &raw, 
            &nchan, 
            &FS, 
            &intlv
            ) < 0) exit(-4);

        //graph->arcFormatIndexConversion[sequentialIndex++] = formatID;
        graph->arcFormatIndexConversion[formatID] = formatID;


        format = &(graph->arcFormat[formatID]);
        format->FMT0 = 0;
        ST(format->FMT0, RAW_FMT0, raw);
        ST(format->FMT0, INTERLEAV_FMT0, intlv);
        ST(format->FMT0, FRAMESIZE_FMT0, frameSize);

        format->FMT1 = 0;
        ST(format->FMT1, NCHANM1_FMT1, nchan -1);  

        U.f32 = FS;
        format->FMT2 = U.u32;   /* FMT2 LSBs will be erased depending on the domain */


        extended_syntax = shortFormat[0] != 'S' && shortFormat[0] != 's';
        short_syntax = shortFormat[0] == 'S' || shortFormat[0] == 's';
        if (extended_syntax)   /* is it a long format ? */
        {   *pt_line = pt_line2;
        if (fields_extract(pt_line, "CIIIIFIIIIIFFF", shortFormat,  
            &formatID, 
            &frameSize, 
            &raw, 
            &nchan, 
            &FS, 
            &intlv,

            &timestp,
            &timeFormat,
            &domain,
            &physicalType,
            &X1, &X2, &X3
            ) < 0) exit(-4);

        ST(format->FMT1, TIMSTAMP_FMT1, timestp);  
        ST(format->FMT1, TSTPSIZE_FMT1, timeFormat);  
        ST(format->FMT1, DOMAIN_FMT1, domain);  
        ST(format->FMT1, PHYTYPE_FMT1, physicalType);  

        U.f32 = X1;
        format->FMT3 = U.u32;  /* TBD */
        }

        graph->nb_formats ++;

        if (FoundEndSection)
            break;
    }

    /* TBD     read the formats 
        merge with existing ones  in arcFormat[]
        prepare a table (arcFormatIndexConversion[]) for renaming the current graph format indexes to the final merge one 
    */
}
/*
==================================   
*/
void stream_tool_read_parameters(char **pt_line, struct stream_node_manifest *node)
{
    uint8_t raw_type;
    uint32_t nb_raw, nbytes, nbits;
    uint8_t *ptr_param, *pt0, preset, tag;

    pt0 = ptr_param = (uint8_t *)(node->PackedParameters);
    while (1)
    {
        read_binary_param(pt_line, ptr_param, &raw_type, &nb_raw);
        if (nb_raw == 0)
            break;
        nbits = stream_bitsize_of_raw(raw_type);
        nbytes = (nbits * nb_raw)/8;
        ptr_param = &(ptr_param[nbytes]);
    }

    preset = *pt0++;        /* shift the graph parameters by 2 bytes */
    tag = *pt0--;
    nbytes = (int)(ptr_param - pt0); 
    nbytes = nbytes +2 /* for the 2Bytes shift */ +3 /* for w32 rounding) */;
    node->defaultParameterSizeW32 = nbytes /4;
    memcpy ((uint8_t *)(&(node->PackedParameters[1])), &(pt0[2]), 4*(node->defaultParameterSizeW32));
    node->PackedParameters[0] = 0;
    ST(node->PackedParameters[0], W32LENGTH_LW3, node->defaultParameterSizeW32);
    ST(node->PackedParameters[0], PRESET_LW3, preset);
    ST(node->PackedParameters[0], PARAM_TAG_LW3, tag);
}


/* update struct node_memory_bank without graph_basePACK = f(arc format) */
void compute_memreq(struct stream_node_manifest *node, struct formatStruct *all_format)
{
    uint32_t imem;
    uint64_t size, workingmem, tmp;
    float FS;

    struct node_memory_bank *m;
    union 
    {   uint32_t u32;
        float f32;
    } U;


    workingmem = 0;

    for (imem = 0; imem < node->nbMemorySegment; imem++)
    {
        /* struct graph stream_node_manifest all_nodes : compute the memreq from arc data 
            update graph->all_nodes . memreq . graph_memreq_size
            Memory Size = A + B x nb_channels_arc(i) + C x sampling_arc(j) + D x frame_size_arc(k) 
        */
        m = &(node->memreq[imem]);

        U.u32 = all_format[m->iarcSamplingJ].FMT2 & 0xFFFFFF00;     /* TBD: mask depends on the domain */
        FS = U.f32;

        size = m->size0;
            tmp = (int)(0.5 + m->sizeFrame * FS);
        size = size + tmp;
            tmp = RD(all_format[m->iarcFrameK].FMT0, FRAMESIZE_FMT0);
            tmp = (int)(0.5 + m->sizeParameter * tmp);
        size = size + tmp;

        size = ((size+3)>>2)<<2;
        m->graph_memreq_size = size;
    }
}
 
 
/**==============================================================
    Fill the arc             fill_arc(arc, pack27b, size,  dbgcmd, dbgreg, ovf, "");
*/
void fill_arc(struct arcStruct *arc, uint32_t pack27b, uint32_t size, 
            uint8_t ConsFMT, uint8_t FLUSH, uint8_t EXTD, 
            uint8_t CMD, uint8_t REG, uint8_t OVF,
            char *comments)
{
    //char STR[NBCHAR_LINE];

    arc->ARCW0 = 0; arc->ARCW1 = 0; arc->ARCW2 = 0; arc->ARCW3 = 0;

    ST(arc->ARCW0, PRODUCFMT_ARCW0, 0);
    ST(arc->ARCW0,   DATAOFF_ARCW0, 0);
    ST(arc->ARCW0,   BASEIDX_ARCW0, pack27b);    

    ST(arc->ARCW1, CONSUMFMT_ARCW1, ConsFMT);
    ST(arc->ARCW1,   MPFLUSH_ARCW1, FLUSH);
    ST(arc->ARCW1, DEBUG_REG_ARCW1, REG);
    ST(arc->ARCW1, BUFF_SIZE_ARCW1, size);

    ST(arc->ARCW2, COMPUTCMD_ARCW2, CMD);
    ST(arc->ARCW2, FLOWERROR_ARCW2, OVF);
    ST(arc->ARCW2,    EXTEND_ARCW2, EXTD);
    ST(arc->ARCW2,      READ_ARCW2, 0);

    ST(arc->ARCW3, COLLISION_ARCW3, 0);
    ST(arc->ARCW3, ALIGNBLCK_ARCW3, 0);
    ST(arc->ARCW3,     WRITE_ARCW3, 0);

    //strcpy (STR, comments);
    //sprintf(DBG,"Script stack and registers in ARC%d base address=0x%X  size=0x%X",graph->nb_arcs, arcBufferBaseW32, arcBufferSizeByte);  
    //DBGG(arcBufferBaseW32 *4, DBG); // check on Byte address
    //ST(arc->ARCW1, BUFF_SIZE_ARCW1, arcBufferSizeByte);
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

    RamW32 = 0; 

    /* ======= memory for FIFO descriptors ======= */
    if (graph->nb_debug_pages > 0)                      /* are there debug data to save ? */
    {                                                   /* arc0 is used to address the 16x64b debug registers pages */
        size = 4 *(ARC_DBG_REGISTER_SIZE_W32 * graph->nb_debug_registers);
        vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, &pack27b, &ptr8, platform, graph);
    }
    RamW32 += (size +3)>>2;

    N = graph->nb_scripts + graph->nb_arcs;
    size = SIZEOF_ARCDESC_W32 * N;                      /* reserve the descriptor area (all the arcs) */
    vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, &pack27b, &ptr8, platform, graph);
    RamW32 += (size +3)>>2;

    /* ======= memory for FIFO buffers ======= */


    /* ======= memory for scripts ======= */
    N = graph->max_shared_stackW32 + graph->max_nregs;
    size = ((SCRIPT_REGSIZE +1) * N);                /* reserve the shared area of scripts */
    vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, &pack27b, &ptr8, platform, graph);


    iAllArcs = 0;
    first_script_sharedRAM = 1;
    for (iscripts = 0; iscripts < graph->nb_scripts; iscripts++)
    {   uint32_t RegAndStackW32, stackdepthW32;
        dbgcmd = dbgreg = ovf = unf = 0;

        if (0 == graph->scriptRAMshared[iscripts] || first_script_sharedRAM)
        {   if (first_script_sharedRAM && (0 != graph->scriptRAMshared[iscripts]))
            {   first_script_sharedRAM = 0;
                sharedArc = graph->nb_arcs;
            }

            RegAndStackW32 = graph->script_nregs[iscripts];
            stackdepthW32 = graph->script_stackdepthW32[iscripts];
            arcBufferSizeByte = RegAndStackW32 * (SCRIPT_REGSIZE +1);   /* register type = 1 byte*/
            arcBufferSizeByte+= stackdepthW32 * (SCRIPT_REGSIZE +1);    /* stack element on 9 bytes too */
            size = ((arcBufferSizeByte +3)>>2)<<2;         /* round to the closest W32 */
            vid_malloc(VID_default, size, MEM_REQ_4BYTES_ALIGNMENT, &pack27b, &ptr8, platform, graph);

            arc = &(graph->arc[iAllArcs]);
            fill_arc(arc, pack27b, (uint32_t)size,   0, 0, 0,  dbgcmd, dbgreg, ovf, "");
            iAllArcs ++;
            /* second sharedRAM script : reuse the RAM of the first shared script */
        }
    }

    /* ======= memory for node instances ======= */
    for (inode = 0; inode < graph->nb_arcs; inode++)
    {   
        node = &(graph->all_nodes[inode]);
        for (imemreq = 0; imemreq < node->nbMemorySegment; imemreq++)
        {
            compute_memreq(node, graph->arcFormat);

            size = node->memreq[imemreq].graph_memreq_size;
            vid_malloc(node->memreq[imemreq].VID, size, MEM_REQ_4BYTES_ALIGNMENT, &(node->memreq[imemreq].graph_basePACK), &ptr8, platform, graph);
        }
    }

}



void stream_tool_read_subgraph (char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{   
}

/**=================================================================================================
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par              read the graph 

  @remark
 */

void arm_stream_read_graph (struct stream_platform_manifest *platform, 
                            struct stream_graph_linkedlist *graph, 
                            char *ggraph_txt)
{
    char *pt_line;
    char paths[MAX_NB_PATH][NBCHAR_LINE];
    uint32_t scriptctrl, procs /* @@@@ */;
    uint32_t first_arc = 1;

    pt_line = ggraph_txt;
    graph->cumulFlashW32 = GRAPH_HEADER_NBWORDS; 
    dbggraph = graph;
    jump2next_valid_line(&pt_line);
    globalEndFile = 0;
    
    while ((globalEndFile != FOUND_END_OF_FILE))
    {
        if (0 == strncmp (pt_line,GRAPH_END,strlen(GRAPH_END)))
        {   break;
        }

        /* -------------------------- HEADER -------------------------------------- */
        if (0 == strncmp (pt_line, SUBGRAPHSTART, strlen(SUBGRAPHSTART)))
        {   jump2next_valid_line(&pt_line);
            continue;
        }

        /* -------------------------- HEADER -------------------------------------- */
        if (0 == strncmp (pt_line, HEADER, strlen(HEADER)))
        {   jump2next_valid_line(&pt_line);         // skip top_header
            fields_extract(&pt_line, "III", 
                &(graph->PackedFormat), &scriptctrl, &procs);

            graph->offsetToRam = 0;
            continue;
        }

        /* -------------------------- PATHS --------------------------------------- */
        if (0 == strncmp (pt_line, PATHS, strlen(PATHS)))
        {   int i = 0;

            jump2next_valid_line(&pt_line);         // skip "paths"

            while (1)
            {   fields_extract(&pt_line, "C", paths[i++]);  /* read the paths until finding _end_ / SECTION_END */
                if (FoundEndSection)
                {   break;
                }
            }
            continue;
        }

        /* ----------------------------STREAM_FORMAT------------------------------- */
        if (0 == strncmp (pt_line, FORMAT, strlen(FORMAT)))
        {   
            stream_tool_read_format(&pt_line, platform, graph);

            graph->cumulFlashW32 += (graph->nb_formats * STREAM_FORMAT_SIZE_W32);

            if (0 == strncmp (pt_line,SECTION_END,strlen(SECTION_END)))
                jump2next_valid_line(&pt_line);     /* remove trailing _end_ (s)  */
            continue;
        }

        /* ----------------------------top_graph_interface------------------------- 
        ;   - index
        ;   - stream format 
        ;   - ID of the interface given in "files_manifests_computer"
        ;   - common setting (8bits intlv/nchan/frame/FS = ARC0 producer format + mixed-signal settings 24bits)
        ;   - VID of buffer : memory bank index from platform_manifest_computer.txt (special DMA, speed)
        */
        if (0 == strncmp (pt_line, IO_INTERFACE, strlen(IO_INTERFACE)))
        {   jump2next_valid_line(&pt_line);         // skip "top_graph_interface"
            read_top_graph_interface (&pt_line, platform, graph);

            /* all the possible IO are in the binary graph */
            graph->cumulFlashW32 += (platform->nb_hwio_stream * STREAM_IOFMT_SIZE_W32);

            continue;
        }


        /* ----------------------------   SCRIPTS  ------------------------------ */
        if (0 == strncmp (pt_line, SCRIPT, strlen(SCRIPT)))
        {  
            stream_tool_script(&pt_line, platform, graph);
            continue;
        }
 
        /* ----------------------------   NODES  ------------------------------ */

        if (0 == strncmp (pt_line, NODE, strlen(NODE)))
        {   stream_tool_read_node(&pt_line, platform, graph);
            continue;
        }
 
        /* ----------------------------   ARCS  ------------------------------ */

        if (0 == strncmp (pt_line, ARC, strlen(ARC)))
        {   stream_tool_read_arcs (&pt_line, platform, graph);

            if (0 == strncmp (pt_line,GRAPH_END,strlen(GRAPH_END)))
            {   break;
            }    
        } 

        /* ----------------------------   SUBGRAPHS  ------------------------------ */
        /*
        ==================================   
            read the subgraph
            create the mangling string 

            ; Subgraph #1
            ;   path and filename
            ;   list of indexes to the top_graph_interface (or to the indexes if we are already in a subgraph)
            ;   list of VIDs used in the subgraph
            ;
            subgraph 
                sub1                    ; subgraph name, used for name mangling    
                include    
                    3   sub_graph_0.txt ; path and file name
                    0   1   2   3   4   ; 5 streaming interfaces data_in_0, data_out_0 .. 
                    0   0   0           ; 3 partitions assigned to VID0 : fast-working slow-working slow-static
                _end_
        */
        if (0 == strncmp (pt_line, SUBGRAPH, strlen(SUBGRAPH)))
        {   
            char *new_ggraph, file_name[NBCHAR_NAME], file_name2[NBCHAR_NAME], *subName;
            uint32_t ipath, i;

            new_ggraph = calloc (MAXINPUT, 1);

            jump2next_valid_line(&pt_line);
            subName = &(graph->mangling[graph->subg_depth][0]);
            sscanf (pt_line, "%s", subName); 
            jump2next_valid_line(&pt_line);

            graph->subg_depth++;

            /* create the first characters of names of this subgraph */
            strcpy(graph->toConcatenate, "");
            for (i = 0; i < graph->subg_depth; i++)
            {   strcat (graph->toConcatenate, graph->mangling[i]);
            }


            /* read subgraph as a recursion */ 
            sscanf (pt_line, "%d %s", &ipath, file_name2); 
            jump2next_valid_line(&pt_line);
            strcpy(file_name, paths[ipath]);
            strcat(file_name, file_name2);
            read_input_file (file_name, new_ggraph);
            arm_stream_read_graph (platform, graph, new_ggraph);

            /* restore the situation : go on step above in the recursion */
            strcpy(&(graph->mangling[graph->subg_depth - 1][0]), "");

            strcpy(graph->toConcatenate, "");
            for (i = 0; i < graph->subg_depth; i++)
            {   strcat (graph->toConcatenate, graph->mangling[i]);
            }

            graph->subg_depth--;

            free(new_ggraph);
            continue;
        }
 
    }
}

#ifdef __cplusplus
}
#endif