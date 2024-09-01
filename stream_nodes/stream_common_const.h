/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_common.h
 * Description:  Common data definition
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

#ifndef cSTREAM_COMMON_CONST_H
#define cSTREAM_COMMON_CONST_H


/* ------------------------------------------------------------------------------------------
   32bits / 64bits architectures 
*/
#define PLATFORM_ARCH_32BIT
//#define PLATFORM_ARCH_64BIT





/*  nbparam = 0 means any or "full set of parameters loaded from binary format" 
    W32LENGTH_LW3 == 0 means no parameter to read
*/
#define NODE_ALL_PARAM 0 


#define NODE_TASKS_COMPLETED 0
#define NODE_TASKS_NOT_COMPLETED 1


#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */
/* ----------------------------------------------------------------------------------------------------------------
    STREAM_IO_DOMAIN (s)    => stream_format + stream_io_control

    enum stream_io_domain : list of stream "domains" categories, max 15 (DOMAIN_FMT1)
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).
*/
#define IO_DOMAIN_GENERAL             0  /* (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings*/
#define IO_DOMAIN_AUDIO_IN            1  /* microphone, line-in, I2S, PDM RX */
#define IO_DOMAIN_AUDIO_OUT           2  /* line-out, earphone / speaker, PDM TX, I2S, */
#define IO_DOMAIN_GPIO_IN             3  /* generic digital IO , control of relay, */
#define IO_DOMAIN_GPIO_OUT            4  /* generic digital IO , control of relay, */
#define IO_DOMAIN_MOTION              5  /* accelerometer, combined or not with pressure and gyroscope */
#define IO_DOMAIN_2D_IN               6  /* camera sensor */
#define IO_DOMAIN_2D_OUT              7  /* display, led matrix, */
#define IO_DOMAIN_ANALOG_IN           8  /* analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage */
#define IO_DOMAIN_ANALOG_OUT          9  /* D/A, position piezzo, PWM converter  */
#define IO_DOMAIN_RTC                10  /* ticks sent from a programmable timer */
#define IO_DOMAIN_USER_INTERFACE_IN  11  /* button, slider, rotary button */
#define IO_DOMAIN_USER_INTERFACE_OUT 12  /* LED, digits, display, */
#define IO_DOMAIN_PLATFORM_3         13  /*  */                             
#define IO_DOMAIN_PLATFORM_2         14  /* platform-specific #2, decoded with callbacks */                             
#define IO_DOMAIN_PLATFORM_1         15  /* platform-specific #1, decoded with callbacks */                             
#define IO_DOMAIN_MAX_NB_DOMAINS     16


/* ==========================================================================================

    IO_DOMAIN physical types and tuning : used to insert extra conversion nodes 
                                          during the graph compilation 

   ==========================================================================================
*/

/* IO_DOMAIN_GENERAL           : subtypes and tuning  SUBTYPE_FMT1  */
    #define STREAM_SUBT_GENERAL          0
    #define STREAM_SUBT_GENERAL_COMP195X 1  /* compressed byte stream following RFC1950 / RFC1951 ("deflate") */
    #define STREAM_SUBT_GENERAL_DPCM     2  /* compressed byte stream */
    #define STREAM_SUBT_GENERAL_JSON     3  /* JSON */
    #define STREAM_SUBT_GENERAL_XFORMAT  4  /* SensorThings MultiDatastream extension */

/* IO_DOMAIN_AUDIO_IN          : subtypes and tuning  SUBTYPE_FMT1 */
    #define STREAM_SUBT_AUDIO_IN        0   /* no subtype_units : integer/ADC format  */
    #define STREAM_SUBT_AUDIO_MPG       0   /* compressed byte stream */

/* IO_DOMAIN_AUDIO_OUT         : subtypes and tuning  SUBTYPE_FMT1 */
    #define STREAM_SUBT_AUDIO_OUT       0   /* no subtype_units : integer/DAC format  */

/* IO_DOMAIN_GPIO_IN           : subtypes and tuning  SUBTYPE_FMT1 */
/* IO_DOMAIN_GPIO_OUT          : subtypes and tuning  SUBTYPE_FMT1 */

   #define STREAM_SUBT_GPIO_IN     0   /* no subtype_units  */
   #define STREAM_SUBT_GPIO_OUT    0   /* no subtype_units  */

/* IO_DOMAIN_MOTION_IN         : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_MOTION_A     1
   #define STREAM_SUBT_MOTION_G     2
   #define STREAM_SUBT_MOTION_B     3
   #define STREAM_SUBT_MOTION_AG    4
   #define STREAM_SUBT_MOTION_AB    5
   #define STREAM_SUBT_MOTION_GB    6
   #define STREAM_SUBT_MOTION_AGB   7

/* IO_DOMAIN_2D_IN             : subtypes and tuning  SUBTYPE_FMT1 */
/*                      raw data is uint8 or uint16 but the subtype tells how to extract the pixel data */
/* IO_DOMAIN_2D_OUT            : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_2D_YUV420P   1  /* Luminance, Blue projection, Red projection, 6 bytes per 4 pixels, reordered */
   #define STREAM_SUBT_2D_YUV422P   2  /* 8 bytes per 4 pixels, or 16bpp, Y0 Cb Y1 Cr (1 Cr & Cb sample per 2x1 Y samples) */
   #define STREAM_SUBT_2D_YUV444P   3  /* 12 bytes per 4 pixels, or 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
   #define STREAM_SUBT_2D_CYM24     4  /* cyan yellow magenta */
   #define STREAM_SUBT_2D_CYMK32    5  /* cyan yellow magenta black */
   #define STREAM_SUBT_2D_RGB8      6  /* RGB  3:3:2,  8bpp, (msb)2B 3G 3R(lsb) */
   #define STREAM_SUBT_2D_RGB16     7  /* RGB  5:6:5, 16bpp, (msb)5R 6G 5B(lsb) */
   #define STREAM_SUBT_2D_RGBA16    8  /* RGBA 4:4:4:4 32bpp (msb)4R */
   #define STREAM_SUBT_2D_RGB24     9  /* BBGGRR 24bpp (msb)8B */
   #define STREAM_SUBT_2D_RGBA32   10  /* BBGGRRAA 32bpp (msb)8B */
   #define STREAM_SUBT_2D_RGBA8888 11  /* AABBRRGG OpenGL/PNG format R=lsb A=MSB ("ABGR32" little endian) */
   #define STREAM_SUBT_2D_BW1B     12  /* Y, 1bpp, 0 is black, 1 is white */
   #define STREAM_SUBT_2D_GREY2B   13  /* Y, 2bpp, 0 is black, 3 is white, ordered from lsb to msb  */
   #define STREAM_SUBT_2D_GREY4B   14  /* Y, 4bpp, 0 is black, 15 is white, ordered from lsb to msb */
   #define STREAM_SUBT_2D_GREY8B   15  /* Grey 8b, 0 is black, 255 is white */


