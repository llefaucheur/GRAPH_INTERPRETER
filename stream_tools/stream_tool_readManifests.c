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
    

#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>  /* for void fields_extract(char **pt_line, char *types,  ...) */

#include "stream_tool_include.h"


#define MAXINPUT 100000
char inputFile[MAXINPUT];

/**
  @brief            decode the domain name 
  @param[in/out]    none
  @return           none
  @par
  @remark
 */
void decode_domain(int *domain_index, char *input)
{                                                       
    if (0 == strncmp(input, "data_in",           strlen("data_in")))            *domain_index = IO_DOMAIN_DATA_IN; /* w/wo  sampling rates */
    if (0 == strncmp(input, "data_out",          strlen("data_out")))           *domain_index = IO_DOMAIN_DATA_OUT;
    if (0 == strncmp(input, "data_stream_in",    strlen("data_stream_in")))     *domain_index = IO_DOMAIN_DATA_STREAM_IN; /* with sampling rate */
    if (0 == strncmp(input, "data_stream_out",   strlen("data_stream_out")))    *domain_index = IO_DOMAIN_DATA_STREAM_OUT;
    if (0 == strncmp(input, "audio_in",          strlen("audio_in")))           *domain_index = IO_DOMAIN_AUDIO_IN; /* for PDM, I2S, ADC */
    if (0 == strncmp(input, "audio_out",         strlen("audio_out")))          *domain_index = IO_DOMAIN_AUDIO_OUT;
    if (0 == strncmp(input, "gpio_in",           strlen("gpio_in")))            *domain_index = IO_DOMAIN_GPIO_IN; /* extra data for BSP (delay, edge, HiZ, debouncing, analog mix..) */
    if (0 == strncmp(input, "gpio_out",          strlen("gpio_out")))           *domain_index = IO_DOMAIN_GPIO_OUT;
    if (0 == strncmp(input, "motion_in",         strlen("motion_in")))          *domain_index = IO_DOMAIN_MOTION_IN;
    if (0 == strncmp(input, "2d_in",             strlen("d2_in")))              *domain_index = IO_DOMAIN_2D_IN; /* control of AGC, zoom in 1/4 image area */
    if (0 == strncmp(input, "2d_out",            strlen("2d_out")))             *domain_index = IO_DOMAIN_2D_OUT;
    if (0 == strncmp(input, "user_interface_in", strlen("user_interface_in")))  *domain_index = IO_DOMAIN_USER_INTERFACE_IN; 
    if (0 == strncmp(input, "user_interface_out",strlen("user_interface_out"))) *domain_index = IO_DOMAIN_USER_INTERFACE_OUT;
    if (0 == strncmp(input, "command_in",        strlen("command_in")))         *domain_index = IO_DOMAIN_COMMAND_IN; /* USB/UART */
    if (0 == strncmp(input, "command_out",       strlen("command_out")))        *domain_index = IO_DOMAIN_COMMAND_OUT;
    if (0 == strncmp(input, "analog_sensor",     strlen("analog_sensor")))      *domain_index = IO_DOMAIN_ANALOG_SENSOR; /* sensor with aging control */
    if (0 == strncmp(input, "analog_transducer", strlen("analog_transducer")))  *domain_index = IO_DOMAIN_ANALOG_TRANSDUCER;
    if (0 == strncmp(input, "rtc_in",            strlen("rtc_in")))             *domain_index = IO_DOMAIN_RTC_IN; /* ticks from clocks */
    if (0 == strncmp(input, "rtc_out",           strlen("rtc_out")))            *domain_index = IO_DOMAIN_RTC_OUT;
    if (0 == strncmp(input, "storage_out",       strlen("storage_out")))        *domain_index = IO_DOMAIN_STORAGE_OUT; /* periodic dump of captured data */
    if (0 == strncmp(input, "av_codec",          strlen("av_codec")))           *domain_index = IO_DOMAIN_AV_CODEC; /* encoded audio/image/video */
}

