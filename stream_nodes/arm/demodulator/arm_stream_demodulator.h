/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_demodulator.h
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
   

#ifndef cARM_STREAM_DEMODULATOR_H
#define cARM_STREAM_DEMODULATOR_H


#include "stream_common_const.h"      
#include "stream_common_types.h"  


typedef struct
{
    int *stream_entry;

} arm_stream_demodulator_instance;



extern void arm_stream_demodulator_process1 (arm_stream_demodulator_instance *instance, 
    void *input, void *output, intptr_t*nsamp);
extern void arm_stream_demodulator_process2 (arm_stream_demodulator_instance *instance, 
    void *input, void *output, intptr_t*nsamp);
extern void arm_stream_demodulator_process3 (arm_stream_demodulator_instance *instance, 
    void *input, void *output, intptr_t *nsamp);



#endif  //cARM_STREAM_DEMODULATOR_H

#ifdef __cplusplus
}
#endif
 