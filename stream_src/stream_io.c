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
    intPtr_t *long_offset;
    uint32_t *pio;

    platform_al (PLATFORM_OFFSETS, (uint8_t *)&long_offset,0,0);
    pio = (uint32_t *)GRAPH_IO_CONFIG_ADDR(graph,long_offset);
    pio = &(pio[fw_io_idx]);
    arc = GRAPH_ARC_LIST_ADDR(graph,long_offset);
    arc_idx = EXTRACT_FIELD(*pio, IOARCID_IOFMT);
    arc = &(arc[SIZEOF_ARCDESC_W32 * arc_idx]);
    rx0tx1 = (uint8_t) EXTRACT_FIELD(*pio, RX0TX1_IOFMT);
    command = (uint8_t) EXTRACT_FIELD(*pio, IOCOMMAND_IOFMT);
    size = data_size;

    switch (rx0tx1)
    {
        /* stream to the graph */
        case RX0_TO_GRAPH:
            /* free area = size - write */
            free_area = EXTRACT_FIELD(arc[BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1) - 
                EXTRACT_FIELD(arc[WRIOCOLL_ARCW3], WRITE_ARCW3);

            if (free_area < data_size)
            {   /* overflow issue */
                flow_error = (uint8_t) EXTRACT_FIELD(arc[RDFLOW_ARCW2], OVERFLRD_ARCW2);
                platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                size = free_area;
            }
            if (command == (uint8_t)IO_COMMAND_DATA_MOVE_RX)
            {   arc_data_operations (arc, arc_move_to_arc, long_offset, data, size);      
            } 
            if (command == (uint8_t)IO_COMMAND_SET_BUFFER_RX)
            {   arc_data_operations (arc, arc_set_base_address_to_arc, long_offset, data, size);    
            }
            break;

        /* stream from the graph */
        case TX1_FROM_GRAPH:
            /* amount of data = write - read */
            amount_of_data = EXTRACT_FIELD(arc[WRIOCOLL_ARCW3], WRITE_ARCW3) -    
                EXTRACT_FIELD(arc[RDFLOW_ARCW2], READ_ARCW2);
    
            if (amount_of_data < data_size)
            {   /* underflow issue */
                flow_error = (uint8_t) EXTRACT_FIELD(arc[RDFLOW_ARCW2], UNDERFLRD_ARCW2);
                /* TODO : implement the flow management desired for "flow_error" */
                size = amount_of_data;
            }
            if (command == (uint8_t)IO_COMMAND_DATA_MOVE_RX)
            {   arc_data_operations (arc, arc_moved_from_arc, long_offset, 0, size);      
            } 
            if (command == (uint8_t)IO_COMMAND_SET_BUFFER_TX)
            {   arc_data_operations (arc, arc_set_base_address_from_arc, long_offset, data, size);    
            }
            break;
        default:
            break;
    }
}
