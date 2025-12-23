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





# Platform Manifest

The “Top” platform manifest has four sections.

1. The first section lists file paths to improve readability.

2. The second section specifies the name of the manifest file describing the processing architecture (see the next paragraph).

3. The third section lists the available data streams that can be connected to a graph.  
   This list corresponds to manifest files describing data format options and provides an index (IO_AL_idx) for the function used to read or write data and update the stream configuration.

The following fields are associated with each data stream entry.

- path: path identifier.  

- Manifest: manifest file name.  
- IO_AL_idx: index used in the graph.  
- ProcCtrl: processor ID affinity bit field.  
- ClockDomain: provision for ASRC (clock domain).  

Some I/Os can alternatively be clocked from the system clock (0) or from other clocks.  
The system integrator uses this field to decide whether flow errors are handled with buffer interpolation (0) or ASRC (other clock domain index).  
The clock domain index is used only to group and synchronize data flows by domain.

1. The fourth section lists the nodes already installed in the device.  This is also a list of manifest files that formally describe how nodes can be connected to each other.



```
;=================================================================
; TOP MANIFEST :
;   - paths to the files
;   - shared memories
;   - list of processors and their private memories and IOs
;   - list of the nodes installed in each processor and/or architectures
;=================================================================
    6                      six file paths

    0 ../../../stream_platform/                   
    1 ../../../stream_platform/computer/manifest/ 
    2 ../../../stream_nodes/arm/                  
    3 ../../../stream_nodes/signal-processingFR/  
    4 ../../../stream_nodes/bitbank/              
    5 ../../../stream_nodes/elm-lang/             



;=================================================================
    3           number of processors
    2           number of shared memory 

;   processor and architecture ID are in the range [1..7]
; 
;   Access    0 data/prog R/W, 1 data R, 2 R/W, 3 Prog, 4 R/W
;   Speed     0 slow/best effort, 1 fast/internal, 2 TCM
;   Type      0 Static, 1 Retention, 2 scratch mem

;---MEMID SIZE  A S T   Comments
    0    8000   1 0 0   shared memory
    3    1000   1 0 1   simulates shared retention memory
   



;=================================================================
;   Processor #1 - architecture #1 , two processors 
;   proc ID, arch ID, main proc, nb mem, service mask, I/O
    1        1        1          2       15            7  

;---MEMID SIZE  A S T   Comments
    1    1000   1 2 0   simulates DTCM 
    2    1000   1 2 1   simulates ITCM

;-----IO AFFINITY WITH PROCESSOR 1-------------------------------
    ;Path      Manifest       IO_AL_idx    Comments               
    1 io_platform_data_in_0.txt       0    application proc
    1 io_platform_data_in_1.txt       1    application proc
    1 io_platform_analog_sensor_0.txt 2    ADC                    
    1 io_platform_audio_in_0.txt      4    microphone       
    1 io_platform_line_out_0.txt      6    audio out stereo       
    1 io_platform_gpio_out_0.txt      7    GPIO/LED               
    1 io_platform_data_out_0.txt      9    application proc  




;=================================================================
;   Processor #2
;   proc ID, arch ID, main proc, nb mem, service mask, I/O
    2        1        0          2       15            2  

;---MEMID SIZE  A S T   Comments
    3    1000   1 2 0   
    4    1000   1 2 1   

;------IO AFFINITY WITH PROCESSOR 2-------------------------------
    ;Path   IO Manifest        IO_AL_idx    Comments               
    1 io_platform_data_in_0.txt        0    shared with Proc 1
    1 io_platform_motion_in_0.txt      3    accelero=gyro          
           



;=================================================================
;   Processor #3 - new architecture, one processor 
;   proc ID, arch ID, main proc, nb mem, service mask, I/O
    1        2        0          0       15            2  

;---MEMID SIZE  A S T   Comments

;------IO AFFINITY WITH PROCESSOR 2-------------------------------
    ;Path   IO Manifest        IO_AL_idx    Comments               
    1 io_platform_2d_in_0.txt          5    camera                 
    1 io_platform_gpio_out_1.txt       8    GPIO/PWM               




;======== ALL NODES ==============================================
; scheduler algorithm :
; if the node archID > 0 then check compatibility with processor archID and exit
; if the node procID > 0 then check compatibility with processor procID and exit
;
;   Path                 Node Manifest                PROC ARCH |  ID
    2       script/node_manifest_script.txt             0   0   |   1 runs everywhere
    2       router/node_manifest_router.txt             0   1   |   2 SMP on archID-1
    2    amplifier/node_manifest_amplifier.txt          0   1   |   3 
    2       filter/node_manifest_filter.txt             0   1   |   4 
    2    modulator/node_manifest_modulator.txt          0   1   |   5 
    2  demodulator/node_manifest_demodulator.txt        0   1   |   6 
    3     detector/node_manifest_detector.txt           0   1   |   7 
    3    resampler/node_manifest_resampler.txt          0   1   |   8 
    3   compressor/node_manifest_compressor.txt         0   1   |   9 
    3 decompressor/node_manifest_decompressor.txt       0   1   |  10 
    4      JPEGENC/node_manifest_bitbank_JPEGENC.txt    0   1   |  11 
    5      TJpgDec/node_manifest_TjpgDec.txt            0   1   |  12 
;-------------------------------------------------------------  |     ----
    2      filter2D/node_manifest_filter2D.txt          2   2   |  13 only archID-2
    3    detector2D/node_manifest_detector2D.txt        0   2   |  14 single processor
;
;
end ; the platform manifest ends here
;==================================================================
```



