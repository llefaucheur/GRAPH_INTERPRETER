/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_enc
 _imadpcm.c
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

/*

    RFC 1890                       AV Profile                   January 1996

    ...
    DVI4 is specified, with pseudo-code, in [6] as the IMA ADPCM wave
    type. A specification titled "DVI ADPCM Wave Type" can also be found
    in the Microsoft Developer Network Development Library CD ROM
    published quarterly by Microsoft. The relevant section is found under
    Product Documentation, SDKs, Multimedia Standards Update, New
    Multimedia Data Types and Data Techniques, Revision 3.0, April 15,
    1994. 
   
    The document, "IMA Recommended Practices for Enhancing Digital Audio
    Compatibility in Multimedia Systems (version 3.0)", contains the
    algorithm description.  It is available from:
   
    Interactive Multimedia Association
    48 Maryland Avenue, Suite 202
    Annapolis, MD 21401-8011
    USA
    phone: +1 410 626-1380

   [6] IMA Digital Audio Focus and Technical Working Groups,
       "Recommended practices for enhancing digital audio compatibility
       in multimedia systems (version 3.00)," tech. rep., Interactive
       Multimedia Association, Annapolis, Maryland, Oct. 1992.

        

   Extracts ()
      Audio Compression and Decompression Algorithm Requirements:
        - Public domain algorithm and stream format.
        - Reference compression and decompression algorithms (pseudo-code or C) to be published by the IMA if 
            not already available in public domain.
        - Compressed data stream definition may allow more sophisticated compression algorithms to be 
            used (scalability feature), however, published decompression algorithm must be able to play resulting 
            data stream (compatibility requirement).
        - No license fees or royalty associated with using the algorithm as published.
        - Real-time decompression must be achievable on PC or workstation CPU.
        - Scalability of decompression bandwidth, etc., to accommodate range of processor capability is acceptable.
        - Non Real-time compression acceptable, or scaleable real-time compression on PC or workstation CPU.
        - Applicable across multiple frequencies, especially 8.0, 11.025, 22.05, and 44.1 kHz.
        - Low data storage overhead for minimum bits per audio sample.
*/


#include <stdio.h>
#include <stdint.h>
#include "arm_enc_imadpcm.h"
#include "dsp/none.h"


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


    
/*
        The "state" keeps encoded data until there is enough to send a byte to outp
*/
void encode_imadpcm(CodecState* state, int16_t* input, uint32_t numSamples, uint8_t* outp)
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

    for ( ; numSamples > 0 ; numSamples-- ) 
    {
	    /* Step 1 - compute difference with previous value */
	    diff = *input++ - valpred; 
	    sign = (diff < 0) ? 8 : 0;
	    if ( sign ) diff = (-diff);

	    delta = 0;
	    vpdiff = (step >> 3);

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
	
        if (sign)
	        valpred = valpred - vpdiff;
        else
	        valpred = valpred + vpdiff;

        __SSAT(valpred,16);

	    /* Step 5 - Output value */
	    if ( bufferstep ) {
	        outputbuffer = delta & 0x0f;
	    } else {
	        *outp++ = ((delta << 4) & 0xf0) | outputbuffer;
	    }
	    bufferstep = !bufferstep;
    }

    /* Output last step, if needed */
    if ( !bufferstep )
      *outp++ = (uint8_t)outputbuffer;
    
    state->valprev = valpred;
    state->index = index;
}

