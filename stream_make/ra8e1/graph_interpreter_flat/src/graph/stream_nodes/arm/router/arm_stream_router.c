/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_router.c
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

extern void arm_stream_router_process (arm_stream_router_instance *instance, stream_xdmbuffer_t *in_out);

/*
;----------------------------------------------------------------------------------------
;Node #2.	arm_stream_router

    Operation : this node receives up to 4 streams (arcs) and generate up to 4 stream, each can be multichannel. 
    The Format of the streams is known with the "reset and "set param" commands to the node. 

    Input streams are moved, routed and mixed to generate the output streams in a desired stream format. 
    The output stream are isochronous to the other graph streams (they have a known sampling rate), but the input can 
    be synchronous (same or different sampling rate) or asynchronous (each sample have a time-stamp).

    The node can be associated with a script (SCRIPT_LW0) in charge of HQoS arcs (High Quality of Service) and 
    synchronization between two arcs (for example I/V sensing stream synchronization)

    Operation:
    At reset time the node checks the conditions of operations (simple routing use-case, time-stamp format, time
        window of the processing, filters conputing for synchronous high-quality rate converter)
    The node starts checking all the input and output arcs in the time domain and use float intermediate format

**Parameters**

    The list of routing and mixing information is :

    - index of the input arc (<= 4)

    - number of channels to read (<= 31)
    - index of the channels (1 Byte to 31 Bytes)
    - index of the output destination arc (<=4)
    - number of channels to write (<= 31)
    - index of the channels (1 Byte to 31 Bytes) and mixer gain to apply fp16 (S.E5.M10 format)



    Example with a router with two stereo input arcs and two output arcs. The first output arc is a mono 
    sum of all the input channels, the second arc is stereo combining the two left channels of the input arcs.
    ```
                 ┌───────────────────┐                         
      Stereo     │      ┌─────────┐  │ Mono sum of all arcs data 
     ─arc 0─────►│      │  Mixer  ┼──┼─arc 2─────────►          
                 │      └─────────┘  │                         
      Stereo     │                   │ Stereo Left(arc0), Right(arc1)
     ─arc 1─────►│      ─────────────┼─arc 3─────────►          
                 └───────────────────┘                                    

        1  u8;  2                   ; nb lines
        1  u8;  0                   ; index of the arc used for its frame size as time reference
        1  u8;  1                   ; accuracy level
        1  u8;  0                   ; 4 bytes alignment padding

        ;  [arc, sub channel]  
        2  u8;  0 0                 ; buffer "0"  Arc0, Left 
        2  u8;  0 1                 ; buffer "1"  Arc0, right 
        2  u8;  1 0                 ; buffer "2"  Arc1, Left 
        2  u8;  1 1                 ; buffer "3"  Arc1, right
;
;   Left+Right fifo input, mixed+gainx2 to Right output (Fifo1)
        4  u8;  0 1 1 1  0  1 -1 0  ; SRC[arc, chan]  DST[arc, chan] ififo-mix1 imix2 imix3 padding
        4 f32;  0.5 0.5 0.0 2.0     ; Gmix1 Gmix2 Gmix3 GmixOut
    ; input "0"   Left input direct to Left output (Fifo0) no mixer
        4  u8;  0 0 1 0 -1 -1 -1 0  ; SRC[arc, chan]  DST[arc, chan] ififo-mix1 imix2 imix3 padding
        4 f32;  1.0 0.0 0.0 1.0     ; Gmix1 Gmix2 Gmix3 GmixOut
;
;   Left+Right fifo input, mixed+gainx2 to Right output (Fifo1)
        4  u8;  0 1 1 1  0  1 -1 0  ; SRC[arc, chan]  DST[arc, chan] ififo-mix1 imix2 imix3 padding
        4 f32;  0.5 0.5 0.0 2.0     ; Gmix1 Gmix2 Gmix3 GmixOut

      2  i8; 2 2          nb input/output arcs
      2  i8; -1 -1        no HQoS arc on input and output
      ;
      ;     arcin ichan arcout ichan 
      4  i8; 0     0     2     0     ; move arc0-left  to arc2 mono x0.25
      2 f32: 0.25  0.1				 ;    gain and convergence speed 
      4  i8; 0     1     2     0     ; move arc0-right to arc2 mono x0.25
      2 f32: 0.25  0.1
      4  i8; 1     0     2     0     ; move arc1-left  to arc2 mono x0.25
      2 f32: 0.25  0.1 
      4  i8; 1     1     2     0     ; move arc1-right to arc2 mono x0.25
      2 f32: 0.25  0.1 
      4  i8; 0     0     3     0     ; move arc0-left  to arc3 left no mixing
      2 f32: 0.25  0.1 
      4  i8; 1     1     3     1     ; move arc1-right to arc3 right no mixing
      2 f32: 0.25  0.1 

Operations :

    - when receiving the reset command: compute the time granularity for the processing, check if 
    bypass are possible (identical sampling rate on input and output arcs).

    - check all input and output arcs to know which is the amount of data (in the time domain) which 
    can me routed and split in "time granularity" chunks.
    - copy the input arcs data in internal FIFO in fp32 format, deinterleaved, with time-stamps 
    attached to each samples.
    - use Lagrange polynomial interpolation to resample the FIFO to the output rate. The interpolator is 
    preceded by a an adaptive low-pass filter removing high-frequency content when the estimated input 
    sampling rate higher than the output rate.

    Synchronization operation : arranges multiple input streams are delivered at the same time.
        Use-case: multi-channel audio stream, each channel is processed with independent
         signal processing feature. The problem is to avoid (on MP devices) one channel to
         be delivered to the final mixer ahead and desynchronized from the others.

    When and an arc is processed with HQoS the other arcs should have a larger buffer size to manage 
    jitter and interpolation.
;----------------------------------------------------------------------------------------
*/

