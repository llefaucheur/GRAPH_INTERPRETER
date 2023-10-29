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
#include "stream_extern.h"


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
    static uint32_t *graph;

    static uint8_t stream_boot_lock;        
    static uint8_t stream_initialization_done;
    static uint8_t stream_mp_synchro[MAX_NB_STREAM_INSTANCES];
    static uint32_t platform_time;

    switch (command)            /*  */
    {

    /* platform_al (PLATFORM_MP_GRAPH_SHARED, start_address,end_address,0); */
    case PLATFORM_INIT_AL: 
    {   /* define MPU_RBAR[Region] */
        /* set MPU_RASR[18] = 1 : the memory area is shared*/
        break;
    }

    /*  who am i ? and which ISR is connected to NVIC (iomask) 
        platform_al (PLATFORM_PROC_ID, uint_8_t *ArchProcID, uint_8_t *IOMask, 0); 
    */
    case PLATFORM_PROC_HW:  
    {   uint8_t *ArchProcID, procID, archID;
        ArchProcID=ptr1;

        platform_specific_processor_arch_iomask(&procID, &archID, (uint32_t *)ptr2);
        
        /* @@@@  call specific HW implementation of the processor ID */
        /* [1,2,3] processor architectures 0="any" 1="commander processor architecture" */
        /* processor index [0..7] for this architecture 0="commander processor" */  

        /* PROCID_LW0_MSB U(22) */
        /* PROCID_LW0_LSB U(20) */
        /* ARCHID_LW0_MSB U(19) */
        /* ARCHID_LW0_LSB U(17) */

        *ArchProcID = archID | (procID<<3); /* 3bits + 3bits */

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


    /* platform_al (PLATFORM_MP_SERVICE_LOCK, *instance index, *result, 0); */
    case PLATFORM_MP_SERVICE_LOCK: 
    {   
    
        uint8_t instance_index = *ptr1;
        uint8_t *result = ptr2;

        /* 
            TBD @@@
            use the MSB bits of each instance 27bits address (before the arcs descriptors)
            for this synchronization requiring a small number of cycles (malloc/free of services):
            1) set my own sync bit
            2) check all the other are 0, if Yes continue, otherwise reset my bit, and loop to 1)
        */
        
        *result = 1;
        break;
    }


    /* platform_al (PLATFORM_MP_SERVICE_LOCK, *instance index, *result, 0); */
    case PLATFORM_MP_SERVICE_UNLOCK: 
    {
        uint8_t instance_index = *ptr1;
        uint8_t *result = ptr2;
        
        *result = 1;
        break;
    }

    /* interface callback to arm_stream_io : 
            platform_al(PLATFORM_IO_ACK, data_ptr1, 0, PACK_PARAM_AL3(fw_io_idx, size));*/
    case PLATFORM_IO_ACK:  
    {
        uint32_t fw_io_idx;
        uint32_t size;
        arm_stream_instance_t *stream_instance;
        fw_io_idx = UNPACK_PARAM_AL3_FWIDX(data3);
        size = UNPACK_PARAM_AL3_SIZE(data3);
        stream_instance = platform_io_callback_parameter[fw_io_idx];
        arm_stream_io (fw_io_idx, stream_instance, ptr1, size);
        break;
    }

    /* platform_al (PLATFORM_IO_SET_STREAM, struct platform_control_stream *, 0,0);

    device driver Abstraction Layer (io_set, io_start, io_stop)
    -----------------------------------------------------------
    
    1) The application and CMSIS-Stream know from the graph data (compilation of IO manifests made during the graph creation),  
        the location of the stream buffer, the settings, the type of data move to operate once being calledback from the 
        device driver (data to copy or processing the data "in place").

    2) "set" The application calls "io_set()" for the stream configuration with calling parameters: the domain settings, 
        a proposed buffer address and size (if needed), ID (the device driver index). The data transfer is starting from 
        this point when the IO is master on the interface. 
        "RX" means a flow of data from external sensor to the graph. 
        "TX" means a flow of data generated by the graph to the external transducer.

    3) "start" is used when the IO interface is slave:  the Abstraction Layer will be called by the Stream scheduler 
        to fill/empty the FIFO. The abstraction layer calls the "start" function and set a bit to tell a transfer is on-going, 
        the Stream scheduler must wait before reinitiating a data transfer. "io_start()" has the parameters : a pointer to 
        the base address of buffer, the size in bytes, the ID. When the IO interface is master there are only "acknowledge" calls.
        RX case : "this memory address and buffer size, will be used for receiving new data".  
        TX case : "this base address and buffer size needs to be transmitted".

    4) "acknowledge" Either IO is master or slave, once new data are ready (or have just been transferred out), 
        the device driver calls the callback (arm_stream_io()) with the parameters : a pointer to a base address of a buffer, 
        the size in bytes, the ID:
        RX case : "I prepared for you a buffer of data, copy the data or use it directly from this place, and for this amount of bytes". 
            The address can change from last callback in case the device driver is master and using a ping-pong buffer protocol.
        TX case : IO is master case : "I have completed the last transfer, you can fill this buffer for the next transfer 
            (data copy case), ". IO is slave case : "I have completed the transfer of this buffer you told me to move out with this 
            amount of bytes, you can reset the flag telling the transfer is on-going". 

    5) "stop" The application ends up the use-case with a call to io_stop(). The device driver acknowledges the end of on-going 
        transfers with a call to the callback, with no parameter.


    Sequence of operations
    ----------------------
    The first field in the graph is "stream_format_io" (2 word per IO)
        Word0: ARC ID, domain, io platform index, in/out, command parameter, format
        Word1: default 32bits mixed-signal settings bit-fields
        Reset :
            set_stream.fw_io_idx = RD(stream_format_io[all IOs], FW_IO_IDX_IOFMT);
            if (IO_COMMAND_SET_BUFFER == TEST_BIT(stream_format_io, IOCOMMAND_IOFMT_LSB)) buffer ptr is the arc buffer
            else IO_COMMAND_DATA_MOVE
        Scheduler loops :
            Check IOs : if arc_ready_for_write(S, arc, &size);
                platform_al(PLATFORM_IO_DATA, [instance; domain_settings; RD(*pio, FW_IO_IDX_IOFMT; xdmbuffer;], 0u, 0u);

                in "platform_computer.c"
                    struct platform_io_control platform_io [LAST_IO_FUNCTION_PLATFORM] = 
                    ...  {   PLATFORM_AUDIO_OUT_INSTANCE_0
                            .io_set = audio_render_set_stream,
                            .io_start = audio_render_start_data_move,
                            .io_stop = audio_render_stop_stream,
                            .stream_setting = audio_render_settings,
                    ... }

                io_manifest = &(platform_io[fw_io_idx]);
                case PLATFORM_IO_SET_STREAM:
                    platform_io_callback_parameter [parameters->fw_io_idx] = parameters->instance;          Instance => callback
                    (io_manifest->io_set)(settings, (uint8_t *)(buffer->address), (uint32_t)(buffer->size));
                case PLATFORM_IO_DATA :
                    (io_manifest->io_start)(settings, (uint8_t *)(buffer->address), (uint32_t)(buffer->size));
                        FILE access .. audio_ap_rx_transfer_done ((uint8_t *)data, size);
                            platform_al(PLATFORM_IO_ACK, data, 0, PACK_PARAM_AL3(PLATFORM_DATA_STREAM_IN_INSTANCE_0, size));
                                case PLATFORM_IO_ACK:  
                                {
                                    fw_io_idx = UNPACK_PARAM_AL3_FWIDX(data3);
                                    size = UNPACK_PARAM_AL3_SIZE(data3);
                                    stream_instance = platform_io_callback_parameter[fw_io_idx];

                                    "stream_io.c"
                                    arm_stream_io (fw_io_idx, stream_instance, ptr1, size);
                                        if (IO_COMMAND_SET_BUFFER != TEST_BIT(*pio, IOCOMMAND_IOFMT_LSB))
                                            arc_data_operations (stream_instance, arc, arc_IO_move_to_arc, data, data_size);   
                                        else IO_COMMAND_SET_BUFFER
                                            arc_data_operations (stream_instance, arc, arc_set_base_address_to_arc, data, data_size);    
                                        ..     data transfer done when the IO is slave 
                                        CLEAR_BIT_MP(stream_instance->ioreq, REQMADE_IO_LSB);
                case PLATFORM_IO_STOP_STREAM:
                    (*io_manifest->io_start)(settings, (uint8_t *)(buffer->address), (uint32_t)(buffer->size));


*/

    case PLATFORM_IO_SET_STREAM :   
    case PLATFORM_IO_DATA :
    case PLATFORM_IO_STOP_STREAM : 
	{   struct platform_control_stream *parameters;
        extern struct platform_io_control platform_io [LAST_IO_FUNCTION_PLATFORM];
        struct platform_io_control *io_manifest;
        stream_xdmbuffer_t *buffer;
        io_function_control_ptr io_func;
        uint32_t fw_io_idx;
        uint32_t *settings;
        
        parameters = (struct platform_control_stream *)ptr1; 
        buffer = &(parameters->buffer);
        fw_io_idx = parameters->fw_io_idx;
        io_manifest = &(platform_io[fw_io_idx]);
        settings = &(parameters->domain_settings);

        switch (command)
        {
        case PLATFORM_IO_SET_STREAM:
            platform_io_callback_parameter [parameters->fw_io_idx] = parameters->instance;
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

    /* receive the index of a node(data3), returns its physical address(ptr1) */
    case PLATFORM_NODE_ADDRESS:
    {   
        extern p_stream_node node_entry_point_table[];
        p_stream_node tmp; 
        tmp = node_entry_point_table[data3]; 
        *(p_stream_node *)ptr1 = tmp; 
        break;
    }

    //Default callback(stream_script_callback)
    //    - sleep / deep - sleep activation
    //    - system regsters access : who am I ?
    //    -timer control(default implementation with SYSTICK)

    case PLATFORM_DEEPSLEEP_ENABLED:
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

