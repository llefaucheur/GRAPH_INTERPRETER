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



#include "stream_const.h"      
#include "stream_types.h"  

    /*
        MIXER INSTANCE :
            configuration bit-field and for each input arc
    */
typedef struct
{
        /* word 0 : one word of 16bits + 2 bytes */
#define            SLOPE_W0_MSB 27  
#define            SLOPE_W0_LSB 24  /* 4  slope of gain ramp-down */
#define       NBINPUTARC_W0_MSB 19       
#define       NBINPUTARC_W0_LSB 16  /* 4  nb input arcs */     
#define OUTGAIN_MANTISSA_W0_MSB 11  
#define OUTGAIN_MANTISSA_W0_LSB  4  /* 8 FP_8m4em : 8b mantissa, e=4b exponent, gain=(m)>>(e+2) */  
#define OUTGAIN_EXPONENT_W0_MSB  3  
#define OUTGAIN_EXPONENT_W0_LSB  0  /* 4   */  

#define PACKMIXERCONFIG_W0(OM,OE,NB,SLOPE) \
    ((OM)<<OUTGAIN_MANTISSA_W0_LSB | (OE)<<OUTGAIN_EXPONENT_W0_LSB | \
     (NB)<<NBINPUTARC_W0_LSB  | (SLOPE)<<SLOPE_W0_LSB)


        /* word 1 : two words of 16bits */
#define         DELAY_UP_W1_MSB 31  
#define         DELAY_UP_W1_LSB 16 /* 16 delay before applying unmute state */   
#define       DELAY_DOWN_W1_MSB 15  
#define       DELAY_DOWN_W1_LSB 0  /* 16 delay before applying mute state */   

         /* word 2 x n_arc : one word of 16bits + 2 bytes */
#define     SUBCHANID_IN_W2_MSB 28 
#define     SUBCHANID_IN_W2_LSB 24  /* 5 input sub-channel index -1  */  
#define   MIXER_ARCID_IN_W2_MSB 20  
#define   MIXER_ARCID_IN_W2_LSB 16  /* 5 mixer arc index  */  
#define    GAIN_MANTISSA_W2_MSB 11  
#define    GAIN_MANTISSA_W2_LSB  4  /* 8 FP_8m4em : 8b mantissa, e=4b exponent, gain=(m)>>(e-2) */  
#define    GAIN_EXPONENT_W2_MSB  3  
#define    GAIN_EXPONENT_W2_LSB  0  /* 4   */  

#define PACKMIXERCHANNELS_W2(OM,OE,ARCID_IN,CHANID_IN) \
    ((OM)<<GAIN_MANTISSA_W2_LSB | (OE)<<GAIN_EXPONENT_W2_LSB | \
     (ARCID_IN)<<MIXER_ARCID_IN_W2_LSB | (CHANID_IN)<<SUBCHANID_IN_W2_LSB )


        /* word 3 x n_arc : 2 bytes */
#define     MUTED_CHANID_W3_MSB 8  
#define     MUTED_CHANID_W3_LSB 8  /* 1 muted */   
#define    SUBCHANID_OUT_W3_MSB 4 
#define    SUBCHANID_OUT_W3_LSB 0  /* 5 input sub-channel index -1  */  

#define PACKMIXERCHANNELS_W3(CHANID_OUT,MUTE) ((CHANID_OUT)<<SUBCHANID_OUT_W3_LSB| (MUTE)<<MUTED_CHANID_W3_LSB )


#define MaskM ((1<<(GAIN_MANTISSA_MSB - GAIN_MANTISSA_LSB +1)) -1)
#define MaskE ((1<<(GAIN_EXPONENT_MSB - GAIN_EXPONENT_LSB +1)) -1)
#define MULFPGAINW(x,W32)  (((long)(x) * ((W32)&MaskM)) >> (2+((W32>>GAIN_EXPONENT_LSB)&MaskE)))
#define MULFPGAINFP(x,M,Eplus2) (((long)(x) * (M)) >> (Eplus2))
#define LDFPGAINW(W32)  ( ((W32)&MaskM) << (22-(((W32)>>GAIN_EXPONENT_LSB)&MaskE)))

    
#define MAXMIXERINDEXES (1<<(NBINPUTARC_W0_MSB - NBINPUTARC_W0_LSB +1))
#define NB_W32_PARAMETERS (2 * MAXMIXERINDEXES + 1)
    uint32_t parameters [NB_W32_PARAMETERS];

#define TAG_CMD_SLOPE 1    
#define TAG_CMD_OUTPUT_GAIN 2
#define TAG_CMD_NB_INPUT 3

    /* 
        target gain is in FP_8m4em format, the mantissa is used to control the slope,
    
        +----------+
        |          |     +-----------------+
        | MANTISSA | --->+ left shift 23bit+---->(+)+--------------------------|
        |          |     +  - slope        +      ^                            |
        |          |     +-----------------+      |   +-------------+   +------v---------+
        |          |                              |   | left shift  |   |current gain in |
        | (target) |                              +---+ (16-slope)  +<--+congergence 32b |
        |          |                                  +-------------+   +------+---------+
        |          |                                                           |
        |          |                                                           |
        |          |                                                    +------v---------+
        | EXPONENT | --->--------------------------------------------->-|Extract 16b MSB +-----> M16 mantissa
        |          |                                                    |Exponent = 9+E  +       and exponent
        +----------+                                                    +----------------+
    */
    uint32_t mixer_current_mantissa [MAXMIXERINDEXES];

    /*
        multi-channel format 
    */
#define   ___MIXERCHAN_FMT_MSB 31  
#define   ___MIXERCHAN_FMT_LSB 30  
#define FRAMESIZE_FMT0_FMT_MSB 29   
#define FRAMESIZE_FMT0_FMT_LSB  6  /* 24 copy of arc's format FRAMESIZE_FMT0 */
#define   INTERLEAVING_FMT_MSB  5  
#define   INTERLEAVING_FMT_LSB  5  /* 1 interleaving "frame_format_type"  */  
#define NBCHANM1_MIXER_FMT_MSB  4  
#define NBCHANM1_MIXER_FMT_LSB  0  /* 5 up to 32 channels */  
    uint32_t channel_fmt [MAXMIXERINDEXES];

    /* CMSIS-STREAM "services" : debug traces, compute library, .. */
    stream_services_entry *services;

} arm_mixer_instance;

extern void arm_stream_mixer_process (arm_mixer_instance *instance, stream_xdmbuffer_t *xdm_buf, uint8_t narc);



#endif

#ifdef __cplusplus
}
#endif
 