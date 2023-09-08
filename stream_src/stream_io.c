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

#ifdef __cplusplus
 extern "C" {
#endif

#include "stream_const.h"    /* #include <stdint.h> */
#include "stream_types.h"

//#define L stream_instance->long_offset
//#define G stream_instance->graph

/**
  @brief         call back of the IO layers of the platform
  @param[in]     command    operation to do
  @param[in]     ptr1       1st data pointer 
  @param[in/out] ptr2       2nd data pointer 
  @param[in]     data3      3rd parameter integer
  @return        none

  @par           arm_stream_io do the data moves with arc descriptor update
                 or simple assign the base address of the ring buffer to the data (no data move)
                 check there is no flow error

                 RX case : "I prepared for you a buffer of data, copy the data or use it directly 
                 from this place, and for this amount of bytes". The address can change from last 
                 callback in case the device driver is commander and using a ping-pong buffer protocol.

                 TX case : IO is commander case : "I have completed the last transfer, you can fill 
                 this buffer for the next transfer". IO is slave case : "I have completed the 
                 transfer of this buffer you told me to move out using io_start() with this amount 
                 of bytes, you can reset the flag telling the transfer is on-going". 
  @remark
 */
void arm_stream_io (uint32_t fw_io_idx, 
            arm_stream_instance_t *stream_instance,
            uint8_t *data, uint32_t data_size)
{   
    uint32_t *arc;
    uint32_t *pio;

    pio = stream_instance->pio;
    pio = &(pio[STREAM_IOFMT_SIZE_W32 * fw_io_idx]);
    arc = stream_instance->all_arcs;
    arc = &(arc[SIZEOF_ARCDESC_W32 * RD(*pio, IOARCID_IOFMT)]);

    /* RX : stream to the graph */
    if (0 == TEST_BIT(*pio, RX0TX1_IOFMT_LSB))
    {   if (IO_COMMAND_SET_BUFFER != TEST_BIT(*pio, IOCOMMAND_IOFMT_LSB))
        {   uint32_t free_area; /* free area = size - write */
            free_area = RD(arc[1], BUFF_SIZE_ARCW1) - RD(arc[3], WRITE_ARCW3);
            
            if (free_area < data_size)
            {   /* overflow issue */
                platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                //flow_error = (uint8_t) RD(arc[2], OVERFLRD_ARCW2);
                data_size = free_area;
            }

            arc_data_operations (stream_instance, arc, arc_IO_move_to_arc, data, data_size);   
        } 
        else /* IO_COMMAND_SET_BUFFER */
        {   arc_data_operations (stream_instance, arc, arc_set_base_address_to_arc, data, data_size);    
        }
    }
    else /* TX: stream from the graph */
    {   if (0u == TEST_BIT(*pio, SERVANT_IOFMT_LSB))
        {   
            /* the IO interface is commander : the buffer needs to be refilled before the next interrupt */
            if (IO_COMMAND_SET_BUFFER != TEST_BIT(*pio, IOCOMMAND_IOFMT_LSB))
            {   uint32_t amount_of_data;    /* amount of data = write - read */
                amount_of_data = RD(arc[3], WRITE_ARCW3) -    
                RD(arc[2], READ_ARCW2);

                if (amount_of_data < data_size)
                {   /* underflow issue */
                    platform_al(PLATFORM_ERROR, 0,0,0);
                    /* TODO : implement the flow management desired for "flow_error" */
                    //flow_error = (uint8_t) RD(arc[2], UNDERFLRD_ARCW2);
                    data_size = amount_of_data;
                }            
                /* data copy ARC=>buffer */
                arc_data_operations (stream_instance, arc, arc_IO_moved_from_arc, data, data_size);     

                /* does data realignement must be done ? : realign and clear the bit */
                if (0u != TEST_BIT (arc[3], ALIGNBLCK_ARCW3_LSB))
                {   arc_data_operations (stream_instance, arc, arc_data_realignment_to_base, 0, 0);
                }
            } 
            else /* IO_COMMAND_SET_BUFFER */
            {   arc_data_operations (stream_instance, arc, arc_set_base_address_from_arc, data, data_size);    
            }
        }
        else
        {   /* the IO interface is slave : just a pointer update */
            {   uint32_t amount_of_data;    /* amount of data = write - read */
                amount_of_data = RD(arc[3], WRITE_ARCW3) -    
                RD(arc[2], READ_ARCW2);

                if (amount_of_data < data_size)
                {   /* underflow issue */
                    platform_al(PLATFORM_ERROR, 0,0,0);
                    /* TODO : implement the flow management desired for "flow_error" */
                    //flow_error = (uint8_t) RD(arc[2], UNDERFLRD_ARCW2);
                    data_size = amount_of_data;
                }            
                arc_data_operations (stream_instance, arc, arc_IO_moved_from_arc_update, data, data_size);     

                /* does data realignement must be done ? : realign and clear the bit */
                if (0u != TEST_BIT (arc[3], ALIGNBLCK_ARCW3_LSB))
                {   arc_data_operations (stream_instance, arc, arc_data_realignment_to_base, 0, 0);
                }
            } 
        }
    }

    /* data transfer done when the IO is slave */
    CLEAR_BIT_MP(stream_instance->ioreq, REQMADE_IO_LSB);
}

#ifdef __cplusplus
}
#endif
