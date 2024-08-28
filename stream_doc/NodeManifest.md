[TOC]
--------------------------------------
# Node manifest template
A node manifest file gives the name of the software component, its author, the targetted architecture, the description of input and output streams connected to it.
The graph interpreter allocates a predefined amount of memory and this file explains the way to compute the memory allocation with respect to stream characteristics.
When the first non-blank character is a semi-colon ";" the entire line is skipped.

## Manifest header
The manifest starts with the identification of the node.

### node_developer_name "name"
Name of the developer/company having the legal owner of this node.
Example: 
`   node_developer_name CompanyA & Sons Ltd  `

### node_name "name"
Name of the node when using a graphical design environment.
Example: 
`   node_name arm_stream_filter `

### node_logo "file name"
Name of the graphical logo file (file path of the manifest) of the node when using a graphical design environment.
Example: 
`   node_logo arm_stream_filter.gif `

### node_nb_arcs "in out"
Number of input and output arcs of the node used for data streaming. 
Example 
`   node_nb_arcs 1 1 ; nb arc input, output, default values "1 1" `

### node_arc_parameter "n"
When the amount of parameters exceeds 256kB (deep-learning models, video file, etc..) the parameters must be read from extra arcs.
Example 
`   node_arc_parameter 2 ; 2 additional arcs for large parameters `

### node_same_rxtx_data_rate "0/1"
The nodes are called with a data structure for each arc giving :
 - a pointers on input and out arcs 
 - the amount of data (input arcs) and free space (output arcs)

The nodes returns after updating the data structures, but to lower the computation overhead in the node it can tell here if the data flow can vary between input and output arcs. If the data rate do not change it tells here the scheduler to update the data structure automatically.

Examples of variable data rate (node_same_rxtx_data_rate 0): MP3 encoder with variable rate encoding option, audio rate converter at using 160/147 ratio.
Examples of identical data rate (node_same_rxtx_data_rate 1) : an audio filter, an amplifier receiving and delivering the same data formats and frame sizes.

Parameter value "0" tells the data flow is variable. The default assumption is "1" : ALL input and output arcs have the same raw data rate (same number of bytes consumed and produced per call).
Example : ` node_steady_stream   0  ; variable stream rates `


### node_using_arc_format "0/1"
During the RESET phase of the node, the scheduler will push (with "1" or not with "0") the arc data formats of all the arcs. The arc format is described in the chapter "IO control and stream data formats" of the file "Graph Template".
Example
` node_using_arc_format 1  the node is using arc stream format `

### node_mask_library    "n" (TBD)
The graph interpreter offers a short list of optimized DSP/ML and Math functions optimized for the platform using dedicated vector insttructions and coprocessors. 
Some platform may not incorporate all the libraries, the "node_mask_library" is a bit-field associate to one of the librarry service. 
This list of service is specially useful when the node is delivered in binary format.
- bit 0 for the STDLIB library (string.h, malloc)
- bit 1 for MATH (trigonometry, random generator, time processing)
- bit 2 for DSP_ML (filtering, FFT, 2D convolution-8bits)
- bit 3 for audio codecs
- bit 4 for image and video codec
Example :
`   node_mask_library  31  ; the node has a dependency to all the computing services `

### node_architecture  "name" (TBD)
Create a dependency of the execution of the node to a specific processor architecture. For example when the code is incorportating in-line assembly to a specific architecture.

Example:
` node_architecture    armv6m  ; a node made for Cortex-M0 ` 


### node_fpu_used "option" (TBD)
The command creates a dependency on the FPU capabilities
Example :
`  node_fpu_used   0              ; fpu option used (default 0: none, no FPU assembly or intrinsic) `

### node_use_unlock_key  "0/1"  (TBD)
Tells if a key-exchange protocol is initiated at reset time to deobfuscate node's firmware and enable specific features.
Example :
`  node_use_unlock_key  1  ` 

