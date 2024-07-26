/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_decompressor.h
 * Description:  filters
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
   

#ifndef cARM_STREAM_DECOMPRESSOR_H
#define cARM_STREAM_DECOMPRESSOR_H


#include "stream_const.h"      
#include "stream_types.h"  
#include "arm_stream_decompressor_imadpcm.h"

#define DECODER_IMADPCM          1  
#define DECODER_LPC              2  
#define DECODER_MIDI             3  
#define DECODER_CVSD             4  
#define DECODER_LC3              5  
#define DECODER_SBC              6  
#define DECODER_MSBC             7  
#define DECODER_OPUS_SILK        8  
#define DECODER_MP3              9  
#define DECODER_MPEG4_AACPLUS_V2 10  
#define DECODER_OPUS_CELT        11 
#define DECODER_JPEG             12 


#define STATE_DEFAULT_SIZE 4                    /* node_malloc_E defines the effective size, depending on presets/Codec selection  */

typedef struct
{
    /* memory area reserved for Codec internal computations */
    uint32_t *TCM;

    stream_al_services *stream_service_entry;
    uint32_t output_format[STREAM_FORMAT_SIZE_W32];

    uint32_t state[STATE_DEFAULT_SIZE];      /* LAST FIELD OF THE DECLARATION !! */
} arm_stream_decompressor_instance;



extern void decode_imadpcm(int32_t *state, uint8_t* input, uint32_t numSamples, int16_t* output);

#endif //cARM_STREAM_DECOMPRESSOR_H

#ifdef __cplusplus
}
#endif
 