[TOC]

# Graph design template
A graph text hasseveral sections :
- Control of the scheduler : debug option, location of the graph in memory
- File path : to easily incorporate sections of data “included” with files
- Formats : most of the arcs are using the same frame length and sampling rate, to avoid repeating the same information the formats are grouped in a table and referenced by indexes
- The boundary of the graph: the IOs are a kind of node with on arc producing or consuming a
stream of data
- The scripts, are a byte-code interpreted language used for simple operations like setting parameters, sharing debug information, calling “callbacks” predefined in the application.
- The list of nodes (also call "linked-list" of nodes), without their connexions with other nodes. This section defines the boot parameters, the memory mapping hand optimized.
- The list of arcs, their relations with two nodes and the minimal type of debug activity on each
transaction

This section gives the commands to insert in the graph text to have it correctly compiled.

## Control of the scheduler
The first words of the binary graph give the portion of the graph to move to RAM.
To have addresses portability of addresses between processors, the graph interpreter is managing a list of "memory-offsets".
Every physical address is computed from a 27 bit-field structure made of : 3 bits used to select maximum 8 memory-offsets (or memory bank). And a 24bits field used as an index in this memory bank.
The function "platform_init_stream_instance()" initializes the interpreter memory-offset table.

### graph_location  "x"
Parameter x :
- 0: (default) the destination of the binary graph is in RAM address corresponding to the first memory-offset
- 1: the graph stays in Flash and the portion copied in RAM is starting from the end of nodes linked-list
- 2: the graph is provided by the application, already in RAM, there is nothing to do

### graph_location_offset "x"
The parameter gives binary graph destination offset from the first memory-offset (default 0)

### debug_script_fields "x"
The parameter is a bit-field of flags controlling the scheduler loop :
bit 0 (lsb 1) set means "call the debug/trace script before each node is called"
bit 1 (2) set means "call the debug script after each node is called"
bit 2 (4) set means "call the debug script at the end of the loop"
bit 3 (8) set means "call the debug script when starting the graph scheduling"
bit 4 (16) set means "call the debug script when returning from the graph scheduling"
no bit is set (default) the debug script is not called 

### scheduler_return "x"
1: return to application caller subroutines after each node execution calls
2: return to caller once all node of the graph are parsed
3: return to caller when all nodes are starving (default 3)

### allowed_processors "x"
bit-field of the processors allowed to execute this graph, (default = 1 main processor)

### set_file_path "index"  "path"
Example `set_file_path 2 ./   ; file path index 2 is the current folder`
Index and its file path, used when including files (sub graphs, parameter files and scripts)

### graph_memory_bank "x"
Example `graph_memory_bank 1   ; selection of memory bank 1`
"x" : index of the memory bank indexes where to map the graph (default 0).

## Formats
### format "n"            
example `format_raw_data 2 ; all further details are for format index 2`
index used to start the declaration of a new format
### format_raw_data "n"
The parameter is the raw data code of the table below.
Example `format_raw_data 17 ; raw data is "signed integers of 16bits" `
The default index is 17 : STREAM_16 (see Annexe "Data Types").

### format_frame_length "n"          
Frame length in number of bytes (default :1)
Example `format_frame_length 160 `
### format_nbchan "n"
Number of channels in the stream (default 1)
Example `format_nbchan 2 ; stereo format ` 
### format_interleaving "n" 
Example `format_interleaving 0 `
0 means interleaved raw data, 1 means deinterleaved data by packets of "frame size"
### format_time_stamp "n"
Example `format_time_stamp 40 ; time-stamp format TIME16D `
time-stamp format 0:none, 1:counter, 39:STREAM_TIME16, 40:STREAM_TIME16D , 41:STREAM_TIME32, 42:STREAM_TIME32D, 43:STREAM_TIMESTMP, 44:STREAM_TIME64, 45:STREAM_TIME64MS, 46:STREAM_TIME64ISO
### format_domain "n"
Example `format_domain 2   ; this format uses specific details of audio out domain `
 | DOMAIN | CODE | COMMENTS |
 | --------- | -- | ---------- |
 | GENERAL | 0 | (a)synchronous sensor, electrical, chemical, color, remote data, compressed streams, JSON, SensorThings, application processor | 
 | AUDIO_IN | 1 | microphone, line-in, I2S, PDM RX | 
 | AUDIO_OUT | 2 | line-out, earphone / speaker, PDM TX, I2S, | 
 | GPIO_IN | 3 | generic digital IO , control of relay, | 
 | GPIO_OUT | 4 | generic digital IO , control of relay, general (a)synchronous sensor + rescaling, .. remote data, compressed streams, JSON, SensorThings | 
 | MOTION | 5 | accelerometer, combined or not with pressure and gyroscope audio_in microphone, line-in, I2S, PDM RX | 
 | 2D_IN | 6 | camera sensor audio_out line-out, earphone / speaker, PDM TX, I2S, | 
 | 2D_OUT | 7 | display, led matrix, gpio_in generic digital IO , control of relay, | 
 | ANALOG_IN | 8 | analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage gpio_out generic digital IO , control of relay, | 
 | ANALOG_OUT | 9 | D/A, position piezzo, PWM converter motion accelerometer, combined or not with pressure and gyroscope | 
 | RTC | 10 | ticks sent from a programmable timer 2d_in camera sensor | 
 | USER_INTERFACE_IN | 11 | button, slider, rotary button 2d_out display, led matrix, | 
 | USER_INTERFACE_OUT | 12 | LED, digits, display, analog_in with aging control | 
 | PLATFORM_3 | 13 | platform-specific #3, decoded with callbacks | 
 | PLATFORM_2 | 14 | platform-specific #2, decoded with callbacks | 
 | PLATFORM_1 | 15 | platform-specific #1, decoded with callbacks | 


