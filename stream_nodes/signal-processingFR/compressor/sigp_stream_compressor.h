/* ----------------------------------------------------------------------
 * Title:        sigp_stream_compressor.h
 * Description:  filters
 *
 * $Date:        15 February 2024
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2013-2024 signal-processing.fr. All rights reserved.
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
#include "platform.h"

#ifdef CODE_SIGP_STREAM_COMPRESSOR

#ifndef CSIGP_STREAM_COMPRESSOR_H
#define CSIGP_STREAM_COMPRESSOR_H


#include "stream_const.h"      
#include "stream_types.h"  
#include "sigp_stream_compressor_imadpcm.h"

#define ENCODER_IMADPCM          1  
#define ENCODER_LPC              2  
#define __CODER_____             3  
#define ENCODER_CVSD             4  
#define ENCODER_LC3              5  
#define ENCODER_SBC              6  
#define ENCODER_MSBC             7  
#define ENCODER_OPUS_SILK        8  
#define ENCODER_MP3              9  
#define ENCODER_MPEG4_AACPLUS_V2 10  
#define ENCODER_OPUS_CELT        11 
#define ENCODER_JPEG             12 


#define STATE_DEFAULT_SIZE 4                    /* node_malloc_E defines the effective size, depending on presets/Codec selection  */

typedef struct
{
    /* memory area reserved for Codec internal computations */
    uint32_t *TCM;

    stream_al_services *stream_service_entry;
    uint32_t output_format[STREAM_FORMAT_SIZE_W32];

    uint32_t state[STATE_DEFAULT_SIZE];      /* LAST FIELD OF THE DECLARATION !! */
} sigp_stream_compressor_instance;


extern void encode_imadpcm(uint32_t *state, int16_t* input, uint32_t numSamples, uint8_t* output);

#endif

#endif //CODE_SIGP_STREAM_COMPRESSOR
#ifdef __cplusplus
}
#endif
 