/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_io.c
 * Description:  callback used after data moves at the boundary of the graph
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
#include "stream_extern.h"




/**
  @brief         data transfer acknowledge
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

        Trigger is "give me data to arc[w], and raise the flag" 
        Upon RX ISR (*,n) : check overflow, then either
            set arc base address to * parameter, and release the flag
            or copy to arc, increment W, 
                if arc(data-received) > consumer frame,  then reset the flag


                 TX case : IO is commander case : "I have completed the last transfer, you can fill 
                 this buffer for the next transfer". IO is slave case : "I have completed the 
                 transfer of this buffer you told me to move out using io_start() with this amount 
                 of bytes, you can reset the flag telling the transfer is on-going". 

        Trigger is "new data to send from arc[r], raise the flag
        Upon TX ISR (*,n) : check overflow, flag-reset happened, then either
            set the arc base address to * parameter, n = frame size, src=dst, reset the flag
            or copy arc[r], n data, R=R+n, if arc(available data) < Frame TX then reset the flag
  @remark
 */

void arm_graph_interpreter_io_ack (uint8_t graph_io_idx, uint8_t *data,  uint32_t data_size)
{
    extern arm_stream_instance_t * platform_io_callback_parameter;
    arm_stream_instance_t *S = platform_io_callback_parameter;
    uint32_t *arc;
    uint32_t *pio;

    uint32_t read;
    uint32_t write;
    uint32_t base;
    uint32_t fifosize;
    uint8_t *long_base;
    uint8_t *src;
    uint8_t *dst;

    pio = S->pio;
    pio = &(pio[STREAM_IOFMT_SIZE_W32 * graph_io_idx]);
    arc = S->all_arcs;
    arc = &(arc[SIZEOF_ARCDESC_W32 * RD(*pio, IOARCID_IOFMT)]);

    base = RD(arc[0], BASEIDXOFFARCW0);
    long_base = (uint8_t *)pack2linaddr_ptr(S->long_offset, base, LINADDR_UNIT_W32);
    fifosize = RD(arc[1], BUFF_SIZE_ARCW1);
    read = RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);

    /* 
        Check overflow, does the data request was already reset ?
    */



    if (0 == TEST_BIT(*pio, RX0TX1_IOFMT_LSB))
    {    
        /* 
            RX : stream going to the graph 
        */
        if (IO_COMMAND_SET_BUFFER != RD(*pio, SET0COPY1_IOFMT))
        {               
            /* IO_COMMAND_DATA_MOVE : reset the ONGOING flag when enough small 
                sub-frames have been received
            */
            if (fifosize - write < data_size)   /* free area too small => overflow */
            {   /* overflow issue */
                //platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                //flow_error = (uint8_t) RD(arc[2], OVERFLRD_ARCW2);
                data_size = fifosize - write;
            }

            /* only one node can read the write-index at a time : no collision is possible */
            src = data;
            dst = &(long_base[write]);
            MEMCPY (dst, src, (uint32_t)data_size)
            write = write + data_size;
            ST(arc[3], WRITE_ARCW3, write);     /* update the write index */

            /* reset the data transfert flag is a frame is fully received */
            {   uint32_t consumer_frame_size, i;
                i = RD(arc[1],CONSUMFMT_ARCW1) * STREAM_FORMAT_SIZE_W32;
                consumer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

                if (write - read >= consumer_frame_size)
                {   CLEAR_BIT(*pio, ONGOING_IOFMT_LSB);
                }
            }

            /* does the write index is already far, ask for data realignment by the consumer node */
            {   uint32_t producer_frame_size, i;
                i = RD(arc[0],PRODUCFMT_ARCW0) * STREAM_FORMAT_SIZE_W32;
                producer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

                if (write > fifosize - producer_frame_size)
                {   SET_BIT(arc[3], ALIGNBLCK_ARCW3_LSB);
                }
            }
        } 
        else /* IO_COMMAND_SET_BUFFER */
        {
            /* arc_set_base_address_to_arc */
            ST(arc[0], BASEIDXOFFARCW0, lin2pack(S, data));
            ST(arc[1], BUFF_SIZE_ARCW1, data_size); /* FIFO size aligned with the buffer size */
            ST(arc[2], READ_ARCW2, 0);
            ST(arc[3], WRITE_ARCW3, data_size); /* automatic rewind after read */
            CLEAR_BIT(*pio, ONGOING_IOFMT_LSB);
        }
    }
    else 
    {   /* 
            TX: stream from the graph 
        */
        if (IO_COMMAND_SET_BUFFER != RD(*pio, SET0COPY1_IOFMT))
        {     
           /* IO_COMMAND_DATA_MOVE : reset the ONGOING flag when the remaining data to transmit 
                is small, and below the transmitter frame size
            */
            if (write - read < data_size)   /* data available for TX is too small => underflow */
            {   /* underflow issue */
                //platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                //flow_error = (uint8_t) RD(arc[2], OVERFLRD_ARCW2);
                data_size = write - read;
            }

            /* only one node can read the write-index at a time : no collision is possible */
            src = &(long_base[read]);
            dst = data;
            MEMCPY (dst, src, (uint32_t)data_size)
            read = read + data_size;
            ST(arc[2], READ_ARCW2, read);   /* update the read index */

            /* check need for alignement */
            if (0u != TEST_BIT (arc[3], ALIGNBLCK_ARCW3_LSB))
            {   src = &(long_base[read]);
                dst =  long_base;
                MEMCPY (dst, src, (uint32_t)(write-read))

                /* update the indexes Read=0, Write=dataLength */
                ST(arc[2], READ_ARCW2, 0);
                ST(arc[3], WRITE_ARCW3, write-read);
                CLEAR_BIT(arc[3], ALIGNBLCK_ARCW3_LSB);
            }

            /* reset the data transfert flag if no frame is ready for transmit */
            {   uint32_t consumer_frame_size, i;
                i = RD(arc[1],CONSUMFMT_ARCW1) * STREAM_FORMAT_SIZE_W32;
                consumer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);
                if (write - read < consumer_frame_size)
                {   CLEAR_BIT(*pio, ONGOING_IOFMT_LSB);
                }
            }
        } 
        else /* IO_COMMAND_SET_BUFFER for the next frame to send */
        {
            /*arc_set_base_address_to_arc */
            ST(arc[0], BASEIDXOFFARCW0, lin2pack(S, data));
            ST(arc[2], READ_ARCW2, 0);
            ST(arc[3], WRITE_ARCW3, 0);
            CLEAR_BIT(*pio, ONGOING_IOFMT_LSB);
        }
    }
}


#ifdef __cplusplus
}
#endif
