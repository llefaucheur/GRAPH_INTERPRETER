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
    "bestEffort",       /* MEM_SPEED_REQ_ANY           0    /* best effort */
    "normal",           /* MEM_SPEED_REQ_NORMAL        1    /* can be external memory */
    "fast",             /* MEM_SPEED_REQ_FAST          2    /* will be internal SRAM when possible */
    "criticalFast",     /* MEM_SPEED_REQ_CRITICAL_FAST 3    /* will be TCM when possible */
};

/* mem_mapping_type */
char memory_usage_name[][CHARLENDOMAINS] = 
{
    "static",           /* MEM_TYPE_STATIC          0  (LSB) memory content is preserved (default ) */
    "worging",          /* MEM_TYPE_WORKING         1  scratch memory content is not preserved between two calls */
    "psudoWorking",     /* MEM_TYPE_PSEUDO_WORKING  2  static only during the uncompleted execution state of the SWC, see “NODE_RUN” */
    "backup",           /* MEM_TYPE_PERIODIC_BACKUP 3  static parameters to reload for warm boot after a crash, holding for example ..*/
};

char domain_name[IO_DOMAIN_MAX_NB_DOMAINS][CHARLENDOMAINS] = 
{
    "",                     // 0 unused
    "data_in",              // #define PLATFORM_DATA_IN              1     not streamed
    "data_out",             // #define PLATFORM_DATA_OUT             2   
    "data_stream_in",       // #define PLATFORM_DATA_STREAM_IN       3     MPEG, temperature, 
    "data_stream_out",      // #define PLATFORM_DATA_STREAM_OUT      4
    "audio_in",             // #define PLATFORM_AUDIO_IN             5      pga, hpf, 3d position
    "audio_out",            // #define PLATFORM_AUDIO_OUT            6
    "gpio_in",              // #define PLATFORM_GPIO_IN              7
    "gpio_out",             // #define PLATFORM_GPIO_OUT             8
    "motion_in",            // #define PLATFORM_MOTION_IN            9
    "2d_in",                // #define PLATFORM_2D_IN               10
    "2d_out",               // #define PLATFORM_2D_OUT              11
    "user_interface_in",    // #define PLATFORM_USER_INTERFACE_IN   12  
    "user_interface_out",   // #define PLATFORM_USER_INTERFACE_OUT  13
    "command_in",           // #define PLATFORM_COMMAND_IN          14
    "command_out",          // #define PLATFORM_COMMAND_OUT         15
    "analog_sensor_in",     // #define PLATFORM_ANALOG_SENSOR       16      aging control        
    "analog_transducer",    // #define PLATFORM_ANALOG_TRANSDUCER   17
    "rtc_in",               // #define PLATFORM_RTC_IN              18
    "rtc_out",              // #define PLATFORM_RTC_OUT             19
    "storage_out"           // #define PLATFORM_STORAGE_OUT         20
     "av_codec",            // #define PLATFORM_AV_CODEC            21                           
     "",                    // #define PLATFORM_UNUSED_2            22                           
     "",                    // #define PLATFORM_UNUSED_3            23
     "",                    // #define PLATFORM_UNUSED_4            24
     "",                    // #define PLATFORM_UNUSED_5            25
     "",                    // #define PLATFORM_UNUSED_6            26
     "",                    // #define PLATFORM_UNUSED_7            27
     "",                    // #define PLATFORM_UNUSED_8            28
     "",                    // #define PLATFORM_UNUSED_9            29
     "",                    // #define PLATFORM_UNUSED_10           30
};


