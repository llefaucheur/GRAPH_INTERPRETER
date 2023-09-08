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
void decode_domain(uint8_t *domain_index, char *input)
{
    if (0 == strcmp(input, "data_in"))              *domain_index = PLATFORM_DATA_IN; /* w/wo  sampling rates */
    if (0 == strcmp(input, "data_out"))             *domain_index = PLATFORM_DATA_OUT;
    if (0 == strcmp(input, "data_stream_in"))       *domain_index = PLATFORM_DATA_STREAM_IN; /* with sampling rate */
    if (0 == strcmp(input, "data_stream_out"))      *domain_index = PLATFORM_DATA_STREAM_OUT;
    if (0 == strcmp(input, "audio_in"))             *domain_index = PLATFORM_AUDIO_IN; /* for PDM, I2S, ADC */
    if (0 == strcmp(input, "audio_out"))            *domain_index = PLATFORM_AUDIO_OUT;
    if (0 == strcmp(input, "gpio_in"))              *domain_index = PLATFORM_GPIO_IN; /* extra data for BSP (delay, edge, HiZ, debouncing, analog mix..) */
    if (0 == strcmp(input, "gpio_out"))             *domain_index = PLATFORM_GPIO_OUT;
    if (0 == strcmp(input, "motion_in"))            *domain_index = PLATFORM_MOTION_IN;
    if (0 == strcmp(input, "2d_in"))                *domain_index = PLATFORM_2D_IN; /* control of AGC, zoom in 1/4 image area */
    if (0 == strcmp(input, "2d_out"))               *domain_index = PLATFORM_2D_OUT;
    if (0 == strcmp(input, "user_interface_in"))    *domain_index = PLATFORM_USER_INTERFACE_IN; 
    if (0 == strcmp(input, "user_interface_out"))   *domain_index = PLATFORM_USER_INTERFACE_OUT;
    if (0 == strcmp(input, "command_in"))           *domain_index = PLATFORM_COMMAND_IN; /* USB/UART */
    if (0 == strcmp(input, "command_out"))          *domain_index = PLATFORM_COMMAND_OUT;
    if (0 == strcmp(input, "analog_sensor"))        *domain_index = PLATFORM_ANALOG_SENSOR; /* sensor with aging control */
    if (0 == strcmp(input, "analog_transducer"))    *domain_index = PLATFORM_ANALOG_TRANSDUCER;
    if (0 == strcmp(input, "rtc_in"))               *domain_index = PLATFORM_RTC_IN; /* ticks from clocks */
    if (0 == strcmp(input, "rtc_out"))              *domain_index = PLATFORM_RTC_OUT;
    if (0 == strcmp(input, "storage_out"))          *domain_index = PLATFORM_STORAGE_OUT; /* periodic dump of captured data */
    if (0 == strcmp(input, "av_codec"))             *domain_index = PLATFORM_AV_CODEC; /* encoded audio/image/video */
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
    read_data_v(pt_line, &(pt->channel_format), 0, 0);

    /* skip the RFC8428 unit description */
    while (*(*pt_line) == ';')   
    {   jump2next_line(pt_line);
    }

    /* IMU options */
    read_common_data_options(pt_line, &(pt->acc_sensitivity));
    read_data_v(pt_line, &(pt->acc_scaling), 0, 0);
    read_common_data_options(pt_line, &(pt->acc_averaging));

    read_common_data_options(pt_line, &(pt->gyro_sensitivity));
    read_data_v(pt_line, &(pt->gyro_scaling), 0, 0);
    read_common_data_options(pt_line, &(pt->gyro_averaging));

    read_common_data_options(pt_line, &(pt->mag_sensitivity));
    read_data_v(pt_line, &(pt->mag_scaling), 0, 0);
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
    char line[MAXNBCHAR_LINE];

    /* digital audio multichannel stream format  */
    read_data_v(pt_line, line, 0, 0);
    decode_audio_channels(pt->bitFieldChannel, line);

    /* skip the RFC8428 unit description */
    while (*(*pt_line) == ';')
    {   jump2next_line(pt_line);
    }

   /* digital scaling from default sensitivity levels */
    read_data_v(pt_line, &(pt->audio_scaling), 0, 0);

    /* analog gain setting */
    read_common_data_options(pt_line, &(pt->analog_gain));
    read_common_data_options(pt_line, &(pt->digital_gain));
    read_common_data_options(pt_line, &(pt->AGC));
    read_common_data_options(pt_line, &(pt->DC_filter));
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
    read_data_v(pt_line, &(pt->default_index), 0, 0);
    read_data_v(pt_line, (void *)&(pt->options), &(pt->raw_type), &(pt->nb_option));
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
    char* pt_line, cstring[NBCHAR_LINE];
    uint32_t nb_io_stream, iB;
    processor_memory_bank_t *ptm;
    struct stream_IO_interfaces *pts;
    struct arcStruct *pta;

#define _Processor 'P'
#define _IO_StreamManifest 'I'

    pt_line = inputFile;
    nb_io_stream = 0;

    while (NOT_YET_END_OF_FILE == jump2next_valid_line(&pt_line))
    {
        switch (*pt_line)
        {
        /* ============================================================= */
        case _Processor:
        {   int32_t iproc, ibank, iservices;
            uint8_t nservices;
            
            jump2next_line(&pt_line);
            read_data_v(&pt_line, &(platform->nb_processors), 0 ,0);

            for (iproc = 0; iproc < platform->nb_processors; iproc++)
            {                
                read_data_v(&pt_line, &(platform->processor[iproc].processorID), 0, 0);
                read_data_v(&pt_line, &(platform->processor[iproc].IamTheMainProcessor), 0, 0);
                read_data_v(&pt_line, &(platform->processor[iproc].architecture), 0, 0);
                read_data_v(&pt_line, &(platform->processor[iproc].nbMemoryBank_simple), 0, 0);
                read_data_v(&pt_line, &(platform->processor[iproc].nbMemoryBank_detailed), 0, 0);

                for (ibank = 0; ibank < platform->processor[iproc].nbMemoryBank_simple +
                    platform->processor[iproc].nbMemoryBank_detailed; ibank++)
                {
                    if (ibank < platform->processor[iproc].nbMemoryBank_simple)
                    {
                        iB = ibank;
                        ptm = &(platform->processor[iproc].membank_simple[iB]);
                    }
                    else
                    {
                        iB = ibank - platform->processor[iproc].nbMemoryBank_simple;
                        ptm = &(platform->processor[iproc].membank_detailed[iB]);
                    }

                    read_data_v(&pt_line, &(ptm->offsetID), 0, 0);
                    read_data_v(&pt_line, &(ptm->offset64b), 0, 0);
                    read_data_v(&pt_line, &(ptm->speed), 0, 0);
                    read_data_v(&pt_line, &(ptm->shareable), 0, 0);
                    read_data_v(&pt_line, &(ptm->data_access), 0, 0);
                    read_data_v(&pt_line, &(ptm->backup), 0, 0);
                    read_data_v(&pt_line, &(ptm->hwio), 0, 0);
                    read_data_v(&pt_line, &(ptm->flash), 0, 0);
                    read_data_v(&pt_line, &(ptm->base32), 0, 0);
                    read_data_v(&pt_line, &(ptm->size), 0, 0);
                }

                /* bit-field of extended library / services */
                read_data_v(&pt_line, &(nservices), 0, 0);

                for (iservices = 0; iservices < nservices; iservices++)
                {
                    uint8_t tmp;
                    read_data_v(&pt_line, &(tmp), 0, 0);
                    platform->processor[iproc].libraries_b += (1 << tmp);
                }
            }
            break; // case _Processor
        }
        /* ============================================================= */
        } // switch case
    } // while not end of file
    platform->nb_hwio_stream = nb_io_stream;
}

