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
    Read and pack the script until finding "end" / SECTION_END

    script_byte_codes
    ....
    end               end of byte codes  
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
    uint8_t *ptr_param, *pt0;

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

    nbytes = (int)(ptr_param - pt0); 

    node->ParameterSizeW32 = (3 + nbytes) /4;   // n parameters in w32, one byte will consume one w32
    memcpy ((uint8_t *)(node->PackedParameters), pt0, 4*(node->ParameterSizeW32));

    
    while (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
    {   jump2next_valid_line(pt_line);
    }
}


/* ========================================================================
    compute node  graph_memreq_size 
*/
void compute_memreq(struct node_memory_bank *m, struct formatStruct *all_format)
{
    uint32_t nchan;
    intPtr_t size;
    float FS, frame_length;

    /* struct graph stream_node_manifest all_nodes : compute the memreq from arc data 
        update graph->all_nodes . memreq . graph_memreq_size
        memory allocation size in bytes = 
        A                               : memory allocation in Bytes (default 0)
        + B x nb_channels of arc(i)     : addition memory as a number of channels in arc index i (default 0)
        + C x sampling_rate of arc(j)   ; .. as proportional to the sampling rate of arc index j (default 0)
        + D x frame_size of arc(k)      ; .. as proportional to the frame size used for the arc index k (default 0)
        + E x parameter from the graph  ; optional field "malloc_E" during the node declaration in the graph, for
                                        ;   example the number of pixels in raw for a scratch area (default 0)
    */

    size = m->size0;                    // A
    
    nchan = all_format[m->iarcChannelI].nchan;          // nchan-1 only when building thr binary graph 
    size += (intPtr_t) (0.5 + (m->sizeNchan * nchan));  // + B x nb_channels_arc(i)
    
    FS = all_format[m->iarcSamplingJ].samplingRate;
    size += (intPtr_t) (0.5 + (m->sizeFS * FS));        // + C x samplingHz_arc(j)
    
    frame_length = all_format[m->iarcFrameK].frame_length;
    size += (intPtr_t) (0.5 + (m->sizeFrame * frame_length));  // + D x frame_size_arc(k)
    
    size = ((size+3)>>2)<<2;
    m->graph_memreq_size = size;
}


/* ========================================================================
    Recursive - subgraphs
*/
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

/**=================================================================================================
  @brief            (main) 
  @param[in/out]    none
  @return           int
  @par              Copy the arc platform fields to the graph
  @remark
 */
