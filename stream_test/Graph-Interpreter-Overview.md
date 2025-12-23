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




# Stream-based processing with a graph interpreter

## What

**Graph-Interpreter** is a scheduler of **DSP/ML Nodes** designed with three objectives: 

1. **Accelerate time to market**

Graph-Interpreter helps system integrators and OEMs who develop complex DSP/ML stream processing systems.  It enables rapid progress from prototypes validated on a computer to the final tuning stages on production boards by updating a graph of computing nodes and their coefficients without recompiling the device.

2. **NanoApps repositories**

It provides an opaque interface to the platform memory hierarchy for computing nodes.  It ensures that data flow is translated into the desired formats for each node.  It prepares the conditions under which nodes can be delivered from a Store.

3. **Portability, scalability**

The same stream-based processing methodology can be used on devices with as little as 1 kilobyte of internal RAM and on heterogeneous multiprocessor architectures.  Nodes can be produced in any programming language.  Graphs are portable when interpreted on another platform.

## Why

The complexity of IoT systems using signal processing and machine learning is continuously increasing.  Over four years (see picture below), the average time-to-market has shifted from months to years.  We must ease the work of system integrators by **splitting problems into small pieces**, which translates into the definition of standard interfaces between those pieces.

Here are some examples of signal-processing components and software portability issues : 

- An algorithm extracts metadata from a pressure sensor whose samples are a stream of floating-point data at a 10 Hz sampling rate.  Can the algorithm be ported as-is to a platform using a pressure sensor that outputs 16-bit integers at a 25 Hz sampling rate?
- A pattern-recognition algorithm uses images in a 300 × 300 pixel RGB888 format.  What happens when the platform uses a sensor with a VGA image format?
- An audio algorithm uses 50 kB of RAM, of which 4 kB are critical for fast access and 25 kB have no speed constraint.  What happens when several algorithms, or several instances of the same algorithm, want to use the fast tightly coupled memory bank (TCM)?  How do we manage data swapping before and after calling the algorithms?
- A device contains two microprocessors, and there is no MMU to hide physical addresses.
- The task scheduler must be portable to devices using an RTOS or running bare metal.
- The graph can be redesigned without requiring code recompilation or the use of containers.  Reconfiguration is managed by a small file describing the graph, which is interpreted.
- A motion sensor subsystem is designed to integrate components from different silicon vendors.  How do we automatically manage the scaling factors associated with the sensors to achieve the same dynamic range and sampling rates in the data stream?
- A microprocessor has an FFT accelerator (or a matrix-multiply accelerator).  Can we offer an abstraction layer to algorithm designers for such coprocessors?  The developer would release a single software implementation.  FFT computation would use software libraries when no coprocessor is available.

Creating standard interfaces allows software component developers to deliver their IP without having to consider the capabilities of the platform used during system integration.

![ ](Iot_TTM.PNG)

We want algorithm developers to focus on their domain expertise without creating dependencies on the protocols used in the graph or on the data formats used by the preceding and following nodes in the graph.

The data format translators provided with the graph scheduler consist of changing:

- The data frame length and the interleaving scheme (block-based or sample-based)
- The raw sample data format (pixel format, integer, or floating-point samples)
- The sampling rate and the management of timestamps
- The scaling of data with respect to standard physical units (see Units, RFC8428, and RFC8798)

Computing nodes and platforms must formally describe their interfaces in “Manifests”.

We want to anticipate the creation of stores of computing nodes, with a platform-specific key exchange protocol, when nodes are delivered in binary format or as obfuscated source code.

We want to allow the graph to be modified without needing to recompile and re-flash the entire application.  
The graph will incorporate sections of interpreted code to manage state machines, scripting, parameter updates, and interfaces with the application.

-----------------------------------------------------------------------------

## How

Graph Interpreter is a scheduler and interpreter of a binary representation of a graph (Graph-design). For portability reasons, Graph Interpreter uses a minimal platform abstraction layer (AL) for memory and input/output stream interfaces.  Graph Interpreter manages the data flow of “arcs” between “nodes”.