/** 
  @brief            read the platform manifest fields
  @param[in/out]    none
  @return           none
  @par              
  @remark
 */
void read_platform_io_stream_manifest(char* inputFile, struct stream_platform_manifest* platform)
{
    char* pt_line, cstring[NBCHAR_LINE];
    uint32_t nb_io_stream, iB;
    processor_memory_bank_t *ptm;
    struct stream_IO_interfaces *pts;
    struct arcStruct *pta;

#define _Processor 'P'
#define _IO_StreamManifest 'I'

    pt_line = inputFile;
    nb_io_stream = 0;

    while (NOT_YET_END_OF_FILE == jump2next_valid_line(&pt_line))
    {
        switch (*pt_line)
        {
        case _IO_StreamManifest:
        {   
            uint8_t iarc;

            jump2next_line(&pt_line);
            pts = &(platform->stream[nb_io_stream]);
            nb_io_stream++;
           
            read_data_v(&pt_line, (pts->IO_name), 0, 0);
            read_data_v(&pt_line, &(pts->nb_arc), 0, 0);

            for (iarc = 0; iarc < pts->nb_arc; iarc++)
            {   
                pta = &((pts->arc_flow)[iarc]);

                /* read standard digital format data */
                read_data_v(&pt_line, cstring, 0, 0);
                decode_domain(&(pta->domain), cstring);
                read_data_v(&pt_line, &(pta->rx0tx1), 0, 0);
                read_data_v(&pt_line, &(pta->setupTime), 0, 0);
                read_data_v(&pt_line, &(pta->set0_copy1), 0, 0);
                read_data_v(&pt_line, &(pta->extraCommandID), 0, 0);

                read_data_v(&pt_line, &(pta->commander0_servant1), 0, 0);
                read_data_v(&pt_line, &(pta->graphalloc0_bsp1), 0, 0);
                read_data_v(&pt_line, &(pta->sram0_hwdmaram1), 0, 0);
                read_data_v(&pt_line, &(pta->processorBitFieldAffinity), 0, 0);

                read_data_v(&pt_line, cstring, 0, 0);
                decode_rawtype(&(pta->raw), cstring);
                read_data_v(&pt_line, &(pta->timestp), 0, 0);
                read_data_v(&pt_line, &(pta->frameSizeFormat_0s_1sample), 0, 0);

                read_common_data_options(&pt_line, &(pta->interleaving_option));
                read_common_data_options(&pt_line, &(pta->nbchannel_option));
                read_common_data_options(&pt_line, &(pta->frame_size_option));
                read_common_data_options(&pt_line, &(pta->sampling_rate_option));

                read_data_v(&pt_line, &(pta->FS_accuracy), 0, 0); /* allowed error on the sampling rate, in percentage */

                /* read domain-specific digital format data */
                /* read domain-specific mixed-signal format data */
                switch (pta->domain)
                {
                case PLATFORM_MOTION_IN:
                    motion_in_specific(&pt_line, &(pts->U.imu));
                    break;
                case PLATFORM_AUDIO_IN:
                    audio_in_specific(&pt_line, &(pts->U.audio));
                    break;
                }
            } // for iarc

        break;
        } // case _IO_StreamManifest:
        /* ============================================================= */
        } // switch case
    } // while not end of file
    platform->nb_hwio_stream = nb_io_stream;
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

}

