/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_format_converter.h
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

#ifndef Carm_stream_format_converter_H
#define Carm_stream_format_converter_H

#include <stdint.h>

#include "stream_common_types.h"  

typedef struct
{   uint8_t raw_fmt;
    uint8_t interleaving_fmt;
    uint8_t timestamp_fmt;
    uint8_t nchan_fmt;
    uint32_t framesize_fmt;
    sfloat sampling_rate;
} arm_stream_format_converter_format;

/*
    converter INSTANCE :
*/
typedef struct
{
    stream_al_services *stream_entry;  /* calls to computing services */
    arm_stream_format_converter_format input, output;

} arm_stream_format_converter_instance;


extern void arm_stream_format_converter_process (arm_stream_format_converter_format *in, arm_stream_format_converter_format *out, stream_xdmbuffer_t *in_out);

#endif

#ifdef __cplusplus
}
#endif
 