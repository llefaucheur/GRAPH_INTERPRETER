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

char current_line[NBCHAR_LINE];

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

}


/* ====================================================================================   
    Read and pack the script until finding "_end_" / SECTION_END

    script_byte_codes
    ....
    _end_               end of byte codes  
*/
void stream_tool_read_script(char **pt_line, struct stream_script *script)
{
    uint8_t raw_type;
    uint32_t nb_raw, nbytes, nbits;

    while (1)
    {
        read_binary_param(pt_line, &(script->script_bytecode[script->script_nb_byte_code]), &raw_type, &nb_raw);
        if (nb_raw == 0)
            break;
        nbits = stream_bitsize_of_raw(raw_type);
        nbytes = (nbits * nb_raw)/8;
        script->script_nb_byte_code += nbytes;
    }

    script->script_nb_byte_code = (3+(script->script_nb_byte_code)) & 0xFFFFFFFC;   // round it to W32
}


/*
====================================================================================   
*/
//void stream_tool_script(char **pt_line, struct stream_script *script, 
//        struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
//{
//    char node_name[MAXINPUT];
//    struct stream_node_manifest *node, *platform_node;
//    char shortFormat[2];
//    uint32_t nbByteCodes=0, nbBytesCodes, privateRAM, iscripts;
//    uint32_t scriptFormat;  /* select byte-code format / binary code of the processor */
//  
//    //FILE *ptf_graph_script_bytecode;
//    //char filename[120];
//
//    jump2next_valid_line(pt_line);  // skip "script" 
//
//    node = &(graph->all_nodes[graph->nb_nodes]); 
//    platform_node = 0;
//    if (fields_extract(pt_line, "ci", node_name, &(node->instance_idx)) < 0) exit (-4);  
//    node_name[MAXINPUT-1] = '\0';
//
//    
//
//  
//    iscripts = node->instance_idx;     // @@@ warning this index must fit with the SWC declaration &(node->local_script_index)
//    /*fields_extract(pt_line, "ciiii", &shortFormat, &nreg, &stackdepthW32, &privateRAM, &scriptFormat);  */
//  
//    //sprintf (filename, "SCRIPT_%d.txt", iscripts); 
//    //ptf_graph_script_bytecode = fopen(filename, "wt");
//  
//    if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
//    {   privateRAM = 0;
//        scriptFormat = 0;
//    }
//  
//    // current BYTECODE offset in script_bytecode[]
//    script->script_offset[INDEX_SCRIPT_STRUCT_SIZE*iscripts+INDEX_SCRIPT_OFFSET] = graph->nb_byte_code; 
//    //graph->script_nregs[iscripts] = nreg;
//    //graph->script_stackdepthW32[iscripts] = stackdepthW32;
//    script->nb_scriptsARC ++;                                // one arc descriptor per script
//  
//    //if (privateRAM == 0)                                    // the script is using the shared memory section
//    //{   graph->max_nregs = MAX(graph->max_nregs, nreg); 
//    //    graph->max_shared_stackW32 = MAX(graph->max_shared_stackW32, stackdepthW32); 
//    //    graph->scriptRAMshared[iscripts] = 1;
//    //    graph->atleastOneScriptShared = 1;
//    //} 
//    
//    stream_tool_read_script(pt_line, &(graph->script_bytecode[graph->nb_byte_code]));
//    nbBytesCodes = (3+nbBytesCodes) & 0xFFFFFFFC;   // round it to W32
//    
//    script->nb_byte_code += nbBytesCodes;
//    script->nb_scripts ++;
//  
//    /* extra parameters in FLASH following the code */
//    //if (0 == strncmp (*pt_line, PARAMETER_START, strlen(PARAMETER_START)))
//    //{   
//    //    stream_tool_read_script(pt_line, &(graph->script_bytecode[graph->nb_byte_code]), &nbBytesCodes);
//    //    nbBytesCodes = (3+nbBytesCodes) & 0xFFFFFFFC;   // round it to W32
//    //    graph->nb_byte_code += nbBytesCodes;
//    //}
//}