/**
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par             
                   
  @remark
 */
void arm_stream_read_manifests (struct stream_platform_manifest *platform, struct stream_node_manifest *all_nodes, char *all_files)
{
    char* pt_line;
    char file_name[MAXNBCHAR_LINE];
    char graph_platform_manifest_name[MAXNBCHAR_LINE];
    uint32_t nb_nodes, inode;
    char node_name[MAXNBCHAR_LINE];
    char root_directory[MAXNBCHAR_LINE];

    /*
        STEP 1 : read the file names : and the digital platform capabilities
    */
    pt_line = all_files;
    jump2next_valid_line(&pt_line);
    sscanf(pt_line, "%s", root_directory);        /* read the ROOT directory name */
    jump2next_line(&pt_line);

    jump2next_valid_line(&pt_line);
    sscanf(pt_line, "%s", graph_platform_manifest_name); /* read the platform_manifest name*/
    jump2next_line(&pt_line);
    strcpy(file_name, root_directory);
    strcat(file_name, graph_platform_manifest_name);

    read_input_file(file_name, inputFile);

    read_platform_digital_manifest(inputFile, platform);


    /*
        STEP 2 : loop on all the list of IO stream manifests
    */
    jump2next_valid_line(&pt_line);
    sscanf(pt_line, "%d", &nb_nodes);  /* read the number of nodes in this plaform */
    jump2next_line(&pt_line);

    if (nb_nodes > MAX_NB_NODES)
    {   fprintf(stderr, "too much nodes !"); exit(-4);
    }

    for (inode = 0; inode < nb_nodes; inode++)
    {
        jump2next_valid_line(&pt_line);
        sscanf (pt_line, "%s", node_name); /* read the number of node's manifest name */
        jump2next_line(&pt_line);
        strcpy(file_name, root_directory);
        strcat(file_name, node_name);

        read_input_file(file_name, inputFile);
        read_platform_io_stream_manifest(inputFile, &(all_nodes[inode]));
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

    for (inode = 0; inode < nb_nodes; inode++)
    {
        jump2next_valid_line(&pt_line);
        sscanf (pt_line, "%s", node_name); /* read the number of node's manifest name */
        jump2next_line(&pt_line);
        strcpy(file_name, root_directory);
        strcat(file_name, node_name);

        read_input_file(file_name, inputFile);
        read_node_manifest(inputFile, &(all_nodes[inode]));
    }
}


#ifdef __cplusplus
}
#endif


//jump2next_line(&pt_line);
//read_data_v(&pt_line, &(platform->nb_processors));
//{
//uint16_t i16;
//uint32_t i32;
//uint64_t i64;
//float f32;
//double f64;
//char stringt[120];
////1    c;  TEST1            test1   FOR TEST
////1   f64;   3.14159265432   test2   FOR TEST
////1 i64; 05314573914857934875   test3   FOR TEST
////1 i16; 33768           test4   FOR TEST
////1 f64; 3.14159265432     test5   FOR TEST
////1 h32; ABCD5678         test6   FOR TEST
////1 i16; -12     test   FOR TEST
//read_data_v(&pt_line, stringt);
//read_data_v(&pt_line, &f64);
//read_data_v(&pt_line, &i64);
//read_data_v(&pt_line, &i16);
//read_data_v(&pt_line, &f64);
//read_data_v(&pt_line, &i32);
//read_data_v(&pt_line, &i16);
//i16 = 0;
//            }