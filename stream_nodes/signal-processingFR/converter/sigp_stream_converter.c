/* ----------------------------------------------------------------------
 * Title:        sigp_stream_convert.c
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

#include "platform.h"
#ifdef CODE_SIGP_CONVERTER

#ifdef __cplusplus
 extern "C" {
#endif
   


#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"

#include "sigp_stream_converter.h"


/*
;----------------------------------------------------------------------------------------
;3.	sigp_stream_converter
;----------------------------------------------------------------------------------------
Operation : convert input arc format to the format of the output arc. 
Conversion takes care of RAW data, frame size, interleaving, time-stamp format, number of channels, sampling-rate.

Operations :
- copy input data to the first internal scratch buffer with the management of the target number of channels and using 
    deinterleave intermediate format. 
    The start of the deinterleaved buffer is a copy of the previously processed end of buffers, 
    for the implementation of filters using a memory.
- sampling rate converter, with a dedicated processing for the asynchronous to synchronous conversion, 
    to second scratch buffer
- raw data conversion, interleaving conversion and copy of the result

The case of IMU format is managed like other one-dimension format. 

The case of 2D format is special with operations of pixel area extraction, zoom, interpolate, rotation  and pixel format conversions.

*/

/*
 * size of each types managed by VLIB
 */
const s32_t ssrc_formatSize[MAXNBFORMATS] = { 2 , 4 }; /* nb of byte for the two formats (Q15 and F32) */


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void sigp_stream_converter (unsigned int command, void *instance, void *data, unsigned int *status)
{
    *status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                memreq are followed by 4 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the NODE manifest

                memreq[ 0] : instance of the component
                memreq[ 1] : pointer to the scratch buff 1
                memreq[ 2] : pointer to the scratch buff 2
                memreq[ 3] : pointer to the rate converter filter memory 
                memreq[ 4] : input arc Word 0 SIZSFTRAW_FMT0 (frame size..)
                memreq[ 5] : input arc Word 1 SAMPINGNCHANM1_FMT1 
                memreq[ 6] : input arc Word 2 SIZSFTRAW_FMT0 (frame size..)
                memreq[ 7] : input arc Word 3 SAMPINGNCHANM1_FMT1 
                memreq[ 8] : output arc Word 0 SIZSFTRAW_FMT0 
                memreq[ 9] : output arc Word 1 SAMPINGNCHANM1_FMT1 
                memreq[10] : output arc Word 2 SIZSFTRAW_FMT0 
                memreq[11] : output arc Word 3 SAMPINGNCHANM1_FMT1 
        */
        case STREAM_RESET: 
        {   stream_al_services *stream_entry = (stream_al_services *)data;
            intPtr_t *memreq = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);
            sigp_stream_converter_instance *pinstance;
            fwd_ssrc_intermediate ssrc_intermediate;

            /* read memory banks */
            pinstance = (sigp_stream_converter_instance *) (*memreq++);  /* 0 main instance */
            pinstance->buff1 = (float *) (*memreq++);                 /* 1 second bank = buff1 */
            pinstance->buff2 = (float *) (*memreq++);                 /* 2 third bank = buff2 */
            pinstance->memory = (float *) (*memreq++);                  /* 3 last bank = static memory */

            /* read the stream format information */
            pinstance->input.framesize     = RD(memreq[       FRAMESZ_FMT0], FRAMESIZE_FMT0);
            pinstance->input.nchan         = RD(memreq[   NCHANDOMAIN_FMT1],   NCHANM1_FMT1) +1;
            pinstance->input.raw           = RD(memreq[   NCHANDOMAIN_FMT1],       RAW_FMT1);
            pinstance->input.interleaving  = RD(memreq[   NCHANDOMAIN_FMT1], INTERLEAV_FMT1);
            pinstance->input.timestamp     = RD(memreq[   NCHANDOMAIN_FMT1],  TIMSTAMP_FMT1);
            pinstance->input.timestampsize = RD(memreq[   NCHANDOMAIN_FMT1],  TSTPSIZE_FMT1);
            pinstance->input.sampling_rate = FLOAT_TO_INT(RD(memreq[  SAMPLINGRATE_FMT2], FS1D_FMT2));
            pinstance->input.mapping       = RD(memreq[DOMAINSPECIFIC_FMT3], AUDIO_MAP_FMT3);
            memreq += STREAM_FORMAT_SIZE_W32;

            pinstance->output.framesize    = RD(memreq[       FRAMESZ_FMT0], FRAMESIZE_FMT0);
            pinstance->output.nchan        = RD(memreq[   NCHANDOMAIN_FMT1],   NCHANM1_FMT1) +1;
            pinstance->output.raw          = RD(memreq[   NCHANDOMAIN_FMT1],       RAW_FMT1);
            pinstance->output.interleaving = RD(memreq[   NCHANDOMAIN_FMT1], INTERLEAV_FMT1);
            pinstance->output.timestamp    = RD(memreq[   NCHANDOMAIN_FMT1],  TIMSTAMP_FMT1);
            pinstance->output.timestampsize= RD(memreq[   NCHANDOMAIN_FMT1],  TSTPSIZE_FMT1);
            pinstance->output.sampling_rate= FLOAT_TO_INT(RD(memreq[  SAMPLINGRATE_FMT2], FS1D_FMT2));
            pinstance->output.mapping      = RD(memreq[DOMAINSPECIFIC_FMT3], AUDIO_MAP_FMT3);


            fwd_ssrc_instance ssrc_instance;
            fwd_ssrc_memories ssrc_memories;
            fwd_ssrc_parameters ssrc_parameters;

            /* init global variables */
            ssrc_instance.memoriesPtr = &ssrc_memories;
            ssrc_instance.paramsPtr   = &ssrc_parameters;
            ssrc_parameters.coefDescriptor = ssrc_RomDescriptor; 
            ssrc_parameters.coefRom = ssrc_RomCoefficients;    
            ssrc_parameters.fsIn = pinstance->input.sampling_rate;
            ssrc_parameters.fsOut = pinstance->output.sampling_rate;
            ssrc_parameters.format = FORMAT_F32;

            ssrc_getInstance (&ssrc_instance, &ssrc_parameters, 0, &ssrc_intermediate);
            ssrc_init (&ssrc_instance, &ssrc_parameters, &ssrc_intermediate);

            /* here reset */
            uint8_t *pt8b, i, n;
            pt8b = (uint8_t *) (pinstance->memory);
            n = pinstance->output.nchan * pinstance->output.framesize * INTERPOLATE_MEMSIZE;
            for (i = 0; i < n; i++) { pt8b[i] = 0; }
        }

        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input size} {*output size}]
        */         
        case STREAM_RUN:   
        {
            sigp_stream_converter_instance *pinstance = (sigp_stream_converter_instance *) instance;
            intPtr_t nb_data_read, nb_data_write;
            stream_xdmbuffer_t *pt_pt;

            float *inBuf;
            float *outBuf;

            pt_pt = data; inBuf  = (float *)(pt_pt->address); nb_data_read = pt_pt->size;  /* amount of data to process */
            pt_pt++;      outBuf = (float *)(pt_pt->address); nb_data_write = pt_pt->size; /* maximum data to produce */

            /* deinterleaving and channel mapping,
                check raw_in, domain_in, nchan(in/out) and intlv */
            sigp_stream_converter_process1 (&(pinstance->input), &(pinstance->output), 
                inBuf, pinstance->buff1);

            /* rate conversion 
                check FS_out / FS_in , compute nb_data_read, nb_data_write */
            sigp_stream_converter_process2 (&(pinstance->ssrc),
                &(pinstance->input), &(pinstance->output), 
                pinstance->buff1, pinstance->buff2, nb_data_read, &nb_data_write);

            /* re-interleaving and format conversion to the output buffer
                check raw_out, domain_out, nchan(out) and intlv_out */
            sigp_stream_converter_process3 (&(pinstance->output),  
                pinstance->buff2, outBuf);

            pt_pt = data;   *(&(pt_pt->size)) = nb_data_read;   /* amount of data consumed */
            pt_pt ++;       *(&(pt_pt->size)) = nb_data_write;  /* amount of data produced */
            break;
        }

        default :
            break;
    }
}


