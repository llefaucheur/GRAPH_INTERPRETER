/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_analysis_process.c
 * Description:  qos
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
   
#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_analysis.h"


/**
  @brief         
  @param[in]     pinst      instance of the component : format of channels + mixing parameters
  @param[in/out] pdata      address and size of buffers
  @return        status     finalized processing
 */
void arm_stream_analysis_process (arm_analysis_instance *instance, stream_xdmbuffer_t *xdm_buf, uint8_t narc)
{

}
#ifdef __cplusplus
}
#endif
 