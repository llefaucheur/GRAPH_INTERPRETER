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

//#include <stdlib.h>
//#include <stdint.h>
//#include <string.h> 

/*---------------------------------------*/
#include "platform_windows.h"
//#include "platform_arduino_nano33.h"
/*---------------------------------------*/

#include "stream_const.h"      
#include "stream_types.h"  

 
 /*  
    data in RAM : a single Stream instance is in charge of one io
    the only extra SRAM area needed to interface with Stream is the address of the graph

 */
uint32_t * platform_io_callback_parameter [LAST_IO_FUNCTION_PLATFORM];


/*==================================================== DOMAIN  ===================================================================*/
/* tuning of PLATFORM_AUDIO_IN */
    //const float audio_settings [] = { 
    ///* [stream_units_physical_t, scaling to full-scale] */  
    //unit_pascal, 1.0f,
    ///* nb of bits */
    ///* 3  nchan */              3,   1, 2, 8,  
    ///* 4  FS */                 2,   16000.0f, 48000.0f, 
    ///* 2  framesize [s] */      2,   0.01f, 0.016f, 
    ///*    1 option = "fixed in HW and this is the setting" */
    ///*    0 option = "fixed in HW, and it depends" */
    ///* 4  PGA gain + digital gain option [dB]*/  0,          
    ///* 2  hpf set at 20Hz */    1,   20.0f,
    ///* 1  agc option */         0,
    ///* 2  router to MIC, LINE, BT, LOOP */ 0,          
    ///*14 bits remains */    
//#define AUDIO_REC_STREAM_SETTING
//    #define _UNUSED_AUDIO_REC_MSB 31
//    #define _UNUSED_AUDIO_REC_LSB 16
//    #define     AGC_AUDIO_REC_MSB 15 
//    #define     AGC_AUDIO_REC_LSB 15 /* O6 AGC on/off */
//    #define     HPF_AUDIO_REC_MSB 14 
//    #define     HPF_AUDIO_REC_LSB 14 /* O5 HPF on/off */
//    #define     PGA_AUDIO_REC_MSB 13 
//    #define     PGA_AUDIO_REC_LSB 10 /* O4 16 analog PGA + digital PGA settings [dB] */
//    #define   DBSPL_AUDIO_REC_MSB  9 
//    #define   DBSPL_AUDIO_REC_LSB  9 /* O3 2 max dBSPL options */
//    #define FRMSIZE_AUDIO_REC_MSB  8 
//    #define FRMSIZE_AUDIO_REC_LSB  7 /* O2 4 frame size options, in seconds */
//    #define      FS_AUDIO_REC_MSB  6 
//    #define      FS_AUDIO_REC_LSB  3 /* O1 16 sampling rates options  */
//    #define   NCHAN_AUDIO_REC_MSB  2 
//    #define   NCHAN_AUDIO_REC_LSB  0 /* O0 8 nchan options  */
//    #define PACKAUDIORECOPTIONS3(O6,O5,O4,O3,O2,O1,O0) ((O6<<15)|(O5<<14)|(O4<<10)|(O3<<9)|(O2<<7)|(O1<<3)|(O0)) 

/* tuning of PLATFORM_AUDIO_OUT */
    //const int32_t audio_out_settings [] = { 
    ///* nb options nbbits */
    ///*  8  3  nchan */         3,   1, 2, 8,
    ///* 16  4  FS */            2,   16000, 48000, 
    ///*  4  2  framesize [ms] */2,   10, 16, 
    ///*  8  3  mVrms max */     2,   100, 700,
    ///* 16  4  PGA gain */      0,
    ///*  4  2  bass gain dB */  4,   0, -3, 3, 6,
    ///*  2  1  bass frequency */2,   80, 200,       
    ///*  4  2  mid gain */      4,   0, -3, 3, 6,
    ///*  2  1  mid frequency */ 2,   500, 2000,       
    ///*  4  2  high gain */     4,   0, -3, 3, 6,
    ///*  2  1  high frequency */2,   4000, 8000,       
    ///*  2  1  agc gain */      0,
    ///*  4  2  router */        0,  /* SPK, EAR, LINE, BT */
    ///*     2 bits remains */ 
