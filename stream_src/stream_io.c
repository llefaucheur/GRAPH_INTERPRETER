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

#include "stream_const.h"    /* #include <stdint.h> */
#include "stream_types.h"

/*----------------------------------------------------------*/
/*
        call back for the low layers of the platform
        mode data to the ring buffers , as described in the graph
          or simple assign the base address of the ring buffer to the data (no data move)
        check there is no flow error
  */

void arm_stream_io (uint32_t fw_io_idx, 
            uint32_t *graph,
            uint8_t *data, uint32_t data_size)
{   
    uint32_t arc_idx;
    intPtr_t *arc;
    uint32_t free_area;
    uint32_t amount_of_data;
    uint32_t size;
    uint8_t rx0tx1;
    uint8_t command;
    uint8_t flow_error;
    uint32_t *pio;
    uint32_t *all_formats;

    pio = (uint32_t *)GRAPH_IO_CONFIG_ADDR();
    pio = &(pio[STREAM_IOFMT_SIZE_W32 * fw_io_idx]);
    arc = GRAPH_ARC_LIST_ADDR();
    arc_idx = RD(*pio, IOARCID_IOFMT);
    arc = &(arc[SIZEOF_ARCDESC_W32 * arc_idx]);
    rx0tx1 = (uint8_t) RD(*pio, RX0TX1_IOFMT);
    command = (uint8_t) RD(*pio, IOCOMMAND_IOFMT);
    size = data_size;
    all_formats = GRAPH_FORMAT_ADDR();

    switch (rx0tx1)
    {
        /* stream to the graph */
        case RX0_TO_GRAPH:
            switch (command)
            {
            case IO_COMMAND_DATA_MOVE_RX:
            {   /* free area = size - write */
                free_area = RD(arc[1], BUFF_SIZE_ARCW1) - 
                RD(arc[3], WRITE_ARCW3);
                
                if (free_area < data_size)
                {   /* overflow issue */
                    flow_error = (uint8_t) RD(arc[2], OVERFLRD_ARCW2);
                    platform_al(PLATFORM_ERROR, 0,0,0);
                    /* TODO : implement the flow management desired for "flow_error" */
                    size = free_area;
                }

                arc_data_operations (arc, arc_move_to_arc, data, size, all_formats);   
                break;
            } 

            case IO_COMMAND_SET_BUFFER_RX:
            {   arc_data_operations (arc, arc_set_base_address_to_arc, data, size, all_formats);    
                break;
            }

            default :
            case IO_COMMAND_NONE:   /* the IO path was disabled */
                break;
            }

        /* stream from the graph */
        case TX1_FROM_GRAPH:
            switch (command)
            {
            case IO_COMMAND_DATA_MOVE_TX:
            {   /* amount of data = write - read */
                amount_of_data = RD(arc[3], WRITE_ARCW3) -    
                RD(arc[2], READ_ARCW2);

                if (amount_of_data < data_size)
                {   /* underflow issue */
                    flow_error = (uint8_t) RD(arc[2], UNDERFLRD_ARCW2);
                    /* TODO : implement the flow management desired for "flow_error" */
                    size = amount_of_data;
                }            
                arc_data_operations (arc, arc_moved_from_arc, 0, size, all_formats);     

                /* does data realignement must be done ? */
                if (0u != TEST_BIT (arc[3], ALIGNBLCK_ARCW3_LSB))
                {   arc_data_operations (arc, arc_data_realignment_to_base, 0, 0, all_formats);
                    DATA_MEMORY_BARRIER;
                }
                break;
            } 

            case IO_COMMAND_SET_BUFFER_TX :
            {   arc_data_operations (arc, arc_set_base_address_from_arc, data, size, all_formats);    
                break;
            }

            default :
            case IO_COMMAND_NONE:  /* the IO path was disabled */
                break;
            }

        default:
            break;
    }

    /* data transfer done */
    CLEAR_BIT_MP(*pio, REQMADE_IOFMT_LSB);
}