### node_node_version    "n"
For information, the version of the node
Example :
`   node_node_version    101            ; version of the computing node '

### node_stream_version  "n"            
Version of the stream scheduler it is compatible with.
Example :
`   node_stream_version    101 ` 

### node_logo  "file nae" 
File name of the node logo picture (JPG/GIF format) to use in the GUI.

--------------------------------------

## Node memory allocation
A node can ask for up to 6 memory banks with tunable fields : 
- type (static, working, static with periodic backup)
- speed (normal, fast, critical fast)
- relocatable (the location can change after the node was notified)
- program / data 
- size in bytes

The size can be a simple number of bytes or a computed number coupled to a function of stream format parameters (number of channels, sampling rate, frame size) and a flexible parameter defined in the graph, here.
The total memory allocation size in bytes = 
`     A                             fixed memory allocation in Bytes (default 0) ` 
`   + B x nb_channels of arc(i)     number of channels in arc index i (default 0) ` 
`   + C x sampling_rate of arc(j)   sampling rate of arc index j (default 0) ` 
`   + D x frame_size of arc(k)      frame size used for the arc index k (default 0) ` 
`   + parameter from the graph      optional field "node_malloc_add" ` 

The memory block index 0 is the node instance, followed by other blocks. 
;
### node_mem "index"           
The command is used to start a memory block declaration with the index in the parameter.
Example :
` node_mem 0    ; starts the declaration section of memory block #0 `  

### node_mem_alloc  "A"
The parameter gives the "A" value of fixed memory allocation in Bytes.
Example :
`   node_mem_alloc          32          ; size = 32Bytes data memory, default Static, default Fast memory block ` 

### node_mem_nbchan "B" "i" 
Declaration of extra memory in proportion to the number of channel of a specified arc index.
Example :
`   node_mem_nbchan 44 3  ; add this amount of bytes : 44 x nb of channels of arc 3

### node_mem_sampling_rate "C" "j'"
Declaration of extra memory in proportion with the sampling rate of a given arc index.
Example :
`   node_mem_sampling_rate 44.0 3  ; add this amount of bytes : 44.0 x sampling_rate in Hertz of arc 3

### node_mem_frame_size "D" "k"   
Declaration of extra memory in proportion with the frame size  of the stream flowing through a specified arc index.
Example :
`   node_mem_frame_size 44 3  ; add this amount of bytes : 44 x frame size of arc 3

### node_mem_alignement "n"
Declaration of the memory Byte alignment
Example :
`   node_mem_alignement     4           ; 4 bytes to (default) ` 

### node_mem_type "n"
Definition of the dynamic characteristics of the memory block :
- 0 STATIC : memory content is preserved (default )
- 1 WORKING : scratch memory content is not preserved between two calls 
- 2 PERIODIC_BACKUP static parameters to reload during a warm reboot 
- 3 PSEUDO_WORKING static only during the uncompleted execution state of the NODE
Example :
`   node_mem_type 3   ; memory block put in a backup memory area when possible ` 

### node_mem_speed "n"
Declaration of the memory desired for the memory block.
- 0 for 'best effort' or 'no constraint' on speed access
- 1 for 'fast' memory selection when possible
- 2 for 'critical fast' section, to be in I/DTCM when available
Example :
`   node_mem_speed 0   ; relax speed constraint for this block ` 

### node_mem_relocatable "0/1"
Declares if the pointer to this memory block is relocatable (parameter '1'), or assigned a fixed address at reset (default, parameter '0').
When the memory block is relocatable a command 'STREAM_UPDATE_RELOCATABLE' is sent to the node with address changes.
Example :
`  node_mem_relocatable    1   ; the address of the block can change `

### node_mem_data0prog1 "0/1"
This command tells if the memory will be used for data or program accesses. Default is '0' for data access.
Example :
`   node_mem_data0prog1  1 ; program memory block ` 

--------------------------------------

## Configuration of the arcs attached to the node
The arc configuration gives the list of compatible options possible for the node processing. Some options are described as a list, or as a range of values. 
The syntax is : an index and the list of numbers within brackets "{" and "}". The index gives the default value to consider in the list. Index "1" corresponds to the first element of the list.
Index value "0" means "any value". The list can be empty in that case.
Example : 
`   { 2  5 6 7 8 9 } ` is an option list between five values, the index is 2 meaning the default value is the second in the list (value = 6).
When the index is negative the list is decoded as a "range". A Range is a set of three numbers : 
- the first option
- the step to the next possible option
- the last (included) option

The absolute index value selects the default value in this range
Example : 
`   { -3  1 0.2 4.2 } ` is an option list of values (1, 1.2, 1.4, 1.6, 1.8, .. , 4.2), the index is -3 meaning the default value is the third in the list (value = 1.4).

### node_arc "n"
The command starts the declaration of a new arc, followed by its index used when connecting two nodes.
Example :
`   node_arc 2    ; start the declaration of a new arc with index 2

