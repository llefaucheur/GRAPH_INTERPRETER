
// This file contains both the standard Intel/DVI/IMA ADPCM encoder & decoder,

/***********************************************************
Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*
** Intel/DVI ADPCM coder/decoder.
**
** The algorithm for this coder was taken from the IMA Compatability Project
** proceedings, Vol 2, Number 2; May 1992.

*/


#include <stdio.h>
#include <stdint.h>
#include "arm_codec_imadpcm.h"

// #include "codec.h"

/* Intel ADPCM step variation table */
static int8_t indexTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

static int16_t stepsizeTable[89] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int16_t clamped_summer(int16_t valpred, int16_t vpdiff, int8_t sign){
	
	if ( sign ){
	valpred =  ((-32768 + vpdiff) <= valpred ) ? valpred - vpdiff :  -32768;
	}
	else{
	valpred =  ( (32767 - vpdiff) >= valpred ) ? valpred + vpdiff :  32767;
	}
	return valpred;

}
    
/*
Note: Two samples are stored per output byte
The output buffer can be written to directly without local variable
Input is never modified so also no need for local variable
*/
void encode(CodecState* state, int16_t* input, uint32_t numSamples, uint8_t* outp)
{
    int8_t sign;			/* Current adpcm sign bit #TODO Remove if sign bit of diff can be used directly */
    int16_t delta;			/* Current adpcm output value */
    int16_t diff;			/* Difference between val and valprev */
    int16_t step;			/* Stepsize */
    int16_t valpred;		/* Predicted output value */
    int16_t vpdiff;			/* Current change to valpred */
    int16_t index;			/* Current step change index */
    int16_t outputbuffer;	/* place to keep previous 4-bit value */
    int16_t bufferstep;		/* toggle between outputbuffer/output */

	// Note: Initial states are 0 which gives step = 7
    valpred = state->valprev;
    index = state->index;
    step = stepsizeTable[index];
    
    bufferstep = 1;

    for ( ; numSamples > 0 ; numSamples-- ) {

	/* Step 1 - compute difference with previous value */
	diff = *input++ - valpred; 
	sign = (diff < 0) ? 8 : 0;
	if ( sign ) diff = (-diff);

	/* Step 2 - Divide */
	/* Note:
	** This code *approximately* computes:
	**    delta = diff*4/step;
	**    vpdiff = (delta+0.5)*step/4;
	**    Which expands to give:
	**    vpdiff = ((step/4)*delta + step/8)*step/4;
	*/

	delta = 0;
	vpdiff = (step >> 3);// step/8

	if ( diff >= step ) {
	    delta = 4;
	    diff -= step;
	    vpdiff += step;
	}
	step >>= 1;
	if ( diff >= step  ) {
	    delta |= 2;
	    diff -= step;
	    vpdiff += step;
	}
	step >>= 1;
	if ( diff >= step ) {
	    delta |= 1;
	    vpdiff += step;
	}

	/* Step 3 - Assemble value, update index and step values */
	delta |= sign;
	
	index += indexTable[delta];
	if ( index < 0 ) index = 0;
	if ( index > 88 )
	index = 88;
	step = stepsizeTable[index];

	/* Step 4 - Update previous value and clamp to 16 bits */
	/* Note: If valpred is less than MIN_INT16 + vpdiff then underflow is impossible
	   and likewise when valpred is greater than MAX_INT16 + vpdiff */
	
	valpred = clamped_summer(valpred, vpdiff, sign);	
	
	/* Step 5 - Output value */
	if ( bufferstep ) {
	    outputbuffer = (delta << 4) & 0xf0;
	} else {
	    *outp++ = (delta & 0x0f) | outputbuffer;
	}
	bufferstep = !bufferstep;
    }

    /* Output last step, if needed */
    if ( !bufferstep )
      *outp++ = (uint8_t)outputbuffer;
    
    state->valprev = valpred;
    state->index = index;
}

void decode(CodecState* state, uint8_t* input, uint32_t numSamples, int16_t* output)
{
    int8_t  sign;			/* Current adpcm sign bit #TODO Remove if sign bit of diff can be used directly */
    int16_t delta;			/* Current adpcm output value */
    //int16_t diff;			/* Difference between val and valprev */
    int16_t step;			/* Stepsize */
    int16_t valpred;		/* Predicted output value */
    int16_t vpdiff;			/* Current change to valpred */
    int16_t index;			/* Current step change index */
	int16_t inputbuffer;		/* place to keep next 4-bit value */
    int16_t bufferstep;		/* toggle between inputbuffer/input */

    valpred = state->valprev;
    index = state->index;
    step = stepsizeTable[index];

    bufferstep = 0;
    
    for ( ; numSamples > 0 ; numSamples-- ) {
	
	/* Step 1 - get the delta value */
	if ( bufferstep ) {
	    delta = inputbuffer & 0xf;
	} else {
	    inputbuffer = *input++;
	    delta = (inputbuffer >> 4) & 0xf;
	}
	bufferstep = !bufferstep;

	/* Step 2 - Find new index value (for later) */
	index += indexTable[delta];
	if ( index < 0 ) index = 0;
	if ( index > 88 ) index = 88;

	/* Step 3 - Separate sign and magnitude */
	sign = delta & 8;
	delta = delta & 7;

	/* Step 4 - Compute difference and new predicted value */
	/*
	** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
	** in adpcm_coder.
	*/
	vpdiff = step >> 3;
	if ( delta & 4 ) vpdiff += step;
	if ( delta & 2 ) vpdiff += step>>1;
	if ( delta & 1 ) vpdiff += step>>2;

	valpred = clamped_summer(valpred, vpdiff, sign);	

	/* Step 6 - Update step value */
	step = stepsizeTable[index];

	/* Step 7 - Output value */
	*output++ = valpred;
    }

    state->valprev = valpred;
    state->index = index;
}