This binary graph description is a compact data structure that uses indexes to reference the physical addresses of nodes and memory instances.  This graph description is generated in three steps:

1. Platform manifest and IO manifest files are prepared ahead of the graph design and describe the hardware.  The manifests provide processing capabilities, including processor architecture, the minimum guaranteed amount of memory per RAM block and its speed, and TCM sizes.  The platform manifest provides a list of node manifest files for each installed processing node, including developer identification, input and output data formats, memory consumption, parameter documentation, and lists of presets, test patterns, and expected results.
2. The graph is either written in a text format or generated using a graphical tool.
3. The binary file to be used on the target is generated or compiled.  The file format is either a C source file or a binary table loaded into a specific flash memory block, allowing fast tuning cycles without full recompilation.

The platform provides an abstraction layer (AL) with the following services:

1. Share the physical memory map base addresses.  The graph uses indexes to the base addresses of 63 different memory banks, such as shared external memory, fast shared internal memory, and fast private per-processor memory (TCM), using the same indexing scheme for multiprocessing without an MMU.  The abstraction layer shares the entry points of the nodes installed in the processor memory space.

2. Interface with the graph boundaries that generate or consume data streams,  as declared in the platform manifest and addressed by indexes from the scheduler when the FIFOs at the boundaries of the graph are full or empty.

3. Share information for scripts.  The graph embeds byte-code scripts used to implement state machines, change node parameters, check arc data content, trigger GPIOs connected to the graph, generate character strings for the application, and more.  The Scripts (Graph-Scripts-byte-codes) provide a simple interface to the application without requiring code recompilation.

Graph-Interpreter is delivered with a generic implementation of the above services for computers, with device drivers emulated using data files and time information emulated with counters.  Graph-Interpreter is delivered as open source.

------------------------------------------------------------------------------

## How (detailed)

Stream-based processing is facilitated using Graph-Interpreter:

1. The Graph Interpreter exposes only **two functions.**  
   One entry point is provided for the application, `void arm_graph_interpreter()`, and one function is used to acknowledge data transfers with the outside of the graph, `void arm_stream_io_ack()`.

2. Nodes can be written in any programming language.  
   The scheduler addresses the nodes through a single entry point using a four-parameter API (Node-parameters) format.  
   There is no restriction on delivering nodes in binary format compiled with the “position independent execution” option.  
   There is no dynamic linking issue: nodes delivered in binary form can still access a subset of the C standard libraries through a Graph-Interpreter service.  
   Nodes can also access DSP/ML kernels compiled without the position-independent option or be executed using platform-specific accelerators.  
   Execution speed therefore scales with the capabilities of the target processor without recompilation.

3. Drift management.  
   Streams do not need to be perfectly isochronous, which can occur when peripherals use different clock trees.  
   Drift and rate conversion services are provided by nodes delivered with the interpreter.  
   The graph defines different qualities of service (QoS).  
   When a main stream is processed together with drifted secondary streams, the time base is adjusted to the highest-QoS streams to minimize latency and distortion, while secondary streams are managed using interpolators in case of flow issues.

4. Graph-Interpreter manages TCM access.  
   When a node declares, in its manifest, the need for a small “critical speed” memory bank (ideally less than 16 kB), the graph compilation step allocates it to the TCM area and can arrange data swapping if several nodes have the same requirement.

5. Backup and retention RAM.  
   Some applications require fast recovery in case of failures (“warm boot”) or when the system restores itself after deep-sleep periods.  
   One of the memory banks allows developers to save the state of algorithms for a fast return to normal operation.  
   Node retention memory should be limited to tens of bytes.

6. Graph-Interpreter allows memory size optimization through overlays of different nodes’ scratch memory banks.