--------------------------------------

# IO Manifest 

The graph interface manifests (“IO manifests”) are text files composed of commands used by the graph compiler to understand the type of data flowing through the I/Os of the graph.  
A manifest provides a precise list of information about the data format, such as frame length, data type, and sampling rate.

An IO manifest starts with a header containing general information, followed by a list of commands used to describe the stream.  
The IO manifest reader assumes default values, and associated commands may be omitted or included by the programmer for information and readability purposes.

Because of the variety of stream data types and configuration options, the graph interpreter introduces the concept of physical domains (audio, motion, 2D, and others).  
The document starts with general information describing the digital format of the stream, followed by specifications of domain-related information.

## IO manifest header 

The "IO manifest" starts with the name which will be used in a GUI design tool, followed by the "domain" (list below). 

Example of a simple IO manifest :

```
    io_name     io_platform_sensor_in_0  ; the IO name
    io_domain   analog_in				 ; the domain of operation
```

### List of IO Domains

See [Stream format "domains"](#Stream-format-"domains")

## Declaration of options

The manifest provides a list of possible options, described either as a list or as a range of values.  
The syntax consists of an index followed by a list of numbers enclosed in braces “{” and “}”.  
The index specifies the default value to select from the list.  
An index value of “1” corresponds to the first element of the list.  
An index value of “0” means “any value”.  
In this case, the list may be empty.

Example of a list of options between five values, the index is 2 meaning the default value is the second in the list (value = 6).

```
{ 2   5 6 7 8 9 }
```

When the index is negative the list is decoded as a "range". A Range is a set of three fields : 

- the first option
- the step to the next possible option
- the last (included) option

The absolute index value selects the default value in this range.

Example of an option list of values (1, 1.2, 1.4, 1.6, 1.8, .. , 4.2), the index is -3 meaning the default value is the third in the list (value = 1.4).

```
 { -3    1 0.2 4.2 } 
```

## Default values of an IO manifest

When not mentioned in the manifest the following assumptions are :

| io manifest command    | default value | comments                               |
| ---------------------- | ------------- | -------------------------------------- |
| io_commander0_servant1 | 1             | servant                                |
| io_set0copy1           | 0             | buffer address is set by the scheduler |
| io_direction_rx0tx1    | 0             | data flow to the graph                 |
| io_raw_format          | float32       | float is the default data format       |
| io_interleaving        | 0             | raw data interleaving                  |
| io_nb_channels         | 1             | mono                                   |
| io_frame_length        | 1             | in byte (mono or multichannel)         |

## Common information of all digital stream

IO manifests describe the stream data format and how data is copied or used.  
This information applies to digital streams of any IO.  
The next section, “IO Controls Bit-fields per domain (IO-Controls-Bit-fields-per-domain)”, is specific to each domain of operation.

### io_commander0_servant1 "0/1"

An IO is a “commander” when it initiates data exchanges with the graph without control from the scheduler, for example an audio codec.  
An IO is a “servant” when the scheduler must asynchronously pull or push data by calling abstraction-layer IO functions, for example an interface to the main application.

IO streams are managed by the graph scheduler using one subroutine per IO, referenced by the IO_AL_idx function of the abstraction layer, using the following template:

```
typedef void (*p_io_function_ctrl)(uint32_t command, void *data, uint32_t length);
```

The command parameter can take the following values:  
STREAM_SET_PARAMETER, STREAM_DATA_START, STREAM_STOP, or STREAM_SET_BUFFER.

Once a data transfer is complete, the external IO driver calls arm_graph_interpreter_io_ack() to notify the scheduler so it can update the corresponding arc.

```
void arm_graph_interpreter_io_ack(uint8_t graph_io_idx, void *data, uint32_t size);
```

Example:

```
io_commander0_servant1  1  ; default is servant (1)
```

**Graph interpreter implementation details**

IO streams are managed by the graph scheduler using one subroutine per IO, with the following template:

typedef void (*p_io_function_ctrl)(uint32_t command, uint8_t *data, uint32_t length);

The command parameter can take the following values:  
STREAM_SET_PARAMETER, STREAM_DATA_START, STREAM_STOP, or STREAM_SET_BUFFER.

When the IO is a commander, it calls arm_graph_interpreter_io_ack() when data is read.  
When the IO is a servant, the scheduler calls p_io_function_ctrl(STREAM_RUN, …) to request a data transfer.  
Once the transfer is complete, the IO driver calls arm_graph_interpreter_io_ack().

### io_buffer_alloc_in_graph "0/1"

This command declares whether the IO stream uses a memory area provided by the scheduler.  
The default value is 0.

```
io_buffer_alloc_in_graph  1  ; data buffer is allocated in the graph
```

### io_direction_rx0tx1 "0/1"

This command declares the direction of the stream from the graph scheduler’s point of view.

```
io_direction_rx0tx1   1  ; direction of the stream  0:input 1:output 
```

### io_raw_format "option"

Declaration of the size and type of the raw [Data Types](#Data-Types) using the [Declaration of options](#Declaration-of-options) format.

```
io_raw_format {1 3}  ; raw arithmetic's computation format is STREAM_S16 
```

### io_interleaving "0/1"

```
io_interleaving    1  ; multichannel interleaved (0), deinterleaved by frame-size (1) 
```

### io_nb_channels "option"

Declaration of the possible number of channels using the [Declaration of options](#Declaration-of-options) format.

```
io_nb_channels {2  1 2 3 4}  ; options for the number of channels, stereo default
```

### io_frame_length "option"

Declaration of the possible frame length using the [Declaration of options](#Declaration-of-options) format, in bytes. A sample can be multichannel but is still counted as one sample.

```
io_frame_length  {1 1 2 16 }   ; options of possible frame_length in bytes
```

### io_frame_duration "U option"

Declaration of the possible frame duration using the [Declaration of options](#Declaration-of-options) format, in a time unit given in the first parameter. See [Standard units](#Standard-units).

```
io_frame_duration minutes {1 1 2 16 }   ; options of possible frame_length in minutes
```

### io_setup_time  "x"

This command provides information about the time required before valid or calibrated samples are ready for processing after reset.  
This information is provided for documentation purposes only.

```
io_setup_time  12.5 ; wait 12.5ms before receiving valid data
```

### io_units_rescale "u a b max"

The syntax is: physical unit name, coefficient a, coefficient b, and maximum value.  
This command provides rescaling information between normalized digital units (1.0 or 0x7FFF) and physical units, using the formula P = a × (D − b).

See file [Units](#Units) for the list of available Units from RFC8798 and RFC8428.  

```
io_units_rescale  vrms  0.0135  -10.1  0.15

; V_physical = a x (X_sample - b) with the default hardware settings
; V [VRMS] <=> 0.0135 x ( X - (-10.1)) 
; 0.15 VRMS <=> 0.0135 x (1.0 - (-10.1))  0.15 Vrms corresponds to digital full-scale 
```

### io_subtype_multiple "..."

This command declares multiple interleaved units with individual rescaling factors defined by io_units_rescale.  
It is used, for example, with motion sensors delivering acceleration, speed, magnetic field, temperature, and similar data.

```
io_subtype_multiple DPS a b max GAUSS a b max
```

### io_position "U 3D"

This command declares the position of the IO in a three-dimensional space using the unit specified by the first parameter. See [Standard units](#Standard-units).

```
io_position centimeter  1.1 -2.2 0.01 ; relative XYZ position from the platform reference point
```

### io_euler_angles "U 3A"

This specifies the relative XYZ position with respect to the platform reference point. See [Standard units](#Standard-units).

```
io_euler_angles degree 10 20 88.5  ; Euler angles with respect to the platform reference orientation, in degrees
```

### io_sampling_rate "U option"

This command declares the IO stream sampling rate using the frequency unit specified by the first parameter. See [Standard units](#Standard-units).

```
io_sampling_rate hertz {2 16e3 44.1e3 48000} ; sampling rate options in Hz=9
```

### io_sampling_period "U option"

Declaration of the sampling period using the [Declaration of options](#Declaration-of-options) format, in a time unit given in the first parameter. See [Standard units](#Standard-units).

```
io_sampling_period  second {1 1 60 120 }  ; sampling period, enumeration in seconds (4)   
```

### io_sampling_rate_accuracy "p"

Percentage of random inaccuracy of the given sampling rate. 

```
io_sampling_rate_accuracy  0.01   ; in percentage, or 100ppm
```

### io_time_stamp_format "option"

See file [Units](#Units) for the definition of time-stamp format inserted before each frame :

- 0: no time stamp
- 1: simple counter
- 2: time difference in float32 [seconds] 
- 3: time distance from Unix epoch in float64 [seconds]

```
io_time_stamp_format {1 2} ; time-stamp differences
```


--------------------------

## IO Controls Bit-fields per domain

The graph starts with a table of four words per IO.  
The first word is used to connect the IO with the graph, including the arc index, direction, and the index of the associated abstraction-layer function.  
Three 32-bit words are reserved for domain-specific tuning items, referred to below as W1 and W2.

### Domain audio_in and audio_out

#### Domain audio_in setting W1

Channel mapping with a bit-field (20 channels description see [audio channels](#Audio-stream-format)) :

```
io_channel_mapping  0x0B       ; Front Left + Right + LFE

Name                         bit position 
Front Left            FL         0
Front Right           FR         1
Front Center          FC         2
Low Frequency         LFE        3
Back Left             BL         4
Back Right            BR         5
Front Left of Center  FLC        6
Front Right of Center FRC        7
Back Center           BC         8
Side Left             SL         9
Side Right            SR         10
Top Center            TC         11
Front Left Height     TFL        12
Front Center Height   TFC        13
Front Right Height    TFR        14
Rear Left Height      TBL        15
Rear Center Height    TBC        16
Rear Right Height     TBR        17
```

Graph syntax example :

```
stream_io_setting 15 ; selection four channels 
```

#### Domain audio_in setting W2

Control of the gains and filters

```
io_audio_analog_gain     {1  0 12 24       }   ; analog gain (PGA)
io_audio_digital_gain    {-1 -12 1 12      }   ; digital gain range
io_audio_hp_filter       {1 1 20 50 300    }   ; high-pass filter (DC blocker) ON(1)/OFF(0) 
io_audio_agc              0                    ; agc automatic gain control, ON(1)/OFF(0) 
io_audio_router          {1  0 1 2 3       }   ; router  from AMIC0 DMIC1 HS2 LINE3 BT/FM4 
io_audio_gbass_filter    {1  1  1  0 -3 3 6}   ; ON(1)/OFF(0) options for gains in dB
io_audio_fbass_filter    {1  20 100 200    }   ; options for frequencies
io_audio_gmid_filter     {1  1  1  0 -3 3 6}   ; ON(1)/OFF(0) options for gains in dB
io_audio_fmid_filter     {1  500 1000      }   ; options for frequencies
io_audio_ghigh_filter    {1  1  0 -3 3 6   }   ; ON(1)/OFF(0) options for gains in dB 
io_audio_fhigh_filter    {1  4000 8000     }   ; options for frequencies
```

| field name | nb bits | comments |
| ---------- | ------- | ------- |
| io_analog_gain  | 3  | analog gain (PGA)                             |
| io_digital_gain | 4  | digital gain range                            |
| io_hp_filter    | 2  | high-pass filter (DC blocker) ON(1)/OFF(0)    |
| io_agc          | 1  | agc automatic gain control, ON(1)/OFF(0)      |
| io_router       | 3  | router  from AMIC0 DMIC1 HS2 LINE3 BT/FM4     |
| io_gbass_filter | 3  | bass gain in dB                               |
| io_fbass_filter | 2  | filter frequencies                            |
| io_gmid_filter  | 3  | mid frequency gains in dB                     |
| io_fmid_filter  | 2  | filter frequencies                            |
| io_ghigh_filter | 3  | high frequency gain in dB                     |
| io_fhigh_filter | 2  | filter frequencies                            |



### Domain GPIO

#### Domain GPIO setting word 1

| Field name | nb bits | comments |
| --------- | ------- | -------- |
|   State        |      3   |    High-Z, low, high, detection (in/out).    |
|  type     |    3   |  PWM, motor control, GPIO   |
|  control         |     3    |   PWM duty, duration, frequency (buzzer)       |
| debouncing | 1 | pin debouncing control |
| edge | 2 | interrupt edge and polarity control (rising, falling, both, disabled) |

 Domain gpio_in/out setting word 2 and word 3 are not used.

--------------------------------------

### Domain motion

#### Domain motion setting word 1

Selection of the multichannel interleaving :

```
aXg0m0 1 only accelerometer 
a0gXm0 2 only gyroscope 
a0g0mX 3 only magnetometer 
aXgXm0 4 A + G 
aXg0mX 5 A + M 
a0gXmX 6 G + M 
aXgXmX 7 A + G + M 

offset removal on A,M,G
Metadata pattern detection activation and sensitivity
```
 Domain motion setting word 2 and word 3 are not used.



### Domain 2d_in and 2d_out

#### Domain 2d setting word 1 & word 2
| Feature name                         | bits | Description                                                  |
| -------------------------------------| -- | ------------------------------------------------------------ |
| io_raw_format_2d                     | 3  | (U16 + RGB16) (U8 + Grey) (U8 + YUV422)  https://gstreamer.freedesktop.org/documentation/additional/design/mediatype-video-raw.html?gi-language=c  <br/> YCbCr 4:2:2 (16b/pixel), RGB 8:8:8 (24b/pixel) |
| io_trigger flash                     | 4  | activate the flash when polling a new image                  |
| io_synchronize_IR                    | 2  | sync with IR transmitter                                     |
| io_frame rate per second             | 1  |                                                              |
| io_exposure time                     | 3  | The amount of time the photosensor is capturing light, in seconds. |
| io_image size                        | 3  |                                                              |
| io_modes                             | 2  | portrait, landscape, barcode, night modes                    |
| io_Gain                              | 3  | Amplification factor applied to the captured light. >1.0 is brighter <1.0 is darker. |
| io_WhiteBalanceColor                 | 2  | Temperature parameter when using the regular HDRP color balancing. |
| io_MosaicPattern                     | 3  | Color Filter Array pattern for the colors                    |
| io_WhiteBalanceRGBCoef            | 2  | RGB scaling values for white balance, used only if EnableWhiteBalanceRGBCoefficients is selected. |
| io_WhiteBalanceRGBCoef | 3  | Enable using custom RGB scaling values for white balance instead of temperature and tint. |
| io_Auto White Balance                | 4  | Assumes the camera is looking at a white reference, and calibrates the WhiteBalanceRGBCoefficients |
| io_wdr                               | 2  | wide dynamic range                                           |
| io_watermark                         | 1  | watermark insertion                                          |
| io_flip                              | 3  | image format                                                 |
| io_night_mode                        | 3  |                                                              |
| motion detection                     | 2  | sensitivity (low, medium, high)                              |
| io_detection_zones                   | 3  | + {center pixel (in %) radius}, {}, {}                       |
| io_focus_area                        | 2  |                                                              |
| io_auto exposure                     | 3  | on focus area                                                |
| io_focus_distance                    | 2  | forced focus to infinity or xxx meter                        |
| io_jpeg_quality                      | 2  | compression level                        |
| io_backlight brightness control      | 2  | 2D rendering forced focus to infinity or xxx meter                        |



### Domain analog_in and analog_out

    aging coefficient 


## Comments section for IOs

Information examples :

- jumpers to set on the board
- manufacturer references for components and internet URLs
- any other system integration warning and recommendations 
