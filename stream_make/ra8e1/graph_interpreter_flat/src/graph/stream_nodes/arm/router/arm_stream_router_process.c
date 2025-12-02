/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_router_process.c
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

#include "presets.h"
#ifdef CODE_ARM_STREAM_ROUTER

#ifdef __cplusplus
 extern "C" {
#endif
   


#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_router.h"


/**
  @brief         Processing function 
  @param[in]     S         points to an instance of the Biquad cascade structure
  @param[in]     in_out    XDM buffer
  @return        none
  
  Initialization : 
    FIFO descriptors : arcID, chanID, Ftime, 
        Interleaving and data format details, 
        Sampling period SrcSP (or estimate), DstSP
  
  check all input and output arcs fit for the time granularity of processing "DT"
    compute "nDT", the number processing loop with DT samples
    check if there is an HQOS arc (and its corresponding output arc)
        OLA removal of samples on the other arcs, or duplicate last samples

  Loop on nDT
    update FIFOs

    update MIXERs
        align the FIFO time (rounding to closest) before the mixing

    update OUTPUT arcs and adapt the sample format
        realign the time reference of input arcs to the output (=0)

  endLoop

  update XDM descriptor with the amount of samples consumed and generated per arc


    copy to deinterleaved data scratch buffers  amount decided by HQOS
  convert to fp32 and extrapolate missing samples for other arcs
  resample / asrc
  
  mix(A(i) x input)i)) => A(j) x output(j)
      resample
      Raw convcert
      interleaved stride
  
  shift scratch buffer to next interpolations

  while all input arcs


 */
void arm_stream_router_process (arm_stream_router_instance *instance, stream_xdmbuffer_t *in_out)
{

}

#ifdef __cplusplus
}
#endif
 
#endif // CODE_ARM_STREAM_router