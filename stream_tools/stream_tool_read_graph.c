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
#include "../stream_nodes/arm/script/arm_stream_script_instructions.h"

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



/*
==================================   
*/
void stream_tool_read_parameters(
        char **pt_line, 
        struct stream_platform_manifest *platform, 
        struct stream_graph_linkedlist *graph, 
        uint32_t *ParameterSizeW32,
        uint32_t *PackedParameters)
{
    uint8_t raw_type;
    uint32_t nb_raw, nbytes, nbits;
    uint8_t *ptr_param, *pt0;

    //node->ParameterSizeW32 = 1;
    //ST(node->PackedParameters[0], W32LENGTH_LW4, node->ParameterSizeW32);
            
    pt0 = ptr_param = (uint8_t *)(PackedParameters);
    while (1)
    {
        read_binary_param(pt_line, ptr_param, &raw_type, &nb_raw);
        //if (0 != strstr(script_label, *pt_line))
        //{   fields_extract(&pt_line, "CI", ctmp, &i);
        //}
        if (nb_raw == 0)
            break;
        nbits = stream_bitsize_of_raw(raw_type);
        nbytes = (nbits * nb_raw)/8;
        ptr_param = &(ptr_param[nbytes]);
    }

    nbytes = (int)(ptr_param - pt0); 

    *ParameterSizeW32 = (3 + nbytes) /4;   // n parameters in w32, one byte will consume one w32
    memcpy ((uint8_t *)(PackedParameters), pt0, 4*(*ParameterSizeW32));

    
    while (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
    {   jump2next_valid_line(pt_line);
    }
}


/* ========================================================================
    @@@ TODO read options from graph data 
*/
float read_default_option (struct options *option, uint32_t graph_data)
{
    return 1.0;
}


///* ========================================================================
//    compute node  frame_length max sum
//
//*/
//void compute_frame_length(struct stream_node_manifest* node, struct formatStruct* all_format,
//    uint32_t iarcFrameMono, uint64_t* sizeArcMono, uint32_t iarcFrame, uint64_t* sizeArc,
//    uint64_t* maxin, uint64_t* maxout, uint64_t* maxall, uint64_t* sumin, uint64_t* sumout, uint64_t* sumall,
//    uint64_t* maxinMono, uint64_t* maxoutMono, uint64_t* maxallMono, uint64_t* suminMono, uint64_t* sumoutMono, uint64_t* sumallMono,
//    uint32_t MulFrameSizeMono, uint32_t MulFrameSize)
//{
//    struct formatStruct* format;
//    uint32_t iarc, fmtProd, fmtCons;
//    uint64_t sizeMono, size;
//
//
//}
//
///* ========================================================================
//    compute node  size from nb_chan / max sum
//
//*/
//void compute_nbchan_length(struct stream_node_manifest* node, struct formatStruct* all_format,
//    uint64_t* memChan, uint32_t iarcFrame, uint64_t* sizeArc,
//    uint64_t* maxin, uint64_t* maxout, uint64_t* maxall, uint64_t* sumin, uint64_t* sumout, uint64_t* sumall,
//    uint64_t* maxinMono, uint64_t* maxoutMono, uint64_t* maxallMono, uint64_t* suminMono, uint64_t* sumoutMono, uint64_t* sumallMono,
//    uint32_t MulFrameSizeMono, uint32_t MulFrameSize, uint32_t MulFrameSizeChan)
//{
//    struct formatStruct* format;
//    uint32_t iarc, fmtProd, fmtCons;
//    uint64_t sizeMono, size;
//
//    /* specific arc MONO */
//    if (iarcFrameMono < node->nbInputArc)   /* is it an input arc ? => read consumer fields */
//    {
//        fmtCons = node->arc[iarcFrameMono].fmtCons; format = &(all_format[fmtCons]);
//    }
//    else
//    {
//        fmtProd = node->arc[iarcFrameMono].fmtProd; format = &(all_format[fmtProd]);
//    }
//
//    if (format->frame_format_byte0_time1 == 0)
//    {
//        *sizeArcMono = (uint64_t)(0.5 + format->frame_length_second * format->samplingRate);
//    }
//    else
//    {
//        *sizeArcMono = format->frame_length_bytes;
//    }
//    (*sizeArcMono) = ((*sizeArcMono) * MulFrameSizeMono) / format->nchan;
//
//    /* specific arc multichannel */
//    if (iarcFrame < node->nbInputArc)   /* is it an input arc ? => read consumer fields */
//    {
//        fmtCons = node->arc[iarcFrame].fmtCons; format = &(all_format[fmtCons]);
//    }
//    else
//    {
//        fmtProd = node->arc[iarcFrame].fmtProd; format = &(all_format[fmtProd]);
//    }
//
//    if (format->frame_format_byte0_time1 == 0)
//    {
//        *sizeArc = (uint64_t)(0.5 + format->frame_length_second * format->samplingRate);
//    }
//    else
//    {
//        *sizeArc = format->frame_length_bytes;
//    }
//    (*sizeArc) *= MulFrameSize;
//    (*memChan) = MulFrameSizeChan;
//
//
//    /* input arcs , read the producer format */
//    for (iarc = 0; iarc < node->nbInputArc; iarc++)
//    {
//        fmtProd = node->arc[iarc].fmtProd; format = &(all_format[fmtProd]);
//
//        if (format->frame_format_byte0_time1 == 0)
//        {
//            size = (uint64_t)(0.5 + format->frame_length_second * format->samplingRate);
//        }
//        else
//        {
//            size = format->frame_length_bytes;
//        }
//        sizeMono = (MulFrameSizeMono * size) / format->nchan;
//        *maxin = MulFrameSize * (MAX(*maxin, size));
//        *maxinMono = (MulFrameSizeMono * (MAX(*maxinMono, sizeMono))) / format->nchan;
//        *sumin = MulFrameSize * (*sumin + size);
//        *suminMono = (MulFrameSizeMono * (*suminMono + sizeMono)) / format->nchan;
//    }
//
//    /* output arcs , read the consumer formats */
//    for (iarc = node->nbInputArc; iarc < node->nbInputArc + node->nbOutputArc; iarc++)
//    {
//        fmtCons = node->arc[iarc].fmtCons; format = &(all_format[fmtCons]);
//
//        if (format->frame_format_byte0_time1 == 0)
//        {
//            size = (uint64_t)(0.5 + format->frame_length_second * format->samplingRate);
//        }
//        else
//        {
//            size = format->frame_length_bytes;
//        }
//        sizeMono = (MulFrameSizeMono * size) / format->nchan;
//        *maxout = MulFrameSize * (MAX(*maxout, size));
//        *maxoutMono = (MulFrameSizeMono * (MAX(*maxoutMono, sizeMono))) / format->nchan;
//        *sumout = MulFrameSize * (*sumout + size);
//        *sumoutMono = (MulFrameSizeMono * (*sumoutMono + sizeMono)) / format->nchan;
//    }
//
//    *maxall = MAX((*maxin), (*maxout));
//    *sumall = (*sumin) + (*sumout);
//}


/* ========================================================================
    compute node  memreq_size  for one of the 6 memreq

*/
void compute_add_length (char *type, uint64_t *size, 
        uint64_t sizeArc, uint64_t maxin, uint64_t maxout, uint64_t maxall, uint64_t sumin, uint64_t sumout, uint64_t sumall)
{
    if (strlen(type) == 0) { *size += 0; }      /* nothing to add */
    if (0==strncmp(type, "arc"   , strlen("arc"   ))) {*size += sizeArc;}
    if (0==strncmp(type, "maxin" , strlen("maxin" ))) {*size += maxin ; }
    if (0==strncmp(type, "maxout", strlen("maxout"))) {*size += maxout; }
    if (0==strncmp(type, "maxall", strlen("maxall"))) {*size += maxall; }
    if (0==strncmp(type, "sumin" , strlen("sumin" ))) {*size += sumin ; }
    if (0==strncmp(type, "sumout", strlen("sumout"))) {*size += sumout; }
    if (0==strncmp(type, "sumall", strlen("sumall"))) {*size += sumall; }
}


/* ========================================================================
    compute node  memreq_size  for one of the 6 memreq

*/
void compute_memreq(struct node_memory_bank *m, struct formatStruct *all_format, struct stream_node_manifest *node)
{
    uint64_t finalSize, sizeArcMono;
    uint64_t maxin, maxout, maxall, sumin, sumout, sumall;
    uint64_t maxinMono, maxoutMono, maxallMono, suminMono, sumoutMono, sumallMono;
    uint64_t maxinChan, maxoutChan, maxallChan;
    struct formatStruct* format;
    uint32_t iarc, fmtProd, fmtCons;
    uint64_t sizeArc, nbChan, memSize;

    sizeArcMono = maxinMono = maxoutMono = maxallMono = suminMono = sumoutMono = sumallMono = 0;
    sizeArc = maxin = maxout = maxall = sumin = sumout = sumall = maxinChan = maxoutChan = maxallChan = 0;
    finalSize = 0;

    for (iarc = 0; iarc < node->nbInputArc + node->nbOutputArc; iarc++)
    {
        if (iarc < node->nbInputArc)                /* is it an input arc ? => read producer format */
        {   fmtProd = node->arc[iarc].fmtProd; format = &(all_format[fmtProd]);
        }
        else                                        /* is it an output arc ? => read consumer format */
        {   fmtCons = node->arc[iarc].fmtCons; format = &(all_format[fmtCons]);
        }

        if (format->frame_format_byte0_time1 != 0)       /* format in samples of in time unit */
        {   sizeArc = (uint32_t)(0.5 + format->frame_length_second * format->samplingRate);
            sizeArcMono = (uint32_t)(0.5 + format->frame_length_second * format->samplingRate) / format->nchan;
        }
        else
        {   sizeArc = format->frame_length_bytes;
            sizeArcMono = format->frame_length_bytes / format->nchan;
        }
        nbChan = format->nchan;

        memSize = 0;
        if (m->iarcFrameMono != (-1))               /* did we selected a specific mono arc ? */
        {   if (iarc == m->iarcFrameMono)           /*   is it this one ? */
            {   memSize = (uint64_t)(sizeArcMono * m->MulFrameSizeMono);
            }
        }

        if (m->iarcFrame != (-1))                   /* did we selected a specific multichannel frame size ? */
        {   if (iarc == m->iarcFrame)               /*   is it this one ? */
            {   memSize = (uint64_t)(sizeArcMono * m->MulFrameSize);
            }
        }

        if (m->iarcFrameChan != (-1))               /* did we selected a specific number of channels */
        {   if (iarc == m->iarcFrameChan)           /*   is it this one ? */
            {   memSize = (uint64_t)(nbChan * m->MulFrameSizeChan);
            }
        }

        if (iarc < node->nbInputArc)
        {   maxin = MAX(maxin, sizeArc);
            maxinMono = MAX(maxinMono, sizeArcMono);
            sumin = sumin + sizeArc;
            suminMono = suminMono + sizeArcMono;
        }
        else
        {   maxout = MAX(maxout, sizeArc);
            maxoutMono = MAX(maxoutMono, sizeArcMono);
            sumout = sumout + sizeArc;
            sumoutMono = sumoutMono + sizeArcMono;
        }

        maxall = MAX(maxin, maxout);
        sumall = sumin + sumout;

        compute_add_length(m->TypeFrameMono, &finalSize,
            sizeArcMono, maxinMono, maxoutMono, maxallMono, suminMono, sumoutMono, sumallMono);

        compute_add_length(m->TypeFrame, &finalSize,
            sizeArc, maxin, maxout, maxall, sumin, sumout, sumall);

        compute_add_length(m->TypeFrameChan, &finalSize,
            sizeArc, maxin, maxout, maxall, sumin, sumout, sumall);
    }

    finalSize = ((finalSize +3)>>2) <<2;

    /* add the extra memory requested in the graph (command "node_malloc_add Bytes segment") */
    m->graph_memreq_size = m->size_mem_alloc_A;
    m->graph_memreq_size += finalSize;
    m->graph_memreq_size += m->malloc_add;
}


/* ========================================================================
    Recursive - subgraphs
*/
void stream_tool_read_subgraph (char **pt_line, struct stream_platform_manifest* platform,struct stream_graph_linkedlist *graph)
{   
    char *new_ggraph, file_name[NBCHAR_NAME], file_name2[NBCHAR_NAME], *subName, paths[MAX_NB_PATH][NBCHAR_LINE], dbg;
    uint32_t ipath, i;
    char *dbgchar;
    extern void arm_stream_read_graph(struct stream_platform_manifest* platform,
        struct stream_graph_linkedlist* graph,
        char* ggraph_txt);
    
    strcpy(file_name, ""); strcpy(file_name2, "");
    new_ggraph = calloc (MAXINPUT, 1);
    
    jump2next_valid_line(pt_line);
    subName = &(graph->mangling[graph->subg_depth][0]);
    dbg = sscanf (*pt_line, "%s", subName); jump2next_valid_line(pt_line);
    
    graph->subg_depth++;
    
    /* create the first characters of names of this subgraph */
    strcpy(graph->toConcatenate, "");
    for (i = 0; i < graph->subg_depth; i++)
    {   strcat (graph->toConcatenate, graph->mangling[i]);
        strcat (graph->toConcatenate, "_");
    }
    
    /* read subgraph as a recursion */ 
    dbg = sscanf (*pt_line, "%d %s", &ipath, file_name2); jump2next_valid_line(pt_line);
    strncpy(file_name, paths[ipath], NBCHAR_NAME); 
    file_name[NBCHAR_NAME-1] = '\0';
    file_name2[NBCHAR_NAME-1] = '\0';
    dbgchar = strcat(file_name, file_name2);
    read_input_file (file_name, new_ggraph);

    /* recursion starts here */
    //
    // TODO
    // 
    //arm_stream_read_graph (platform, graph, new_ggraph);
    //
    ///* restore the situation : go on step above in the recursion */
    //strcpy(&(graph->mangling[graph->subg_depth - 1][0]), "");
    //
    //strcpy(graph->toConcatenate, "");
    //for (i = 0; i < graph->subg_depth; i++)
    //{   strcat (graph->toConcatenate, graph->mangling[i]);
    //}
    //
    //graph->subg_depth--;
    
    free(new_ggraph);
}


/**=================================================================================================
*/
void findArcIOWithThisID(struct stream_graph_linkedlist *graph, uint32_t idx_stream_io, uint32_t *arcID)
{   int i;
    
    for (i = 0; i < graph->nb_arcs; i++)
    {
        if (idx_stream_io == graph->arc[i].idx_arc_in_graph)
        {   *arcID = i;
            break;
        }
    }
}

/**=================================================================================================
* returns the pointer to the installed platform node and its index  
*/
void search_platform_node (char *cstring, struct stream_node_manifest **platform_node, 
                 uint32_t *platform_node_idx, 
                 struct stream_platform_manifest *platform, struct stream_graph_linkedlist *graph)
{   uint32_t inode; 
    
    for (inode = 0; inode < platform->nb_nodes; inode++)
    {   if (0 == strncmp(cstring, platform->all_nodes[inode].nodeName,strlen(cstring))) break;
    }   
    *platform_node_idx = inode;
    *platform_node = &(platform->all_nodes[inode]);
}

void search_graph_node(char *cstring,  struct stream_node_manifest **graph_node, uint32_t *graph_NODE_idx,
                  struct stream_graph_linkedlist *graph)
{   uint32_t inode, found; 
    
    found = 0;
    for (inode = 0; inode < graph->nb_nodes; inode++)
    {   if (0 == strncmp(cstring, graph->all_nodes[inode].nodeName,strlen(cstring))) 
        {   found = 1; 
            break;
        }
    }   

    if (found == 0)
    {   printf("\n search_graph_node not found \n");
        exit( 1);
    }
    *graph_NODE_idx = inode;
    *graph_node = &(graph->all_nodes[inode]);
}

/**=================================================================================================
  @brief            (main) 
  @param[in/out]    none
  @return           int
  @par              Copy the arc platform fields to the graph
  @remark
 */
void LoadPlatformArc(struct arcStruct *Agraph, struct arcStruct *Aplatform)
{
    /* decode options :
        struct options raw_format_options;
        struct options nb_channels_option;
        struct options sampling_rate_option;
        struct options sampling_period_s_option;
        struct options sampling_period_day_option;
        struct options frame_length_samples_option;
        struct options frame_duration_option;
    */
    //Agraph->raw_data =
    //Agraph->IO_FMT_manifest.raw_data =

    Agraph->rx0tx1=                   Aplatform->rx0tx1;                    // ARC0_LW1 and RX0TX1_IOFMT : direction rx=0 tx=1 parameter=2 (rx never consumed) 
    Agraph->setupTime=                Aplatform->setupTime;                 // [ms] to avoid this information to being only in the BSP 
    Agraph->settings[0]=              Aplatform->settings[0];               // @@@@@TODO pack format of digital + MSIC options (SETTINGS_IOFMT2), the format depends on the IO domain 
    Agraph->domain=                   Aplatform->domain;                    // domain of operation
    Agraph->fw_io_idx=                Aplatform->fw_io_idx;                 // ID of the interface given in "files_manifests_computer" associated function (platform dependent) 
    Agraph->set0copy1=                Aplatform->set0copy1;                 // SET0COPY1_IOFMT data move through pointer setting of data copy 
    Agraph->commander0_servant1=      Aplatform->commander0_servant1;       // SERVANT1_IOFMT selection for polling protocol 
    Agraph->raw_format_options=       Aplatform->raw_format_options;
    Agraph->nb_channels_option=       Aplatform->nb_channels_option;    
    Agraph->sampling_rate_option=     Aplatform->sampling_rate_option;
    Agraph->sampling_period_s_option= Aplatform->sampling_period_s_option;
    Agraph->sampling_period_day_option=Aplatform->sampling_period_day_option;
    Agraph->frame_length_samples_option=Aplatform->frame_length_samples_option;
    Agraph->frame_duration_option=    Aplatform->frame_duration_option;
    Agraph->sampling_accuracy=        Aplatform->sampling_accuracy;

    strcpy(Agraph->IO_name,     Aplatform->IO_name);        // IO stream name used in the GUI
    strcpy(Agraph->domainName,  Aplatform->domainName);     // arc name used in the GUI 
}
/**=================================================================================================
  @brief            (main) 
  @param[in/out]    none
  @return           int
  @par              Copy the arc platform fields to the graph
  @remark
 */
void LoadPlatformNode(struct stream_node_manifest *graph_node, struct stream_node_manifest *platform_node)
{   uint32_t size;
    graph_node->nbInputArc         = platform_node->nbInputArc;        
    graph_node->nbOutputArc        = platform_node->nbOutputArc;       
    graph_node->nbParamArc         = platform_node->nbParamArc;        
    //graph_node->formatUsed         = platform_node->formatUsed;           
    graph_node->masklib            = platform_node->masklib;              
    graph_node->codeVersion        = platform_node->codeVersion;          
    graph_node->arc_parameter      = platform_node->arc_parameter;        
    graph_node->mask_library       = platform_node->mask_library;         
    graph_node->subtype_units      = platform_node->subtype_units;        
    graph_node->architecture       = platform_node->architecture;         
    graph_node->fpu_used           = platform_node->fpu_used;             
    graph_node->node_node_version  = platform_node->node_node_version;         
    graph_node->stream_version     = platform_node->stream_version;       
    graph_node->nbMemorySegment    = platform_node->nbMemorySegment;   
    graph_node->inPlaceProcessing  = platform_node->inPlaceProcessing; 
    graph_node->arcIDbufferOverlay = platform_node->arcIDbufferOverlay;
    graph_node->locking_arc        = platform_node->locking_arc;

    size = MAX_NB_MEM_REQ_PER_NODE * sizeof (node_memory_bank_t);
    memcpy(graph_node->memreq, platform_node->memreq, size);

    strcpy(graph_node->developerName, platform_node->developerName);  
    strcpy(graph_node->nodeName, platform_node->nodeName);               
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

#define COMPARE(x) (0==strncmp(pt_line, x, strlen(x)))

    char* pt_line, ctmp[NBCHAR_LINE], paths[MAX_NB_PATH][NBCHAR_LINE];
    int32_t idx_format, idx_node, idx_stream_io, idx_path, i, j, iarc, platform_node_idx, idx_node_script;
    int32_t current_arc_is_IO;
    int64_t iL;
    
    pt_line = ggraph_txt;
    idx_path = idx_format = idx_node = idx_stream_io = 0;
    graph->procid_allowed_gr4 = 1;          // default values : main proc = #0
    for (i = 0; i< MAX_NB_NODES; i++) 
    {   graph->arc[i].sizeFactor = 1.0;
    }


    jump2next_valid_line(&pt_line);

    while (globalEndFile != FOUND_END_OF_FILE && *pt_line != '\0')
    {
        if (COMPARE(subgraph))
        {   stream_tool_read_subgraph (&pt_line, platform, graph);
        }   
        if (COMPARE(graph_file_path))       // graph_file_path 0 ./  
        {   fields_extract(&pt_line, "cic", ctmp, &idx_path, paths[idx_path]); 
        }
        if (COMPARE(graph_locations))        // graph_locations x
        {   
            #define m(i) graph->option_graph_locations[i]
            fields_extract(&pt_line, "ciiiiiii", ctmp, &(m(GRAPH_PIO_HW)), &(m(GRAPH_PIO_GRAPH)), &(m(GRAPH_SCRIPTS)), &(m(GRAPH_LINKED_LIST)), &(m(GRAPH_ONGOING)), &(m(GRAPH_FORMATS)), &(m(GRAPH_ARCS))); 
            #undef m
        }
        if (COMPARE(graph_memory_bank))     // graph_memory_bank <VID> 
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->memVID)); 
        }
        if (COMPARE(debug_script_fields))   //  LSB set means "call the debug script before each nanoAppsRT is called"
        {  fields_extract(&pt_line, "ci", ctmp, &(graph->dbg_script_fields));
        }
        if (COMPARE(scheduler_return))      // 3: return to caller when all SWC are starving (default 3)
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->sched_return));
        }
        if (COMPARE(allowed_processors))    
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->procid_allowed_gr4));
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
        if (COMPARE(mem_fill_pattern))
        {   fields_extract(&pt_line, "cih", ctmp, &(graph->debug_pattern_size), &(graph->debug_pattern));
        }
        if (COMPARE(memory_mapping))
        {   //;              original_id  new_id    start   length 
            //memory_mapping      2        100      1024    32700 
            fields_extract(&pt_line, "ciiII", ctmp, &i, &i, &iL, &iL);    // TODO
        }
        if (COMPARE(subgraph))
        {   fields_extract(&pt_line, "c", ctmp);    // TODO
        }
        /* ----------------------------------------------- STREAM IO --------------------------------------------------------*/
        if (COMPARE(stream_io_graph))           // stream_io_graph "soft ID" 
        {   uint32_t io_al_idx; 
            fields_extract(&pt_line, "cii", ctmp, &(graph->arc[graph->nb_arcs].idx_arc_in_graph), &io_al_idx);   
            graph->arc[graph->nb_arcs].ioarc_flag = 1;
            graph->current_io_arc = graph->nb_arcs;
            graph->nb_arcs++; graph->nb_io_arcs++;

            graph->arc[graph->current_io_arc].fw_io_idx = io_al_idx;
            LoadPlatformArc(&(graph->arc[graph->current_io_arc]), &(platform->IO_arc[io_al_idx]));
            graph->arc[graph->current_io_arc].initialized_from_platform = 1; /* initialization is done */
            platform->IO_arc[io_al_idx].arc_graph_ID = graph->current_io_arc;

            if (platform->max_io_al_idx <= io_al_idx)  /* largest io_al_idx to dimension platform_io_al_idx_to_stream[] */
            {   platform->max_io_al_idx = io_al_idx+1;
            }
        }
        if (COMPARE(stream_io_hwid))            // stream_io HWID 
        {   
        }
        if (COMPARE(stream_io_format))          // stream_io FORMAT 
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->arc[graph->current_io_arc].format_idx));
        }            
        if (COMPARE(stream_io_setting))
        {   uint32_t w1, w2, w3;
            fields_extract(&pt_line, "ciii", ctmp, &w1, &w2, &w3); 
            graph->arc[graph->current_io_arc].settings[0] = w1;
            graph->arc[graph->current_io_arc].settings[1] = w2;
            graph->arc[graph->current_io_arc].settings[2] = w3;
        }     
        /* ----------------------------------------------- FORMATS ----------------------------------------------------------*/
        if (COMPARE(format_index))
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->current_format_index)); 
            graph->arcFormat[graph->current_format_index].raw_data = STREAM_S16;    /* default format data */
            graph->arcFormat[graph->current_format_index].frame_length_bytes = 1;
            graph->arcFormat[graph->current_format_index].nchan = 1;
            graph->nb_formats = MAX(graph->current_format_index, graph->nb_formats);
        }
        if (COMPARE(format_frame_length))
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->arcFormat[graph->current_format_index].frame_length_bytes)); 
        }
        if (COMPARE(format_interleaving))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            graph->arcFormat[graph->current_format_index].deinterleaved = i; 
        }   
        if (COMPARE(format_raw_data))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            graph->arcFormat[graph->current_format_index].raw_data = i;
        }   
        if (COMPARE(format_nbchan))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            graph->arcFormat[graph->current_format_index].nchan = i;
        }   
        if (COMPARE(format_sampling_rate))      /* format_sampling_rate  "unit frequency" */
        {   float fdata;
            char unit[NBCHAR_LINE];
            fields_extract(&pt_line, "ccf", ctmp, unit, &fdata); 
            convert_to_mks_unit(unit, &fdata);   /* conversion to standard unit used in "format" (Hz) */
            graph->arcFormat[graph->current_format_index].samplingRate = fdata;
        }   
        if (COMPARE(format_sampling_period))    /* format_sampling_period "unit timePeriod" */
        {   float fdata;
            char unit[NBCHAR_LINE];
            fields_extract(&pt_line, "ccf", ctmp, unit, &fdata); 
            convert_to_mks_unit(unit, &fdata);   /* conversion to standard unit used in "format" (Hz) */
            graph->arcFormat[graph->current_format_index].samplingRate = fdata;
        }   
        if (COMPARE(format_time_stamp))
        {   
        }   
        if (COMPARE(format_sdomain))
        {   
        }   
        if (COMPARE(format_domain))
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
            char ctmp[NBCHAR_LINE], cstring1[NBCHAR_LINE];
            struct stream_node_manifest *platform_node;
            struct stream_node_manifest *graph_node;

            fields_extract(&pt_line, "cci", ctmp, cstring1, &instance);
            search_platform_node(cstring1, &platform_node, &platform_node_idx, platform, graph);
            idx_node_script = -1;
            if (arm_stream_script_index == platform_node_idx)                 /* is it arm_stream_script ? */
            {   
                idx_node_script = graph->nb_nodes;
                graph->all_nodes[idx_node_script].node_script.nb_reg = 6;       /* default number of registers + R12 + stack size */
                graph->all_nodes[idx_node_script].node_script.nb_stack = 6;
                graph->all_nodes[idx_node_script].node_script.ram_heap_size = 0;
                graph->all_nodes[idx_node_script].node_script.arc_script = graph->nb_arcs;
                graph->all_nodes[idx_node_script].arc[0].arcID = graph->nb_arcs;   /* dummy arc used for regsiters */
                
                graph->nb_arcs++;
            }
            graph->all_nodes[graph->nb_nodes].platform_node_idx = platform_node_idx;
            graph_node = &(graph->all_nodes[graph->nb_nodes]);
            LoadPlatformNode(graph_node, platform_node);
            graph_node->initialized_from_platform = 1;
            graph_node->graph_instance = instance;
            graph_node->locking_arc = 1;
            graph->nb_nodes++;

        }
        if (COMPARE(node_preset))
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->all_nodes[graph->nb_nodes -1].preset)); 
        }
        if (COMPARE(node_malloc_add))     // addition of Bytes in a memory bank 
        {   uint32_t nBytes, iMem; 
            fields_extract(&pt_line, "cii", ctmp, &nBytes, &iMem);
             graph->all_nodes[graph->nb_nodes -1].memreq[iMem].malloc_add = nBytes;
        }
        if (COMPARE(node_map_hwblock))
        {   fields_extract(&pt_line, "cii", ctmp, &i, &j); graph->all_nodes[graph->nb_nodes -1].memreq[i].mem_VID = j; 
        }
        if (COMPARE(node_map_swap))
        {   fields_extract(&pt_line, "cii", ctmp, &i, &j); graph->all_nodes[graph->nb_nodes -1].memreq[i].toSwap = 1;
            graph->all_nodes[graph->nb_nodes -1].memreq[i].swapVID = j;
            graph->all_nodes[graph->nb_nodes -1].memreq[i].swap_arc = graph->nb_arcs;
            graph->nb_arcs++;               /* an arc is declared for memory swapping */
        }
        if (COMPARE(node_memory_clear))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  graph->all_nodes[graph->nb_nodes -1].memreq[i].toClear = 1;
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
        if (COMPARE(node_user_key))     //   node_user_key 0x2334809458	; 64 bits key
        {   uint64_t K0; 
            fields_extract(&pt_line, "cI", ctmp, &K0);
            graph->all_nodes[graph->nb_nodes -1].use_user_key = 1;
            graph->all_nodes[graph->nb_nodes -1].user_key[0] = K0;
        }   
        if (COMPARE(node_map_verbose))
        {   
        }   
        if (COMPARE(node_memory_isolation))
        {   
        }   
        if (COMPARE(node_parameters))
        {   uint32_t TAG; 
            fields_extract(&pt_line, "ci", ctmp, &TAG);
            graph->all_nodes[graph->nb_nodes -1].TAG = TAG; /* selection of parameters to load "0" means all */ 
            stream_tool_read_parameters(&pt_line, platform, graph, 
                &(graph->all_nodes[graph->nb_nodes -1].ParameterSizeW32), &(graph->all_nodes[graph->nb_nodes -1].PackedParameters[0])); 
        }
        if (COMPARE(node_script_code))
        {   stream_tool_read_code(&pt_line, platform, graph, &(graph->all_nodes[graph->nb_nodes -1].node_script));  // macro assembler
        }
        /* ----------------------------------------------- SCRIPTS ----------------------------------------------------------*/
        if (COMPARE(common_script))
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->all_scripts[graph->idx_script].script_ID)); /* instance number (its identification) */
            graph->idx_script = graph->nb_scripts;
            graph->all_scripts[graph->idx_script].nb_reg = 16;       /* default number of registers  + R12 + stack size */
            graph->all_scripts[graph->idx_script].nb_stack = 6;
            graph->all_scripts[graph->idx_script].ram_heap_size = 0;
            graph->all_scripts[graph->idx_script].arc_script = graph->nb_arcs;
            graph->nb_arcs++;
            graph->nb_scripts++;
        }
        if (COMPARE(script_name))               // script_name TEST1    ; for references in the GUI
        {   fields_extract(&pt_line, "cc", ctmp, ctmp); 
        }
        if (COMPARE(script_stack))              // script_stack        6    size of the stack in word64 (default = 6)
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            if (idx_node_script >= 0)
            {   graph->all_nodes[idx_node_script].node_script.nb_stack = i; 
            } else
            {   graph->all_scripts[graph->idx_script].nb_stack = i;
            }
        }
        if (COMPARE(script_parameter))          // script_parameter 50    size of the heap
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            if (idx_node_script >= 0)
            {   graph->all_nodes[idx_node_script].node_script.ram_heap_size = i; 
            } else
            {   graph->all_scripts[graph->idx_script].ram_heap_size = i;
            }
        }
        if (COMPARE(script_mem_shared))         // script_mem_shared 1      Is it a private RAM(0) or can it be shared with other scripts(1)
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            if (idx_node_script >= 0)
            {   graph->all_nodes[idx_node_script].node_script.stack_memory_shared = i; 
            } else
            {   graph->all_scripts[graph->idx_script].stack_memory_shared = i;
            }
        }
        if (COMPARE(script_mem_map))            // script_mem_map    0      Memory mapping to VID #0 (default) 
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            if (idx_node_script >= 0)
            {   graph->all_nodes[idx_node_script].node_script.mem_VID = i; 
            } else
            {   graph->all_scripts[graph->idx_script].mem_VID = i;
            }
        }
        if (COMPARE(script_code))               // script_code
        {   stream_tool_read_code(&pt_line, platform, graph, &(graph->all_scripts[graph->idx_script]));  // macro assembler
        }
        if (COMPARE(script_assembler))          // script_assembler 0      ; code of the binary format (0 : default, or native architecture)
        {   fields_extract(&pt_line, "ci", ctmp, &(graph->all_scripts[graph->idx_script].script_format));  

            stream_tool_read_assembler(&pt_line, platform, graph, &(graph->all_scripts[graph->idx_script]));
        }
        
        /* --------------------------------------------- ARCS ----------------------------------------------------------------------*/
        if (COMPARE(arc_input))              //arc_input    idx_stream_io set0copy1 fmtProd     node_name instance arc_index Format
        {   uint32_t instCons, inPort, fmtCons, fmtProd, arcIO, SwcConsGraphIdx, set0copy1;
            struct stream_node_manifest *graph_node_Cons;
            char Consumer[NBCHAR_LINE], HQOS[NBCHAR_LINE];

            fields_extract(&pt_line, "ciiiciiic", ctmp, &idx_stream_io, &set0copy1, &fmtProd, Consumer, &instCons, &inPort, &fmtCons, HQOS); 
            findArcIOWithThisID(graph, idx_stream_io, &arcIO);          /* arcIO receives the stream from idx_stream_IO */
            current_arc_is_IO = 1;
            //graph->arc[arcIO] = graph->arc[arcIO];                    /* copy the already filled arc IO details to this new arc */

            search_graph_node(Consumer, &graph_node_Cons, &SwcConsGraphIdx, graph); /* update the arc of the consumer */
            graph->arc[arcIO].fmtProd = fmtProd;        
            graph->arc[arcIO].fmtCons = fmtCons;        
            graph->arc[arcIO].SwcProdGraphIdx = SwcConsGraphIdx;
            graph->arc[arcIO].set0copy1 = set0copy1;
            graph->arc[arcIO].HQoS = HQOS[0] == 'h' || HQOS[0] == 'H';                  // HQoS
            graph_node_Cons->arc[inPort].fmtProd = fmtProd;   // @@@ check frame_length_option.. is compatible with this format            
            graph_node_Cons->arc[inPort].fmtCons = fmtCons;               
            graph_node_Cons->arc[inPort].arcID = arcIO;               
            graph_node_Cons->arc[inPort].rx0tx1 = 0;
            graph_node_Cons->connected_to_the_graph = 1;
        }
        if (COMPARE(arc_output))             //arc_output   idx_stream_io set0copy1 fmtCons     node_name    instance arc_index Format
        {   uint32_t instProd, outPort, fmtCons, fmtProd, arcIO, SwcProdGraphIdx, set0copy1;
            struct stream_node_manifest *graph_node_Prod;
            char Producer[NBCHAR_LINE], HQOS[NBCHAR_LINE];

            fields_extract(&pt_line, "ciiiciiic", ctmp, &idx_stream_io, &set0copy1, &fmtCons, Producer, &instProd, &outPort, &fmtProd, HQOS);
            findArcIOWithThisID(graph, idx_stream_io, &arcIO);      /* arcIO send the stream to idx_stream_IO */
            current_arc_is_IO = 1;
            //graph->arc[arcIO] = graph->arc[arcIO];                  /* copy the already filled arc IO details to this new arc */

            search_graph_node(Producer, &graph_node_Prod, &SwcProdGraphIdx, graph);
            graph->arc[arcIO].fmtProd = fmtProd;           
            graph->arc[arcIO].fmtCons = fmtCons;           
            graph->arc[arcIO].SwcProdGraphIdx = SwcProdGraphIdx;
            graph->arc[arcIO].set0copy1 = set0copy1;
            graph->arc[arcIO].HQoS = HQOS[0] == 'h' || HQOS[0] == 'H';                  // HQoS
            graph_node_Prod->arc[outPort].fmtProd = fmtProd;               
            graph_node_Prod->arc[outPort].fmtCons = fmtCons;  // @@@ check frame_length_option.. is compatible with this format                
            graph_node_Prod->arc[outPort].arcID = arcIO;               
            graph_node_Prod->arc[outPort].rx0tx1 = 1;
            graph_node_Prod->arc[outPort].HQoS = HQOS[0] == 'h' || HQOS[0] == 'H';      // HQoS
            graph_node_Prod->connected_to_the_graph = 1;
        }
        if (COMPARE(arc_nodes))  //arc_nodes node1 instance arc_index arc_format_src     node2 instance arc_index arc_format_dst
        {   uint32_t instProd, instCons, outPort, inPort, fmtProd, fmtCons, SwcProdGraphIdx, SwcConsGraphIdx;
            struct stream_node_manifest *graph_node_Prod, *graph_node_Cons;
            char Producer[NBCHAR_LINE], Consumer[NBCHAR_LINE], HQOS[NBCHAR_LINE];

            fields_extract(&pt_line, "cciiiciiic", ctmp, Producer, &instProd, &outPort, &fmtProd, Consumer, &instCons, &inPort, &fmtCons, HQOS);
            search_graph_node(Producer, &graph_node_Prod, &SwcProdGraphIdx, graph);
            current_arc_is_IO = 0;

            graph_node_Prod->arc[outPort].fmtProd = fmtProd;
            graph_node_Prod->arc[outPort].SwcProdGraphIdx = SwcProdGraphIdx;
            graph_node_Prod->arc[outPort].arcID = graph->nb_arcs;  
            graph_node_Prod->arc[outPort].rx0tx1 = 1;
            graph_node_Prod->arc[outPort].HQoS = HQOS[0] == 'h' || HQOS[0] == 'H';      // HQoS
            graph_node_Prod->connected_to_the_graph = 1;


            search_graph_node(Consumer, &graph_node_Cons, &SwcConsGraphIdx, graph);
            graph_node_Cons->arc[inPort].fmtCons = fmtCons;
            graph_node_Cons->arc[inPort].SwcConsGraphIdx = SwcConsGraphIdx;
            graph_node_Cons->arc[inPort].arcID = graph->nb_arcs;    
            graph_node_Cons->arc[inPort].rx0tx1 = 0;
            graph_node_Cons->arc[inPort].HQoS = HQOS[0] == 'h' || HQOS[0] == 'H';       // HQoS
            graph_node_Cons->connected_to_the_graph = 1;

            graph->arc[graph->nb_arcs].SwcProdGraphIdx = SwcProdGraphIdx;
            graph->arc[graph->nb_arcs].SwcConsGraphIdx = SwcConsGraphIdx;
            graph->arc[graph->nb_arcs].fmtProd = fmtProd;
            graph->arc[graph->nb_arcs].fmtCons = fmtCons;
            graph->arc[graph->nb_arcs].HQoS = HQOS[0] == 'h' || HQOS[0] == 'H';         // HQoS

            graph->nb_arcs ++;
        } 
        if (COMPARE(arc_map_hwblock))    //arc_map_hwblock     0    
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].memVID = i;
        }
        if (COMPARE(arc_flow_error))     //arc_flow_error  1 1 ; read write 
        {   fields_extract(&pt_line, "cii", ctmp, &i, &j); 
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].flow_error_rd = i;
            graph->arc[iarc].flow_error_wr = j;
        }
        if (COMPARE(arc_debug_cmd))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].memVID = i;
        }
        if (COMPARE(arc_debug_reg))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].memVID = i;
        }
        if (COMPARE(arc_flush))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].memVID = i;
        }
        if (COMPARE(arc_extend_address))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].memVID = i;
        }
        if (COMPARE(arc_map_hwblock))
        {   fields_extract(&pt_line, "ci", ctmp, &i); 
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].memVID = i;
        }
        if (COMPARE(arc_jitter_ctrl))   // factor to apply to the minimum size between the producer and the consumer, default = 1.0 (no jitter)
        {   float jitterFactor;
            fields_extract(&pt_line, "cf", ctmp, &jitterFactor);
            iarc = current_arc_is_IO ? idx_stream_io : graph->nb_arcs - 1;
            graph->arc[iarc].sizeFactor = jitterFactor;
        }
    }
}

#ifdef __cplusplus
}
#endif