/*
    Read and pack the script until finding "_end_"

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
    Read and pack the parameters until finding "parameter_end"
        2  u8;  0 255                       new preset + tag (all)
        1 u16;  22                          byte length of the parameters from next line
        1  u8;  2                           Two biquads
        1  u8;  0                           postShift
        5 h16; 5678 2E5B 71DD 2166 70B0     b0/b1/b2/a1/a2 
        5 h16; 5678 2E5B 71DD 2166 70B0     second biquad
        ;  _include    1   arm_stream_filter_parameters_x.txt      
    parameter_end    
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
void compute_memreq(struct stream_node_manifest *node, struct formatStruct *all_format, uint64_t *mbank_1_Byte)
{
    uint32_t imem, alignmask, aligninc;
    uint64_t size, workingmem, mem, tmp;
    struct node_memory_bank *m;

    workingmem = 0;

    for (imem = 0; imem < node->nbMemorySegment; imem++)
    {
        /* struct graph stream_node_manifest all_nodes : compute the memreq from arc data 
            update graph->all_nodes . memreq . graph_mem_size
            Memory Size = A + B x nb_channels_arc(i) + C x sampling_arc(j) + D x frame_size_arc(k) 
        */
        m = &(node->memreq[imem]);
        size = m->size0;
        size = size + m->DeltaSize64;   // take the worst case with aarch64 
            tmp = RD(all_format[m->iarcChannelI].FMT1, NCHANM1_FMT1);
            tmp = (int)(0.5 + m->sizeFS * tmp);
        size = size + tmp;
            tmp = RD(all_format[m->iarcSamplingJ].FMT1, SAMPLING_FMT1) << SAMPLING_FMT1_LSB;  /* sampling rate is truncated on MSBs */
            tmp = (int)(0.5 + m->sizeFrame * tmp);
        size = size + tmp;
            tmp = RD(all_format[m->iarcFrameK].FMT0, FRAMESIZE_FMT0);
            tmp = (int)(0.5 + m->sizeParameter * tmp);
        size = size + tmp;

        size = ((size+3)>>2)<<2;
        m->graph_mem_size = size;

        if (m->usage == MEM_TYPE_WORKING)
        {   
            workingmem += size;
            // m->graph_basePACK will be computed after all the static areas (swc+arcs) are allocated 

            //sprintf(DBG,"%s scratch memory bank(%d) address=0x%X",node->nodeName,(int)imem,(int)mem);  
            //DBGG((uint32_t)working address/4, DBG);
        }
        else
        {
            /* use graph_mem_size and alignmentBytes =>  increments mbank_1_Byte generates the graph_basePACK (s) 
                and the "sum_of_working_simplemap" ooverlay of all the nanoAppRTs
                update "max_of_sum_of_working_simplemap"
              assign the lin2pack value to  memreq.graph_basePACK  increment mbank_1_Byte
            */

            mem = (*mbank_1_Byte);
            aligninc = m->alignmentBytes -1;
            alignmask = ~aligninc;
            mem = mem + aligninc;
            mem = mem & alignmask;

            sprintf(DBG,"%s static memory bank(%d) address=0x%X",node->nodeName,(int)imem,(int)mem);  
            DBGG((uint32_t)mem/4, DBG);

            m->graph_basePACK = (uint32_t)mem >> 2; /* memory in W32 */

            /* jump to the next memory segment */
            (*mbank_1_Byte) =  (*mbank_1_Byte) + size;
        }
    }

    if (node->sum_of_working_simplemap < workingmem)
    {   node->sum_of_working_simplemap = workingmem;
    }
}
            


/*----------------------------------------------------------------------------
    convert a physical address to a portable multiprocessor address with offset 0
 *----------------------------------------------------------------------------*/
uint32_t lin2pack_simple (arm_stream_instance_t *S, uint8_t *buffer)
{
    uint64_t mindist;
    uint64_t pack;
    uint64_t buff;
    int64_t distance;
    uint8_t i;

    /* packed address range is [ long_offset[IDX]  +/- 8MB ]*/
#define MAX_PACK_ADDR_RANGE ((1<<(BASEIDX_ARCW0_MSB - BASEIDX_ARCW0_LSB-1))-1)

    buff = (uint64_t)buffer;
    pack = 0;

    /* find the base offset */
    mindist = MAX_PACK_ADDR_RANGE;
    for (i = 0; i < (uint8_t)MAX_NB_MEMORY_OFFSET; i++)
    {
        distance = S->long_offset[i] - buff;
        if (ABS(distance) > MAX_PACK_ADDR_RANGE) 
        {   continue; 
        }
        else
        {   mindist = distance & MAX_PACK_ADDR_RANGE;
            pack = mindist | ((uint64_t)i << (uint8_t)DATAOFF_ARCW0_LSB);
        }
    }

    /* @@@ to check */
    return (uint32_t)pack;
}


