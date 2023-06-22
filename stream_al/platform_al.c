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
#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  

 
 /*  
    data in RAM : a single Stream instance is in charge of one io
    the only extra SRAM area needed to interface with Stream is the address of the graph

 */
arm_stream_instance_t * platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];

/* 
    See platform_sensor.h for the bit-field meaning per domain
*/
const uint8_t platform_audio_out_bit_fields[] = { 3,4,2,3,4,2,1,2,1,2,1,2,1 };


/**
  @brief        Extract setting fields
  @param[in]    *bit_field list of the bit-fields used for this IO domain
  @param[in]    settings   the specific setting of this IO interface
  @param[in]    line       the setting to have access to
  @param[in]    index      the selected index 
  @return       int32      the extracted field

  @par          Each IO interface is associated to a "domain" of operation. Each domain
                is set with default values at reset, or a list of proposed options. 

  @remark       
 */
int32_t extract_sensor_field (const uint8_t *platform_bit_fields, 
                              const int32_t *settings,
                              uint8_t setting_line,
                              uint8_t index)
{
    uint8_t i, j, i_field, nb_fields;

    i_field = 0;

    for (i = 0; i < setting_line; i++)
    {   nb_fields = settings[i_field];
        for (j = 0; j < nb_fields; j++)
        {
            /* TBC */
        }
        i_field = i_field + nb_fields;
    }
    return 3;
}



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

void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t data3)
{   
    static uint8_t stream_boot_lock;        
    static uint8_t stream_initialization_done;
    static uint8_t stream_reset_synchro[MAX_NB_STREAM_INSTANCES];
    static uint32_t platform_time;

    switch (command)            /*  */
    {

    /* platform_al (PLATFORM_PROC_ID, uint_8_t *procID, uint_8_t *archID,0); */
    case PLATFORM_PROC_ID:  
    {   uint8_t *proc;
        uint8_t *arch;
        proc=ptr1;
        arch=ptr2;
        
        /* [1,2,3] processor architectures 0="any" 1="commander processor architecture" */
        *arch = 1;

        /* processor index [0..7] for this architecture 0="commander processor" */  
        *proc = 0;
        break;
    }

    /* platform_al (PLATFORM_OFFSETS, intPtr_t **,0,0); */
    case PLATFORM_OFFSETS: 
    {   extern intPtr_t long_offset[NB_MEMINST_OFFSET];
        extern void platform_specific_long_offset(void);
        intPtr_t **D; 
        intPtr_t *S;

        platform_specific_long_offset();
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

    /* platform_al (PLATFORM_MP_SYNCHRO, *instance index, *copy allowed for you, 0); */
    case PLATFORM_MP_BOOT_SYNCHRO: 
    {   uint8_t check;
        uint8_t instance_index;
        uint8_t *copy_to_do;
        instance_index = *(uint8_t *)ptr1;
        copy_to_do=ptr2;

        /* if the graph copy is on-going, return*/
        RD_BYTE_MP_(check, &stream_boot_lock);
        if (check != 0u)
        {   *copy_to_do = 0;
            break;
        }

        /* reservation attempt */
        if (0 == WR_BYTE_AND_CHECK_MP_(&stream_boot_lock, instance_index))
        {   *copy_to_do = 0;
            break;
        }

        /* no collision */
        *copy_to_do = 1;
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
    {   stream_reset_synchro[data3] = 0;
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
        { *wait &= stream_reset_synchro[i];
        }
        break;
    }

    /* interface callback to arm_stream_io : 
            platform_al(PLATFORM_IO_ACK, data_ptr1, 0, PACK_PARAM_AL3(fw_idx, size));*/
    case PLATFORM_IO_ACK:  
    {
        uint32_t fw_idx;
        uint32_t size;
        arm_stream_instance_t *stream_instance;
        fw_idx = UNPACK_PARAM_AL3_FWIDX(data3);
        size = UNPACK_PARAM_AL3_SIZE(data3);
        stream_instance = platform_io_callback_parameter[fw_idx];
        arm_stream_io (fw_idx, stream_instance, ptr1, size);
        break;
    }

    /* platform_al (PLATFORM_IO_SET_STREAM, struct platform_control_stream *, 0,0); */
    case PLATFORM_IO_SET_STREAM :   
    case PLATFORM_IO_DATA :
    case PLATFORM_IO_STOP_STREAM : 
	{   struct platform_control_stream *parameters;
        extern struct platform_io_control platform_io [LAST_IO_FUNCTION_PLATFORM];
        struct platform_io_control *io_manifest;
        data_buffer_t *buffer;
        io_function_control_ptr io_func;
        uint32_t fw_idx;
        uint32_t *settings;
        
        parameters = (struct platform_control_stream *)ptr1; 
        buffer = &(parameters->buffer);
        fw_idx = parameters->fw_idx;
        io_manifest = &(platform_io[fw_idx]);
        settings = &(parameters->domain_settings);

        switch (command)
        {
        case PLATFORM_IO_SET_STREAM:
            platform_io_callback_parameter [parameters->fw_idx] = parameters->instance;
            io_func = io_manifest->io_set;
            (*io_func)(settings, (uint8_t *)(buffer->address), (uint32_t)(buffer->size));
            break;

        case PLATFORM_IO_DATA :
            io_func = io_manifest->io_start;
            (*io_func)(settings, (uint8_t *)(buffer->address), (uint32_t)(buffer->size));
            break;

        case PLATFORM_IO_STOP_STREAM:
            io_func = io_manifest->io_stop;
            (*io_func)(settings, (uint8_t *)(buffer->address), (uint32_t)(buffer->size));
            break;

        default:
            break;
        }
	    break; 
    }

    /* platform_al (PLATFORM_EXEC_TIME, *delta_time, 0,0); */
    case PLATFORM_EXEC_TIME:        
    {   break;
    }

    /* platform_al (PLATFORM_ERROR, uint_8_t type, uint_8_t data,0); */
    case PLATFORM_ERROR:  
    {   uint8_t *type;
        uint8_t *error;
        type=ptr1;
        error=ptr2;
        break;
    }

    /* platform_al (PLATFORM_CLEAR_BACKUP_MEM, 0,0,0); cold start : clear backup memory */
    case PLATFORM_CLEAR_BACKUP_MEM: 
    break;

    case PLATFORM_TIME_SET:
        break;

    case PLATFORM_RTC_SET:
        break;

    case PLATFORM_TIME_READ:
    {   /* if the HAL is not ready then use a counter of calls */
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

