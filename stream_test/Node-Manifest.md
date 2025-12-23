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



# Node manifest

A node manifest file provides the name of the software component, the author, the targeted architecture, and the description of the input and output streams connected to it.

The graph compiler allocates a predefined amount of memory, and this file explains how to compute the memory allocation.  See Declaration of options for the option syntax.

## Example of node manifest

```
; --------------------------------------------------------------------------------------
; SOFTWARE COMPONENT MANIFEST - "stream_filter"
; --------------------------------------------------------------------------------------
;
node_developer_name    ARM          ; developer name
node_name             stream_filter ; node name

node_mask_library      64           ; dependency with DSP services

;----------------------------------------------------------------------------------------
;   MEMORY ALLOCATIONS

node_mem                0		    ; first memory bank (node instance)
node_mem_alloc         76			; amount of bytes

node_mem                1			; second memory bank (node fast working area)
node_mem_alloc         52           ;
node_mem_type           1           ; working memory
node_mem_speed          2           ; critical fast 
;---------------------------------------------------------------------------------------
;    ARCS CONFIGURATION
node_arc                0
node_arc_nb_channels    {1 1 2}     ; arc intleaved,  options for the number of channels
node_arc_raw_format     {1 3 1}   ; options for the raw arithmetics STREAM_S16, STREAM_FP32

node_arc                1
node_arc_nb_channels    {1 1 2}     ; options for the number of channels
node_arc_raw_format     {1 3 1}   ; options for the raw arithmetics STREAM_S16, STREAM_FP32

end
```

The nodes have the same interface : 

```
void (node name) (uint32_t command, void *instance, void *data, uint32_t *state);
```

Nodes are called with the parameter “data” containing a table of arc data structures with two fields.

 - a pointers the arc buffer
 - the  second field is the number of bytes placed after that address for input arcs, or the free space available for output arcs.

The nodes returns after updating the second field of the structures :

 - The amount of data consumed, for RX arcs
 - The amount of data produced in the TX arcs

## Default values of a node manifest 

When not mentioned in the manifest the following assumptions are :

| io manifest command    | default value | comments                          |
| ---------------------- | ------------- | --------------------------------- |
| io_commander0_servant1 | 1             | servant                           |
| io_set0copy1           | 0             | in-place processing               |
| io_direction_rx0tx1    | 0             | data flow to the graph            |
| io_raw_format          | float32       | float is the default data format  |
| io_interleaving        | 0             | raw data interleaving             |
| io_nb_channels         | 1             | mono                              |
| io_frame_length        | 1             | one sample (mono or multichannel) |

## Manifest header

The manifest starts with the identification of the node.

### node_developer_name "name"

Name of the developer/company having the legal owner of this node.
Example: 

```
    node_developer_name CompanyA & Sons Ltd  
```

### node_name "name"

Name of the node when using a graphical design environment.
Example: 

```
    node_name arm_stream_filter 
```

### node_logo "file name"

Name of the graphical logo file (file path of the manifest) of the node when using a graphical design environment.
Example: 

```
    node_logo arm_stream_filter.gif 
```

### node_nb_arcs "in" "out"

Number of input and output arcs of the node used for data streaming. 
Example 

```
    node_nb_arcs 1 1 ; nb arc input, output, default values "1 1"
```

### node_mask_library    "n" 

The graph interpreter provides a short list of optimized DSP, ML, and math functions for the platform, using dedicated vector instructions and coprocessors.  
Some platforms may not include all libraries.  
The node_mask_library parameter is a bit field that specifies dependencies on these library services.  
This information is particularly useful when the node is delivered in binary format.

Bit definitions are as follows.

Bit 2 corresponds to the standard library (string.h, malloc).  
Bit 3 corresponds to math functions (trigonometry, random number generation, time processing).  
Bit 4 corresponds to DSP and ML functions (filtering, FFT, 2D convolution for 8-bit data).  
Bit 5 corresponds to 8-bit linear algebra for neural networks.  
Bit 6 corresponds to audio codecs.  
Bit 7 corresponds to image and video codecs.

Example :

