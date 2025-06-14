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

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
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

void arm_stream_io_ack (uint8_t graph_io_idx, void *data, uint32_t size)
{
    extern arm_stream_instance_t * platform_io_callback_parameter;
    arm_stream_instance_t *S = platform_io_callback_parameter;
    uint32_t *arc;
    uint8_t *ongoing;
    uint32_t *pio_control;

    uint32_t read;
    uint32_t write;
    uint32_t fifosize;
    int32_t signed_base;
    uint8_t *long_base;
    uint8_t *src;
    uint8_t *dst;
    uint8_t extend;

    pio_control = &(S->graph[(int)GRAPH_HEADER_NBWORDS + (int)graph_io_idx * (int)STREAM_IOFMT_SIZE_W32]);
    ongoing = &(S->ongoing[graph_io_idx]);
    arc = S->all_arcs;
    arc = &(arc[(int)SIZEOF_ARCDESC_W32 * (int)RD(*pio_control, IOARCID_IOFMT0)]);

    long_base = S->long_offset[RD(arc[0],ADDR_OFFSET_FMT0)];   /* platfom memory offsets */
    signed_base = RD(arc[2], SIGNED_SIZE_FMT0);
    signed_base = signed_base << (32-SIGNED_SIZE_FMT0_MSB);
    signed_base = signed_base >> (32-SIGNED_SIZE_FMT0_MSB);
    extend = RD(arc[2], EXTENSION_FMT0);
    signed_base <<= (extend << 1);
    long_base = (uint8_t *) &(long_base[signed_base]);

    fifosize = RD(arc[1], BUFF_SIZE_ARCW1);
    read = RD(arc[2], READ_ARCW2);
    write = RD(arc[3], WRITE_ARCW3);

    /* 
        Check overflow @@@, does the data request was already reset ?
    */

    if (0 == TEST_BIT(*pio_control, RX0TX1_IOFMT0_LSB))
    {    
        /* 
            RX : stream going to the graph 
        */
        if (IO_COMMAND_SET_BUFFER != RD(*pio_control, SET0COPY1_IOFMT0))
        {               
            /* IO_COMMAND_DATA_COPY : reset the ONGOING flag when enough small 
                sub-frames have been received
            */
            if (fifosize - write < size)   /* free area too small => overflow */
            {   /* overflow issue */
                //platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                //flow_error = (uint8_t) RD(arc[2], OVERFLRD_ARCW2);
                size = fifosize - write;
            }

            /* only one node can read the write-index at a time : no collision is possible */
            src = data;
            dst = &(long_base[write]);
            MEMCPY (dst, src, size)
            write = write + size;


            /* does the write index is already far, ask for data realignment by the consumer node */
            {   uint32_t producer_frame_size, i;
                i = RD(arc[4],PRODUCFMT_ARCW4) * STREAM_FORMAT_SIZE_W32;
                producer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

                if (write > fifosize - producer_frame_size)
                {   SET_BIT(arc[2], ALIGNBLCK_ARCW2_LSB);
                }
            }
        } 
        else /* IO_COMMAND_SET_BUFFER */
        {   
            /* arc_set_base_address_to_arc */
            dst = data;
            ST(arc[0], BASEIDXOFFARCW0, platform_lin2pack((intptr_t)data));
            ST(arc[1], BUFF_SIZE_ARCW1, size);  /* FIFO size aligned with the buffer size */
            ST(arc[2], READ_ARCW2, 0);
            write = size;
        }

        /* reset the data transfert flag is a frame is fully received */
        {   uint32_t i, time_stamp_prod, time_stamp_cons, consumer_frame_size;

            i = STREAM_FORMAT_SIZE_W32 * RD(arc[4],CONSUMFMT_ARCW4);
            time_stamp_prod = RD(S->all_formats[i +1], TIMSTAMP_FMT1);
            consumer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

            /* check the need for time-stamp insertion */
            i = RD(arc[4],CONSUMFMT_ARCW4) * STREAM_FORMAT_SIZE_W32;
            time_stamp_cons = RD(S->all_formats[i +1], TIMSTAMP_FMT1);

            if (time_stamp_prod == NO_TIMESTAMP && time_stamp_cons != NO_TIMESTAMP)
            {
                src = dst;
                dst = src + 4;
                MEMCPY (dst, src, size)
                write = write + 4;
                *src = 0;       // TIME-STAMP
            }

            if (write - read >= consumer_frame_size)
            {   CLEAR_BIT(*ongoing, ONGOING_IO_LSB);
            }
        }

        ST(arc[3], WRITE_ARCW3, write);     /* finaly update the write index */
    }
    else 
    {   /* 
            TX: stream from the graph 
        */
        if (IO_COMMAND_SET_BUFFER != RD(*pio_control, SET0COPY1_IOFMT0))
        {     
           /* IO_COMMAND_DATA_COPY : reset the ONGOING flag when the remaining data to transmit 
                is small, and below the transmitter frame size
            */
            if (write - read < size)   /* data available for TX is too small => underflow */
            {   /* underflow issue */
                //platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                //flow_error = (uint8_t) RD(arc[2], OVERFLRD_ARCW2);
                size = write - read;
            }

            /* only one node can read the write-index at a time : no collision is possible */
            src = &(long_base[read]);
            dst = data;
            MEMCPY (dst, src, size)
            read = read + size;
            ST(arc[2], READ_ARCW2, read);   /* update the read index */

            /* check need for alignement */
            if (TEST_BIT (arc[2], ALIGNBLCK_ARCW2_LSB))
            {   src = &(long_base[read]);
                dst =  long_base;
                MEMCPY (dst, src, (uint32_t)(write-read))

                /* update the indexes Read=0, Write=dataLength, then clear the flag */
                ST(arc[2], READ_ARCW2, 0);
                ST(arc[3], WRITE_ARCW3, write-read);
                CLEAR_BIT(arc[2], ALIGNBLCK_ARCW2_LSB);
            }

            /* reset the data transfert flag if no frame is ready for transmit */
            {   uint32_t consumer_frame_size, i;
                i = RD(arc[4],CONSUMFMT_ARCW4) * STREAM_FORMAT_SIZE_W32;
                consumer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);
                if (write - read < consumer_frame_size)
                {   CLEAR_BIT(*ongoing, ONGOING_IO_LSB);
                }
            }
        } 
        else /* IO_COMMAND_SET_BUFFER for the next frame to send */
        {
            /*arc_set_base_address_to_arc */
            ST(arc[0], BASEIDXOFFARCW0, platform_lin2pack((intptr_t)data));
            ST(arc[2], READ_ARCW2, 0);
            ST(arc[3], WRITE_ARCW3, 0);
            CLEAR_BIT(*ongoing, ONGOING_IO_LSB);
        }
    }

    /* flush the cache and the memory barriers for buffers used with multiprocessing */
    if (0 != TEST_BIT(arc[2], MPFLUSH_ARCW2_LSB))
    {   DATA_MEMORY_BARRIER
    }
}


#ifdef __cplusplus
}
#endif