Implementation comment : all the nodes have at least one arc on the transmit side used to manage the node's locking field.

### node_arc_rx0tx1 "0/1"
Declares the direction of the arc from the node point of view : "0" means a stream is received  through this arc, "1" means the arc is used to push a stream of procesed data.
node_arc_rx0tx1             0               ; followed by 0:input 1:output, default = 0 and 1

### node_arc_interleaving  "0/1"
Arc data stream interleaving scheme: "0" for no interleaving (independent data frames per channel), "1" for data interleaving at raw-samples levels.
Example :
`   node_arc_interleaving 0     data is deinterleaved on this arc `

### node_arc_nb_channels  "n"
Number of the channels possible for this arc (default is 1).
Example :
`   node_arc_nb_channels {1 1 2}  ; options for the number of channels is mono or stereo `

### node_arc_raw_format "f"
Raw samples data format for read/write and arithmetic's operations. The stream in the "2D domain" are defining other sub-format 
Example :
`   node_arc_raw_format {1 17 27} raw format options: STREAM_S16, STREAM_FP32, default values S16 `

### node_arc_frame_length "n"
Frame size options in Bytes. 
node_arc_frame_length       {1 1 2 16}      ; options of possible frame_size in number of sample (can mono or multi-channel)
Example :
`   node_arc_frame_length 2    ; start the declaration of a new arc with index 2 `

### node_arc_frame_duration "t"
Duration of the frame in milliseconds. The translation to frame length in Bytes is made during the compilation of the graph from the sampling-rate and the number of channels. 
A value "0" means "any duration" which is the default.
Example :
`   node_arc_frame_duration {1 10 22.5}  frame of 10ms (default) or 22.5ms `

### node_arc_sampling_rate "fs"
Declaration of the allowed options for the node_arc_sampling_rate in Hertz.
Example :
`   node_arc_sampling_rate {1 16000 44100} ; sampling rate options, 16kHz is the default value if not specified `  

### node_arc_sampling_period_s  "T"
Duration of the frame in seconds. The translation to frame length in Bytes is made during the compilation of the graph from the sampling-rate and the number of channels. 
A value "0" means "any duration" which is the default.
Example :
`   node_arc_sampling_period_s {-2 0.1 0.1 1}  frame sampling going from 100ms to 1000ms, with default 200ms`

### node_arc_sampling_period_day "D"
Duration of the frame in days. The translation to frame length in Bytes is made during the compilation of the graph from the sampling-rate and the number of channels. 
A value "0" means "any duration" which is the default.
Example :
`   node_arc_sampling_period_day {-2 1 1 30}  frame sampling going from 1 day to 1 month with steps of 1 day.`

### node_arc_sampling_accuracy  "p"
When a node does not need the input data to be rate-accurate, this command allows some rate flexibility without the need for the insertion of a synchronous rate converter.
The command parameter is in percent.
Example :
`   node_arc_sampling_accuracy  0.1  ; sampling rate accuracy is 0.1%  `


### node_arc_inPlaceProcessing  "in out"
Memory optimization with arc buffer overlay. This command tells the "in" arc index is overlaid with the "out" arc index. The default configuration is to allocate different memory for input and output arcs. 
The arc descriptors are different but the base address of the buffers are identical.
Example :
`   node_arc_inPlaceProcessing  1 2   ; in-place processing can be made between arc 1 and 2 `

--------------------------------------

# Node design

