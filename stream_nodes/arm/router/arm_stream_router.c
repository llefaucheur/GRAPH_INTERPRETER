/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_router.c
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
   


#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_router.h"

extern void router_processing (arm_stream_router_instance *instance, stream_xdmbuffer_t *in_out);

/*
;----------------------------------------------------------------------------------------
;Node #2.	arm_stream_router
;----------------------------------------------------------------------------------------
;    I/O streams have the same RAW format size, if time-stamped then only with frame size of 1
;    
;
;
;   table of routing : a list of 16bits words (source 8b + destination 8b)
;   format: router arc index (3b)           up to 8 arcs
;           sub-channel index -1 (5b)       up to 32 channels
;
;   example with the router_2x2 used to merge 2 stereo input in a 4-channels output
;
arm_stream_router
    3  i8; 0 0 0        instance, preset, tag
    PARSTART
    2  i8; 2 2          nb input/output arcs
    4 i16; 0 0 2 0      move arc0,chan0, to arc2,chan0
    4 i16; 0 1 2 1      move arc0,chan1, to arc2,chan1
    4 i16; 1 0 2 2      move arc1,chan0, to arc2,chan2
    4 i16; 1 1 2 3      move arc1,chan1, to arc2,chan3
    PARSTOP
*/

//const uint16_t router_preset1_bypass [] = 
//{   {PACKROUTERCHANNELS(2,0,0,0), },   /* preset #1 = bypass two mono input channel(0,1) to the output(2,3) */
//    {PACKROUTERCHANNELS(3,0,1,0), },   
//};
//
//const uint16_t router_preset2_swap [] = 
//{   {PACKROUTERCHANNELS(3,0,0,0), },   /* preset #2 = swap two mono input channel(0,1) to the output(3,2) */
//    {PACKROUTERCHANNELS(2,0,1,0), },   
//};
//
//const uint16_t router_preset3_demux [] = 
//{   {PACKROUTERCHANNELS(2,0,0,0), },   /* preset #3 = demux stereo input channel(0) to two mono output(2,3) */
//    {PACKROUTERCHANNELS(3,0,0,1), },   
//};
//
//const uint16_t router_preset4_merge [] = 
//{   {PACKROUTERCHANNELS(2,0,0,0), },   /* preset #4 = merge two mono input channel(0,1) to one stereo output(2) */
//    {PACKROUTERCHANNELS(2,1,1,0), },   
//};
//
//const uint16_t router_preset5_duplicate [] = 
//{   {PACKROUTERCHANNELS(2,0,0,0), },   /* preset #5 = input arc(0) duplicated to the two output arcs(1,2) */
//    {PACKROUTERCHANNELS(3,0,0,0), },   /*  input arc(1) is unused */
//};


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_router (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                    => used by the router to know the size of each raw element 
                    (see stream_bitsize_of_raw())
                memory pointers are in the same order as described in the NODE manifest

                the number of arcs (NARC_CMD) is used to configure arm_stream_router_instance.configuration
        */
        case STREAM_RESET: 
        {   stream_al_services *stream_entry = (stream_al_services *)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_stream_router_instance *pinstance = (arm_stream_router_instance *) *memresults;
            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
                *instance, 
                data = parameters
        */ 
        case STREAM_SET_PARAMETER:  
        {   uint8_t *pt8bsrc, *pt8bdst;
            uint16_t i, n;
            
            arm_stream_router_instance *pinstance = (arm_stream_router_instance *) instance;

            /* copy the parameters from preset or from the graph */
            pt8bsrc = (uint8_t *) data;
            pinstance->configuration = (uint32_t)(*((uint32_t *)pt8bsrc)); 
            pt8bsrc = pt8bsrc+2;    /* copy of 2 uint16 */

            n = RD(pinstance->configuration, NB_ROUTER_INDEX);
            pt8bdst = (uint8_t *)(pinstance->router_parameter);

            for (i = 0; i < n; i++)
            {   pt8bdst[i] = pt8bsrc[i];
            }
            break;
        }

        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input size} {*output size}]
        */         
        case STREAM_RUN:   
        {
            arm_stream_router_instance *pinstance = (arm_stream_router_instance *) instance;
            stream_xdmbuffer_t *pt_pt;

            pt_pt = data;
            arm_stream_router_process (pinstance, pt_pt);
            break;
        }

        default :
            break;
    }
}

#ifdef __cplusplus
}
#endif
 