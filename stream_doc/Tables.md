[TOC]
-------------------------
# Tables

## Stream format Words 0,1,2
Words 0, 1 and 2 are common to all domains :
| Word | Bits | Comments |
| --------- | -- | :---------- |
| 0 | 0..21 | frame size in Bytes (including the time-stamp field)  |
| 0 | 22..31 | reserved |
| 1 | 0..4 |  nb channels-1 [1..32 channels] |
| 1 | 5 | 0 for raw data interleaving (for example L/R audio or IMU stream), 1 for a pointer to the first channel, next channel address is computed by adding the frame size divided by the number of channels |
| 1 | 6..7 | time-stamp format of the stream applied to each frame :<br />0: no time-stamp <br />1: absolute time reference  <br />2: relative time from previous frame  <br />3: simple counter |
| 1 | 8..9 | time-stamp size on 16bits 32/64/64-ISO format |
| 1 | 10..15 | raw data format |
| 1 | 16..19 | domain of operations (see list below) |
| 1 | 20..21 | extension of the size and arc descriptor indexes by a factor 1/64/1024/16k |
| 1 | 22..26 | sub-type (see below) for pixel type and analog formats |
| 2 | 0..7 | reserved |
| 2 | 8..31 | IEEE-754 FP32 truncated to 24bits (S-E8-M15), 0 means "asynchronous" | 

## Stream format Word 3
Word 3 of "Formats" holds specific information of each domain.
### Audio
Audio channel mapping is encoded on 20 bits. For example a stereo channel holding "Back Left" and "Back Right" will be encoded as 0x0030.
  | Channel name    | Name | Bit | 
  | ----            | -- | -- | 
 |  Front Left               | FL    | 0    |
 |  Front Right              | FR    |    1    |
 |  Front Center             | FC    |    2    |
 |  Low Frequency            | LFE   |    3    |
 |  Back Left                | BL    |    4    |
 |  Back Right               | BR    |    5    |
 |  Front Left of Center     | FLC   |    6    |
 |  Front Right of Center    | FRC   |    7    |
 |  Back Center              | BC    |    8    |
 |  Side Left                | SL    |    9    |
 |  Side Right               | SR    |    10   |
 |  Top Center               | TC    |    11   |
 |  Front Left Height        | TFL   |    12   |
 |  Front Center Height      | TFC   |    13   |
 |  Front Right Height       | TFR   |    14   |
 |  Rear Left Height         | TBL   |    15   |
 |  Rear Center Height       | TBC   |    16   |
 |  Rear Right Height        | TBR   |    17   |
 |  Channel 19        | C19   |    18   |
 |  Channel 20        | C20   |    19   |

### Motion
Motion sensor channel mapping (w/wo the temperature)
  | Motion sensor data    | Code |  
  | ----            | -- | 
 |  only acceleromete   | 1  |
 |  only gyroscope    | 2  |
 |  only magnetometer   | 3  |
 |  A + G    | 4  |
 |  A + M    | 5  |
 |  G + M    | 6  |
 |  A + G + M    | 7  |

### 2D
Format of the images in pixels: height, width, border. The "extension" bit-field of the word -1 allow managing larger images.
  | 2D  data    | bits range |  comments |
  | ----            | -- | --- |
  | smallest dimension  | 0 - 11  | the largest dimension is computed with (frame_size - time_stamp_size)/smallest_dimension | 
  | image ratio   | 12 - 14  | TBD =0, 1/1 =1, 4/3 =2, 16/9 =3, 3/2=4 | 
  | image format   | 15  |  0 for horizontal, 1 for vertical | 
  | image sensor border   | 17 - 18  | 0 .. 3 pixels border | 