const uint8_t platform_audio_out_bit_fields[] = { 3,4,2,3,4,2,1,2,1,2,1,2,1 };



/* tuning of PLATFORM_GPIO_IN / OUT */
    //const int32_t gpio_out_settings [] = {    
    /* nb options nbbits */
    //State : High-Z, low, high, duration, frequency
    //type : PWM, motor control, GPIO
    //PWM duty, duration, frequency (buzzer)
    //Servo motor control -120 .. +120 deg
    //keep the servo position


/* PLATFORM_COMMAND_IN */
    // FIFO(s) of commands from the application    

/* PLATFORM_COMMAND_OUT */
    // execusion trace/debug, FIFO(s) of command result to the application


/* tuning of PLATFORM_RTC_IN / OUT */
    /* a FIFO is filled from RTC on periodic basis : */
    //const int32_t timer_in_settings [] = {    
    //    /* nb options nbbits */
    //    /*  8  3  unit : Days Hours Minutes Seconds 1ms
    //    /* Flexible bit-fields implementation-dependent
    //    /*   period Q8.4 x [unit]
    //    /*   offset (Q8) from the start of the day (watering)
    // use-case = periodic comparison of ADC value, periodic activation of GPIO
    //   one FIFO is trigered by unit: days one other by minutes, a script can
    //   implement the state machine to read sensors or activate watering

/* tuning of PLATFORM_USER_INTERFACE_IN / OUT */
    /* a FIFO is filled from UI detection of a button */
    /* one FIFO per button to ease the routing to specific nodes */

    /* Default implementation using SYSTICK
       Time format - 40 bits - SYSTICK 1ms increments, global register
       GLOBALS : G_INCTICK (64bits), INV_RVR (32bits), G_SYST0 (32bits) interval in Q32 second
       G_INCTICK : incremented in the SYSTICK interrupt, SYST0=1ms or 10ms
       FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
       ________________________qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq with 1ms increment => 35 years (32bits gives only 50 days)
  
       SYST_CVR (current value), SYST_CALIB (reload value for 10ms), SYST_RVR (reload value), INV_RVR = 0x10000.0000/SYST_RVR
       Time sequence : G_INCTICK=>T0  CVR=>C0  G_INCTICK=>T1  CVR=>C1  

       Algorithm : if T0==T1 (no collision) then T=T0,C=C0  else T=T1,C=C1 endif
        the fraction of SYST0 is (RVR - C) x INV_RVR = xx...xxx
        the time (64bits Q16)  is  (T<<16) + (((RVR - C) x INV_RVR) >> 16)  => used for time-stamps 
        FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
        ________qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqXXXXXXXXXXXXXXXX 

       stream_time_seconds in 32bits :
       bits 31-30 = format   0: seconds from reset, 1:seconds from Jan 1st 2023, 2:format U14.16 for IMU time-stamps differences
       bit  29- 0 = time     (30bits seconds = 68years)
     */


/*
    skip the fields line by line, until finding the "setting_line"
    and read the "index" setting from this line

    The table is made of int32_t to unquantize 
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
        }
        i_field = i_field + nb_fields;
    }
    return 3;
}


/*
 * --- platform abstraction layer -------------------------------------------------------
 */