/* IO_DOMAIN_ANALOG_IN     : subtypes and tuning  SUBTYPE_FMT1 */
/* IO_DOMAIN_ANALOG_OUT : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_ANA_ANY             0 /*        any                        */        
   #define STREAM_SUBT_ANA_METER           1 /* m         meter                   */
   #define STREAM_SUBT_ANA_KGRAM           2 /* kg        kilogram                */
   #define STREAM_SUBT_ANA_GRAM            3 /* g         gram                    */
   #define STREAM_SUBT_ANA_SECOND          4 /* s         second                  */
   #define STREAM_SUBT_ANA_AMPERE          5 /* A         ampere                  */
   #define STREAM_SUBT_ANA_KELVIB          6 /* K         kelvin                  */
   #define STREAM_SUBT_ANA_CANDELA         7 /* cd        candela                 */
   #define STREAM_SUBT_ANA_MOLE            8 /* mol       mole                    */
   #define STREAM_SUBT_ANA_HERTZ           9 /* Hz        hertz                   */
   #define STREAM_SUBT_ANA_RADIAN         10 /* rad       radian                  */
   #define STREAM_SUBT_ANA_STERADIAN      11 /* sr        steradian               */
   #define STREAM_SUBT_ANA_NEWTON         12 /* N         newton                  */
   #define STREAM_SUBT_ANA_PASCAL         13 /* Pa        pascal                  */
   #define STREAM_SUBT_ANA_JOULE          14 /* J         joule                   */
   #define STREAM_SUBT_ANA_WATT           15 /* W         watt                    */
   #define STREAM_SUBT_ANA_COULOMB        16 /* C         coulomb                 */
   #define STREAM_SUBT_ANA_VOLT           17 /* V         volt                    */
   #define STREAM_SUBT_ANA_FARAD          18 /* F         farad                   */
   #define STREAM_SUBT_ANA_OHM            19 /* Ohm       ohm                     */
   #define STREAM_SUBT_ANA_SIEMENS        20 /* S         siemens                 */
   #define STREAM_SUBT_ANA_WEBER          21 /* Wb        weber                   */
   #define STREAM_SUBT_ANA_TESLA          22 /* T         tesla                   */
   #define STREAM_SUBT_ANA_HENRY          23 /* H         henry                   */
   #define STREAM_SUBT_ANA_CELSIUSDEG     24 /* Cel       degrees Celsius         */
   #define STREAM_SUBT_ANA_LUMEN          25 /* lm        lumen                   */
   #define STREAM_SUBT_ANA_LUX            26 /* lx        lux                     */
   #define STREAM_SUBT_ANA_BQ             27 /* Bq        becquerel               */
   #define STREAM_SUBT_ANA_GRAY           28 /* Gy        gray                    */
   #define STREAM_SUBT_ANA_SIVERT         29 /* Sv        sievert                 */
   #define STREAM_SUBT_ANA_KATAL          30 /* kat       katal                   */
   #define STREAM_SUBT_ANA_METERSQUARE    31 /* m2        square meter (area)     */
   #define STREAM_SUBT_ANA_CUBICMETER     32 /* m3        cubic meter (volume)    */
   #define STREAM_SUBT_ANA_LITER          33 /* l         liter (volume)                               */
   #define STREAM_SUBT_ANA_M_PER_S        34 /* m/s       meter per second (velocity)                  */
   #define STREAM_SUBT_ANA_M_PER_S2       35 /* m/s2      meter per square second (acceleration)       */
   #define STREAM_SUBT_ANA_M3_PER_S       36 /* m3/s      cubic meter per second (flow rate)           */
   #define STREAM_SUBT_ANA_L_PER_S        37 /* l/s       liter per second (flow rate)                 */
   #define STREAM_SUBT_ANA_W_PER_M2       38 /* W/m2      watt per square meter (irradiance)           */
   #define STREAM_SUBT_ANA_CD_PER_M2      39 /* cd/m2     candela per square meter (luminance)         */
   #define STREAM_SUBT_ANA_BIT            40 /* bit       bit (information content)                    */
   #define STREAM_SUBT_ANA_BIT_PER_S      41 /* bit/s     bit per second (data rate)                   */
   #define STREAM_SUBT_ANA_LATITUDE       42 /* lat       degrees latitude[1]                          */
   #define STREAM_SUBT_ANA_LONGITUDE      43 /* lon       degrees longitude[1]                         */
   #define STREAM_SUBT_ANA_PH             44 /* pH        pH value (acidity; logarithmic quantity)     */
   #define STREAM_SUBT_ANA_DB             45 /* dB        decibel (logarithmic quantity)               */
   #define STREAM_SUBT_ANA_DBW            46 /* dBW       decibel relative to 1 W (power level)        */
   #define STREAM_SUBT_ANA_BSPL           47 /* Bspl      bel (sound pressure level; log quantity)     */
   #define STREAM_SUBT_ANA_COUNT          48 /* count     1 (counter value)                            */
   #define STREAM_SUBT_ANA_PER            49 /* /         1 (ratio e.g., value of a switch; )          */
   #define STREAM_SUBT_ANA_PERCENT        50 /* %         1 (ratio e.g., value of a switch; )          */
   #define STREAM_SUBT_ANA_PERCENTRH      51 /* %RH       Percentage (Relative Humidity)               */
   #define STREAM_SUBT_ANA_PERCENTEL      52 /* %EL       Percentage (remaining battery energy level)  */
   #define STREAM_SUBT_ANA_ENERGYLEVEL    53 /* EL        seconds (remaining battery energy level)     */
   #define STREAM_SUBT_ANA_1_PER_S        54 /* 1/s       1 per second (event rate)                    */
   #define STREAM_SUBT_ANA_1_PER_MIN      55 /* 1/min     1 per minute (event rate, "rpm")             */
   #define STREAM_SUBT_ANA_BEAT_PER_MIN   56 /* beat/min  1 per minute (heart rate in beats per minute)*/
   #define STREAM_SUBT_ANA_BEATS          57 /* beats     1 (Cumulative number of heart beats)         */
   #define STREAM_SUBT_ANA_SIEMPERMETER   58 /* S/m       Siemens per meter (conductivity)             */
   #define STREAM_SUBT_ANA_BYTE           59 /* B         Byte (information content)                   */
   #define STREAM_SUBT_ANA_VOLTAMPERE     60 /* VA        volt-ampere (Apparent Power)                 */
   #define STREAM_SUBT_ANA_VOLTAMPERESEC  61 /* VAs       volt-ampere second (Apparent Energy)         */
   #define STREAM_SUBT_ANA_VAREACTIVE     62 /* var       volt-ampere reactive (Reactive Power)        */
   #define STREAM_SUBT_ANA_VAREACTIVESEC  63 /* vars      volt-ampere-reactive second (Reactive Energy)*/
   #define STREAM_SUBT_ANA_JOULE_PER_M    64 /* J/m       joule per meter (Energy per distance)        */
   #define STREAM_SUBT_ANA_KG_PER_M3      65 /* kg/m3     kg/m3 (mass density, mass concentration)     */
   #define STREAM_SUBT_ANA_DEGREE         66 /* deg       degree (angle)                               */
   #define STREAM_SUBT_ANA_NTU            67 /* NTU       Nephelometric Turbidity Unit                 */

   // Secondary Unit (rfc8798)           Description          SenML Unit     Scale     Offset 
   #define STREAM_SUBT_ANA_MS             68 /* millisecond                  s      1/1000    0       1ms = 1s x [1/1000] */
   #define STREAM_SUBT_ANA_MIN            69 /* minute                       s      60        0        */
   #define STREAM_SUBT_ANA_H              70 /* hour                         s      3600      0        */
   #define STREAM_SUBT_ANA_MHZ            71 /* megahertz                    Hz     1000000   0        */
   #define STREAM_SUBT_ANA_KW             72 /* kilowatt                     W      1000      0        */
   #define STREAM_SUBT_ANA_KVA            73 /* kilovolt-ampere              VA     1000      0        */
   #define STREAM_SUBT_ANA_KVAR           74 /* kilovar                      var    1000      0        */
   #define STREAM_SUBT_ANA_AH             75 /* ampere-hour                  C      3600      0        */
   #define STREAM_SUBT_ANA_WH             76 /* watt-hour                    J      3600      0        */
   #define STREAM_SUBT_ANA_KWH            77 /* kilowatt-hour                J      3600000   0        */
   #define STREAM_SUBT_ANA_VARH           78 /* var-hour                     vars   3600      0        */
   #define STREAM_SUBT_ANA_KVARH          79 /* kilovar-hour                 vars   3600000   0        */
   #define STREAM_SUBT_ANA_KVAH           80 /* kilovolt-ampere-hour         VAs    3600000   0        */
   #define STREAM_SUBT_ANA_WH_PER_KM      81 /* watt-hour per kilometer      J/m    3.6       0        */
   #define STREAM_SUBT_ANA_KIB            82 /* kibibyte                     B      1024      0        */
   #define STREAM_SUBT_ANA_GB             83 /* gigabyte                     B      1e9       0        */
   #define STREAM_SUBT_ANA_MBIT_PER_S     84 /* megabit per second           bit/s  1000000   0        */
   #define STREAM_SUBT_ANA_B_PER_S        85 /* byteper second               bit/s  8         0        */
   #define STREAM_SUBT_ANA_MB_PER_S       86 /* megabyte per second          bit/s  8000000   0        */
   #define STREAM_SUBT_ANA_MV             87 /* millivolt                    V      1/1000    0        */
   #define STREAM_SUBT_ANA_MA             88 /* milliampere                  A      1/1000    0        */
   #define STREAM_SUBT_ANA_DBM            89 /* decibel rel. to 1 milliwatt  dBW    1       -30     0 dBm = -30 dBW       */
   #define STREAM_SUBT_ANA_UG_PER_M3      90 /* microgram per cubic meter    kg/m3  1e-9      0        */
   #define STREAM_SUBT_ANA_MM_PER_H       91 /* millimeter per hour          m/s    1/3600000 0        */
   #define STREAM_SUBT_ANA_M_PER_H        92 /* meterper hour                m/s    1/3600    0        */
   #define STREAM_SUBT_ANA_PPM            93 /* partsper million             /      1e-6      0        */
   #define STREAM_SUBT_ANA_PER_100        94 /* percent                      /      1/100     0        */
   #define STREAM_SUBT_ANA_PER_1000       95 /* permille                     /      1/1000    0        */
   #define STREAM_SUBT_ANA_HPA            96 /* hectopascal                  Pa     100       0        */
   #define STREAM_SUBT_ANA_MM             97 /* millimeter                   m      1/1000    0        */
   #define STREAM_SUBT_ANA_CM             98 /* centimeter                   m      1/100     0        */
   #define STREAM_SUBT_ANA_KM             99 /* kilometer                    m      1000      0        */
   #define STREAM_SUBT_ANA_KM_PER_H      100 /* kilometer per hour           m/s    1/3.6     0        */
                                                                                                 
   #define STREAM_SUBT_ANA_GRAVITY       101 /* earth gravity                m/s2   9.81      0       1g = m/s2 x 9.81     */
   #define STREAM_SUBT_ANA_DPS           102 /* degrees per second           1/s    360       0     1dps = 1/s x 1/360     */   
   #define STREAM_SUBT_ANA_GAUSS         103 /* Gauss                        Tesla  10-4      0       1G = Tesla x 1/10000 */
   #define STREAM_SUBT_ANA_VRMS          104 /* Volt rms                     Volt   0.707     0    1Vrms = 1Volt (peak) x 0.707 */
   #define STREAM_SUBT_ANA_MVPGAUSS      105 /* Hall effect, mV/Gauss        millivolt 1      0    1mV/Gauss                    */

