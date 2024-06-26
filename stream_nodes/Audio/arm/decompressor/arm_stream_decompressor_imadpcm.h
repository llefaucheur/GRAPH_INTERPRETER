/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_dec_imadpcm.h
 * 
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

#ifndef codec_h
#define codec_h

#ifndef cARM_STREAM_DEC_IMADPCM_H
#define cARM_STREAM_DEC_IMADPCM_H


#include <stdint.h>


typedef struct 
{
	int valprev;
	int index;
} CodecState;

extern void decode_imadpcm(CodecState* state, uint8_t* input, uint32_t numSamples, int16_t* output);

#endif

#endif