## Data Types
Raw data types
| TYPE | CODE | COMMENTS |
| --------- | -- | :---------- |
| STREAM_DATA_ARRAY | 0 | stream_array : [0NNNTT00] number, type |
| STREAM_S1 | 1 | S, one signed bit, "0" = +1 one bit per data |
| STREAM_U1 | 2 | one bit unsigned, Boolean |
| STREAM_S2 | 3 | SX two bits per data |
| STREAM_U2 | 4 | uu |
| STREAM_Q1 | 5 | Sx ~stream_s2 with saturation management |
| STREAM_S4 | 6 | Sxxx four bits per data |
| STREAM_U4 | 7 | xxxx |
| STREAM_Q3 | 8 | Sxxx |
| STREAM_FP4_E2M1 | 9 | Seem micro-float [8 .. 64] |
| STREAM_FP4_E3M0 | 10| Seee [8 .. 512] |
| STREAM_S8 | 11| ` Sxxxxxxx`  eight bits per data |
| STREAM_U8 | 12| ` xxxxxxxx`  ASCII char, numbers.. |
| STREAM_Q7 | 13| ` Sxxxxxxx`  arithmetic saturation |
| STREAM_CHAR | 14| ` xxxxxxxx`  |
| STREAM_FP8_E4M3 | 15| ` Seeeemmm`  NV tiny-float [0.02 .. 448] |
| STREAM_FP8_E5M2 | 16| ` Seeeeemm`  IEEE-754 [0.0001 .. 57344] |
| STREAM_S16 | 17| ` Sxxxxxxx.xxxxxxxx` 2 bytes per data |
| STREAM_U16 | 18| ` xxxxxxxx.xxxxxxxx`  Numbers, UTF-16 characters |
| STREAM_Q15 | 19| ` Sxxxxxxx.xxxxxxxx`  arithmetic saturation |
| STREAM_FP16 | 20 | ` Seeeeemm.mmmmmmmm`  half-precision float |
| STREAM_BF16 | 21 | ` Seeeeeee.mmmmmmmm`  bfloat |
| STREAM_Q23 | 22 | ` Sxxxxxxx.xxxxxxxx.xxxxxxxx`  24bits 3 bytes per data |
| STREAM_Q23_ | 32 | ` SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxx`  4 bytes per data |
| STREAM_S32 | 24 | one long word |
| STREAM_U32 | 25 | ` xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx`  UTF-32, .. |
| STREAM_Q31 | 26 | ` Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx`  |
| STREAM_FP32 | 27 | ` Seeeeeee.mmmmmmmm.mmmmmmmm..`  FP32 |
| STREAM_CQ15 | 28 | ` Sxxxxxxx.xxxxxxxx+Sxxxxxxx.xxxxxxxx (I Q)`  |
| STREAM_CFP16 | 29 | ` Seeeeemm.mmmmmmmm+Seeeeemm.. (I Q)`  |
| STREAM_S64 | 30 | long long 8 bytes per data |
| STREAM_U64 | 31 | unsigned 64 bits |
| STREAM_Q63 | 32 | ` Sxxxxxxx.xxxxxx ....... xxxxx.xxxxxxxx`  |
| STREAM_CQ31 | 33 | ` Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxx..`  |
| STREAM_FP64 | 34 | ` Seeeeeee.eeemmmmm.mmmmmmm ...`  double |
| STREAM_CFP32 | 35 | ` Seeeeeee.mmmmmmmm.mmmmmmmm.m..+Seee..`  (I Q) |
| STREAM_FP128 | 36 | ` Seeeeeee.eeeeeeee.mmmmmmm ...`  quadruple precision |
| STREAM_CFP64 | 37 | fp64 + fp64 (I Q) |
| STREAM_FP256 | 38 | ` Seeeeeee.eeeeeeee.eeeeemm ...`  octuple precision |
| STREAM_TIME16 | 39 | ssssssssssssqqqq q14.2 1 hour + 8mn +/- 0.0625s |
| STREAM_TIME16D | 40 | qqqqqqqqqqqqqqqq q15 [s] time difference +/- 15us |
| STREAM_TIME32 | 41 | ssssss .. ssssssssssssqqqq q28.4 [s] 8.5 years +/- 0.0625s |
| STREAM_TIME32D | 42 | ssssss..sssqqqqq.. q17.15 [s] 36h  +/- 30us time difference |
| STREAM_TIMESTMP | 43 | ssssssssssssssssssssqqqqqqqqqqqq q20.12 [s] 12 days |
| STREAM_TIME64 | 44 | ____sssssssssssssss..qqqqqqqqqq q32.28 [s] 140 Y +Q28 [s] |
| STREAM_TIME64MS | 45 | ___________mmmmmmmmmmmmmmm ms |
| STREAM_TIME64ISO | 46 | ISO8601___..YY .. MM..MM..DD..DD..SS..SS.....offs..MM .. |
| STREAM_WGS84 | 47 | <--LAT 32B--><--LONG 32B--> |
| STREAM_HEXBINARY | 48 | UTF-8 lower case hexadecimal byte stream |
| STREAM_BASE64 | 49 | RFC-2045 base64 for xsd:base64Binary XML data |
| STREAM_STRING8 | 50 |UTF-8 string of char terminated by 0 |
| STREAM_STRING16 | 51 | UTF-16 string of char terminated by 0 |

