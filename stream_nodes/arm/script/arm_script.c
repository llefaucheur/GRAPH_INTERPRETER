/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_script.c
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


#include "stream_const.h"
#include "stream_types.h" 
#include "arm_script.h"

extern void arm_script_interpreter (uint32_t *instance, uint8_t reset);

void script_processing (uint32_t *instance)
{
}


/*
    scripts can have 
*/


void arm_script (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = 1;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(STREAM_RESET, "instance" = memory pointer of instance, "data" not used */
        case STREAM_RESET: 
        {   
            intPtr_t *memresults = (intPtr_t *)instance;
            uint32_t *pinstance = (uint32_t *) (memresults[0]);
            pinstance = (uint32_t *)(memresults[1]);
            break;

            /* no reset until we know where is the code (STREAM_SET_PARAMETER) */
            break;
        }    

        /* the byte-codes are in the parameter section */ 
        case STREAM_SET_PARAMETER:  
        {   
            uint32_t *pinstance = (uint32_t *)instance;
            pinstance = (uint32_t *)data;

            /* reset the script (conditional flag is set) */
            arm_script_interpreter(pinstance, 1);
            break;
        }

        /* byte-code execution */         
        case STREAM_RUN:   
        {
            uint32_t *pinstance = (uint32_t *)instance;
            arm_script_interpreter(pinstance, 0);
            break;
        }

        default: break;
    }
}

#ifdef __cplusplus
}
#endif
    
