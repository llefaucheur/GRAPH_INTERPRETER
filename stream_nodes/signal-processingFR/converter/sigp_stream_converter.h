/* ----------------------------------------------------------------------
 * Title:        sigp_stream_converter.h
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
   

#ifndef CSIGP_STREAM_CONVERTER_H
#define CSIGP_STREAM_CONVERTER_H

#define NMAC 0 
#define FILEL 1
#define PHASES 2
#define LIMITQ15 3
extern const uint16_t ssrc_RomDescriptor[];
extern const f32_t ssrc_RomCoefficients[];

/*
 * MEM_ALIGN return the input i64 integer incremented to be aligned on a number of bits/bytes boundary
 *    for example MEM_ALIGN_BITS( (long long)0x0000000000000FFF,0x00000080L) = 
 *                                           0x0000000000001080
 *    for example MEM_ALIGN_BYTES((long long)0x0000000000000FFF,0x00000010L) = 
 *                                           0x0000000000001080
 */
#define MEM_ALIGN_BYTES(x,nbytes) (((long long)x+((1<<nbytes)-1))&((long long)(-1)<<nbytes))

/*
 * The frequency change ratio is FsOut/FsIn = L/M
 * The algorithm arranges L or M to be as close as possible to the original PHASE
 *    with a variation of +/- 10% (factor 0.1 below)
 * The filter length is FILEL/(coefficient step reading the Sinc(x)) and the step is
 *    step = 
 */
#define PHASE_MARGIN 0.1

/* 
 *  low-complexity SSRC configurations 
 */
#define SRC_24T12P 1        /* linear phase, 85dB out-of-band attenuation */
#define SRC_24T12P_IIR 0    /* latency 7 times lower, 70dB out-of-band attenuation */
#define SRC_24T1280P 0      /* 1280 phases for exact 44.1kHz conversions, 100dB out-of-band attenuation */
#define SRC_32T1280P 0      /* complexity increase (32 TAPS), 140dB out-of-band attenuation (THDN+A) */


#define NEON_ENABLE 0
#define CORTEXM 0
#define MVE 0

#define MAX_NCHAN_CONVERTER 2       // mono / stereo
#define MAX_SIZE_SAMP 2             // q15 samples
#define INTERPOLATE_MEMSIZE 2       // two interpolation coefficients
#define MAX_NSAMP_INTERPOLATE (MAX_NCHAN_CONVERTER * MAX_SIZE_SAMP * INTERPOLATE_MEMSIZE)


#define FORMAT_Q15 0
#define FORMAT_F32 1
#define MAXNBFORMATS 2

#define MAXOSR 8                    // interpolation ratio max
#define FIRLMAX 32                  // 32 taps max (typ = 24)
#define SSRCFIRMEM FIRLMAX*(MAXOSR+1)// memory allocation for the FIR filter
#define FILTER_STATE_MEMORY_SIZE (FIRLMAX*MAXOSR)

/****************************************************************************************/
/*                                                                                      */
/*  Types                                                                               */
/*                                                                                      */
/****************************************************************************************/

#define BYTES_ALIGNMENTS 4

typedef struct
{
    f32_t *coefRom;         /* sinc(x) low-pass filter shape NMAC*NPHASE length */
    union {
        s16_t *q15;
        f32_t *f32;
    } firRam;               /* FIR memory */
    union {
        s16_t *q15;
        f32_t *f32;
    } coefReordered;        /* Coefficients used during the VLIB computations (Q15, F32) */
 
    s32_t *push;            /* Number of samples to puch in the FIR at each phase increments */
    s32_t *idx;

} fwd_ssrc_memories;    
typedef fwd_ssrc_memories *fwd_ssrc_memories_ptr;


typedef struct
{
    const uint16_t *coefDescriptor;
    const f32_t *coefRom;
    s32_t fsIn, fsOut, fsResult;
    s32_t format;

} fwd_ssrc_parameters;      

typedef fwd_ssrc_parameters *fwd_ssrc_parameters_ptr;

typedef struct
{
    s32_t phases;
    s32_t nmac;
    s32_t L, M; 
    s32_t firlen; 
    s32_t coefSteps; 
    s32_t startOffset; 
    s32_t extraSteps;

    s32_t memOffsetFirmen;
    s32_t memOffsetCoefReordered;
    s32_t memOffsetPush;
    s32_t memOffsetIdx;

} fwd_ssrc_intermediate;      
typedef fwd_ssrc_intermediate *fwd_ssrc_intermediate_ptr;



typedef struct
{
    fwd_ssrc_parameters_ptr paramsPtr;
    fwd_ssrc_memories_ptr memoriesPtr;

    iidx_t phase;           // counter for the polyphase
    iidx_t nPhases;
    s32_t firlen;           // FIR length

    s32_t idxCoef;
    s32_t idxFirmem;
    s32_t idxFirLast;
    s32_t remainSamp;
    s32_t idxMax;
    s32_t format;
} fwd_ssrc_instance;


typedef struct
{
    uint8_t nchan;
    uint8_t raw;
    uint8_t interleaving;
    uint8_t timestamp;
    uint8_t timestampsize;
    uint8_t domain;
    uint32_t framesize;
    uint32_t sampling_rate;
    uint32_t mapping;
} sigp_stream_converter_format;

/*
    converter INSTANCE :
*/
typedef struct
{
    stream_al_services *stream_entry;  /* calls to computing services */
    sigp_stream_converter_format input, output;
    fwd_ssrc_instance ssrc;
    float *buff1, *buff2, *memory;
    //uint8_t static_mem[MAX_NSAMP_INTERPOLATE];
} sigp_stream_converter_instance;


extern void sigp_stream_converter_process1 (sigp_stream_converter_format *in, sigp_stream_converter_format *out, 
    float *inBuf, float *tmpBuf1);

extern void sigp_stream_converter_process2 (fwd_ssrc_instance *ssrc,
    sigp_stream_converter_format *input_format, sigp_stream_converter_format *output_format, 
    float *inputBuffer, float *outputBuffer, int32_t nSampIn, intPtr_t  *nSampOut);

extern void sigp_stream_converter_process3 (sigp_stream_converter_format *out, 
    float *tmpBuf2, float *outBuf);

extern void  ssrc_findRationalRatio (s32_t phases, f32_t ratio, s32_t *Lreturned, s32_t *Mreturned);

extern void ssrc_computeFirLength (fwd_ssrc_parameters *ssrc_parameters, fwd_ssrc_intermediate *ssrc_intermediate);

extern void  ssrc_init (fwd_ssrc_instance *ssrc, 
                fwd_ssrc_parameters *ssrc_parameters, 
                fwd_ssrc_intermediate *ssrc_intermediate);

extern void  ssrc_getWorkingMemRequirement (s32_t *amountOfMemory, 
                s32_t *memoryAlignmentInBytes, 
                fwd_ssrc_parameters *ssrc_parameters,
                fwd_ssrc_intermediate *ssrc_intermediate);

extern void  ssrc_getInstance (fwd_ssrc_instance *ssrc, 
                        fwd_ssrc_parameters *ssrc_parameters, 
                        char *allocatedMemory, 
                        fwd_ssrc_intermediate *ssrc_intermediate);

extern const uint16_t ssrc_RomDescriptor[];
extern const f32_t ssrc_RomCoefficients[];
#endif

#ifdef __cplusplus
}
#endif
 