LINK is using shared memory, all processors have access to the same graph, there is no message passing scheme, the LINK scheduler’s instances and doing the same estimations in parallel, deciding which node needs to be executed in priority. Data moves with the shared memory can implemented with a DMA and dedicated abstractions in LINK. The application is connected to the boundary of the graph, also looking to the same graph structure.

General Programming Guidelines : 
· Algorithms are C callable, or respecting the EABI
· Algorithms are reentrant
. Data are treated as little endian by default
. Data references are relocatable, there is no “hard-coded” data memory locations
. All algorithm code must be fully relocatable. That is, there can be no hard coded program memory locations.
· Algorithms are independent of any particular I/O peripheral, No hard coded addresses
· Algorithms are characterized by their memory and MIPS requirements
. Algorithms must characterize their ROM-ability; i.e., state whether or not they are ROM-able (no self-modifying code unless documented so).
. Run-time object creation should be avoid : memory reservation should be done once during the initialization step.
. All algorithms must characterize their worst-case interrupt latency for every operation.
. Algorithms are managing buffers with pointer using physical addresses
. Processors have no MMU : there is no mean of mapping physically non-contiguous segments into a contiguous block
. Cache coherency is managed in LINK at transitions from one node to the next one.
. Algorithms should not use stack allocated buffers as the source or destination of any DMA transfer.
. Algorithms must request all data buffers in external memory with 32-bit alignment and sizes in multiples of 4 (bytes).
. Persistent / scratch data, stack and heap usage will be documented

## Conformance checks
Purpose: create an automatic process to incorporate new NODE in a large repository and have a scalable mean to check conformance: 
- verification of the conformance to the APIs
- injection of typical and non-typical data aligned with NODE description
- check of outbound parameter behavior
- check of stack consumption and memory leakage.

### Calling sequence

### Data pointers 

XDM data READ/WRITE index is updated before returning when node_same_rxtx_data_rate=0

when node_same_rxtx_data_rate=1 the scheduler takes care 

### Command parameters

The main CMSIS-Stream instance (the one located in the main process or processor) is called by the application to compute the amount of memory needed to execute the graph : the buffers of the arcs, the NODE instances of the graph, the buffers used for IOs (command “STREAM_MEMREQ” below).

In a second step, the application provides the memory pointers to the requested memory banks. The CMSIS-Stream instances are now allowed to activate of the IOs at the boundary of the graph, do the memory initialization of all NODE (command “STREAM_RESET” below).

Finally, the application lets the graph being scheduled by CMSIS-Stream (command “STREAM_RUN” above).

The first control API has four parameters,  three data parameters and a command with values :
STREAM_MEMREQ : the application asks for the amount of memory needed to schedule the graph; the function returns the needed amount of memory for each memory bank (see “1.1.2 processor characteristics”). The parameters are :
•	A function pointer to the firmware of the platform, in charge of the low-level abstraction of the hardware controls:
o	Returns the details of current processor: its index in the manifest table, its architecture and the FPU options
o	Call one on the three functions used to control the device drivers : “set”, “start”, “stop” (see “2) The graph boundaries”)
o	Read the time information, for example computed from a SYSTICK global counter.
•	A pointer to the list of NODE entry points, and a pointer to their respective manifests (see “1.3 NODE manifests”)
•	A pointer to the “graph description text” to be compiled to “binary graph structure”
a)	STREAM_RESET : pointers memory banks are provided to “arm_stream()”  which can initialize its instances and the NODE instances of the graph. In a similar way described for NODE (see “3.1.2 NODE  parameter RESET”) the application is providing to the CMSIS-Stream a callback mechanism. Each CMSIS-Stream instance is stored in the shared memory of “binary graph structure” with the format:
•	7 offsets to the physical memory banks (see “1.1.2 processor characteristics”). This information lets the arc’s buffer being address with indexes instead of physical pointers, it allows sharing the same arc’s descriptors among processors having different memory address decoding (including arch64 processors).
•	Debug information, the execution state of the instance, the current NODE under processing.
•	A 32bits-field of the graph IO ports to have look to. Most CMSIS-Stream instances will not be given access to the peripherals. Those indexes are used to address the platform IO manifest and checking the associated graph’s ring buffers are not having flow problems.
•	A function pointer to the firmware of the platform (see above).
•	The list of NODE entry points

