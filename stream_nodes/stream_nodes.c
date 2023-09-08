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

#ifdef __cplusplus
 extern "C" {
#endif
   
    

#include "platform_computer.h"
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

1. arm_script
    Operation : interpreter of byte-codes
    Parameters : address of the byte-codes

2.	arm_stream_router
    I/O streams have the same RAW format size, if time-stamped then only with frame size of 1
    If there is one High-QoS input arc then there is also one High-QoS output arc
    1) find the frame size from the High-QoS I/O stream, or find a common denominator of all
    2) for each output stream: find *in *out m n p q , to do: out(m.i +n) = in(p.i +q)

    stream_route
    Operation : copy the content of one stream to another stream. One input stream can be replicated to 
    several output. Route can manage up to 8 input and 8 output arcs.
    Parameters : list of input arc names and corresponding output arc names (or “none” is unused).

    stream_split / demux
    Operation : deinterleave an input arc to several output arcs (mono). Split manages up to 8 output arcs.
    Parameters : none (stream details are shared during the reset sequence)

    stream_merge / mux
    Operation : receives several mono or multichannel streams and produces one interleaved output arc. 
    Merge manages up to 8 input arcs. One input arc can be tagged as “High QoS” and manage with priority : 
    whatever the data content on the other arcs data is flowing synchronized by this one. 
    The other arcs are using interpolations scheme, defined in the detailed YAML file format. 
    Parameters : name of the High QoS arc or “none”

3.	arm_stream_converter
    Operation : converter raw data format to the format of the output arc. 
    Parameters : Preset give the operations to do 

4.	arm_stream_amplifier
    Operation : control of the amplitude of the input stream with controlled time of ramp-up and ramp-down. 
    The gain control “mute” is used to store the current gain setting, being reloaded when the command “unmute” is sent
    Parameters :  new gain/mute/unmute, ramp-up/down slope, delay before starting the slope.

5.	arm_stream_mixer
    Operation : receives several mono or multichannel streams and produces several mixed output identical arcs. 
    Mixer manages up to 8 input arcs. One input arc can be tagged as “High QoS” and manage with priority : 
    whatever the data content on the other arcs data is flowing synchronized by this one. 
    The other arcs are using interpolations scheme, defined in the detailed YAML file format. 
    Parameters : input arcs names, their associated gain and time-constant in [ms], name of the 
    output arc and an extra mixing gain, name of the High QoS arc, or “none”

6.	arm_stream_filter
    Operation : receives one multichannel stream and produces one filtered multichannel stream. 
    Filtering is identical on each channel. GUI parameters are given in float, implementation 
    is Q15-DF1 with error spectral shaping, optimized for code size and ARM-v6
    Parameters : number of biquad filters used in cascade, filter coefficients
        or let SET_PARAM compute the coefficients https://www.w3.org/TR/audio-eq-cookbook/
    Presets : Bandwidth + Fc + (LPF, HPF, BPF), and a DC offset canceller (Fc=0)

7.	arm_stream_detector
    Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
    a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time 
    for debouncing and LED / user-interface).
    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.

8.	arm_stream_compressor
    Operation : wave compression using IMADPCM(4bits/sample) and LPC(<<1bit/sample) de/encoding.
    Parameters : coding scheme 

9.	arm_stream_data_rescaler
    Operation : applies vq = interp1(x,v,xq)
    Following https://fr.mathworks.com/help/matlab/ref/interp1.html
    Parameters : X,V vectors

10.	arm_signal_generator
    Operation : sine, noise, sweep, square, modulated bit-stream (FSK, AM, QAM, vocoder)
    Parameters : type of wave, amplitude, VCO, phase of multichannel

11. arm_stream_serial_data
    Operation : decode a bit-stream from analog data. Use-case: IR decoder, UART on I2S audio
    Parameters : clock and parity setting or let the algorithm discover the frame setting after 
        some time. https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter

12.	arm_stream_demodulator
    Operation : input signal frequency estimator. Best-effort accuracy on time-stamped streams. Use of DC
    removal (randomized downsampling and simple accumulation), zero-crossing rate, second-order autoregression. 
    Use-case example: measure 50Hz from power lines on the three phases (a -1% deviation means "power-down").
    Parameters : number of average, length of the analysis buffer, offset removal, known range of the frequency
    to insert a band-pass filter, known drift/sweep characteristics of the input signal, the signal is square.

13.	arm_stream_interpolator
    Operation : interpolate the input stream format to the output stream format. For example for the 
    rescaling of images, the audio sampling-rate conversion, the interpolation of an asynchronous stream, 
    and time-stamped samples from a motion sensor to a synchronous data rate.
    Parameters : Preset give the operations to do : (a)synchronous rate conversion
    reuse of the arc format given in the parmeters at reset, and the filling state of the output buffer to 
    adjust the drift. 


*/
extern p_stream_node arm_script; 
extern p_stream_node arm_stream_router;       
extern p_stream_node arm_stream_converter;    
extern p_stream_node arm_stream_amplifier;    
extern p_stream_node arm_stream_mixer;        
extern p_stream_node arm_stream_filter;       
extern p_stream_node arm_stream_detector;     
extern p_stream_node arm_stream_rescaler;
extern p_stream_node arm_stream_compressor;   
extern p_stream_node arm_stream_decompressor;
extern p_stream_node arm_stream_modulator;    
extern p_stream_node arm_stream_demodulator;
extern p_stream_node arm_stream_interpolator;

#define TBD 0

/* 
    this table will be extended with pointers to nodes loaded 
    dynamically and compiled with position independent execution options
*/
p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS] =
{
    /*--------- ARM ---------*/
    /*  0*/ (void *)0,
    /*  1*/ (void *)&arm_script, 
    /*  2*/ (void *)&arm_stream_router,                       
    /*  3*/ (void *)&arm_stream_converter,                      
    /*  4*/ (void *)&arm_stream_amplifier,                
    /*  5*/ (void *)&arm_stream_mixer,                         
    /*  6*/ (void *)&arm_stream_filter,                             
    /*  7*/ (void *)&arm_stream_detector,                         
    /*  8*/ (void *)&arm_stream_rescaler,
    /*  9*/ (void *)&arm_stream_compressor,                    
    /* 10*/ (void *)&arm_stream_decompressor,
    /* 11*/ (void *)&arm_stream_modulator,        
    /* 12*/ (void *)&arm_stream_demodulator,
    /* 13*/ (void *)&arm_stream_interpolator,
    /* 14*/ TBD, 
    /* 15*/ TBD,

    /*------ COMPANY #1 -----*/
    TBD,                           
    /*------ COMPANY #2 -----*/
    TBD,TBD,TBD,
    /*------ COMPANY #3, etc ... -----*/
                
};
 
#ifdef __cplusplus
}
#endif
 