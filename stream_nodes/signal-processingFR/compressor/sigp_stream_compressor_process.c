/* ----------------------------------------------------------------------
 * Title:        sigp_stream_compressor_process.c
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
    
#include "platform.h"
#ifdef CODE_SIGP_STREAM_COMPRESSOR
   

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "sigp_stream_compressor.h"


/**
  @brief         
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void sigp_stream_compressor_process (sigp_stream_compressor_instance *instance, void *input, void *output, intptr_t *nsamp)
{


}

#ifdef __cplusplus
}
#endif

#endif //CODE_SIGP_STREAM_COMPRESSOR
 