/* IO_DOMAIN_RTC               : subtypes and tuning  SUBTYPE_FMT1 */

/* IO_DOMAIN_USER_INTERFACE_IN    11 : subtypes and tuning  SUBTYPE_FMT1 */

/* IO_DOMAIN_USER_INTERFACE_OUT   12 : subtypes and tuning  SUBTYPE_FMT1 */


/* 
   ================================= stream_format  FORMATS =======================================
    
    Format 23+4_offsets for buffer BASE ADDRESS
    Frame SIZE and ring indexes are using 22bits linear (0..4MB)
*/

#define STREAM_FORMAT_SIZE_W32 4     /*  digital, common part of the format  */
/*
*   STREAM_DATA_START_data_format (size multiple of 3 x uint32_t)
*   _FMT0 word 0 : common to all domains : frame size
*   _FMT1 word 1 : common to all domains : time-stamp, nchan, raw format, interleaving, domain, size extension
*   _FMT2 word 2 : specific to domains : hashing, sampling rate
*   _FMT3 word 3 : specific to domains : hashing, channel mapping 
*/

/* for MISRA-2012 compliance to Rule 10.4 */
#define U8(x) ((uint8_t)(x)) 
#define S8(x) ((int8_t)(x)) 

//enum time_stamp_format_type {
#define NO_TS 0
#define ABS_TS 1                     /* long time reference stream_time64 */
#define REL_TS 2                     /* time difference from previous frame packet in stream_time_seconds format */
#define COUNTER_TS 3                 /* 32bits counter of data frames */

//enum hashing_type {
#define NO_HASHING 0                 /* cipher protocol under definition */
#define HASHING_ON 1                 /* the stream is ciphered (HMAC-SHA256 / stream_encipher XTEA)*/

//enum frame_format_type {
#define FMT_INTERLEAVED 0           /* "arc_descriptor_interleaved" for example L/R audio or IMU stream..   */
                                    /* the pointer associated to the stream points to data (L/R/L/R/..)     */
#define FMT_DEINTERLEAVED_1PTR 1    /* single pointer to the first channel, next channel base address is    */
                                    /*  computed by adding the frame size or buffer size/nchan, also for ring buffers  */
////+enum frame_format_synchro {
#define SYNCHRONOUS 0               /* tells the output buffer size is NOT changing */
#define ASYNCHRONOUS 1              /* tells the output frame length is variable, input value "Size" tells the maximum value  
//                                       data format : optional time-stamp (stream_time_stamp_format_type)
//                                                    domain [2bits] and sub-domain [6bits rfc8428]
//                                                    payload : [nb samples] [samples]  */
////enum direction_rxtx {
#define IODIRECTION_RX 0              /* RX from the Graph pont of view */
#define IODIRECTION_TX 1


/*  WORD 0 - frame size --------------- */
#define FRAMESZ_FMT0   0           
    /* frame size in bytes for one deinterleaved channel Byte-acurate up to 4MBytes       */
    /* raw interleaved buffer size is framesize x nb channel, max = 4MB x nchan           */
    /* in node manifests it gives the minimum input size (grain) before activating the node */
    /* A "frame" is the combination of several channels sampled at the same time          */
    /* A value =0 means the size is any or defined by the IO AL.                          */
    /* For sensors delivering burst of data not isochronous, it gives the maximum         */
    /* framesize; same comment for the sampling rate.                                     */
    /* The frameSize is including the time-stamp field                                    */
    #define __________FMT0_MSB 31 /* 10 reserved */
    #define __________FMT0_LSB 22 
    #define FRAMESIZE_FMT0_MSB 21 /* 22 frame size, bit-field of the same size as READ_ARCW2 */
    #define FRAMESIZE_FMT0_LSB  0 