```
node_mask_library  64  ; the node has a dependency to DSP/ML computing services
```

### node_architecture  "name" 

Create a dependency of the execution of the node to a specific processor architecture. For example when the code is incorporating in-line assembly of a specific architecture.

Example:

```
   node_architecture    armv6m  ; a node made for Cortex-M0
```


### node_fpu_used "option" (TBD)

The command creates a dependency on the FPU capabilities
Example :

```
   node_fpu_used   0  ; fpu option used (default 0: none, no FPU assembly or intrinsic) 
```

### node_version    "n"

For information, the version of the node
Example :

```
 node_version    101            ; version of the computing node
```

### node_complexity_index    "n"

This provides information for debugging and watchdog configuration.  
The parameters specify the maximum number of cycles for node initialization and for processing a frame.
Example :

```
 node_complexity_index 1e5 1e6 ; maximum number of cycles at initialization time and execution of a frame
```

### node_dynamic_malloc

This command indicates that node memory allocation is not precomputed during graph compilation but is performed dynamically during the RESET sequence of the node.  
During reset, the scheduler first requests the number of bytes required per memory segment, as defined by the node_mem index.  
It then allocates the memory and restarts the reset sequence with pointers to the allocated physical memory.
Example :

```
 node_dynamic_malloc ; the node returns at reset time the amount of bytes per memory segment
```

### node_not_reentrant  

This indicates that only a single instance of the node can be scheduled in the graph.
Example :

```
 node_not_reentrant ; one single instance of the node can be scheduled in the graph
```

### node_stream_version  "n"            

Version of the stream scheduler it is compatible with.
Example :

```
  node_stream_version    101
```

 

### node_logo  "file name" 

File name of the node logo picture (JPG/GIF format) to use in the GUI.

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

## Configuration of the arcs attached to the node

The arc configuration specifies the list of compatible options available for node processing.  
Some options are described as lists, while others are described as ranges of values.

The syntax consists of an index followed by a list of numbers enclosed in braces “{” and “}”.  
The index specifies the default value to select from the list.  
An index value of 1 corresponds to the first element of the list.  
An index value of 0 means “any value”.  
In this case, the list may be empty.

Example of an option list with five values, where the index is 2, meaning the default value is the second element of the list (value = 6):

```
   { 2  5 6 7 8 9 }
```

When the index is negative the list is decoded as a "range". A Range is a set of three numbers : 

- the first option
- the step to the next possible option
- the last (included) option

The absolute index value selects the default value in this range.

Example of is an option list of values (1, 1.2, 1.4, 1.6, 1.8, .. , 4.2), the index is -3 meaning the default value is the third in the list (value = 1.4).

```
  { -3  1  0.2  4.2 } 
```

### node_arc "n"

This command starts the declaration of a new arc, followed by its index, which is used when connecting two nodes.
Example :

```
   node_arc 2    ; start the declaration of a new arc with index 2
```

Implementation comment : all nodes have at least one arc on the transmit side, which is used to manage the node’s locking field.

### node_arc_name "name"

Name of the arc used in the GUI.
Example: 

```
    node_arc_name filter_output  ; "filter_output" is the name of the arc
```

### node_arc_rx0tx1 "0/1"

Declares the direction of the arc from the node point of view : "0" means a stream is received  through this arc, "1" means the arc is used to push a stream of procesed data.

```
    node_arc_rx0tx1             0               ; followed by 0:input 1:output, default = 0 and 1
```

### node_arc_interleaving  "0/1"

Arc data stream interleaving scheme: "0" for no interleaving (independent data frames per channel), "1" for data interleaving at raw-samples levels.
Example :

```
    node_arc_interleaving 0     data is deinterleaved on this arc
```

### node_arc_nb_channels  "n"

Number of the channels possible for this arc (default is 1).
Example :

```
    node_arc_nb_channels {1 1 2}  ; options for the number of channels is mono or stereo
```

### node_arc_units_scale "unit" "scale"