/**
  @brief            decode the raw data type
  @param[in/out]    none
  @return           none
  @par
  @remark
 */
void decode_rawtype(uint8_t* domain_index, char* input)
{
    if (0 == strcmp(input, "u8") || 0 == strcmp(input, "i8"))
    {   *domain_index = STREAM_U8;
    }

    if (0 == strcmp(input, "u16") || 0 == strcmp(input, "i16") || 0 == strcmp(input, "h16"))
    {   *domain_index = STREAM_U16;
    }

    if (0 == strcmp(input, "u32") || 0 == strcmp(input, "i32") || 0 == strcmp(input, "h32"))
    {   *domain_index = STREAM_U32;
    }

    if (0 == strcmp(input, "u64") || 0 == strcmp(input, "i64") || 0 == strcmp(input, "h64"))
    {   *domain_index = STREAM_U64;
    }

    if (0 == strcmp(input, "f32"))
    {   *domain_index = STREAM_FP32;
    }

    if (0 == strcmp(input, "f64"))
    {   *domain_index = STREAM_FP64;
    }

    if (0 == strcmp(input, "c"))
    {   *domain_index = STREAM_U8;
    }
}

/**
  @brief            read the IMU manifest data (specific to IMU)
  @param[in/out]    none
  @return           none
  @par
  @remark
 */
void motion_in_specific(char **pt_line, struct stream_interfaces_motion_specific *pt)
{   
    /* digital stream format (see "imu_channel_format") */
    read_binary_param(pt_line, &(pt->channel_format), 0, 0);

    /* skip the RFC8428 unit description */
    while (*(*pt_line) == ';')   
    {   jump2next_line(pt_line);
    }

    /* IMU options */
    read_common_data_options(pt_line, &(pt->acc_sensitivity));
    read_binary_param(pt_line, &(pt->acc_scaling), 0, 0);
    read_common_data_options(pt_line, &(pt->acc_averaging));

    read_common_data_options(pt_line, &(pt->gyro_sensitivity));
    read_binary_param(pt_line, &(pt->gyro_scaling), 0, 0);
    read_common_data_options(pt_line, &(pt->gyro_averaging));

    read_common_data_options(pt_line, &(pt->mag_sensitivity));
    read_binary_param(pt_line, &(pt->mag_scaling), 0, 0);
    read_common_data_options(pt_line, &(pt->mag_averaging));
}

/**
  @brief            read the audio manifest data (specific to audio capture)
  @param[in/out]    none
  @return           none
  @par
  @remark
 */
void audio_in_specific(char** pt_line, struct stream_interfaces_audio_specific* pt)
{
//    char line[MAXNBCHAR_LINE];
//
//    /* digital audio multichannel stream format  */
//    read_binary_param(pt_line, line, 0, 0);
//    decode_audio_channels(pt->bitFieldChannel, line);
//
//    /* skip the RFC8428 unit description */
//    while (*(*pt_line) == ';')
//    {   jump2next_line(pt_line);
//    }
//
//   /* digital scaling from default sensitivity levels */
//    read_binary_param(pt_line, &(pt->audio_scaling), 0, 0);
//
//    /* analog gain setting */
//    read_common_data_options(pt_line, &(pt->analog_gain));
//    read_common_data_options(pt_line, &(pt->digital_gain));
//    read_common_data_options(pt_line, &(pt->AGC));
//    read_common_data_options(pt_line, &(pt->DC_filter));
}



/**
  @brief            decode the domain name to an index
  @param[in/out]    none
  @return           none
  @par
  @remark
 */
void read_common_data_options(char** pt_line, struct options *pt)
{
    read_binary_param(pt_line, &(pt->default_index), 0, 0);
    read_binary_param(pt_line, (void *)&(pt->options), 0/*STREAM_FP32*/, &(pt->nb_option));
}

/** 
  @brief            read the platform manifest fields
  @param[in/out]    none
  @return           none
  @par              
  @remark
 */