/*- WORD 1 - domain, sub-types , size extension, time-stamp, raw format, interleaving, nchan  -------------*/
#define NCHANDOMAIN_FMT1   1
    #define    _______FMT1_MSB  31 /*     */  
    #define    _______FMT1_LSB  30 /* 2 reserved */
    #define   SUBTYPE_FMT1_MSB  29 /*     */  
    #define   SUBTYPE_FMT1_LSB  23 /* 7  sub-type for pixels and analog formats (STREAM_SUBT_ANA_)  */
    #define FSZEXTEND_FMT1_MSB  22 /*    Frame Size are used with to extend by <<(2x{0..7}) */
    #define FSZEXTEND_FMT1_LSB  20 /* 3  to  256MB, 4GB, 64GB , for use-cases with NN models, video players, etc */
    #define    DOMAIN_FMT1_MSB  19 
    #define    DOMAIN_FMT1_LSB  16 /* 4  STREAM_IO_DOMAIN */
    #define       RAW_FMT1_MSB  15
    #define       RAW_FMT1_LSB  10 /* 6  arithmetics stream_raw_data 6bits (0..63)  */
    #define  TSTPSIZE_FMT1_MSB   9 
    #define  TSTPSIZE_FMT1_LSB   8 /* 2  16/32/64/64TXT time-stamp time format */
    #define  TIMSTAMP_FMT1_MSB   7 
    #define  TIMSTAMP_FMT1_LSB   6 /* 2  time_stamp_format_type for time-stamped streams for each interleaved frame */
    #define INTERLEAV_FMT1_MSB   5       
    #define INTERLEAV_FMT1_LSB   5 /* 1  interleaving : frame_format_type */
    #define   NCHANM1_FMT1_MSB   4 
    #define   NCHANM1_FMT1_LSB   0 /* 5  nb channels-1 [1..32] */

/*  WORD2 - sampling rate */
#define SAMPLINGRATE_FMT2   2
    /*--------------- WORD 2 -------------*/
    #define      FS1D_FMT2_MSB  31 /* 24 truncated IEEE-754 to 24 bits, 0 means "asynchronous" or "any" */
    #define      FS1D_FMT2_LSB   8 /*    FP24_S_E8_M15 in [Hz]   */
    #define _______1D_FMT2_MSB   7
    #define _______1D_FMT2_LSB   0 /* 8  reserved */


/*  WORD 3 for IO_DOMAIN_AUDIO_IN and IO_DOMAIN_AUDIO_OUT */
#define DOMAINSPECIFIC_FMT3   3
    #define   ________FMT3_MSB U(31) /* 8b   */
    #define   ________FMT3_LSB U(24) 
    #define AUDIO_MAP_FMT3_MSB U(23) /* 24 mapping of channels example of 7.1 format (8 channels): */
    #define AUDIO_MAP_FMT3_LSB U( 0) /*     FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..*/

    
    /*--------------- WORD 3 for IO_DOMAIN_MOTION */
    //enum imu_channel_format /* uint8_t : data interleaving possible combinations */
    enum imu_channel_format { aXg0m0=1, /* only accelerometer */
        a0gXm0=2, /* only gyroscope */
        a0g0mX=3, /* only magnetometer */
        aXgXm0=4, /* A + G */
        aXg0mX=5, /* A + M */
        a0gXmX=6, /* G + M */
        aXgXmX=7, /* A + G + M */
    };
    #define MOTION_MAPPING_FMT3_MSB U(31) /* 28 reserved for mapping information */
    #define MOTION_MAPPING_FMT3_LSB U( 4) 
    #define MOTION_TEMPERA_FMT3_MSB U( 3) /*  1 temperature capture */
    #define MOTION_TEMPERA_FMT3_LSB U( 3) /*     */
    #define MOTION_DATAFMT_FMT3_MSB U( 2) /*  3 imu_channel_format : A, A+G, A+G+M, .. MSB used for the temperature */
    #define MOTION_DATAFMT_FMT3_LSB U( 0) /*     */
   

    /*--------------- WORD 3  for IO_DOMAIN_2D_IN and IO_DOMAIN_2D_OUT */
    // IO_DOMAIN_2D_IN              camera sensor, video sensor */
    // IO_DOMAIN_2D_OUT             display, led matrix, */

    /* EXTEND_FMT1 gives some margin of size extension */
    enum ratio_2d_fmt3 { R2D_TBD=0, R2D_1_1=1, R2D_4_3=2, R2D_16_9=3, R2D_3_2=4 };

    #define   ___________FMT3_MSB U(31) /* 14 */
    #define   ___________FMT3_LSB U(18)
    #define   I2D_BORDER_FMT3_MSB U(17) /*  2 pixel border 0,1,2,3   */
    #define   I2D_BORDER_FMT3_LSB U(16)
    #define I2D_VERTICAL_FMT3_MSB U(15) /*  1 set to 0 for horizontal, 1 for vertical */
    #define I2D_VERTICAL_FMT3_LSB U(15)
    #define     RATIO_2D_FMT2_MSB U(14) /*  3 ratio_2d_fmt3 */
    #define     RATIO_2D_FMT2_LSB U(12) 
    #define I2D_SMALLDIM_FMT3_MSB U(11) /* 12 smallest pixel dimension, the largest comes with a division with */
    #define I2D_SMALLDIM_FMT3_LSB U( 0) /*    largest dimension = (frame_size - time_stamp_size)/smallest_dimension 
                                              or  largest dimension = ratio x smallest_dimension  */


/*=====================================================================================*/                          
/*
    commands 
        from the application to the graph scheduler         arm_graph_interpreter(command,  arm_stream_instance_t *S, uint8_t *, uint32_t);
        from the graph scheduler to the nanoApps            S->address_node (command, stream_handle_t instance, stream_xdmbuffer_t *, uint32_t *);
        from the Scripts to the IO configuration setting    arm_stream_services (command, uint8_t *, uint8_t *, uint8_t *, uint32_t)
*/

    /*  FROM APP TO SCHEDULER :     arm_graph_interpreter (STREAM_RESET, &stream_instance, 0, 0); 
        FROM SCHEDULER to NODE :     devxx_fyyyy (STREAM_RESET, &node_instance, &memreq, &status); 
            Command + nb arcs, preset 0..15m TAG 0..255
        -  (STREAM_RESET, ptr1, ptr2, ptr3); 
            ptr1 = instance pointer, memory banks
            ptr2 = stream_al_services function address, followed by all the arc format (node manifest node_using_arc_format =1)
        -  (STREAM_SET_PARAMETER, ptr1, ptr2, ptr3); 
            ptr1 = instance
            ptr2 = byte pointer to parameters, depends on the TAG 
        -  (STREAM_READ_PARAMETER, ptr1, ptr2, ptr3); 
            ptr1 = instance
            ptr2 = parameter data destination, depends on the TAG
        -  (STREAM_RUN, ptr1, ptr2, ptr3); 
            ptr1 = instance
            ptr2 = list of XDM arc buffers (X,n) , the size field means :
                 rx arc . size = amount of data available for processing
                 tx arc . size = amount of free area in the buffer 
                when XDM11=0 (node_same_rxtx_data_rate=0) NODE updates the XDM size fields with :
                 rx arc . size = amount of data consumed
                 tx arc . size = amount of data produced
        -  (STREAM_STOP, ptr1, ptr2, ptr3); 
    */
    #define STREAM_RESET            1   /* arm_graph_interpreter(STREAM_RESET, *instance, * memory_results) */
    #define STREAM_SET_PARAMETER    2   /* APP sets NODE parameters node instances are protected by multithread effects when 
                                          changing parmeters on the fly, used to exchange the unlock key */

    //#define STREAM_SET_IO_CONFIG STREAM_SET_PARAMETER 
    /*  @@@ TODO
            reconfigure the IO : p_io_function_ctrl(STREAM_SET_IO_CONFIG + (FIFO_ID<<NODE_TAG_CMD), 0, new_configuration_index) 

            io_power_mode     0             ; to set the device at boot time in stop / off (0)
                                            ; running mode(1) : digital conversion (BIAS always powered for analog peripherals )
                                            ; running mode(2) : digital conversion BIAS shut-down between conversions
                                            ; Sleep (3) Bias still powered but not digital conversions            
            */

    #define STREAM_READ_PARAMETER   3   /* used from script */
    #define STREAM_RUN              4   /* arm_graph_interpreter(STREAM_RUN, instance, *in_out) */
    #define STREAM_STOP             5   /* arm_graph_interpreter(STREAM_STOP, instance, 0)  node calls free() if it used stdlib's malloc */
    #define STREAM_UPDATE_RELOCATABLE 6 /* update the nanoAppRT pointers to relocatable memory segments */

    #define STREAM_SET_BUFFER       7   /* platform_IO(STREAM_SET_BUFFER, *data, size)  */