Command used when the node needs the streams to be rescaled to absolute scaled units (See paragraph "Units" of [Units](#Units)).

```
    node_arc_units_scale VRMS  0.15  ; full-scale is equivalent to 0.15 VRMS
```

### node_arc_units_scale_multiple "unit" "scale"

Command used when the node needs the streams to be rescaled to absolute scaled units and there are multiple units in sequence (See paragraph "Units" of [Units](#Units)).

```
    node_arc_units_scale_multiple DPS 360 GAUSS 0.002 
    ; interleaved format with maximum 360 dps and 0.002 Gauss
```

### node_arc_raw_format "f"

Raw samples data format for read/write and arithmetic's operations. The stream in the "2D domain" are defining other sub-format 
Example :

```
    node_arc_raw_format {1 3 1} raw format options: STREAM_S16, STREAM_FP32, default values S16
```

### node_arc_frame_samples "n"

Frame size options in samples,

```
 node_arc_frame_samples  {-1 1 1 160} ; options of possible frame_size in number of sample (which can mono or multi-channel) 
```

### node_arc_frame_duration "t"

Duration of the frame in milliseconds. The translation to frame length in Bytes is made during the compilation of the graph from the sampling-rate and the number of channels. 
A value "0" means "any duration" which is the default.
Example :

```
node_arc_frame_duration {1 10 22.5}  frame of 10ms (default) or 22.5ms
```

### node_arc_sampling_rate "fs"

Declaration of the allowed options for the node_arc_sampling_rate in Hertz.
Example :

```
node_arc_sampling_rate {1 16000 44100} ; sampling rate options, 16kHz is the default value if not specified 
```

### node_arc_sampling_period_s  "T"

This command declares the duration of the frame in seconds.  
The translation to frame length in bytes is performed during graph compilation, based on the sampling rate and the number of channels. A value of 0 means “any duration” and is the default.
Example :

```
node_arc_sampling_period_s {-2 0.1 0.1 1}  frame sampling going from 100ms to 1000ms, with default 200ms
```

### node_arc_sampling_period_day "D"

This command declares the duration of the frame in days.  
The translation to frame length in bytes is performed during graph compilation, based on the sampling rate and the number of channels. A value of 0 means “any duration” and is the default.
Example :

```
node_arc_sampling_period_day {-2 1 1 30}  frame sampling going from 1 day to 1 month with steps of 1 day.
```

### node_arc_sampling_accuracy  "p"

When a node does not require rate-accurate input data, this command allows some flexibility in the sampling rate without requiring insertion of a synchronous rate converter.  
The parameter is expressed as a percentage.
Example :

```
node_arc_sampling_accuracy  0.1  ; sampling rate accuracy is 0.1%
```


### node_arc_inPlaceProcessing  "in out"

This command enables memory optimization through arc buffer overlay.  
It specifies that the input arc index “in” is overlaid with the output arc index “out”.  
By default, separate memory is allocated for input and output arcs.  
In this case, the arc descriptors remain distinct, but the base address of the buffers is identical.
Example :

```
node_arc_inPlaceProcessing  1 2   ; in-place processing can be made between arc 1 and 2
```

--------------------------------------

# Node design

All programs can be used with the Graph scheduler, also called computing “nodes”, as long as a minimal description is provided in a manifest and the program can be invoked through a single entry point using a wrapper with the following prototype.

```
void (node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);
```

The command parameter indicates operations such as reset, run, or set parameters.  
The instance parameter provides opaque access to the static area of the node.  
The data parameter is a table of pointer-and-size pairs for all arcs used by the node.  
The state parameter returns information about the completion of the computation for the data exchanged through the arcs.

During the reset sequence of the graph, nodes are initialized.  
Nothing prevents a node from calling standard library functions for memory allocation or mathematical computation.  
However, the context of the graph interpreter is embedded IoT, with strong optimization constraints for cost and power consumption.

--------------------------------------

## General recommendations

General programming guidelines for nodes are as follows.

- Nodes must be callable from C, or from any other language that respects the EABI.  
- Nodes are reentrant, and if not, this must be explicitly stated in the manifest.  
- Data is treated as little-endian by default.  
- Data references are relocatable, and there must be no hard-coded data memory locations.  
- All node code must be fully relocatable, and there must be no hard-coded program memory locations.  
- Nodes are independent of any particular I/O peripheral, and there must be no hard-coded addresses.  
- Nodes are characterized by their memory and, when possible, by their MIPS requirements with respect to the buffer length to process.  
- Nodes must specify whether they are ROM-able, meaning whether self-modifying code is used, unless explicitly documented otherwise.  
- Run-time object creation should be avoided, and memory reservation should be performed once during the initialization step.  
- Nodes manage buffers using pointers with physical addresses shared through parameters.  
- Processors have no MMU, so there is no means of mapping physically non-contiguous segments into a contiguous block.  
- Cache coherency is managed by Graph-Interpreter at transitions from one node to the next.  
- Nodes should not use stack-allocated buffers as the source or destination of any graph services for memory transfer.  
- Static and persistent data, retention data used for warm boot, scratch data, stack usage, and heap usage should be documented.  
- The manifest details the memory allocation section with respect to latency and speed requirements.



## Node parameters

A node is using the following prototype 

```
void (node) (uint32_t command, void *instance, void *data, uint32_t *state);
```

With following parameters: 

| Parameter name | Details         | Types                                                        |
| :------------- | :-------------- | ------------------------------------------------------------ |
| command        | input parameter | uint32_t                                                     |
| instance       | instance        | void * casted to the node type                               |
| data           | input data      | casted pointer to struct stream_xdmbuffer { int address;  int size; } |
| state          | returned state  | uint32_t *                                                   |

### Command parameter

Command bit-fields :

| Bit-fields | Name     | Details                                                      |
| :--------- | :------- | :----------------------------------------------------------- |
| 31-24      | reserved |                                                              |
| 16-23      | node tag | Depending on the command, the node tag specifies the index of the parameter to update from the data address.  <br/>If the value is 0, all parameters are prepared in the data block. |
| 15-12      | preset   | A node can define up to 16 presets, each corresponding to a preconfigured set of parameters. |
| 11-5       | reserved |                                                              |
| 4          | extended | When set to 1 for a reset command with warm boot, static areas are initialized except for memory segments assigned to retention memory in the manifest.  <br/>When the processor has no retention memory, those static areas are cleared by the scheduler. |
| 3-0 (LSB)  | command  | A value of 1 corresponds to reset.  <br/>A value of 2 corresponds to set parameter.  <br/>A value of 3 corresponds to read parameter.  <br/>A value of 4 corresponds to run.  <br/>A value of 5 corresponds to stop.  <br/>A value of 6 corresponds to updating the physical address of a relocatable memory segment. |

### Instance

The instance parameter is an opaque memory pointer to the main static area of the node.  
The memory alignment requirement is specified in the node manifest.

### Data

The multichannel data parameter is a pointer to arc data.  
It points to a list of structures composed of two INTPTR_T values, which are 32-bit or 64-bit wide depending on the processor architecture.  
The first value is a pointer to the data.  
The second value specifies the number of bytes to process for an input arc or the number of bytes available in the buffer for an output arc.

A node can have up to 16 arcs.  
Each arc can have its own format, including number of channels, frame length, interleaving scheme, raw sample type, sampling rate, and time stamps.  
Arcs can also be used for purposes other than data streaming, such as parameter storage.

### Status

Nodes return a state value of 0 unless data processing is not finished.  
In that case, the returned state value is 1.



## Node calling sequence

Nodes are first called with the reset command, followed by set parameter, then several run commands, and finally stop to release memory.  
This section details the content of the node parameters during the reset, set parameter, and run phases.

```
void (node) (uint32_t command, void *instance, void *data, uint32_t *state); 
```

### Reset command

Each node is identified by a 10-bit index and a synchronization byte.  
The synchronization byte contains 3 bits defining the target architecture (values 1 to 7, where 0 means any architecture),  
3 bits defining the processor index within this architecture (values 1 to 7, where 0 means any processor),  
and 2 bits defining the thread instance.  
A value of 0 means any thread, while values 1, 2, and 3 correspond respectively to low-latency, normal-latency, and background tasks.

At reset time, processor 1 of architecture 1 is allowed to copy the graph from flash to RAM and unlock the other processors.

Each processor then parses the graph, identifies the nodes assigned to it, resets them, and updates their parameters from the graph data.  
When all nodes have been initialized, the application is notified and the graph switches to run mode.

Each graph scheduler instance ensures that input and output streams are not blocked.  
Each IO is associated with a processor, and in most cases a single processor manages all IOs.

Multiprocessor synchronization mechanisms are abstracted outside the graph interpreter, in the platform abstraction layer.  
A software-based lock is provided by default.

The second parameter, instance, is a pointer to the list of memory banks reserved by the scheduler for the node,  
in the same sequence order as the declarations in the node manifest.  
The first element of the list is the node instance, followed by pointers to data or program memory allocations.

The third parameter, data, is used to share the address of the function providing computing services.

### Set Parameter command 

The node tag bit field specifies which parameter or parameters are updated.

The third parameter, data, is a pointer to the new parameter values.

### Run command

The node tag bit field specifies which parameter or parameters are updated.

The third parameter, data, is a pointer to the list of buffers,  
represented as structures containing an address and a size,  
associated with each arc connected to the node.



## Test-bench and non-regression test-patterns

Nodes are delivered with a test-bench (code and non-regression database).

## Node example 

TBC

    typedef struct
    {   q15_t coefs[MAX_NB_BIQUAD_Q15*6];
        q15_t state[MAX_NB_BIQUAD_Q15*4];    
     } arm_filter_memory;
    
    typedef struct
    {   arm_filter_memory *TCM;
    } arm_filter_instance;
    	
    void arm_stream_filter (int32_t command, void *instance, void *data, uint32_t *status)
    {
    	*status = NODE_TASKS_COMPLETED;    /* default return status, unless processing is not finished */
    
    	switch (RD(command,COMMAND_CMD))
    	{ 
        /* func(command = (STREAM_RESET, COLD, PRESET, TRACEID tag, NB ARCS IN/OUT)
                instance = memory_results and all memory banks following
                data = address of Stream function
                
                memresults are followed by 4 words of STREAM_FORMAT_SIZE_W32 of all the arcs 
                memory pointers are in the same order as described in the NODE manifest
    
                memresult[0] : instance of the component
                memresult[1] : pointer to the allocated memory (biquad states and coefs)
    
                memresult[2] : input arc Word 0 SIZSFTRAW_FMT0 (frame size..)
                memresult[ ] : input arc Word 1 SAMPINGNCHANM1_FMT1 
                ..
                memresult[ ] : output arc Word 0 SIZSFTRAW_FMT0 
                memresult[ ] : output arc Word 1 SAMPINGNCHANM1_FMT1 
    
                preset (8bits) : number of biquads in cascade, max = 4, from NODE manifest 
                tag (8bits)  : unused
        */
        case STREAM_RESET: 
        {   
            uint8_t *pt8b, i, n;
            intPtr_t *memreq;
            arm_filter_instance *pinstance;
            uint8_t preset = RD(command, PRESET_CMD);
            uint16_t *pt16dst;
    
            /* read memory banks */
            memreq = (intPtr_t *)instance;
            pinstance = (arm_filter_instance *) (*memreq++);        /* main instance */
            pinstance->TCM = (arm_filter_memory *) (*memreq);       /* second bank = fast memory */
    
            /* here reset */
            pt8b = (uint8_t *) (pinstance->TCM->state);
            n = sizeof(pinstance->TCM->state);
            for (i = 0; i < n; i++) { pt8b[i] = 0; }
    
            /* load presets */
            pt16dst = (uint16_t *)(&(pinstance->TCM->coefs[0]));
            switch (preset)
            {   default: 
                case 0:     /* by-pass*/
                    pt16dst[0] = 0x7FFF;
                    break;
            }
            break;
        }       
    
        /* func(command = bitfield (STREAM_SET_PARAMETER, PRESET, TAG, NB ARCS IN/OUT)
                    TAG of a parameter to set, NODE_ALL_PARAM means "set all the parameters" in a raw
                *instance, 
                data = (one or all)
        */ 
        case STREAM_SET_PARAMETER:   
        {   uint8_t *pt8bsrc, i, numStages;
            uint16_t *pt16src, *pt16dst;
            int8_t postShift;
            arm_filter_instance *pinstance = (arm_filter_instance *) instance;
    
            pt8bsrc = (uint8_t *) data;
            numStages = (*pt8bsrc++);
            postShift = (*pt8bsrc++);
    
            pt16src = (uint16_t *)pt8bsrc;
            pt16dst = (uint16_t *)(&(pinstance->TCM->coefs[0]));
            for (i = 0; i < numStages; i++)
            {   /* format:  {b10, 0, b11, b12, a11, a12, b20, 0, b21, b22, a21, a22, ...} */
                *pt16dst++ = *pt16src++;    // b10
                *pt16dst++ = 0;             // 0
                *pt16dst++ = *pt16src++;    // b11    
                *pt16dst++ = *pt16src++;    // b12
                *pt16dst++ = *pt16src++;    // a11
                *pt16dst++ = *pt16src++;    // a12
            }
    
            stream_filter_arm_biquad_cascade_df1_init_q15(
                &(pinstance->TCM->biquad_casd_df1_inst_q15),
                numStages,
                (const q15_t *)&(pinstance->TCM->coefs[0]),
                (q15_t *)&(pinstance->TCM->state),
                postShift);
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
            arm_filter_instance *pinstance = (arm_filter_instance *) instance;
            intPtr_t nb_data, stream_xdmbuffer_size;
            stream_xdmbuffer_t *pt_pt;
            int16_t *inBuf, *outBuf;


            pt_pt = data;   inBuf = (int16_t *)pt_pt->address;   
            stream_xdmbuffer_size = pt_pt->size;  /* data amount in the input buffer */
            pt_pt++;        outBuf = (int16_t *)(pt_pt->address); 
            nb_data = stream_xdmbuffer_size / sizeof(int16_t);
            
            /* data processing here  
                ..
            */
    
            /* update the data consumption/production */
            pt_pt = data;
            *(&(pt_pt->size)) = nb_data * sizeof(SAMP_IN); /* amount of data consumed */
            pt_pt ++;
            *(&(pt_pt->size)) = 1 * sizeof(SAMP_OUT);   /* amount of data produced */
            break;
        }
    
        case STREAM_STOP:
        case STREAM_READ_PARAMETER:
        case STREAM_UPDATE_RELOCATABLE:
        default : break;
    }


## Conformance checks

The purpose of conformance checks is to create an automatic process for incorporating new nodes into a large repository and to provide a scalable means of verifying conformance.

- The checks include verification of conformance to the APIs.  
- They include injection of typical and non-typical data aligned with the node description.  
- They include verification of outbound parameter behavior.  
- They include verification of stack consumption and detection of memory leakage.

## Services provided to the nodes 

The "service" function has the following prototype

```
typedef void    (services) (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n); 
```

Service command bit-fields :

| Bit-fields | Name     | Details                                                      |
| :--------- | :------- | :----------------------------------------------------------- |
| 31-28      | control  | set/init/run w/wo wait completion, in case of coprocessor usag |
| 27-24      | options  | compute accuracy, in-place processing, frame size            |
| 23-4       | function | Operation/function within the Group                          |
| 3-0 (LSB)  | Group    | index to the groups of services : <br/SERV_INTERNAL     1 <br/>SERV_STDLIB       2 : extract of string and stdlib.h (atof, memset, strstr, malloc..)<br/>SERV_MATH         3 : extract of math.h (srand, sin, tan, sqrt, log..)<br/>SERV_DSP_ML       4 : filtering, spectrum fixed point and integer<br/>SERV_DEEPL        5 : fully-connected and convolutional network <br/>SERV_MM_AUDIO     6 : audio codecs (TBD)<br/>SERV_MM_IMAGE     7 : image processing (TBD) |