///*
//====================================================================================   
//*/
//void stream_tool_read_node(char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
//{
//    char *pt_line2;
//    char node_name[MAXINPUT];
//    struct stream_node_manifest *node, *platform_node;
//    uint32_t inode, tmp;
//    char *pt_line_saved;
//    char shortFormat[2];
//
//    jump2next_valid_line(pt_line);  // skip "node" 
//
//    node = &(graph->all_nodes[graph->nb_nodes]); 
//    platform_node = 0;
//    if (fields_extract(pt_line, "ci", node_name, &(node->instance_idx)) < 0) exit (-4);  
//    node_name[MAXINPUT-1] = '\0';
//
//    //if (0 != strcmp(node_name, "arm_stream_script"))        /* is it NOT a script ? */
//    for (inode = 1; inode < platform->nb_nodes; inode++)
//    {   platform_node = &(platform->all_nodes[inode]);
//        if (0 == strncmp(node_name, platform_node->nodeName,strlen(platform_node->nodeName)))
//        {   break;
//        }
//    }
//
//    /* COPY DEFAULT DATA FROM MANIFEST in graph->all_node[] */
//    *node = platform->all_nodes[inode];
//    node->swc_idx = inode;
//
//    pt_line2 = *pt_line;
//    if (fields_extract(pt_line, "CI", shortFormat, &(node->preset)) < 0) 
//        exit(-4);
//
//    if (shortFormat[0] != 'S' && shortFormat[0] != 's')   /* is it a long format ? */
//    {   *pt_line = pt_line2;
//        if (fields_extract(pt_line, "CIIIIII", shortFormat,
//            &(node->preset),
//            &(node->local_script_index),
//            &(node->swc_assigned_arch),
//            &(node->swc_assigned_proc),
//            &(node->swc_assigned_priority),
//            &(node->swc_verbose)) <0) exit(-4);
//    }
//
//    if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
//    {   node->local_script_index = 0;
//        node->swc_assigned_arch = 0;
//        node->swc_assigned_proc = 0;
//        node->swc_assigned_priority = 0;
//        node->swc_verbose = 0;
//    }
//    else /* read the VID of the node memory segments, finishing with the one for code */
//    {   int Pseg;
//        switch (node->nbMemorySegment)
//        {   case 4: fields_extract(pt_line, "IIIII", &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID), &(node->memreq[2].VID), &(node->memreq[0].VID)); break;
//            case 3: fields_extract(pt_line, "IIII",  &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID), &(node->memreq[2].VID)); break;
//            case 2: fields_extract(pt_line, "III",   &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID)); break;
//            case 1: fields_extract(pt_line, "II",    &Pseg, &(node->memreq[0].VID)); break;
//            default: case 0: break;
//        }
//    }
//  
//     ST(node->headerPACK, SWC_IDX_LW0, node->swc_idx);
//     ST(node->headerPACK, ARCSRDY_LW0, platform_node->nbInputOutputArc);
//     ST(node->headerPACK, NBARCW_LW0, platform_node->nbParameArc + platform_node->nbInputOutputArc);
//     ST(node->headerPACK, PRIORITY_LW0, node->swc_assigned_priority);
//     ST(node->headerPACK, ARCHID_LW0, node->swc_assigned_arch);
//     ST(node->headerPACK, PROCID_LW0, node->swc_assigned_proc);
//     ST(node->headerPACK, SCRIPT_LW0, node->local_script_index);
//  
//     pt_line_saved = *pt_line;
//  
//     if (0 == strncmp (*pt_line, PARAMETER_START, strlen(PARAMETER_START)))
//     {  
//         jump2next_valid_line(pt_line);
//         stream_tool_read_parameters(pt_line, graph, node); /*  returns on "_end_" SECTION_END */
//     }
//     else
//     {   node->PackedParameters[0] = 0;
//         node->ParameterSizeW32 = 0;
//         ST(node->PackedParameters[0], W32LENGTH_LW3, 1);
//         ST(node->PackedParameters[0], PRESET_LW3, node->preset);
//         *pt_line = pt_line_saved;
//     }
//  
//     ST(node->PackedParameters[0], TRACEID_LW3, node->swc_verbose);
//  
//     /* keep the parameter header for the PRESET set at reset */
//     if (node->ParameterSizeW32 < 1) node->ParameterSizeW32 =1;
//  
//     graph->cumulFlashW32 += 1;                                /* SWC header */
//     tmp = node->nbInputArc + node->nbOutputArc + node->nbParameArc;
//     graph->cumulFlashW32 += (tmp+1)/2;                        /* SWC arcs */
//     graph->cumulFlashW32 += 2*(node->nbMemorySegment);        /* two words per memory bank */
//     graph->cumulFlashW32 += node->ParameterSizeW32;    /* parameters */
//  
//     graph->nb_nodes++;
//}



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
//void stream_tool_read_arcs (char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
//{   uint32_t arcID, ProdFMT, ConsFMT, VID,   OVF, CMD, REG, FLUSH, EXTD;
//    uint32_t inode, instSrc, top_graph_idx, set0_copy1, iarcSrc, instDst, iarcDst, node_inst, minFrameSize;
//    float IOSizeMulfac;
//    struct stream_node_manifest *node, *platform_node;
//    char nodeNameSrc[120], nodeNameDst[120], shortFormat[2], *node_name;
//    struct formatStruct *format;
//    uint32_t tmp, arcBufferSizeByte;
//    struct arcStruct *arc;
//    struct arcStruct *arcIO;
//    uint32_t fw_io_idx, PAGE;
//    char *pt_line2;
//
//    jump2next_valid_line(pt_line);
//
//    node = &(graph->all_nodes[0]);  // init
//    arcID = 0;
//
//    while (1)
//    {
//        arc = &(graph->arc[arcID]); 
//        arc->arcID = arcID;
//
//        pt_line2 = *pt_line;
//        if (fields_extract(pt_line, "cii",  shortFormat, &ProdFMT, &ConsFMT) < 0) 
//            break;
//
//        if (shortFormat[0] != 'S' && shortFormat[0] != 's')   /* is it a long format ? */
//        {   *pt_line = pt_line2;
//            if (fields_extract(pt_line, "ciiiiiiiiif",  shortFormat,
//            &ProdFMT, &ConsFMT, &OVF, &CMD, &REG, &PAGE, &FLUSH, &EXTD, &VID, &IOSizeMulfac
//            ) < 0) exit (-5);
//        }
//    
//        if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
//        {   OVF = 0; CMD = 0; REG = 0; PAGE = 0;
//            FLUSH = 0; EXTD = 0; VID = 0; IOSizeMulfac = 1.0;
//        }
//        graph->nb_arcs++;
//
//        fields_extract(pt_line, "cii", nodeNameSrc, &instSrc, &iarcSrc);  
//        if (0 == strncmp(nodeNameSrc, "arm_stream_script",NBCHAR_LINE))
//        {
//        }
//        else
//        {
//            fields_extract(pt_line, "cii", nodeNameDst, &instDst, &iarcDst);  
//        }
//
//
//        //minFrameSize=0;
//     
//        if (0 == strncmp(nodeNameSrc, "graph_interface",NBCHAR_LINE))
//        {   
//            top_graph_idx = instSrc;
//            set0_copy1 = iarcSrc;
//            for (fw_io_idx=0; fw_io_idx<platform->nb_hwio_stream; fw_io_idx++)
//            {   if (top_graph_idx == graph->arcIO[fw_io_idx].top_graph_index)
//                    break;
//            }
//            arcIO = &(graph->arcIO[fw_io_idx]);
//            arcIO->arcIDstream = arcID;             /* arcID associated to this IO */
//            arcIO->si.set0_copy1 = set0_copy1;
//        }
//        else
//        {   fw_io_idx = 0;  /* this is not an IO arc */
//            /* search SRC node , assign the arcID*/
//            node_name = nodeNameSrc;
//            node_inst = instSrc;
//            for (inode = 0; inode < platform->nb_nodes/* io node */; inode++)
//            {   
//                node = &(graph->all_nodes[inode]);  // scan the node of the graph, not the platform manifest
//                                                    // node->idx point to the node in flash
//                platform_node = &(platform->all_nodes[node->swc_idx]); // platform manifest, to read the name
//
//                if (0 == strncmp(node_name, platform_node->nodeName,NBCHAR_LINE))
//                {   node = &(graph->all_nodes[inode]);  // inode=0 will have dummy arcs
//                    if (node_inst == node->instance_idx)    // is this the good instance of the source node ?
//                    {   
//                        node->arc[iarcSrc].arcID = arcID;   // save the arc src index to the node structure
//                        break;
//                    }
//                }
//            } 
//        }
//        format = &(graph->arcFormat[ProdFMT]);
//        minFrameSize = RD(format->FMT0, FRAMESIZE_FMT0);
//
//        /* ------------------------------------------------------- */
//
//        if (0 == strncmp(nodeNameDst, "graph_interface", NBCHAR_LINE))
//        {   uint32_t fw_io_idx;
//            top_graph_idx = instDst;
//            set0_copy1 = iarcSrc;
//            for (fw_io_idx=0; fw_io_idx<platform->nb_hwio_stream; fw_io_idx++)
//            {   if (top_graph_idx == graph->arcIO[fw_io_idx].top_graph_index)
//                    break;
//            }
//            arcIO = &(graph->arcIO[fw_io_idx]);
//            arcIO->arcIDstream = arcID;             /* arcID associated to this IO */
//            arcIO->si.set0_copy1 = set0_copy1;
//
//        }
//        else
//        {            /* search DST node , assign the arcID*/
//            node_name = nodeNameDst;
//            node_inst = instDst;
//            for (inode = 0; inode < platform->nb_nodes /* io node */; inode++)
//            {   
//                node = &(graph->all_nodes[inode]);  // scan the node of the graph, not the platform manifest
//                                                    // node->idx point to the node in flash
//                platform_node = &(platform->all_nodes[node->swc_idx]); // platform manifest, to read the name
//        
//                if (0 == strncmp(node_name, platform_node->nodeName,NBCHAR_LINE))
//                {   node = &(graph->all_nodes[inode]);
//
//                    if (node_inst == node->instance_idx)
//                    {   
//                        node->arc[iarcDst].arcID = arcID;  // save the arc src index to the node structure 
//                        break;
//                    }
//                } 
//            }             
//        }
//
//        node->arc[iarcSrc].debug_page_DBGB0_LW1 = PAGE; // DBGB0_LW1
//        node->arc[iarcDst].debug_page_DBGB0_LW1 = PAGE; // must be the page of the producer node
//
//
//        {
//            extern uint32_t gcd(uint32_t a, uint32_t b);
//            extern uint32_t lcm(uint32_t a, uint32_t b);
//            uint32_t a, b, minprodcons, maxprodcons, ratio;
//
//            format = &(graph->arcFormat[ConsFMT]);
//            tmp = RD(format->FMT0, FRAMESIZE_FMT0);
//            a = minFrameSize;
//            b = RD(format->FMT0, FRAMESIZE_FMT0);
//            //minFrameSize = lcm(a,b);    // take LCM of consumer,producer frame sizes
//
//            if (a == b)
//            {   minFrameSize = a;
//            }
//            else
//            {   minprodcons = MAX(1, MIN(a,b));
//                maxprodcons = MAX(a,b);
//                ratio = maxprodcons / minprodcons;
//                minFrameSize = minprodcons * (ratio + 1);
//            }
//        }
//
//
//
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
//
//        if (FoundEndSection || (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END))))
//        {   break;
//        }          
//    } /* while 1 on ARCS */
//}