/*  FROM THE GRAPH SCHEDULER TO THE NANOAPPS   NODE_COMMANDS  */
    #define  _UNUSED2_CMD_MSB U(31)       
    #define  _UNUSED2_CMD_LSB U(24) /* 8 _______ */
    #define   NODE_TAG_CMD_MSB U(23) /*    parameter, function selection / debug arc index / .. */      
    #define   NODE_TAG_CMD_LSB U(16) /* 8  instanceID for the trace / FIFO_ID for status checks */
    #define    PRESET_CMD_MSB U(15)       
    #define    PRESET_CMD_LSB U(12) /* 4  #16 presets */
    #define      NARC_CMD_MSB U(11)       
    #define      NARC_CMD_LSB U( 8) /* 4 number of arcs */
    #define  __UNUSED_CMD_MSB U( 7)       
    #define  __UNUSED_CMD_LSB U( 5) /* 3 _______ */
    #define  COMMDEXT_CMD_MSB U( 4)       
    #define  COMMDEXT_CMD_LSB U( 4) /* 1 command option (RESET + warmboot)  */
    #define   COMMAND_CMD_MSB U( 3)       
    #define   COMMAND_CMD_LSB U( 0) /* 4 command */

    #define PACK_COMMAND(SWCTAG,PRESET,NARC,EXT,CMD) (((SWCTAG)<<NODE_TAG_CMD_LSB)|((PRESET)<<PRESET_CMD_LSB)|((NARC)<<NARC_CMD_LSB)|((EXT)<<COMMDEXT_CMD_LSB)|(CMD))

/*================================================================================================================*/    
/*
    "stream_service_command"  from the nodes, to "arm_stream_services"

    void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n)
*/

#define NOOPTION_SSRV 0
#define NOCONTROL_SSRV 0

/* arm_stream_services COMMAND */
#define  CONTROL_SSRV_MSB U(31)       
#define  CONTROL_SSRV_LSB U(24) /* 8   set/init/wait completion, in case of coprocessor usage */
#define   OPTION_SSRV_MSB U(23)       
#define   OPTION_SSRV_LSB U(20) /* 4   compute accuracy, in-place processing, frame size .. */
#define FUNCTION_SSRV_MSB U(19)       
#define FUNCTION_SSRV_LSB U( 4) /* 16   64K functions/group  */
#define    GROUP_SSRV_MSB U( 3)       
#define    GROUP_SSRV_LSB U( 0) /* 4    16 groups */


/* clears the MALLOC_SSRV field */
#define PACK_SERVICE(CTRL,OPTION,FUNC,GROUP) (((CTRL)<<CONTROL_SSRV_LSB)|((OPTION)<<OPTION_SSRV_LSB)|((FUNC)<<FUNCTION_SSRV_LSB)|(GROUP)<<GROUP_SSRV_LSB)

/*
    commands from the NODE to Stream
    16 family of commands:
    - 1 : internal to Stream, reset, debug trace, report errors
    - 2 : arc access for scripts : pointer, last data, debug fields, format changes
    - 3 : format converters (time, raw data)
    - 4 : stdlib.h subset (time, stdio)
    - 5 : math.h subset
    - 6 : Interface to CMSIS-DSP
    - 7 : Interface to CMSIS-NN 
    - 8 : Multimedia audio library
    - 9 : Image processing library
    - 10..15 : reserved

    each family can define 256 operations (TAG_CMD_LSB)
*/
//enum stream_service_group     

                /* needs a registered return address (Y/N)  (TBD @@@) */
#define STREAM_SERVICE_INTERNAL     1   /* Y */
#define STREAM_SERVICE_FLOW         2   /* Y */
#define STREAM_SERVICE_CONVERSION   3   /* N */
#define STREAM_SERVICE_STDLIB       4   /* Y */
#define STREAM_SERVICE_MATH         5   /* N */
#define STREAM_SERVICE_DSP_ML       6   /* N */
#define STREAM_SERVICE_DEEPL        7   /* N */
#define STREAM_SERVICE_MM_AUDIO     8   /* Y */
#define STREAM_SERVICE_MM_IMAGE     9   /* Y */


//{
/* 1/STREAM_SERVICE_INTERNAL ------------------------------------------------ */

#define STREAM_SERVICE_INTERNAL_RESET 1u
#define STREAM_SERVICE_INTERNAL_NODE_REGISTER 2u

/* change stream format from NODE media decoder, script applying change of use-case (IO_format, vocoder frame-size..): sampling, nb of channel, 2D frame size */
#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE 3u      

//#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_FS 3u       /* NODE information for a change of stream format, sampling, nb of channel */
//#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_NCHAN 4u     /* raw data sample, mapping of channels, (web radio use-case) */
//#define STREAM_SERVICE_INTERNAL_FORMAT_UPDATE_RAW 5u

#define STREAM_SERVICE_INTERNAL_SECURE_ADDRESS 6u       /* this call is made from the secured address */
#define STREAM_SERVICE_INTERNAL_AUDIO_ERROR 7u          /* PLC applied, Bad frame (no header, no synchro, bad data format), bad parameter */
#define STREAM_SERVICE_INTERNAL_DEBUG_TRACE 8u          /* 1b, 1B, 16char */
#define STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STAMPS 9u
#define STREAM_SERVICE_INTERNAL_AVAILABLE 10u
#define STREAM_SERVICE_INTERNAL_SETARCDESC 11u          /* buffers holding MP3 songs.. rewind from script, 
                                                            switch a NN model to another, change a parameter-set using arcs */
#define STREAM_SERVICE_INTERNAL_KEYEXCHANGE 12          /* at reset time : key exchanges */

#define STREAM_SERVICE_CPU_CLOCK_UPDATE 13              /* notification from the application of the CPU clock setting (TBD @@@) */

//STREAM_SERVICE_INTERNAL_DEBUG_TRACE, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_1B, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_DIGIT, 
// 
//STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STAMPS, STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STRING,
// 
//STREAM_SAVE_HOT_PARAMETER, 

