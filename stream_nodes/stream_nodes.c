/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_synchro_process.c
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
   
    


#include "stream_const.h"
#include "stream_types.h"


/* -----------------------------------
    Full node descriptions given in ./stream_tools/TEMPLATE_GRAPH.txt
*/

extern p_stream_node arm_script;                /*  0*/
extern p_stream_node arm_stream_router;         /*  1*/
extern p_stream_node arm_stream_converter;      /*  2*/
extern p_stream_node arm_stream_amplifier;      /*  3*/
extern p_stream_node arm_stream_mixer;          /*  4*/
extern p_stream_node arm_stream_filter;         /*  5*/
extern p_stream_node arm_stream_detector;       /*  6*/
extern p_stream_node arm_stream_rescaler;       /*  7*/
extern p_stream_node arm_stream_compressor;     /*  8*/
extern p_stream_node arm_stream_decompressor;   /*  9*/
extern p_stream_node arm_stream_modulator;      /* 10*/
extern p_stream_node arm_stream_demodulator;    /* 11*/
extern p_stream_node arm_stream_interpolator;   /* 12*/
extern p_stream_node arm_stream_synchro;        /* 13*/
extern p_stream_node arm_stream_share;          /* 14*/
                                                /* 15*/
#define TBD 0

/* 
    this table will be extended with pointers to nodes loaded 
    dynamically and compiled with position independent execution options

    it is aligned with the list of SOFTWARE COMPONENTS MANIFESTS in "files_manifests_computer.txt"
*/
p_stream_node node_entry_point_table[NB_NODE_ENTRY_POINTS] =
{
    /*--------- ARM ---------*/
    /*  0*/ (void *)0,                          /* node disabled */
    ///*  1*/ (void *)&arm_script,                /* byte-code interpreter */
    ///*  2*/ (void *)&arm_stream_router,         /* copy input arcs and subchannel and output arcs and subchannels   */     
    ///*  3*/ (void *)&arm_stream_converter,      /* raw data format converter */
    ///*  4*/ (void *)&arm_stream_amplifier,      /* amplifier mute and un-mute with ramp and delay control */
    ///*  5*/ (void *)&arm_stream_mixer,          /* multichannel mixer with mute/unmute and ramp control */
    /*  6*/ (void *)&arm_stream_filter,         /* cascade of DF1 filters */
    /*  7*/ (void *)&arm_stream_detector,       /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
    ///*  8*/ (void *)&arm_stream_rescaler,       /* raw data values remapping using "interp1" */
    ///*  9*/ (void *)&arm_stream_compressor,     /* raw data compression with adaptive prediction */
    ///* 10*/ (void *)&arm_stream_decompressor,   /* raw data decompression */
    ///* 11*/ (void *)&arm_stream_modulator,      /* signal generator with modulation */
    ///* 12*/ (void *)&arm_stream_demodulator,    /* signal demodulator, frequency estimator */
    ///* 13*/ (void *)&arm_stream_interpolator,   /* asynchronous sample-rate converter */
    ///* 14*/ (void *)&arm_stream_synchro,        /* raw data interpolator with synchronization with one HQoS stream */
    ///* 15*/ (void *)&arm_stream_share,          /* let a buffer be used by several nodes */

    /*------ COMPANY #1 -----*/
    TBD,                           
    /*------ COMPANY #2 -----*/
    TBD,TBD,TBD,
    /*------ COMPANY #3, etc ... -----*/
                
};
 
#ifdef __cplusplus
}
#endif
 