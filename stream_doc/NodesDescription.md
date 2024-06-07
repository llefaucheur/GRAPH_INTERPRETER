# NODE MANIFESTS (FROM THE DEVELOPER)

ARM ; developer name

arm_stream_detector ; node name

node_nb_arcs 1 1 ; nb arc input, output, default values \"1 1\"

node_arc_parameter 0 ; SWC with extra-large amount of parameters (NN
models) will declare it with extra arcs

node_steady_stream 1 ; (0) the data flow is variable (or constant,
default value :1) on all input and output arcs

node_same_data_rate 1 ; (0) the arcs have different data rates, (1) all
arcs have the same data rate

node_use_dtcm 1 ; default 0 (no MP DTCM_LW2), 1: fast memory pointer
placed after the arc format

node_use_arc_format 0 ; default 1 : the scheduler must push each arc
format (LOADFMT_LW0_LSB)

node_mask_library 15 ; default 0 bit-field of dependencies to computing
libraries

node_subtype_units VRMS ; triggers the need for rescaling and data
conversion

node_architecture 0 ; arch compatible with (default: 0 = source code) to
merge and sort for ARCHID_LW0

node_fpu_used 0 ; fpu option used (default 0: none, no FPU assembly or
intrinsic)

node_node_version 101 ; version of the computing node

node_stream_version 001 ; version of the stream scheduler it is
compatible with

# MEMORY ALLOCATIONS

;

; memory allocation size in bytes =

; A : memory allocation in Bytes (default 0)

; + B x nb_channels of arc(i) : addition memory as a number of channels
in arc index i (default 0)

; + C x sampling_rate of arc(j) ; .. as proportional to the sampling
rate of arc index j (default 0)

; + D x frame_size of arc(k) ; .. as proportional to the frame size used
for the arc index k (default 0)

; + E x parameter from the graph ; optional field \"malloc_E\" during
the node declaration in the graph (default 0)

;

; Additional information :

; i j k ; the three indexes of the arcs used above

; (A) Alignment in byte numbers ; default = 4 (bytes)

; (S) Retention mode ; 0 for a Static memory allocation, preserved along
the execution

; ; 1 for Working (or Scratch) area which can be reused and overlaid by
other nodes

; ; 2 for memory to be preserved (Retention) after a platform reboot

;

; (N) Speed requirement ; 0 for \'best effort\' or \'no constraint\' on
speed access

; ; 1 for \'fast\' memory selection when possible

; ; 2 for \'critical fast\' section, to be in I/DTCM when available

;

; (R) Relocatable ; Default 0 : not relocatable, 1: a command
\'STREAM_UPDATE_RELOCATABLE\' is

; ; sent to the node to update the pointer to this memory allocation

;

; (DP) Data0Program1 ; Default 0: selection of data (0) or program
access (1).

; A S N

node_mem_alloc 32 0 1 ; size = 32Bytes data memory, Static, Fast memory
block

; A B i C j D k E A S N R DP

node_mem_alloc_detail 32 4 0 0.1 1 0 0 44 4 0 1 0 0 ; in this example we
have a data memory allocation of

; 32 + 4xnb of channels of arc 0 + 0.1x sampling rate of arc 1 + 44x the
parameter \"malloc_E\" in the graph

; This memory area has 4bytes alignment, is Static(0) Fast(1) and not
relocatable

# ARC CONFIGURATION

node_arc_rx0tx1 0 0 ; index of the arc, followed by 0:input 1:output,
default = 0 0 and 1 1

node_arc_sampling_rate 0 {1 16000 44100}; index of the arc, sampling
rate options (enumeration in Hz), default \"any\"

node_arc_channels 0 0 {1 1 2} ; index of the arc, multichannel intleaved
(0, default), deinterleaved by frame-size (1) +

; options for the number of channels (default 1)

node_arc_raw_format 0 {1 S16} ; index of the arc, options for the raw
arithmetics computation format here STREAM_S16, , default values \"1
S16\"

node_arc_frame_length 0 {1 1 2 16} ; index of the arc, options of
possible frame_size in number of sample (can mono or multi-channel)

node_arc_frame_duration 0 {1 10 22.5} ; index of the arc, options of
possible frame_size in \[milliseconds\]

; (one sample can mono or multi-channel), default is \"any length\"

node_arc_sampling_rate 0 1 0 {1 16000 48000} ; index of the arc,
sampling rate options (default: Any)

node_arc_sampling_period_s {1 0.01 0.02 0.04} ; sampling period options
(enumeration in \[second\])

node_arc_sampling_period_day {1 0.25 1 7}; sampling period options
(enumeration in \[day\])

node_arc_sampling_accuracy 0.8 ; sampling rate accuracy in percent

node_arc_inplace_buffer 1 0 ; index of the output arc sharing the same
interface buffer as one input arc buffer (default: buffer separated)

;\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--

; all the nodes must have at least one TX-arc (even a dummy one) used to
manage the

; lock field. See scheduler\'s pt8b_collision_arc / ARCLOCK_LW0 = 0

;\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--