//STREAM_LOW_POWER,     /* interface to low-power platform settings, "wake-me in 24h with deep-sleep in-between" */
//                          " I have nothing to do most probably for the next 100ms, do what is necessary "

 
//STREAM_PROC_ARCH,     /* returns the processor architecture details, used before executing specific assembly codes */

/* 2/STREAM_SERVICE_FLOW ------------------------------------------------ */
//for scripts:
    #define STREAM_SERVICE_FLOW_ARC_RWPTR 1 
    #define STREAM_SERVICE_FLOW_ARC_DATA 2 
    #define STREAM_SERVICE_FLOW_ARC_FILLING 3

//for scripts/Nodes: fast data moves
    #define STREAM_SERVICE_FLOW_DMA_SET 4       /* set src/dst/length */
    #define STREAM_SERVICE_FLOW_DMA_START 5
    #define STREAM_SERVICE_FLOW_DMA_STOP 6
    #define STREAM_SERVICE_FLOW_DMA_CHECK 7

/* 3/STREAM_SERVICE_CONVERSION ------------------------------------------------ */
    #define STREAM_SERVICE_CONVERSION_INT16_FP32 1


/* 4/STREAM_SERVICE_STDLIB ------------------------------------------------ */
    /* stdlib.h */
    /* string.h */
    //STREAM_MEMSET, STREAM_STRCHR, STREAM_STRLEN,
    //STREAM_STRNCAT, STREAM_STRNCMP, STREAM_STRNCPY, STREAM_STRSTR, STREAM_STRTOK,
    #define STREAM_RAND     1 /* (STREAM_RAND + OPTION_SSRV(seed), *ptr1, 0, 0, n) */
    #define STREAM_SRAND    2
    #define STREAM_ATOF     3
    #define STREAM_ATOI     4
    #define STREAM_MEMSET   5
    #define STREAM_STRCHR   6
    #define STREAM_STRLEN   7
    #define STREAM_STRNCAT  8
    #define STREAM_STRNCMP  9
    #define STREAM_STRNCPY  10
    #define STREAM_STRSTR   11
    #define STREAM_STRTOK   12
    //STREAM_RAND, STREAM_SRAND, STREAM_ATOF, STREAM_ATOI
    #define STREAM_FREE     13
    #define STREAM_MALLOC   14


/* 5/STREAM_SERVICE_MATH ------------------------------------------------ */

    /* returns a code corresponding to the processor architecture and its FPU options */
#define STREAM_SERVICE_CHECK_ARCHITECTURE 2
    /* time.h */
    //STREAM_ASCTIMECLOCK, STREAM_DIFFTIME, STREAM_SYS_CLOCK (ms since reset), STREAM_TIME (linux seconds)
    //STREAM_READ_TIME (high-resolution timer), STREAM_READ_TIME_FROM_START, 
    //STREAM_TIME_DIFFERENCE, STREAM_TIME_CONVERSION,  
    // 
    //STREAM_TEA,

    /* From Android CHRE  https://source.android.com/docs/core/interaction/contexthub
    String/array utilities: memcmp, memcpy, memmove, memset, strlen
    Math library: Commonly used single-precision floating-point functions:
    Basic operations: ceilf, fabsf, floorf, fmaxf, fminf, fmodf, roundf, lroundf, remainderf
    Exponential/power functions: expf, log2f, powf, sqrtf
    Trigonometric/hyperbolic functions: sinf, cosf, tanf, asinf, acosf, atan2f, tanhf
    */
#define STREAM_SERVICE_SQRT_Q15       15
#define STREAM_SERVICE_SQRT_F32       16
#define STREAM_SERVICE_LOG_Q15        17
#define STREAM_SERVICE_LOG_F32        18

#define STREAM_SERVICE_SINE_Q15       19
#define STREAM_SERVICE_SINE_F32       20
#define STREAM_SERVICE_COS_Q15        21
#define STREAM_SERVICE_COS_F32        22
#define STREAM_SERVICE_ATAN2_Q15      23
#define STREAM_SERVICE_ATAN2_F32      24

#define STREAM_SERVICE_SORT 3


/* 6/STREAM_SERVICE_DSP_ML ------------------------------------------------ */

#define STREAM_SERVICE_NO_INIT         0    /* CONTROL_SSRV_LSB field*/
#define STREAM_SERVICE_INIT_RETASAP    1    /* return even when computation is not finished */
#define STREAM_SERVICE_INIT_WAIT_COMP  2    /* tell to return when processing completed */
#define STREAM_SERVICE_CHECK_COPROCESSOR 3  /* check for services()*/
#define STREAM_SERVICE_CHECK_END_COMP  4    /* check completion for the caller */


#define STREAM_SERVICE_CASCADE_DF1_Q15 1    /* IIR filters, use STREAM_SERVICE_CHECK_COPROCESSOR */
#define STREAM_SERVICE_CASCADE_DF1_Q15_CHECK_COMPLETION 2    
#define STREAM_SERVICE_CASCADE_DF1_F32 3         
#define STREAM_SERVICE_CASCADE_DF1_F32_CHECK_COMPLETION 4

#define STREAM_SERVICE_rFFT_Q15        5    /* RFFT windowing, module, dB , use STREAM_SERVICE_CHECK_COPROCESSOR */
#define STREAM_SERVICE_rFFT_Q15_CHECK_COMPLETION        6
#define STREAM_SERVICE_rFFT_F32        7
#define STREAM_SERVICE_rFFT_F32_CHECK_COMPLETION        8
                                       
#define STREAM_SERVICE_cFFT_Q15        8    /* cFFT windowing, module, dB */
#define STREAM_SERVICE_cFFT_F32       10
                                       
#define STREAM_SERVICE_DFT_Q15        12    /* DFT/Goertzel windowing, module, dB */
#define STREAM_SERVICE_DFT_F32        14

#define STREAM_WINDOWS                /* windowing for spectral estimations */
#define STREAM_FIR                    
#define STREAM_FC                     /* fully connected layer Mat x Vec */
#define STREAM_CNN                    /* convolutional NN : 3x3 5x5 fixed-weights */
#define STREAM_CONV2D                 /* Sobel */

#define   OPTION_AL_SRV_MSB U(25)       
#define   OPTION_AL_SRV_LSB U(10) /* 16   .. */
#define FUNCTION_AL_SRV_MSB U( 9)       
#define FUNCTION_AL_SRV_LSB U( 4) /* 6    64 functions/group  */
#define    GROUP_AL_SRV_MSB U( 3)       
#define    GROUP_AL_SRV_LSB U( 0) /* 4    16 groups */  
#define PACK_AL_SERVICE(OPTION,FUNC,GROUP) (((OPTION)<<OPTION_AL_SRV_LSB)|((FUNC)<<FUNCTION_AL_SRV_LSB)|(GROUP)<<GROUP_AL_SRV_LSB)


/* 7/STREAM_SERVICE_DEEPL ------------------------------------------------ */

/* 8/STREAM_SERVICE_MM_AUDIO ------------------------------------------------ */

/* 9/STREAM_SERVICE_MM_IMAGE ------------------------------------------------ */


/*
* system subroutines : 
* - IO settings : 
* - Get Time, in different formats, and conversion, extract time-stamps
* - Get Peripheral data : RSSI, MAC/IP address
* - Low-level : I2C string of commands, GPIO, physical address to perpherals
*/
#define PLATFORM_DEEPSLEEP_ENABLED 0x20   /* deep-sleep activation is possible when returning from arm_graph_interpreter(STREAM_RUN..) */
#define PLATFORM_TIME_SET          0x21
#define PLATFORM_RTC_SET           0x22
#define PLATFORM_TIME_READ         0x23
#define PLATFORM_HW_WORD_READ      0x24  
#define PLATFORM_HW_WORD_WRITE     0x25  
#define PLATFORM_HW_BYTE_READ      0x26  
#define PLATFORM_HW_BYTE_WRITE     0x27  

