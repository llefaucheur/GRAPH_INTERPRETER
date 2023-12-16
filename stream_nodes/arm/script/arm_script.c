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

/*
    command  = reset/run + nbreg + stack size
    instance = pointer to the static area (the base address of the arc)
    data     = byte codes
*/
void arm_script (int32_t command, stream_handle_t registers, stream_xdmbuffer_t *byte_code, uint32_t *unused)
{
    uint32_t *stack, *parameter;
    stack = (uint32_t*)registers;
    stack = &(stack[RD(command, NREG_SCRIPT)]);
    parameter = &(stack[RD(command, STACK_SIZE_SCRIPT)]);

    switch (RD(command,COMMAND_CMD))
    { 
        /* jump flag set to 0, comparison flag set to 1, stack pointer set to 1 */
        case STREAM_RESET: 
        {   
            stack[0] = (1 + RD(command, NREG_SCRIPT)) + (1 << COMP_SCT_LSB);
            break;
        }    

        /* byte-code execution, reset the stack pointer */         
        case STREAM_RUN:   
        {   
            ST(stack[0], SP_SCT, 1 + RD(command, NREG_SCRIPT));
            arm_script_interpreter ((uint8_t *)byte_code, (script_register_t *)registers, stack, parameter);
            break;
        }
        default: break;
    }
}

#ifdef __cplusplus
}
#endif
    