b)	STREAM_RUN : the graph of components is scheduled (the linked-list of the “binary graph structure” is parsed, see “3) Linked list of NODE”)
c)	STREAM_END : command forwarded to each NODE to release memory allocated with stdlib.

CMSIS-Stream is scheduling the software components of a graph. The nodes of the graph are software components (“NODE ”) independent of the platform capabilities.

The graph description is a text file (example here) and is the result from the translation made in a GUI tool, using : 
•	a manifest of the platform (details on processors, memory, peripherals)
•	a manifest of each NODE : description of the data formats of the interfaces

CMSIS-Stream is translating the graph description text file to a binary graph structure, with the help of the data in the manifests. This result is placed in shared memory area to all processes and processors.

This shared binary graph structure consists in :
•	the linked list of arcs and nodes (the NODE ) of the graph
•	the arcs descriptors (read and write indexes to circular buffers)
•	the memory of the CMSIS-Stream instances scheduled the graph.
•	the structure describing the operations at the boundary of the graph (the graph “IOs”)
•	registers used to synchronize the different CMSIS-Stream instances, if any

Two entry-points 
CMSIS-Stream has two entry-points, one for controlling and asking for services, and a second one used as callback for notifications of data transfers : 
void arm_stream (uint32_t command, PTR_INT ptr1, PTR_INT ptr2, PTR_INT ptr3);
void arm_stream_io (uint32_t fw_io_idx, void *data, uint32_t length);

The second control API (arm_stream_io) has three parameters : the index of the device driver calling this function, the base address of the buffer, the size of the buffer. The “index” is given in the platform IO manifest (see ”1.2.4 The ID of the hardware port”). Data format and interleaving is described at “A.3.1 Data format fields common to all streams”. 

The description of the scheduling of the graph consists in :
•	the content of the manifests of the platform and the manifests of the NODE  
o	paragraph below “1) Platform and NODE manifests”
•	the way the IOs are sharing data with the ring buffers at the boundary of the graph 
o	paragraph below “2) The graph boundaries”
•	the description of the linked-list and the connections between arcs and nodes 
o	paragraph below “3) Linked list of NODE ”