void read_platform_digital_manifest(char* inputFile, struct stream_platform_manifest* platform)
{
    char* pt_line;
    uint32_t nb_io_stream, ioffset;
    uint32_t iproc, ibank;

    pt_line = inputFile;
    nb_io_stream = 0;

    fields_extract(&pt_line, "II", &(platform->nb_architectures), &(platform->nb_processors));


    for (iproc = 0; iproc < platform->nb_processors; iproc++)
    {        
        fields_extract(&pt_line, "iiiii", 
            &(platform->processor[iproc].processorID),
            &(platform->processor[iproc].IamTheMainProcessor),
            &(platform->processor[iproc].nb_long_offset),
            &(platform->processor[iproc].nb_threads),
            &(platform->processor[iproc].nbMemoryBank_detailed)
         );    

        fields_extract(&pt_line, "i", &(platform->processor[iproc].libraries_b));
        
        /* read the table of offset 64b for debug / listing */
        for (ioffset = 0; ioffset < platform->processor[iproc].nb_long_offset; ioffset++)
        {
            fields_extract(&pt_line, "iH", 
                &(platform->processor[iproc].offset_ID[ioffset]), 
                &(platform->processor[iproc].offset_base[ioffset]));    
        }

        /*  memory mapping managed using several memory bank */
        for (ibank = 0; ibank < platform->processor[iproc].nbMemoryBank_detailed -1; ibank ++)
        {
        fields_extract(&pt_line, "iiiiiiiII",
             &(platform->processor[iproc].membank[ibank].offsetID),
             &(platform->processor[iproc].membank[ibank].virtualID),

             &(platform->processor[iproc].membank[ibank].speed),
             &(platform->processor[iproc].membank[ibank].working),
             &(platform->processor[iproc].membank[ibank].private_ram),
             &(platform->processor[iproc].membank[ibank].hwio),
             &(platform->processor[iproc].membank[ibank].data_access),

             &(platform->processor[iproc].membank[ibank].size),
             &(platform->processor[iproc].membank[ibank].base32)
         );
        }
    }
}

/** 
  @brief            read the platform manifest fields
  @param[in/out]    none
  @return           none
  @par              
  @remark
 */
void read_platform_io_stream_manifest(char* inputFile, struct arcStruct *io_stream)
{
    char* pt_line, cstring[NBCHAR_LINE];
    int32_t nb_io_stream;

    pt_line = inputFile;
    nb_io_stream = 0;

    fields_extract(&pt_line, "c", (io_stream->IO_name));
    fields_extract(&pt_line, "c", cstring);
    decode_domain(&(io_stream->si.domain), cstring);

    fields_extract(&pt_line, "i", &(io_stream->si.commander0_servant1)); 
    fields_extract(&pt_line, "i", &(io_stream->si.graphalloc_X_bsp_0)); 
    fields_extract(&pt_line, "i", &(io_stream->si.sram0_hwdmaram1)); 
    fields_extract(&pt_line, "i", &(io_stream->si.processorBitFieldAffinity)); 

    fields_extract(&pt_line, "i", &(io_stream->sc.rx0tx1)); 
    fields_extract(&pt_line, "i", &(io_stream->sc.raw_type)); 
    fields_extract(&pt_line, "i", &(io_stream->sc.timestamp)); 
    fields_extract(&pt_line, "i", &(io_stream->sc.framelength_format)); 
    fields_extract(&pt_line, "i", &(io_stream->sc.samplingRate_format)); 
    fields_extract(&pt_line, "f", &(io_stream->sc.percentFSaccuracy)); 

    fields_list(&pt_line, &(io_stream->interleaving_option)); 
    fields_list(&pt_line, &(io_stream->nbchannel_option)); 
    fields_list(&pt_line, &(io_stream->frame_size_option)); 
    fields_list(&pt_line, &(io_stream->sampling_rate_option)); 


    /* read domain-specific digital format data */
    /* read domain-specific mixed-signal format data */

    // @@@@@@@@@@@@@ 
    //switch (pta->si.domain)
    //{
    //case IO_PLATFORM_MOTION_IN:
    //    motion_in_specific(&pt_line, &(io_stream->U.imu));
    //    break;
    //case IO_PLATFORM_AUDIO_IN:
    //    audio_in_specific(&pt_line, &(io_stream->U.audio));
    //    break;
    //}
}


