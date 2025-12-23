| Table of content |
| --------------------- |
| [How to start](#How-to-start) |
| [Graph Interpreter instance](#Graph-Interpreter-instance) |
| [Manifest Files](#Top-Manifest)                             |
| [Platform Manifest](#Platform-Manifest)      |
| [Interfaces Manifests](#IO-Manifest)                        |
| [Nodes Manifests](#Node-manifest)                           |
| [Design of Nodes](#Node-design)                             |
| [Designing a graph](#Graph-design)                          |
| [Formats and Domains](#Common-tables)                       |
| [Common Nodes](#List-of-pre-installed-nodes-(development))  |



# Graph Interpreter instance

The Graph Interpreter exposes two functions.  
One entry point is `void arm_graph_interpreter()`, and one function is used to confirm that data transfers with the outside of the graph have completed, `void arm_graph_interpreter_io_ack()`.  
Both functions can be called once the instance has been created by the platform abstraction layer using `void platform_init_stream_instance(arm_stream_instance_t *S)`.

Interpreter instance structure: 

| name of the field     | comments                                                     |
| --------------------- | ------------------------------------------------------------ |
| long_offset           | A pointer to the table of physical addresses of the memory banks (up to 64).  <br/>The table is located in the abstraction layer of each processor.  <br/>The graph does not use physical memory addresses directly, but offsets into one of these 64 memory banks, as defined in the platform manifest.  <br/>This table enables memory translation between processors without requiring an MMU. |
| linked_list           | pointer to the linked-list of nodes of the graph             |
| platform_io           | table of functions ([IO_AL_idx](#Top-Manifest)) associated to each IO stream of the platform |
| node_entry_points     | table of entry points to each node (see "TOP" manifest)      |
| application_callbacks | The application can provide a list of functions to be called from scripts.  <br/>One callback serves as an entry point for nodes generating metadata information in natural-language text, ready to be processed by specialized small language models. |
| al_services           | pointer to the function proposing services (time, script, stdlib, compute) |
| iomask                | bit-field of the allowed IOs this interpreter instance can trigger |
| scheduler_control     | Execution options of the scheduler, such as returning to the application after each node execution, after a full graph traversal, or when no data is available to process, as well as processor identification. |
| script_offsets        | pointer to scripts used as subroutines for the other scripts placed in the node "arm_stream_script" parameter section. |
| all_arcs              | A pointer to the list of arc descriptors.  <br/>These structures provide the base address, buffer size, read and write indexes, data formats of consumers and producers, and debug or trace information. |
| all_formats           | pointer to the section of the graph describing the stream formats. This section is in RAM. |
| ongoing               | pointer to a table of bytes associated to each IO ports of the graph. Each byte tells if a transfer is on-going. |

Graphical view of the memory mapping

![ ](Graph_mapping.png)

--------------------------------------

## Node memory allocation

A node can request **up to six memory banks** with configurable fields.

These fields include the memory type (static, working or scratch, static with periodic backup),  
the speed requirement (normal, fast, or critical fast),  
whether the memory is relocatable,  
whether it is used for program or data storage,  
and the size in bytes.

The size may be a fixed number of bytes or a computed value based on stream format parameters such as number of channels, sampling rate, or frame size, combined with a flexible parameter defined in the graph.

The total memory allocation size in bytes is computed as follows.

```
   A + 							   (fixed memory allocation in Bytes)
   B x frame_size(mono) of arc(i) or max(input/output arcs) or their sum +
   C x frame_size(mono) x nb_channel of arc(j) + 
   D x nb_channels of arc(k) 
      + parameter from the graph optional field "node_malloc_add" 

```

The first memory block is the node instance, followed by other blocks. This first block has the index #0.

### node_mem "index"           

This command starts the declaration of a memory block with the specified index.
Example :

```
 node_mem 0    ; starts the declaration section of memory block #0 (its instance)
```

### node_mem_alloc  "A"

This parameter specifies the fixed memory allocation value A in bytes.
Example :

```
node_mem_alloc  32    ; add 32 bytes to the current node_mem
```

### node_mem_frame_size_mono "B" "type" ("i")   

Declaration of extra memory in proportion with the mono frame size of the stream flowing through a specified arc index. 

- t = arc : tells the index of arc to consider is "i"
- t = maxin : tells to take the maximum of all input arcs
- t = maxout : the maximum of all output arcs
- t = maxall : the maximum of all arcs
- t = sumin : the sum of frame size of all input arcs
- t = sumout : the sum of frame size of all output arcs
- t = sumall : the sum of frame size of all arcs

Example :

```
node_mem_frame_size_mono 2 maxin   ; declare the 2x maximum size of all input arcs
node_mem_frame_size_mono 5.3 arc 0 ;   add 5.3x mono frame size of arc #0
```

### node_mem_frame_size "C" "type" ("j")   

Declaration of extra memory in proportion with the multichannel frame size of the stream flowing through a specified arc index. 

```
node_mem_frame_size 2 arc 0 ; declare the 2x multichannel frame size of arc #0
```

### node_mem_nbchan "D" "type" ("k")

Declaration of extra memory in proportion to the number of channel of arcs 

```
node_mem_nbchan 44 maxin ; add 44 x maximum nb of channels of input arcs
```

### node_mem_alignment "n"

Declaration of the memory Byte alignment
Example :

```
node_mem_alignement     4           ; 4 bytes to (default) ` 
```

### node_mem_type "n"

Definition of the dynamic characteristics of the memory block :

0 STATIC : memory content is preserved between two calls (default )

1 WORKING : scratch memory content is not preserved between two calls 

2 PERIODIC_BACKUP static memory to reload during a warm reboot 

3 PSEUDO_WORKING static only during the uncompleted execution state of the NODE

Example :

```
node_mem_type 3   ; memory block put in a backup memory area when possible
```

### node_mem_speed "n"

Declaration of the memory desired for the memory block.

0 for 'best effort' or 'no constraint' on speed access

1 for 'fast' memory selection when possible

2 for 'critical fast' section, to be in I/DTCM when available

Example :

```
node_mem_speed 0   ; relax speed constraint for this block
```

### node_mem_relocatable "0/1"

Declares if the pointer to this memory block is relocatable, or assigned a fixed address at reset (default, parameter = '0').
When the memory block is relocatable a command 'STREAM_UPDATE_RELOCATABLE' is used with address changes:  void (node) (command, .. ); This is done with the associated script of the node (TBD).

Example :

```
node_mem_relocatable    1   ; the address of the block can change
```

### node_mem_data0prog1 "0/1"

This command specifies whether the memory block is used for data or program access.

A value of 0 specifies data access.  
A value of 1 specifies program access.

RAM program memory segments are loaded by the node.  
If memory allocation is not possible, the address provided to the node is set to −1.

Example :

```
  node_mem_data0prog1  1 ; program memory block 
```

--------------------------------------



# Common tables

## Stream format Words 0,1,2

Words 0, 1 and 2 are common to all domains :

| Word | Bits   | Comments                                                     |
| ---- | ------ | :----------------------------------------------------------- |
| 0    | 0..24  | frame size in Bytes (including the time-stamp field) + extension |
| 0    | 25..31 | reserved                                                     |
| 1    | 0..4   | nb channels-1 [1..32 channels]                               |
| 1    | 5      | 0 for raw data interleaving (for example L/R audio or IMU stream), 1 for a pointer to the first channel, next channel address is computed by adding the frame size divided by the number of channels |
| 1    | 6..7   | time-stamp format of the stream applied to each frame :<br />0: no time-stamp <br />1: absolute time reference  <br />2: relative time from previous frame  <br />3: simple counter |
| 1    | 8..9   | time-stamp size on 16bits 32/64/64-ISO format                |
| 1    | 10..15 | raw data format                                              |
| 1    | 16..19 | domain of operations (see list below)                        |
| 1    | 20..21 | extension of the size and arc descriptor indexes by a factor 1/64/1024/16k |
| 1    | 22..26 | sub-type (see below) for pixel type and analog formats       |
| 2    | 0..7   | reserved                                                     |
| 2    | 8..31  | IEEE-754 FP32 truncated to 24bits (S-E8-M15), 0 means "asynchronous" |

## Stream format Word 1

Word 3 of "Formats" holds specific information of each domain.

### Audio stream format

Audio channel mapping is encoded on 20 bits. For example a stereo channel holding "Back Left" and "Back Right" will be encoded as 0x0030.

| Channel name          | Name | Bit  |
| --------------------- | ---- | ---- |
| Front Left            | FL   | 0    |
| Front Right           | FR   | 1    |
| Front Center          | FC   | 2    |
| Low Frequency         | LFE  | 3    |
| Back Left             | BL   | 4    |
| Back Right            | BR   | 5    |
| Front Left of Center  | FLC  | 6    |
| Front Right of Center | FRC  | 7    |
| Back Center           | BC   | 8    |
| Side Left             | SL   | 9    |
| Side Right            | SR   | 10   |
| Top Center            | TC   | 11   |
| Front Left Height     | TFL  | 12   |
| Front Center Height   | TFC  | 13   |
| Front Right Height    | TFR  | 14   |
| Rear Left Height      | TBL  | 15   |
| Rear Center Height    | TBC  | 16   |
| Rear Right Height     | TBR  | 17   |
| Channel 19            | C19  | 18   |
| Channel 20            | C20  | 19   |

### Motion

Motion sensor channel mapping (w/wo the temperature)

| Motion sensor data | Code |
| ------------------ | ---- |
| only accelerometer | 1    |
| only gyroscope     | 2    |
| only magnetometer  | 3    |
| A + G              | 4    |
| A + M              | 5    |
| G + M              | 6    |
| A + G + M          | 7    |

### 2D

Format of the images in pixels: height, width, border. The "extension" bit-field of the word -1 allow managing larger images.

| 2D  data            | bits range | comments                                                     |
| ------------------- | ---------- | ------------------------------------------------------------ |
| smallest dimension  | 0 - 11     | the largest dimension is computed with (frame_size - time_stamp_size)/smallest_dimension |
| image ratio         | 12 - 14    | TBD =0, 1/1 =1, 4/3 =2, 16/9 =3, 3/2=4                       |
| image format        | 15         | 0 for horizontal, 1 for vertical                             |
| image sensor border | 17 - 18    | 0 .. 3 pixels border                                         |
| interlace mode      | 2          | progressive, interleaved, mixed, alternate                   |
| chroma              | 2          | jpeg, mpeg2, dv, none                                        |
| color space         | 2          | ITU-BT.601, ITU-BT.709, SMPTE 240M                           |
| invert pixels       | 1          | for  test/debug                                              |
| brightness          | 4          | display control                                              |
| contrast            | 4          | display control                                              |



------

## Data Types

Raw data types

| TYPE              | CODE | COMMENTS                                                |
| ----------------- | ---- | :------------------------------------------------------ |
| STREAM_DATA_ARRAY |  0   | stream_array : `{ 0NNN TT 00 }` number, type            |
| STREAM_FP32       |  1   | ` Seeeeeee.mmmmmmmm.mmmmmmmm..`  FP32                   |
| STREAM_FP64       |  2   | ` Seeeeeee.eeemmmmm.mmmmmmm ...`  double                |
| STREAM_S16        |  3   | ` Sxxxxxxx.xxxxxxxx` 2 bytes per data                   |
| STREAM_S32        |  4   | one long word                                           |
| STREAM_S2         |  5   | `Sx` two bits per data                                  |
| STREAM_U2         |  6   | `uu`                                                    |
| STREAM_S4         |  7   | `Sxxx` four bits per data                               |
| STREAM_U4         |  8   | `xxxx`                                                  |
| STREAM_FP4_E2M1   |  9   | `Seem`  micro-float [8 .. 64]                           |
| STREAM_FP4_E3M0   | 10   | `Seee`   [8 .. 512]                                     |
| STREAM_S8         | 11   | ` Sxxxxxxx`  eight bits per data                        |
| STREAM_U8         | 12   | ` xxxxxxxx`  ASCII char, numbers..                      |
| STREAM_FP8_E4M3   | 13   | ` Seeeemmm`  NV tiny-float [0.02 .. 448]                |
| STREAM_FP8_E5M2   | 14   | ` Seeeeemm`  IEEE-754 [0.0001 .. 57344]                 |
| STREAM_U16        | 15   | ` xxxxxxxx.xxxxxxxx`  Numbers, UTF-16 characters        |
| STREAM_FP16       | 16   | ` Seeeeemm.mmmmmmmm`  half-precision float              |
| STREAM_BF16       | 17   | ` Seeeeeee.mmmmmmmm`  bfloat                            |
| STREAM_S23        | 18   | ` Sxxxxxxx.xxxxxxxx.xxxxxxxx`  24bits 3 bytes per data  |
| STREAM_S23_32     | 19   | ` SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxx`  4 bytes per data |
| STREAM_U32        | 20   | ` xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx`  UTF-32, ..      |
| STREAM_CS16       | 21   | ` Sxxxxxxx.xxxxxxxx+Sxxxxxxx.xxxxxxxx (I Q)`            |
| STREAM_CFP16      | 22   | ` Seeeeemm.mmmmmmmm+Seeeeemm.. (I Q)`                   |
| STREAM_S64        | 23   | long long 8 bytes per data                              |
| STREAM_U64        | 24   | unsigned 64 bits                                        |
| STREAM_CS32       | 25   | ` Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxx..`          |
| STREAM_CFP32      | 26   | ` Seeeeeee.mmmmmmmm.mmmmmmmm.m..+Seee..`  (I Q)         |
| STREAM_FP128      | 27   | ` Seeeeeee.eeeeeeee.mmmmmmm ...`  quadruple precision   |
| STREAM_CFP64      | 28   | fp64 + fp64 (I Q)                                       |
| STREAM_FP256      | 29   | ` Seeeeeee.eeeeeeee.eeeeemm ...`  octuple precision     |
| STREAM_WGS84      | 30   | `<--LAT 32B--><--LONG 32B-->`                           |
| STREAM_HEXBINARY  | 31   | UTF-8 lower case hexadecimal byte stream                |
| STREAM_BASE64     | 32   | RFC-2045 base64 for xsd:base64Binary XML data           |
| STREAM_STRING8    | 33   | UTF-8 string of char terminated by 0                    |
| STREAM_STRING16   | 34   | UTF-16 string of char terminated by 0                   |

------



## Units

| NAME                | CODE | UNIT                              | COMMENT                                        |
| ------------------- | ---- | --------------------------------- | ---------------------------------------------- |
| _ANY                | 0    |                                   | any                                            |
| _METER              | 1    | m                                 | meter                                          |
| _KGRAM              | 2    | kg                                | kilogram                                       |
| _GRAM               | 3    | g                                 | gram                                           |
| _SECOND             | 4    | s                                 | second                                         |
| _AMPERE             | 5    | A                                 | ampere                                         |
| _KELVIB             | 6    | K                                 | kelvin                                         |
| _CANDELA            | 7    | cd                                | candela                                        |
| _MOLE               | 8    | mol                               | mole                                           |
| _HERTZ              | 9    | Hz                                | hertz                                          |
| _RADIAN             | 10   | rad                               | radian                                         |
| _STERADIAN          | 11   | sr                                | steradian                                      |
| _NEWTON             | 12   | N                                 | newton                                         |
| _PASCAL             | 13   | Pa                                | pascal                                         |
| _JOULE              | 14   | J                                 | joule                                          |
| _WATT               | 15   | W                                 | watt                                           |
| _COULOMB            | 16   | C                                 | coulomb                                        |
| _VOLT               | 17   | V                                 | volt                                           |
| _FARAD              | 18   | F                                 | farad                                          |
| _OHM                | 19   | Ohm                               | ohm                                            |
| _SIEMENS            | 20   | S                                 | siemens                                        |
| _WEBER              | 21   | Wb                                | weber                                          |
| _TESLA              | 22   | T                                 | tesla                                          |
| _HENRY              | 23   | H                                 | henry                                          |
| _CELSIUSDEG         | 24   | Cel                               | degrees Celsius                                |
| _LUMEN              | 25   | lm                                | lumen                                          |
| _LUX                | 26   | lx                                | lux                                            |
| _BQ                 | 27   | Bq                                | becquerel                                      |
| _GRAY               | 28   | Gy                                | gray                                           |
| _SIVERT             | 29   | Sv                                | sievert                                        |
| _KATAL              | 30   | kat                               | katal                                          |
| _SQUAREMETER        | 31   | m2                                | square meter (area)                            |
| _CUBICMETER         | 32   | m3                                | cubic meter (volume)                           |
| _LITER              | 33   | l                                 | liter (volume)                                 |
| _M_PER_S            | 34   | m/s                               | meter per second (velocity)                    |
| _M_PER_S2           | 35   | m/s2                              | meter per square second (acceleration)         |
| _M3_PER_S           | 36   | m3/s                              | cubic meter per second (flow rate)             |
| _L_PER_S            | 37   | l/s                               | liter per second (flow rate)                   |
| _W_PER_M2           | 38   | W/m2                              | watt per square meter (irradiance)             |
| _CD_PER_M2          | 39   | cd/m2                             | candela per square meter (luminance)           |
| _BIT                | 40   | bit                               | bit (information content)                      |
| _BIT_PER_S          | 41   | bit/s                             | bit per second (data rate)                     |
| _LATITUDE           | 42   | lat                               | degrees latitude[1]                            |
| _LONGITUDE          | 43   | lon                               | degrees longitude[1]                           |
| _PH                 | 44   | pH                                | pH value (acidity; logarithmic quantity)       |
| _DB                 | 45   | dB                                | decibel (logarithmic quantity)                 |
| _DBW                | 46   | dBW                               | decibel relative to 1 W (power level)          |
| _BSPL               | 47   | Bspl                              | bel (sound pressure level; log quantity)       |
| _COUNT              | 48   | count                             | 1 (counter value)                              |
| _PER                | 49   | /                                 | 1 (ratio e.g., value of a switch; )            |
| _PERCENT            | 50   | %                                 | 1 (ratio e.g., value of a switch; )            |
| _PERCENTRH          | 51   | %RH                               | Percentage (Relative Humidity)                 |
| _PERCENTEL          | 52   | %EL                               | Percentage (remaining battery energy level)    |
| _ENERGYLEVEL        | 53   | EL                                | seconds (remaining battery energy level)       |
| _1_PER_S            | 54   | 1/s                               | 1 per second (event rate)                      |
| _1_PER_MIN          | 55   | 1/min                             | 1 per minute (event rate, "rpm")               |
| _BEAT_PER_MIN       | 56   | beat/min                          | 1 per minute (heart rate in beats per minute)  |
| _BEATS              | 57   | beats                             | 1 (Cumulative number of heart beats)           |
| _SIEMPERMETER       | 58   | S/m                               | Siemens per meter (conductivity)               |
| _BYTE               | 59   | B                                 | Byte (information content)                     |
| _VOLTAMPERE         | 60   | VA                                | volt-ampere (Apparent Power)                   |
| _VOLTAMPERESEC      | 61   | VAs                               | volt-ampere second (Apparent Energy)           |
| _VAREACTIVE         | 62   | var                               | volt-ampere reactive (Reactive Power)          |
| _VAREACTIVESEC      | 63   | vars                              | volt-ampere-reactive second (Reactive Energy)  |
| _JOULE_PER_M        | 64   | J/m                               | joule per meter (Energy per distance)          |
| _KG_PER_M3          | 65   | kg/m3                             | kg/m3 (mass density, mass concentration)       |
| _DEGREE             | 66   | deg                               | degree (angle)                                 |
| _NTU                | 67   | NTU                               | Nephelometric Turbidity Unit                   |
| ----- rfc8798 ----- |      | Secondary Unit   (SenML Unit)     | Scale and Offset                               |
| _MS                 | 68   | s     millisecond                 | scale = 1/1000    1ms = 1s x [1/1000]          |
| _MIN                | 69   | s     minute                      | scale = 60                                     |
| _H                  | 70   | s     hour                        | scale = 3600                                   |
| _MHZ                | 71   | Hz    megahertz                   | scale = 1000000                                |
| _KW                 | 72   | W     kilowatt                    | scale = 1000                                   |
| _KVA                | 73   | VA    kilovolt-ampere             | scale = 1000                                   |
| _KVAR               | 74   | var   kilovar                     | scale = 1000                                   |
| _AH                 | 75   | C     ampere-hour                 | scale = 3600                                   |
| _WH                 | 76   | J     watt-hour                   | scale = 3600                                   |
| _KWH                | 77   | J     kilowatt-hour               | scale = 3600000                                |
| _VARH               | 78   | vars  var-hour                    | scale = 3600                                   |
| _KVARH              | 79   | vars  kilovar-hour                | scale = 3600000                                |
| _KVAH               | 80   | VAs   kilovolt-ampere-hour        | scale = 3600000                                |
| _WH_PER_KM          | 81   | J/m   watt-hour per kilometer     | scale = 3.6                                    |
| _KIB                | 82   | B     kibibyte                    | scale = 1024                                   |
| _GB                 | 83   | B     gigabyte                    | scale = 1e9                                    |
| _MBIT_PER_S         | 84   | bit/s megabit per second          | scale = 1000000                                |
| _B_PER_S            | 85   | bit/s byteper second              | scale = 8                                      |
| _MB_PER_S           | 86   | bit/s megabyte per second         | scale = 8000000                                |
| _MV                 | 87   | V     millivolt                   | scale = 1/1000                                 |
| _MA                 | 88   | A     milliampere                 | scale = 1/1000                                 |
| _DBM                | 89   | dBW   decibel rel. to 1 milliwatt | scale = 1       Offset = -30   0 dBm = -30 dBW |
| _UG_PER_M3          | 90   | kg/m3 microgram per cubic meter   | scale = 1e-9                                   |
| _MM_PER_H           | 91   | m/s   millimeter per hour         | scale = 1/3600000                              |
| _M_PER_H            | 92   | m/s   meterper hour               | scale = 1/3600                                 |
| _PPM                | 93   | /     partsper million            | scale = 1e-6                                   |
| _PER_100            | 94   | /     percent                     | scale = 1/100                                  |
| _PER_1000           | 95   | /     permille                    | scale = 1/1000                                 |
| _HPA                | 96   | Pa    hectopascal                 | scale = 100                                    |
| _MM                 | 97   | m     millimeter                  | scale = 1/1000                                 |
| _CM                 | 98   | m     centimeter                  | scale = 1/100                                  |
| _KM                 | 99   | m     kilometer                   | scale = 1000                                   |
| _KM_PER_H           | 100  | m/s   kilometer per hour          | scale = 1/3.6                                  |
| _GRAVITY            | 101  | m/s2  earth gravity               | scale = 9.81         1g = m/s2 x 9.81          |
| _DPS                | 102  | 1/s   degrees per second          | scale = 360        1dps = 1/s x 1/360          |
| _GAUSS              | 103  | Tesla Gauss                       | scale = 10-4         1G = Tesla x 1/10000      |
| _VRMS               | 104  | Volt  Volt rms                    | scale = 0.707     1Vrms = 1Volt (peak) x 0.707 |
| _MVPGAUSS           | 105  | millivolt Hall effect, mV/Gauss   | scale = 1    1mV/Gauss                         |
| _DBSPL              | 106  | Bspl versus dB SPL(A)             | scale = 1/10                                   |

## Stream format "domains"

| Domain name       | Code | Comments                                                     |
| ----------------- | ---- | ------------------------------------------------------------ |
| GENERAL           | 0    | (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings |
| AUDIO_IN          | 1    | microphone, line-in, I2S, PDM RX                             |
| AUDIO_OUT         | 2    | line-out, earphone / speaker, PDM TX, I2S,                   |
| GPIO              | 3    | generic digital IO, programmable timer ticks, control of relay |
| MOTION            | 4    | accelerometer, combined or not with pressure and gyroscope   |
| 2D_IN             | 5    | camera sensor                                                |
| 2D_OUT            | 6    | display, led matrix,                                         |
| ANALOG_IN         | 7    | analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage |
| ANALOG_OUT        | 8    | D/A, position piezzo, PWM converter                          |
| USER_INTERFACE_IO | 9    | button, slider, rotary button, LED, digits, display,         |
| PLATFORM_6        | 10   | platform-specific #6                                         |
| PLATFORM_5        | 11   | platform-specific #5                                         |
| PLATFORM_4        | 12   | platform-specific #4                                         |
| PLATFORM_3        | 13   | platform-specific #3                                         |
| PLATFORM_2        | 14   | platform-specific #2                                         |
| PLATFORM_1        | 15   | platform-specific #1                                         |



## Architectures codes of platform manifest

Architecture codes (https://sourceware.org/binutils/docs/as/ARM-Options.html)  armv1, armv2, armv2a, armv2s, armv3, armv3m, armv4, armv4xm, armv4t, armv4txm, armv5, armv5t, armv5txm, armv5te, armv5texp, armv6, armv6j, armv6k, armv6z, armv6kz, armv6-m, armv6s-m, armv7, armv7-a, armv7ve, armv7-r, armv7-m, armv7e-m, armv8-a, armv8.1-a, armv8.2-a, armv8.3-a, armv8-r, armv8.4-a, armv8.5-a, armv8-m.base, armv8-m.main, armv8.1-m.main, armv8.6-a, armv8.7-a, armv8.8-a, armv8.9-a, armv9-a, armv9.1-a, armv9.2-a, armv9.3-a, armv9.4-a, armv9.5-a



# List of pre-installed nodes (development)



---------------------------------------------------------------------------------------

| ID   | Name                     | Comments                                               |
| ---- | ------------------------ | ------------------------------------------------------ |
| 1    | arm_stream_script        | byte-code interpreter index "arm_stream_script_INDEX"  |
| 2    | arm_stream_router        | router, mixer, rate and format converter               |
| 3    | arm_stream_amplifier     | amplifier mute and un-mute with ramp and delay control |
| 4    | arm_stream_filter        | cascade of filters                                     |
| 5    | arm_stream_modulator     | signal generator with modulation                       |
| 6    | arm_stream_demodulator   | signal demodulator frequency estimator                 |
| 7    | arm_stream_filter2D      | filter / rescale / zoom / extract / merge / rotate     |
| 8    | sigp_stream_detector     | signal detection in noise                              |
| 9    | sigp_stream_detector2D   | image activity detection                               |
| 10   | sigp_stream_resampler    | asynchronous high-quality sample-rate converter        |
| 11   | sigp_stream_compressor   | raw data compression with adaptive prediction          |
| 12   | sigp_stream_decompressor | raw data decompression                                 |
| 13   | bitbank_jpg_encoder      | jpeg encoder                                           |
| 14   | elm_jpg_decoder          | TjpgDec                                                |


## arm_stream_script (tbd)

Scripts are nodes interpreted from byte codes declared in the indexed SCRIPTS section of the graph or inlined in the parameter section of the node arm_stream_script.  
The first scripts are simple code sequences used as subroutines or called using the node_script index.

Script nodes manage data RAM locations in a shared arc used by all scripts.  
Instance registers, stack parameters, and constants are placed after the byte codes.

The default memory configuration is shared, meaning that buffers associated with scripts share the same memory buffer.

To assign individual static memory to a script, the script_mem_shared parameter must be set to 0.

Special functions activated through Syscall and conditional instructions include the following.

- If-then execution, which conditionally executes blocks of nodes based on script decisions such as FIFO content or debug register values.  
- Loop execution, which repeats a list of nodes multiple times for cache efficiency and small frame sizes.  
- End-thread execution, where the script is the last executed before a list of low-priority nodes.  In this case, ongoing IO operations can be flushed and control can return to the application.



```
node arm_stream_script 1  ; script (instance) index           
    script_stack      12  ; size of the stack in word64      
    script_parameter  30  ; size of the parameter/heap in word32
    script_mem_shared  1  ; private memory (0) or shared(1)  
    script_mem_map     0  ; mapping to VID #0 (default)      

    script_code  
        r1 = add r2 3       ; r1 = add r2 3
      label AAA         
        set r2 graph sigp_stream_detector_0         
        r0 = 0x412              ; r0 = STREAM_SET_PARAMETER(2)
        set r3 param BBB        ; set r3 param BBB 
        sp0 = 1                 ; push 1 Byte (threshold size in BBB)
        Syscall 1 r2 r0 r3 sp0  ; Syscall NODE(1) r2(cmd=set_param) r0(set) r3(data) 
        return              ; return
    end
        
    script_parameters      0    
        1  u8 ;  34             
        2  u32; 0x33333333 0x444444444 
        label BBB              
        1  u8 ;  0x55           
        1  u32;  0x66666666     
    end

```

----------------------------------------------------------------------------------------

## arm_stream_router (tbd)

**Operation** 

This node receives up to four input streams (arcs) and generates up to four output streams.  
Each stream may be multichannel.

The format of the streams is known by the node during the reset and set-parameter commands.

Input streams may be asynchronous and include time stamps.  
Output streams are isochronous with other graph streams and have a defined sampling rate.

The first parameters specify the number of arcs and identify the input arc to be used with high quality of service (HQoS), or −1 if none is used.  
These parameters are followed by a list of routing and mixing rules.

When an HQoS arc is defined, data movement is aligned to that arc in the time domain.  
Other arcs are synchronized accordingly, and data is interpolated or zeroed in case of flow issues.

When no HQoS arc is defined, the node evaluates all input and output arcs and determines the minimum amount of data that can be processed consistently across all arcs in the time domain.

**Use-cases**

The following use-cases can be combined:

1. Router, deinterleaving, interleaving, channels recombination: the input arc data is processed deinterleaved, and the output arc is the result of recombination of any input arc. Audio example with two stereo input arcs using 5ms and 12ms frame lengths, recombined to create a stereo stream interleaved output using 20ms frame length, the left channel from the first arc and the left channel of the second arc.

2. Router and mixer with smoothed gain control: the output arc data can result from the weighted mix of input arcs. The applied gain can be changed on the fly. The slope of the time taken to the desired gain is controlled. Audio example: a mono output arc is computed from the combination of two stereo input arc, by mixing the four input channels with a factor 0.25 applied in the mixer.

3. Router and raw data conversion. The raw formats can be converted to any other format in this list : int16, int32, int64, float16, float32, float64.

4. Router and sampling-rate conversion of isochronous streams (input streams have a determined and independent sampling-rate). Audio example: input streams sampled at 44100Hz is converted to 48000Hz + 0.026Hz for drift compensation (the sampling-rate information, and all the details of the arc's data format, is shared by the graph scheduler during the reset phase of the nodes).

5. Router and conversion of asynchronous streams using time-stamps to an isochronous stream with a determined sampling-rate. Motion sensor example: an accelerometer is sampled at 200Hz (5ms period)  with +/- 1ms jitter sampling time uncertainty. The samples are provided with an accurate time-stamp in float32 format of time differences between samples (or float64 for absolute time reference to Jan 1st 2025). The output samples are delivered resampled at 410Hz with no jitter.

6. Router of data needing a time synchronization at sample or frame level. In this use-case the node waits the input samples are arriving within a time window before delivering an output frame. Example with motor control and the capture of current and voltage on two input arcs: it is important to send time-synchronized pairs of data. The command [node_script “index”](node_script-"index") is used to call a script checking the arrival of current and voltage with their respective time-stamps (logged in the arc descriptors), the scripts check the arrival of data within a time and release execution of the router when conditions are met.

7. Router of streams generated from different threads. The problem is to avoid on multiprocessing devices one channel to be delivered to the final mixer ahead and desynchronized from the others. This problem is solved with an external script controlling the system time like in the use-case 6.

   

**Parameters**

The list of routing and mixing information is :

- index of the input arc (<= 4)
- index of the channels (1 Byte to 31 Bytes)
- index of the output destination arc (<=4)
- index of the channels (1 Byte to 31 Bytes) 
- mixer gain to apply (fp32) and convergence speed (fp32)



Example with the router with two stereo input arcs and two output arcs at 16kHz sampling rate. The first output arc is mono and the sum of the input channels of the first input, upsampled to 48kHz in float32. The second arc is a stereo combination of the two left channels of the input arcs, at the same sampling rate but with 32bits/sample instead of 16bits for the input.

```
format_index            0               ; router input
format_frame_length     16              ; 
format_raw_data         3               ; STREAM_S16
format_nbchan           2               ; stereo
format_sampling_rate    hertz 16000     ; 
format_interleaving     0               ; raw data interleaving
;
format_index            1               ; router output
format_frame_length     96              ; 
format_raw_data         1               ; STREAM_FP32
format_nbchan           1               ; mono
format_sampling_rate    hertz 48000     ; 
format_interleaving     0               ; raw data interleaving
;
format_index            2               ; router output
format_frame_length     32              ; 
format_raw_data         4               ; STREAM_S32
format_nbchan           2               ; stereo
format_sampling_rate    hertz 16000     ; 
format_interleaving     0               ; raw data interleaving
;----------------------------------------------------------------------
stream_io_graph         0   2           ; Platform index 2
stream_io_format        0               ; 
stream_io_setting       2 0 0           ; io_sampling_rate hertz {1 8e3 16e3 48e3}, '2' to set 16kHz
;
stream_io_graph         1   4           ; Platform index 4 microphone 
stream_io_format        0               ; 
stream_io_setting       2 0 0           ; io_sampling_rate hertz {1 8e3 16e3 48e3}, '2' to set 16kHz

stream_io_graph         2   9           ; 
stream_io_format        1               ; 
stream_io_setting       3 0 0           ; io_sampling_rate hertz {2 16e3 44.1e3 48000}, '3' to set 48kHz

stream_io_graph         3   8           ; Platform index 8 
stream_io_format        2               ; 
stream_io_setting       3 0 0           ; io_sampling_rate hertz {2 16e3 44.1e3 48000}, '3' to set 16kHz
;----------------------------------------------------------------------
; 
                 +----------------------------+                    
  Stereo s16     |F0 ---.   F7 +---------+  F4|  Mono 48kHz fp32 Format 1
-----arc0------->|F1    |   F8 |  Mixer  +----+---arc2-------------------> 
Format 0 16kHz   |      |      +---------+    | sum of Left arc0+Right arc0
                 |      |                     |
  Stereo s16     |F2    '-------------------F5| Stereo 16kHz, s32 Format 2 
-----arc1------->|F3 -----------------------F6+---arc3-------------------> 
Format 0 16kHz   +----------------------------+   Left(arc0)   Left(arc1) 

```



    node arm_stream_router  0 
        node_malloc_add    680 1    ; 2 internal FIFO (20 bytes each) on Segment-1 (default = 1 FIFO)
                                    ; 9 FIFOs (64Bytes each) 
                                    ; + 1 MIXERs (40Bytes) 
                                    ; + 2 INTERPOLATORs (32Bytes) 
                                    ; = 680
        node_preset         0
        node_parameters     0       ; TAG = "all parameters"
            1  u8;  2               ; nb input arcs
            1  u8;  2               ; nb output arcs
            2  u8;  255 255         ; arc (in/out) used with HQOS: none
            1  u8;  9               ; nb FIFO
    ;
    ; FIFO declarations
    ;   DIRECT ACCESS TO INPUT FIFO
            4  u8;  0 0   0 0         ; FIFO 0  input  left  arc-0 sub-channel-0 
            4  u8;  1 0   0 1         ; FIFO 1  input  right arc-0 sub-channel-1
            4  u8;  2 0   1 0         ; FIFO 2  input  left  arc-1 sub-channel-0 
            4  u8;  3 0   1 1         ; FIFO 3  input  right arc-1 sub-channel-1 
    ;   DIRECT ACCESS TO OUTPUT FIFO
            4  u8;  4 1   2 0         ; FIFO 4  output mono  arc-2 
            4  u8;  5 1   3 0         ; FIFO 5  output left  arc-3 sub-channel-0 
            4  u8;  6 1   3 1         ; FIFO 6  output right arc-3 sub-channel-1 
    ;   STATIC INTERNAL FIFO
            4  u8;  7 2   2 0         ; FIFO 7  Mixer input0, arc-2(FS) FP32
            4  u8;  8 2   2 0         ; FIFO 8  Mixer input1, arc-2(FS) FP32
    ;
    ; MICROCODE OPERATORS: copy to output(0), interpolate(1), mixer(2), copy to internal fifo(3) 
    ;   move data to prepare {Left arc0 + Right arc0}     
    ;              OP  PARAM
            4  u8;  17 0 7  0       ; interpolate FIFO 0 => FIFO 7 don't increment read index 
            4  u8;  17 1 8  1       ; interpolate FIFO 1 => FIFO 8 increment read index 
            4  u8;  33 7 8 4        ; Mixer FIFO 7 + FIFO 8 => FIFO 4 (mono output)
    
    ;   direct move to arc3 stereo with raw format conversion
            4  u8;  1  0 2  1       ; copy FIFO 0 => FIFO 5, Left arc0  now increment read index
            4  u8;  1  3 6  1       ; copy FIFO 3 => FIFO 6, Right arc1 increment read index
    ;
    end
Operations :

- when receiving the reset command: compute the time granularity for the processing, check if bypass are possible (identical sampling rate on input and output arcs).
- check all input and output arcs to know which is the amount of data (in the time domain) which can me routed and split in "time granularity" chunks. Clear the mixer buffers.

Loop with "time granularity" increments :

- copy the input arcs data in internal FIFO in fp32 format, deinterleaved, with time-stamps attached to each samples.
- use Lagrange polynomial interpolation to resample the FIFO to the output rate. The interpolator is preceded by a an adaptive low-pass filter removing high-frequency content when the estimated input sampling rate higher than the output rate.
- recombination and mixing of resampled data using a programmable gain and smoothed ramp-up to the target value.
- raw data format conversion to the output arc

----------------------------------------------------------------------------------------

## arm_stream_amplifier (TBD)

Operation : rescale and control of the amplitude of the input stream with controlled time of ramp-up/ramp-down. 
The gain control “mute” is used to store the current gain setting, being reloaded with the command “unmute”
Option : either the same gain/controls for all channels or list of parameters for each channel

Parameters :  new gain/mute/unmute, ramp-up/down slope, delay before starting the slope. 
Use-cases :
    Features : adaptive gain control (compressor, expander, AGC) under a script control with energy polling 
    Metadata features : "saturation occurred" "energy"
    Mixed-Signal glitches : remove the first seconds of an IR sensor until it was self-calibrated (same for audio Class-D)

parameters of amplifier (variable size): 
TAG_CMD = 1, uint8_t, 1st-order shifter slope time (as stream_mixer, 0..75k samples)
TAG_CMD = 2, uint16_t, desired gain FP_8m4e, 0dB=0x0805
TAG_CMD = 3, uint8_t, set/reset mute state
TAG_CMD = 4, uint16_t, delay before applying unmute, in samples
TAG_CMD = 5, uint16_t, delay before applying mute, in samples

Slopes of rising and falling gains, identical to all channels
slope coefficient = 0..15 (iir_coef = 1-1/2^coef = 0 .. 0.99)
Convergence time to 90% of the target in samples:
 slope   nb of samples to converge
     0           0
     1           3
     2           8
     3          17
     4          36
     5          73
     6         146
     7         294
     8         588
     9        1178
    10        2357
    11        4715
    12        9430
    13       18862
    14       37724
    15       75450
    convergence in samples = abs(round(1./abs(log10(1-1./2.^[0:15])'))

 Operation : applies vq = **interp1**(x,v,xq) 
 Following https://fr.mathworks.com/help/matlab/ref/interp1.html
   linear of polynomial interpolation (implementation)
 **Parameters : X,V vectors, size max = 32 points**

no preset ('0')

Or used as compressor / expander using long-term estimators instead of sample-based estimator above.

```
node arm_stream_rescaler 0

    parameters     0             ; TAG   "load all parameters"
        
;               input   output
        2; f32; -1      1
        2; f32;  0      0       ; this table creates the abs(x) conversion
        2; f32;  1      1
    end  
end
```

---------------------------------------------------------------------------------------

## 

```
node  arm_stream_amplifier 0


    parameters     0             ; TAG   "load all parameters"
        1  i8;  1           load only rising/falling coefficient slope
        1 h16;  805         gain -100dB .. +36dB (+/- 1%)
        1  i8;  0           muted state
        2 i16;  0 0         delay-up/down
    end  
end
```

----------------------------------------------------------------------------------------



## arm_stream_filter

Operation : receives one multichannel stream and produces one filtered multichannel stream. 
Parameters : biquad filters coefficients used in cascade. Implementation is 2 Biquads max.
(see www.w3.org/TR/audio-eq-cookbook)
Presets:
#0 : bypass
#1 : offset removal filter 
#2 : Median filter, 5 points
#3 : Low pass filter
#4 : High pass filter 
#5 : Peaking  filter
#6 : Bandpass filter
#7 : Notch filter
#8 : Low shelf filter
#9 : High shelf filter
#10: All pass filter
#11: Dithering filter

parameter of filter : 

Normalized frequency f0/FS default = 0.25, 

Q factor default = 1.414

Default gain = 4  (12dB)



```
node arm_stream_filter 0         node subroutine name + instance ID
    node_preset         1        ; parameter preset used at boot time, default = #0
    parameters          0        ; TAG   "load all parameters"
        1  u8;  2       Two biquads
        1  i8;  0       unused
        5 f32; 0.284277f 0.455582f 0.284277f 0.780535f -0.340176f  
        5 f32; 0.284277f 0.175059f 0.284277f 0.284669f -0.811514f 
        ; or  include    1 arm_stream_filter_parameters_x.txt      (path + file-name)
    end
end
```

----------------------------------------------------------------------------------------

## arm_stream_modulator (TBD)

 Operation : sine, noise, square, saw tooth with amplitude or frequency modulation
 use-case : ring modulator, sweep generation with a cascade of a ramp generator and
    a frequency modulator

see https://www.pjrc.com/teensy/gui/index.html?info=AudioSynthWaveform 

```
u8  wave type 1=cosine 2=square 3=white noise 4=pink noise 
    5=sawtooth 6=triangle 7=pulse
    8=prerecorded pattern playback from arc 
    9=sigma-delta with OSR control for audio on PWM ports or 8b DAC
    10=PWM 11=ramp 12=step
u8  modulation type, 0:amplitude, 1:frequency, 2:FSK 
u8  modulation, 0:none 1=from arc bit stream

f32 modulation amplitude
f32 offset
f32 wave frequency [Hz]
f32 starting phase,[-pi .. +pi]
f32 modulation y=ax+b, x=input data, index (a) and offset (b)
f32 modulation frequency [Hz] separating two data bits/samples from the arc
```

```
node arm_stream_modulator (i)

    parameters     0             ; TAG   "load all parameters"
        
        1  u8;  1       sinewave
        2 h16;  FFFF 0  full-scale, no offset
        1 f32;  1200    1200Hz
        1 s16;  0       initial phase
        2  u8;  1 1     frequency modulation from bit-stream
        2 h16;  8000 0  full amplitude modulation with sign inversion of the bit-stream
        1 f32;  300     300Hz modulation => (900Hz .. 1500Hz modulation)
    end
end
```

----------------------------------------------------------------------------------------

## arm_stream_demodulator (TBD)

 Operation : decode a bit-stream from analog data. Use-case: IR decoder, CAN/UART on SPI/I2S audio.

Next: sinewave frequency recognition for alarm detector.

 Parameters : clock and parity setting or let the algorithm discover the frame setting after some time. https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter and https://github.com/Arduino-IRremote/Arduino-IRremote https://www.arduinolibraries.info/categories/display https://www.arduinolibraries.info/categories/display

presets control :
#1 .. 10: provision for  demodulators 

Metadata information can be extracted with the command "parameter-read":
TAG_CMD = 1 read the signal amplitude
TAG_CMD = 2 read the signal to noise ratio

```
node 
    arm_stream_demodulator (i)
    parameters     0             ; TAG   "load all parameters"
        
        2  i8; 2 2          nb input/output arcs
        4 i16; 0 0 2 0      move arc0,chan0, to arc2,chan0
    end
end
```


----------------------------------------------------------------------------------------

## arm_stream_filter2D (TBD)

Filter, rescale/zoom/extract, rotate, exposure compensation, background removal. 

Channel mixer : insert a portion of the processed image in a larger frame buffer.

Operation : 2D filters 
Parameters : spatial and temporal filtering, decimation, distortion, color mapping/log-effect

presets:
#1 : bypass

parameter of filter : 

```
node arm_stream_filter2D   (i)

	TBD
end
```


----------------------------------------------------------------------------------------



## sigp_stream_detector

Operation : provides a boolean output stream from the detection of a rising 
edge above a tunable signal to noise ratio. 
A tunable delay allows to maintain the boolean value for a minimum amount of time 
Use-case example 1: debouncing analog input and LED / user-interface.
Use-case example 2: IMU and voice activity detection (VAD)
Parameters : time-constant to gate the output, sensitivity of the use-case

presets control
#1 : no HPF pre-filtering, fast and high sensitivity detection (button debouncing)
#2 : VAD with HPF pre-filtering, time constants tuned for ~10kHz
#3 : VAD with HPF pre-filtering, time constants tuned for ~44.1kHz
#4 : IMU detector : HPF, slow reaction time constants
#5 : IMU detector : HPF, fast reaction time constants

Metadata information can be extracted with the command "TAG_CMD" from parameter-read:
0 read the floor noise level
1 read the current signal peak
2 read the signal to noise ratio

```
node arm_stream_detector 0               node name  + instance ID
    preset              1               parameter preset used at boot time, default = #0
end
```


----------------------------------------------------------------------------------------

## sigp_stream_detector2D (TBD)

Motion and pattern detector. Counting moving object and build a text report (object sizes, shape, speed, color, ..) to be later processed by SLM on the application processor.

Operation : detection of movement(s) and computation of the movement map
Parameters : sensitivity, floor-noise smoothing factors
Metadata : decimated map of movement detection

```
node arm_stream_detector2D (i)

TBD

end
```

----------------------------------------------------------------------------------------

## sigp_stream_resampler (TBD)

Operation : high quality conversion of multichannel input data rate to the rate of the output arcs 

  + asynchronous rate conversion within +/- 1% adjustment
  + 

SSRC synchronous rate converter, FS in/out are exchanged during STREAM_RESET
ASRC asynchronous rate converter using time-stamps (in) to synchronous FS (out) pre-LP-filtering tuned from Fout/Fin ratio + Lagrange polynomial interpolator

drift compensation managed with STREAM_SET_PARAMETER command:
TAG_CMD = 0 bypass
TAG_CMD = 1 rate conversion

The script associated to the node is used to read the in/out arcs filling state
    to tune the drift control

``` 
node arm_stream_resampler (i)

    parameters     0             ; TAG   "load all parameters"
        
        2  i8; 2 2          nb input/output arcs
        4 i16; 0 0 2 0      move arc0,chan0, to arc2,chan0
    end
end

```

----------------------------------------------------------------------------------------

## sigp_stream_compressor (TBD)

Operation : wave compression using IMADPCM(4bits/sample)
Parameters : coding scheme 

presets (provision codes):

- 1 : coder IMADPCM
- 2 : coder LPC
- 4 : coder CVSD for BT speech 
- 5 : coder SBC
- 6 : coder MP3

```
node 
    arm_stream_compressor 0

    parameters     0             ; TAG   "load all parameters"
        4; i32; 0 0 0 0     provision for extra parameters in other codecs
    end
end
```

----------------------------------------------------------------------------------------

## sigp_stream_decompressor (TBD)

Operation : decompression of encoded data
Parameters : coding scheme and a block of 16 parameter bytes for codecs, VAD threshold and silence frame format (w/wo time-stamps)

​	dynamic parameters : pause, stop, fast-forward x2 and x4.



    WARNING : if the output format can change (mono/stereo, sampling-rate, ..)
        the variation is detected by the node and reported to the scheduler with 
        "STREAM_SERVICE_INTERNAL_FORMAT_UPDATE", the "uint32_t *all_formats" must be 
        mapped in a RAM for dynamic updates with "COPY_CONF_GRAPH0_COPY_ALL_IN_RAM"
    
    Example of data to share with the application
        outputFormat: AndroidOutputFormat.MPEG_4,
        audioEncoder: AndroidAudioEncoder.AAC,
        sampleRate: 44100,
        numberOfChannels: 2,
        bitRate: 128000,

presets provision

- 1 : decoder IMADPCM
- 2 : decoder LPC
- 3 : MIDI player
- 4 : decoder CVSD for BT speech 
- 5 : decoder SBC
- 6 : decoder MP3

```
node arm_stream_decompressor 0

    parameters     0             ; TAG   "load all parameters"
        4; i32; 0 0 0 0     provision for extra parameters in other codecs
    end
end
```

----------------------------------------------------------------------------------------

## bitbank_jpg_encoder (TBD)

From "bitbank"

https://github.com/google/jpegli/tree/main

https://opensource.googleblog.com/2024/04/introducing-jpegli-new-jpeg-coding-library.html

## eml_tjpg_decoder (TBD)

From "EML"

Use-case : images decompression, pattern generation.



