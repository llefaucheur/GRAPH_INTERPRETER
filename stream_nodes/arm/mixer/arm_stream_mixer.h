/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_mixer.h
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
   

#ifndef cARM_STREAM_MIXER_H
#define cARM_STREAM_MIXER_H


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  



/*
    MIXER INSTANCE :
        configuration bit-field and for each input arc
*/
typedef struct
{
#define  __UNUSED_MIXER_MSB 31 
#define  __UNUSED_MIXER_MSB 31  
#define     OUTPUT_GAIN_MSB 30  
#define     OUTPUT_GAIN_LSB 14  /* 17 format Q6.11 (-66dB .. +36dB) 0x00800 = 0dB) */  
#define        SLOPE_UP_MSB 13  
#define        SLOPE_UP_LSB 11  /* 3  slope of gain ramp-dup */
#define      SLOPE_DOWN_MSB 10  
#define      SLOPE_DOWN_LSB  8  /* 3  slope of gain ramp-down */
#define    HQOS_INPUTID_MSB  7   
#define    HQOS_INPUTID_LSB  4  /* 4  HQoS input, 15=none */     
#define      NBINPUTARC_MSB  3       
#define      NBINPUTARC_LSB  0  /* 4  nb input arcs */     

#define MIXER_HQOS_ARC_ID ((1<<(HQOS_OUTPUTID_MSB - HQOS_OUTPUTID_LSB +1))-1) 
#define PACKMIXERCONFIG(nidx,nin,nout,Hin,Hout) (((nidx) << 14) | ((Hout)<<10) | ((Hin)<<6) | ((nout)<<3) | (nin))

    uint32_t configuration;


#define MUTED_CHANID_OUT_MSB 31  
#define MUTED_CHANID_OUT_LSB 31  /*  1 muted */   
#define       INPUT_GAIN_MSB 15  
#define       INPUT_GAIN_LSB 14  /* 17 format Q6.11 (-66dB .. +36dB) 0x00800 = 0dB) */  
#define MIXER_CHANID_OUT_MSB 13  
#define MIXER_CHANID_OUT_LSB  9  /*  5 output mixer arc index  */  
#define  MIXER_CHANID_IN_MSB  8  
#define  MIXER_CHANID_IN_LSB  4  /*  5 input sub-channel index -1  */  
#define   MIXER_ARCID_IN_MSB  3  
#define   MIXER_ARCID_IN_LSB  0  /*  4 input mixer arc index 0..15 */ 

#define PACKMIXERCHANNELS(ARCID_IN,CHANID_IN,CHANID_OUT,GAIN) \
    (((GAIN)<<INPUT_GAIN_IN_LSB) | ((CHANID_OUT)<<MIXER_CHANID_OUT_LSB) | \
    ((CHANID_IN)<<MIXER_CHANID_IN_LSB) | (ARCID_IN)<<MIXER_ARCID_IN_LSB)


/* memory reservation assumption = 16 input arcs mono, or 8 stereo */
#define MAXMIXERINDEXES (1<<(NBINPUTARC_MSB - NBINPUTARC_LSB +1))
    uint32_t mixer_parameter [MAXMIXERINDEXES];

} arm_mixer_instance;

extern void arm_stream_mixer_process (arm_mixer_instance *instance, void *input, void *output, intPtr_t *nsamp);



#endif

#ifdef __cplusplus
}
#endif
 