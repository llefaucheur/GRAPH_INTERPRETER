/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
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


#include <stdint.h>
#include "platform_computer/platform_computer.h"
#include "../stream_src/stream_const.h"      

#include "stream_types.h"  
#include "stream_extern.h"

SECTION_START
 /*  
    data in RAM : a single Graph interpreter  instance is in charge of one io
    the only extra SRAM area needed to interface with Graph interpreter is the address of the graph

 */
arm_stream_instance_t * platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];


/**
  @brief         Platform abstraction layer
  @param[in]     command    operation to do
  @param[in]     ptr1       1st data pointer 
  @param[in/out] ptr2       2nd data pointer 
  @param[in]     data3      3rd parameter integer
  @return        none

  @par           platform abstraction layer
                   Interface to platform-specific stream controls (set, start, stop)
                   and flags used during the initialization sequence
  @remark
 */

void platform_al(uint32_t command, void *ptr1, void *ptr2, uint32_t data3)
{   
    static uint8_t stream_initialization_done;
    static uint8_t stream_mp_synchro[MAX_NB_STREAM_INSTANCES];

    switch (command)            /*  */
    {

    /* platform_al (PLATFORM_MP_GRAPH_SHARED, start_address,end_address,0); */
    case PLATFORM_INIT_AL: 
    {   /* define MPU_RBAR[Region] */
        /* set MPU_RASR[18] = 1 : the memory area is shared*/
        break;
    }

    /*  who am i ? and which ISR is connected to NVIC (iomask) 
        platform_al (PLATFORM_PROC_HW, (struct HW_params *)&params, 0, 0);
    */
    case PLATFORM_PROC_HW:  
    {   struct HW_params *params;
        params = (struct HW_params *)ptr1;
        platform_specific_processor_arch_iomask(&params);
      
        break;
    }

    /* platform_al (PLATFORM_OFFSETS, intPtr_t **,0,0); */
    case PLATFORM_OFFSETS: 
    {   extern intPtr_t long_offset[MAX_NB_MEMORY_OFFSET];
        extern void platform_specific_long_offset(intPtr_t long_offset[]);
        intPtr_t **D; 
        intPtr_t *S;

        platform_specific_long_offset(long_offset);
        D = (intPtr_t **)ptr1;
        S = long_offset;
        *D = S;
        break;
    }

    /* platform_al (PLATFORM_MP_GRAPH_SHARED, start_address,end_address,0); */
    case PLATFORM_MP_GRAPH_SHARED: 
    {   /* define MPU_RBAR[Region] */
        /* set MPU_RASR[18] = 1 : the memory area is shared*/
        break;
    }

    /* platform_al (PLATFORM_MP_BOOT_WAIT, uint_8_t *wait, 0,0); */
    case PLATFORM_MP_BOOT_WAIT: 
    {   uint8_t *wait=ptr1;
        *wait = (stream_initialization_done == 0u)? 1u : 0u;
        break;
    }

    /* platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); */
    case PLATFORM_MP_BOOT_DONE: 
    {   DATA_MEMORY_BARRIER;
        stream_initialization_done = 1;
        break;
    }

    /* platform_al (PLATFORM_MP_RESET_DONE,0,0,instance_idx); */
    case PLATFORM_MP_RESET_DONE: 
    {   stream_mp_synchro[data3] = 1;
        DATA_MEMORY_BARRIER;
        break;
    }

    /* platform_al (PLATFORM_MP_RESET_WAIT, &wait, 0, total_nb_stream_instance); */
    case PLATFORM_MP_RESET_WAIT: 
    {   uint8_t *wait;
        uint8_t nb;
        uint8_t i;
        wait = ptr1;
        nb = (uint8_t)data3;
                
        DATA_MEMORY_BARRIER;

        /* all the instances must have set their flag at "1" to tell "reset completed" */
        *wait = 1;
        for (i = 0; i < nb; i++)
        { *wait &= stream_mp_synchro[i];
        }
        break;
    }

    /* platform_al (PLATFORM_IO_STRUCT,  struct platform_io_control *, 0, 0); */
    case PLATFORM_IO_STRUCT :
    {   
        extern struct platform_io_control platform_io [LAST_IO_FUNCTION_PLATFORM];
        *(uint8_t **)ptr1 = (uint8_t *)(&platform_io);
        break;
    }


    /* platform_al (PLATFORM_EXEC_TIME, *delta_time, 0,0); */
    case PLATFORM_EXEC_TIME:        
    {   break;
    }

    /* platform_al (PLATFORM_ERROR, uint_8_t type, uint_8_t data,0); */
    case PLATFORM_ERROR:  
    {   //uint8_t *type;
        //uint8_t *error;
        //type=ptr1;
        //error=ptr2;
        *(uint8_t *)ptr2=0;
        break;
    }

    /* platform_al (PLATFORM_CLEAR_BACKUP_MEM, 0,0,0); cold start : clear backup memory */
    case PLATFORM_CLEAR_BACKUP_MEM: 
    break;

    /* receive the index of a node(data3), returns its physical address(ptr1) */
    case PLATFORM_NODE_ADDRESS:
    {   
        extern p_stream_node node_entry_point_table[];
        p_stream_node tmp; 
        tmp = node_entry_point_table[data3]; 
        *(p_stream_node *)ptr1 = tmp; 
        break;
    }


    /* platform_al (PLATFORM_DEEPSLEEP_ENABLED, 0, 0, (stream_time32)sleep_interval); */
    case PLATFORM_DEEPSLEEP_ENABLED:
        break;

    //    -timer control(default implementation with SYSTICK)
    case PLATFORM_TIME_SET:
        /* @@@ set timer (SW timer) */
    case PLATFORM_RTC_SET:
    case PLATFORM_TIME_READ:
    {   /* if the HAL is not ready then use a counter of calls */
        /* return the time from boot/UTC/local-time 
            converted to MMDDHHMMSS*/
        break;
    }

    /* interface to push DATA to remote servers
        SendToUDP,<IP address>,<Portnumber>,<command>
    */
    case PLATFORM_REMOTE_DATA:
    default:
        break;
    }
}

SECTION_STOP