/* integer modulo */
int modulo (int a, int b)
{
    int c = a;
    while (c < 0) c += b; 
    while (c >= b) c -= b; 
    return c;
}


/**
* @brief computes the SSRC filtering length from FSOUT/FSIN
*
* @param ssrc_parameters        Pointer to the parameters
*
* @return NONE
*/

void ssrc_computeFirLength (fwd_ssrc_parameters *ssrc_parameters, 
                            fwd_ssrc_intermediate *ssrc_intermediate)
{
    float ratio;
    iidx_t coefSteps, startOffset, extraSteps;
    iidx_t L, M;
    iidx_t phases, limitq15, filel;
    iidx_t i, j;
    iidx_t RMax, RMin;
    s32_t firlen;


    phases = M = L = ssrc_parameters->coefDescriptor[PHASES];
    limitq15 = ssrc_parameters->coefDescriptor[LIMITQ15];
    filel = ssrc_parameters->coefDescriptor[FILEL];


    if (ssrc_parameters->fsOut > ssrc_parameters->fsIn)
        ratio = (float)ssrc_parameters->fsOut / (float)ssrc_parameters->fsIn;
    else
        ratio = (float)ssrc_parameters->fsIn / (float)ssrc_parameters->fsOut;

    if (ratio > MAXOSR)
    { /* sys_error(ERR_ssrc_osr, "OSR > 8 "); */ 
    }

    ssrc_findRationalRatio (phases, ratio, &L, &M);


    ssrc_intermediate->L = L;        /* save the L/M results */
    ssrc_intermediate->M = M;

    /* and avoid surprises in the output results */
    ssrc_parameters->fsResult = (s32_t) ((float)L * ssrc_parameters->fsIn / M);

/*-------------------------------------------------------------------------*/
    
    if (ssrc_parameters->fsOut > ssrc_parameters->fsIn) 
    {   RMax = L;
        RMin = M;
    }
    else
    {   i = L; j = M; 
        L = RMin = j;
        M = RMax = i;
    }

    if (L > M)
    {   coefSteps = RMax;
    }
    else
    {   coefSteps = RMin;
    }


    firlen = (filel + coefSteps-1 ) / coefSteps;
    startOffset = 0;
    // if NEON
    if (firlen & 3)
        extraSteps = 4 - (firlen & 3);
    else
        // if not NEON
        extraSteps = 0;

    
    ssrc_intermediate->coefSteps = coefSteps;
    ssrc_intermediate->startOffset = startOffset;
    ssrc_intermediate->extraSteps = extraSteps;
    ssrc_intermediate->firlen = firlen;
    ssrc_intermediate->coefSteps = coefSteps;
    ssrc_intermediate->extraSteps = extraSteps;
    ssrc_intermediate->phases = L;
    ssrc_intermediate->nmac = firlen + extraSteps;
}