3.1 NODE interface
The NODE have a single entry point in the format “func (int, *,*,*)”. The first parameter is the execution command (memory request, reset, set parameters, run, stop). The NODE can call CMSIS-Stream through a dedicated function pointer provided at reset time. An example of NODE API here.
3.1.1 NODE parameter “MEMREQ”
The first operation asked by the scheduler is to ask the NODE for memory allocation with respect to parameters associated to the input and output stream format (the NODE may ask for working memory buffer size in relation with the frame size of the streams). 
A NODE delivered in source-code, or as library object using the same compilation tool chain as the application, can use memory allocation function from the standard C library (malloc(), realloc(), calloc()), and will have to manage the “free()” deallocation upon reception of the command “STREAM_END”.
The format is : func (STREAM_MEMREQ,  *ptr1, *ptr2, *ptr3):
•	The first pointer is a memory space of 7 words of 32bits. The NODE will fill this area with up to 6 memory allocation requests terminated with “0”. Each word is a bit-field (description in “A.4 Memory types”) giving the size of the memory buffer, the byte alignment and the recommended NODE. The memory can be declared “static” or “working”(or “scratch memory area”), depending if the content needs to be preserved between two calls. The first memory request is the “instance”, which holds pointers to static and working memory buffers. The pointer to this memory area is reused in all the other NODE commands.
•	The second parameter is pointing to a table of the stream formats used (see “A.3 Stream digital “data formats"”). This information (buffer size, sampling rate, interleaving scheme) can be used by the NODE to adjust the request to the minimum amount of memory.
•	The last parameter is TBD and reserved for a NODE activation protocol with key exchanges

3.1.2 NODE parameter “RESET”
The second operation of the scheduler is to provide the NODE with the memory being allocated.
The format is : func (STREAM_RESET,  *ptr1, *ptr2, *ptr3):
•	The first parameter points to the NODE instance, with memory allocation corresponding to the first word of the STREAM_MEMREQ. The following data is a vector of pointers corresponding the memory allocation requested in the same following order provided by the STREAM_MEMREQ.
•	The second parameter is a pointer to the entry point of CMSIS-Stream, and giving access to optional services in computing, signal compression. There is a protocol TBD to activate this link : the NODE will use a single subroutine as calling address and will register the return address (seen by CMSIS-Stream) with a dummy call during this initialization sequence.
•	The last parameter is unused.

3.1.3 NODE parameter “SET_PARAMETER”
CMSIS-Stream is setting the NODE parameter at reset time with the default reset parameter vector provided in its manifest. This API allows to change a single parameter or the full set. 
The format is : func (STREAM_SET_PARAMETERS,  *ptr1, *ptr2, *ptr3):
•	The first parameter “ptr1” points to the NODE instance.
•	The second parameter points to the parameters to be updated
•	The last parameter will be casted to integer and the LSB 9 bits tell the index or the tag used by the NODE documentation to change one parameter. The value 256 tells the full parameter list will be set.
The scheduler has no way to decide to change a parameter during the execution of the graph. The Scripts are used for this purpose.
3.1.4 NODE parameter “READ_PARAMETER”
The scheduler is reading the NODE parameter at reset time with the default reset parameter vector provided in its manifest. This API allows to change a single parameter or the full set. 
The format is : func (STREAM_READ_PARAMETERS,  *ptr1, *ptr2, *ptr3):
•	The first parameter “ptr1” points to the NODE instance.
•	The second parameter points to the parameters to be updated
•	The last parameter will be casted to integer and the LSB 9 bits tell the index or the tag used by the NODE documentation to change one parameter. The value 256 tells the full parameter list will be set.

3.1.5 NODE parameter “RUN”
The scheduler launches the execution of the NODE when the conditions, found in the linked-list fields (processor  architecture, arc’s ready-flags) are set. The stream of data from the arcs are exchanged in the format detailed in the NODE manifest (see “A.3.1 Data format fields common to all streams”).
The calling format is : func (STREAM_RUN,  *ptr1, *ptr2, *ptr3). 
As previously “ptr1” is the instance pointer of the component. “ptr2” is a pointer to a structure.
The "*ptr2" field points to a structure : [{data pointer  stream1} {data pointer  stream2} .. ]
The "*ptr3" field points to a similar structure :  [{parameter of stream1} {parameter of stream2} .. ]
Simple components have two streams : one as input, the other as output other can have up to 4 streams (several input/output combinations).
A data buffer is combination of a pointer and size. For input streams the size is the amount of data in the buffer. For output streams this is the amount of free space in the output buffer starting from the pointer address,
The NODE is updating the base address pointers and data sizes before returning to caller.
When a stream data format is FMT_INTERLEAVED, (for example Left and Right audio samples are in this order : LRLRLRLRLRLR .. )  then {data stream} is a pointer to the base address, {parameter stream} is the number bytes in the buffer.
When a stream data format is FMT_DEINTERLEAVED_1PTR, (for example Left and Right audio samples are in this order : LLLL..LLLLRRRR...RRRR ) the size of the first buffer (the "frame") then {data stream} is a pointer to the base address, {parameter stream} is the number bytes for a single frame (the size of the Left sample portion). The NODE will address the second and following channels by incrementing the base pointer address with the size of the frame.
When a stream data format is FMT_DEINTERLEAVED_NPTR. The buffers have independent positions (for example color planes of images). The {data stream} is a combination of the pointer to the base address, and the size of the corresponding buffer. {parameter stream} is unused. For example with stereo audio : {data stream} points to a structure : [ {*ptr_L, size L}, {*ptr_R, size R} ], this is the format used in EEMBC-audiomark.

3.1.6 NODE parameter “END”
This command is used to free the allocated memory. The format is : func (STREAM_END,  instance pointer, 0, 0).



### Test-bench and non-regression test-patterns

Delivery of test-bench.