-----------------------------------------

# Sub-types 

## GENERAL 
## AUDIO_IN and  AUDIO_OUT 
    STREAM_SUBT_AUDIO_MPG  0    /* compressed byte stream */
    
## GPIO_IN and GPIO_OUT
## MOTION 
## 2D_IN  
## 2D_OUT
## ANALOG_IN and ANALOG_OUT 

RFC2458

 IO_DOMAIN_GENERAL           : subtypes and tuning  SUBTYPE_FMT1  
     GENERAL          0
     GENERAL_COMP195X 1   compressed byte stream following RFC1950 / RFC1951 ("deflate") 
     GENERAL_DPCM     2   compressed byte stream 
     GENERAL_JSON     3   JSON 
     GENERAL_XFORMAT  4   SensorThings MultiDatastream extension 

 IO_DOMAIN_AUDIO_IN          : subtypes and tuning  SUBTYPE_FMT1 
     AUDIO_IN        0    no subtype_units : integer/ADC format  
     AUDIO_MPG       0    compressed byte stream 

 IO_DOMAIN_AUDIO_OUT         : subtypes and tuning  SUBTYPE_FMT1 
     AUDIO_OUT       0    no subtype_units : integer/DAC format  

 IO_DOMAIN_GPIO_IN           : subtypes and tuning  SUBTYPE_FMT1 
 IO_DOMAIN_GPIO_OUT          : subtypes and tuning  SUBTYPE_FMT1 

    GPIO_IN     0    no subtype_units  
    GPIO_OUT    0    no subtype_units  

 IO_DOMAIN_MOTION_IN         : subtypes and tuning  SUBTYPE_FMT1 
    MOTION_A     1
    MOTION_G     2
    MOTION_B     3
    MOTION_AG    4
    MOTION_AB    5
    MOTION_GB    6
    MOTION_AGB   7

 IO_DOMAIN_2D_IN             : subtypes and tuning  SUBTYPE_FMT1 
                      raw data is uint8 or uint16 but the subtype tells how to extract the pixel data 
 IO_DOMAIN_2D_OUT            : subtypes and tuning  SUBTYPE_FMT1 
    2D_YUV420P   1   Luminance, Blue projection, Red projection, 6 bytes per 4 pixels, reordered 
    2D_YUV422P   2   8 bytes per 4 pixels, or 16bpp, Y0 Cb Y1 Cr (1 Cr & Cb sample per 2x1 Y samples) 
    2D_YUV444P   3   12 bytes per 4 pixels, or 24bpp, (1 Cr & Cb sample per 1x1 Y samples) 
    2D_CYM24     4   cyan yellow magenta 
    2D_CYMK32    5   cyan yellow magenta black 
    2D_RGB8      6   RGB  3:3:2,  8bpp, (msb)2B 3G 3R(lsb) 
    2D_RGB16     7   RGB  5:6:5, 16bpp, (msb)5R 6G 5B(lsb) 
    2D_RGBA16    8   RGBA 4:4:4:4 32bpp (msb)4R 
    2D_RGB24     9   BBGGRR 24bpp (msb)8B 
    2D_RGBA32   10   BBGGRRAA 32bpp (msb)8B 
    2D_RGBA8888 11   AABBRRGG OpenGL/PNG format R=lsb A=MSB ("ABGR32" little endian) 
    2D_BW1B     12   Y, 1bpp, 0 is black, 1 is white 
    2D_GREY2B   13   Y, 2bpp, 0 is black, 3 is white, ordered from lsb to msb  
    2D_GREY4B   14   Y, 4bpp, 0 is black, 15 is white, ordered from lsb to msb 
    2D_GREY8B   15   Grey 8b, 0 is black, 255 is white 


 IO_DOMAIN_ANALOG_IN     : subtypes and tuning  SUBTYPE_FMT1 
 IO_DOMAIN_ANALOG_OUT : subtypes and tuning  SUBTYPE_FMT1 

    // Secondary Unit (rfc8798)           Description          SenML Unit     Scale     Offset 

 


  | NAME           |  CODE | UNIT        | COMMENT                                                    |     
  | -- | -- | ---- | ------------- |     
  | ANA_ANY           |  0 |           | any                                                    |     
  | ANA_METER         |  1 |  m        | meter                                                  |
  | ANA_KGRAM         |  2 |  kg       | kilogram                                               |
  | ANA_GRAM          |  3 |  g        | gram                                                   |
  | ANA_SECOND        |  4 |  s        | second                                                 |
  | ANA_AMPERE        |  5 |  A        | ampere                                                 |
  | ANA_KELVIB        |  6 |  K        | kelvin                                                 |
  | ANA_CANDELA       |  7 |  cd       | candela                                                |
  | ANA_MOLE          |  8 |  mol      | mole                                                   |
  | ANA_HERTZ         |  9 |  Hz       | hertz                                                  |
  | ANA_RADIAN        | 10 |  rad      | radian                                                 |
  | ANA_STERADIAN     | 11 |  sr       | steradian                                              |
  | ANA_NEWTON        | 12 |  N        | newton                                                 |
  | ANA_PASCAL        | 13 |  Pa       | pascal                                                 |
  | ANA_JOULE         | 14 |  J        | joule                                                  |
  | ANA_WATT          | 15 |  W        | watt                                                   |
  | ANA_COULOMB       | 16 |  C        | coulomb                                                |
  | ANA_VOLT          | 17 |  V        | volt                                                   |
  | ANA_FARAD         | 18 |  F        | farad                                                  |
  | ANA_OHM           | 19 |  Ohm      | ohm                                                    |
  | ANA_SIEMENS       | 20 |  S        | siemens                                                |
  | ANA_WEBER         | 21 |  Wb       | weber                                                  |
  | ANA_TESLA         | 22 |  T        | tesla                                                  |
  | ANA_HENRY         | 23 |  H        | henry                                                  |
  | ANA_CELSIUSDEG    | 24 |  Cel      | degrees Celsius                                        |
  | ANA_LUMEN         | 25 |  lm       | lumen                                                  |
  | ANA_LUX           | 26 |  lx       | lux                                                    |
  | ANA_BQ            | 27 |  Bq       | becquerel                                              |
  | ANA_GRAY          | 28 |  Gy       | gray                                                   |
  | ANA_SIVERT        | 29 |  Sv       | sievert                                                |
  | ANA_KATAL         | 30 |  kat      | katal                                                  |
  | ANA_METERSQUARE   | 31 |  m2       | square meter (area)                                    |
  | ANA_CUBICMETER    | 32 |  m3       | cubic meter (volume)                                   |
  | ANA_LITER         | 33 |  l        | liter (volume)                                         |
  | ANA_M_PER_S       | 34 |  m/s      | meter per second (velocity)                            |
  | ANA_M_PER_S2      | 35 |  m/s2     | meter per square second (acceleration)                 |
  | ANA_M3_PER_S      | 36 |  m3/s     | cubic meter per second (flow rate)                     |
  | ANA_L_PER_S       | 37 |  l/s      | liter per second (flow rate)                           |
  | ANA_W_PER_M2      | 38 |  W/m2     | watt per square meter (irradiance)                     |
  | ANA_CD_PER_M2     | 39 |  cd/m2    | candela per square meter (luminance)                   |
  | ANA_BIT           | 40 |  bit      | bit (information content)                              |
  | ANA_BIT_PER_S     | 41 |  bit/s    | bit per second (data rate)                             |
  | ANA_LATITUDE      | 42 |  lat      | degrees latitude[1]                                    |
  | ANA_LONGITUDE     | 43 |  lon      | degrees longitude[1]                                   |
  | ANA_PH            | 44 |  pH       | pH value (acidity; logarithmic quantity)               |
  | ANA_DB            | 45 |  dB       | decibel (logarithmic quantity)                         |
  | ANA_DBW           | 46 |  dBW      | decibel relative to 1 W (power level)                  |
  | ANA_BSPL          | 47 |  Bspl     | bel (sound pressure level; log quantity)               |
  | ANA_COUNT         | 48 |  count    | 1 (counter value)                                      |
  | ANA_PER           | 49 |  /        | 1 (ratio e.g., value of a switch; )                 |
  | ANA_PERCENT       | 50 |  %        | 1 (ratio e.g., value of a switch; )                |
  | ANA_PERCENTRH     | 51 |  %RH      | Percentage (Relative Humidity)                         |
  | ANA_PERCENTEL     | 52 |  %EL      | Percentage (remaining battery energy level)            |
  | ANA_ENERGYLEVEL   | 53 |  EL       | seconds (remaining battery energy level)               |
  | ANA_1_PER_S       | 54 |  1/s      | 1 per second (event rate)                              |
  | ANA_1_PER_MIN     | 55 |  1/min    | 1 per minute (event rate, "rpm")                      |
  | ANA_BEAT_PER_MIN  | 56 |  beat/min | 1 per minute (heart rate in beats per minute)          |
  | ANA_BEATS         | 57 |  beats    | 1 (Cumulative number of heart beats)                  |
  | ANA_SIEMPERMETER  | 58 |  S/m      | Siemens per meter (conductivity)                       |
  | ANA_BYTE          | 59 |  B        | Byte (information content)                             |
  | ANA_VOLTAMPERE    | 60 |  VA       | volt-ampere (Apparent Power)                           |
  | ANA_VOLTAMPERESEC | 61 |  VAs      | volt-ampere second (Apparent Energy)                   |
  | ANA_VAREACTIVE    | 62 |  var      | volt-ampere reactive (Reactive Power)                  |
  | ANA_VAREACTIVESEC | 63 |  vars     | volt-ampere-reactive second (Reactive Energy)          |
  | ANA_JOULE_PER_M   | 64 |  J/m      | joule per meter (Energy per distance)                  |
  | ANA_KG_PER_M3     | 65 |  kg/m3    | kg/m3 (mass density, mass concentration)               |
  | ANA_DEGREE        | 66 |  deg      | degree (angle)                                        |
  | ANA_NTU           | 67 |  NTU      | Nephelometric Turbidity Unit                           |
  |  ----- rfc8798 ----- | |  Secondary Unit   (SenML Unit) |   Scale and Offset  |
  | ANA_MS           | 68 |s     millisecond                    | scale = 1/1000    1ms = 1s x [1/1000]        |
  | ANA_MIN          | 69 |s     minute                         | scale = 60                                   |
  | ANA_H            | 70 |s     hour                           | scale = 3600                                 |
  | ANA_MHZ          | 71 |Hz    megahertz                      | scale = 1000000                              |
  | ANA_KW           | 72 |W     kilowatt                       | scale = 1000                                 |
  | ANA_KVA          | 73 |VA    kilovolt-ampere                | scale = 1000                                 |
  | ANA_KVAR         | 74 |var   kilovar                        | scale = 1000                                 |
  | ANA_AH           | 75 |C     ampere-hour                    | scale = 3600                                 |
  | ANA_WH           | 76 |J     watt-hour                      | scale = 3600                                 |
  | ANA_KWH          | 77 |J     kilowatt-hour                  | scale = 3600000                              |
  | ANA_VARH         | 78 |vars  var-hour                       | scale = 3600                                 |
  | ANA_KVARH        | 79 |vars  kilovar-hour                   | scale = 3600000                              |
  | ANA_KVAH         | 80 |VAs   kilovolt-ampere-hour           | scale = 3600000                              |
  | ANA_WH_PER_KM    | 81 |J/m   watt-hour per kilometer        | scale = 3.6                                  |
  | ANA_KIB          | 82 |B     kibibyte                       | scale = 1024                                 |
  | ANA_GB           | 83 |B     gigabyte                       | scale = 1e9                                  |
  | ANA_MBIT_PER_S   | 84 |bit/s megabit per second             | scale = 1000000                              |
  | ANA_B_PER_S      | 85 |bit/s byteper second                 | scale = 8                                    |
  | ANA_MB_PER_S     | 86 |bit/s megabyte per second            | scale = 8000000                              |
  | ANA_MV           | 87 |V     millivolt                      | scale = 1/1000                               |
  | ANA_MA           | 88 |A     milliampere                    | scale = 1/1000                                      |
  | ANA_DBM          | 89 |dBW   decibel rel. to 1 milliwatt    | scale = 1       Offset = -30   0 dBm = -30 dBW              |
  | ANA_UG_PER_M3    | 90 |kg/m3 microgram per cubic meter      | scale = 1e-9                                    |
  | ANA_MM_PER_H     | 91 |m/s   millimeter per hour            | scale = 1/3600000                               |
  | ANA_M_PER_H      | 92 |m/s   meterper hour                  | scale = 1/3600                                  |
  | ANA_PPM          | 93 |/     partsper million               | scale = 1e-6                                    |
  | ANA_PER_100      | 94 |/     percent                        | scale = 1/100                                   |
  | ANA_PER_1000     | 95 |/     permille                       | scale = 1/1000                                  |
  | ANA_HPA          | 96 |Pa    hectopascal                    | scale = 100                                     |
  | ANA_MM           | 97 |m     millimeter                     | scale = 1/1000                                  |
  | ANA_CM           | 98 |m     centimeter                     | scale = 1/100                                   |
  | ANA_KM           | 99 |m     kilometer                      | scale = 1000                                    |
  | ANA_KM_PER_H     |100 |m/s   kilometer per hour             | scale = 1/3.6                                   |
  | ANA_GRAVITY      |101 |m/s2  earth gravity                  | scale = 9.81         1g = m/s2 x 9.81           |
  | ANA_DPS          |102 |1/s   degrees per second             | scale = 360        1dps = 1/s x 1/360           |
  | ANA_GAUSS        |103 |Tesla Gauss                          | scale = 10-4         1G = Tesla x 1/10000       |
  | ANA_VRMS         |104 |Volt  Volt rms                       | scale = 0.707     1Vrms = 1Volt (peak) x 0.707  |
  | ANA_MVPGAUSS     |105 |millivolt Hall effect, mV/Gauss      | scale = 1    1mV/Gauss                     |

