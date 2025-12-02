/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_router.h
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

#ifndef cARM_STREAM_ROUTER_H
#define cARM_STREAM_ROUTER_H


#include "stream_const.h"      
#include "stream_types.h"  



// ;----------------------------------------------------------------------
// ;                               %%%%%%%%%%%%%%%%%%%%%                         
// ;                    Stereo     % F0,F1  %%%%%%%%%  % Mono, sum of arcs data {Left arc0 + Right arc0} 
// ; Format 0 16kHz    %arc 0%%%%%%% F2     %  +    <%%%<%%arc 2%%%%%%%%%% Format 1, 48kHz, fp32
// ;                               %        %%%%%%%%%  % 
// ;                    Stereo     %                   % Stereo Left(arc0), Right(arc1)
// ; Format 0 16kHz    %arc 1%%%%%%% F3   %%%%%%%%%%%%%<%%arc 3%%%%%%%%%% Format 2, 32kHz, s16     
// ;                               %%%%%%%%%%%%%%%%%%%%%                                    
// ;----------------------------------------------------------------------
// node arm_stream_router  0 
//     node_preset         2           ; 0:just interleaving, 1:just format conversion, 2:with ASRC, 3:with SSRC
//     node_malloc_add     60 1        ; Adds 3 FIFO (20 bytes each) on Segment-1 (default = 1 FIFO)
//     node_parameters     0           ; TAG = "all parameters"
// ;
//         1  u8;  2                   ; nb input arcs
//         1  u8;  2                   ; nb output arcs
//         1  u8;  5                   ; nb intermediate FIFO
//         1  u8;  1                   ; nb mixers (used for arc 2 {Left arc0 + Right arc0})
//         1  u8;  3                   ; nb output channels
//         1  u8;  255                 ; arc used with HQOS: none
//         1  u8;  1                   ; accuracy level (1..4)
//         1 f32;  0.0002              ; 0.2ms processing granularity (3.2 @16k, 6.4 @32k, 9.6 @48kHz)
// ;
//         3  u8;  0 0  2              ; FIFO 0  Left arc0 to arc 2 (for Mixer 0)
//         3  u8;  0 0  3              ; FIFO 1  Left arc0 to arc 3
//         3  u8;  0 1  2              ; FIFO 2  Righ arc0 to arc 2 (for Mixer 0)
//         3  u8;  1 0  1              ; FIFO 3  Righ arc1 to arc 3 
//         3  u8;  0 0  2              ; FIFO 4  Mixer output to arc 2
// ;
//         4  u8;  4 0 2 255           ; MIXER 0 (FIFO 4) = FIFO 0 + FIFO 2 + (none)
//         4 f32;  0.5 0.5 0.0 1.0     ;           Gmix0    Gmix1 (none)  GmixOut
// ;
//         3  u8;  4 0 0               ; OUT 0 FIFO 4 to DST[arc, chan]  
//         3  u8;  1 3 0               ; OUT 1 FIFO 1 to DST[arc, chan]    
//         3  u8;  3 3 1               ; OUT 2 FIFO 3 to DST[arc, chan]    
//     end
// ;----------------------------------------------------------------------

//typedef struct
//{
//    #define MAXNBINPUTMIXER 4                     // each output channel results from a mix of several input streams
//    uint8_t in[MAXNBINPUTMIXER];                  // selection of the router_in_param[]
//
//    uint32_t consumption_index[MAXNBINPUTMIXER];  // input streams are consumed at different rates by the output mixers
//    //float_t input_gain[MAXNBINPUTMIXER];         // linear gain applied on mixer input
//
//    //float_t gain_update_slope_s;                 // ramp-up round(1./abs(log10(1-1./2.^[0:15])')/Fs
//    //float_t gain_update_delay_s;                 // time in seconds before gain rises
//
//} router_out_param;



/*
    input streams control parameters :
*/
typedef struct
{
    // per channel
    uint32_t frame_size;
    uint32_t raw_stride;

    // global
    float_t time_period;                // 
    float_t input_time;                 // 
    float_t next_xxput_time;            // 
    uint8_t raw_format_conversion;      // switch case of th conversion
    uint8_t time_stamp_type;            // 0 or STREAM_TIME(D)xx

} router_param;


/*
    ROUTER INSTANCE :
        pointer to the scheduler list of services
        configuration bit-field (nb arcs)
        list of routing indexes
*/
typedef struct
{
    uint32_t frame_size;
    float_t sampling_rate;
    uint32_t raw_stride;
    uint8_t raw_format;

    stream_services *stream_entry;

    uint8_t hqos_in;                                // low-latency arc indexes, >128 means not applicable
    uint8_t hqos_out;

    /*  The node has 4 inputs and 4 output arcs
        each can have multichannel stream
    */
    #define MAXROUTERINOUT 8                        // total of multichannel streams x number of arcs
    #define MAXNBROUTERIN (MAXROUTERINOUT/2)        // 4 + 4 arcs
    #define MAXNBROUTEROUT (MAXROUTERINOUT/2)

    router_param in_param[MAXNBROUTERIN];
    router_param out_param[MAXNBROUTEROUT];

    #define NBPASTSAMPLES 4                         // polynomial order
    float_t past_samples[NBPASTSAMPLES];             // samples used for the rate conversion (polynomial)
    float_t output_gain;                             // linear gain applied on mixer output

} arm_stream_router_instance;

extern void arm_stream_router_process (arm_stream_router_instance *instance, stream_xdmbuffer_t *in_out);

#endif

