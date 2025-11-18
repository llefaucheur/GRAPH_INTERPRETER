/* ----------------------------------------------------------------------
 * Title:        sigp_stream_decompressor.h
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
   

#ifndef csigp_STREAM_DECOMPRESSOR_H
#define csigp_STREAM_DECOMPRESSOR_H


//#include "stream_const.h"      
//#include "stream_types.h"  
#include "sigp_stream_decompressor_imadpcm.h"

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


#define STATE_generic_SIZE 4                    /* node_malloc_add  defines the effective size, depending on presets/Codec selection  */

typedef struct
{
    /* memory area reserved for Codec internal computations */
    uint32_t *TCM;

    stream_services *stream_service_entry;
    uint32_t output_format[STREAM_FORMAT_SIZE_W32];

    /* LAST FIELD OF THE DECLARATION,  to let it grow without changing the "*TCM" field 
    
        state = decoder state + memory state  of the filters
            pause
    */
    uint8_t decoder_state; 
    uint32_t memory_state[STATE_generic_SIZE];      

} sigp_stream_decompressor_instance;

#define STATE_RESET 1
#define STATE_RUN 2
#define STATE_PAUSE 3
#define STATE_FAST_FORWARD2 4  // play speed x 2
#define STATE_FAST_FORWARD4 5  // play speed x 4


extern void decode_imadpcm(int32_t *state, uint8_t* input, uint32_t numSamples, int16_t* output, uint8_t decoder_state);

#endif //csigp_STREAM_DECOMPRESSOR_H

#ifdef __cplusplus
}
#endif
 