[TOC]

# Node manifest template
A node manifest file gives the name of the software component, its author, the targetted architecture, the description of input and output streams connected to it.
The graph interpreter allocates a predefined amount of memory and this file explains the way to compute the memory allocation with respect to stream characteristics.
When the first non-blank character is a semi-colon ";" the entire line is skipped.

## Manifest header
The manifest starts with the identification of the node.

### node_developer_name "name"
Name of the developer/company having the legal owner of this node.
Example: 
` node_developer_name CompanyA & Sons Ltd  `

### node_name "name"
Name of the node when using a graphical design environment.
Example: 
` node_name arm_stream_filter `

### node_logo "file name"
Name of the graphical logo file (file path of the manifest) of the node when using a graphical design environment.
Example: 
` node_logo arm_stream_filter.gif `

### node_nb_arcs "in out"
Number of input and output arcs of the node used for data streaming. 
Example 
` node_nb_arcs 1 1 ; nb arc input, output, default values "1 1" `

### node_arc_parameter "n"
When the amount of parameters exceeds 256kB (deep-learning models, video file, etc..) the parameters are read from extra arcs.
Example 
` node_arc_parameter 2 ; 2 additional arcs for large parameters `

### node_same_rxtx_data_rate "0/1"
The nodes are called with a data structure for each arc giving 1) a pointers on input and out arcs 2) the amount of data (input arcs) and free space (output arcs).
The nodes returns after updating the data structures, but to lower the computation overhead in the node it can tell if the data flow can vary between input and output arcs.
If the data rate does not change it tells here the scheduler to update the data structure automatically.
Examples of variable data rate (node_same_rxtx_data_rate 0): MP3 encoder with variable rate encoding option, audio rate converter at using 160/147 ratio.
Examples of identical data rate (node_same_rxtx_data_rate 1) : a filter, an amplifier receiving and delivering the same data formats and frame sizes.
Parameter value "0" tells the data flow is variable. The default assumption is "1" when ALL input and output arcs have the same data rate.
` node_steady_stream   0  ; variable stream rates `

### node_use_mpdtcm "0/1"
In a multiprocessor for relocatable scratch DTCM usage with SMP, address is changing : arc_index_update() pushed the DTCM address after XDM buffers

              ; default 0, 1: dtcm pointer is shared in multiprocessing environements 
Example: ` node_use_mpdtcm 1  ; dtcm pointer is shared 
### node_use_arc_format  0              ; default 0.  "1": the scheduler loads the 4 words of each arc format at reset
### node_mask_library    15             ; default 0 bit-field of dependencies to computing libraries
### node_subtype_units   VRMS           ; triggers the need for rescaling and data conversion
### node_architecture    0              ; arch compatible with (default: 0 = source code) to merge and sort for ARCHID_LW0
### node_fpu_used        0              ; fpu option used (default 0: none, no FPU assembly or intrinsic)
### node_use_unlock_key  1              ; key-exchange of the encrypted entitlement key used to deobfuscate node's firmware
### node_node_version    101            ; version of the computing node
### node_stream_version  001            ; version of the stream scheduler it is compatible with
### node_logo           FILENAME        ; file name of the node logo to use in the GUI in JPG/GIF format

;----------------------------------------------------------------------------------------
;   MEMORY ALLOCATIONS
;
;   memory allocation size in bytes = 
;   A                               : memory allocation in Bytes (default 0)
;   + B x nb_channels of arc(i)     : addition memory as a number of channels in arc index i (default 0)
;   + C x sampling_rate of arc(j)   ; .. as proportional to the sampling rate of arc index j (default 0)
;   + D x frame_size of arc(k)      ; .. as proportional to the frame size used for the arc index k (default 0)
;   + parameter from the graph      ; optional field "malloc_E" during the node declaration in the graph, for
;                                   ;   example the number of pixels in raw for a scratch area (default 0)
;   The memory block index 0 is the one of the node instance, followed by other blocks 
;   The graph holds the address (shared with the node) and size (for memory the memory protection) of each block
;   The address is in "27bits" data format (3 bits for the physical memory offset + 24 linear), size is in Bytes
;   The translation of 27bits to physical address is made by the graph interpreter instance before the node call
;
### node_mem                2           ; start the declaration of a new memory block with index 2
### node_mem_alloc          32          ; size = 32Bytes data memory, Static, Fast memory block
### node_mem_nbchan         4 0         ; add in Bytes : 4 x nb of channels of arc 0 
### node_mem_sampling_rate  0.1 1       ; add in Bytes : 0.1 x sampling rate of arc 1
### node_mem_frame_size     1 0         ; add in Bytes : 1 x frame size of arc 0 
  
### node_mem_alignement     4           ; 4 bytes to (default) 
### node_mem_type           0           ; MEM_TYPE_STATIC          0 (LSB) memory content is preserved (default )
                                        ; MEM_TYPE_WORKING         1 scratch memory content is not preserved between two calls 
                                        ; MEM_TYPE_PERIODIC_BACKUP 2 static parameters to reload for warm boot after a crash 
                                        ; MEM_TYPE_PSEUDO_WORKING  3 static only during the uncompleted execution state of the SWC, see NODE_RUN 
### node_mem_speed          2           ; 0 for 'best effort' or 'no constraint' on speed access
                                        ;   1 for 'fast' memory selection when possible
                                        ;   2 for 'critical fast' section, to be in I/DTCM when available
### node_mem_relocatable    1           ; Default 0 : not relocatable, 1: a command 'STREAM_UPDATE_RELOCATABLE' is
                                        ;   sent to the node to update the pointer to this memory allocation
### node_mem_data0prog1     0           ; selection data / program

;--------------------------------------------------------------------------------------------
;    ARC CONFIGURATION

node_arc                    2               ; start the declaration of a new arc with index 2
node_arc_rx0tx1             0               ; followed by 0:input 1:output, default = 0 and 1
node_arc_sampling_rate      {1 16000 44100} ; sampling rate options (enumeration in Hz), default "any"  
node_arc_interleaving       0               ; multichannel intleaved (0, default), deinterleaved by frame-size (1) 
node_arc_nb_channels        {1 1 2}         ; options for the number of channels (default 1)
node_arc_raw_format         {1 17}          ; options for the raw arithmetics computation format here  STREAM_S16, , default values "1 S16"
node_arc_frame_length       {1 1 2 16}      ; options of possible frame_size in number of sample (can mono or multi-channel)
node_arc_frame_duration     {1 10 22.5}     ; options of possible frame_size in [milliseconds]
                                            ;   (one sample can mono or multi-channel), default is "any length"

node_arc_sampling_period_s  {1 0.1 0.2 0.4} ; sampling period options (enumeration in [second])   
node_arc_sampling_period_day {1 0.25 1 7}   ; sampling period options (enumeration in [day])   
node_arc_sampling_accuracy  0.8             ; sampling rate accuracy in percent
     
node_arc_inPlaceProcessing  1 0             ; index of the output arc sharing the same interface buffer as one 
                                            ;  input arc buffer (default: all output buffers are separated from the input buffers)

;-------------------------------------------------------------------------------------------
;   all the nodes must have at least one TX-arc (even a dummy one) used to manage the 
;   lock field. See scheduler's pt8b_collision_arc / ARCLOCK_LW0 = 0
;-------------------------------------------------------------------------------------------
