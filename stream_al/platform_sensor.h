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
#ifndef platform_sensor_H
#define platform_sensor_H


/*==================================================== DOMAINS  ===================================================================*/
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
#define PLATFORM_AUDIO_OUT_NCHAN    0
#define PLATFORM_AUDIO_OUT_FS       1
#define PLATFORM_AUDIO_OUT_FRAMESIZE 2
#define PLATFORM_AUDIO_OUT_MVRMS    3
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
       bits 31-30 = format  0:seconds from reset, 
                            1:seconds from Jan 1st 2023 (local time) 
                            2:format U14.16 for IMU time-stamps differences
                            3:date format YY:MM:DD:HH:MN:SS
       bit  29- 0 = time    30bits seconds = 68years
                            U14.16 = 15us accuracy
                            __DCBA987654321 FEDCBA9876543210
                              YYYYYYYMMMMDDDDDHHHHHMMMMMMSSS
     */

/*

Decode INPUT image/video files (engineering can change if needed):
BMP, PNG, JPG*
WMV*
AVI, MP4
RAW

96x96
128x128
192x192
224x224
300x300
320x320
416x416
1920x1080
1080p
*/




#endif /* #ifndef platform_sensor_H */