/**
* @brief estimate the amout of memory needed for the VLIB
*
* This function returns the first parameter structure the amount of memory to allocate
* from the the second parameter which is the VLIB input parameter 
*
* @param ssrc_memories          Pointer to the list of buffer to be allocated
* @param ssrc_parameters        Pointer to the parameters
*
* @return NONE
*/
void  ssrc_init (fwd_ssrc_instance *ssrc, 
                fwd_ssrc_parameters *ssrc_parameters, 
                fwd_ssrc_intermediate *ssrc_intermediate)
{
    float *fircoef_f32;
    float max_acc_f32, acc_f32, *c_f32;
    float *pt_coef_f32;
    iidx_t L, M;
    iidx_t *idx, *push, filel;
    iidx_t coefSteps, startOffset, extraSteps;
    iidx_t i, phase;
    s32_t firlen;
    fwd_ssrc_intermediate ssrc_intermediates;


    ssrc_computeFirLength (ssrc_parameters, &ssrc_intermediates);

    coefSteps = ssrc_intermediates.coefSteps;
    startOffset = ssrc_intermediates.startOffset;
    firlen = ssrc_intermediates.firlen;
    extraSteps = ssrc_intermediates.extraSteps;

    L = ssrc_intermediates.L;
    M = ssrc_intermediates.M;
    ssrc->phase = L;
/*-------------------------------------------------------------------------*/


    filel = ssrc_parameters->coefDescriptor[FILEL];
    idx = ssrc->memoriesPtr->idx;
    push = ssrc->memoriesPtr->push;

    *(idx+0)  = 0;
    for (i = 1; i < L+1; i++)  {              // compute the coef indexes 

        *(idx+i)  = modulo((*(idx+i-1) + M), L);
        *(push+i) = (*(idx+i-1) + M) / L;
    }
    *(push+0) = *(push+L);


/*-------------------------------------------------------------------------*/
    fircoef_f32 = ssrc->memoriesPtr->coefRom;

/*-------------------------------------------------------------------------*/
    // Normalize the amplitude based on the number of phases , to avoid the saturation during ringing effects of the low-pass filters
    //   this translates to about 12% -1dB loss 

#define AVOID_CLIPPING 0    // avoids any saturation in the filter (amplitude becomes ~ -6dB)
#define AVOID_OVERSHOOT 0   // avoids overshoots with square waves (amplitude becomes ~ -1dB)
#define MAXIMUM_LEVEL 1     // preserve original sinewave level

    max_acc_f32 = 0;
    for (phase = 0; phase < L; phase++)
    {
        acc_f32 = 0;
        for (c_f32 = &(fircoef_f32[idx[phase]]), i = 0; i < firlen; i++, c_f32+=coefSteps) 
        {
#if AVOID_CLIPPING
            acc_f32 += fabs(*c_f32);
#elif AVOID_OVERSHOOT 
            acc_f32 += (*c_f32);
#elif MAXIMUM_LEVEL
            acc_f32 += (*c_f32);
#endif
        }
        acc_f32 = (acc_f32 < 0)? (-acc_f32) : acc_f32;
#if MAXIMUM_LEVEL
        max_acc_f32 += acc_f32/L;
#else
        max_acc_f32 = (acc_f32 > max_acc_f32) ? acc_f32:max_acc_f32;
#endif
    }

    if (ssrc_parameters->format == FORMAT_Q15 && max_acc_f32 < 1)
    {   max_acc_f32 = 1;
    }


    /* 
     *  reorder the coefficients to be addressed linearly 
     */
    pt_coef_f32 = ssrc->memoriesPtr->coefReordered.f32;

    for (phase = 0; phase < L; phase++)
    {   c_f32 = &(fircoef_f32[idx[phase]]);
        c_f32 += startOffset;
        c_f32 += (firlen -1) * coefSteps;
        for (i = 0; i <firlen; i++, c_f32 -= coefSteps) 
        {  *pt_coef_f32++ = (*c_f32)/ max_acc_f32;
        }
        for (i = 0; i < extraSteps; i++) 
        {  *pt_coef_f32++ = 0;
        }
    }
    // now firlen is multiple of 4 for NEON
    ssrc->firlen = firlen + extraSteps;
    


    /* number of phases during filtering */
    ssrc->nPhases = L;
    ssrc->idxCoef = 0;
    ssrc->idxFirmem = ssrc_intermediate->nmac;
    ssrc->idxFirLast = 0;
    ssrc->remainSamp = 0;
    ssrc->idxMax = FILTER_STATE_MEMORY_SIZE;
    ssrc->format = ssrc_parameters->format;
}



