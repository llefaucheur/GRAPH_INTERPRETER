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
    

#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>  /* for void fields_extract(char **pt_line, char *types,  ...) */

#include "stream_tool_include.h"


/**
  @brief            decode the domain name 
  @param[in/out]    none
  @return           none
  @par
  @remark
 */
void decode_domain(int *domain_index, char *input)
{                                                       
    if (0 == strncmp(input, "general",           strlen("general")))            *domain_index = IO_DOMAIN_GENERAL;         
    if (0 == strncmp(input, "audio_in",          strlen("audio_in")))           *domain_index = IO_DOMAIN_AUDIO_IN;         /* for PDM, I2S, ADC */
    if (0 == strncmp(input, "audio_out",         strlen("audio_out")))          *domain_index = IO_DOMAIN_AUDIO_OUT;
    if (0 == strncmp(input, "gpio_in",           strlen("gpio_in")))            *domain_index = IO_DOMAIN_GPIO_IN;          /* extra data for BSP (delay, edge, HiZ, debouncing, analog mix..) */
    if (0 == strncmp(input, "gpio_out",          strlen("gpio_out")))           *domain_index = IO_DOMAIN_GPIO_OUT;
    if (0 == strncmp(input, "motion_in",         strlen("motion_in")))          *domain_index = IO_DOMAIN_MOTION;
    if (0 == strncmp(input, "2d_in",             strlen("d2_in")))              *domain_index = IO_DOMAIN_2D_IN;            /* control of AGC, zoom in 1/4 image area */
    if (0 == strncmp(input, "2d_out",            strlen("2d_out")))             *domain_index = IO_DOMAIN_2D_OUT;
    if (0 == strncmp(input, "user_interface_in", strlen("user_interface_in")))  *domain_index = IO_DOMAIN_USER_INTERFACE_IN; 
    if (0 == strncmp(input, "user_interface_out",strlen("user_interface_out"))) *domain_index = IO_DOMAIN_USER_INTERFACE_OUT;
    if (0 == strncmp(input, "analog_sensor",     strlen("analog_sensor")))      *domain_index = IO_DOMAIN_ANALOG_IN;        /* sensor with aging control */
    if (0 == strncmp(input, "analog_out",        strlen("analog_out")))         *domain_index = IO_DOMAIN_ANALOG_OUT;
    if (0 == strncmp(input, "rtc_in",            strlen("rtc")))                *domain_index = IO_DOMAIN_RTC;              /* ticks from clocks */
    if (0 == strncmp(input, "platform_1",        strlen("platform1")))          *domain_index = IO_DOMAIN_PLATFORM_1;       /* platform callbacks */
    if (0 == strncmp(input, "platform_2",        strlen("platform2")))          *domain_index = IO_DOMAIN_PLATFORM_2;       
    if (0 == strncmp(input, "platform_3",        strlen("platform3")))          *domain_index = IO_DOMAIN_PLATFORM_3;       
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
    ///* digital stream format (see "imu_channel_format") */
    //read_binary_param(pt_line, &(pt->channel_format), 0, 0);

    ///* skip the RFC8428 unit description */
    //while (*(*pt_line) == ';')   
    //{   jump2next_line(pt_line);
    //}

    /* IMU options */
    //read_common_data_options(pt_line, &(pt->acc_sensitivity));
    //read_binary_param(pt_line, &(pt->acc_scaling), 0, 0);
    //read_common_data_options(pt_line, &(pt->acc_averaging));

    //read_common_data_options(pt_line, &(pt->gyro_sensitivity));
    //read_binary_param(pt_line, &(pt->gyro_scaling), 0, 0);
    //read_common_data_options(pt_line, &(pt->gyro_averaging));

    //read_common_data_options(pt_line, &(pt->mag_sensitivity));
    //read_binary_param(pt_line, &(pt->mag_scaling), 0, 0);
    //read_common_data_options(pt_line, &(pt->mag_averaging));
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
//    char line[NBCHAR_LINE];
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
    //read_binary_param(pt_line, &(pt->default_index), 0, 0);
    //read_binary_param(pt_line, (void *)&(pt->options), 0/*STREAM_FP32*/, &(pt->nb_option));
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
    uint32_t nb_io_stream;
    uint32_t iproc, iarch, ibank, i;
    struct processor_memory_bank *mem;

    pt_line = inputFile;
    nb_io_stream = 0;

    jump2next_valid_line(&pt_line);
    fields_extract(&pt_line, "IIII", 
        &(platform->nb_architectures), 
        &(platform->nb_processors), 
        &(platform->nbMemoryBank_detailed),
        &(platform->nbOffset)
        );

    for (i = 0; i < MAX_PROC_MEMBANK; i++)
    {   mem = &(platform->membank[i]); 
        mem->offsetID = 0;
        mem->virtualID= 0;     
        mem->speed= 0;         
        mem->stat0work1ret2= 0;
        mem->private_ram= 0;   
        mem->hwio= 0;          
        mem->data0prog1= 0;    
        mem->size= 0;          
        mem->base64= 0;        
        mem->ptalloc_static= 0;    
        mem->max_working= 0;       
        mem->max_working_alignement = 0;
        mem->max_working_booking= 0;
    }

    /*  memory mapping managed using several memory bank */
    for (ibank = 0; ibank <platform->nbMemoryBank_detailed; ibank ++)
    {
        fields_extract(&pt_line, "IIIIIIIII",
            &(platform->membank[ibank].offsetID),
            &(platform->membank[ibank].virtualID),

            &(platform->membank[ibank].speed),
            &(platform->membank[ibank].stat0work1ret2),
            &(platform->membank[ibank].private_ram),
            &(platform->membank[ibank].hwio),
            &(platform->membank[ibank].data0prog1),

            &(platform->membank[ibank].size),
            &(platform->membank[ibank].base64)
         );
    }

    for (iarch = 0; iarch < platform->nb_architectures; iarch++)
    {   for (iproc = 0; iproc < platform->nb_processors; iproc++)
        {        
            fields_extract(&pt_line, "III", 
                &(platform->processor[iproc].processorID),
                &(platform->processor[iproc].IamTheMainProcessor),
                &(platform->processor[iproc].libraries_b)
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

#define COMPARE(x) 0==strncmp(pt_line, x, strlen(x))

void read_platform_io_stream_manifest(char* inputFile, struct arcStruct *arc)
{
    char* pt_line, cstring[NBCHAR_LINE];
    int32_t nb_io_stream;

    pt_line = inputFile;
    nb_io_stream = 0;

    jump2next_valid_line(&pt_line);
    fields_extract(&pt_line, "c", (arc->IO_name));
    fields_extract(&pt_line, "c", cstring);
    decode_domain(&(arc->format.domain), cstring);

    /* default IO values */
    arc->commander0_servant1 = 1;
    arc->format.nchan = 1;
    arc->format.framelength_format = 1;   /* frame length format (0:in milliseconds 1:in samples) */
    arc->format.frame_length = 1;         

    while (globalEndFile != FOUND_END_OF_FILE && *pt_line != '\0')
    {
        if (COMPARE(io_commander0_servant1))
        {   fields_extract(&pt_line, "CI", cstring, &(arc->commander0_servant1)); 
        }
        if (COMPARE(io_direction_rx0tx1))
        {   fields_extract(&pt_line, "CI", cstring, &(arc->rx0tx1));
        }
        if (COMPARE(io_raw_format))
        {   fields_extract(&pt_line, "CI", cstring, &(arc->format.raw_data)); 
        }
        if (COMPARE(io_interleaving))
        {   fields_extract(&pt_line, "CI", cstring, &(arc->format.deinterleaved)); 
        }
        if (COMPARE(io_nb_channels))
        {   fields_extract(&pt_line, "CI", cstring, &(arc->format.nchan)); 
        }
        if (COMPARE(io_frame_length))
        {   fields_options_extract(&pt_line, &(arc->frame_length_option));  
        }

        //if (COMPARE(io_frame_duration))
        //{   fields_extract(&pt_line, "CF", cstring, &(arc->format.frame_length)); 
        //}
        //if (COMPARE(io_units_rescale))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_units_rescale_multiple))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_power_mode))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_position))
        //{   fields_extract(&pt_line, "CI",cstring,  &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_euler_angles))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_sampling_rate))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_sampling_period_s))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_sampling_period_day))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_sampling_rate_accuracy))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_time_stamp_format))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}

        /* --------------  DOMAIN SPECIFIC --------------- */
        //if (COMPARE(io_analog_scale))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_units_rescale))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_channel_mapping))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_analog_gain))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_digital_gain))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_hp_filter))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_agc))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_router))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}



        //if (COMPARE(io_motion_format))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_motion_sensitivity))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_motion_averaging))
        //{   fields_extract(&pt_line, "CI", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}




        //if (COMPARE(io_WhiteBalanceColorTemp))
        //{   
        //}
        // jump2next_valid_line(&pt_line);
    }
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
    char* pt_line, ctmp[NBCHAR_LINE];
    uint32_t i, idx_mem, idx_arc;
 
    /* initializations, fill the default values, different of 0 */
    pt_line = inputFile;
    idx_mem = 0;

    for (i = 0; i < MAX_NB_MEM_REQ_PER_NODE; i++) node->memreq[i].alignmentBytes = 4;
    node->nbInputArc = node->nbOutputArc = node->arc[1].rx0tx1 = node->locking_arc = 1;

    jump2next_valid_line(&pt_line);

    while (globalEndFile != FOUND_END_OF_FILE)
    {
        if (COMPARE(node_developer_name))   
        {   fields_extract(&pt_line, "CC", ctmp, node->developerName);  
        }
        if (COMPARE(node_name))  
        {   fields_extract(&pt_line, "CC", ctmp, node->nodeName);  
        }
        if (COMPARE(node_nb_arcs))          // node_nb_arcs rx tx
        {   fields_extract(&pt_line, "CII", ctmp, &(node->nbInputArc), &(node->nbOutputArc));  
        }
        if (COMPARE(node_logo))                     // file name (file path of the manifest) 
        {   //fields_extract(&pt_line, "CIC", ctmp, node->nodeName);  
        }
        if (COMPARE(node_use_mpdtcm))                
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_mask_library))
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_subtype_units))
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_architecture))
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_fpu_used)) 
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_version))
        {   fields_extract(&pt_line, "CI", ctmp, &(node->node_node_version));  // TBC
        }
        if (COMPARE(node_stream_version))
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_mem))              // node_mem  block ID
        {   fields_extract(&pt_line, "CI", ctmp, &idx_mem);  
            if (idx_mem +1 > node->nbMemorySegment) // memory segment counts from 0
            {   node->nbMemorySegment = idx_mem +1;
            }
        }
        if (COMPARE(node_mem_alignement))
        {   fields_extract(&pt_line, "CI", ctmp, &i);
            switch(i) {
            default:
            case 4:   node->memreq[idx_mem].alignmentBytes = MEM_REQ_4BYTES_ALIGNMENT  ; break;
            case 8:   node->memreq[idx_mem].alignmentBytes = MEM_REQ_8BYTES_ALIGNMENT  ; break;
            case 16:  node->memreq[idx_mem].alignmentBytes = MEM_REQ_16BYTES_ALIGNMENT ; break;
            case 32:  node->memreq[idx_mem].alignmentBytes = MEM_REQ_32BYTES_ALIGNMENT ; break;
            case 64:  node->memreq[idx_mem].alignmentBytes = MEM_REQ_64BYTES_ALIGNMENT ; break;
            case 128: node->memreq[idx_mem].alignmentBytes = MEM_REQ_128BYTES_ALIGNMENT; break;
            }
        }
        if (COMPARE(node_mem_alloc))        // node_mem_alloc    A=32 
        {   fields_extract(&pt_line, "CI", ctmp, &(node->memreq[idx_mem].size0)); 
        }
        if (COMPARE(node_mem_nbchan))       
        {   fields_extract(&pt_line, "CII", ctmp, &(node->memreq[idx_mem].sizeNchan), &(node->memreq[idx_mem].iarcChannelI)); 
        }
        if (COMPARE(node_mem_sampling_rate))
        {   fields_extract(&pt_line, "CFI", ctmp,  &(node->memreq[idx_mem].sizeFS),  &(node->memreq[idx_mem].iarcSamplingJ)); 
        }
        if (COMPARE(node_mem_frame_size))   
        {   fields_extract(&pt_line, "CII", ctmp,  &(node->memreq[idx_mem].sizeFrame),  &(node->memreq[idx_mem].iarcFrameK));
        }
        if (COMPARE(node_mem_type))
        {   fields_extract(&pt_line, "CI", ctmp, &(node->memreq[idx_mem].stat0work1ret2));
        }
        if (COMPARE(node_mem_speed))
        {   fields_extract(&pt_line, "CI", ctmp, &(node->memreq[idx_mem].speed));
        }
        if (COMPARE(node_mem_relocatable))
        {   fields_extract(&pt_line, "CI", ctmp, &i);// TBC
        }
        if (COMPARE(node_mem_data0prog1))
        {   fields_extract(&pt_line, "CI", ctmp, &i); // TBC
        }
        if (COMPARE(node_not_reentrant))
        {   fields_extract(&pt_line, "CI", ctmp, &i); // default : node_not_reentrant = 0  all nodes are reentrant
        }

        if (COMPARE(node_new_arc))
        {   fields_extract(&pt_line, "CI", ctmp, &idx_arc);  
        }
        if (COMPARE(node_arc_name))
        {   fields_extract(&pt_line, "CC", ctmp, &(node->arc[idx_arc].IO_name));  
        }
        if (COMPARE(node_arc_rx0tx1))
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_arc_sampling_rate))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].sampling_rate_option));  
        }
        if (COMPARE(node_arc_interleaving))
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        if (COMPARE(node_arc_nb_channels)) 
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].nb_channels_option)); 
        }
        if (COMPARE(node_arc_raw_format))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].raw_format_options)); 
        }
        if (COMPARE(node_arc_frame_length))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].frame_length_option)); 
        }
        if (COMPARE(node_arc_frame_duration))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].frame_duration_option)); 
        }
        if (COMPARE(node_arc_sampling_period_s))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].sampling_period_s)); 
        }
        if (COMPARE(node_arc_sampling_period_day))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].sampling_period_day)); 
        }
        if (COMPARE(node_arc_sampling_accuracy))
        {   fields_extract(&pt_line, "CF", ctmp, &(node->arc[idx_arc].sampling_accuracy));  
        }
        if (COMPARE(node_arc_inPlaceProcessing))
        {   fields_extract(&pt_line, "CI", ctmp, &i);  // TBC
        }
        /* ----------------------------------------------------------------------
            example               INT FLOAT
           -XXX  1 2 3              I  F     int/float F1  int/float F2  int/float F3
           -XXX  {opt} {opt}        O        option F1 option F2
        */
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
    char file_name[NBCHAR_LINE];
    char graph_platform_manifest_name[NBCHAR_LINE];
    char node_name_[NBCHAR_LINE];
    char IO_name[NBCHAR_LINE];
    char paths[MAX_NB_PATH][NBCHAR_LINE];
    char* pt_line;
    uint32_t nb_stream, istream;
    int32_t nb_paths, ipath, fw_io_idx, processorBitFieldAffinity, clockDomain;
    extern uint8_t globalEndFile;
    int forScanf;
    char *forStr;
    char *inputFile;

    strcpy(graph_platform_manifest_name, "");
    strcpy(IO_name, "");
    strcpy(node_name_, "");

    /*
        STEP 1 : read the file names : and the digital platform capabilities
    */
    pt_line = all_files;
    jump2next_valid_line(&pt_line);
    forScanf = sscanf(pt_line, "%d", &nb_paths);        /* read the PATH directory name */
    jump2next_line(&pt_line);
    for (ipath  = 0; ipath < nb_paths; ipath++)
    {
        forScanf = sscanf(pt_line, "%s", paths[ipath]);        
        jump2next_line(&pt_line);
    }

    jump2next_valid_line(&pt_line);
    forScanf = sscanf(pt_line, "%d %s", &ipath, graph_platform_manifest_name); /* read the platform_manifest name*/

    forStr = strcpy(file_name, paths[ipath]);
    forStr = strcat(file_name, graph_platform_manifest_name);

    if (0 == (inputFile = malloc (MAXINPUT))) exit(0); 
    memset(inputFile, 0, MAXINPUT);
    read_input_file(file_name, inputFile);

    read_platform_digital_manifest(inputFile, platform);

    /*
        STEP 2 : loop on all the list of IO stream manifests
    */
    jump2next_valid_line(&pt_line);
    forScanf = sscanf(pt_line, "%d", &nb_stream);  /* read the number of streams in this plaform */

    platform->nb_hwio_stream = nb_stream;

    for (istream = 0; istream < nb_stream; istream++)
    {
        jump2next_valid_line(&pt_line);
        /* read the number of node's manifest name */
        forScanf = sscanf (pt_line, "%d %s %d %d %d", &ipath, IO_name, &fw_io_idx, &processorBitFieldAffinity, &clockDomain); 
        strcpy(file_name, paths[ipath]);
        strcat(file_name, IO_name);

        platform->arc[fw_io_idx].clockDomain = clockDomain;
        platform->arc[fw_io_idx].processorBitFieldAffinity = processorBitFieldAffinity;

        memset(inputFile, 0, MAXINPUT);
        read_input_file(file_name, inputFile);
        read_platform_io_stream_manifest(inputFile, &(platform->arc[fw_io_idx]));
        platform->arc[fw_io_idx].fw_io_idx = fw_io_idx;
    }

    jump2next_valid_line(&pt_line);

    /*
        STEP 3 : loop on all the list of nodes manifests
    */

#define _INTERFACE_NODE "graph_interface"
#define _INTERFACE_NODE_ID 0
    strcpy(platform->all_nodes[_INTERFACE_NODE_ID].nodeName, _INTERFACE_NODE);      /* node[0] = IO interface */
    platform->nb_nodes = 0;

    do
    {   if (platform->nb_nodes > MAX_NB_NODES)
        {   printf("too much nodes !"); exit(-4);
        }

        forScanf= sscanf (pt_line, "%d %s", &ipath, node_name_); /* read the node's manifest name */
        strcpy(file_name, paths[ipath]);
        forStr = strcat(file_name, node_name_);

        memset(inputFile, 0, MAXINPUT);
        read_input_file(file_name, inputFile);
        read_node_manifest(inputFile, &(platform->all_nodes[platform->nb_nodes]));
        platform->all_nodes[platform->nb_nodes].platform_NODE_idx = platform->nb_nodes;
        platform->nb_nodes ++;

        jump2next_valid_line(&pt_line);
        if (globalEndFile == FOUND_END_OF_FILE)
        {   break;
        }
    } while (1);


    //printf("\n %d nodes ", platform->nb_nodes);
}


#ifdef __cplusplus
}
#endif