/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_router (unsigned int command, void *instance, void *data, unsigned int *status)
{
    *status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                    => used by the router to know the size of each raw element 
                    (see stream_bitsize_of_raw())
                memory pointers are in the same order as described in the NODE manifest

                the number of arcs (NARC_CMD) is used to configure arm_stream_router_instance.configuration
        */
        case STREAM_RESET: 
        {   //stream_services *stream_entry = (stream_services *)data;
            //intptr_t *memresults = (intptr_t *)instance;
            //uint16_t preset = RD(command, PRESET_CMD);
            //arm_stream_router_instance *pinstance = (arm_stream_router_instance *) *memresults;
            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
                *instance, 
                data = parameters

        - when receiving the reset command: compute the time granularity for the processing, check if 
            bypass are possible (identical sampling rate on input and output arcs).

        - check all input and output arcs to know which is the amount of data (in the time domain) which 
            can me routed and split in "time granularity" chunks.
        */ 
        case STREAM_SET_PARAMETER:  
        {   uint8_t *pt8bsrc;
            //uint16_t *pt16bdst;
            //uint16_t *pt16bsrc;
            //uint16_t i;
            //uint16_t n;
            
            arm_stream_router_instance *pinstance = (arm_stream_router_instance *) instance;

            /* copy the parameters from preset or from the graph */
            pt8bsrc = (uint8_t *) data;

            //for (i = 0; i < n; i++)
            //{   pt16bdst[i] = pt16bsrc[i];
            //}
            break;
        }

        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input nbdata} {*output nbfree}]
        */         
        case STREAM_RUN:   
        {
            arm_stream_router_instance *pinstance = (arm_stream_router_instance *) instance;
            stream_xdmbuffer_t *pt_pt;

            //
            //
            //gather nb_input / output, frame size ..
            //check memrory allocation fits
            // 
            //copy to deinterleaved data scratch buffers  amount decided by HQOS 
            //convert to fp32 and extrapolate missing samples for other arcs
            //resample / asrc 
            //
            //mix(A(i) x input)i)) => A(j) x output(j) 
            //    resample
            //    Raw convcert
            //    interleaved stride
            //
            //shift scratch buffer to next interpolations

            //while all input arcs 



            pt_pt = data;
            arm_stream_router_process (pinstance, pt_pt);
            break;
        }

        default :
            break;
    }
}

#ifdef __cplusplus
}
#endif
 
#else
void arm_stream_router (unsigned int command, void *instance, void *data, unsigned int *status) { /* fake access */ if(command || instance || data || status) return;}
#endif  // #ifndef CODE_ARM_STREAM_router