/**
* @brief find the best L/M ratio from a floating point number 
*
* This function returns the L/M the closest to a floating point number
*
* @param ssrc_memories          Pointer to the list of buffer to be allocated
* @param ssrc_parameters        Pointer to the parameters
*
* @return NONE
*/
void  ssrc_findRationalRatio (s32_t phases, f32_t ratio, s32_t *Lreturned, s32_t *Mreturned)
{
    float approx, bestErr, err;
    int invert, Sign_Linc, found;
    s32_t L_increment, L, M, bestL, bestM;
    iidx_t i;

/*-------------------------------------------------------------------------
    FILEL = PHASES * NMAC
    Flat filter = NMAC taps, coefficients taken every PHASES (reference coefficients steps)
    Half-band filter = Coefficients taken at steps PHASES/2

    if L < M    DOWNSAMPLING  :  48kHz => 44.1kHz for example
  */      

    if (ratio < 1) 
    {  ratio = 1/ratio; invert = 1;
    }
    else invert=0; 

    L = phases; M = phases; 

    if (ratio != 1)
    {        
        L = phases;  
        L_increment = 0; 
        Sign_Linc = 1;
        found = 0;
        bestErr = 1e9;
        bestL = phases;
        bestM = phases;
        
        while (1) 
        {
            approx = (float)M/L;

            M = 1;
            while (M < 8*L)
            {
                err = ratio - (float)L/M;
                if (err < 0) err = (-err);
                if (err < 1e-6)
                {
                    found = 1;
                    bestL = L;
                    bestM = M;
                    break;
                }
                else
                {   if (err < bestErr)
                    {   bestL = L;
                        bestM = M;
                        bestErr = err;
                    }
                }
                M = M+1;
            }
            if (found)
                break; 
            
            // arrange the best ratio to start from PHASE starting point and
            // progressively find solution away from it
            if (L_increment >= PHASE_MARGIN * phases) break; 
            else 
            {
                if (Sign_Linc == 1)
                {
                    Sign_Linc = (-1);
                    L = phases + Sign_Linc * L_increment;
                }
                else
                {
                    L_increment ++;
                    Sign_Linc = (+1);
                    L = phases + Sign_Linc * L_increment;
                }
            } 
        }
        M = bestM;
        L = bestL;
        if (invert)
        {   i = L; L = M; M = i;
            ratio = 1/ratio;
        }
    }

    *Lreturned = L;
    *Mreturned = M;
}


