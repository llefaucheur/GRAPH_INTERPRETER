/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_codec.c
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


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  


#ifdef __cplusplus
 extern "C" {
#endif

typedef struct
{
    stream_entrance *stream_entry;
    float data0;
    float data1;
    float data2;
} arm_stream_codec_instance;


/**
  @brief        single entry point interface to the services of Stream 
  @param[in]    command     service asked by the component
  @param[in]    instance    holds the address of the Stream interface
  @param[in]    data        buffer address
  @param[in]    size        size of the buffer
  @return        none
 */
int32_t arm_stream_codec_calls_stream (int32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *size)
{
    stream_entrance *STREAM; /* function pointer used for debug/trace, memory move and free, signal processing */
    arm_stream_codec_instance *pinstance = (arm_stream_codec_instance *)instance;

    switch (command)
    {   
        //case STREAM_NODE_REGISTER: first command to register the SWC
        //case STREAM_DEBUG_TRACE:
        //{  
        //}
        default: 
            return 0;
    }
    STREAM = pinstance->stream_entry;
    STREAM (command, (uint8_t*)instance, (uint8_t*)data, 0);  /* single interface to STREAM for controls ! */
    return 0;
}


/**
  @brief         Processing function 
  @param[in]     S         points to an instance of the floating-point Biquad cascade structure
  @param[in]     pSrc      points to the block of input data
  @param[out]    pDst      points to the block of output data
  @param[in]     blockSize  number of samples to process
  @return        none
 */
void codec_processing (arm_stream_codec_instance *instance, 
                     uint8_t *in, int32_t nb_data, 
                     int16_t *outBufs)
{
    /* fake decoder*/ 
    int i;
    for (i = 0; i < nb_data; i++)
    {
        outBufs[i] = (int16_t)((int16_t)(in[i])<<8);
    }
}


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_codec (int32_t command, uint32_t *instance, data_buffer_t *data, uint32_t *status)
{
    *status = 1;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, PRESET, TAG, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memory_results are followed by the first two words of STREAM_FORMAT_SIZE_W32 
                memory pointers are in the same order as described in the SWC manifest
        */
        case STREAM_RESET: 
        {   stream_entrance *stream_entry = (stream_entrance *)(uint64_t)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_stream_codec_instance *pinstance = (arm_stream_codec_instance *)  *memresults;
            memresults++;

            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:
        {   
            uint8_t *new_parameters = (uint8_t *)data;
            break;
        }



        /* func(command = STREAM_READ_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG/index of a parameter to read (Metadata, Needle), 0xFF means "read all the parameters"
                *instance, 
                data = *parameter(s) to read
        */ 
        case STREAM_READ_PARAMETER:  
        {   
            uint8_t *new_parameters = (uint8_t *)data;

            break;
        }
        


        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input size} {*output size}]

               data format is given in the node's manifest used during the YML->graph translation
               this format can be FMT_INTERLEAVED or FMT_DEINTERLEAVED_1PTR
        */              
        case STREAM_RUN:   
        {
            arm_stream_codec_instance *pinstance = (arm_stream_codec_instance *)instance;
            intPtr_t nb_data, data_buffer_size, bufferout_free;
            data_buffer_t *pt_pt;
            #define SAMP_IN uint8_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;   
            data_buffer_size     = pt_pt->size;
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 
            bufferout_free        = pt_pt->size;

            nb_data = data_buffer_size / sizeof(SAMP_IN);

            //processing(pinstance, inBuf, nb_data, outBuf);

            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_OUT);   /* amount of data produced */

            
            break;
        }



        /* func(command = STREAM_STOP, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_STOP:  break;    
    }
}


struct CodecState
{
	int valprev;
	int index;
};

void encode(struct CodecState* state, int16_t* input, uint32_t numSamples, uint8_t* output);
void decode(struct CodecState* state, uint8_t* input, uint32_t numSamples, int16_t* output);

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
void encode(struct CodecState* state, int16_t* input, uint32_t numSamples, uint8_t* outp)
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

void decode(struct CodecState* state, uint8_t* input, uint32_t numSamples, int16_t* output)
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


#ifdef __cplusplus
}
#endif