/**
  @brief            read the node manifest fields
  @param[in/out]    none
  @return           none
  @par
  @remark
 */
void read_node_manifest(char* inputFile, struct stream_node_manifest* node)
{
    char *pt_line;
    uint32_t iarch, iarc, ibank, NARCS, iscripts = 0, simple_syntax /* @@@@ */;
    struct arcStruct *pta;

    pt_line = inputFile;

   /* -------------------------- HEADER -------------------------------------- */

    fields_extract(&pt_line, "c", node->developerName);     /* developer's name */
    fields_extract(&pt_line, "c", node->nodeName);          /*  node name for the GUI */
    fields_extract(&pt_line, "iiiiiiii", &(node->nbInputArc), &(node->nbOutputArc), &(node->nbParameArc), 
        &(node->idxStreamingArcSync), /* index of the arc for the byte synchronization in SMP */ 
        &(node->RWinSWC),           /* XDM11 read/write index is managed in SWC, for variable buffer consumption */
        &(node->formatUsed),        /* buffer format is used by the component */
        &(node->deliveryMode),      /* 0:source, 1:binary, 2: 2 binaries (fat binary)*/
        &(node->masklib));          /* dependency to Stream conpute libraries */
 
    node->nbInputOutputArc = node->nbInputArc + node->nbOutputArc;

    fields_extract(&pt_line, "i", &simple_syntax);  // 0:default 1:simplified

    fields_extract(&pt_line, "i", &(node->nbArch));
    for (iarch = 0; iarch < node->nbArch; iarch++)
    {   fields_extract(&pt_line, "ii", &(node->arch), &(node->fpu));  
    }
    
    /* code version sub-version */
    fields_extract(&pt_line, "ii", &(node->codeVersion), &(node->schedulerVersion));  

    /* number of memory banks */
    fields_extract(&pt_line, "i", &(node->nbMemorySegment));  

    for (ibank =0; ibank < node->nbMemorySegment; ibank++)
    {
        fields_extract(&pt_line, "iiffffiiiiiii", 
            &(node->memreq[ibank].size0),           /* 'A' */
            &(node->memreq[ibank].DeltaSize64),    /* 'DA64' */
            &(node->memreq[ibank].sizeNchan),       /* 'B' */
            &(node->memreq[ibank].sizeFS),          /* 'C' */
            &(node->memreq[ibank].sizeFrame),       /* 'D' */
            &(node->memreq[ibank].sizeParameter),   /* 'E' */

            &(node->memreq[ibank].iarcChannelI),
            &(node->memreq[ibank].iarcSamplingJ),
            &(node->memreq[ibank].iarcFrameK),

            &(node->memreq[ibank].alignmentBytes),
            &(node->memreq[ibank].usage),
            &(node->memreq[ibank].speed),
            &(node->memreq[ibank].relocatable) );  
    }
    
    NARCS = node->nbInputArc +  node->nbOutputArc + node->nbParameArc;
    for (iarc = 0; iarc < NARCS; iarc++)
    {
        pta = &(node->arc[iarc]);

        /* inplace buffer destination = n + NARCS */
        fields_extract(&pt_line, "i", &(node->inPlaceProcessing)); 
        if (iarc == node->inPlaceProcessing)
        {   node->arcIDbufferOverlay = 0;
            node->inPlaceProcessing = 0;
        }
        else
        {   node->arcIDbufferOverlay = NARCS - node->inPlaceProcessing;
            node->inPlaceProcessing = 1;
        }

        fields_extract(&pt_line, "i", &(pta->sc.rx0tx1)); 
        fields_extract(&pt_line, "i", &(pta->sc.raw_type)); 
        fields_extract(&pt_line, "i", &(pta->sc.timestamp)); 
        fields_extract(&pt_line, "i", &(pta->sc.framelength_format)); 
        fields_extract(&pt_line, "i", &(pta->sc.samplingRate_format)); 
        fields_extract(&pt_line, "f", &(pta->sc.percentFSaccuracy)); 

        fields_list(&pt_line, &(pta->interleaving_option)); 
        fields_list(&pt_line, &(pta->nbchannel_option)); 
        fields_list(&pt_line, &(pta->frame_size_option)); 
        fields_list(&pt_line, &(pta->sampling_rate_option)); 
    }
}