void LoadPlatformArc(struct arcStruct *Agraph, struct arcStruct *Aplatform)
{
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
    Agraph->sampling_period_s=        Aplatform->sampling_period_s;
    Agraph->sampling_period_day=      Aplatform->sampling_period_day;
    Agraph->frame_length_option=      Aplatform->frame_length_option;
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
    graph_node->RWinSWC            = platform_node->RWinSWC;              
    graph_node->formatUsed         = platform_node->formatUsed;           
    graph_node->masklib            = platform_node->masklib;              
    graph_node->codeVersion        = platform_node->codeVersion;          
    graph_node->arc_parameter      = platform_node->arc_parameter;        
    graph_node->same_data_rate     = platform_node->same_data_rate;       
    graph_node->using_arc_format   = platform_node->using_arc_format;       
    graph_node->mask_library       = platform_node->mask_library;         
    graph_node->subtype_units      = platform_node->subtype_units;        
    graph_node->architecture       = platform_node->architecture;         
    graph_node->fpu_used           = platform_node->fpu_used;             
    graph_node->node_version       = platform_node->node_version;         
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
    int32_t idx_format, idx_node, idx_stream_io, idx_path, i, j, platform_swc_idx;
    
    pt_line = ggraph_txt;
    idx_path = idx_format = idx_node = idx_stream_io = 0;
    graph->procid_allowed_gr4 = 1;          // default values : main proc = #0
    for (i = 0; i< MAX_NB_NODES; i++) graph->arc[i].sizeFactor = 1.0;


    jump2next_valid_line(&pt_line);

    while (globalEndFile != FOUND_END_OF_FILE && *pt_line != '\0')
    {
        if (COMPARE(subgraph))
        {   stream_tool_read_subgraph (&pt_line, platform, graph);
        }
        if (COMPARE(set_file_path))         // set_file_path 0 ./  
        {   fields_extract(&pt_line, "CIC", ctmp, &idx_path, paths[idx_path]); 
        }
        if (COMPARE(graph_location))        // graph_location 1 : keep the graph in Flash and copy in RAM a portion  
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->option_graph_location)); 
        }
        if (COMPARE(graph_location_offset)) // graph_location offset from VID0 (long_offset[0])
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->graph_location_from_0)); 
        }
        if (COMPARE(graph_memory_bank))     // graph_memory_bank <VID> 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->memVID)); 
        }
        if (COMPARE(debug_script_fields))   //  LSB set means "call the debug script before each nanoAppsRT is called"
        {  fields_extract(&pt_line, "CI", ctmp, &(graph->dbg_script_fields));
        }
        if (COMPARE(scheduler_return))      // 3: return to caller when all SWC are starving (default 3)
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->sched_return));
        }
        if (COMPARE(allowed_processors))    
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->procid_allowed_gr4));
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
        {   fields_extract(&pt_line, "CIH", ctmp, &(graph->debug_pattern_size), &(graph->debug_pattern));
        }
        /* ----------------------------------------------- STREAM IO --------------------------------------------------------*/
        if (COMPARE(stream_io_new))         // stream_io "soft ID" 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arc[graph->nb_arcs].idx_arc_in_graph));   
            graph->arc[graph->nb_arcs].ioarc_flag = 1;
            graph->current_io_arc = graph->nb_arcs;
            graph->nb_arcs++; graph->nb_io_arcs++;
        }
        if (COMPARE(stream_io_hwid))        // stream_io HWID 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arc[graph->current_io_arc].fw_io_idx)); 
            if (graph->arc[graph->current_io_arc].initialized_from_platform == 0)
            {   LoadPlatformArc(&(graph->arc[graph->current_io_arc]), &(platform->arc[graph->arc[graph->current_io_arc].fw_io_idx]));
                graph->arc[graph->current_io_arc].initialized_from_platform = 1; /* initialization is done */
            }
        }
        if (COMPARE(stream_io_format))      // stream_io FORMAT 
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arc[graph->current_io_arc].format)); 
            if (graph->arc[graph->current_io_arc].initialized_from_platform == 0)
            {   LoadPlatformArc(&(graph->arc[graph->current_io_arc]), &(platform->arc[graph->arc[graph->current_io_arc].fw_io_idx]));
                graph->arc[graph->current_io_arc].initialized_from_platform = 1; /* initialization is done */
            }
        }            
        if (COMPARE(stream_io_setting))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arc[graph->current_io_arc].settings)); 
            if (graph->arc[graph->current_io_arc].initialized_from_platform == 0)
            {   LoadPlatformArc(&(graph->arc[graph->current_io_arc]), &(platform->arc[graph->arc[graph->current_io_arc].fw_io_idx]));
                graph->arc[graph->current_io_arc].initialized_from_platform = 1; /* initialization is done */
            }
        }     
        /* ----------------------------------------------- FORMATS ----------------------------------------------------------*/
        if (COMPARE(format_index))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->current_format_index)); 
            graph->arcFormat[graph->current_format_index].raw_data = STREAM_S16;    /* default format data */
            graph->arcFormat[graph->current_format_index].frame_length = 1.0;
            graph->arcFormat[graph->current_format_index].nchan = 1;
            graph->nb_formats = MAX(graph->current_format_index, graph->nb_formats);
        }
        if (COMPARE(format_frame_length))
        {   fields_extract(&pt_line, "CF", ctmp, &(graph->arcFormat[graph->current_format_index].frame_length)); 
        }
        if (COMPARE(format_raw_data))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->arcFormat[graph->current_format_index].raw_data)); 
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
            char ctmp[NBCHAR_LINE], cstring1[NBCHAR_LINE];
            struct stream_node_manifest *platform_node;
            struct stream_node_manifest *graph_node;

            fields_extract(&pt_line, "CCI", ctmp, cstring1, &instance);
            search_platform_node(cstring1, &platform_node, &platform_swc_idx, platform, graph);

            graph->all_nodes[graph->nb_nodes].platform_swc_idx = platform_swc_idx;
            graph_node = &(graph->all_nodes[graph->nb_nodes]);
            LoadPlatformNode(graph_node, platform_node);
            graph_node->initialized_from_platform = 1;
            graph_node->graph_instance = instance;
            graph_node->locking_arc = 1;
            graph->nb_nodes++;

        }
        if (COMPARE(node_preset))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->all_nodes[graph->nb_nodes -1].preset)); 
        }
        if (COMPARE(node_malloc_add))     // addition of Bytes in a memory bank 
        {   uint32_t nBytes, iMem; 
            fields_extract(&pt_line, "CII", ctmp, &nBytes, &iMem);
             graph->all_nodes[graph->nb_nodes -1].memreq[iMem].malloc_add = nBytes;
        }
        if (COMPARE(node_map_hwblock))
        {   fields_extract(&pt_line, "CII", ctmp, &i, &j); graph->all_nodes[graph->nb_nodes -1].memreq[i].mem_VID = j; 
        }
        if (COMPARE(node_map_swap))
        {   fields_extract(&pt_line, "CII", ctmp, &i, &j); graph->all_nodes[graph->nb_nodes -1].memreq[i].toSwap = 1;
            graph->all_nodes[graph->nb_nodes -1].memreq[i].swapVID = j;
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
        if (COMPARE(node_memory_isolation))
        {   
        }   
        if (COMPARE(node_memory_clear))
        {   /* @@@@@  set CLEAR_LW2S */
        }   
        if (COMPARE(node_parameters))
        {   uint32_t TAG; 
            fields_extract(&pt_line, "CI", ctmp, &TAG);
            graph->all_nodes[graph->nb_nodes -1].TAG = TAG; /* selection of parameters to load "0" means all */ 
            stream_tool_read_parameters(&pt_line, platform, graph, &(graph->all_nodes[graph->nb_nodes -1])); 
        }

        /* ----------------------------------------------- SCRIPTS ----------------------------------------------------------*/
        if (COMPARE(common_script))
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->idx_script));  
            if (graph->nb_scripts < graph->idx_script)
            {   graph->nb_scripts = graph->idx_script;
            }
            graph->all_scripts[graph->idx_script].nb_reg = 6;       /* default number of registers and stack size */
            graph->all_scripts[graph->idx_script].nb_stack = 6;
        }
        if (COMPARE(script_stack))          // script_stack        6       ; size of the stack in word64 (default = 6)
        {  fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->all_scripts[graph->idx_script].nb_stack = i; 
        }
        if (COMPARE(script_mem_shared))     // script_mem_shared 1         ; Is it a private RAM(0) or can it be shared with other scripts(1)
        {  fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->all_scripts[graph->idx_script].stack_memory_shared = i; 
        }
        if (COMPARE(script_mem_map))        // script_mem_map    0         ; Memory mapping to VID #0 (default) 
        {  fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->all_scripts[graph->idx_script].mem_VID = i; 
        }
        if (COMPARE(script_code))           // script_code 0               ; code of the binary format (0 : default, or native architecture)
        {   fields_extract(&pt_line, "CI", ctmp, &(graph->all_scripts[graph->idx_script].script_format));  
            //jump2next_valid_line(&pt_line);
            stream_tool_read_script(&pt_line, &(graph->all_scripts[graph->idx_script]));
        }
        
        /* --------------------------------------------- ARCS ----------------------------------------------------------------------*/
        if (COMPARE(arc_input))                                         //arc_input  idx_stream_io node_name instance arc_index Format
        {   uint32_t instCons, inPort, fmtCons, fmtProd, arcIO, SwcConsGraphIdx, set0copy1;
            struct stream_node_manifest *graph_node_Cons;
            char Consumer[NBCHAR_LINE];

            fields_extract(&pt_line, "CIIICIII", ctmp, &idx_stream_io, &set0copy1, &fmtProd, Consumer, &instCons, &inPort, &fmtCons); 
            findArcIOWithThisID(graph, idx_stream_io, &arcIO);      /* arcIO receives the stream from idx_stream_IO */
            //graph->arc[arcIO] = graph->arc[arcIO];                  /* copy the already filled arc IO details to this new arc */

            search_graph_node(Consumer, &graph_node_Cons, &SwcConsGraphIdx, graph); /* update the arc of the consumer */
            graph->arc[arcIO].fmtProd = fmtProd;        
            graph->arc[arcIO].fmtCons = fmtCons;        
            graph->arc[arcIO].SwcProdGraphIdx = SwcConsGraphIdx;
            graph->arc[arcIO].set0copy1 = set0copy1;
            graph_node_Cons->arc[inPort].fmtProd = fmtProd;   // @@@ check frame_length_option.. is compatible with this format            
            graph_node_Cons->arc[inPort].fmtCons = fmtCons;               
            graph_node_Cons->arc[inPort].arcID = arcIO;               
            graph_node_Cons->arc[inPort].rx0tx1 = 0;
        }
        if (COMPARE(arc_output))                                        //arc_output  idx_stream_io node_name instance arc_index Format
        {   uint32_t instProd, outPort, fmtCons, fmtProd, arcIO, SwcProdGraphIdx, set0copy1;
            struct stream_node_manifest *graph_node_Prod;
            char Producer[NBCHAR_LINE];

            fields_extract(&pt_line, "CIIICIII", ctmp, &idx_stream_io, &set0copy1, &fmtCons, Producer, &instProd, &outPort, &fmtProd);
            findArcIOWithThisID(graph, idx_stream_io, &arcIO);      /* arcIO send the stream to idx_stream_IO */
            //graph->arc[arcIO] = graph->arc[arcIO];                  /* copy the already filled arc IO details to this new arc */

            search_graph_node(Producer, &graph_node_Prod, &SwcProdGraphIdx, graph);
            graph->arc[arcIO].fmtProd = fmtProd;           
            graph->arc[arcIO].fmtCons = fmtCons;           
            graph->arc[arcIO].SwcProdGraphIdx = SwcProdGraphIdx;
            graph->arc[arcIO].set0copy1 = set0copy1;
            graph_node_Prod->arc[outPort].fmtProd = fmtProd;               
            graph_node_Prod->arc[outPort].fmtCons = fmtCons;  // @@@ check frame_length_option.. is compatible with this format                
            graph_node_Prod->arc[outPort].arcID = arcIO;               
            graph_node_Prod->arc[outPort].rx0tx1 = 1;
        }
        if (COMPARE(arc_new))       //arc node1 instance arc_index arc_format_src node2 instance arc_index arc_format_dst
        {   uint32_t instProd, instCons, outPort, inPort, fmtProd, fmtCons, SwcProdGraphIdx, SwcConsGraphIdx;
            struct stream_node_manifest *graph_node_Prod, *graph_node_Cons;
            char Producer[NBCHAR_LINE], Consumer[NBCHAR_LINE];

            fields_extract(&pt_line, "CCIIICIII", ctmp, Producer, &instProd, &outPort, &fmtProd, Consumer, &instCons, &inPort, &fmtCons);
            search_graph_node(Producer, &graph_node_Prod, &SwcProdGraphIdx, graph);
            graph_node_Prod->arc[outPort].fmtProd = fmtProd;
            graph_node_Prod->arc[outPort].SwcProdGraphIdx = SwcProdGraphIdx;
            graph_node_Prod->arc[outPort].arcID = graph->nb_arcs;  
            graph_node_Prod->arc[outPort].rx0tx1 = 1;

            search_graph_node(Consumer, &graph_node_Cons, &SwcConsGraphIdx, graph);
            graph_node_Cons->arc[inPort].fmtCons = fmtCons;
            graph_node_Cons->arc[inPort].SwcConsGraphIdx = SwcConsGraphIdx;
            graph_node_Cons->arc[inPort].arcID = graph->nb_arcs;    
            graph_node_Cons->arc[inPort].rx0tx1 = 0;

            graph->arc[graph->nb_arcs].SwcProdGraphIdx = SwcProdGraphIdx;
            graph->arc[graph->nb_arcs].SwcConsGraphIdx = SwcConsGraphIdx;
            graph->arc[graph->nb_arcs].fmtProd = fmtProd;
            graph->arc[graph->nb_arcs].fmtCons = fmtCons;
            graph->nb_arcs ++;
        } 
        if (COMPARE(arc_map_hwblock))    //arc_map_hwblock     0    
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i;
        }
        if (COMPARE(arc_flow_error))
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i;
        }
        if (COMPARE(arc_debug_cmd))
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i;
        }
        if (COMPARE(arc_debug_reg))
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i;
        }
        if (COMPARE(arc_debug_page))
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i; 
        }
        if (COMPARE(arc_flush))
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i;
        }
        if (COMPARE(arc_extend_address))
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i;
        }
        if (COMPARE(arc_map_hwblock))
        {   fields_extract(&pt_line, "CI", ctmp, &i); 
            graph->arc[graph->nb_arcs -1].memVID = i;
        }
        if (COMPARE(arc_jitter_ctrl))   // factor to apply to the minimum size between the producer and the consumer, default = 1.0 (no jitter)
        {   fields_extract(&pt_line, "CF", ctmp, &(graph->arc[graph->nb_arcs -1].sizeFactor)); 
        }
    }
}

#ifdef __cplusplus
}
#endif