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

#ifdef __cplusplus
 extern "C" {
#endif
   

#ifndef cARM_STREAM_ROUTER_H
#define cARM_STREAM_ROUTER_H


#include "stream_const.h"      
#include "stream_types.h"  



//; parameters arranged to be accessed with 32bits data
//  2  i8; 2 2          nb input/output arcs
//  2  i8; -1 -1        no HQoS arc on input and output
//  ;
//  ;     arcin ichan arcout ichan 
//  4  i8; 0     0     2     0     ; move arc0-left  to arc2 mono x0.25
//  2 f32: 0.25  0.1				 ;    gain and convergence speed 
//  4  i8; 0     1     2     0     ; move arc0-right to arc2 mono x0.25
//  2 f32: 0.25  0.1
//  4  i8; 1     0     2     0     ; move arc1-left  to arc2 mono x0.25
//  2 f32: 0.25  0.1 
//  4  i8; 1     1     2     0     ; move arc1-right to arc2 mono x0.25
//  2 f32: 0.25  0.1 
//  4  i8; 0     0     3     0     ; move arc0-left  to arc3 left no mixing
//  2 f32: 0.25  0.1 
//  4  i8; 1     1     3     1     ; move arc1-right to arc3 right no mixing
//  2 f32: 0.25  0.1 

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
    uint32_t frame_size;
    uint32_t raw_stride;
    float_t input_time;
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

    stream_al_services *stream_entry;

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

#ifdef __cplusplus
}
#endif
 
#if 0
#include <stdio.h>
#include <stdint.h>
#include <math.h>

float minifloat_to_float(uint8_t mf) {
    // Extract bits
    int sign = (mf >> 7) & 0x01;
    int exp = (mf >> 3) & 0x0F;
    int mantissa = mf & 0x07; // 3 bits mantissa

    float value;
    int bias = 7; // Exponent bias = 2^(4-1) - 1

    if (exp == 0) {
        if (mantissa == 0) {
            // Zero
            value = 0.0f;
        } else {
            // Denormalized number
            float frac = mantissa / 8.0f; // 3 mantissa bits
            value = powf(2, 1 - bias) * frac;
        }
    } else if (exp == 0x0F) {
        // Special cases (Inf or NaN)
        if (mantissa == 0) {
            value = INFINITY;
        } else {
            value = NAN;
        }
    } else {
        // Normalized number
        float frac = 1 + (mantissa / 8.0f);
        value = powf(2, exp - bias) * frac;
    }

    // Apply sign
    if (sign)
        value = -value;

    return value;
}

int main() {
    // Example: Test with minifloat 0x4B
    uint8_t minifloat = 0x4B; // 0b01001011 example
    float result = minifloat_to_float(minifloat);
    printf("Minifloat 0x%02X translates to float: %f\n", minifloat, result);

    return 0;
}

#endif