/**
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
    uint32_t iscripts = 0, first_arc = 1;
    uint64_t cumulStaticW32, cumulStaticByte, offset_buffer, offset_instance, offset_descriptor;
    char shortFormat[2];

    pt_line = ggraph_txt;
    cumulStaticW32 = GRAPH_HEADER_NBWORDS; 
    dbggraph = graph;

    
    while (NOT_YET_END_OF_FILE == jump2next_valid_line(&pt_line))
    {
        /* -------------------------- HEADER -------------------------------------- */
        if (0 == strncmp (pt_line, HEADER, strlen(HEADER)))
        {   fields_extract(&pt_line, "III", 
                &(graph->PackedFormat), &scriptctrl, &procs);

            graph->offsetToRam = 0;
        }

        /* -------------------------- PATHS --------------------------------------- */
        if (0 == strncmp (pt_line, PATHS, strlen(PATHS)))
        {   int i;

            i = 0;
            while (1)
            {   if (fields_extract(&pt_line, "C", paths[i]) > 0)
                    break;
            }
        }
        /* ----------------------------top_graph_interface------------------------- */
        /* 
        ;   - index
        ;   - stream format 
        ;   - ID of the interface given in "files_manifests_computer"
        ;   - common setting (8bits intlv/nchan/frame/FS = ARC0 producer format + mixed-signal settings 24bits)
        ;   - VID of buffer : memory bank index from platform_manifest_computer.txt (special DMA, speed)

        */
        if (0 == strncmp (pt_line, IO_INTERFACE, strlen(IO_INTERFACE)))
        {   uint32_t fw_io_idx, iformat, settings, idx, memVID_buffer, i;
            struct arcStruct *arcIO;

            /*
            top_graph_interface
                2               4 interfaces (max 255)
            ;   I  F  I S  V
                0  0  2 0  0       index-0   format 0 FWIDX=2 gpio_out_0  default settings, VID0
                1  1  3 0  0       index-1,  format 1 FWIDX=3 line_out_0  default settings, VID0   
                _end_    
            */
            for (i=0; i<=platform->nb_hwio_stream; i++)
            {   graph->arcIO[i].top_graph_index = 0xFFFF;   /* reset the index for later search the good ones */
                graph->arcIO[i].arcIDstream = 0xFFFF;
            }

            graph->nbio_interfaces = 0;
            graph->nbio_interfaces_with_arcBuffer = 0;

            while (1)
            {   
                if (fields_extract(&pt_line, "CIIIII", shortFormat, &idx, &iformat, &fw_io_idx, &settings, &memVID_buffer) < 0)
                {   break;
                }
                if (idx != i)
                {   printf ("\n\n IO index incorrect \n\n");
                    exit (-100);
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

            }

            /* all the possible IO are in the binary graph */
            cumulStaticW32 += (platform->nb_hwio_stream * STREAM_IOFMT_SIZE_W32);

            offset_instance = cumulStaticW32;
        }


        /* ----------------------------STREAM_FORMAT------------------------------- */
        if (0 == strncmp (pt_line, FORMAT, strlen(FORMAT)))
        {   uint32_t frameSize, raw, nchan, timestp, intlv, specific;
            uint32_t formatID;
            struct formatStruct *format;
            float FS;
            union 
            {   uint32_t u32;
                float f32;
            } U;

            graph->nb_formats = 0;
            while (1)
            {   /*
                    the "options" are given in the graph  (interleaving, nchan, framesize, sampling rate)
                */
                if (fields_extract(&pt_line, "iiiifiii",  
                    &formatID, &frameSize, &raw, &nchan, &FS, &timestp, &intlv, &specific) < 0)
                {   break;
                }
                graph->nb_formats ++;

                format = &(graph->arcFormat[formatID]);

                format->FMT0 = 0;
                ST(format->FMT0, RAW_FMT0, raw);
                ST(format->FMT0, INTERLEAV_FMT0, intlv);
                ST(format->FMT0, FRAMESIZE_FMT0, frameSize);

                format->FMT1 = 0;
                U.f32 = FS;
                format->FMT1 = U.u32;
                ST(format->FMT1, TIMSTAMP_FMT1, timestp);   
                ST(format->FMT1, NCHANM1_FMT1, nchan -1);  

                format->FMT2 = specific;
                FS = 0;
                FS = U.f32;
            }
            cumulStaticW32 += (graph->nb_formats * STREAM_FORMAT_SIZE_W32);
        }


        /* -------------------------------------------------------------------- */
        //node
        //    arm_stream_filter 0         node subroutine name + instance ID
        //;
        //    0 0 0 0 0                   preset if no parameter + local script index (0=none)
        //                              ; no assignment to proc / arch / verbose debug trace
        //
        //    parameters
        //        2  u8;  0 255                       new preset + tag (all)
        //        1 u16;  22                          byte length of the parameters from next line
        //        1  u8;  2                           Two biquads
        //        1  u8;  0                           postShift
        //        5 h16; 5678 2E5B 71DD 2166 70B0     b0/b1/b2/a1/a2 
        //        5 h16; 5678 2E5B 71DD 2166 70B0     second biquad
        //        ;  _include    1   arm_stream_filter_parameters_x.txt      
        //    parameter_end
        //    node_end

        if (0 == strncmp (pt_line, NODE, strlen(NODE)))
        {   char node_name[MAXINPUT];
            struct stream_node_manifest *node, *platform_node;
            uint32_t inode, tmp;
            char *pt_line_saved;


            node = &(graph->all_nodes[graph->nb_nodes]); 
            platform_node = 0;
            fields_extract(&pt_line, "ci", node_name, &tmp);  
            node_name[MAXINPUT-1] = '\0';

            if (0 != strcmp(node_name, "arm_script"))
            {
                node->instance_idx = tmp;
                for (inode = 1; inode < platform->nb_nodes+1; inode++)
                {   platform_node = &(platform->all_nodes[inode]);
                    if (0 == strncmp(node_name, platform_node->nodeName,strlen(platform_node->nodeName)))
                    {   break;
                    }
                }

                /* COPY DEFAULT DATA FROM MANIFEST in graph->all_node[] */
                *node = platform->all_nodes[inode];
                node->swc_idx = inode;

                fields_extract(&pt_line, "CIIIIII", shortFormat,
                        &(node->preset),
                        &(node->local_script_index),
                        &(node->swc_assigned_arch),
                        &(node->swc_assigned_proc),
                        &(node->swc_assigned_priority),
                        &(node->swc_verbose)
                    );  

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
                    {   case 4: fields_extract(&pt_line, "IIIII", &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID), &(node->memreq[2].VID), &(node->memreq[0].VID)); break;
                        case 3: fields_extract(&pt_line, "IIII",  &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID), &(node->memreq[2].VID)); break;
                        case 2: fields_extract(&pt_line, "III",   &Pseg, &(node->memreq[0].VID), &(node->memreq[1].VID)); break;
                        case 1: fields_extract(&pt_line, "II",    &Pseg, &(node->memreq[0].VID)); break;
                        default: case 0: break;
                    }
                }

                ST(node->headerPACK, SWC_IDX_LW0, node->swc_idx);
                ST(node->headerPACK, ARCSRDY_LW0, platform_node->nbInputOutputArc);
                ST(node->headerPACK, ARCLOCK_LW0, platform_node->idxStreamingArcSync);
                ST(node->headerPACK, NBARCW_LW0, platform_node->nbParameArc + platform_node->nbInputOutputArc);
                ST(node->headerPACK, PRIORITY_LW0, node->swc_assigned_priority);
                ST(node->headerPACK, ARCHID_LW0, node->swc_assigned_arch);
                ST(node->headerPACK, PROCID_LW0, node->swc_assigned_proc);
                ST(node->headerPACK, SCRIPT_LW0, node->local_script_index);

                pt_line_saved = pt_line;
                jump2next_valid_line(&pt_line);

                if (0 == strncmp (pt_line, PARAMETER_START, strlen(PARAMETER_START)))
                {  stream_tool_read_parameters(&pt_line, node); /*  returns on "_end_"  */
                }
                else
                {   node->PackedParameters[0] = 0;
                    node->defaultParameterSizeW32 = 0;
                    ST(node->PackedParameters[0], W32LENGTH_LW3, 1);
                    ST(node->PackedParameters[0], PRESET_LW3, node->preset);
                    pt_line = pt_line_saved;
                }

                ST(node->PackedParameters[0], TRACEID_LW3, node->swc_verbose);

                /* keep the parameter header for the PRESET set at reset */
                if (node->defaultParameterSizeW32 < 1) node->defaultParameterSizeW32 =1;

                cumulStaticW32 += 1;                       /* SWC header */
                tmp = node->nbInputArc + node->nbOutputArc + node->nbParameArc;
                cumulStaticW32 += (tmp+1)/2;               /* SWC arcs */
                cumulStaticW32 += node->nbMemorySegment;   /* one word per memory bank */
                cumulStaticW32 += node->defaultParameterSizeW32; /* parameters */

                graph->nb_nodes++;
            }
            else  /* SCRIPTS 
                        no SET_PARAMETER step  the "nregs, stacklength, byte code format" are pre-loaded in the FIFO  
                   */
            {
                uint32_t nreg, stackdepth, nbByteCodes=0, nbBytesCodes, sharedRAM;
                uint32_t scriptFormat;  /* select byte-code format / binary code of the processor */

                //FILE *ptf_graph_script_bytecode;
                //char filename[120];

                iscripts = tmp;     // @@@ warning this index must fit with the SWC declaration &(node->local_script_index)
                fields_extract(&pt_line, "iii", &nreg, &stackdepth, &sharedRAM, &scriptFormat);  

                //sprintf (filename, "SCRIPT_%d.txt", iscripts); 
                //ptf_graph_script_bytecode = fopen(filename, "wt");

                graph->script_indirect[iscripts] = graph->nb_byte_code;    // BYTECODE offset in script_bytecode[]
                graph->script_nregs[iscripts] = nreg;
                graph->script_stackdepthW32[iscripts] = stackdepth;
                if (sharedRAM)  // @@@@
                {   graph->max_nregs = MAX(graph->max_nregs, nreg); 
                    graph->max_stack = MAX(graph->max_stack, stackdepth); 
                    graph->scriptRAMshared[iscripts] = 1;
                    graph->atleastOneScriptShared = 1;
                } 
                else
                {   graph->nb_scriptsARC ++;
                }


                /* reserve 2 bytes for the TX arc ID and the optional const-parameters arc ID */
                graph->script_bytecode[graph->nb_byte_code] = 0x55;    graph->nb_byte_code ++;
                graph->script_bytecode[graph->nb_byte_code] = 0x55;    graph->nb_byte_code ++;
                stream_tool_read_script(&pt_line, &(graph->script_bytecode[graph->nb_byte_code]), &nbBytesCodes);

                graph->nb_byte_code += nbBytesCodes;
                //graph->nb_byte_code = (3+graph->nb_byte_code) & 0xFFFFFFFC;
                /*cumulStaticW32 += (3+nbBytesCodes)>>2;*/

                graph->nb_scripts ++;


                // check extra parameters need to be inserted in the static data arc (indexed with "Read")
                //      parameters         arc with parameters 
                //          2  u8; 0 255                        no preset, TAG = "all parameters"
                //          1  u8;  2                           Two biquads
                //          1  u8;  0                           postShift
                //          5 h16; 1231 1D28 1231 63E8 D475     b0/b1/b2/-a1/-a2  ellip(4, 1, 40, 3600/8000, 'low') 
                //          5 h16; 1231 0B34 1231 2470 9821     second biquad
                //          ;  _include    1   arm_stream_filter_parameters_x.txt      
                //          _end_

                /* parameters in RAM */
                jump2next_valid_line(&pt_line);
                if (0 == strncmp (pt_line, PARAMETER_RAM_START, strlen(PARAMETER_RAM_START)))
                {   stream_tool_read_script(&pt_line, 
                        &(graph->script_embedded_param[iscripts][0]), 
                        &(graph->script_param_length[iscripts]) );
                }

                /* parameters in FLASH W32 aligned */
                if (0 == strncmp (pt_line, PARAMETER_START, strlen(PARAMETER_START)))
                {   uint32_t tmp;
                    graph->nb_byte_code = ((graph->nb_byte_code +3)>>2)<<2; // align parameters on W32 address
                
                    stream_tool_read_script(&pt_line, 
                        &(graph->script_bytecode[graph->nb_byte_code]), 
                        &tmp );
                    graph->nb_byte_code += tmp;
                }

            }
        }

        /* -------------------------------------------------------------------- */
        //arc
        //    0 0  0 0 0 0 0 0   0 0       ProdFMT ConsFMT   OVF, UND, DBGCMD, REG, FLUSH, EXTD     VID IOSizeMulfac 
        //    _graph_interface    0  0        0= IO "interface" from application processor, second parameter is unused
        //    arm_stream_filter   0  0        [0]:filter instance       [0]:RX arc of the node
        //;    parameters
        //;       _include    2   TestPattern.txt filepathID and fileName (test-pattern, NN model, ...)
        //;       parameter_end    
        //    arc_end
        //    

        if (0 == strncmp (pt_line, ARC, strlen(ARC)))
        {   uint32_t arcID, ProdFMT, ConsFMT, VID,   OVF, UND, CMD, REG, FLUSH, EXTD;
            uint32_t inode, instSrc, top_graph_idx, set0_copy1, iarcSrc, instDst, iarcDst, node_inst, minFrameSize;
            float IOSizeMulfac;
            struct stream_node_manifest *node, *platform_node;
            char nodeNameSrc[120], nodeNameDst[120], shortFormat[2], *node_name;
            struct formatStruct *format;
            uint32_t tmp, arcBufferSizeByte, arcBufferBaseW32;
            struct arcStruct *arc;
            struct arcStruct *arcIO;
            uint32_t fw_io_idx;



            /* NO OTHER SCRIPT AFTER ARC DECLARATION */
            cumulStaticW32 += (graph->nb_scripts +1)>>1; // graph->script_indirect[..]
            cumulStaticW32 += (graph->atleastOneScriptShared + graph->nb_scriptsARC) 
                * SIZEOF_ARCDESC_W32; // TX arc used to lock the script, declare the static memory and constants

            cumulStaticW32 += (3+graph->nb_byte_code)>>2;
            arcID = 0;

          while (1)
          {
            arc = &(graph->arc[arcID]); 
            arc->arcID = arcID;

            if (fields_extract(&pt_line, "ciiiiiiiiif",  shortFormat,
                &ProdFMT, &ConsFMT, &OVF, &UND, &CMD, &REG, &FLUSH, &EXTD, &VID, &IOSizeMulfac
                ) < 0)
            {   break;
            }

            if (shortFormat[0] == 'S' || shortFormat[0] == 's')   /* is it a short format ? */
            {   OVF = 0; UND = 0; CMD = 0; REG = 0; FLUSH = 0; EXTD = 0; VID = 0; IOSizeMulfac = 1.0;
            }
            graph->nb_arcs++;

            fields_extract(&pt_line, "cii", nodeNameSrc, &instSrc, &iarcSrc);  
            fields_extract(&pt_line, "cii", nodeNameDst, &instDst, &iarcDst);  

            //minFrameSize=0;
             
            if (0 == strncmp(nodeNameSrc, "_graph_interface",NBCHAR_LINE))
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

            if (0 == strncmp(nodeNameDst, "_graph_interface", NBCHAR_LINE))
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
                {   minprodcons = MIN(a,b);
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
            arcBufferBaseW32 = (int)cumulStaticW32;        /* in W32   */

            cumulStaticByte = cumulStaticW32 * 4;
            cumulStaticByte += arcBufferSizeByte;
            cumulStaticW32 = ((cumulStaticByte + 3)>>2);            /* next buffer is W32-aligned */
          

            // @@@@ arc sequence starting with the one used for locking, the streaming arcs, then the metadata arcs 
            // @@@@ arc(tx) used for locking is ARC0_LW1

            ST(arc->ARCW0, PRODUCFMT_ARCW0, ProdFMT);
            ST(arc->ARCW0,   DATAOFF_ARCW0, 0);
            ST(arc->ARCW0,   BASEIDX_ARCW0, arcBufferBaseW32);      
            sprintf(DBG,"Data stream ARC%d base address=0x%X  size=0x%X",arcID, arcBufferBaseW32, arcBufferSizeByte);  
            DBGG(arcBufferBaseW32, DBG); // check on Byte address

            ST(arc->ARCW1, CONSUMFMT_ARCW1, ConsFMT);
            ST(arc->ARCW1,   MPFLUSH_ARCW1, FLUSH);
            ST(arc->ARCW1, DEBUG_REG_ARCW1, REG);
            ST(arc->ARCW1, BUFF_SIZE_ARCW1, minFrameSize);

            ST(arc->ARCW2, COMPUTCMD_ARCW2, CMD);
            ST(arc->ARCW2, UNDERFLRD_ARCW2, UND);
            ST(arc->ARCW2,  OVERFLRD_ARCW2, OVF);
            ST(arc->ARCW2,    EXTEND_ARCW2, EXTD);
            ST(arc->ARCW2,      READ_ARCW2, 0);

            ST(arc->ARCW3, COLLISION_ARCW3, 0);
            ST(arc->ARCW3, ALIGNBLCK_ARCW3, 0);
            ST(arc->ARCW3,     WRITE_ARCW3, 0);

          } /* while 1*/

          offset_descriptor = cumulStaticW32;
          cumulStaticW32 += ((graph->nbio_interfaces_with_arcBuffer + graph->nb_arcs) * SIZEOF_ARCDESC_W32);
          offset_buffer = cumulStaticW32;
            
        }   /*  if (0 == strncmp (pt_line, ARC, strlen(ARC))) */
    }



    /* arc used for SCRIPTS */            
    {   uint32_t iscripts, iByteCode, first_script_sharedRAM, sharedArc;
        uint32_t arcBufferSizeByte, arcBufferBaseW32, RegAndStackW32, stackdepthW32;
        struct arcStruct *arc;

        first_script_sharedRAM = 1;
        graph->script_indirect[0] = (uint8_t)(0xFF & graph->nb_scripts); //LSB
        graph->script_indirect[1] = (uint8_t)((graph->nb_scripts)>>8); //MSB

        for (iscripts = 0 ; iscripts < graph->nb_scripts; iscripts++)
        {    
            arc = &(graph->arc[graph->nb_arcs]); 
            arc->ARCW0 = 0;
            arc->ARCW1 = 0;
            arc->ARCW2 = 0;
            arc->ARCW3 = 0;

            if (0 == graph->scriptRAMshared[iscripts] || first_script_sharedRAM)
            {   if (first_script_sharedRAM && (0 != graph->scriptRAMshared[iscripts]))
                {   first_script_sharedRAM = 0;
                    sharedArc = graph->nb_arcs;
                }

                RegAndStackW32 = graph->script_nregs[iscripts];
                stackdepthW32 = graph->script_stackdepthW32[iscripts];
                arcBufferSizeByte = RegAndStackW32 * SCRIPT_REGISTER_SIZEBYTE; /* SCRIPT_REGISTER_SIZEBYTE = 9 */
                arcBufferSizeByte+= stackdepthW32 * SCRIPT_REGISTER_SIZEBYTE; /* stack element on 9 bytes too */
                arcBufferSizeByte = ((arcBufferSizeByte +3)>>2)<<2;
                graph->script_stackLengthW32[iscripts] = arcBufferSizeByte>>2;

                // constants
                arcBufferSizeByte += graph->script_param_length[iscripts];
                arcBufferBaseW32 = (int)cumulStaticW32;

                iByteCode = graph->script_indirect[iscripts+1];  // @@@@@@@@@@@@@@@@@@
                graph->script_bytecode[iByteCode]   = (uint8_t)(0xFF & graph->nb_arcs); // LSB
                graph->script_bytecode[iByteCode+1] = (uint8_t)((graph->nb_arcs) >>8);  // MSB
                cumulStaticByte = cumulStaticW32 * 4;
                cumulStaticByte += arcBufferSizeByte;
                cumulStaticW32 = ((cumulStaticByte + 3)>>2);            /* next buffer is W32-aligned */

                ST(arc->ARCW0, PRODUCFMT_ARCW0, 0);
                ST(arc->ARCW0,   DATAOFF_ARCW0, 0);
                ST(arc->ARCW0,   BASEIDX_ARCW0, arcBufferBaseW32);      
                sprintf(DBG,"Script stack and registers in ARC%d base address=0x%X  size=0x%X",graph->nb_arcs, arcBufferBaseW32, arcBufferSizeByte);  
                DBGG(arcBufferBaseW32, DBG); // check on Byte address
                ST(arc->ARCW1, BUFF_SIZE_ARCW1, arcBufferSizeByte);


                //@@@@ add nregs, stacklen, auto-reset/clean on 4bits MSB
                graph->script_offset[iscripts] = arcBufferBaseW32;
                graph->nb_arcs ++;   // script arcs
            }

            /* second sharedRAM script : reuse the RAM of the first shared script */
            else
            {   iByteCode = graph->script_indirect[iscripts+1]; //@@@@@@@@@@@@@@@@@
                graph->script_bytecode[iByteCode]   = (uint8_t)(0xFF & sharedArc); // LSB
                graph->script_bytecode[iByteCode+1] = (uint8_t)((sharedArc) >>8);  // MSB
            }
        }
    }   /* arcs of SCRIPTS */


    {             
        uint32_t inode, ibin;
        struct stream_node_manifest *node;


        graph->end_binary_graph = 0;

        offset_instance = cumulStaticW32;
        graph->mbank_1_Byte = (offset_instance << 2);

        /* ----------------------- MEMORY MAPPING SECTION --------------------------------*/

        for (ibin = 0; ibin < offset_descriptor; ibin++)
        {   graph->binary_graph[ibin] = 0x11111111;             /* header area */
        }

        for (ibin = (int)offset_descriptor; ibin < offset_buffer; ibin++)
        {   graph->binary_graph[ibin] = 0xDDDDDDDD;             /* static area for arcs descriptors */
        }

        for (ibin = (int)offset_buffer; ibin < offset_instance; ibin++)
        {   graph->binary_graph[ibin] = 0xBBBBBBBB;             /* static area for arcs buffers */
        }

        for (ibin = (int)offset_instance; ibin < MAXBINARYGRAPHW32; ibin++)
        {   graph->binary_graph[ibin] = 0x22222222;             /* static area for arcs buffers */
        }
        
        for (inode = 0; inode < graph->nb_nodes; inode++)
        {
            node = &(graph->all_nodes[inode]);

            /* update struct node_memory_bank without graph_basePACK = f(arc format) 
            use graph_mem_size and alignmentBytes =>  increments mbank_1_Byte generates the graph_basePACK (s) 
                and the "sum_of_working_simplemap" ooverlay of all the nanoAppRTs
                update "max_of_sum_of_working_simplemap"
                */
            compute_memreq(node, graph->arcFormat, &(graph->mbank_1_Byte));

            if (graph->max_of_sum_of_working_simplemap < node->sum_of_working_simplemap)
            {   graph->max_of_sum_of_working_simplemap = node->sum_of_working_simplemap;
            }
        }

        //////////////////
        /* arc used for SCRIPTS */            
        {   uint32_t iscripts, iByteCode, ibin;
            for (iscripts = 0 ; iscripts < graph->nb_scripts; iscripts++)
            {    
                if (0 < graph->script_param_length[iscripts])
                {   ibin = graph->script_offset[iscripts];
                    ibin += graph->script_stackLengthW32[iscripts];
                    for (iByteCode = 0; iByteCode < graph->script_param_length[iscripts]; iByteCode+=4)
                    {   uint32_t W32, tmp; uint8_t *pt8;
                        pt8 = &(graph->script_embedded_param[iscripts][iByteCode]);
                        W32 = *pt8++;   tmp = *pt8++;
                        W32 |= tmp<<8;  tmp = *pt8++;
                        W32 |= tmp<<16; tmp = *pt8++;
                        W32 |= tmp<<24; 
                        graph->binary_graph[ibin++] = W32;
                    }
                }
            }
        }   /* arcs of SCRIPTS */


        /* PIO LOOP aAND ALLOC BUFFER @@@@@ + COPY FMT3 */
        //for (inode = 0; inode < graph->nb_nodes; inode++)
        //{
        //    node = &(graph->all_nodes[inode]);

        //    /* update struct node_memory_bank without graph_basePACK = f(arc format) 
        //    use graph_mem_size and alignmentBytes =>  increments mbank_1_Byte generates the graph_basePACK (s) 
        //        and the "sum_of_working_simplemap" overlay of all the nanoAppRTs
        //        update "max_of_sum_of_working_simplemap"
        //        */
        //    compute_memreq(node, graph->arcFormat, &(graph->mbank_1_Byte));

        //    if (graph->max_of_sum_of_working_simplemap < node->sum_of_working_simplemap)
        //    {   graph->max_of_sum_of_working_simplemap = node->sum_of_working_simplemap;
        //    }
        //}


        /* round the base address of working area to W32 */
        graph->mbank_1_Byte = ((graph->mbank_1_Byte+3) >> 2)<<2;

        for (ibin = (uint32_t)(graph->mbank_1_Byte)/4; ibin < MAXBINARYGRAPHW32; ibin++)
        {   graph->binary_graph[ibin] =(uint32_t) 0x333333333;                /* working area */
        }


        /* static areas are set, now update nodes with their working area */
        for (inode = 0; inode < graph->nb_nodes; inode++)
        {   
            uint32_t ibank, tmpw;

            node = &(graph->all_nodes[inode]);
            tmpw = (int)(graph->mbank_1_Byte);

            /* nodes can have several working banks */
            for (ibank = 0; ibank < node->nbMemorySegment; ibank++)
            {   
                if (node->memreq[ibank].usage == MEM_TYPE_WORKING)
                {   node->memreq[ibank].graph_basePACK = tmpw >> 2;     // memory in W32
                    tmpw = tmpw + (uint32_t)(node->memreq[ibank].graph_mem_size);

                    sprintf(DBG,"WORKING %s base=0x%X  max size (all)=0x%X",node->nodeName, 
                        (int)(graph->mbank_1_Byte), (int)(graph->max_of_sum_of_working_simplemap));  
                    DBGG((uint32_t)graph->mbank_1_Byte, DBG);

                }
            }   
        }

        (graph->mbank_1_Byte) = (graph->mbank_1_Byte) + graph->max_of_sum_of_working_simplemap;

        for (ibin = (uint32_t)(graph->mbank_1_Byte)/4; ibin < MAXBINARYGRAPHW32; ibin++)
        {   graph->binary_graph[ibin] = 0x44444444;                 /* free are */
        }

        graph->end_binary_graph = (uint32_t)(graph->mbank_1_Byte)/4;
    }
}

#ifdef __cplusplus
}
#endif