//enum error_codes 
#define ERROR_MEMORY_ALLOCATION     1u


/*
    STREAM SERVICES
*/

#define  UNUSED_SRV_MSB U(31)
#define  UNUSED_SRV_LSB U(16) /* 16 reserved */
#define    INST_SRV_MSB U(15)       
#define    INST_SRV_LSB U(12) /* 4  instance */
#define   GROUP_SRV_MSB U(11)       
#define   GROUP_SRV_LSB U( 8) /* 4  command family groups under compilation options (DSP, Codec, Stdlib, ..) */
#define COMMAND_SRV_MSB U( 7)       
#define COMMAND_SRV_LSB U( 0) /* 8  256 service IDs */


/*
    Up to 16 family of processing extensions "SERVICE_COMMAND_GROUP"
    EXTDSPML EXTMATH EXTAUDIO EXTIMAGE EXTSTDLIB
*/

#define EXT_SERVICE_MATH   1
#define EXT_SERVICE_DSPML  2
#define EXT_SERVICE_AUDIO  3
#define EXT_SERVICE_IMAGE  4 
#define EXT_SERVICE_STDLIB 5



//------------------------------------------------------------------------------------------------------
/*  Time format - 64 bits
 * 
 *  140 years ~2^32 ~ 4.4 G = 0x1.0000.0000 seconds 
 *  1 year = 31.56 M seconds
 *  1 day = 86.400 seconds
 * 
 *  "stream_time64" 
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s]  140 Y + Q28 [s]
 *  systick increment for  1ms =  0x00041893 =  1ms x 2^28
 *  systick increment for 10ms =  0x0028F5C2 = 10ms x 2^28
 *
 *  140 years in ms = 4400G ms = 0x400.0000.0000 ms 42bits
 *  milliseconds from January 1st 1970 UTC (or internal AL reference)
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ______________________mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm ms 
 *
 *
 *  Local time in BINARY bit-fields : years/../millisecond, WWW=day of the week 
 *  (0=Sunday, 1=Monday..)
 *      COVESA allowed formats : ['YYYY_MM_DD', 'DD_MM_YYYY', 'MM_DD_YYYY', 'YY_MM_DD', 'DD_MM_YY', 'MM_DD_YY']
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  _________________________.YY.YY.YY.YY.MMM.DDDD.SSSSS.MM.MM.MM.WW
 * 
 */

//typedef uint64_t stream_time64;
#define   TYPE_T64_MSB 63
#define   TYPE_T64_LSB 61 
#define SECOND_T64_MSB 60
#define SECOND_T64_LSB 29
#define  FRACT_T64_MSB 28 
#define  FRACT_T64_LSB  0

/*
 * stream_time_seconds in 32bits : "stream_time32"
 *  FEDCBA9876543210FEDCBA9876543210
 *  ssssssssssssssssssssssssssssqqqq q28.4  [s] (8.5 years +/- 0.0625s)
 *  stream_time32 = (stream_time64 >> 24) 
 *
 * delta_stream_time_seconds in 32bits : "stream_time32_diff"
 *  FEDCBA9876543210FEDCBA9876543210
 *  sssssssssssssssssqqqqqqqqqqqqqqq q17.15 [s] (36h, +/- 30us)
 *  stream_time32_diff = (stream_time64 >> 13) 

 * ARC time-stamp in 32bits : "stream_timestamp32" 
 *  FEDCBA9876543210FEDCBA9876543210
 *  ssssssssssssssssssssqqqqqqqqqqqq q20.12 [s] (12 days, +/- 0.25ms)
 *  stream_time32_diff = (stream_time64 >> 10) 
 *
 */
//typedef uint32_t stream_time32;
#define   FORMAT_T32_MSB 31
#define   FORMAT_T32_LSB 30 
#define     TIME_T32_MSB 29
#define     TIME_T32_LSB  0 

/*
 *  stream_time_seconds in 16bits : "stream_time16"
 *  FEDCBA9876543210
 *  ssssssssssssqqqq q14.2   1 hour + 8mn +/- 0.0625s
 *
 *  FEDCBA9876543210
 *  stream_time_seconds differencein 16bits : "stream_time16diff"
 *  qqqqqqqqqqqqqqqq q15 [s] time difference +/- 15us
 */
//typedef uint32_t stream_time16;
#define     TIME_T16_MSB 15
#define     TIME_T16_LSB  0 


/*================================ STREAM ARITHMETICS DATA/TYPE ====================================================*/
/* types fit in 6bits, arrays start with 0, stream_bitsize_of_raw() is identical */