void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3)
{
    static uint8_t stream_boot_lock;        
    static uint8_t stream_initialization_done;
    static uint8_t stream_reset_synchro[MAX_NB_STREAM_INSTANCES];

    static uint32_t platform_time;

    switch (command)            /*  */
    {
    case PLATFORM_PROC_ID:  /* platform_al (PLATFORM_PROC_ID, uint_8_t *procID, uint_8_t *archID,0); */
    {   
        uint8_t *proc;
        uint8_t *arch;
        proc=ptr1;
        arch=ptr2;
        
        /* [1,2,3] processor architectures 0="any" 1="master processor architecture" */
        *arch = 1;

        /* processor index [0..7] for this architecture 0="master processor" */  
        *proc = 0;
        break;
    }

    case PLATFORM_OFFSETS: /* platform_al (PLATFORM_OFFSETS, intPtr_t **,0,0); */
    {
        extern intPtr_t long_offset[NB_MEMINST_OFFSET];
        extern void platform_specific_long_offset(void);
        intPtr_t **D; 
        intPtr_t *S;

        platform_specific_long_offset();
        D = (intPtr_t **)ptr1;
        S = long_offset;

        *D = S;
        break;
    }



    case PLATFORM_MP_GRAPH_SHARED: /* platform_al (PLATFORM_MP_GRAPH_SHARED, start_address,end_address,0); */
    {
        /* define MPU_RBAR[Region] */
        /* set MPU_RASR[18] = 1 : the memory area is shared*/
        break;
    }
    case PLATFORM_MP_BOOT_SYNCHRO: /* platform_al (PLATFORM_MP_SYNCHRO, *instance index, *copy allowed for you, 0); */
    {
        uint8_t check;
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
    case PLATFORM_MP_BOOT_WAIT: /* platform_al (PLATFORM_MP_BOOT_WAIT, uint_8_t *wait, 0,0); */
    {   
        uint8_t *wait=ptr1;
        *wait = (stream_initialization_done == 0u)? 1u : 0u;
        break;
    }
    case PLATFORM_MP_BOOT_DONE: /* platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); */
    {   
        DATA_MEMORY_BARRIER;
        stream_initialization_done = 1;
        break;
    }
    case PLATFORM_MP_RESET_DONE: /* platform_al (PLATFORM_MP_RESET_DONE, &stream_instance,0,0); */
    {
        uint8_t *instance=ptr1;
        stream_reset_synchro[*instance] = 0;
        DATA_MEMORY_BARRIER;
        break;
    }
    case PLATFORM_MP_RESET_WAIT: /* platform_al (PLATFORM_MP_RESET_WAIT, &wait, &total_nb_stream_instance, 0); */
    {
        uint8_t *wait;
        uint8_t *nb;
        uint8_t i;
        wait = ptr1;
        nb = ptr2;
                
        DATA_MEMORY_BARRIER;

        /* all the instances must have set their flag at "1" to tell "reset completed" */
        *wait = 1;
        for (i = 0; i < *nb; i++)
        { *wait &= stream_reset_synchro[i];
        }
        break;
    }
    case PLATFORM_IO_ACK:  /* interface callback to arm_stream_io : platform_al(PLATFORM_IO_ACK, fx_idx, *,int);*/
    {
        uint32_t fw_idx;
        uint32_t size;
        uint32_t *graph;
        fw_idx = (uint32_t)convert_ptr_to_int(ptr1);
        size = (uint32_t)convert_ptr_to_int(ptr3);
        graph = platform_io_callback_parameter[fw_idx];
        arm_stream_io (fw_idx, graph, ptr2, size);
        break;
    }

    case PLATFORM_IO_SET_STREAM :   /* platform_al (PLATFORM_IO_SET_STREAM, struct platform_control_stream *, 0,0); */
    case PLATFORM_IO_DATA :
    case PLATFORM_IO_STOP_STREAM : 
	{   
        struct platform_control_stream *parameters;
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
            platform_io_callback_parameter [parameters->fw_idx] = parameters->graph;
            io_func = io_manifest->io_set;
            (*io_func)(settings, (uint8_t *)(buffer->address), (uint32_t)(buffer->size));
            break;

        /* to avoid blocking the CPU waiting for the data to arrive : send a message to RTOS 
            and return to caller. "arm_stream_io" will get notified of the completion of the 
            transfer (bit REQMADE_IOFMT)
         */
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
    case PLATFORM_EXEC_TIME:        /* platform_al (PLATFORM_EXEC_TIME, *delta_time, 0,0); */
    {
        break;
    }

    case PLATFORM_ERROR:  /* platform_al (PLATFORM_ERROR, uint_8_t type, uint_8_t data,0); */
    {   
        uint8_t *type;
        uint8_t *error;
        type=ptr1;
        error=ptr2;
        break;
    }
    case PLATFORM_CLEAR_BACKUP_MEM: /* platform_al (PLATFORM_CLEAR_BACKUP_MEM, 0,0,0); cold start : clear backup memory */
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

