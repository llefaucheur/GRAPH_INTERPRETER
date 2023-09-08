/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_stream_modulator.c
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


#include "platform_computer.h"
#include "stream_const.h"      
#include "stream_types.h"  
#include "arm_stream_modulator.h"


//;----------------------------------------------------------------------------------------
//;11.	arm_signal_modulator
//;----------------------------------------------------------------------------------------
//;    Operation : sine, noise, square, saw tooth with amplitude or frequency modulation
//;    use-case : ring modulator, sweep generation with a cascade of a ramp generator and
//;       a frequency modulator
//;
//;   Parameters types and Tags:
//;   u8 #1 wave type : 1=sine 2=square 3=white noise 4=pink noise 
//;                  5=sawtooth 6=triangle 7=pulse
//;                  8=prerecorded pattern playback from arc 
//;   u16 #2 linear amplitude, format UQ16 0dB .. -96dB
//;   u16 #3 amplitude offset, format Q15 [-1 .. +1]
//;   f32 #4 wave frequency [Hz] [0.1uHz .. 250kHz];
//;   u16 #5 starting phase, format Q15 [-pi .. +pi]/pi
//;   u8  #6 modulation type, 0:amplitude, 1:frequency 
//;          (reserved 2:FSK)
//;   u8  #7 modulation, 0:none 1=from arc bit stream, 2=from arc q15 words
//;   u16 #8 modulation index (a) and offset (b), in q15 as fraction of frequency or amplitude
//;          modulation y=ax+b, x=input data.
//;   u32 #9 modulation frequency [Hz] separating two data read from the arc
//;
//arm_signal_modulator 
//    3  i8; 0 0 0        instance, preset, tag
//    PARSTART
//    1  u8;  1       sinewave
//    2 h16;  FFFF 0  full-scale, no offset
//    1 f32;  1200    1200Hz
//    1 u16;  0       initial phase
//    2  u8;  1 1     frequency modulation from bit-stream
//    2 h16;  8000 0  full amplitude modulation with sign inversion of the bit-stream
//    1 f32;  300     300Hz modulation => (900Hz .. 1500Hz modulation)
//    PARSTOP
//;


/**
  @brief         
  @param[in]     command    bit-field
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_modulator (int32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *status)
{
    *status = 1;    /* default return status, unless processing is not finished */

    switch (RD(command,COMMAND_CMD))
    { 
        /* func(command = (STREAM_RESET, PRESET, TAG, NB ARCS IN/OUT)
                instance = *memory_results,  
                data = address of Stream function
                
                memresults are followed by 2 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the SWC manifest
        */
        case STREAM_RESET: 
        {   stream_services_entry *stream_entry = (stream_services_entry *)(uint64_t)data;
            intPtr_t *memresults = (intPtr_t *)instance;
            uint16_t preset = RD(command, PRESET_CMD);

            arm_stream_modulator_instance *pinstance = (arm_stream_modulator_instance *)  *memresults;
            memresults++;
            /* here reset */

            break;
        }    

        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, 0xFF means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:  
        {   
            uint8_t *new_parameters = (uint8_t *)data;
            break;
        }



        /* func(command = STREAM_READ_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG/index of a parameter to read (Metadata, Needle), 0xFF means "read all the parameters"
                *instance, 
                data = *parameter(s) to read
        */ 
        case STREAM_READ_PARAMETER:  
        {   
            uint8_t *new_parameters = (uint8_t *)data;

            break;
        }
        


        /* func(command = STREAM_RUN, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = array of [{*input size} {*output size}]

               data format is given in the node's manifest used during the YML->graph translation
               this format can be FMT_INTERLEAVED or FMT_DEINTERLEAVED_1PTR
        */         
        case STREAM_RUN:   
        {
            arm_stream_modulator_instance *pinstance = (arm_stream_modulator_instance *) instance;
            intPtr_t nb_data, stream_xdmbuffer_size, bufferout_free;
            stream_xdmbuffer_t *pt_pt;
            #define SAMP_IN uint8_t 
            #define SAMP_OUT int16_t
            SAMP_IN *inBuf;
            SAMP_OUT *outBuf;

            pt_pt = data;
            inBuf  = (SAMP_IN *)pt_pt->address;   
            stream_xdmbuffer_size    = pt_pt->size;
            pt_pt++;
            outBuf = (SAMP_OUT *)(pt_pt->address); 
            bufferout_free        = pt_pt->size;

            nb_data = stream_xdmbuffer_size / sizeof(SAMP_IN);

            arm_stream_modulator_process(pinstance, inBuf, outBuf, &nb_data);

            /* the SWC is producing an amount of data different from the consumed one (see xdm11 in the manifest) */
            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_OUT);   /* amount of data produced */

            
            break;
        }



        /* func(command = STREAM_STOP, PRESET, TAG, NB ARCS IN/OUT)
               instance,  
               data = unused
           used to free memory allocated with the C standard library
        */  
        case STREAM_STOP:  break;    
    }
}

#ifdef __cplusplus
}
#endif
    
