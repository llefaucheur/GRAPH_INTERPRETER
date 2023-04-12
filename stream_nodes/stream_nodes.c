/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
 * Description:  
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

  
#include "stream_const.h"
#include "stream_types.h"


/* -----------------------------------
    Nodes are delivered source-code
      or binary with code generation made for “position independent execution”. 
      This is the reason for having standard C library access through LINK.
    https://developer.arm.com/search#q=Position%20Independent%20Executables&sort=relevancy
    https://en.wikipedia.org/wiki/Position-independent_code
--------------------------------------
*/

/* 

0. arm_script
Operation : interpreted byte-code.
Parameters : none 

1.	stream_split
Operation : deinterleave an input arc to several output arcs (mono). Split manages up to 8 output arcs.
Parameters : none (stream details are shared during the reset sequence)

2.	stream_merge
Operation : receives several mono or multichannel streams and produces one interleaved output arc. 
Merge manages up to 8 input arcs. One input arc can be tagged as “High QoS” and manage with priority : 
whatever the data content on the other arcs data is flowing synchronized by this one. 
The other arcs are using interpolations scheme, defined in the detailed YAML file format. 
Parameters : name of the High QoS arc or “none”

3.	stream_route
Operation : copy the content of one stream to another stream. One input stream can be replicated to 
several output. Route can manage up to 8 input and 8 output arcs.
Parameters : list of input arc names and corresponding output arc names (or “none” is unused).

4.	stream_format_converter
Operation : converter raw data format and interleaving to the format of the output arc. 
Parameters : none (stream format details are shared during the reset sequence)

5.	stream_rate_converter
Operation : interpolate the input stream format to the output stream format. For example for the 
rescaling of images, the audio sampling-rate conversion, the interpolation of an asynchronous stream, 
and time-stamped, of samples from a motion sensor to a synchronous data rate.
Parameters : drift coefficient (1+/- eps) (stream format details are shared during the reset sequence)

6.	stream_amplitude
Operation : control of the amplitude of the input stream with controlled time of ramp-up and ramp-down. 
The gain control “mute” is used to store the current gain setting, being reloaded when the command “unmute” is sent
Parameters :  new gain/mute/unmute, ramp-up/down slope, delay before starting the slope.

7.	stream_mixer
Operation : receives several mono or multichannel streams and produces several mixed output identical arcs. 
Mixer manages up to 8 input arcs. One input arc can be tagged as “High QoS” and manage with priority : 
whatever the data content on the other arcs data is flowing synchronized by this one. 
The other arcs are using interpolations scheme, defined in the detailed YAML file format. 
Parameters : input arcs names, their associated gain and time-constant in [ms], name of the 
output arc and an extra mixing gain, name of the High QoS arc, or “none”

8.	stream_dsp_filter
Operation : receives one multichannel stream and produces one filtered multichannel stream. 
Filtering is identical on each channel.
Parameters : number of biquad filters used in cascade, filter coefficients
    or let SET_PARAM compute the coefficients https://www.w3.org/TR/audio-eq-cookbook/

9.	stream_detector
Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
A tunable delay allows to maintain the boolean value for a minimum amount of time 
for debouncing and LED / user-interface).
Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
time-constant in [ms] for the energy integration time, time-constant to gate the output.

10.	stream_codec, stream_decompression
Operation : simple wave compression using Alaw (8bits/sample), IMADPCM(4bits/sample) and 
LPC10(1bit/sample) de/encoding.
Parameters : coding scheme 

11.	data conditioning
Operation : applies y = f(x), where f = c.(H(a.G(x) + b) + d, 
  with H and G being either x^2, sqrt, x, log10, pow10, 1/x
Parameters : all 

12.	signal generator
Operation : sine, noise, sweep, square
Parameters : type of wave, amplitude,   

*/
extern p_stream_node arm_script; 
extern p_stream_node arm_stream_split; 
extern p_stream_node arm_stream_merge; 
extern p_stream_node arm_stream_route; 
extern p_stream_node arm_stream_converter; 
extern p_stream_node arm_stream_rate_converter; 
extern p_stream_node arm_stream_amplitude; 
extern p_stream_node arm_stream_mixer; 
extern p_stream_node arm_stream_dsp_filter; 
extern p_stream_node arm_stream_detector; 
extern p_stream_node arm_stream_codec; 
extern p_stream_node arm_stream_decompression; 
extern p_stream_node arm_data_conditioning;
extern p_stream_node arm_stream_generator;
//extern p_stream_node arm_stream_classifier;

#define TBD 0

p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS] =
{
    /*--------- ARM ---------*/
    /*  0*/ (void *)&arm_script, 
    /*  1*/ (void *)&arm_stream_split,                        
    /*  2*/ (void *)&arm_stream_merge,                        
    /*  3*/ (void *)&arm_stream_route, 
    /*  4*/ (void *)&arm_stream_converter,                        
    /*  5*/ (void *)&arm_stream_rate_converter,                        
    /*  6*/ (void *)&arm_stream_amplitude,                        
    /*  7*/ (void *)&arm_stream_mixer, 
    /*  8*/ (void *)&arm_stream_dsp_filter,                        
    /*  9*/ (void *)&arm_stream_detector,                        
    /* 10*/ (void *)&arm_stream_codec,                        
    /* 11*/ (void *)&arm_data_conditioning,  
    /* 12*/ (void *)&arm_stream_generator, 
    /* 13*/ TBD, //(void *)&arm_stream_classifier, 
    /* 14*/ TBD, 
    /* 15*/ TBD, 

    /*------ COMPANY #1 -----*/
    TBD,                           
    /*------ COMPANY #2 -----*/
    TBD,TBD,TBD,
    /*------ COMPANY #3, etc ... -----*/
                
};
 