### Information specific of domains (TBC)

#### Audio
format_sampling_rate 16000.0     
format_audio_mapping (FrontLeft, FrontRight, FrontCenter, LowFrequency, BackLeft, BackRight, SideLeft, SideRight ..)

#### Motion
format_motion_mapping 1 for accelerometer only 
format_motion_mapping 2 for gyroscope only
format_motion_mapping 3 for magnetometer only
format_motion_mapping 4 for A + G 
format_motion_mapping 5 for A + M 
format_motion_mapping 6 for G + M 
format_motion_mapping 7 for A + G + M 

#### 2D
format_2d_height pixels
format_2d_width pixels
format_2d_border pixels


## Interfaces of the graph

### stream_io "n" 
The parameter is the interface index used in the graph. This declaration starts the definition of a new IO 
Example ` stream_io 2 `

### stream_io_hwid "ID"
The stream_io is using the ID of the physical interface given in platform manifests (default #0)
Example ` stream_io_hwid 2  `

### stream_io_format "n" 
Parameter: index to the table of formats (default #0)
Example ` stream_io_format 0 `

### stream_io_setting "W32"
"IO settings" is a specific bit-field structure, specific to the IO domain, placed at the beginning of the binary graph, and used during the initialization sequence of the graph (default #0).
Example ` stream_io_setting 7812440 `

### stream_io_setting_callback "cb" "X" 
The function "platform_init_stream_instance()" initializes the interpreter pointers to the callbacks proposed by the platform. 
Example 
` stream_io_setting_callback 6 7812440 ; Use callback 6 for the setting of the  `
`                                      ; current stream_io using parameter 7812440 `

## Memory mapping (TBD)
Split the memory mapping to ease memory overlays between nodes and arcs by defining new memory-offset index ("ID").
Format : ID, new ID to use in the node/arc declarations, byte offset within the original ID, length of the new memory offset
` ;               original_id  new_id    start   length `
` ; memory_mapping      2        100      1024    32700 `

### Memory fill
Filling pattern placed after the arc descriptors 
` mem_fill_pattern 5 3355AAFF   memory fill 5 word32 value 0x3355AAFF (total 20 Bytes) `


## Subgraphs (TBD)
A subgraph is equalivalent to program subroutines for graphs. A subgraph can be reused in several places in the graph or in other subgraph.
The graph compiler creates references by name mangling from the call hierarchy. A subgraph receives indexes of IO streams and memory bank indexes for tuning the memory map.
The caller gives its indexes of the arcs to use in the subgraph, and the memory mapping offset indexes. 
Example :
` subgraph `
`    sub1                        ; subgraph name, used for name mangling ` 
`    3 sub_graph_0.txt           ; path and file name `
`    5 i16: 0 1 2 3 4            ; 5 streaming interfaces data_in_0, data_out_0 ..  `
`    3 i16: 0 0 0                ; 3 partitions for fast/slow/working (identical here) `

## Nodes declarations
Nodes are declared with their name and respective instance index in the graph (or subgraph). 
The system integrator can set a "preset" (pre-tuned list of parameters described on node's documentation) and node-specific parameters to load at boot-time.
The address offset of the nodes is provided as a result of the graph compilation step.
Declaration syntax : 
` node <node_name> <instance_index> `

### node_preset "n"
The system intgrator can select 16 "presets" when using a node, each corresponding to a configuration of the node (see its documentation). 
The Preset value is with RESET and SET_PARAMETER commands, the default value is 0.
Example : ` node_preset              1      ; parameter preset used at boot time `

### node_malloc_add "n" "i"
A node memory allocation is described in its manifest. 
A node can ask for up to 4 memory banks with tunable fields : type (static, working, periodic backup), speed (normal, fast, critical fast), relocatable, prog/data and the size.
The size can be a simple number of bytes coupled to a function of stream format parameters (number of channels, sampling rate, frame size) and a flexible parameter defined in the graph, here.
The total memory allocation size in bytes = 
` A                              fixed memory allocation in Bytes (default 0) ` 
` +B x nb_channels of arc(i)     number of channels in arc index i (default 0) ` 
` +C x sampling_rate of arc(j)   sampling rate of arc index j (default 0) ` 
` +D x frame_size of arc(k)      frame size used for the arc index k (default 0) ` 
` +parameter from the graph      optional field "node_malloc_add" ` 

For example an extra scratch area allocation can added as a function of the number of pixels in the images to process (default 0).
Example : ` node_malloc_add 12 0  ; add 12 bytes to segment 0` 

### node_map_hwblock "memory segment" "HW memory offset"
This command is used to tune the memory mapping and bypass the speed requirement of the node manifest. It tells to force the memory segment index given in the first parameter to be mapped to the memory offset index of the second parameter.
Example : ` node_map_hwblock 0 2 ; memory segment 0 is mapped to bank offset 2 `

### node_map_copy / node_map_swap "memory segment" "HW memory offset"
This command is used to optimize the memory mapping of small and fast memory segment by copying, or swapping, a memory segment content from and other memory offset (usually a slower one).
Usage : 
` node_map_copy 1 0; forced copy of the indexed node memory segment 1 to hardware memory offset 0 `
` node_map_swap 1 0; forced swap of the indexed node memory segment 1 to hardware memory offset 0 `
In the above both cases the memory segment 1 is copied from offset memory segment 1 (a dummy arc descriptor is created to access this temporary area). 
In the second swap case the scheduler reads node memory segment and updates the slow memory.

### node_trace_id "io"
Selection of the graph IO interface used for sending the debug and trace informations.
Example : ` node_trace_id  0      ; IO port 0 is used to send the trace `

### node_map_proc, node_map_arch and node_map_rtos
The graph can be executed in a multiprocessor and multi tasks platform. Those commands allow the graph interpreter scheduler to skip the nodes not associated to the current processor / architecture and task.
The platform can define 7 architectures and 7 processors. When the parameter is not defined (or with value 0) the scheduler interprets it as "any processor" or "any architecture" can execute this node.
Several OS threads can interpret the graph at the same time. A parameter "0" means any thread can execute this node, and by convention the value "1" is allocated to low-latency tasks, "3" to background tasks. 
Examples :
` node_map_proc 2 ; run this node on processor 2 defined in the architecture manifest` 
` node_map_arch 1 ; run this node on architecture 1 defined in the architecture manifest` 
` node_map_rtos 1 ; run this node on thread index 1 (low-latency)`  

### node_map_verbose "level (TBD)
The default verbose level on the debug trace of this node is level "0", meaning "quiet". Other values from 1 to 3 tell to trace more information.
Example:
` node_map_verbose 3 ; highest level of verbose debug trace `

### node_memory_isolation  "0 / 1"
Activate (parameter "1") the processor memory protection unit during the execution of this node. 
Example : 
`node_memory_isolation 1 ; activation of the memory protection unit (MPU), default 0 `

### node_script "index of the script"
The indexed script is executed before and after the node execution. The conditionnal flag is clear on the first call and set on the second call.
Example :
` node_script 12 ; call script #12 associated to this node `

### node_parameters "tag"
This command declares the parameters to share with the node during the RESET sequence. If the "tag" parameter is null is tells the following parameters is a full set. Otherwise it is an index of a subset defined in the node documentation.
The following declaration is a list of data terminated with the "end".
Example of a packed structure of 22 bytes of parameters:
    ` node_parameters     0                   TAG = "all parameters" `
    `     1  u8;  2                           Two biquads `
    `     1  u8;  1                           postShift `
    `     5 s16; 681   422   681 23853 -15161  elliptic band-pass 1450..1900/16kHz `
    `     5 s16; 681 -1342   681 26261 -15331 `
    ` end `
    
## Scripts 
Scripts are small interpreted byte-codes designed for control and calls to the graph scheduler for node control and parameter settings.
The declaration is made to tune for a minimum amount of memory consumption : by a limitation of the number of virtual CPU registers, the size of the stack and allowing the same stack memory to be reused for several scripts.
A script is an instance of the node `arm_stream_script` the parameter of which is holding the byte-codes.

` node arm_stream_script 1  ; script (instance) index         `  
`     script_registers   2  ; number of word64 registers      ` 
`     script_stack      12  ; size of the stack in word64     ` 
`     script_mem_shared  1  ; private memory (0) or shared(1) ` 
`     script_mem_map     0  ; mapping to VID #0 (default)     ` 
`                                                             ` 
`     script_language 0     ; start of macro assembler        ` 
`         test R1 = 0                                         ` 
`         if yes R2 = R3 + 3.14                               ` 
`         if no jump L_AA                                     ` 
`         R2 = min(R1, R4)                                    `
`         label L_AA                                          ` 
`         return                                               ` 
`    end                                                      ` 
`    node_parameters <ID2>        ;                           ` 
`        include 1 binary_code.txt ; path ID and file name    ` 
`    end                                                      ` 

## ARC of the graph
The syntax is different for arcs connected to the boundary of the graph, and arcs placed between two nodes.
Depending on real-time behaviors (CPU load and jitter, task priorities, speed of data streams) the data can processed in-place (large input images for examples) or it can be mandatory to copy the data in temporary FIFO before being processied in the graph.
The parameter "set0copy1" is set to 0 (default value) for a processing made "in-place" : the base address the IO FIFO descriptor is modified in the IO transfer acknowlegment subroutine arm_graph_interpreter_io_ack() to point directly to the IO data.
When the parameter is 1 the data is copied in the FIFO, and the graph compiler will allocate an amount of memory corresponding to a "format_frame_length".

Example :
` input arc from graph IO 4 using set0copy1=1 and using format #0      ` 
`           to node xxfilter instance 6 input #0 using format #8       `
` arc_input 4 1 0    xxfilter 6 0 8                                    `
`                                                                      `
` output arc from node xxdetector instance 5 output #1 using format #2 `
`            to graph IO 7 using set0copy1=0 and format #9             `
` arc_output 5 1 2   xxdetector 7 0 9                                  ` 
`                                                                      `
` arc between nodeAAA instance 1 output #2 using format #0             `
`     and nodeBBB instance 3 output #4 using format #1                 `
` arc nodeAAA 1 2 0   nodeBBB 3 4 1                                    `

### arc flow control 
Example
` arc_flow_error 1  ; #1 do something depending on the IO domain when a flow error occurs, default #0 (no extra processing, just skip data)

### arc debug
Each arc descriptor can be configured to have an operation (in a list of 32) implemented with result returned in a dedicated memory section of the graph.
| CODE | DEBUG OPERATION |
| --------- | :---------- |
| 0   | no operation                                                                        |
| 1   | increment DEBUG_REG_ARCW1 with the number of RAW samples                            |
| 2   | set a 0 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to clear                        |
| 3   | set a 1 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to set                          |
| 4   | increment *DEBUG_REG_ARCW1                                                          |
| 5   |                                                                                     |
| 6   | call-back in the application side, data rate estimate in DEBUG_REG_ARCW1            |
| 7   | second call-back : wake-up processor from DEBUG_REG_ARCW1=[ProcID, command]         |
| 8   |                                                                                     |
| 9   | time_stamp_last_access                                                              |
| 10  | peak with forgeting factor 1/256 in DEBUG_REG_ARCW1                                 |
| 11  | mean with forgeting factor 1/256 in DEBUG_REG_ARCW1                                 |
| 12  | min with forgeting factor 1/256 in DEBUG_REG_ARCW1                                  |
| 13  | absmin with forgeting factor 1/256 in DEBUG_REG_ARCW1                               |
| 14  | when data is changing the new data is push to another arc DEBUG_REG_ARCW1=[ArcID]   |
| 15  | automatic rewind read/write                                                         |

Example :
` arc_debug_cmd  1  debug action "ARC_INCREMENT_REG"         ` 
` arc_debug_reg  3  index of the 64bits result, default = #0 ` 
` arc_debug_page 0  page of 32 words / page, default = #0    ` 

### arc_flush
` arc_flush           0           ; control of register "MPFLUSH_ARCW1" : forced flush of data in MProcessing and shared tasks` 

### arc_extend_address
` arc_extend_addr     1           ; address range extension-mode of the arc descriptor "EXTEND_ARCW2" for large NN models, default = #0 (no extension) ` 

### arc_map_hwblock
` arc_map_hwblock     0  map the buffer to a memory offset, default = #0 (VID0)` 

### arc_jitter_ctrl
` arc_jitter_ctrl     1.5         ; factor to apply to the minimum size between the producer and the consumer, default = 1.0 (no jitter) ` 

### arc_parameters
Arcs are used to node parameters when the inlined way (with the node declaration) is limited to 256kBytes. The node manifest declares the number of arcs used for large amount of parameters (NN model, video file, etc ..).
` arc_parameters  0       ; (parameter arcs) buffer preloading, or arc descriptor set with script ` 
`     7  i8; 2 3 4 5 6 7 8 ; parameters                                                           ` 
`include 1 filter_parameters.txt ; path + text file-name using parameter syntax                   ` 
` end                                                                                             ` 

# Data Types
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
