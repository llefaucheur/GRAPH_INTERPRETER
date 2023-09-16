/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_router.h
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
   

#ifndef cARM_STREAM_ROUTER_H
#define cARM_STREAM_ROUTER_H


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  


/*
    ROUTER INSTANCE :
        pointer to the scheduler list of services
        configuration bit-field (nb arcs)
        list of routing indexes
*/
typedef struct
{
    stream_services_entry *stream_entry;

#define          UNUSED_MSB 31 
#define          UNUSED_LSB 22
#define NB_ROUTER_INDEX_MSB 21  /* 6 bits : max 64 indexes/PACKROUTERCHANNELS */
#define NB_ROUTER_INDEX_LSB 16    

#define     NBOUTPUTARC_MSB 7   /* 4bits  nb output arcs */     
#define     NBOUTPUTARC_LSB 4
#define      NBINPUTARC_MSB 3   /* 4bits  nb input arcs */     
#define      NBINPUTARC_LSB 0

    uint32_t configuration;
#define PACKROUTERCONFIG(nidx,nin,nout,Hin,Hout) (((nidx) << 14) | ((Hout)<<10) | ((Hin)<<6) | ((nout)<<3) | (nin))



#define ROUTER_CHANID_OUT_MSB 15    /* output sub-channel index -1 (5b) */
#define ROUTER_CHANID_OUT_LSB 11
#define  ROUTER_ARCID_OUT_MSB 10    /* output router arc index (3b) */
#define  ROUTER_ARCID_OUT_LSB 8
#define  ROUTER_CHANID_IN_MSB 7     /* input sub-channel index -1 (5b) */
#define  ROUTER_CHANID_IN_LSB 3
#define   ROUTER_ARCID_IN_MSB 2     /* input router arc index (3b) */
#define   ROUTER_ARCID_IN_LSB 0
#define PACKROUTERCHANNELS(arcOUT,subcOUT, arcIN,subcIN) (((subcOUT)<<11) | ((arcOUT)<<8) | ((subcIN)<<3) | (arcIN))


#define MAXROUTERINDEXES (1<<(NB_ROUTER_INDEX_MSB - NB_ROUTER_INDEX_LSB +1))
    uint16_t router_parameter [MAXROUTERINDEXES];

} arm_stream_router_instance;

extern void arm_stream_router_process (arm_stream_router_instance *instance, stream_xdmbuffer_t *in_out);

#endif

#ifdef __cplusplus
}
#endif
 