[TOC]
--------------------------------------
# Introduction
All the programs can be used with the Graph-scheduler (also called computing "nodes") as soon as a minimal amount of description is given in a "manifest" and the program can be used through a single entry point with a wrapper using the prototype :

```
void (node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);
```

Where "command" tells to reset, run, set parameters, ..  "instance" is an opaque access to the static area of the node, "data" is a table of pointers+size pairs of all the arcs used by the node, and "state" returns information of computing completion of the subroutine for the data being shared through the arcs.

During "reset" sequence of the graph the node are initialized. Nothing prevents a node to call the standard library for memory allocations or math computing. But the context of the graph interpreter is Embedded IoT, with extreme optimization for costs and power consumption. 

--------------------------------------

# General recommendations

General programming guidelines of Node : 

- Nodes must be C callable, or respecting the EABI.
- Nodes are reentrant, or this must be mentioned in the manifest.
- Data are treated as little endian by default.
- Data references are relocatable, there is no “hard-coded” data memory locations.
- All Node code must be fully relocatable: there cannot be hard coded program memory locations.
- Nodes are independent of any particular I/O peripheral, there is no hard coded address.
- Nodes are characterized by their memory, and MIPS requirements when possible (with respect to buffer length to process).
- Nodes must characterize their ROM-ability; i.e., state whether or not they are ROM-able (no self-modifying code unless documented so).
- Run-time object creation should be avoid : memory reservation should be done once during the initialization step.
- Nodes are managing buffers with pointer using physical addresses and shared in the parameters.
- Processors have no MMU : there is no mean of mapping physically non-contiguous segments into a contiguous block.
- Cache coherency is managed in Graph-Interpreter at transitions from one node to the next one.
- Nodes should not use stack allocated buffers as the source or destination of any graph services for memory transfer.
- Static/persistent data, retention data used for warm-boot, scratch data, stack and heap usage will be documented.
- Manifest is detailing the memory allocation section with respect to latency/speed requirements. 



# Node parameters

A node is using the following prototype 

```
void (node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state);
```

With following parameters: 

| Parameter name | Details         | Types                                                        |
| :------------- | :-------------- | ------------------------------------------------------------ |
| command        | input parameter | uint32_t                                                     |
| instance       | instance        | void *                                                       |
| data           | input data      | pointer to struct stream_xdmbuffer { int address;  int size; } |
| state          | returned state  | uint32_t *                                                   |

## Command parameter

Command bit-fields :

| Bit-fields | Name     | Details                                                      |
| :--------- | :------- | :----------------------------------------------------------- |
| 31-24      | reserved |                                                              |
| 16-23      | node tag | different roles depending on the command. With "set parameter" it gives the index of the parameter to update from the data* address  (if 0 then all the parameters are prepared in data*) |
| 15-12      | preset   | the node can define 16 "presets", preconfigured sets of parameters |
| 11-5       | reserved |                                                              |
| 4          | extended | set to 1 for a reset command with warm boot : static areas need to be initialized except the memory segments assigned to a retention memory in the manifest. When the processor has no retention memory those static areas area cleared by the scheduler. |
| 3-0 (LSB)  | command  | 1: reset</br> 2: set parameter</br> 3: read parameter</br>4: run</br>5: stop</br>6: update a relocatable memory segment</br> |

## Instance

Instance is an opaque memory pointer (void *) to the main static area of the node. The memory alignment requirement is specified in the node manifest. 

## Data

The multichannel data field is a pointer of arcs' data. This is pointer to list of structures of two "INTPTR_T" (32bits or 64bits wide depending on the processor architecture). The first INTPTR_T is a pointer to the data, the second tells the number of bytes to process (for an input arc) or the number of bytes available in the buffer (for output arcs). 

In the default situations all the "size" fields are identical : the number of bytes to process is equal to the amount of memory free to take the computation result. But the node manifest can specify node_variable_rxtx_data_rate=1 and the scheduler may call the node with different "size" fields.

A node can have **4 arcs**. Each of them can have individual format (number of channels, frame length, interleaving scheme, raw sample type, sampling rate, time-stamps).

## Status

Nodes return state is "0" unless the data processing is not finished, then the returned status of "1".



# Node calling sequence

The nodes are first called with the command *reset* followed by *set parameters* and finally *stop* to release memory. This paragraph details the content of the parameters of the node during "reset", "set parameter" and "run" :

```
void (node) (uint32_t command, stream_handle_t instance, stream_xdmbuffer_t *data, uint32_t *state); 
```

## Reset command

**The second parameter "instance"** is a pointer to the list of memory banks reserved by the scheduler for the node, in the same sequence order of the declarations made in the node manifest. The first element of the list is the instance of the node, followed by the pointers to the data (or program) memory reservations.

After this list addresses the scheduler will push (when the manifest command *node_using_arc_format* = 1) the arc data formats of all the arcs. The arc format is described in the chapter "IO control and stream data formats" of the file "Graph design". </br>

**The third parameter "data"** is used to share the address of function providing computing services. 

## Set Parameter command

The bit-field "Node Tag" tells which (or all) parameter will be updated.

**The third parameter "data"** is a pointer to the new parameters.

## Run command

The bit-field "Node Tag" tells which (or all) parameter will be updated.

**The third parameter "data"** is a pointer to the list buffer ("struct stream_xdmbuffer { int address;  int size; }") associated to each arc connected to the node.



## Test-bench and non-regression test-patterns

Nodes are delivered with a test-bench (code and non-regression database).

# Conformance checks

Purpose: create an automatic process to incorporate new NODE in a large repository and have a scalable mean to check conformance: 

- verification of the conformance to the APIs
- injection of typical and non-typical data aligned with NODE description
- check of outbound parameter behavior
- check of stack consumption and memory leakage.

# Services to the nodes (to be completed)

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
| 3-0 (LSB)  | Group    | index to the groups of services : <br/>SERV_INTERNAL     1 <br/>SERV_SCRIPT       2 <br/>SERV_CONVERSION   3  : raw data format conversion (fp32 to int16, etc..)<br/>SERV_STDLIB       4 : extract of string and stdlib (atof, memset, strstr, malloc..)<br/>SERV_MATH         5 : extract of math (srand, sin, tan, sqrt, log..)<br/>SERV_DSP_ML       6 : filtering, spectrum fixed point and integer<br/>SERV_DEEPL        7 : fully-connected and convolutional network <br/>SERV_MM_AUDIO     8 : TBD<br/>SERV_MM_IMAGE     9 : TBD |



