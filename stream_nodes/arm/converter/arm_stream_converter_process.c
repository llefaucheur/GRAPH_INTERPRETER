/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_converter_process.c
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
   



#include "stream_const.h"
#include "stream_types.h"
#include "arm_stream_converter.h"


/*
   swc_run - "the" subroutine  ------------------------------------------
*/

#define FREE_MEMORY 1
#define REGISTER_ME 2


/**
  @brief         Processing function for _______________
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
int32_t arm_stream_converter_calls_stream (int32_t command, arm_stream_converter_instance *instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    stream_services_entry *stream_services; /* function pointer used for debug/trace, memory move and free, signal processing */
    arm_stream_converter_instance *pinstance = (arm_stream_converter_instance *)instance;

    switch (command)
    {   
        //case STREAM_SERVICE_INTERNAL_NODE_REGISTER: first command to register the SWC
        //case STREAM_SERVICE_INTERNAL_DEBUG_TRACE:
        //{  
        //}
        default: 
            return 0;
    }
    stream_services = pinstance->stream_entry;
    /* arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3) */
    //stream_services (0, STREAM_SERVICE_CONVERSION_INT16_FP32, 0, 0);  /* single interface for controls ! */
    return 0;
}




/**
  @brief         Processing function 
  @param[in]     in_out    XDM buffer
  @return        none
 */

/*           pt_pt = data;
             raw format converter processing  (pinstance->input, pinstance->output, pt_pt);
*/
void arm_stream_converter_process (arm_stream_converter_format *in, arm_stream_converter_format *out, stream_xdmbuffer_t *in_out)
{


}

#ifdef __cplusplus
}
#endif
 