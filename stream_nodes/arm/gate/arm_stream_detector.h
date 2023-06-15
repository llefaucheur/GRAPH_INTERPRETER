/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_detector.h
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
   

#ifdef _MSC_VER 
#include "../../../stream_al/platform_computer.h"
#include "../../../stream_src/stream_const.h"      
#include "../../../stream_src/stream_types.h"  
#else
#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  
#endif


/*
    9.	stream_detector
    Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
    a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time 
    for debouncing and LED / user-interface).
    
    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.
*/

enum STREAM_DETECTOR_PRESETS 
{
    // STREAM_DETECTOR_PRESET_NONE,
    STREAM_DETECTOR_PRESET_VAD_16kHz,
    STREAM_DETECTOR_PRESET_VAD_48kHz,
    STREAM_DETECTOR_PRESET_AD_16kHz,
    STREAM_DETECTOR_PRESET_AD_48kHz,
};

enum STREAM_DETECTOR_PARAMS
{
    // STREAM_DETECTOR_USE_PRESETS,
    STREAM_DETECTOR_LOW_PASS,
    STREAM_DETECTOR_RISE_TIME,
    STREAM_DETECTOR_FALL_TIME,
};

typedef struct          /* 8 Bytes  */
{
    uint8_t log2counter;        /* sample counter= 2^log2counter: maintains the "detected" flag at least for this number of samples */
    uint8_t log2decfMAX;        /* decimation = a power of 2 (-1) */
    uint8_t high_pass_shifter;  /* for z1 */
    uint8_t low_pass_shifter;   /* for z6 */
    uint8_t floor_peak_shifter; /* for z7 */
    uint8_t vad_rise;           /* rise time MiniFloat Mantissa 3bits Exponent 5bits */
    uint8_t vad_fall;           /* fall time Mantissa=[0..7] Exponent=(-1)x[0..31] */
    uint8_t THR;                /* detection threshold z8/z7 */
} detector_parameters;

// TODO Liam Revise byte counts
typedef struct /* total = 36 Bytes*/
{
    detector_parameters config; /* 9 bytes */

    int32_t z1;    /* memory of the high-pass filter (recursive part) */
    int32_t z2;    /* memory of the high-pass filter (direct part) */
    int32_t z3;    /* output of the high-pass filter */
    int32_t z6;    /* memory of the first low-pass filter */
    int32_t z7;    /* memory of the floor-noise tracking low-pass filter */
    int32_t z8;    /* memory of the envelope tracking low-pass filter */
    int32_t accvad;/* accumulator / estimation */
    int32_t Flag;  /* accumulator 2 / estimation  */
    int32_t down_counter;    /* memory of the debouncing downcounter  */
    int16_t decf;  /* memory of the decimator for z7/floor noise estimation */
    uint8_t previous_vad; 
} arm_detector_instance;

#define F2Q31(f) (long)(0x7FFFFFFFL*(f))
#define ConvertSamp(f) (f<<12)
#define DIVBIN(s,n) (s>>n)

// Threshold/clamping to prevent the value 0 entering filter calculations
#define lowestS  F2Q31(0.0001)

#define THR  3
#define MAXVAD  F2Q31(0.98)

// Filter variables
#define Z1 instance->z1
#define Z2 instance->z2
#define Z3 instance->z3
#define Z6 instance->z6
#define Z7 instance->z7
#define Z8 instance->z8
#define DECF instance->decf
#define ACCVAD instance->accvad
#define FLAG instance->Flag
#define SHPF instance->config.high_pass_shifter
#define SLPF instance->config.low_pass_shifter // S1 in tinyVad
#define PREVIOUSVAD instance->previous_vad

// TODO Verify if these can be replaced by a single value 
// S2 in tinyvad. Used for both floor & peak calculations (z7 & z8)
#define SFloorPeak instance->config.floor_peak_shifter

// Replace above with #define SPeak  instance->config.peak_signal_shifter if we do need separate values for each

// Time constants for VAD algorithm
#define MINIF(m,exp) ((m)<<5 | (exp))
#define MINIFLOAT2Q31(x) (((x) & 0xE0) << (23 - ((x) & 0x1F)))
#define VADRISE MINIFLOAT2Q31(instance->config.vad_rise)
#define VADFALL MINIFLOAT2Q31(instance->config.vad_fall)

// Value of decremented counter is 2^log2decfMAX -1 to allow mask to be used to detect rollover
#define decfMASK ((1 << (instance->config.log2decfMAX)) -1)

#ifdef __cplusplus
}
#endif
 