/**
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par             
                   
  @remark
 */
void arm_stream_read_manifests (struct stream_platform_manifest *platform, char *all_files)
{
    char* pt_line;
    char file_name[MAXNBCHAR_LINE];
    char graph_platform_manifest_name[MAXNBCHAR_LINE];
    uint32_t nb_nodes, inode, nb_stream, istream;
    char node_name[MAXNBCHAR_LINE];
    char IO_name[MAXNBCHAR_LINE];
    char paths[MAX_NB_PATH][NBCHAR_LINE];
    int32_t nb_paths, ipath, fw_io_idx;

    /*
        STEP 1 : read the file names : and the digital platform capabilities
    */
    pt_line = all_files;
    jump2next_valid_line(&pt_line);
    sscanf(pt_line, "%d", &nb_paths);        /* read the PATH directory name */
    jump2next_line(&pt_line);
    for (ipath  = 0; ipath < nb_paths; ipath++)
    {
        sscanf(pt_line, "%s", paths[ipath]);        
        jump2next_line(&pt_line);
    }

    jump2next_valid_line(&pt_line);
    sscanf(pt_line, "%d %s", &ipath, graph_platform_manifest_name); /* read the platform_manifest name*/
    jump2next_line(&pt_line);
    strcpy(file_name, paths[ipath]);
    strcat(file_name, graph_platform_manifest_name);

    read_input_file(file_name, inputFile);

    read_platform_digital_manifest(inputFile, platform);


    /*
        STEP 2 : loop on all the list of IO stream manifests
    */
    jump2next_valid_line(&pt_line);
    sscanf(pt_line, "%d", &nb_stream);  /* read the number of streams in this plaform */
    jump2next_line(&pt_line);
    platform->nb_hwio_stream = nb_stream;

    for (istream = 0; istream < nb_stream; istream++)
    {
        jump2next_valid_line(&pt_line);
        sscanf (pt_line, "%d %s %d", &ipath, IO_name, &fw_io_idx); /* read the number of node's manifest name */
        strcpy(file_name, paths[ipath]);
        strcat(file_name, IO_name);

        read_input_file(file_name, inputFile);
        read_platform_io_stream_manifest(inputFile, &(platform->io_stream[istream]));
    }

    /* read the fw_io_idx mapping to platform capabilities @@@ */


    /*
        STEP 3 : loop on all the list of nodes manifests
    */
    jump2next_valid_line(&pt_line);
    sscanf(pt_line, "%d", &nb_nodes);  /* read the number of nodes in this plaform */
    jump2next_line(&pt_line);

    if (nb_nodes > MAX_NB_NODES)
    {   fprintf(stderr, "too much nodes !"); exit(-4);
    }

#define _INTERFACE_NODE "_graph_interface"
#define _INTERFACE_NODE_ID 0
    strcpy(platform->all_nodes[_INTERFACE_NODE_ID].nodeName, _INTERFACE_NODE);      /* node[0] = IO interface */
    platform->nb_nodes = nb_nodes;
    for (inode = 1; inode < nb_nodes+1; inode++)
    {
        jump2next_valid_line(&pt_line);
        sscanf (pt_line, "%d %s", &ipath, node_name); /* read the node's manifest name */
        strcpy(file_name, paths[ipath]);
        strcat(file_name, node_name);

        read_input_file(file_name, inputFile);
        read_node_manifest(inputFile, &(platform->all_nodes[inode]));
    }
}


#ifdef __cplusplus
}
#endif