## Stream format "domains"
  | Domain name        | Code | Comments | 
  | ----            |   -- | -- | 
  | GENERAL            |   0 | (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings  | 
  | AUDIO_IN           |   1 | microphone, line-in, I2S, PDM RX                                                                                        | 
  | AUDIO_OUT          |   2 | line-out, earphone / speaker, PDM TX, I2S,                                                                              | 
  | GPIO_IN            |   3 | generic digital IO                                                                                   | 
  | GPIO_OUT           |   4 | generic digital IO , control of relay,                                                                                  | 
  | MOTION             |   5 | accelerometer, combined or not with pressure and gyroscope                                                              | 
  | 2D_IN              |   6 | camera sensor                                                                                                           | 
  | 2D_OUT             |   7 | display, led matrix,                                                                                                    | 
  | ANALOG_IN          |   8 | analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage        | 
  | ANALOG_OUT         |   9 | D/A, position piezzo, PWM converter                                                                                     | 
  | RTC                |  10 | ticks sent from a programmable timer                                                                                    | 
  | USER_INTERFACE_IN  |  11 | button, slider, rotary button                                                                                           | 
  | USER_INTERFACE_OUT |  12 | LED, digits, display,                                                                                                   | 
  | PLATFORM_3         |  13 | platform-specific #3            | 
  | PLATFORM_2         |  14 | platform-specific #2                                                                            | 
  | PLATFORM_1         |  15 | platform-specific #1                                                                         | 

-------------------------

# IO Controls Bit-fields per domain

TODO