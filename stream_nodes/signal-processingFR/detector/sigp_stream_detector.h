/* ----------------------------------------------------------------------
 * Title:        sigp_stream_detector.h
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

#ifdef __cplusplus
 extern "C" {
#endif
   
#ifndef csigp_STREAM_DETECTOR_H
#define csigp_STREAM_DETECTOR_H


//#include "stream_const.h"      
//#include "stream_types.h"  


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
    STREAM_DETECTOR_PRESET_ACCEL_103Hz,
    STREAM_DETECTOR_PRESET_ECG_360Hz_NOT_IMPLEMENTED,
};


typedef struct          /* 8 Bytes  */
{
/* 
    76543210
    MMMEEEEE
    #define MINIF(m,exp) ((uint8_t)((m)<<5 | (exp)))
    #define MINIFLOAT2Q31(x) (((x) & 0xE0) << (23 - ((x) & 0x1F)))
    #define     MULTIPLIER_MSB 7     
    #define     MULTIPLIER_LSB 5
    #define       EXPONENT_MSB 4     
    #define       EXPONENT_LSB 0
*/
    uint8_t log2counter;        /* sample counter= 2^(log2counter/8) x (2^(log2counter&7))/8  [0 .. ~2^32]
                                    maintains the "detected" flag at least for this number of samples */
    uint8_t log2decfMASK;        /* decimation = a power of 2 (-1) */
    uint8_t high_pass_shifter;  /* for z1 */
    uint8_t low_pass_shifter;   /* for z6 */
    uint8_t floor_peak_shifter; /* for z7 */
    uint8_t vad_rise;           /* rise time MiniFloat Mantissa 3bits Exponent 5bits */
    uint8_t vad_fall;           /* fall time Mantissa=[0..7] Exponent=(-1)x[0..31] */
    uint8_t THR;                /* detection threshold z8/z7 */
} detector_parameters;



typedef struct
{
    int32_t z7;    /* memory of the floor-noise tracking low-pass filter */
    int32_t z8;    /* memory of the envelope tracking low-pass filter */
    int32_t accvad;/* accumulator / estimation */
    int32_t Flag;  /* accumulator 2 / estimation  */
    int32_t down_counter;    /* memory of the debouncing downcounter  */
    uint32_t previous_vad; 

} sigp_backup_memory;


typedef struct 
{
    stream_al_services *services;
    detector_parameters config; /* 8 bytes */
    int32_t z1;    /* memory of the high-pass filter (recursive part) */
    int32_t z2;    /* memory of the high-pass filter (direct part) */
    int32_t z3;    /* output of the high-pass filter */
    int32_t decf;  /* memory of the decimator for z7/floor noise estimation */
    int32_t z6;    /* memory of the first low-pass filter */
    uint32_t traceID_tag; 

    sigp_backup_memory *backup;

} sigp_detector_instance;

#define F2Q31(f) (long)(0x7FFFFFFFL*(f))
#define ConvertSamp(f, s) ((f)<<(s))
#define DIVBIN(s,n) (s>>n)


#define VADRISE MINIFLOAT2Q31(pinstance->config.vad_rise)
#define VADFALL MINIFLOAT2Q31(pinstance->config.vad_fall)

// Threshold/clamping to prevent the value 0 entering filter calculations
// TODO - This prevents zero entering filter but effectively restricts us to
// less than half of the range of Q31
#define CLAMP_MIN  F2Q31(0.0001)
// Clamping to prevent overflow of intermediate calculations and flags

// Note extra headroom required for low fs
#define CLAMP_MAX  F2Q31(0.98)-VADRISE-VADFALL

// Filter variables
#define Z1 pinstance->z1
#define Z2 pinstance->z2
#define Z3 pinstance->z3
#define Z6 pinstance->z6
#define Z7 pinstance->backup->z7
#define Z8 pinstance->backup->z8
#define DECF pinstance->decf
#define ACCVAD pinstance->backup->accvad
#define FLAG pinstance->backup->Flag
#define SHPF pinstance->config.high_pass_shifter
#define SLPF pinstance->config.low_pass_shifter
#define PREVIOUSVAD pinstance->backup->previous_vad
#define THR pinstance->config.THR
#define RELOADCOUNTER pinstance->config.log2counter
#define DOWNCOUNTER pinstance->backup->down_counter

// Replace below with #define SPeak  pinstance->config.peak_signal_shifter and change SLPF to SFloor 
// if we do need separate values for each
#define SFloorPeak pinstance->config.floor_peak_shifter  // S2 in tinyvad.

// Value of decremented counter is 2^log2decfMASK -1 to allow mask to be used to detect rollover
#define decfMASK ((1 << (pinstance->config.log2decfMASK)) -1)

extern void sigp_stream_detector_process (sigp_detector_instance *instance, 
                     int16_t *in, int32_t inputLength, 
                     int16_t *pResult);

#endif //csigp_STREAM_DETECTOR_H

#ifdef __cplusplus
}
#endif
 