//void stream_tool_read_format_extension (char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
//{
//    uint32_t frameSize, raw, nchan, timestp, intlv;
//    uint32_t formatID, short_syntax, extended_syntax;
//    char shortFormat[2], *pt_line2;
//    char ctmp[NBCHAR_LINE];
//    uint32_t timeStamp, timeFormat;
//    uint32_t sequentialIndex;
//    float X1, X2, X3;
//    struct formatStruct *format;
//    float FS;
//    union 
//    {   uint32_t u32;
//        float f32;
//    } U;
//    uint32_t domain, subdomain;
//
//    #define MAXNB_DOMAINEXTENSIONS 5
//    float X[MAXNB_DOMAINEXTENSIONS];
//
//    //                          ID    DOMAIN     SDOMAIN  EXTENSIONS DEPENDS OF THE DOMAIN
//    //stream_format_extension   2     audio_in   VRMS     16000.0  1  ; audio_in domain extension = sampling rate + channel mapping     
//    fields_extract(&pt_line, "CIIIIIII", ctmp, &domain, &subdomain, &X[0], &X[1], &X[2], &X[3], &X[4]); 
//}


/*
==================================   
*/
void stream_tool_read_parameters(
        char **pt_line, 
        struct stream_platform_manifest *platform, 
        struct stream_graph_linkedlist *graph, 
        struct stream_node_manifest *node)
{
    uint8_t raw_type;
    uint32_t nb_raw, nbytes, nbits;
    uint8_t *ptr_param, *pt0, preset, tag;

