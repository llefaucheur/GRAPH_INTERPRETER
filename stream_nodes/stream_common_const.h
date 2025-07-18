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
 */\


#ifdef __cplusplus
 extern "C" {
#endif

#ifndef cSTREAM_COMMON_CONST_H
#define cSTREAM_COMMON_CONST_H


/*  nbparam = 0 means any or "full set of parameters loaded from binary format" 
    W32LENGTH_LW4 == 0 means no parameter to read
*/
#define NODE_ALL_PARAM 0u 


#define NODE_TASKS_COMPLETED 0u
#define NODE_TASKS_NOT_COMPLETED 1u


#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */
/* ----------------------------------------------------------------------------------------------------------------
    STREAM_IO_DOMAIN (s)    => stream_format + stream_io_control

    enum stream_io_domain : list of stream "domains" categories, max 15 (DOMAIN_FMT1_LSB/ IO_DOMAIN_IOFMT0_LSB) @@@@@
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).
*/
#define IO_DOMAIN_GENERAL                  0u /* (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings*/
#define IO_DOMAIN_AUDIO_IN                 1u /* microphone, line-in, I2S, PDM RX */
#define IO_DOMAIN_AUDIO_OUT                2u /* line-out, earphone / speaker, PDM TX, I2S, */
#define IO_DOMAIN_GPIO                     3u /* generic digital IO, control of relay, timer ticks */
#define IO_DOMAIN_MOTION                   4u /* accelerometer, combined or not with pressure and gyroscope */
#define IO_DOMAIN_2D_IN                    5u /* camera sensor */
#define IO_DOMAIN_2D_OUT                   6u /* display, led matrix, */
#define IO_DOMAIN_ANALOG_IN                7u /* analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage */
#define IO_DOMAIN_ANALOG_OUT               8u /* D/A, position piezzo, PWM converter  */
#define IO_DOMAIN_USER_INTERFACE           9u /* button, slider, rotary button, LED, digits, display */
#define IO_DOMAIN_PLATFORM_6              10u                              
#define IO_DOMAIN_PLATFORM_5              11u                              
#define IO_DOMAIN_PLATFORM_4              12u                              
#define IO_DOMAIN_PLATFORM_3              13u
#define IO_DOMAIN_PLATFORM_2              14u /* platform-specific #2, decoded with callbacks */
#define IO_DOMAIN_PLATFORM_1              15u /* platform-specific #1, decoded with callbacks */
#define IO_DOMAIN_MAX_NB_DOMAINS          16u


/* ==========================================================================================

    IO_DOMAIN physical types and tuning : used to insert extra conversion nodes 
                                          during the graph compilation 

   ==========================================================================================
*/

/* IO_DOMAIN_GENERAL           : subtypes and tuning  SUBTYPE_FMT1  */
    #define STREAM_SUBT_GENERAL            0u
    #define STREAM_SUBT_GENERAL_COMP195X   1u /* compressed byte stream following RFC1950 / RFC1951 ("deflate") */
    #define STREAM_SUBT_GENERAL_DPCM       2u /* compressed byte stream */
    #define STREAM_SUBT_GENERAL_JSON       3u /* JSON */
    #define STREAM_SUBT_GENERAL_XFORMAT    4u /* SensorThings MultiDatastream extension */

/* IO_DOMAIN_AUDIO_IN          : subtypes and tuning  SUBTYPE_FMT1 */
    #define STREAM_SUBT_AUDIO_IN           0u  /* no subtype_units : integer/ADC format  */

/* IO_DOMAIN_AUDIO_OUT         : subtypes and tuning  SUBTYPE_FMT1 */
    #define STREAM_SUBT_AUDIO_OUT          0u  /* no subtype_units : integer/DAC format  */

    #define STREAM_SUBT_AUDIO_MPG         32u  /* compressed byte stream */

/* IO_DOMAIN_GPIO_IN           : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_GPIO_IN             0u  /* no subtype_units  */

/* IO_DOMAIN_GPIO_OUT          : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_GPIO_OUT            0u  /* no subtype_units  */

/* IO_DOMAIN_MOTION_IN         : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_MOTION_A            1u
   #define STREAM_SUBT_MOTION_G            2u
   #define STREAM_SUBT_MOTION_B            3u
   #define STREAM_SUBT_MOTION_AG           4u
   #define STREAM_SUBT_MOTION_AB           5u
   #define STREAM_SUBT_MOTION_GB           6u
   #define STREAM_SUBT_MOTION_AGB          7u

/* IO_DOMAIN_2D_IN             : subtypes and tuning  SUBTYPE_FMT1 */
/*                      raw data is uint8 or uint16 but the subtype tells how to extract the pixel data */
/* IO_DOMAIN_2D_OUT            : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_2D_YUV420P          1u /* Luminance, Blue projection, Red projection, 6 bytes per 4 pixels, reordered */
   #define STREAM_SUBT_2D_YUV422P          2u /* 8 bytes per 4 pixels, or 16bpp, Y0 Cb Y1 Cr (1 Cr & Cb sample per 2x1 Y samples) */
   #define STREAM_SUBT_2D_YUV444P          3u /* 12 bytes per 4 pixels, or 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
   #define STREAM_SUBT_2D_CYM24            4u /* cyan yellow magenta */
   #define STREAM_SUBT_2D_CYMK32           5u /* cyan yellow magenta black */
   #define STREAM_SUBT_2D_RGB8             6u /* RGB  3:3:2,  8bpp, (msb)2B 3G 3R(lsb) */
   #define STREAM_SUBT_2D_RGB16            7u /* RGB  5:6:5, 16bpp, (msb)5R 6G 5B(lsb) */
   #define STREAM_SUBT_2D_RGBA16           8u /* RGBA 4:4:4:4 32bpp (msb)4R */
   #define STREAM_SUBT_2D_RGB24            9u /* BBGGRR 24bpp (msb)8B */
   #define STREAM_SUBT_2D_RGBA32          10u /* BBGGRRAA 32bpp (msb)8B */
   #define STREAM_SUBT_2D_RGBA8888        11u /* AABBRRGG OpenGL/PNG format R=lsb A=MSB ("ABGR32" little endian) */
   #define STREAM_SUBT_2D_BW1B            12u /* Y, 1bpp, 0 is black, 1 is white */
   #define STREAM_SUBT_2D_GREY2B          13u /* Y, 2bpp, 0 is black, 3 is white, ordered from lsb to msb  */
   #define STREAM_SUBT_2D_GREY4B          14u /* Y, 4bpp, 0 is black, 15 is white, ordered from lsb to msb */
   #define STREAM_SUBT_2D_GREY8B          15u /* Grey 8b, 0 is black, 255 is white */
                                          
   #define STREAM_SUBT_2D_JPEG            64u /* legacy JPEG */
   #define STREAM_SUBT_2D_JP2             65u /* jpeg 2000  */
   #define STREAM_SUBT_2D_JXL             66u /* jpeg XL  */
   #define STREAM_SUBT_2D_GIF             67u /* Graphics Interchange Format */
   #define STREAM_SUBT_2D_PNG             68u /* Portable Network Graphics */
   #define STREAM_SUBT_2D_SVG             69u /* Scalable Vector Graphics */
   #define STREAM_SUBT_2D_TIFF            70u /* Tag Image File Format */
                                   

/* IO_DOMAIN_ANALOG_IN     : subtypes and tuning  SUBTYPE_FMT1 */
/* IO_DOMAIN_ANALOG_OUT : subtypes and tuning  SUBTYPE_FMT1 */
   #define STREAM_SUBT_ANA_ANY             0u /*        any                        */        
   #define STREAM_SUBT_ANA_METER           1u /* m         meter                   */
   #define STREAM_SUBT_ANA_KGRAM           2u /* kg        kilogram                */
   #define STREAM_SUBT_ANA_GRAM            3u /* g         gram                    */
   #define STREAM_SUBT_ANA_SECOND          4u /* s         second                  */
   #define STREAM_SUBT_ANA_AMPERE          5u /* A         ampere                  */
   #define STREAM_SUBT_ANA_KELVIB          6u /* K         kelvin                  */
   #define STREAM_SUBT_ANA_CANDELA         7u /* cd        candela                 */
   #define STREAM_SUBT_ANA_MOLE            8u /* mol       mole                    */
   #define STREAM_SUBT_ANA_HERTZ           9u /* Hz        hertz                   */
   #define STREAM_SUBT_ANA_RADIAN         10u /* rad       radian                  */
   #define STREAM_SUBT_ANA_STERADIAN      11u /* sr        steradian               */
   #define STREAM_SUBT_ANA_NEWTON         12u /* N         newton                  */
   #define STREAM_SUBT_ANA_PASCAL         13u /* Pa        pascal                  */
   #define STREAM_SUBT_ANA_JOULE          14u /* J         joule                   */
   #define STREAM_SUBT_ANA_WATT           15u /* W         watt                    */
   #define STREAM_SUBT_ANA_COULOMB        16u /* C         coulomb                 */
   #define STREAM_SUBT_ANA_VOLT           17u /* V         volt                    */
   #define STREAM_SUBT_ANA_FARAD          18u /* F         farad                   */
   #define STREAM_SUBT_ANA_OHM            19u /* Ohm       ohm                     */
   #define STREAM_SUBT_ANA_SIEMENS        20u /* S         siemens                 */
   #define STREAM_SUBT_ANA_WEBER          21u /* Wb        weber                   */
   #define STREAM_SUBT_ANA_TESLA          22u /* T         tesla                   */
   #define STREAM_SUBT_ANA_HENRY          23u /* H         henry                   */
   #define STREAM_SUBT_ANA_CELSIUSDEG     24u /* Cel       degrees Celsius         */
   #define STREAM_SUBT_ANA_LUMEN          25u /* lm        lumen                   */
   #define STREAM_SUBT_ANA_LUX            26u /* lx        lux                     */
   #define STREAM_SUBT_ANA_BQ             27u /* Bq        becquerel               */
   #define STREAM_SUBT_ANA_GRAY           28u /* Gy        gray                    */
   #define STREAM_SUBT_ANA_SIVERT         29u /* Sv        sievert                 */
   #define STREAM_SUBT_ANA_KATAL          30u /* kat       katal                   */
   #define STREAM_SUBT_ANA_METERSQUARE    31u /* m2        square meter (area)     */
   #define STREAM_SUBT_ANA_CUBICMETER     32u /* m3        cubic meter (volume)    */
   #define STREAM_SUBT_ANA_LITER          33u /* l         liter (volume)                               */
   #define STREAM_SUBT_ANA_M_PER_S        34u /* m/s       meter per second (velocity)                  */
   #define STREAM_SUBT_ANA_M_PER_S2       35u /* m/s2      meter per square second (acceleration)       */
   #define STREAM_SUBT_ANA_M3_PER_S       36u /* m3/s      cubic meter per second (flow rate)           */
   #define STREAM_SUBT_ANA_L_PER_S        37u /* l/s       liter per second (flow rate)                 */
   #define STREAM_SUBT_ANA_W_PER_M2       38u /* W/m2      watt per square meter (irradiance)           */
   #define STREAM_SUBT_ANA_CD_PER_M2      39u /* cd/m2     candela per square meter (luminance)         */
   #define STREAM_SUBT_ANA_BIT            40u /* bit       bit (information content)                    */
   #define STREAM_SUBT_ANA_BIT_PER_S      41u /* bit/s     bit per second (data rate)                   */
   #define STREAM_SUBT_ANA_LATITUDE       42u /* lat       degrees latitude[1]                          */
   #define STREAM_SUBT_ANA_LONGITUDE      43u /* lon       degrees longitude[1]                         */
   #define STREAM_SUBT_ANA_PH             44u /* pH        pH value (acidity; logarithmic quantity)     */
   #define STREAM_SUBT_ANA_DB             45u /* dB        decibel (logarithmic quantity)               */
   #define STREAM_SUBT_ANA_DBW            46u /* dBW       decibel relative to 1 W (power level)        */
   #define STREAM_SUBT_ANA_BSPL           47u /* Bspl      bel (sound pressure level; log quantity)     */
   #define STREAM_SUBT_ANA_COUNT          48u /* count     1 (counter value)                            */
   #define STREAM_SUBT_ANA_PER            49u /* /         1 (ratio e.g., value of a switch; )          */
   #define STREAM_SUBT_ANA_PERCENT        50u /* %         1 (ratio e.g., value of a switch; )          */
   #define STREAM_SUBT_ANA_PERCENTRH      51u /* %RH       Percentage (Relative Humidity)               */
   #define STREAM_SUBT_ANA_PERCENTEL      52u /* %EL       Percentage (remaining battery energy level)  */
   #define STREAM_SUBT_ANA_ENERGYLEVEL    53u /* EL        seconds (remaining battery energy level)     */
   #define STREAM_SUBT_ANA_1_PER_S        54u /* 1/s       1 per second (event rate)                    */
   #define STREAM_SUBT_ANA_1_PER_MIN      55u /* 1/min     1 per minute (event rate, "rpm")             */
   #define STREAM_SUBT_ANA_BEAT_PER_MIN   56u /* beat/min  1 per minute (heart rate in beats per minute)*/
   #define STREAM_SUBT_ANA_BEATS          57u /* beats     1 (Cumulative number of heart beats)         */
   #define STREAM_SUBT_ANA_SIEMPERMETER   58u /* S/m       Siemens per meter (conductivity)             */
   #define STREAM_SUBT_ANA_BYTE           59u /* B         Byte (information content)                   */
   #define STREAM_SUBT_ANA_VOLTAMPERE     60u /* VA        volt-ampere (Apparent Power)                 */
   #define STREAM_SUBT_ANA_VOLTAMPERESEC  61u /* VAs       volt-ampere second (Apparent Energy)         */
   #define STREAM_SUBT_ANA_VAREACTIVE     62u /* var       volt-ampere reactive (Reactive Power)        */
   #define STREAM_SUBT_ANA_VAREACTIVESEC  63u /* vars      volt-ampere-reactive second (Reactive Energy)*/
   #define STREAM_SUBT_ANA_JOULE_PER_M    64u /* J/m       joule per meter (Energy per distance)        */
   #define STREAM_SUBT_ANA_KG_PER_M3      65u /* kg/m3     kg/m3 (mass density, mass concentration)     */
   #define STREAM_SUBT_ANA_DEGREE         66u /* deg       degree (angle)                               */
   #define STREAM_SUBT_ANA_NTU            67u /* NTU       Nephelometric Turbidity Unit                 */

   // Secondary Unit (rfc8798)           Description          SenML Unit     Scale     Offset 
   #define STREAM_SUBT_ANA_MS             68u /* millisecond                  s      1/1000    0       1ms = 1s x [1/1000] */
   #define STREAM_SUBT_ANA_MIN            69u /* minute                       s      60        0        */
   #define STREAM_SUBT_ANA_H              70u /* hour                         s      3600      0        */
   #define STREAM_SUBT_ANA_MHZ            71u /* megahertz                    Hz     1000000   0        */
   #define STREAM_SUBT_ANA_KW             72u /* kilowatt                     W      1000      0        */
   #define STREAM_SUBT_ANA_KVA            73u /* kilovolt-ampere              VA     1000      0        */
   #define STREAM_SUBT_ANA_KVAR           74u /* kilovar                      var    1000      0        */
   #define STREAM_SUBT_ANA_AH             75u /* ampere-hour                  C      3600      0        */
   #define STREAM_SUBT_ANA_WH             76u /* watt-hour                    J      3600      0        */
   #define STREAM_SUBT_ANA_KWH            77u /* kilowatt-hour                J      3600000   0        */
   #define STREAM_SUBT_ANA_VARH           78u /* var-hour                     vars   3600      0        */
   #define STREAM_SUBT_ANA_KVARH          79u /* kilovar-hour                 vars   3600000   0        */
   #define STREAM_SUBT_ANA_KVAH           80u /* kilovolt-ampere-hour         VAs    3600000   0        */
   #define STREAM_SUBT_ANA_WH_PER_KM      81u /* watt-hour per kilometer      J/m    3.6       0        */
   #define STREAM_SUBT_ANA_KIB            82u /* kibibyte                     B      1024      0        */
   #define STREAM_SUBT_ANA_GB             83u /* gigabyte                     B      1e9       0        */
   #define STREAM_SUBT_ANA_MBIT_PER_S     84u /* megabit per second           bit/s  1000000   0        */
   #define STREAM_SUBT_ANA_B_PER_S        85u /* byteper second               bit/s  8         0        */
   #define STREAM_SUBT_ANA_MB_PER_S       86u /* megabyte per second          bit/s  8000000   0        */
   #define STREAM_SUBT_ANA_MV             87u /* millivolt                    V      1/1000    0        */
   #define STREAM_SUBT_ANA_MA             88u /* milliampere                  A      1/1000    0        */
   #define STREAM_SUBT_ANA_DBM            89u /* decibel rel. to 1 milliwatt  dBW    1       -30     0 dBm = -30 dBW       */
   #define STREAM_SUBT_ANA_UG_PER_M3      90u /* microgram per cubic meter    kg/m3  1e-9      0        */
   #define STREAM_SUBT_ANA_MM_PER_H       91u /* millimeter per hour          m/s    1/3600000 0        */
   #define STREAM_SUBT_ANA_M_PER_H        92u /* meterper hour                m/s    1/3600    0        */
   #define STREAM_SUBT_ANA_PPM            93u /* partsper million             /      1e-6      0        */
   #define STREAM_SUBT_ANA_PER_100        94u /* percent                      /      1/100     0        */
   #define STREAM_SUBT_ANA_PER_1000       95u /* permille                     /      1/1000    0        */
   #define STREAM_SUBT_ANA_HPA            96u /* hectopascal                  Pa     100       0        */
   #define STREAM_SUBT_ANA_MM             97u /* millimeter                   m      1/1000    0        */
   #define STREAM_SUBT_ANA_CM             98u /* centimeter                   m      1/100     0        */
   #define STREAM_SUBT_ANA_KM             99u /* kilometer                    m      1000      0        */
   #define STREAM_SUBT_ANA_KM_PER_H      100u /* kilometer per hour           m/s    1/3.6     0        */
                                                                                                
   #define STREAM_SUBT_ANA_GRAVITY       101u /* earth gravity                m/s2   9.81      0       1g = m/s2 x 9.81     */
   #define STREAM_SUBT_ANA_DPS           102u /* degrees per second           1/s    360       0     1dps = 1/s x 1/360     */   
   #define STREAM_SUBT_ANA_GAUSS         103u /* Gauss                        Tesla  10-4      0       1G = Tesla x 1/10000 */
   #define STREAM_SUBT_ANA_VRMS          104u /* Volt rms                     Volt   0.707     0    1Vrms = 1Volt (peak) x 0.707 */
   #define STREAM_SUBT_ANA_MVPGAUSS      105u /* Hall effect, mV/Gauss        millivolt 1      0    1mV/Gauss                    */

/* IO_DOMAIN_RTC               : subtypes and tuning  SUBTYPE_FMT1 */

/* IO_DOMAIN_USER_INTERFACE_IN    11 : subtypes and tuning  SUBTYPE_FMT1 */

/* IO_DOMAIN_USER_INTERFACE_OUT   12 : subtypes and tuning  SUBTYPE_FMT1 */




/*============================  ARCHITECTURES ============================================*/

enum stream_processor_architectures  /* architectures */
{
    ANY_ARCH    = 0,    /* source code without intrinsics neither inline assembly */
    ARMv6_M0    = 1,    /* Cortex-M0+ */
    ARMv6_M3    = 2,    /* Cortex-M3 */
    ARMv7E_M    = 3,    /* Cortex-M4/M7 */
    ARMv8_M     = 4,    /* Cortex-M33 */

    ARMv7_R     = 11,   /* Cortex-R4 */
    ARMv8_R     = 12,   /* Cortex-R52 */

    ARMv7_A     = 21,   /* Cortex-A7 */

    ARMv8_A32   = 31,   /* Cortex-A32 */
    ARMv8_A64   = 32,   /* Cortex-A55 */

    ARMv9_A     = 41,

    C166        = 81, 
    C51         = 82,  
    LAST_ARCH
};

enum stream_processor_sub_arch_fpu    
{
    ARCH_INTEGER    = 1,  /* non constraint : computations on integers */
    ARCH_FPU        = 2,  /* scalar fp32 */
    ARCH_DPFPU      = 3,  /* scalar fp32 + fp64 */
    ARCH_VECTOR     = 4,  /* NEON/MVE + fp16 + fp32 + dotProd8b */
    ARCH_ADVANCED_VECTOR = 5 /* ARCH_VECTOR + NN CA55 */
};

/* 
   ================================= stream_format  FORMATS =======================================
    
    Format 23+4_offsets for buffer BASE ADDRESS
    Frame SIZE and ring indexes are using 22bits linear (0..4MB)


        Word0: Frame size, interleaving scheme, arithmetics raw data type
        Word1: time-stamp, domain, nchan, physical unit (pixel format, IMU interleaving..)
        Word2: depends on IO Domain
        Word3: depends on IO Domain

*/

#define STREAM_FORMAT_SIZE_W32 4u     /*  digital, common part of the format  */
/*
*   _FMT0 word 0 : common to all domains : frame size
*   _FMT1 word 1 : common to all domains : time-stamp, nchan, raw format, interleaving, domain, size extension
*   _FMT2 word 2 : specific to domains : sampling rate
*   _FMT3 word 3 : specific to domains : hashing, channel mapping  ..
*       _FMT4 word 4 : specific to domains TBD
*       _FMT5 word 5 : specific to domains TBD
*/

/* for MISRA-2012 compliance to Rule 10.4 */
#define U8(x) ((uint8_t)(x)) 
#define S8(x) ((int8_t)(x)) 

//enum time_stamp_format_type {
////+enum frame_format_synchro {
#define SYNCHRONOUS 0u              /* tells the output buffer size is NOT changing */
#define ASYNCHRONOUS 1u             /* tells the output frame length is variable, input value "Size" tells the maximum value  
//                                       data format : optional time-stamp (stream_time_stamp_format_type)
//                                                    domain [2bits] and sub-domain [6bits rfc8428]
//                                                    payload : [nb samples] [samples]  */
#define NO_TIMESTAMP    0u
#define FRAME_COUNTER   1u          /* 32bits counter of data frames */
#define TIMESTAMP_ABS   2u          /* float64 absolute frame distance in [s] from Unix Epoch */
#define TIMESTAMP_REL   3u          /* float32 relative frame distance in [s] */


//enum hashing_type {
#define NO_HASHING 0u                /* cipher protocol under definition */
#define HASHING_ON 1u                /* the stream is ciphered (HMAC-SHA256 / stream_encipher XTEA)*/

//enum frame_format_type {
#define FMT_INTERLEAVED 0u          /* "arc_descriptor_interleaved" for example L/R audio or IMU stream..   */
#define FMT_DEINTERLEAVED_1PTR 1u   /* pointer to the first channel, next base address is frame size/nchan */

//enum direction_rxtx {
#define IODIRECTION_RX 0u          /* RX from the Graph pont of view */
#define IODIRECTION_TX 1u


/*          SIZE_EXT_FMT0 / COLLISION_ARC / NODE memory bank

    -8 bits-[--------24 bits ------]
    1_987654321_987654321_987654321_
    cccccccc________________________  collision byte in the WRITE word32
    R_______________________________  flag: relocatable memory segment
    ____OOOO________________________  long offset of the buffer base address
    ________EXT_____________________  extension 3bits shifter
    ___________s44443333222211110000  5 hex signed digits[1+20b] = FrameSize and Arc-R/W
            <--SIZE_EXT_FMT0_MSB--->  R/W arc indexe leaves one byte
        <---SIZE_EXT_OFF_FMT0_MSB-->  base addresses on 28bits 
    <-->                              tells the loader to copy a graph section in RAM
    
   max = +/- 0x000FFFFF << (EXT x 2) 

   EXT  Shift   Granul  range   
   0    0       1       1MB     
   1    2       4       4MB     
   2    4       16      16MB
   3    6       64      64MB
   4    8       256     256MB
   5    10      1K      1GB
   6    14      16K     16GB    (TBD)
   7    18      256K    256GB   (TBD)

   max = (+/-)0x000FFFFF << (2xEXT) = +/-1M x (2<<EXT) 
*/
#define SIZE_EXT_OFF_FMT0_MSB 27u /*       28 = offsets(4) + EXT(3) + sign(1) + size(20) */
#define  ADDR_OFFSET_FMT0_MSB 27u
#define  ADDR_OFFSET_FMT0_LSB 24u /*  4    offsets */
#define     SIZE_EXT_FMT0_MSB 23u /*       24 = EXT(3) + sign(1) + size(20) */
         
#define    EXTENSION_FMT0_MSB 23u /*    */
#define    EXTENSION_FMT0_LSB 21u /*  3 */
#define  SIGNED_SIZE_FMT0_MSB 20u /*       21 bits for sign(1) + size(20) */
#define    SIZE_SIGN_FMT0_MSB 20u /*    */
#define    SIZE_SIGN_FMT0_LSB 20u /*  1 */
#define         SIZE_FMT0_MSB 19u /*    */
#define         SIZE_FMT0_LSB  0u /* 20 */
#define  SIGNED_SIZE_FMT0_LSB  0u /*    */

#define     SIZE_EXT_FMT0_LSB  0u /*    */
#define SIZE_EXT_OFF_FMT0_LSB  0u /*    */

#define MAX_NB_MEMORY_OFFSET ((-1) + (1<<(ADDR_OFFSET_FMT0_MSB-ADDR_OFFSET_FMT0_LSB+1)))

#define PACK2LIN(R,LL,I)                                            \
    {   uint8_t *base;                                              \
        uint8_t extend;                                             \
        int32_t signed_base;                                        \
                                                                    \
        base = LL[RD(I,ADDR_OFFSET_FMT0)];                          \
        signed_base = RD(I, SIGNED_SIZE_FMT0);                      \
        signed_base = signed_base << (32-SIGNED_SIZE_FMT0_MSB);     \
        signed_base = signed_base >> (32-SIGNED_SIZE_FMT0_MSB);     \
        extend = (uint8_t)RD(I, EXTENSION_FMT0);                    \
        signed_base <<= (extend << 1);                              \
        R = (intptr_t)(&(base[signed_base]));                       \
    }


/*  WORD 0 - frame size --------------- */
#define FRAMESZ_FMT0   0u          
    /* A "frame" is the combination of several channels sampled at the same time            */
    /* frame size in bytes, data interleaved or not                                         */
    /* A value =0 means the size is any or defined by the IO AL.                            */
    /* in node manifests it gives the minimum input size (grain) before activating the node */
    /* For sensors delivering burst of data not isochronous, it gives the maximum           */
    /* framesize; same comment for the sampling rate.                                       */
    /* The frameSize is including the time-stamp field                                      */

    #define unused____FMT0_MSB  31u /*  8 reserved */
    #define unused____FMT0_LSB  24u 
    #define FRAMESIZE_FMT0_MSB  SIZE_EXT_FMT0_MSB
    #define FRAMESIZE_FMT0_LSB  SIZE_EXT_FMT0_LSB

/*- WORD 1 - domain, sub-types , size extension, time-stamp, raw format, interleaving, nchan  -------------*/
#define NCHANDOMAIN_FMT1   1
    #define unused____FMT1_MSB  31u /*     */  
    #define unused____FMT1_LSB  28u /* 4 reserved */
    #define   SUBTYPE_FMT1_MSB  27u /*     */  
    #define   SUBTYPE_FMT1_LSB  21u /* 7  sub-type for pixels and analog formats (STREAM_SUBT_ANA_)  */
    #define    DOMAIN_FMT1_MSB  20u 
    #define    DOMAIN_FMT1_LSB  17u /* 4  STREAM_IO_DOMAIN = DOMAIN_FMT1 */
    #define       RAW_FMT1_MSB  16u
    #define       RAW_FMT1_LSB  11u /* 6  arithmetics stream_raw_data 6bits (0..63)  */
    #define  TSTPSIZE_FMT1_MSB  10u 
    #define  TSTPSIZE_FMT1_LSB   9u /* 2  16/32/64/64TXT time-stamp time format */
    #define  TIMSTAMP_FMT1_MSB   8u 
    #define  TIMSTAMP_FMT1_LSB   6u /* 3  time_stamp_format_type for time-stamped streams for each interleaved frame */
    #define INTERLEAV_FMT1_MSB   5u       
    #define INTERLEAV_FMT1_LSB   5u /* 1  interleaving : frame_format_type */
    #define   NCHANM1_FMT1_MSB   4u 
    #define   NCHANM1_FMT1_LSB   0u /* 5  nb channels-1 [1..32] */

/*  WORD2 - sampling rate */
#define SAMPLINGRATE_FMT2   2
    /*--------------- WORD 2 -------------*/
    #define      FS1D_FMT2_MSB  31u /* 32 IEEE-754, 0 means "asynchronous" or "any" */
    #define      FS1D_FMT2_LSB   0u /*    [Hz]   */
    //#define  LEVEL_1D_FMT2_MSB   7u
    //#define  LEVEL_1D_FMT2_LSB   0u /* 8  Maximum level from SUBTYPE_FMT1-unit */


/*  WORD 3 for IO_DOMAIN_AUDIO_IN and IO_DOMAIN_AUDIO_OUT */
#define DOMAINSPECIFIC_FMT3   3
    #define unused____FMT3_MSB U(31) /* 8b   */
    #define unused____FMT3_LSB U(24) 
    #define AUDIO_MAP_FMT3_MSB U(23) /* 24 mapping of channels example of 7.1 format (8 channels): */
    #define AUDIO_MAP_FMT3_LSB U( 0) /*     FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..*/

    
    /*--------------- WORD 3 for IO_DOMAIN_MOTION */
    //enum imu_channel_format /* uint8_t : data interleaving possible combinations */
    typedef enum  
    {   aXg0m0=1, /* only accelerometer */
        a0gXm0=2, /* only gyroscope */
        a0g0mX=3, /* only magnetometer */
        aXgXm0=4, /* A + G */
        aXg0mX=5, /* A + M */
        a0gXmX=6, /* G + M */
        aXgXmX=7, /* A + G + M */
    } imu_channel_format;
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
    typedef enum 
    { 
        R2D_TBD=0, R2D_1_1=1, R2D_4_3=2, R2D_16_9=3, R2D_3_2=4 
    }   ratio_2d_fmt3 ;

    #define   unused_____FMT3_MSB U(31) /* 14 */
    #define   unused_____FMT3_LSB U(18)
    #define   I2D_BORDER_FMT3_MSB U(17) /*  2 pixel border 0,1,2,3   */
    #define   I2D_BORDER_FMT3_LSB U(16)
    #define I2D_VERTICAL_FMT3_MSB U(15) /*  1 set to 0 for horizontal, 1 for vertical */
    #define I2D_VERTICAL_FMT3_LSB U(15)
    #define     RATIO_2D_FMT2_MSB U(14) /*  3 ratio_2d_fmt3 */
    #define     RATIO_2D_FMT2_LSB U(12) 
    #define I2D_SMALLDIM_FMT3_MSB U(11) /* 12 smallest pixel dimension, the largest comes with a division with */
    #define I2D_SMALLDIM_FMT3_LSB U( 0) /*    largest dimension = (frame_size - time_stamp_size)/smallest_dimension 
                                              or  largest dimension = ratio x smallest_dimension  */
//
//
//    /*  WORD 4 IO domain specific */
//#define DOMAINSPECIFIC_FMT4   4
//    #define unused____FMT4_MSB U(31) 
//    #define unused____FMT4_LSB U( 0) 
//
//
//    /*  WORD 5 IO domain specific */
//#define DOMAINSPECIFIC_FMT5   5
//    #define unused____FMT5_MSB U(31) 
//    #define unused____FMT5_LSB U( 0) 

/*====================================================================================================================*/                          
/*
    commands 
        from the application to the graph scheduler         arm_graph_interpreter(command,  arm_stream_instance_t *S, uint8_t *, uint32_t);
        from the graph scheduler to the nanoApps            S->address_node (command, stream_handle_t instance, stream_xdmbuffer_t *, uint32_t *);
        from the Scripts to the IO configuration setting    arm_stream_services (command, uint8_t *, uint8_t *, uint8_t *, uint32_t)
*/

    /*  FROM APP TO SCHEDULER :     arm_graph_interpreter (STREAM_RESET, &stream_instance, 0, 0); 
        FROM SCHEDULER to NODE :     devxx_fyyyy (STREAM_RESET, &node_instance, &memreq, &status); 
            Command + nb arcs, preset 0..15, TAG 0..255
        -  (STREAM_RESET, ptr1, ptr2, ptr3); 
            ptr1 = instance pointer, memory banks
            ptr2 = stream_services function address, followed by all the arc format
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
                NODE updates the XDM size fields with :
                 rx arc . size = amount of data consumed
                 tx arc . size = amount of data produced
        -  (STREAM_STOP, ptr1, ptr2, ptr3); 
    */
    #define STREAM_RESET            1u  /* arm_graph_interpreter(STREAM_RESET, *instance, * memory_results) */
        #define STREAM_COLD_BOOT    0u  /* if (STREAM_COLD_BOOT == RD(command, COMMDEXT_CMD)) */
        #define STREAM_WARM_BOOT    1u  /* Reset + restore memory banks from retention */

    #define STREAM_SET_PARAMETER    2u  /* APP sets NODE parameters node instances are protected by multithread effects when 
                                          changing parmeters on the fly, used to exchange the unlock key */
            
    //#define STREAM_SET_IO_CONFIG STREAM_SET_PARAMETER 
    /*  @@@ TODO
            reconfigure the IO : p_io_function_ctrl(STREAM_SET_IO_CONFIG + (FIFO_ID<<NODE_TAG_CMD), 0, new_configuration_index) 

            io_power_mode     0             ; to set the device at boot time in stop / off (0)
                                            ; running mode(1) : digital conversion (BIAS always powered for analog peripherals )
                                            ; running mode(2) : digital conversion BIAS shut-down between conversions
                                            ; Sleep (3) Bias still powered but not digital conversions            
            */

    #define STREAM_READ_PARAMETER   3u   /* used from script */
    #define STREAM_RUN              4u   /* arm_graph_interpreter(STREAM_RUN, instance, *in_out) */
    #define STREAM_STOP             5u   /* arm_graph_interpreter(STREAM_STOP, instance, 0)  node calls free() if it used stdlib's malloc */
    #define STREAM_UPDATE_RELOCATABLE 6u /* update the nanoAppRT pointers to relocatable memory segments */

    #define STREAM_SET_BUFFER       7u   /* platform_IO(STREAM_SET_BUFFER, *data, size)  */
    #define STREAM_READ_DATA        8u   /* COMMAND_SSRV syscall read access to arc data */
    #define STREAM_WRITE_DATA       9u   /* COMMAND_SSRV syscall write access to arc data */

    #define STREAM_LIBRARY          10u  /* other functions of the node (IIR parameters compute, ..) */

    #define NOWAIT_OPTION_SSRV      0u   /* OPTION_SSRV  stall or not the COMMAND */
    #define   WAIT_OPTION_SSRV      1u


/*  FROM THE GRAPH SCHEDULER TO THE NANOAPPS   NODE_COMMANDS  */
    #define  POSITION_CMD_MSB U(31)       
    #define      NARC_CMD_MSB U(23)       
    #define      NARC_CMD_LSB U(20) /* 4 number of arcs */
    #define    PRESET_CMD_MSB U(19)       
    #define    PRESET_CMD_LSB U(16) /* 4  #16 presets */
    #define  POSITION_CMD_LSB U(16) /* 16 node position (script calls set_param to a node at this position) */

    #define  NODE_TAG_CMD_MSB U(15) /*    parameter, function selection / debug arc index / .. */      
    #define  NODE_TAG_CMD_LSB U( 8) /* 8  instanceID for the trace / FIFO_ID for status checks */
    #define   UNUSED1_CMD_MSB U( 7)       
    #define   UNUSED1_CMD_LSB U( 5) /* 3 _______ */
    #define  COMMDEXT_CMD_MSB U( 4)       
    #define  COMMDEXT_CMD_LSB U( 4) /* 1 command option (RESET + warmboot, (SET_PARAM + wait)  */
    #define   COMMAND_CMD_MSB U( 3)       
    #define   COMMAND_CMD_LSB U( 0) /* 4 command */

    #define PACK_COMMAND(SWCTAG,PRESET,NARC,EXT,CMD) (((SWCTAG)<<NODE_TAG_CMD_LSB)|((PRESET)<<PRESET_CMD_LSB)|((NARC)<<NARC_CMD_LSB)|((EXT)<<COMMDEXT_CMD_LSB)|(CMD))


/*================================================================================================================*/    
/*
    "SERV_command"  from the nodes, to "arm_stream_services"

    void arm_stream_services (uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n)

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

#define NOCOMMAND_SSRV 0u
#define NOOPTION_SSRV 0u
#define NOTAG_SSRV 0u

/* -------------------------------------------------------
    arm_stream_services                 CottfFFg 
            
            Command                     C
            Option                       o
            TAG                           tt
            sub Function                    f
            Function                         FF
            Service Group                      g
   -------------------------------------------------------
*/
#define  COMMAND_SSRV_MSB U(31)       
#define  COMMAND_SSRV_LSB U(28) /* 4   C   set/init/run w/wo wait completion, in case of coprocessor usage */
#define   OPTION_SSRV_MSB U(27)       
#define   OPTION_SSRV_LSB U(24) /* 4   o   compute accuracy, in-place processing, frame size .. */
#define      TAG_SSRV_MSB U(23)       
#define      TAG_SSRV_LSB U(16) /* 8   tt  parameter of the function  */
#define FUNCTION_SSRV_MSB U(15)       
#define SUBFUNCT_SSRV_MSB U(15)       
#define SUBFUNCT_SSRV_LSB U(12) /* 4   f   16 sub functions or parameters on MSB */
#define FUNCTION_SSRV_LSB U( 4) /* 12  fFF functions/group x 16 subfunct, total = 4K */
#define    GROUP_SSRV_MSB U( 3)       
#define    GROUP_SSRV_LSB U( 0) /* 4   g   16 service groups */


#define PACK_SERVICE(COMMAND,OPTION,TAG,FUNC,GROUP) \
     ((COMMAND)<<COMMAND_SSRV_LSB)  | \
   ( ((OPTION) <<OPTION_SSRV_LSB)   | \
   ( ((TAG)    <<TAG_SSRV_LSB)      | \
   ( ((FUNC)   <<FUNCTION_SSRV_LSB) | \
     ((GROUP)  <<GROUP_SSRV_LSB)     )))

/* mask for node_mask_library, is there a need for a registered return address (Y/N)  */
#define SERV_GROUP_INTERNAL     1u  /* 1   N internal : Semaphores, DMA, Clocks */
#define SERV_GROUP_SCRIPT       2u  /* 2   N script : Node parameters  */
#define SERV_GROUP_CONVERSION   3u  /* 4   N Compute : raw conversions */
#define SERV_GROUP_STDLIB       4u  /* 8   Y Compute : malloc, string */
#define SERV_GROUP_MATH         5u  /* 16  N math.h */
#define SERV_GROUP_DSP_ML       6u  /* 32  N cmsis-dsp */
#define SERV_GROUP_DEEPL        7u  /* 64  N cmsis-nn */
#define SERV_GROUP_MM_AUDIO     8u  /* 128 Y speech/audio processing */
#define SERV_GROUP_MM_IMAGE     9u  /* 256 Y image processing */



/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 2/SERV_SCRIPT ------------------------------------------------ */
/* --------------------------------------------------------------------------- */

/*     List of                      FUNCTION_SSRV  (12bits) */
#define SERV_SCRIPT_RESET               0x001
#define SERV_SCRIPT_NODE                0x002
#define SERV_SCRIPT_SCRIPT              0x003  /* node control from scripts */
                                       
#define SERV_SCRIPT_FORMAT_UPDATE       0x004  /* change stream format from NODE media decoder, script applying change of 
                                                use-case (IO_format, vocoder frame-size..): sampling, nb of channel, 2D frame size */
    /* SYSCALL_NODE (CMD=set/read_param + TAG, Node offset, Pointer, Nbytes) */
    #define SYSCALL_FUNCTION_SSRV_NODE      1u       

/*      #define STREAM_RESET                1
        #define STREAM_SET_PARAMETER        2
        #define STREAM_READ_PARAMETER       3
        #define STREAM_RUN                  4
        #define STREAM_STOP                 5
        #define STREAM_UPDATE_RELOCATABLE   6
        #define STREAM_SET_BUFFER           7
        #define STREAM_READ_DATA            8
        #define STREAM_WRITE_DATA           9


//#define SERV_SCRIPT_FORMAT_UPDATE_FS 3u /* NODE information for a change of stream format, sampling, nb of channel */
//#define SERV_SCRIPT_FORMAT_UPDATE_NCHAN 4u     /* raw data sample, mapping of channels, (web radio use-case) */
//#define SERV_SCRIPT_FORMAT_UPDATE_RAW 5u

#define SERV_SCRIPT_SECURE_ADDRESS      0x106   /* this call is made from the secured address */
#define SERV_SCRIPT_AUDIO_ERROR         0x107   /* PLC applied, Bad frame (no header, no synchro, bad data format), bad parameter */
#define SERV_SCRIPT_DEBUG_TRACE         0x108   /* 1b, 1B, 16char */
#define SERV_SCRIPT_DEBUG_TRACE_STAMP   0x109   
#define SERV_SCRIPT_AVAILABLE           0x10A   
#define SERV_SCRIPT_SETARCDESC          0x10B   /* buffers holding MP3 songs.. rewind from script, 
                                                   switch a NN model to another, change a parameter-set using arcs */


//SERV_SCRIPT_DEBUG_TRACE, SERV_SCRIPT_DEBUG_TRACE_1B, SERV_SCRIPT_DEBUG_TRACE_DIGIT, 
// 
//SERV_SCRIPT_DEBUG_TRACE_STAMPS, SERV_SCRIPT_DEBUG_TRACE_STRING,
// 
//STREAM_SAVE_HOT_PARAMETER, 

//STREAM_LOW_POWER,     /* interface to low-power platform settings, "wake-me in 24h with deep-sleep in-between" */
//                          " I have nothing to do most probably for the next 100ms, do what is necessary "

//STREAM_PROC_ARCH,     /* returns the processor architecture details, used before executing specific assembly codes */


/*

        
        script language : SYSCALL ARC  r-CMD r-ARC r-ADDR r-N
                                  FUNC   SET arcID addr   n 

        SYSCALL 1 (NODE = FUNCTION_SSRV)
            r-node      node address
            r-cmd       reset id + set_param
            r-addr      data address
            r-n         nbbytes
                                  FUNC   SET  Node addr   n 
        script interpreter :
            (*al_func)(
                PACK_SERVICE(STREAM_READ_DATA, NOWAIT_OPTION_SSRV, PARAM_TAG, 
                    SYSCALL_FUNCTION_SSRV_NODE, SERV_GROUP_SCRIPT), 
                offset to the node from graph_computer_header.h (ex: #define arm_stream_filter_0  0x15)
                address of the data move,
                unused
                number of bytes
             );
    */

    /* SYSCALL_ARC (CMD read/set_param_descriptor, read/write_data, arc ID, Pointer, Nbytes) 
        descriptor = filling/empty state, locked flag, R/W index, buffer size
            debug information, time-stamp of the last access
            format update ? FS, frame-size ?
        read_data option : without updating the read pointer

        script language : SYSCALL ARC r-CMD r-ARC r-ADDR r-N
        (*al_func)(
            PACK_SERVICE(STREAM_READ_DATA, NOWAIT_OPTION_SSRV, PARAM_TAG, 
                SYSCALL_FUNCTION_SSRV_ARC, SERV_GROUP_SCRIPT), 
            arcID
            address of the data to be read,
            unused
            number of bytes
            );

    */
    #define SYSCALL_FUNCTION_SSRV_ARC       2u

    /* SYSCALL_CALLBACK (CMD , r1, r2, r3, r4) 
            depends on the application. Address the case of Arduino libraries
            example specific : IP address, password to share, Ping IP to blink the LED, read RSSI, read IP@
    */
    #define SYSCALL_FUNCTION_SSRV_CALLBACK  3u

    /* SYSCALL_IO       CMD(=set/read_param) FWIOIDX_IOFMT0 to select another A/D or GPIO (when the graph is in RAM)
                        CMD(=read/write_data) domain-specific settings
                        CMD(=stop/reset/run) stop and initiate data transfer
    */
    #define SYSCALL_FUNCTION_SSRV_IO        4u

    /* SYSCALL_DEBUG    receive remote commands, send string of debug data
    */
    #define SYSCALL_FUNCTION_SSRV_DEBUG     5u

    /* SYSCALL_COMPUTE  call compute library
    *                   Compute Median/absMax from data in a circular buffer of the Heap + mean/STD

        Math library: Commonly used single-precision floating-point functions:
            Basic operations: ceilf, fabsf, floorf, fmaxf, fminf, fmodf, roundf, lroundf, remainderf
            Exponential/power functions: expf, log2f, powf, sqrtf
            Trigonometric/hyperbolic functions: sinf, cosf, tanf, asinf, acosf, atan2f, tanhf 
    */
    #define SYSCALL_FUNCTION_SSRV_COMPUTE   6u
                                          
    /* SYSCALL_TIME_IDLE    Timer setting, read the time in different formats (16/32/64/delta)
                        default idle mode commands
                        compute time elapsed from today, from a reference, from reset, UTC/local time
    */
    #define SYSCALL_FUNCTION_SSRV_TIME_IDLE 7u

    /* SYSCALL_LOWLEVEL     TBD : low-level I2C/peripheral/memory access
    *                   Call a relocatable native-binary section in the Param area
    */
    #define SYSCALL_FUNCTION_SSRV_LOWLEVEL  8u

    // for scripts/Nodes: fast data moves
    #define SERV_SCRIPT_DMA_SET             9u      /* set src/dst/length */
    #define SERV_SCRIPT_DMA_START          10u
    #define SERV_SCRIPT_DMA_STOP           11u
    #define SERV_SCRIPT_DMA_CHECK          12u

/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 3/SERV_CONVERSION -------------------------------------------- */
/* --------------------------------------------------------------------------- */
    #define SERV_CONVERSION_INT16_FP32 1


/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 4/SERV_STDLIB ------------------------------------------------ */
/* --------------------------------------------------------------------------- */

    /* stdlib.h */
    /* string.h */
    //STREAM_MEMSET, STREAM_STRCHR, STREAM_STRLEN,
    //STREAM_STRNCAT, STREAM_STRNCMP, STREAM_STRNCPY, STREAM_STRSTR, STREAM_STRTOK,
    #define STREAM_ATOF      3u
    #define STREAM_ATOI      4u
    #define STREAM_MEMSET    5u
    #define STREAM_STRCHR    6u
    #define STREAM_STRLEN    7u
    #define STREAM_STRNCAT   8u
    #define STREAM_STRNCMP   9u
    #define STREAM_STRNCPY  10u
    #define STREAM_STRSTR   11u
    #define STREAM_STRTOK   12u
    //STREAM_ATOF, STREAM_ATOI
    #define STREAM_FREE     13u
    #define STREAM_MALLOC   14u


/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 5/SERV_MATH -------------------------------------------------- */
/* --------------------------------------------------------------------------- */

    /* minimum service : tables of 64 data RAND, SRAND */
    #define STREAM_RAND     1 /* (STREAM_RAND + OPTION_SSRV(seed), *ptr1, 0, 0, n) */
    #define STREAM_SRAND    2
    #define SERV_TABLE_SIN      
    #define SERV_TABLE_TAN      
    #define SERV_TABLE_ATAN      
    #define SERV_TABLE_SQRT      
    #define SERV_TABLE_LOG


    /* returns a code corresponding to the processor architecture and its FPU options */
#define SERV_CHECK_ARCHITECTURE 2
    /* time.h */
    //STREAM_ASCTIMECLOCK, STREAM_DIFFTIME, STREAM_SYS_CLOCK (ms since reset), STREAM_TIME (linux seconds)
    //STREAM_READ_TIME (high-resolution timer), STREAM_READ_TIME_FROM_START, 
    //STREAM_TIME_DIFFERENCE, STREAM_TIME_CONVERSION,  
    // 
    //STREAM_TEAM

    /* From Android CHRE  https://source.android.com/docs/core/interaction/contexthub
    String/array utilities: memcmp, memcpy, memmove, memset, strlen
    Math library: Commonly used single-precision floating-point functions:
    Basic operations: ceilf, fabsf, floorf, fmaxf, fminf, fmodf, roundf, lroundf, remainderf
    Exponential/power functions: expf, log2f, powf, sqrtf
    Trigonometric/hyperbolic functions: sinf, cosf, tanf, asinf, acosf, atan2f, tanhf
    */
    #define SERV_MATH_SQRT_Q15       15
    #define SERV_MATH_SQRT_F32       16
    #define SERV_MATH_LOG_Q15        17
    #define SERV_MATH_LOG_F32        18

    #define SERV_MATH_SINE_Q15       19
    #define SERV_MATH_SINE_F32       20
    #define SERV_MATH_COS_Q15        21
    #define SERV_MATH_COS_F32        22
    #define SERV_MATH_ATAN2_Q15      23
    #define SERV_MATH_ATAN2_F32      24
               
    #define SERV_MATH_SORT           3 


/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 6/SERV_DSP_ML ------------------------------------------------ */
/* --------------------------------------------------------------------------- */

    /* list from ETAS "Embedded AI Coder" :
    * Batchnorm, Convolutions, Depthwise Convolutions, LSTM, Fully Connected, Elementwise Add, Sub, Mul, 
    Softmax, Relu, Leaky Relu, Logistic, Padding, StridedSlice, Tanh, MaxPooling, AveragePooling and 
    TransposeConv. It supports the data types int8 and float32.
    */
    /* minimum service : IIRQ15/FP32, DFTQ15/FP32 */
            /* FUNCTION_SSRV */
    #define SERV_DSP_CHECK_COPROCESSOR  1u   /* check for services() */
    #define SERV_DSP_CHECK_END_COMP     2u   /* check completion for the caller */
    #define SERV_DSP_DFT_Q15            9u   /* DFT/Goertzel windowing, module, dB */
    #define SERV_DSP_DFT_F32            10u
    #define SERV_DSP_CASCADE_DF1_Q15    3u   /* IIR filters, use SERV_CHECK_COPROCESSOR */
    #define SERV_DSP_CASCADE_DF1_F32    4u         

            /* COMMAND_SSRV */
    #define SERV_DSP_RUN                0u   /* run = default */
    #define SERV_DSP_INIT               1u   /* */
    #define SERV_DSP_WINDOW             2u    
    #define SERV_DSP_WINDOW_DB          3u    

            /* OPTION_SSRV */
    #define SERV_WAIT_COMP          0u   /* tell to return when processing completed (default) */
    #define SERV_RETASAP            1u   /* return even when init/computation is not finished */

            /* FFT with tables rebuilded */
    #define SERV_DSP_rFFT_Q15           5u   /* RFFT windowing, module, dB , use SERV_CHECK_COPROCESSOR */
    #define SERV_DSP_rFFT_F32           6u
                                   
    #define SERV_DSP_cFFT_Q15           7u   /* cFFT windowing, module, dB */
    #define SERV_DSP_cFFT_F32           8u
                                       



/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 7/SERV_DEEPL ------------------------------------------------ */
/* --------------------------------------------------------------------------- */

        /* COMMAND_SSRV */

        /* OPTION_SSRV */

        /* FUNCTION_SSRV */
    #define STREAM_FC                   /* fully connected layer Mat x Vec */
    #define STREAM_CNN                  /* convolutional NN : 3x3 5x5 fixed-weights */


/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 8/SERV_MM_AUDIO ------------------------------------------------ */
/* --------------------------------------------------------------------------- */

        /* COMMAND_SSRV */

        /* OPTION_SSRV */

        /* FUNCTION_SSRV */

/* --------------------------------------------------------------------------- */
/* GROUP_SSRV = 9/SERV_MM_IMAGE ------------------------------------------------ */
/* --------------------------------------------------------------------------- */

        /* COMMAND_SSRV */

        /* OPTION_SSRV */

        /* FUNCTION_SSRV */
            // SOBEL

/*
* system subroutines : 
* - IO settings : 
* - Get Time, in different formats, and conversion, extract time-stamps
* - Get Peripheral data : RSSI, MAC/IP address
* - Low-level : I2C string of commands, GPIO, physical address to perpherals
*/
#define PLATFORM_DEEPSLEEP_ENABLED 20u   /* deep-sleep activation is possible when returning from arm_graph_interpreter(STREAM_RUN..) */
#define PLATFORM_TIME_SET          21u
#define PLATFORM_RTC_SET           22u
#define PLATFORM_TIME_READ         23u
#define PLATFORM_HW_WORD_READ      24u  
#define PLATFORM_HW_WORD_WRITE     25u  
#define PLATFORM_HW_BYTE_READ      26u  
#define PLATFORM_HW_BYTE_WRITE     27u  

//enum error_codes 
#define ERROR_MEMORY_ALLOCATION     1u


/*
    STREAM SERVICES
*/

#define  UNUSED_SRV_MSB  31u
#define  UNUSED_SRV_LSB  16u /* 16 reserved */
#define    INST_SRV_MSB  15u       
#define    INST_SRV_LSB  12u /* 4  instance */
#define   GROUP_SRV_MSB  11u       
#define   GROUP_SRV_LSB   8u /* 4  command family groups under compilation options (DSP, Codec, Stdlib, ..) */
#define COMMAND_SRV_MSB   7u       
#define COMMAND_SRV_LSB   0u /* 8  256 service IDs */


/*
    Up to 16 family of processing extensions "SERVICE_COMMAND_GROUP"
    EXTDSPML EXTMATH EXTSTDLIB
*/

#define EXT_SERVICE_MATH   1u
#define EXT_SERVICE_DSPML  2u
#define EXT_SERVICE_STDLIB 3u



/*================================ STREAM ARITHMETICS DATA/TYPE ====================================================*/
/* types fit in 6bits, arrays start with 0, stream_bitsize_of_raw() is identical */


#define STREAM_DATA_ARRAY 0u /* stream_array : [0NNNTT00] 0, type, nb */
#define STREAM_S1         1u /* S, one signed bit, "0" = +1 */                           /* one bit per data */
#define STREAM_U1         2u /* one bit unsigned, boolean */
#define STREAM_S2         3u /* SX  */                                                   /* two bits per data */
#define STREAM_U2         4u /* XX  */
#define STREAM_Q1         5u /* Sx ~stream_s2 with saturation management*/
#define STREAM_S4         6u /* Sxxx  */                                                 /* four bits per data */
#define STREAM_U4         7u /* xxxx  */
#define STREAM_Q3         8u /* Sxxx  */
#define STREAM_FP4_E2M1   9u /* Seem  micro-float [8 .. 64] */
#define STREAM_FP4_E3M0  10u /* Seee  [8 .. 512] */
#define STREAM_S8        11u /* Sxxxxxxx  */                                             /* eight bits per data */
#define STREAM_U8        12u /* xxxxxxxx  ASCII char UTF-8, numbers.. */
#define STREAM_Q7        13u /* Sxxxxxxx  arithmetic saturation */
#define STREAM_CHAR      14u /* xxxxxxxx  */
#define STREAM_FP8_E4M3  15u /* Seeeemmm  NV tiny-float  +/-[0, 0.015 .. 240]  */ 
#define STREAM_FP8_E5M2  16u /* Seeeeemm  IEEE-754       +/-[0, 0.015 .. 15.5] */
#define STREAM_S16       17u /* Sxxxxxxx.xxxxxxxx  */                                    /* 2 bytes per data */
#define STREAM_U16       18u /* xxxxxxxx.xxxxxxxx  Numbers, UTF-16 characters */
#define STREAM_Q15       19u /* Sxxxxxxx.xxxxxxxx  arithmetic saturation */
#define STREAM_FP16      20u /* Seeeeemm.mmmmmmmm  half-precision float */
#define STREAM_BF16      21u /* Seeeeeeee.mmmmmmm  bfloat truncated FP32 = BFP16 = S E8 M7 */
#define STREAM_Q23       22u /* Sxxxxxxx.xxxxxxxx.xxxxxxxx  24bits */                    /* 3 bytes per data */ 
#define STREAM_S9_23     23u /* SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxxx  */                  /* 4 bytes per data, s8_24 */
#define STREAM_S32       24u /* one long word  */
#define STREAM_U32       25u /* xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx UTF-32, .. */
#define STREAM_Q31       26u /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx  */
#define STREAM_FP32      27u /* Seeeeeee.emmmmmmm.mmmmmmmm.mmmmmmmm  FP32 = S E8 M23 */             
#define STREAM_CQ15      28u /* Sxxxxxxx.xxxxxxxx Sxxxxxxx.xxxxxxxx (I Q) */             
#define STREAM_CFP16     29u /* Seeeeemm.mmmmmmmm Seeeeemm.mmmmmmmm (I Q) */             
#define STREAM_S64       30u /* long long */                                             /* 8 bytes per data */
#define STREAM_U64       31u /* unsigned  64 bits */
#define STREAM_Q63       32u /* Sxxxxxxx.xxxxxx ....... xxxxx.xxxxxxxx  */
#define STREAM_CQ31      33u /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxx..*/
#define STREAM_FP64      34u /* Seeeeeee.eeeemmmm.mmmmmmm ... double = S E11 M52 */
#define STREAM_CFP32     35u /* Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm Seee.. (I Q)  */
#define STREAM_FP128     36u /* Seeeeeee.eeeeeeee.mmmmmmm ... quadruple S E15 M112 */    /* 16 bytes per data */
#define STREAM_CFP64     37u /* fp64 fp64 (I Q)  */
#define STREAM_FP256     38u /* Seeeeeee.eeeeeeee.eeeeemm ... octuple  S E19 M236 */     /* 32 bytes per data */
#define STREAM_WGS84     39u /* <--LATITUDE 32B--><--LONGITUDE 32B-->  lat="52.518611" 0x4252130f   lon="13.376111" 0x4156048d - dual IEEE754 */   
#define STREAM_HEXBINARY 40u /* UTF-8 lower case hexadecimal byte stream */
#define STREAM_BASE64    41u /* RFC-2045 base64 for xsd:base64Binary XML data */
#define STREAM_STRING8   42u /* UTF-8 string of char terminated by 0 */
#define STREAM_STRING16  43u /* UTF-16 string of char terminated by 0 */

#define LAST_RAW_TYPE    64u /* coded on 6bits RAW_FMT0_LSB */

/* ========================== MINIFLOAT 8bits ======================================*/

// Time constants for algorithm
// MiniFloat 76543210
//           MMMEEEEE x= MMM(0..7) << EEEEE(0..31) = [0..15e9] +/-1
#define MINIF(m,exp) ((uint8_t)((m)<<5 | (exp)))
#define MINIFLOAT2Q31(x) ((((x) & 0xE0)>>5) << ((x) & 0x1F))
#define MULTIPLIER_MSB 7u     
#define MULTIPLIER_LSB 5u
#define EXPONENT_MSB 4u     
#define EXPONENT_LSB 0u

// just for information: OFP8_E4M3 SEEEEMMM x= (sign).(1 + M/8).(2<<(E-7)) =[+/- 240] +/- 0.015625 (2^-6)
// https://en.wikipedia.org/wiki/Minifloat
// … 000	… 001	… 010	… 011	… 100	… 101	… 110	… 111
// 0 0000 …	0	0.001953125	0.00390625	0.005859375	0.0078125	0.009765625	0.01171875	0.013671875
// 0 0001 …	0.015625	0.017578125	0.01953125	0.021484375	0.0234375	0.025390625	0.02734375	0.029296875
// 0 0010 …	0.03125	0.03515625	0.0390625	0.04296875	0.046875	0.05078125	0.0546875	0.05859375
// 0 0011 …	0.0625	0.0703125	0.078125	0.0859375	0.09375	0.1015625	0.109375	0.1171875
// 0 0100 …	0.125	0.140625	0.15625	0.171875	0.1875	0.203125	0.21875	0.234375
// 0 0101 …	0.25	0.28125	0.3125	0.34375	0.375	0.40625	0.4375	0.46875
// 0 0110 …	0.5	0.5625	0.625	0.6875	0.75	0.8125	0.875	0.9375
// 0 0111 …	1	1.125	1.25	1.375	1.5	1.625	1.75	1.875
// 0 1000 …	2	2.25	2.5	2.75	3	3.25	3.5	3.75
// 0 1001 …	4	4.5	5	5.5	6	6.5	7	7.5
// 0 1010 …	8	9	10	11	12	13	14	15
// 0 1011 …	16	18	20	22	24	26	28	30
// 0 1100 …	32	36	40	44	48	52	56	60
// 0 1101 …	64	72	80	88	96	104	112	120
// 0 1110 …	128	144	160	176	192	208	224	240
// 0 1111 …	Inf	NaN	NaN	NaN	NaN	NaN	NaN	NaN
// 1 0000 …	?0	?0.001953125	?0.00390625	?0.005859375	?0.0078125	?0.009765625	?0.01171875	?0.013671875
// 1 0001 …	?0.015625	?0.017578125	?0.01953125	?0.021484375	?0.0234375	?0.025390625	?0.02734375	?0.029296875
// 1 0010 …	?0.03125	?0.03515625	?0.0390625	?0.04296875	?0.046875	?0.05078125	?0.0546875	?0.05859375
// 1 0011 …	?0.0625	?0.0703125	?0.078125	?0.0859375	?0.09375	?0.1015625	?0.109375	?0.1171875
// 1 0100 …	?0.125	?0.140625	?0.15625	?0.171875	?0.1875	?0.203125	?0.21875	?0.234375
// 1 0101 …	?0.25	?0.28125	?0.3125	?0.34375	?0.375	?0.40625	?0.4375	?0.46875
// 1 0110 …	?0.5	?0.5625	?0.625	?0.6875	?0.75	?0.8125	?0.875	?0.9375
// 1 0111 …	?1	?1.125	?1.25	?1.375	?1.5	?1.625	?1.75	?1.875
// 1 1000 …	?2	?2.25	?2.5	?2.75	?3	?3.25	?3.5	?3.75
// 1 1001 …	?4	?4.5	?5	?5.5	?6	?6.5	?7	?7.5
// 1 1010 …	?8	?9	?10	?11	?12	?13	?14	?15
// 1 1011 …	?16	?18	?20	?22	?24	?26	?28	?30
// 1 1100 …	?32	?36	?40	?44	?48	?52	?56	?60
// 1 1101 …	?64	?72	?80	?88	?96	?104	?112	?120
// 1 1110 …	?128	?144	?160	?176	?192	?208	?224	?240
// 1 1111 …	?Inf	NaN	NaN	NaN	NaN	NaN	NaN	NaN

                       /* |123456789|123456789|123456789|123456789|123456789|123456789|123 */
// STREAM_TIME16    39 /* ssssssssssssqqqq q14.2   1 hour + 8mn +/- 0.0625 */
// STREAM_TIME16D   40 /* qqqqqqqqqqqqqqqq q15 [s] time difference +/- 15us */
// STREAM_TIME32    41 /* ssssssssssssssssssssssssssssqqqq q28.4  [s] (8.5 years +/- 0.0625s) */ 
// STREAM_TIME32D   42 /* ssssssssssssssssqqqqqqqqqqqqqqqq q17.15 [s] (36h, +/- 30us) time difference */   
// STREAM_TIME64    43 /* 0000ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s] 140 Y +Q28 [s] */   
// STREAM_TIME64MS  44 /* 0010000000000000000000mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm u42 [ms] 140 years */   
// 
// STREAM_TIME64ISO 45 /* 010..YY..YY..YY..YY..MM..MM..DD..DD..SS..SS.....offs..MM..MM..MM ISO8601 signed offset 2024-05-04T21:12:02+07:00  */   
// *  Local time in BINARY bit-fields : years/millisecond, WWW=day of the week 
// *  (0=Sunday, 1=Monday..)
// *      COVESA allowed formats : ['YYYY_MM_DD', 'DD_MM_YYYY', 'MM_DD_YYYY', 'YY_MM_DD', 'DD_MM_YY', 'MM_DD_YY']
// *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
// *  _________________________.YY.YY.YY.YY.MMM.DDDD.SSSSS.MM.MM.MM.WW


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
//#define CONCATENATE_(a, b) a##b
//#define CONCATENATE(a, b) CONCATENATE_(a, b)
//#define PADDING_BYTES(N) char CONCATENATE(PADDING_MACRO__, __COUNTER__)[N]

#define SHIFT_SIZE(base,shift) ((base) << ((shift) << 2));           

#define MIN(a, b) (((a) > (b))?(b):(a))
#define MAX(a, b) (((a) < (b))?(b):(a))
#define ABS(a) (((a)>0)? (a):-(a))

#define MAXINT32 0x7FFFFFFFL
#define MEMCPY(dst,src,n) {uint32_t imcpy; for(imcpy=0;imcpy<(n);imcpy++){((dst)[imcpy])=((src)[imcpy]);}}
#define MEMSET(dst,c,n) {uint32_t i; uint8_t *pt8=(uint8_t *)(dst); for(i=0;i<(n);i++){(pt8[i])=(c);} }


/* bit-field manipulations */
#define CREATE_MASK(msb, lsb)               ((uint32_t)((U(1L) << ((msb) - (lsb) + U(1L))) - U(1L)) << (lsb))
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
#define TEST_BIT(arg, bit)  (U(0) != (U(arg) & (U(1) << U(bit))))

#define FLOAT_TO_INT(x) ((x)>=0.0f?(int)((x)+0.5f):(int)((x)-0.5f))

/* DSP processing data types */
#define samp_t int16_t  /* default size of input samples = 16bits */
#define coef_t int16_t
#define accu_t int32_t  /* accumulator with software pre-shifter */
#define data_t int32_t 
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
    

