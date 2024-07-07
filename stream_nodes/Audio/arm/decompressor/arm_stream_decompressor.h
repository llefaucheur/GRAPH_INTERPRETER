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

typedef struct
{
    stream_al_services *stream_entry;
    CodecState* state;
} arm_stream_decompressor_instance;



extern void arm_stream_decompressor_process (arm_stream_decompressor_instance *instance, uint8_t *input, int16_t *output, uint32_t *nsamp);

#endif //cARM_STREAM_DECOMPRESSOR_H

#ifdef __cplusplus
}
#endif
 