    //node->ParameterSizeW32 = 1;
    //ST(node->PackedParameters[0], W32LENGTH_LW3, node->ParameterSizeW32);
            
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
    node->ParameterSizeW32 = nbytes /4;
    memcpy ((uint8_t *)(&(node->PackedParameters[1])), &(pt0[2]), 4*(node->ParameterSizeW32));
    node->PackedParameters[0] = 0;
    ST(node->PackedParameters[0], W32LENGTH_LW3, node->ParameterSizeW32);
    ST(node->PackedParameters[0], PRESET_LW3, preset);
    ST(node->PackedParameters[0], PARAM_TAG_LW3, tag);

    //vid_malloc_booking (graph->memVID, 4*(intPtr_t)node->ParameterSizeW32, MEM_REQ_4BYTES_ALIGNMENT, MEM_TYPE_STATIC, platform, graph);

    while (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
    {   jump2next_valid_line(pt_line);
    }
}


/* update struct node_memory_bank without graph_basePACK = f(arc format) */
void compute_memreq(struct stream_node_manifest *node, struct formatStruct *all_format)
{
    uint32_t imem;
    uint64_t size, workingmem;
    float FS;

    struct node_memory_bank *m;
    //union 
    //{   uint32_t u32;
    //    float f32;
    //} U;


    workingmem = 0;

    for (imem = 0; imem < node->nbMemorySegment; imem++)
    {
        /* struct graph stream_node_manifest all_nodes : compute the memreq from arc data 
            update graph->all_nodes . memreq . graph_memreq_size
            Memory Size = A + B x nb_channels_arc(i) + C x sampling_arc(j) + D x frame_size_arc(k) 
        */
        m = &(node->memreq[imem]);

        //U.u32 = all_format[m->iarcSamplingJ].FMT2 & 0xFFFFFF00;     /* TBD: mask depends on the domain */
        //FS = U.f32;
        FS = 0;

        size = m->size0;
        //    tmp = (int)(0.5 + m->sizeFrame * FS);
        //size = size + tmp;
        //    tmp = RD(all_format[m->iarcFrameK].FMT0, FRAMESIZE_FMT0);
        //    tmp = (int)(0.5 + m->sizeParameter * tmp);
        //size = size + tmp;

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




void stream_tool_read_subgraph (char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{   
    char *new_ggraph, file_name[NBCHAR_NAME], file_name2[NBCHAR_NAME], *subName, paths[MAX_NB_PATH][NBCHAR_LINE];
    uint32_t ipath, i;
    extern void arm_stream_read_graph (struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph, char *ggraph_txt);
    
    new_ggraph = calloc (MAXINPUT, 1);
    
    jump2next_valid_line(pt_line);
    subName = &(graph->mangling[graph->subg_depth][0]);
    sscanf (*pt_line, "%s", subName); jump2next_valid_line(pt_line);
    
    graph->subg_depth++;
    
    /* create the first characters of names of this subgraph */
    strcpy(graph->toConcatenate, "");
    for (i = 0; i < graph->subg_depth; i++)
    {   strcat (graph->toConcatenate, graph->mangling[i]);
        strcat (graph->toConcatenate, "_");
    }
    
    /* read subgraph as a recursion */ 
    sscanf (*pt_line, "%d %s", &ipath, file_name2); jump2next_valid_line(pt_line);
    strcpy(file_name, paths[ipath]); strcat(file_name, file_name2);
    read_input_file (file_name, new_ggraph);

    /* recursion starts here */
    arm_stream_read_graph (platform, graph, new_ggraph);
    
    /* restore the situation : go on step above in the recursion */
    strcpy(&(graph->mangling[graph->subg_depth - 1][0]), "");
    
    strcpy(graph->toConcatenate, "");
    for (i = 0; i < graph->subg_depth; i++)
    {   strcat (graph->toConcatenate, graph->mangling[i]);
    }
    
    graph->subg_depth--;
    
    free(new_ggraph);
}


/**=================================================================================================
*/
void findArcIOWithThisID(struct stream_graph_linkedlist *graph, uint32_t *idx_stream_io)
{   int i;
    for (i = 0; i < graph->nb_arcs; i++)
    {
        if (*idx_stream_io == graph->arc[i].idx_arc_in_graph)
        {   *idx_stream_io = i;
            break;
        }
    }
}

/**=================================================================================================
* returns the pointer to the installed platform node and its index  
*/
void search_platform_node (char *cstring, struct stream_node_manifest **platform_node, 
                 uint32_t *platform_swc_idx, 
                 struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph)
{   uint32_t inode; 
    
    for (inode = 0; inode < platform->nb_nodes; inode++)
    {   if (0 == strncmp(cstring, platform->all_nodes[inode].nodeName,strlen(cstring))) break;
    }   
    *platform_swc_idx = inode;
    *platform_node = &(platform->all_nodes[inode]);
}

void search_graph_node(char *cstring,  struct stream_node_manifest **graph_node, uint32_t *graph_swc_idx,
                  struct stream_graph_linkedlist *graph)
{   uint32_t inode; 
    
    for (inode = 0; inode < graph->nb_nodes +1u; inode++)
    {   if (0 == strncmp(cstring, graph->all_nodes[inode].nodeName,strlen(cstring))) break;
    }   
    *graph_swc_idx = inode;
    *graph_node = &(graph->all_nodes[inode]);
}

//void search_node(char *cstring, struct stream_node_manifest **platform_node, struct stream_node_manifest **graph_node, 
//                 uint32_t *platform_swc_idx, uint32_t *graph_swc_idx,
//                 struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph)
//{   uint32_t inode; 
//    
//    for (inode = 0; inode < platform->nb_nodes; inode++)
//    {   if (0 == strncmp(cstring, platform->all_nodes[inode].nodeName,strlen(cstring))) break;
//    }   
//    *platform_swc_idx = inode;
//    *platform_node = &(platform->all_nodes[inode]);
//
//    for (inode = 0; inode < graph->nb_nodes; inode++)
//    {   if (0 == strncmp(cstring, graph->all_nodes[inode].nodeName,strlen(cstring))) break;
//    }   
//    *graph_swc_idx = inode;
//    *graph_node = &(graph->all_nodes[graph->idx_node]);
//}

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

#define COMPARE(x) 0==strncmp(pt_line, x, strlen(x))

    char* pt_line, ctmp[NBCHAR_LINE], cstring1[NBCHAR_LINE], cstring2[NBCHAR_LINE], paths[MAX_NB_PATH][NBCHAR_LINE];
    int32_t idx_format, idx_node, idx_stream_io, idx_path, i, j, platform_swc_idx, graph_swc_idx;
    struct stream_node_manifest *platform_node, *graph_node;

    pt_line = ggraph_txt;
    idx_format = idx_node = idx_stream_io = 0;
    graph->nb_arcs = graph->nb_nodes = -1;

    jump2next_valid_line(&pt_line);

    while (globalEndFile != FOUND_END_OF_FILE && *pt_line != '\0')
    {
        if (COMPARE(subgraph))
        {   stream_tool_read_subgraph (&pt_line, platform, graph);
        }
        if (COMPARE(set_file_path))         // set_file_path 0 ./  
        {   fields_extract(&pt_line, "CIC", ctmp, &idx_path, cstring1); strcpy(paths[idx_path], cstring1);
        }
        if (COMPARE(graph_location))        // graph_location 1 : keep the graph in Flash and copy in RAM a portion  
        {   fields_extract(&pt_line, "CIC", ctmp, &idx_path); 
        }
        if (COMPARE(graph_map_hwblock))     // graph_map_hwblock <VID> 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->memVID)); 
        }
        if (COMPARE(debug_script_fields))    
        {  
        }
        if (COMPARE(scheduler_return))    
        {   
        }
        if (COMPARE(allowed_processors))    
        {   
        }
        if (COMPARE(include))           
        {   
        }
        if (COMPARE(memory_mapping))    
        {   
        }
        if (COMPARE(top_trace_node))    
        {   
        }

        /* ----------------------------------------------- STREAM IO --------------------------------------------------------*/
        if (COMPARE(stream_io_new))         // stream_io "soft ID" 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arc[graph->idx_arc].idx_arc_in_graph));  graph->idx_arc++;
        }
        if (COMPARE(stream_io_hwid))        // stream_io HWID 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arc[graph->idx_arc].fw_io_idx)); 
        }
        if (COMPARE(stream_io_format))      // stream_io FORMAT 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arc[graph->idx_arc].format)); 
        }            
        if ((COMPARE(stream_io_format)) ||  (COMPARE(stream_io_hwid)))
        {   if (graph->arc[graph->idx_arc].initialized_from_platform == 0)
            {   graph->arc[graph->idx_arc] = platform->io_stream[graph->arc[graph->idx_arc].fw_io_idx];
                graph->arc[graph->idx_arc].initialized_from_platform = 1; /* initialization is done */
            }
        }
        if (COMPARE(stream_io_setting1))
        {   
        }     
        /* ----------------------------------------------- FORMATS ----------------------------------------------------------*/
        if (COMPARE(format_new))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->idx_format)); 
            graph->arcFormat[graph->idx_format].raw_data = STREAM_S16;    /* default format data */
            graph->arcFormat[graph->idx_format].frame_length = 1;
            graph->arcFormat[graph->idx_format].nchan = 1;
            graph->nb_formats ++;
        }
        if (COMPARE(format_frame_length))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arcFormat[graph->idx_format].frame_length)); 
        }
        if (COMPARE(format_raw_data))
        {   
        }   
        if (COMPARE(format_interleaving))
        {   
        }   
        if (COMPARE(format_raw_data))
        {   
        }   
        if (COMPARE(format_nbchan))
        {   
        }   
        if (COMPARE(format_time_stamp))
        {   
        }   
        if (COMPARE(format_time_stamp_size))
        {   
        }   
        if (COMPARE(format_sdomain))
        {   
        }   
        if (COMPARE(format_domain))
        {   
        }   
        if (COMPARE(format_sampling_rate))
        {   
        }   
        if (COMPARE(format_audio_mapping))
        {   
        }   
        if (COMPARE(format_motion_mapping))
        {   
        }   
        if (COMPARE(format_2d_height))
        {   
        }   
        if (COMPARE(format_2d_width))
        {   
        }   
        if (COMPARE(format_2d_border))
        {   
        }   

        /* ----------------------------------------------- NODES ----------------------------------------------------------*/
        if (COMPARE(node_new))  //node <node_name> <instance_index>
        {   uint32_t instance;
            fields_extract(&pt_line, "CCI", ctmp, cstring1, &instance);
            search_platform_node(cstring1, &platform_node, &platform_swc_idx, platform, graph);
            graph->nb_nodes++;
            graph_node = &(graph->all_nodes[graph->nb_nodes]);
            (*graph_node) = (*platform_node);
            graph_node->initialized_from_platform = 1;
        }
        if (COMPARE(node_preset))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->all_nodes[graph->nb_nodes].preset)); 
        }
        if (COMPARE(node_malloc_E))     // malloc done when the arcs are connected to the node (for the arc's format)
        {   fields_extract(&pt_line, "CF", ctmp, &(graph->all_nodes[graph->nb_nodes].malloc_E)); 
        }
        if (COMPARE(node_map_hwblock))
        {   fields_extract(&pt_line, "CII", ctmp, &i, &j); graph->all_nodes[graph->nb_nodes].memreq[i].VID = j; 
        }
        if (COMPARE(node_map_swap))
        {   fields_extract(&pt_line, "CII", ctmp, &i, &j); graph->all_nodes[graph->nb_nodes].memreq[i].toSwap = 1;
            graph->all_nodes[graph->nb_nodes].memreq[i].swapVID = j;
        }
        if (COMPARE(node_map_copy))
        {   
        }   
        if (COMPARE(node_trace_id))
        {   
        }   
        if (COMPARE(node_map_proc))
        {   
        }   
        if (COMPARE(node_map_arch))
        {   
        }   
        if (COMPARE(node_map_rtos))
        {   
        }   
        if (COMPARE(node_map_verbose))
        {   
        }   
        if (COMPARE(node_parameters))
        {   jump2next_valid_line(&pt_line);
            stream_tool_read_parameters(&pt_line, platform, graph, &(graph->all_nodes[graph->nb_nodes])); 
        }

        /* ----------------------------------------------- SCRIPTS ----------------------------------------------------------*/
        if (COMPARE(common_script))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->idx_script));  
        }
        if (COMPARE(script_registers))
        {   
        }
        if (COMPARE(script_pointers))
        {   
        }
        if (COMPARE(script_stack))
        {   
        }
        if (COMPARE(script_mem_shared))
        {   
        }
        if (COMPARE(script_mem_map))
        {   
        }
        if (COMPARE(script_code))
        {   jump2next_valid_line(&pt_line);
            stream_tool_read_script(&pt_line, &(graph->all_scripts[graph->idx_script]));
        }
        
        /* --------------------------------------------- ARCS ----------------------------------------------------------------------*/
        if (COMPARE(arc_input))     //arc_input  idx_stream_io node_name instance arc_index Format
        {   uint32_t inst2, idx_stream_io, swca2, fmt2;
            graph->nb_arcs ++;
            fields_extract(&pt_line, "CICIII", ctmp, &idx_stream_io, cstring2, &inst2, &swca2, &fmt2); 
            findArcIOWithThisID(graph, &idx_stream_io);
            graph->arc[graph->nb_arcs] = graph->arc[idx_stream_io];   /* copy the arc IO details */
            graph->arc[graph->nb_arcs].IO_arc = 1;

            search_graph_node(cstring2, &graph_node, &graph_swc_idx, graph);
            graph->arc[graph->nb_arcs].format_idx_dst = fmt2;           /* @@@ check collision with IO arc */
            graph->arc[graph->nb_arcs].graph_swc_idx_dst = graph_swc_idx;
            graph_node->arc[swca2].format_idx_dst = fmt2;               /* @@@ check collision with node arc */
        }
        if (COMPARE(arc_output))    //arc_output  idx_stream_io node_name instance arc_index Format
        {   uint32_t inst1, idx_stream_io, swca1, fmt1;
            graph->nb_arcs ++;
            fields_extract(&pt_line, "CICIII", ctmp, &idx_stream_io, cstring1, &inst1, &swca1, &fmt1);
            findArcIOWithThisID(graph, &idx_stream_io);
            graph->arc[graph->nb_arcs] = graph->arc[idx_stream_io];   /* copy the arc IO details */
            graph->arc[graph->nb_arcs].IO_arc = 1;

            search_graph_node(cstring1, &graph_node, &graph_swc_idx, graph);
            graph->arc[graph->nb_arcs].format_idx_dst = fmt1;           /* @@@ check collision with IO arc */
            graph->arc[graph->nb_arcs].graph_swc_idx_dst = graph_swc_idx;
            graph_node->arc[swca1].format_idx_dst = fmt1;               /* @@@ check collision with node arc */
        }
        if (COMPARE(arc_new))       //arc node1 instance arc_index arc_format_src node2 instance arc_index arc_format_dst
        {   uint32_t inst1, inst2, swca1, swca2, fmt1, fmt2;
            graph->nb_arcs ++;
            fields_extract(&pt_line, "CCIIICIII", ctmp, cstring1, &inst1, &swca1, &fmt1, cstring2, &inst2, &swca2, &fmt2);
            search_graph_node(cstring1, &graph_node, &graph_swc_idx, graph);
            graph_node->arc[swca1].format_idx_src = fmt1;
            graph_node->arc[swca1].graph_swc_idx_src = graph_swc_idx;

            search_graph_node(cstring2, &graph_node, &graph_swc_idx, graph);
            graph_node->arc[swca2].format_idx_dst = fmt2;
            graph_node->arc[swca2].graph_swc_idx_src = graph_swc_idx;
        } 
        if (COMPARE(arc_map_hwblock))    //arc_map_hwblock     0    
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs].memVID = i;
        }
        if (COMPARE(arc_flow_error))
        {   
        }
        if (COMPARE(arc_debug_cmd))
        {   
        }
        if (COMPARE(arc_debug_reg))
        {   
        }
        if (COMPARE(arc_debug_page))
        {   
        }
        if (COMPARE(arc_flush))
        {   
        }
        if (COMPARE(arc_extend_addr))
        {   
        }
        if (COMPARE(arc_map_hwblock))
        {   
        }
        if (COMPARE(arc_jitter_ctrl))
        {   
        }
    }
}

#ifdef __cplusplus
}
#endif