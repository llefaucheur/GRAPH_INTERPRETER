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