/**
* @brief Create one instance of the SSRC 
*
* This function returns an instance of VLIB located in the allocatedMemory area.
* It call ssrc_init() to initialize the memory from input parameters
*
* @param ssrc_instance          Pointer to the list of buffer to be allocated
* @param ssrc_memories          Pointer to the list of buffer to be allocated
* @param ssrc_parameters        Pointer to the parameters
*
* @return NONE
*/
void  ssrc_getInstance (fwd_ssrc_instance *ssrc, 
                        fwd_ssrc_parameters *ssrc_parameters, 
                        char *allocatedMemory, 
                        fwd_ssrc_intermediate *ssrc_intermediate)
{
    s32_t memoryAlignmentBitsShifts, amountOfMemory;
    memoryAlignmentBitsShifts = 4;

    /* if the input parameters are not complete, then take the default one an report the error */
    if (allocatedMemory == NULL)
    {   
        ssrc_parameters->coefDescriptor = (uint16_t *)ssrc_RomDescriptor;
        ssrc_parameters->coefRom = (f32_t *)ssrc_RomCoefficients;

        ssrc_getWorkingMemRequirement (&amountOfMemory, &memoryAlignmentBitsShifts, ssrc_parameters, ssrc_intermediate);
    }

    /*
     * reordered coefficients
     */
    ssrc->memoriesPtr->coefReordered.f32 = (f32_t *)(allocatedMemory + ssrc_intermediate->memOffsetCoefReordered);

    /*
     * push[]
     */
    ssrc->memoriesPtr->push = (s32_t *)(allocatedMemory + ssrc_intermediate->memOffsetPush);

    /*
     * idx[]
     */
    ssrc->memoriesPtr->idx = (s32_t *)(allocatedMemory + ssrc_intermediate->memOffsetIdx);

    /*
     * Fir memory
     */
    ssrc->memoriesPtr->firRam.f32 = (f32_t *)(allocatedMemory + ssrc_intermediate->memOffsetFirmen);

    ssrc->memoriesPtr->coefRom = (float *)(ssrc_parameters->coefRom);
}


