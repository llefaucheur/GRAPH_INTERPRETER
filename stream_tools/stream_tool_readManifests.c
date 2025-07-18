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
    *domain_index = (char)(-1);
    if (0 == strncmp(input, "general",           strlen("general")))            *domain_index = IO_DOMAIN_GENERAL;         
    if (0 == strncmp(input, "audio_in",          strlen("audio_in")))           *domain_index = IO_DOMAIN_AUDIO_IN;         /* for PDM, I2S, ADC */
    if (0 == strncmp(input, "audio_out",         strlen("audio_out")))          *domain_index = IO_DOMAIN_AUDIO_OUT;
    if (0 == strncmp(input, "gpio",              strlen("gpio")))               *domain_index = IO_DOMAIN_GPIO;             /* extra data for BSP (delay, edge, HiZ, debouncing, analog mix..) */
    if (0 == strncmp(input, "motion",            strlen("motion")))             *domain_index = IO_DOMAIN_MOTION; 
    if (0 == strncmp(input, "2d_in",             strlen("d2_in")))              *domain_index = IO_DOMAIN_2D_IN;            /* control of AGC, zoom in 1/4 image area */
    if (0 == strncmp(input, "2d_out",            strlen("2d_out")))             *domain_index = IO_DOMAIN_2D_OUT;
    if (0 == strncmp(input, "user_interface",    strlen("user_interface")))     *domain_index = IO_DOMAIN_USER_INTERFACE; 
    if (0 == strncmp(input, "analog_in",         strlen("analog_in")))          *domain_index = IO_DOMAIN_ANALOG_IN;        /* sensor with aging control */
    if (0 == strncmp(input, "analog_out",        strlen("analog_out")))         *domain_index = IO_DOMAIN_ANALOG_OUT;
    if (0 == strncmp(input, "platform_1",        strlen("platform1")))          *domain_index = IO_DOMAIN_PLATFORM_1;       /* platform callbacks */
    if (0 == strncmp(input, "platform_2",        strlen("platform2")))          *domain_index = IO_DOMAIN_PLATFORM_2;       
    if (0 == strncmp(input, "platform_3",        strlen("platform3")))          *domain_index = IO_DOMAIN_PLATFORM_3;       

    if (*domain_index == (char)(-1))
    {   fprintf(stderr, "\n\n decode_domain error for %s \n\n", input);
        exit (-6);
    }
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
    arc->format.frame_length_format = 1;   /* frame length format (0:in milliseconds 1:in Bytes) */
    arc->format.frame_length_bytes = 1;         

    while (globalEndFile != FOUND_END_OF_FILE /* && *pt_line != '\0'*/ )
    {
        if (COMPARE(io_commander0_servant1))
        {   fields_extract(&pt_line, "ci", cstring, &(arc->commander0_servant1)); 
        }
        if (COMPARE(io_direction_rx0tx1))
        {   fields_extract(&pt_line, "ci", cstring, &(arc->rx0tx1));
        }
        if (COMPARE(io_raw_format))
        {   fields_extract(&pt_line, "ci", cstring, &(arc->format.raw_data)); 
        }
        if (COMPARE(io_interleaving))
        {   fields_extract(&pt_line, "ci", cstring, &(arc->format.deinterleaved)); 
        }
        if (COMPARE(io_nb_channels))
        {   fields_options_extract(&pt_line, &(arc->nb_channels_option)); 
        }
        if (COMPARE(io_frame_length))
        {   fields_options_extract(&pt_line, &(arc->frame_length_option));  
        }

        if (COMPARE(io_frame_duration))
        {   float fdata;
            fields_extract(&pt_line, "cf", cstring, &fdata); 
            convert_to_mks_unit(cstring, &fdata);   /* conversion to standard unit used in "format" (Hz) */
        }   
        //if (COMPARE(io_units_rescale))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_units_rescale_multiple))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_power_mode))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_position))
        //{   fields_extract(&pt_line, "ci",cstring,  &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_euler_angles))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        if (COMPARE(io_sampling_rate))
        {   fields_options_extract(&pt_line, &(arc->sampling_rate_option)); 
        }
        //if (COMPARE(io_sampling_period_s))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_sampling_period_day))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_sampling_rate_accuracy))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_time_stamp_format))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}

        /* --------------  DOMAIN SPECIFIC --------------- */
        //if (COMPARE(io_analog_scale))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_units_rescale))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_channel_mapping))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_analog_gain))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_digital_gain))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_hp_filter))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_agc))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_router))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}



        //if (COMPARE(io_motion_format))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_motion_sensitivity))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
        //}
        //if (COMPARE(io_motion_averaging))
        //{   fields_extract(&pt_line, "ci", cstring, &(arc->graphalloc_X_bsp_0)); 
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
        {   fields_extract(&pt_line, "cc", ctmp, node->developerName);  
        }
        if (COMPARE(node_name))  
        {   fields_extract(&pt_line, "cc", ctmp, node->nodeName);  
        }
        if (COMPARE(node_nb_arcs))          // node_nb_arcs rx tx
        {   fields_extract(&pt_line, "cii", ctmp, &(node->nbInputArc), &(node->nbOutputArc));  
        }
        if (COMPARE(node_logo))                     // file name (file path of the manifest) 
        {   //fields_extract(&pt_line, "cic", ctmp, node->nodeName);  
        }
        if (COMPARE(node_use_mpdtcm))                
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
        }
        if (COMPARE(node_mask_library))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
        }
        if (COMPARE(node_subtype_units))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
        }
        if (COMPARE(node_architecture))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
        }
        if (COMPARE(node_fpu_used)) 
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
        }
        if (COMPARE(node_version))
        {   fields_extract(&pt_line, "ci", ctmp, &(node->node_node_version));  // TBC
        }
        if (COMPARE(node_stream_version))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
        }
        if (COMPARE(node_mem))              // node_mem  block ID
        {   fields_extract(&pt_line, "ci", ctmp, &idx_mem);  
            if (idx_mem +1 > node->nbMemorySegment) // memory segment counts from 0
            {   node->nbMemorySegment = idx_mem +1;
            }
        }
        if (COMPARE(node_mem_alignement))
        {   fields_extract(&pt_line, "ci", ctmp, &i);
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
        if (COMPARE(node_mem_alloc))            // node_mem_alloc    A=32 
        {   fields_extract(&pt_line, "cici", ctmp, &(node->memreq[idx_mem].size_additional), ctmp, &i); 
        }
        if (COMPARE(node_mem_frame_size_mono))  // "mulfac B" "type" ("i") 
        {   fields_extract(&pt_line, "cici", ctmp,  &(node->memreq[idx_mem].MulFrameSizeMono), 
               node->memreq[idx_mem].TypeFrameMono, &(node->memreq[idx_mem].iarcFrameMono));
        }
        if (COMPARE(node_mem_frame_size))       // "mulfac C" "type" ("j")   
        {   fields_extract(&pt_line, "cici", ctmp,  &(node->memreq[idx_mem].MulFrameSize), 
                  node->memreq[idx_mem].TypeFrame, &(node->memreq[idx_mem].iarcFrame));
        }
        if (COMPARE(node_mem_nbchan))           // "mulfac D" "type" ("k")
        {   fields_extract(&pt_line, "cici", ctmp,  &(node->memreq[idx_mem].MulFrameSizeChan), 
               node->memreq[idx_mem].TypeFrameChan, &(node->memreq[idx_mem].iarcFrameChan));
        }
        if (COMPARE(node_mem_type))
        {   fields_extract(&pt_line, "ci", ctmp, &(node->memreq[idx_mem].stat0work1ret2));
        }
        if (COMPARE(node_mem_speed))
        {   fields_extract(&pt_line, "ci", ctmp, &(node->memreq[idx_mem].speed));
        }
        if (COMPARE(node_mem_relocatable))
        {   fields_extract(&pt_line, "ci", ctmp, &i);// TBC
        }
        if (COMPARE(node_mem_data0prog1))
        {   fields_extract(&pt_line, "ci", ctmp, &i); // TBC
        }
        if (COMPARE(node_not_reentrant))
        {   fields_extract(&pt_line, "ci", ctmp, &i); // default : node_not_reentrant = 0  all nodes are reentrant
        }

        if (COMPARE(node_new_arc))
        {   fields_extract(&pt_line, "ci", ctmp, &idx_arc);  
        }
        if (COMPARE(node_arc_name))
        {   fields_extract(&pt_line, "cc", ctmp, &(node->arc[idx_arc].IO_name));  
        }
        if (COMPARE(node_arc_rx0tx1))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
        }
        if (COMPARE(node_arc_sampling_rate))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].sampling_rate_option));  
        }
        if (COMPARE(node_arc_interleaving))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
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
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].sampling_period_s_option)); 
        }
        if (COMPARE(node_arc_sampling_period_day))
        {   fields_options_extract(&pt_line, &(node->arc[idx_arc].sampling_period_day_option)); 
        }
        if (COMPARE(node_arc_sampling_accuracy))
        {   fields_extract(&pt_line, "cf", ctmp, &(node->arc[idx_arc].sampling_accuracy));  
        }
        if (COMPARE(node_arc_inPlaceProcessing))
        {   fields_extract(&pt_line, "ci", ctmp, &i);  // TBC
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
    uint32_t indexpaths[MAX_NB_PATH];
    char* pt_line;
    uint32_t istream, imem;
    uint32_t nb_paths, ipath, io_al_idx, iproc, i;
    extern uint8_t globalEndFile;
    char *inputFile;

    strcpy(graph_platform_manifest_name, "");
    strcpy(IO_name, "");
    strcpy(node_name_, "");
    if (0 == (inputFile = calloc (MAXINPUT, 1))) {  printf ("\n init error \n"); exit(-1); }

    /*
        STEP 1 : FILE PATHS
    */
    pt_line = all_files;
    jump2next_valid_line(&pt_line);
    fields_extract(&pt_line, "i", &nb_paths);
    for (ipath = 0; ipath < nb_paths; ipath++)
    {   fields_extract(&pt_line, "ic", &(indexpaths[ipath]), paths[ipath]);
    }


    /*
        STEP 2 : read the shared memories
    */
    fields_extract(&pt_line, "i", &(platform->nb_processors));
    fields_extract(&pt_line, "i", &(platform->nb_shared_memory_banks));
    platform->nbMemoryBank_shared_and_private = platform->nb_shared_memory_banks;

    for (imem = 0; imem < platform->nb_shared_memory_banks; imem++)
    {        
        fields_extract(&pt_line, "iiiii", 
            &(platform->membank[imem].offsetID),        /* MEMID long_offset64 index addressing the range of the memory bank */
            &(platform->membank[imem].size),            /* SIZE which size is guaranteed for graph operations */
            &(platform->membank[imem].access),          /* A    0 data/prog R/W, 1 data R, 2 R/W, 3 Prog, 4 R/W */
            &(platform->membank[imem].speed),           /* S    mem_speed_type 0:best effort, 1:normal, 2:fast, 3:critical fast */
            &(platform->membank[imem].stat0work1ret2)   /* T    0 Static, 1 Retention, 2 scratch mem */
        );
        platform->membank[imem].private_ram = 0;
    }



    /*
        STEP 3 : list of processors and their private memories and IOs
        proc ID, arch ID, main proc, nb mem, service mask, I/O
        1        1        1          2       15            7  
    */
    for (iproc = 0; iproc < platform->nb_processors; iproc++)
    {   uint32_t index_mem;

        fields_extract(&pt_line, "iiiiii", 
            &(platform->processor[platform->nb_processors].processorID),             /* proc ID */     
            &(platform->processor[platform->nb_processors].architectureID),          /* arch ID */
            &(platform->processor[platform->nb_processors].IamTheMainProcessor),     /* main proc flag */
            &(platform->processor[platform->nb_processors].nb_private_memory_banks), /* nb private memory bank */
            &(platform->processor[platform->nb_processors].libraries_b),             /* service bit field */
            &(platform->processor[platform->nb_processors].nb_hwio_processor)        /* nb of IOs in affinity with this processor */
        );

        index_mem = platform->nbMemoryBank_shared_and_private;
        for (imem = 0; imem < platform->processor[platform->nb_processors].nb_private_memory_banks; imem++, index_mem++)
        {        
            fields_extract(&pt_line, "iiiii", 
                &(platform->membank[index_mem].offsetID),        /* MEMID long_offset64 index addressing the range of the memory bank */
                &(platform->membank[index_mem].size),            /* SIZE which size is guaranteed for graph operations */
                &(platform->membank[index_mem].access),          /* A    0 data/prog R/W, 1 data R, 2 R/W, 3 Prog, 4 R/W */
                &(platform->membank[index_mem].speed),           /* S    mem_speed_type 0:best effort, 1:normal, 2:fast, 3:critical fast */
                &(platform->membank[index_mem].stat0work1ret2)   /* T    0 Static, 1 Retention, 2 scratch mem */
            );
            platform->membank[imem].private_ram = 1;
        } 
        platform->nbMemoryBank_shared_and_private = index_mem;

        /*
            ;Path   IO Manifest        IO_AL_idx    Comments               
            1 io_platform_2d_in_0.txt          5    camera                 
            1 io_platform_gpio_out_1.txt       8    GPIO/PWM               
        */
        for (istream = 0; istream < platform->processor[platform->nb_processors].nb_hwio_processor; istream++)
        {        
            char tmpchar[NBCHAR_LINE];  /* file name of the HW IO */ 

            fields_extract(&pt_line, "ici", &ipath, tmpchar, &io_al_idx);

            strcpy(platform->arc[io_al_idx].manifest_file, tmpchar);

            for (i = 0; i < nb_paths; i++) {  if (ipath == indexpaths[i]) {strcpy(file_name, paths[i]); break; } }

            strcat (file_name, tmpchar);
            memset(inputFile, 0, MAXINPUT);
            read_input_file(file_name, inputFile);

            /* read all the parameters of the IO arc from this IO manifest */
            read_platform_io_stream_manifest(inputFile, &(platform->arc[io_al_idx]));
            platform->arc[io_al_idx].fw_io_idx = io_al_idx;
            platform->arc[io_al_idx].procID = platform->processor[platform->nb_processors].processorID;
            platform->arc[io_al_idx].archID = platform->processor[platform->nb_processors].architectureID;
            platform->arc[io_al_idx].arc_graph_ID = NOT_CONNECTED_TO_GRAPH;
        } 

        platform->nb_hwio_stream += platform->processor[iproc].nb_hwio_processor; /* sum of all IO, one per processor */
    } // iproc < platform->nb_processors


    /*
        STEP 4 : list of the nodes installed in each processor and/or architectures
    */
    #define _INTERFACE_NODE "graph_interface"
    #define _INTERFACE_NODE_ID 0
    strcpy(platform->all_nodes[_INTERFACE_NODE_ID].nodeName, _INTERFACE_NODE);      /* node[0] = IO interface */
    platform->nb_nodes = 1;     // reserve the Node 0 for "no tasks"

    do
    {   uint32_t procID, archID, foundEndOfNodeList;
        char tmpchar[NBCHAR_LINE] = "";
    
        if (platform->nb_nodes > MAX_NB_NODES)
        {   printf("too much nodes !"); exit(-4);
        }
        fields_extract(&pt_line, "icii", &ipath, tmpchar, &procID, &archID);
        foundEndOfNodeList = (globalEndFile == FOUND_END_OF_FILE);

        for (i = 0; i < nb_paths; i++) {  if (ipath == indexpaths[i]) 
            {strcpy(file_name, paths[i]); break; } }

        strcat(file_name, tmpchar);
        memset(inputFile, 0, MAXINPUT);
        read_input_file(file_name, inputFile);

        read_node_manifest(inputFile, &(platform->all_nodes[platform->nb_nodes]));
        platform->all_nodes[platform->nb_nodes].platform_node_idx = platform->nb_nodes;
        platform->nb_nodes ++;

        if (foundEndOfNodeList)
        {   break;
        }
    } while (1);

    free (inputFile);
}


#ifdef __cplusplus
}
#endif

