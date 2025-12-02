/* ----------------------------------------------------------------------
 * Title:        sigp_stream_converter.h
 * Description:  resampler
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
   

#ifndef CSIGP_STREAM_RESAMPLER_H
#define CSIGP_STREAM_RESAMPLER_H




/*
    converter INSTANCE :

*/
typedef struct
{
    stream_services *stream_entry;  /* calls to computing services */
    uint32_t configuration;
} sigp_stream_resampler_instance;


#endif      //Csigp_STREAM_RESAMPLER_H

#ifdef __cplusplus
}
#endif
 