/**
* @brief estimate the amout of memory needed for the VLIB
*
* This function returns the number of bytes and alignement requirements
*
* @param ssrc_memories          Pointer to the list of buffer to be allocated
* @param ssrc_parameters        Pointer to the parameters
*
* @return NONE
*/
void  ssrc_getWorkingMemRequirement (s32_t *amountOfMemory, 
                                     s32_t *memoryAlignmentInBytes, 
                                     fwd_ssrc_parameters *ssrc_parameters,
                                     fwd_ssrc_intermediate *ssrc_intermediate)
{
    s32_t accMemory, dataSize, newMem;

    /* if the input parameters are not complete, then take the default one an report the error */
    if (ssrc_parameters->coefDescriptor == NULL ||
        ssrc_parameters->coefRom == NULL)
    {   
    
        ssrc_parameters->coefDescriptor = (uint16_t *)ssrc_RomDescriptor;
        ssrc_parameters->coefRom = (f32_t *)ssrc_RomCoefficients;
    
        //ssrc_initDefaultRomTable(ssrc_parameters);
    }

    *memoryAlignmentInBytes = 4;                                /* memory alignment in W32 */

    accMemory = 0;
    /*
     * FIRMEM[]
     */
    ssrc_intermediate->memOffsetFirmen = 0;                     /* save the byte offset to firmem */
    ssrc_computeFirLength (ssrc_parameters, ssrc_intermediate);
    //if (ssrc_intermediate->firlen > SSRCFIRMEM)
    //    sys_error(ERR_ssrc_malloc, "ssrc size overflow");

    dataSize = ssrc_formatSize[ssrc_parameters->format];        /* data size is precomputed */     
    newMem = dataSize * FILTER_STATE_MEMORY_SIZE;               /* contribution of the firmem + NEON alignment */
    newMem = ((newMem + 3) >> 2) <<2;                           /* rounded to W32 */
    accMemory += newMem;

    /*
     * reordered coefficients
     */
    ssrc_intermediate->memOffsetCoefReordered = accMemory;      /* save the byte offset to firmem */
    newMem = ssrc_intermediate->nmac * ssrc_intermediate->phases; /* contribution of the rearanged FIR coefficients */
    dataSize = ssrc_formatSize[ssrc_parameters->format];        /* data size is precomputed */    
    newMem = newMem * dataSize;
    newMem = ((newMem + 3) >> 2) <<2;                           /* rounded to W32 */
    accMemory += newMem;


    /*
     * push[]
     */
    ssrc_intermediate->memOffsetPush = accMemory;               /* save the byte offset to push */
    newMem = sizeof(s32_t) * ssrc_intermediate->L;              /* contribution of the push[] */
    newMem = ((newMem + 3) >> 2) <<2;                           /* rounded to W32 */
    accMemory += newMem;


    /*
     * idx[]
     */
    ssrc_intermediate->memOffsetIdx = accMemory;                /* save the byte offset to idx */
    newMem = sizeof(s32_t) * ssrc_intermediate->L;              /* contribution of the idx[] */
    newMem = ((newMem + 3) >> 2) <<2;                           /* rounded to W32 */
    accMemory += newMem;

    *amountOfMemory = accMemory;                                /* return the total amount of memory */
}


#endif  //CODE_SIGP_STREAM_CONVERTER

#ifdef __cplusplus
}

#else
void sigp_stream_converter(unsigned int command, void *instance, void *data, unsigned int *status) { /* fake access */ if(command || instance || data || status) return;}

#endif  // #ifndef CODE_sigp_STREAM_CONVERTER

 