#define STREAM_DATA_ARRAY 0 /* see stream_array: [0NNNTT00] 0, type, nb */
#define STREAM_S1         1 /* S, one signed bit, "0" = +1 */                           /* one bit per data */
#define STREAM_U1         2 /* one bit unsigned, boolean */
#define STREAM_S2         3 /* SX  */                                                   /* two bits per data */
#define STREAM_U2         4 /* XX  */
#define STREAM_Q1         5 /* Sx ~stream_s2 with saturation management*/
#define STREAM_S4         6 /* Sxxx  */                                                 /* four bits per data */
#define STREAM_U4         7 /* xxxx  */
#define STREAM_Q3         8 /* Sxxx  */
#define STREAM_FP4_E2M1   9 /* Seem  micro-float [8 .. 64] */
#define STREAM_FP4_E3M0  10 /* Seee  [8 .. 512] */
#define STREAM_S8        11 /* Sxxxxxxx  */                                             /* eight bits per data */
#define STREAM_U8        12 /* xxxxxxxx  ASCII char, numbers.. */
#define STREAM_Q7        13 /* Sxxxxxxx  arithmetic saturation */
#define STREAM_CHAR      14 /* xxxxxxxx  */
#define STREAM_FP8_E4M3  15 /* Seeeemmm  NV tiny-float [0.02 .. 448] */                 
#define STREAM_FP8_E5M2  16 /* Seeeeemm  IEEE-754 [0.0001 .. 57344] */
#define STREAM_S16       17 /* Sxxxxxxx.xxxxxxxx  */                                    /* 2 bytes per data */
#define STREAM_U16       18 /* xxxxxxxx.xxxxxxxx  Numbers, UTF-16 characters */
#define STREAM_Q15       19 /* Sxxxxxxx.xxxxxxxx  arithmetic saturation */
#define STREAM_FP16      20 /* Seeeeemm.mmmmmmmm  half-precision float */
#define STREAM_BF16      21 /* Seeeeeeee.mmmmmmm  bfloat truncated FP32 = BFP16 = S E8 M7 */
#define STREAM_Q23       22 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx  24bits */                    /* 3 bytes per data */ 
#define STREAM_Q23_32    23 /* SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxxx  */                  /* 4 bytes per data */
#define STREAM_S32       24 /* one long word  */
#define STREAM_U32       25 /* xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx UTF-32, .. */
#define STREAM_Q31       26 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx  */
#define STREAM_FP32      27 /* Seeeeeeee.mmmmmmm.mmmmmmmm.mmmmmmmm  FP32 = S E8 M23 */             
#define STREAM_CQ15      28 /* Sxxxxxxx.xxxxxxxx Sxxxxxxx.xxxxxxxx (I Q) */             
#define STREAM_CFP16     29 /* Seeeeemm.mmmmmmmm Seeeeemm.mmmmmmmm (I Q) */             
#define STREAM_S64       30 /* long long */                                             /* 8 bytes per data */
#define STREAM_U64       31 /* unsigned  64 bits */
#define STREAM_Q63       32 /* Sxxxxxxx.xxxxxx ....... xxxxx.xxxxxxxx  */
#define STREAM_CQ31      33 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxx..*/
#define STREAM_FP64      34 /* Seeeeeee.eeemmmmm.mmmmmmm ... double  */
#define STREAM_CFP32     35 /* Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm Seee.. (I Q)  */
#define STREAM_FP128     36 /* Seeeeeee.eeeeeeee.mmmmmmm ... quadruple precision */     /* 16 bytes per data */
#define STREAM_CFP64     37 /* fp64 fp64 (I Q)  */
#define STREAM_FP256     38 /* Seeeeeee.eeeeeeee.eeeeemm ... octuple precision  */      /* 32 bytes per data */
#define STREAM_TIME16    39 /* ssssssssssssqqqq q14.2   1 hour + 8mn +/- 0.0625 */
#define STREAM_TIME16D   40 /* qqqqqqqqqqqqqqqq q15 [s] time difference +/- 15us */
#define STREAM_TIME32    41 /* ssssssssssssssssssssssssssssqqqq q28.4  [s] (8.5 years +/- 0.0625s) */ 
#define STREAM_TIME32D   42 /* ssssssssssssssssqqqqqqqqqqqqqqqq q17.15 [s] (36h, +/- 30us) time difference */   
#define STREAM_TIME64    43 /* ____10ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqq u32.26 [s] 140 Y +Q26 [s] */   
#define STREAM_TIME64MS  44 /* ____011000000000000000mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm u42 [ms] 140 years, 4b MSB=DTYPE */   
#define STREAM_TIME64ISO 45 /* ____0YY..YY..YY..YY..MM..MM..DD..DD..SS..SS.....offs..MM..MM..MM ISO8601 signed offset 2024-05-04T21:12:02+07:00  */   
#define STREAM_WGS84     46 /* <--LATITUDE 32B--><--LONGITUDE 32B-->  lat="52.518611" 0x4252130f   lon="13.376111" 0x4156048d - dual IEEE754 */   
#define STREAM_HEXBINARY 47 /* UTF-8 lower case hexadecimal byte stream */
#define STREAM_BASE64    48 /* RFC-2045 base64 for xsd:base64Binary XML data */
#define STREAM_STRING8   49 /* UTF-8 string of char terminated by 0 */
#define STREAM_STRING16  50 /* UTF-16 string of char terminated by 0 */

#define LAST_RAW_TYPE    64 /* coded on 6bits RAW_FMT0_LSB */
                            /*   |123456789|123456789|123456789|123456789|123456789|123456789|123 */



/* ========================== MINIFLOAT 8bits ======================================*/

// Time constants for algorithm
// MiniFloat 76543210
//           MMMEEEEE x= MMM(0..7) << EEEEE(0..31) =[0..15e9] +/-1
// just for information: OFP8_E4M3 SEEEEMMM x= (sign).(1 + M/8).(2<<(E-7)) =[-8..+8] +/-1e-6
#define MINIF(m,exp) ((uint8_t)((m)<<5 | (exp)))
#define MINIFLOAT2Q31(x) ((((x) & 0xE0)>>5) << ((x) & 0x1F))
#define MULTIPLIER_MSB 7     
#define MULTIPLIER_LSB 5
#define EXPONENT_MSB 4     
#define EXPONENT_LSB 0

/*============================ BIT-FIELDS MANIPULATIONS ============================*/
/*
 *  stream constants / Macros.
 */
 
// We define a preprocessor macro that will allow us to add padding
// to a data structure in a way that helps communicate our intent.
// Example : 
//   struct alignas(4) Pixel {
//       char R, G, B;
//       PADDING_BYTES(1);
//   };
#define CONCATENATE_(a, b) a##b
#define CONCATENATE(a, b) CONCATENATE_(a, b)
#define PADDING_BYTES(N) char CONCATENATE(PADDING_MACRO__, __COUNTER__)[N]

#define SHIFT_SIZE(base,shift) ((base) << ((shift) << 2));           

#define MIN(a, b) (((a) > (b))?(b):(a))
#define MAX(a, b) (((a) < (b))?(b):(a))
#define ABS(a) (((a)>0)? (a):-(a))

#define MAXINT32 0x7FFFFFFFL
#define MEMCPY(dst,src,n) {uint32_t i; for(i=0;i<n;i++){dst[i]=src[i];}}
#define MEMSWAP(dst,src,n) {uint32_t i, x; for(i=0;i<n;i++){x=dst[i];dst[i]=src[i];src[i]=x;}}
#define MEMSET(dst,c,n) {uint32_t i; uint8_t *pt8=(uint8_t *)dst; for(i=0;i<n;i++){pt8[i]=c;} }


/* bit-field manipulations */
#define CREATE_MASK(msb, lsb)               (((U(1LL) << ((msb) - (lsb) + U(1))) - U(1LL)) << (lsb))
#define MASK_BITS(arg, msb, lsb)            ((arg) & CREATE_MASK(msb, lsb))
#define EXTRACT_BITS(arg, msb, lsb)         (MASK_BITS(arg, msb, lsb) >> (lsb))
#define INSERT_BITS(arg, msb, lsb, value) \
    ((arg) = ((arg) & ~CREATE_MASK(msb, lsb)) | (((value) << (lsb)) & CREATE_MASK(msb, lsb)))
#define MASK_FIELD(arg, field)              MASK_BITS((arg), field##_MSB, field##_LSB)

#define EXTRACT_FIELD(arg, field)           U(EXTRACT_BITS((U(arg)), field##_MSB, field##_LSB))
#define RD(arg, field) U(EXTRACT_FIELD(arg, field))

#define INSERT_FIELD(arg, field, value)     INSERT_BITS((arg), field##_MSB, field##_LSB, value)
#define ST(arg, field, value) INSERT_FIELD((arg), field, U(value)) 

#define LOG2NBYTESWORD32 2 
#define NBYTESWORD32 (1<<LOG2NBYTESWORD32)

// replaced #define LINADDR_UNIT_BYTE   1
//      #define LINADDR_UNIT_W32    4
//      #define LINADDR_UNIT_EXTD  64

#define SET_BIT(arg, bit)   ((arg) |= (U(1) << U(bit)))
#define CLEAR_BIT(arg, bit) ((arg) = U(arg) & U(~(U(1) << U(bit))))
#define TEST_BIT(arg, bit)  (U(arg) & (U(1) << U(bit)))

#define FLOAT_TO_INT(x) ((x)>=0.0f?(int)((x)+0.5f):(int)((x)-0.5f))

/* DSP processing data types */
#define samp_t int16_t  /* default size of input samples = 16bits */
#define coef_t int16_t
#define accu_t int32_t  /* accumulator with software pre-shifter */
#define data_t int32_t+
#define iidx_t int32_t  /* index of the loop */

#define samp_f float
#define coef_f float
#define accu_f float
#define data_f float

#define f32_t float
#define s16_t int16_t
#define s32_t int32_t

#endif /* cSTREAM_COMMON_CONST_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
    

