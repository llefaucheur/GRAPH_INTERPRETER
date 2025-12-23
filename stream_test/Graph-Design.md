| Table of content |
| --------------------- |
| [Designing a graph](#Graph-design)                          |
| [Formats and Domains](#Common-tables)                       |
| [Common Nodes](#List-of-pre-installed-nodes-(development))  |



# Graph design

The Graph-Interpreter schedules a linked list of computing nodes interconnected by arcs.  
Node descriptors specify which processor can execute the code and which arcs the node is connected to.  
Arc descriptors specify the base address of buffers, read and write indexes, debug and trace information to log, and a flag indicating whether the consumer node should wrap data to the base address.  
Buffer base addresses are made portable using a 6-bit offset and a 22-bit index.  
The offset is translated by each graph interpreter instance into a physical address defined in the Platform Manifest.

The graph is placed in shared memory accessible to all processors.  
There is no message-passing scheme.  
Graph-Interpreter scheduler instances perform the same estimations in parallel and independently decide which node should be executed with priority.



A graph text description contains several sections.

- Control of the scheduler, including debug options and the location of the graph in memory.  
- File paths used to include external data sections.  
- Formats, which group common frame length and sampling rate information to avoid repetition.  
- The I/Os or graph boundaries, which produce or consume data streams.  
- Scripts, which are byte-code interpreted programs used for simple operations such as setting parameters, sharing debug information, and calling application callbacks.  
- The list of nodes, represented as a linked list without explicit connections, including boot parameters and memory mapping.  
- The list of arcs, describing connections between nodes and minimal debug activity on data transfers.

## **Binary format of the graph** 

| Graph section name                                           | Description                                                  |
| ------------------------------------------------------------ | :----------------------------------------------------------- |
| ------ next sections can either be in RAM or Flash           |                                                              |
| Header (7 words)                                             | The header tells where the graph will be in RAM the size of the following sections, the percentage of memory consumed in the memory banks |
| IO description and links to the device-driver abstraction (4 words/IO) | Each IO descriptor tell if data will be copied in the arc buffers or if the arc descriptor will be set to point directly to the data. |
| Scripts byte code and parameters                             | Scripts are made to update parameters, interface with the application's callbacks, implement simple state-machines, interface with the IOs of the graph |
| List of Nodes instance and their parameters to use at reset time | This section is the translation of the node manifests with additional information from the graph : memory mapping of the node data banks and parameters (preset and specific paremeters) |
| ------ graph sections in RAM area starts here                |                                                              |
| List of flags telling if data requests are on-going on the IOs (1 byte/IO) | The flags "on-going" are set by the scheduler and reset upon data transfer completion |
| List of debug/trace registers used by arcs (2 words/debug register) | Basic programmable data stream analysis (time of last access, average values, estimated data rate) |
| List of Formats, max 256, 4 words/format                     | Frame length, number of channels, interleaving scheme, specific data of the domain |
| List of arc descriptors (5 words/arc)                        | Base address in the portable format (6bits offset 22bits index in words), read/write indexes with Byte accuracy. The descriptor has an "extension" factor to scale all parameter up to 64GB addressing space. |



## Example of graph

The graph in text format :

```
;--------------------------------------------------------------------------
;   Stream-based processing using a graph interpreter :                    
;   
;       - The ADC detection is used to toggle a GPIO
; 
;   +----------+     +--------+      +--------+     +--------+
;   | ADC      +-----> filter +------> detect +-----> GPIO   | 
;   +----------+     +--------+      +--------+     +--------+
;                
;------------------------------------------------------------------
format_index            0
format_frame_length     8
format_index            1
format_frame_length     16
;------------------------------------------------------------------
stream_io               0                   ; IO0
stream_io_hwid          1                   ; io_platform_data_in_1.txt
stream_io               1                   ; IO1
stream_io_hwid          9                   ; io_platform_data_out_0.txt
;------------------------------------------------------------------
node arm_stream_filter  0                   ; first node 
    node_preset         1                   ; Q15 filter
    node_map_hwblock    1  5                ; TCM = VID5
    node_parameters     0                   ; TAG = "all parameters"
        1  u8;  2                           ; Two biquads
        1  u8;  1                           ; postShift
        5 s16; 681   422   681 23853 -15161 ; band-pass 1450..1900/16kHz
        5 s16; 681 -1342   681 26261 -15331 ; 
    end
;----------------------------------------------------------------------
node sigp_stream_detector 0                     ; second node
    node_preset         3                       ; detector preset 
;----------------------------------------------------------------------
;  arc connexions between IOs and node and between nodes

arc_input   0 1 0 arm_stream_filter     0 0 0  
arc_output  1 1 1 sigp_stream_detector  0 1 1  

; arc going from the filter to the detector 
arc_nodes arm_stream_filter 0 1 0 sigp_stream_detector 0 0 1                  
arc_jitter_ctrl  1.5                    ; increase the buffer size

end
```

The compiled result which will be the input file of the interpreter:

```
//--------------------------------------
//  DATE Thu Sep 19 19:49:37 2024
//  AUTOMATICALLY GENERATED CODES
//  DO NOT MODIFY !
//--------------------------------------
0x0000003C, // ------- Graph size = Flash=36[W]+RAM24[W]  +Buffers=48[B] 12[W] 
0x00000000, // 000 000 [0] Destination in RAM 0, and RAM split 0 
0x00000042, // 004 001 [1] Number of IOs 2, Formats 2, Scripts 0 
0x00000015, // 008 002 LinkedList size = 21, ongoing IO bytes, Arc debug table size 0 
0x00000003, // 00C 003 [3] Nb arcs 3  SchedCtrl 0 ScriptCtrl 0   
0x00000001, // 010 004 [4] Processors allowed 
0x00000000, // 014 005 [5] memory consumed 0,1,2,3 
0x00000000, // 018 006 [6] memory consumed 4,5,6,7 ...  
0x00083000, // 01C 007 IO(graph0) 1 arc 0 set0copy1=1 rx0tx1=0 servant1 1 shared 0 domain 0 
0x00000000, // 020 008 IO(settings 0, fmtProd 0 (L=8) fmtCons 0 (L=8) 
0x00000000, // 024 009  
0x00000000, // 028 00A  
...
0x00000000, // 0A0 028           domain-dependent 
0x00000010, // 0A4 029 Format  1 frameSize 16  
0x00004400, // 0A8 02A           nchan 1 raw 17 
0x00000000, // 0AC 02B           domain-dependent 
0x00000000, // 0B0 02C           domain-dependent 
0x0000003C, // 0B4 02D IO-ARC descriptor(0) Base 3Ch (Fh words) fmtProd_0 frameL 8.0 
0x00000008, // 0B8 02E     Size 8h[B] fmtCons_0 FrameL 8.0 jitterScaling 1.0 
0x00000000, // 0BC 02F  
0x00000000, // 0C0 030  
0x00000000, // 0C4 031     fmtCons 0 fmtProd 0 dbgreg 0 dbgcmd 0 
0x0000003E, // 0C8 032 IO-ARC descriptor(1) Base 3Eh (Fh words) fmtProd_1 frameL 16.0 
0x00000010, // 0CC 033     Size 10h[B] fmtCons_1 FrameL 16.0 jitterScaling 1.0 
0x00000000, // 0D0 034  
0x00000000, // 0D4 035  
0x00000101, // 0D8 036     fmtCons 1 fmtProd 1 dbgreg 0 dbgcmd 0 
0x00000042, // 0DC 037 ARC descriptor(2) Base 42h (10h words) fmtProd_0 frameL 8.0 
0x00000018, // 0E0 038     Size 18h[B] fmtCons_1 FrameL 16.0 jitterScaling 1.5 
0x00000000, // 0E4 039  
0x00000000, // 0E8 03A  
0x00000100, // 0EC 03B     fmtCons 1 fmtProd 0 dbgreg 0 dbgcmd 0 
```


## Graph: control of the scheduler

The first words of the binary graph specify the portion of the graph that must be moved to RAM.

To ensure address portability between processors, the graph interpreter manages a list of memory offsets.  
Each physical address is computed from a 28-bit structure composed of 6 bits used to select up to 64 memory offsets, or memory banks, and 22 bits used as an index within the selected memory bank.

The function platform_init_stream_instance() initializes the interpreter memory-offset table.

### graph_locations  "x"

This command defines seven parameters corresponding to the seven graph memory sections listed below.  
Each parameter is associated with a code x.  
A value of −1 means that the section remains in its original location and is used as-is.  
Any other value indicates that the specified memory bank ID will be used for that graph section and that the section will be moved there before execution.

The seven graph memory sections are defined as follows.

```
GRAPH_PIO_HW        0
GRAPH_PIO_GRAPH     1
GRAPH_SCRIPTS       2
GRAPH_LINKED_LIST   3
GRAPH_ONGOING       4
GRAPH_FORMATS       5
GRAPH_ARCS          6
```

Example describing the default assignment.

```
graph_locations -1 -1 -1 -1 0 0 0 ; 4 segments stay in Flash, others go in RAMID 0
```

### debug_script_fields "x"

This command defines a bit field controlling the scheduler loop behavior.

- Bit 0 indicates that the debug or trace script is called before each node execution.  
- Bit 1 indicates that the debug script is called after each node execution.  
- Bit 2 indicates that the debug script is called at the end of the scheduling loop.  
- Bit 3 indicates that the debug script is called when scheduling starts.  
- Bit 4 indicates that the debug script is called when returning from scheduling.  
- If no bits are set, the debug script is not called.

Example :

```
debug_script_fields 0	; no debug script activated
```

### scheduler_return "x"

This command defines when control returns to the application.

A value of 1 returns control after each node execution.  
A value of 2 returns control after all nodes in the graph have been processed.  
A value of 3 returns control when all nodes are starving, which is the default.

Example :

```
scheduler_return 1
```

### allowed_processors "x"

bit-field of the processors allowed to execute this graph, (default = 1 main processor)

Example :

```
allowed_processors 0x81	; (10000001) processor ID 1 and 8 can read the graph
```

### graph_file_path "index"  "path"

Index and its file path, used when including files (sub graphs, parameter files and scripts).

Example :

```
graph_file_path 2  ../nodes/   ; file path index 2 to the folder ../nodes
```

### graph_memory_bank "x"

Command used in the context of memory mapping tuning.
"x" : index of the memory bank indexes where to map the graph (default 0).

Example :

```
graph_memory_bank 1   ; select of memory bank 1 of the Platform manifest
```

-----------------------------------------

## Graph: IO control and stream data formats

There are three data declared in the graph scheduler instance (*arm_stream_instance_t*):
A - a pointer to a RAM area giving 

   - on-going transfer flags
   - debug area of arcs

B - a pointer to the list of IOs bit-fields controlling the setting of the IO, the content of which depends on the *Domain*:

   - the index of the arc creating the interface between the IO and a node of the graph ("arcID")
   - the Rx/Tx direction of the stream, from the point of view of the graph
   - the dynamic behavior : data polling initiated by the scheduler or transfer initiated outside of the graph
   - flag telling if the data are copied in the arc's buffer or if the arc's descriptor is modified to point directly to the data
   - flag telling if the buffer used by the IO interface must be reserved by the graph compiler 
   - physical Domain of the data (see command "format_domain")
   - index to the Abstraction Layer in charge of operating the transfers
   - for audio  (mixed-signal setting, gains, sampling-rate, ..)

C - a pointer to the "Formats" which are structures of four words giving :

 - word 0 : frame size 4MB (Byte accurate) 
 - word 1 : number of channels (1..32), interleaving scheme, time-stamp, raw format, domain, sub-type, frame size extension (up to 64GB +/-16kB)
 - word 2 : sampling rate in [Hz], truncated IEEE FP32 on 24bits : S_E8_M15 
 - word 3 : specific to each domain (audio and motion channel mapping, image format and border)


### format_index "n"            

This command starts the declaration of a new format.

```
format_index 2 ; all further details are for format index 2
```

index used to start the declaration of a new format

### format_raw_data "n"

The parameter is the raw data code of the table below.

```
format_raw_data 3 ; raw data is "signed integers of 16bits" 
```

The default index is 3 : STREAM_16 (see Annexe "Data Types").

### format_frame_length "n"          

Frame length in number of bytes of the current format declaration (default :1)

```
format_frame_length 160
```

### format_nbchan "n"

Number of channels in the stream (default 1)

```
format_nbchan 2 ; stereo format 
```

### format_sampling_rate "u f"

Sampling rate in unit "u": hertz, bpm (beats per minute), rpm (rotation per minute)

```
format_sampling_rate hertz 1e-3  ;   1mHz
```

### format_sampling_period "u t"

Sampling unit "u" : second, minute, hour, day

```
format_sampling_period hour 12  ;   period of 12 hours
```

### format_interleaving "n" 

```
format_interleaving 0
```

0 means interleaved raw data, 1 means deinterleaved data by packets of "frame size"

### format_time_stamp "n"

```
format_time_stamp 0 ; no time-stamp
```

time-stamp format :

- 0: no time stamp
- 1: simple counter
- 2: time difference in second (float)
- 3: time distance from Unix Epoch (double)

### format_domain "n"

Usage context of this command is for the section "B" of above chapter "IO control and stream data formats".
Example `format_domain 2   ; this format uses specific details of audio out domain `

| DOMAIN            | CODE   | COMMENTS                                                     |
| ----------------- | ------ | ------------------------------------------------------------ |
| GENERAL           | 0      | (a)synchronous sensor, electrical, chemical, color, remote data, compressed streams, JSON, SensorThings, application processor |
| AUDIO_IN          | 1      | microphone, line-in, I2S, PDM RX                             |
| AUDIO_OUT         | 2      | line-out, earphone / speaker, PDM TX, I2S,                   |
| GPIO              | 3      | generic digital IO, control of relay, timer ticks            |
| MOTION            | 4      | accelerometer, combined or not with pressure and gyroscope   |
| 2D_IN             | 5      | camera sensor                                                |
| 2D_OUT            | 6      | display, led matrix                                          |
| ANALOG_IN         | 7      | analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage |
| ANALOG_OUT        | 8      | D/A, position piezo, PWM converter                           |
| USER_INTERFACE    | 9      | button, slider, rotary button, LED, digits, display control  |
| PLATFORM SPECIFIC | 10..15 | platform-specific #1..6, used with platform callbacks        |

-----------------------------------------

## Graph: interfaces of the graph

### stream_io_graph "n H" 

This command starts a section for declaring graph IO n.  
The first parameter is the graph IO index used later in the graph.  
The second parameter specifies the physical interface index defined in the platform manifest.

Example 

```
    stream_io_graph 2 18	; the graph IO number 2 is connect ed to the HW 18
```

### stream_io_format "n" 

Parameter: index to the table of formats (default #0)
Example 

```
    stream_io_format 0 
```

### stream_io_setting "W1 W2 W3"

This command specifies the IO settings bit fields.  
These settings are domain-specific and are placed at the beginning of the binary graph.  
They are used during the graph initialization sequence.

See the section “IO Controls Bit-fields per domain” for details.

Example

```
stream_io_setting 7812440 0 ; 0=default restting at reset
```

### stream_io_setting_callback "X" "Y" 

Initializes the IO using a platform callback.  Example 

```
 stream_io_setting_callback 6 7812440 ; Use the platform io_setting callback for the 
;	current stream_io_graph using parameters 6 7812440 
```

-----------------------------------------

## Graph: memory mapping 

This section describes how memory mapping can be split to simplify memory overlays between nodes and arcs by defining new memory-offset indexes.

Another use case is splitting TCM memory between low-latency tasks and other tasks.

The syntax defines an original memory offset ID, a new ID to use in node and arc declarations, a byte offset within the original ID, and the length of the new memory offset.

```
;              original_id  new_id    start   length 
memory_mapping      2        100      1024    32700 
```

### Memory fill

Filling of a word32 pattern after the arc descriptors 


```
    mem_fill_pattern 5 3355AAFF   memory fill 5 word32 value 0x3355AAFF (total 20 Bytes)
```

-----------------------------------------

## Graph: subgraphs 

A subgraph is equivalent to a program subroutine for graphs.  
A subgraph can be reused in multiple places within a graph or in other subgraphs.

The graph compiler creates references using name mangling derived from the call hierarchy.  
A subgraph receives indexes of IO streams and memory bank indexes used for tuning the memory map.

The caller provides the indexes of the arcs to be used in the subgraph, as well as the memory mapping offset indexes.

Example :

```
    subgraph 
       sub1                        ; subgraph name, used for name mangling 
       3 sub_graph_0.txt           ; path and file name 
       5 i16: 0 1 2 3 4            ; 5 streaming interfaces data_in_0, data_out_0 ..  
       3 i16: 0 0 0                ; 3 partitions for fast/slow/working (identical here) 
```

-----------------------------------------

## Graph: nodes declarations

Nodes are declared by their name and their instance index within the graph or subgraph.

The system integrator can select a preset, which is a pre-tuned list of parameters described in the node documentation, and can also provide node-specific parameters to be loaded at boot time.

The address offset of each node is determined during the graph compilation step.

Declaration syntax example.

```
    node arm_stream_filter  0  ; first instance of the nore "arm_stream_filter" 
```

### node_preset "n"

This command allows the system integrator to select one of up to 16 presets when using a node.  
Each preset corresponds to a predefined configuration of the node.

The preset value is applied during the RESET and SET_PARAMETER commands.  
The default value is 0.

Example : 

```
    node_preset              1      ; parameter preset used at boot time 
```

### node_malloc_add "A s"

Adds and extra number of bytes "A" to the "node_mem" segment index "s".
Example : 

```
    node_malloc_add 12 0  ; add 12 bytes to segment 0
```

### node_map_hwblock "m" "o"

This command forces a memory segment index m to be mapped to the memory offset index o, overriding the speed requirement defined in the node manifest.
Example : 

```
    node_map_hwblock 0 2 ; memory segment 0 is mapped to bank offset 2 
```

### node_map_swap "m" "o"

This command optimizes memory mapping by swapping the content of a small, fast memory segment with another memory offset, typically a slower one. 
Usage : 

```
 node_map_swap 1 0 ; swap node memory segment 1 to a memory area of offset 0
```

In both mapping cases, the memory segment content is copied or swapped from the specified offset before execution.  
A dummy arc descriptor is created to access this temporary area.

### node_trace_id "io"

Selection of the graph IO interface used for sending the debug and trace information.
Example : 

```  
    node_trace_id  0      ; IO port 0 is used to send the trace 
```

### node_map_proc, node_map_arch, node_map_thread

The graph can be executed in a multiprocessor and multi tasks platform. Those commands allow the graph interpreter scheduler to skip the nodes not associated to the current processor / architecture and task.
The platform can define 7 architectures and 7 processors. When the parameter is not defined (or with value 0) the scheduler interprets it as "any processor" or "any architecture" can execute this node.
Several OS threads can interpret the graph at the same time. A parameter "0" means any thread can execute this node, and the value "1" is associated to low-latency tasks, "3" to background tasks. 
Examples :

```
node_map_proc 2 	; the node will only be executed on processor 2
```

### node_memory_isolation  "0/1"

Activate (parameter "1") the processor memory protection unit (on code, private memory allocated segments, and stack) during the execution of this node. 
Example : 

```
   node_memory_isolation 1 ; activation of the memory protection unit (MPU), default 0 
```

### node_memory_clear "m"

Debug feature: clear the memory bank "m" before and after the execution of the node.  
Example : 

```
   node_memory_clear 2 ; clear the bank 2 from manifest before and after execution 
```

### node_script "index" 

The indexed script is executed before and after the node execution. The conditional flag is set on the first call and cleared before calling the following calls.
Example :

```
  node_script 12 ; call script #12 associated to this node
```

### node_user_key "k64"

The 64bits key is sent to the node during the reset sequence, It is placed after the memory allocation pointers. 

The node receives the "node_key" from the scheduler and this "user_key" to decide the features to activate.

Example :

```
  node_user_key 101447945804525706  64 bits key
```

### node_parameters "tag"

This command declares parameters to be shared with the node during the RESET sequence.

If the tag parameter is zero, the following parameters represent a full parameter set.  
Otherwise, the tag specifies a subset defined in the node documentation.

The parameter list is terminated by the keyword end.

```
     node_parameters     0                   TAG = "all parameters" 
         1  u8;  2                           Two biquads 
         1  u8;  1                           postShift 
         5 s16; 681   422   681 23853 -15161 elliptic band-pass 1450..1900/16kHz 
         5 s16; 681 -1342   681 26261 -15331 
     end 
```

-----------------------------------------

## Graph Scripts byte codes

Scripts are interpreted byte codes designed for control and calls to the graph scheduler for node control and parameter settings.  
Scripts are declared as standard nodes with additional parameters to define memory size and allow reuse across multiple scripts.  
Script nodes use their transmit arc to hold instance memory, including registers, stack, and heap.

The virtual execution engine provides 20 instructions and up to 10 registers.

Two instruction formats are supported.

- The first format is test and load instructions, which include a test field, a register to test or load, an ALU operation, and ALU operands.  
- The second format is jump and special operations, including calls, scatter and gather loads, and bit-field operations.

### Test instructions

The result of the test is evaluated by adding  a conditional field to any instruction :

```
List of test instructions :

test_equ	     test if equal
test_leq	     test if less or equal
test_lt 	     test if lower
test_neq	     test if non equal
test_geq	     test if great or equal
test_gt 	     test if greater

test the result :
if_yes ...
if_no ...
```

### Arithmetic operations

```
add   		addition of two operands
sub   		substraction
mul   		multiplication
div   		division
or    		logical OR,  FP32 operands are pre-converted to "int"
nor   		logical NOR
and   		logical AND
xor   		logical XOR
shr   		shift right, sign extension applied on "signed" registers
shl   		shift left
set   		set a bit
clr   		clear a bit
max   		compute the maximum of two operands
min   		minimum
amax  		maximum of absolute values
amin  		minmum of absolute values
norm  		normalize to MSB and return the amount of shifts
addmod		addition with modulo defined by "base" and "size"
submod		subtraction with modulo
```

The 10 registers of the virtual machine are "r0" .. "r9". Using "sp0" (or simply "sp") means an access to the data located at the stack pointer position, "sp1" tells to increment the stack pointer **SP** after a write to the stack and to decrement it after a read. In case several stack accesses are made in the same instruction the update of the stack pointer are made when reading the instructions from right to left.

For example : ` sp1 = add sp1 #float 3.14 `  : the literal constant "3.14" is added to the data on top of the stack and SP is post-decremented after the read ("pop" operation), the result of the addition is saved on the stack ("push") with SP post-incremented. ` sp1 = add sp0 sp1 ` pops the stack adds the next stack value (without SP decrement) and the result is pushed.

```
r6 = 3                    r6 = 3  (the default litterals type is int32)
r11 = sp                  read data from the top of the stack 
r6 = add r5 3             r6 = ( r5 + 3 )
sp1 = r6                  push the result on stack (SP incremented)
test_eq r6 sub r5 r4      test if r6 == ( r5 - r4 )
if_yes r6 = add r5 3      conditional addition of r5 with 3 saved in r6
```

Literal constants are signed integers by default, if other data types are needed the constant is preceded by "#float" or "uint8", for example :

```
r3 = 3.14159              load PI in r3
r4 = mul r3 12.0          floating-point multiplication saved in r4
```

Other instructions examples :

```
swap r2 sp1               swap r2 with the top of the stack, pop it
label L1                  label declaration
if_not call L1            conditional call 
banz L1 r2                decrement r2 and branch if not zero
jump L1 r1                jump to label and push up to 2 registers 
call L1 r2 r3             call a subroutine and push 2 registers
set r4 #uint32            cast r4 as an unsigned integer
set r4 #heap L2           load r4 with an address in the heap RAM
set r4 base L2            set the base address of a circular buffer
set r4 size 12            set the size of a circular buffer
set r0 graph node_name_2  set r0 with the graph's node instance #2
save r4 r5 r0 r2 r11      push 5 registers on the stack
restore r4 r5             pop 2 registers from the stack
delete 4                  remove the last 4 registers from the stack
[ r4  12 ]+ =  r5         scatter load with pre-increment
r3 = [ r4 ]  r0           gather load 
r3 | 8 15 | = r2          bit-field load of r2 to the 2nd byte of r3
r3 = r2 | 0 7 |           bit-field extract the LSB of r2 to r3
return                    return from subroutine or script
Syscall 1 r1 r4 r5        system call (below)
```

### Graph syntax

```
script 1  			      ; script (instance) index           
    script_name    TEST1  ; for reference in the GUI
    script_stack      12  ; size of the stack in word64      
    script_mem_shared  1  ; default is private memory (0) or shared (1)  
 
    script_code       
    ...
    return                ; return to the graph scheduler
    script_parameters 0   ;
        1  u8 ;  34       ; data section following the code 
        label BBB         ; label to the the second byte
        2  u32; 0x33333333 0x444444444 ;
        label CCC        
        1  u8 ;  0x55     ; second label address
        1  u32;  0x66666666 ;

    script_heap			  ; heap RAM section (arc buffer)
        1  u8 ;  0        ; RAM               
        label DDD              
        4  u32;  0 0 0 0  ; label DDD points to a byte address
        label EEE              
        1  u8 ;  0        ; heap is initialized at node reset
    end                                                      
```

### System calls

The `"Syscall"` instruction gives access to nodes (set/read parameters) and arc (read/write data). It allows the access to other system information: 

- FIFO content (read/write), filling status and access to the arc debug information (last time-stamp access, average of samples, etc ..)
- Node parameters read and update, with / without a reset of the node
- Basic compute and data move functions
- The call-backs provided by the application (use-case, change the graph IO parameters, debug and trace)

#### Syscall syntax

Syscall instructions have five parameters 

```
syscall index command param1 param2 param3
```



| Syscall index                    | register parameters                                          |
| -------------------------------- | ------------------------------------------------------------ |
| 1 (access to nodes)              | R1: command (set/read parameter)<br/>R2: address of the node<br/>R3: address of data<br/>R4: number of bytes |
| 2 (access to arcs)               | R1: command  set/read data=8/9<br/>R2: arc's ID<br/>R3: address of data<br/>R4: number of bytes |
| 3 (callbacks of the application) | R1: application_callback's ID<br/>R2: parameter1 (depends on CB)<br/>R3: parameter2 (depends on CB)<br/>R4: parameter3 (depends on CB) |
| 4 (IO settings)                  | R1: command <br/>    set/read parameter=2/3<br/>R2: IO's graph index<br/>R3: address of data<br/>R4: number of bytes |
| 5 (debug and trace)              | TBD                                                          |
| 6 (computation)                  | TBD                                                          |
| 7 (low-level functions)          | TBD, peek/poke directly to memory, direct access to IOs (I2C driver, GPIO setting, interrupts generation and settings) |
| 8 (idle controls)                | TBD, Share to the application the recommended Idle strategy to apply (small or deep-sleep). |
| 9 (time)                         | R1: command and time format <br/>R2: parameter1 (depends on CB)<br/>R3: parameter2 (depends on CB)<br/>R4: parameter3 (depends on CB) |
| 10 (Script)                      | R1: command  (call before/after node execution)<br/>R2: infomation1 (node offset) |
|                                  |                                                              |
|                                  |                                                              |



------

## GUI design tool

The compiled binary graph can be generated with graphical tool (prototyped in "stream_tools\gui"). The tool creates the compiled binary format and the intermediate text file for later manual tuning.

![c](GUI_PoC.png)

-----

## Arcs of the graph

The syntax differs for arcs connected to the boundary of the graph and for arcs placed between two nodes.

Depending on real-time behavior such as CPU load, jitter, task priorities, and stream speed, data can be processed in place or copied into a temporary FIFO before processing.

The parameter set0copy1 is set to 0 by default for in-place processing.  
In this case, the base address of the arc FIFO descriptor is modified during the transfer acknowledgment subroutine arm_graph_interpreter_io_ack() to point directly to the IO data, and no data copy is performed.

When the parameter is set to 1, the data is copied into the arc FIFO.  
The graph compiler allocates a memory buffer sized according to the frame length.

Example :

```
; Syntax :
; arc_input   { io / set0copy1 / fmtProd } + { node / inst / arc / fmtCons }
; arc_output  { io / set0copy1 / fmtCons } + { node / inst / arc / fmtProd }
; arc  { node1 / inst / arc / fmtProd } + { node2 / inst / arc / fmtCons }

 arc_input 4 1 0    xxfilter 6 0 8   ; IO-4 sends data to the node xxfilter                            
                                                                      
; output arc from node xxdetector instance 5 output #1 using format #2 
;            to graph IO 7 using set0copy1=0 and format #9             
 arc_output 5 1 2   xxdetector 5 1 2                                   
                                                                      
;  arc between nodeAAA instance 1 output #2 using format #0             
;      and nodeBBB instance 3 output #4 using format #1                 
 arc nodeAAA 1 2 0   nodeBBB 3 4 1                                    
```

### arc_input 

A declaration of a graph input gives the name of the index of the stream which is the "producer" and the node it is connected to (the "consumer").

- index of the IO (see [stream_io](#stream_io))
- 0 or 1 to indicate the data is consumed "in-place" (parameter =0), or will be copied in the buffer associated to the arc (parameter =1). When the data is processed in-place the graph declares an arc descriptor without buffer, the function `void arm_stream_io_ack()` will copy the address of the data in the base address of the arc descriptor
- format ID (see [format "n"](#format-"n"))  used to produce the stream
- name of the consumer node 
- Instance index of the node, starting from 0
- arc index of the node (see [node_arc "n"](#node-arc="n"))
- format ID (see [format "n"](#format-"n"))  used to by the node consumer of the stream
- optional information to tell this arc is managed with "high quality of service" (HQoS) : the node consuming the stream will treat the corresponding processing with the highest priority whatever the content of the other arc connected to this node. This consumer node will arrange with data interpolations to let the HQoS stream be processed first with the lowest latency.

Example

``` 
arc_input   1 0 3 arm_stream_filter   4 0 6 
; 1 input stream from io 1
; 0 set the pointer to IO buffer without copy
; 3 third format used 
; arm_stream_filter receives the data
; 4 fifth instance of the node in the graph
; 0 arc index of the node connected to the stream (node input)
; 6 stream is consumed using the seventh format
```

### arc_output 

A declaration of a graph output gives the name of the index of the stream which is the "consumer" and the node it is connected to (the "producer").

Example

```
arc_output   1 1 3 arm_stream_filter   4 1 0
; 1 output stream from io 1
; 1 copy the data from the arc buffer 
; 3 third format used 
; arm_stream_filter produces the data
; 4 fifth instance of the node in the graph
; 1 arc index of the node connected to the stream (node output)
; 0 stream is generated using the first format
```

### arc_nodes node1 - node2

Declaration of an arc between two nodes.

Example

``` 
arc_nodes arm_stream_filter 4 1 0 sigp_stream_detector 0 0 1   H
; arm_stream_filter produces the data to the sigp_stream_detector
; 4 fifth instance of the node in the graph
; 1 arc index of the node connected to the stream (node output)
; 0 stream is generated using the first format
; sigp_stream_detector consumes the data
; 0 first instance of the node in the graph
; 0 arc index of the node connected to the stream (node input)
; 1 stream is consumed using the second format
; 'H' tells to process the stream with priority 
```

### arc flow control RD WR

Flow error management with arc descriptor bits bits FLOW_RD_ARCW2 / FLOW_WR_ARCW2, to let an arc stay with 25% .. 75% of data. Process done in "router" node when using HQOS arc and IO master interfaces

The arc is initialized with 50% of null data. The processing is frame-based, there are minimum 3 frames in the buffer.

When a IO-master writes in an arc with FLOW_WR_ARCW2=1 and the arc is full at +75%, the new data 
    is extrapolated and the arc stays at 75% full 

```
  buffer full after NewData was push by the IO-master
  Buff  xxxxxxxx|xxxx|xxxx|xxxx|bbbb|aaaa|  
          R_ptr                      W_ptr
The previous frame (bbb) is filled (bbb x win_rampDown) + (newData_aaa x win_rampUp) W_ptr steps back 
        xxxxxxxx|xxxx|xxxx|xxxx|bbaa|----|  buffer full
          R_ptr                      W_ptr
```

When a IO-master read from an arc with FLOW_RD_ARCW2=1 and the arc is empty at -25%, the new data 
    is extrapolated and the arc stays at 25% empty

```
  Buff  |bbbb| is read by the IO-master
  buffer hold only ONE frame |aaaa| after the previous read
        |bbbb|aaaa|----|  
              R_ptr W_ptr
  The previous frame (bbb) is filled (aaa x win_rampDown) + (bbb x win_rampUp) and R_ptr steps back
        |aabb|aaaa|----|  bbbb
         R_ptr      W_ptr
```

Activation of the error flow management bits on read and write access:

``` 
arc_flow_error  1 1 ; read write  
```

### arc debug

Each arc descriptor can be configured to have an operation (in a list of 32) implemented with result returned in a dedicated memory section of the graph.

| CODE | DEBUG OPERATION                                              |
| ---- | :----------------------------------------------------------- |
| 0    | no operation                                                 |
| 1    | increment DEBUG_REG_ARCW1 with the number of RAW samples     |
| 2    | set a 0 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to clear |
| 3    | set a 1 in to *DEBUG_REG_ARCW1, 5 MSB gives the bit to set   |
| 4    | increment *DEBUG_REG_ARCW1                                   |
| 5    |                                                              |
| 6    | call-back in the application side, data rate estimate in DEBUG_REG_ARCW1 |
| 7    | second call-back : wake-up processor from DEBUG_REG_ARCW1=[ProcID, command] |
| 8    |                                                              |
| 9    | time_stamp_last_access                                       |
| 10   | peak with forgeting factor 1/256 in DEBUG_REG_ARCW1          |
| 11   | mean with forgeting factor 1/256 in DEBUG_REG_ARCW1          |
| 12   | min with forgeting factor 1/256 in DEBUG_REG_ARCW1           |
| 13   | absmin with forgeting factor 1/256 in DEBUG_REG_ARCW1        |
| 14   | when data is changing the new data is push to another arc DEBUG_REG_ARCW1=[ArcID] |
| 15   | automatic rewind read/write                                  |

Example :

```
    arc_debug_cmd  1  debug action "ARC_INCREMENT_REG"         
    arc_debug_reg  3  index of the 64bits result, default = #0  
```

### arc_memory_alignment "n"

Memory alignment to cache lines width of n=32bytes or n=64bytes

```
	arc_memory_alignment 64		; 
```

### arc_flush

```
    arc_flush  0 ; forced flush of data in MProcessing and shared tasks
```

### arc_map_hwblock

```
     arc_map_hwblock   0  map the buffer to a memory offset, default = #0 (VID0)
```

### arc_jitter_ctrl

Command used during the compilation step for the FIFO buffer memory allocation with some margin.

Without this jitter added margin , the buffer size is the largest frame size value between producer and consumer of the arc.

```
    arc_jitter_ctrl  1.5  ; factor to apply to the minimum size between the producer and the consumer, default = 1.0 (no jitter) 
```

### arc_parameters

Arcs data structures are used to allocate large memory used as node parameters. This is done during the node declaration. The node manifest declares this "arc is used as parameter area" for large parameters (NN model, video file, etc ..). There is no data producers for this type of arc.

```
    arc_parameters  0       ; (parameter arcs) buffer preloading, or arc descriptor set with script 
       7  i8; 2 3 4 5 6 7 8 ; parameters                                                            
    include 1 filter_parameters.txt ; path + text file-name using parameter syntax                    
    end                                                                                              
```


-------------------------

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