7. Multiprocessing.  
   SMP and AMP configurations with 32-bit and 64-bit processor architectures are supported.  
   The graph description is placed in shared memory.  
   Any processor with access to this shared memory can contribute to processing.  
   Buffer addresses are described using a 6-bit offset and an index, allowing the same address to be processed without an MMU.  
   The node execution reservation protocol is defined in the abstraction layer, with a proposed lock-free algorithm.  
   Node execution can be mapped to a specific processor and architecture.  
   Buffers associated with arcs can be allocated in a processor’s private memory banks.


8. Scripting is designed to avoid repeated interactions with application interfaces for simple decisions, without requiring application recompilation.  
   This low-code or no-code strategy supports operations such as toggling a GPIO, changing a node parameter, or building a JSON string.  
   The graph scheduler interprets a compact byte stream of codes to execute simple scripts.

9. Process isolation.  
   Nodes never read the graph description data.  
   Arc descriptors and memory mappings are designed to support the use of hardware memory protection.

10. Format conversions.  
    Developers declare the input and output data formats of nodes in the manifests.  
    The Graph Interpreter implements format translation between nodes, including sampling-rate conversion, raw data changes, timestamp removal, and channel de-interleaving.  
    Specific conversion nodes are inserted into the graph during binary file translation.

11. Graph-Interpreter manages the various methods of controlling I/O,  
    with one function per I/O for parameter setting and buffer allocation, data movement, stopping, and mixed-signal component configuration.

12. Graph-Interpreter is open source and portable to 32-bit processors and computers.

13. Examples of nodes include image and audio codecs, data conditioning components, motion classifiers, and data mixers.  
    Graph-Interpreter includes a short list of components for data routing, mixing, conversion, and detection.

14. From the developer’s point of view, Graph-Interpreter creates opaque memory interfaces to the input and output streams of a graph and ensures that data is exchanged in the desired formats for each node.  
    Graph-Interpreter manages memory mapping with speed constraints provided by the developer at instance creation.  
    This allows software to run with maximum performance in increasingly memory-bounded scenarios.

15. From the system integrator’s point of view, Graph-Interpreter simplifies tuning and replacement of one node by another and facilitates processing distribution across multiprocessors.  
    Streams are described using a graph, represented as a text file designed with a graphical tool.  
    DSP/ML processing can be developed without writing code, allowing graph changes and tuning without recompilation.

16. Graph-Interpreter design objectives include a low RAM footprint.  
    Graph descriptors can be placed in flash memory, with only a small portion stored in RAM.  
    Use cases range from small Cortex-M0 devices with 1 kilobyte of RAM (approximately 200 bytes of stack and 100 bytes of static RAM) to SMP, AMP, coprocessor systems, and mixed 32-bit and 64-bit architectures, enabled by shared RAM and indexed memory banks provided by the platform abstraction layer.  
    Each arc descriptor can address buffer sizes of up to 64 gigabytes in each of the 64 memory banks.

![](ProcessingFlow0.JPG)

1) The development flow is as follows:

   1) The platform provider produces a manifest describing the processor and I/O interfaces, together with the abstraction layer and an optional list of callbacks providing platform-specific services.  
   2) The node software developer produces the node code and the corresponding manifest.  
   3) Finally, the system integrator creates a binary file representing the graph of DSP/ML components for the application.  
      The system integrator adds additional callbacks that will be used by the scripting capability of the graph.

------

# How to start

It depends who you are.

| User               | Actions                                                      |
| ------------------ | ------------------------------------------------------------ |
| platform vendor    | deliver an abstraction layer and a manifest describing the platform capabilities, including memory mapping, computing services, and data-streaming interfaces. |
| software developer | deliver a program together with a manifest describing the node capabilities, including I/O port descriptions (data formats) and memory consumption. |
| system integrator  | use a GUI or the graph description language to create arcs between nodes and, progressively if needed, tune performance parameters such as memory overlays, FIFO sizes, and processor affinity. |



--------------------------------------

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



## GUI design tool

The compiled binary graph can be generated with graphical tool (prototyped in "stream_tools\gui"). The tool creates the compiled binary format and the intermediate text file for later manual tuning.

![c](GUI_PoC.png)

