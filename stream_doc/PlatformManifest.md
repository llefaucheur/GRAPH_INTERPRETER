[TOC]
--------------------------------------
An example of computing graph is given in the picture below. The “nodes” (or called “nanoApps”) are processing data provided through “arcs”. Each arc’s stream is characterized by its conveyed data format (raw format, number of channels, interleaving options, time-stamps, sampling-rate, frame size). 



Graph of nodes for stream-based computing

The nodes at the boundary of the graph are called “IOs” (as Input / Output ports). The IOs are characterized by the physical domain of operation (connexion the application, image stream, audio, motion sensor, GPIO), the commander / servant protocol used with the platform AL (platform Abstraction Layer. When IOs are exchanging data they do it with a list of functions with an index named IO_AL_idx. 

A platform is characterized by the list of IO interfaces the graph can use with the IO_AL_idx indexes, by the list of processors and their memory banks pre-reserved for the execution of the graph, by the list of the nodes pre-installed before graph execution. The details of a platform, nodes and IOs are recorded in respective “manifests”. 

A graph description includes the presets and parameters of the nodes to use at graph reset time. When the parameter and states needs to be exchanged dynamically during the graph execution a script (picture) can be coupled before/after the execution of each node. The scripts consist in a compact byte-code language similar to the ones of old pocket calculators. A global script can be used for the interactions with the application (specific parameter settings during use-case transitions). A graph can be reused as a sub-graph of a more complex graph.



The Graph Interpreter is using two interfaces : one from the application and one to notify data move on IOs are finished. 

The first interface is : void arm_graph_interpreter (uint32_t command,  arm_stream_instance_t *instance, uint8_t *data, uint32_t size)
Where “command” tells to reset the graph, execute it, check boundary IOs filling state to move data in/out, set parameters. Instance is the memory allocated for the execution of the graph : a structure of pointers to the binary graph, to the installed nodes, to the AL stream interfaces functions (indexed with IO_AL_idx), some debug control fields.

The second interface is a call-back used to notify the end data moves with IOs :
    void arm_graph_interpreter_io_ack (uint8_t IO_AL_idx, uint8_t *data,  uint32_t data_size) 
The parameters of this function tell the “data” pointer with an amount of “size” bytes have been exchanged on the graph boundary with the AL interface indexed by IO_AL_idx. This function will read the binary graph information to find which arc circular buffer descriptor needs to be updated with this data move.

----------------------------
# graph instance 
FW IO IDX [] => 
NODES [] => 

----------------------------

Platform manifest
The “VID” (Virtual Identifier) index is used to translate the graph memory map addresses to physical addresses. This is a memory plane used to have compact representation of 64bits addresses and to help multiprocessors pointing to the same physical addresses even if they have address translators.
The platform digital manifest gives the base address and sizes of the memory planes addressed with up to 8 IDs, each memory plane has multiple VID corresponding to physical memory blocks. By convention the VID index 0 is used for the shared RAM holding the graph’s arc FIFO descriptors (read and write index indexes to buffers).
A system integrator can avoid specifying the VID memory mapping and let the graph compiler manage. Tuning the performance means taking care of overlays, or arranging processors don’t have simultaneous access to the same physical memory banks, and this is where VID indexes are used.

Paths

3                                      
../../stream_platform/                 
../../stream_platform/windows/manifest/
../../stream_nodes/                    


Digital manifests

1   procmap_manifest_computer.txt       path index + file name

List IO manifest files

10  

;Path      Manifest         IO_AL_idx ProcCtrl clock-domain
1   io_platform_data_in_0.txt       0     1        0       
1   io_platform_data_in_1.txt       1     1        0       
1   io_platform_analog_sensor_0.txt 2     1        0       
1   io_platform_motion_in_0.txt     3     1        0       
1   io_platform_audio_in_0.txt      4     1        0       
1   io_platform_2d_in_0.txt         5     1        0       
1   io_platform_line_out_0.txt      6     1        0       
1   io_platform_gpio_out_0.txt      7     1        0       
1   io_platform_gpio_out_1.txt      8     1        0       
1   io_platform_data_out_0.txt      9     1        0       

List of nodes manifests

20                          
2                          node_manifest_none.txt          
2         Basic/arm/script/node_manifest_graph_control.txt 
2         Basic/arm/script/node_manifest_script.txt        
2         Basic/arm/router/node_manifest_router.txt        
2      Basic/arm/converter/node_manifest_converter.txt     
2      Basic/arm/amplifier/node_manifest_amplifier.txt     
2          Basic/arm/mixer/node_manifest_mixer.txt         
2         Audio/arm/filter/node_manifest_filter.txt        
2       Audio/arm/detector/node_manifest_detector.txt      
2       Basic/arm/rescaler/node_manifest_rescaler.txt      
2     Audio/arm/compressor/node_manifest_compressor.txt    
2   Audio/arm/decompressor/node_manifest_decompressor.txt  
2      Basic/arm/modulator/node_manifest_modulator.txt     
2    Basic/arm/demodulator/node_manifest_demodulator.txt   
2      Basic/arm/resampler/node_manifest_resampler.txt     
2            Basic/arm/qos/node_manifest_qos.txt           
2          Basic/arm/split/node_manifest_split.txt         
2     image/arm/detector2D/node_manifest_detector2D.txt    
2       image/arm/filter2D/node_manifest_filter2D.txt      
2       Basic/arm/analysis/node_manifest_analysis.txt      



# Digital manifest

; ------------------------------------------------------------------------------------------------------------
; Processor and memory configuration + default memory mapping 
; ------------------------------------------------------------------------------------------------------------
;

    1 1 9  number of architectures, number of processors, number of memory banks

;   memory banks:
;   - ID        base offset ID reference above
;   - VID       virtual ID used in the graph for manual mapping, must stay below 99 for swap controls (see NodeTemplate.txt)
;   - S         0=any/1=normal/2=fast/3=critical-Fast, 
;   - W         static0/working1/retention2,
;   - P         shared0/private1,
;   - H         DMAmemHW1
;   - D         Data0/Prog1/Both2 
;   - Size      minimum sizes guaranteed per VID starting from @[ID]+offset below
;   - Offset    maximum offset from the base offset ID, (continuous banks means = previous size + previous offset)

;   the memory is further split in the graph "top_memory_mapping" to ease mapping and overlays

;   ID VID  S W P H D     Size offset from offsetID 
    0   0   1 0 0 0 0    95526 10       VID0=DEFAULT flat memory bank, can overlap with the others
    0   1   0 0 0 0 0    65526 10       SRAM0 static, hand tuned memory banks
    0   2   0 0 0 0 0    30000 65536    SRAM1 static
    0   3   0 1 0 0 0    15000 95536    SRAM1 working at application level
    0   4   0 1 0 0 0   256000 262144   DDR working at application level
    2   5   3 1 1 0 0     1024 262144   DTCM Private memory of processor 1 
    1  10   0 2 0 0 0     1024 524288   Retention memory
    3  20   0 0 0 0 0   200000 10       Data in Flash
    2   8   3 1 1 0 1    16384 0        ITCM Private memory of processor 1 


;   memory offsets ID used by all processors and physical address seen from the main processor
;   0       h20000000   image of "platform_specific_long_offset(intPtr_t long_offset[])" 
;   1       h28000000       in stream_al/platform_XXXXX.c
;   2       h2C000000   TCM Private memory of processor 1 
;   3       h08000000   Internal Flash   

;----------------------------------------------------------------------------------------
; all architectures
    ;   all processors (processor IDs >0)
        ;------------------------------------------------------------
        1 1 15      processor ID, boolean "I am the main processor" allowed to boot the graphs
        ;           Bit-field computation firmware extensions, on top of the basic one, embedded in Stream services
        ;                 EXT_SERVICE_MATH 1, EXT_SERVICE_DSPML 2, EXT_SERVICE_AUDIO 3, EXT_SERVICE_IMAGE 4






# void platform_init_stream_instance(arm_stream_instance_t *S)
{
extern p_stream_node node_entry_point_table[];
extern const uint32_t graph_input[];
extern const p_stream_al_services application_callbacks[];
    uint32_t PIOoffsetWords;

#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 1

    S->graph = (uint32_t *) &(graph_input[1]);      /* binary graph address loaded in the graph interpreter instance */ 
    S->long_offset = long_offset;                   /* there is one single graph executed per platform */ 
    
    S->scheduler_control = PACK_STREAM_PARAM(
            STREAM_MAIN_INSTANCE,
            STREAM_NB_INSTANCE,
            STREAM_COLD_BOOT,
            STREAM_SCHD_NO_SCRIPT, 
            STREAM_SCHD_RET_END_ALL_PARSED
            );
    
    ST(S->whoami_ports, ARCHID_PARCH, ARCH_ID);  /* 3 fields used for NODE locking */
    ST(S->whoami_ports, PROCID_PARCH, PROC_ID);
    ST(S->whoami_ports, PRIORITY_PARCH, STREAM_INSTANCE_LOWLATENCYTASKS);
    ST(S->whoami_ports, NODE_W32OFF_PARCH, 0);   /* index in the linked list */
    S->al_services = al_service;
    S->application_callbacks = application_callbacks;
    S->node_entry_point_table = &(node_entry_point_table[0]);
    S->platform_io = platform_io;
    
    platform_init_copy_graph (S);
    
    S->iomask = (1 << (RD((S->graph)[1], NB_IOS_GR1))) -1;      // IOs are enabled                                    
    
    PIOoffsetWords = GRAPH_HEADER_NBWORDS                       // Header
                   + RD((S->graph)[1], NB_IOS_GR1);             // PIO settings
    S->script_offsets = &(S->graph[PIOoffsetWords]);
    
    PIOoffsetWords += RD((S->graph)[1], SCRIPTSSZW32_GR1);       // Scripts
    S->linked_list = &(S->graph[PIOoffsetWords]);
    S->linked_list_ptr = S->linked_list;
    
    PIOoffsetWords += RD((S->graph)[2], LINKEDLISTSZW32_GR2);   // linked list
    S->ongoing = (uint8_t *) &(S->graph[PIOoffsetWords]);
    
    PIOoffsetWords += RD((S->graph)[2], ARC_DEBUG_IDX_GR2) + ((3 + RD((S->graph)[1], NB_IOS_GR1))>>2);
    S->all_formats = &(S->graph[PIOoffsetWords]);               // PIO is an array of Bytes of size NB_IOS_GR1/4 words
    
    PIOoffsetWords += RD((S->graph)[1], NBFORMATS_GR1) * STREAM_FORMAT_SIZE_W32;
    S->all_arcs = &(S->graph[PIOoffsetWords]);
    
    /* @@@ TODO  check scheduler_control S->scheduler_control .BOOT_SCTRL to clear 
        the backup memory area STREAM_COLD_BOOT / STREAM_WARM_BOOT
     */
    /* if the application sets the debug option then don't use the one from the graph
        and the application sets the scheduler return option in platform_init_stream_instance() 
     */
    if (STREAM_SCHD_NO_SCRIPT == RD(S->scheduler_control, SCRIPT_SCTRL))
    {   uint32_t debug_script_option;
        debug_script_option = RD((S->graph)[3], SCRIPT_SCTRL_GR3);
        ST(S->scheduler_control, SCRIPT_SCTRL, debug_script_option);
    }
    
    /* does the graph is deciding the return-from-scheduler option */
    if (STREAM_SCHD_RET_NO_ACTION == RD(S->scheduler_control, RETURN_SCTRL))
    {   uint32_t return_script_option;
        return_script_option = RD((S->graph)[3], RETURN_SCTRL_GR3);
    
        /* the return option is undefined : return when no more data is available */
        if (return_script_option == 0) 
        {   return_script_option = STREAM_SCHD_RET_END_NODE_NODATA;
        }
        ST(S->scheduler_control, RETURN_SCTRL, return_script_option);
    }
    
    S->linked_list_ptr = S->linked_list;
    
    ST(S->scheduler_control, INSTANCE_SCTRL, 1); /* this instance is active */ 
    
    //@@@ TODO for multiprocessing 
    //if (RD(S->scheduler_control, MAININST_SCTRL)) 
    //{   /* all other process can be released from wait state */
    //    platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); 
    
    platform_init_io (S);
    
    //@@@ TODO for multiprocessing 
    //} else
    //{   /* wait until the graph is copied in RAM */
    //    uint8_t wait; 
    //    do {
    //        platform_al (PLATFORM_MP_BOOT_WAIT, &wait, 0,0);
    //    } while (0u != wait);
    //}
}

 


/**
  @brief        Graph copy in RAM (STREAM_RESET phase)
  @param[in]    instance   global data of this instance
  @param[in]    graph      input graph
  @return       none

  @par          Initialization of the Graph interpreter instance 
                   long offsets, address of nodes, arcs, stream formats

                The Graph is a table using uint32 :
                - size of the graph in Words
                [0] 28b RAM address of part/all the graph, HW-shared MEM configuration, which part is going in RAM
                [1] number of FORMAT, size of SCRIPTS
                [2] size of LINKEDLIST, number of STREAM_INSTANCES
                [3] number of ARCS, number of DEBUG registers
    
                The application shares a pointer, the register [0] tells the graph is either:
                - already in RAM : nothing to do
                - to move in RAM (internal / external) partially or totally
                
                platform_init_copy_graph() implements this data move and calls platform AL to synchronize the other processor 
                to let them initialize the node instances in parallel.
  @remark
 */
# void platform_init_copy_graph(arm_stream_instance_t *S)
{
    uint8_t RAMsplit;
    uint32_t graph_words_copy_in_ram, PIOoffsetWords;
    uint32_t *graph_src, *graph_dst;

    /* 
        Graph is read from Flash and a small part is copied in RAM
        to save RAM (for example Cortex-M0 with 2kB of internal SRAM)
    
                   RAMsplit Option 0      1      2:already in RAM at graph0 address
        IO settings                RAM    Flash  
        SCRIPTS                    RAM    Flash  
        LINKED-LIST                RAM    Flash   RAM allows NODE to be desactivated
        PIO                        RAM    RAM    
        FORMAT x3 words            RAM    RAM
        ARC descriptors 4 words    RAM    RAM    
        Debug registers, Buffers   RAM    RAM    
     */


    graph_dst = (uint32_t *)GRAPH_RAM_OFFSET_PTR(L,(S->graph));
    PIOoffsetWords = GRAPH_HEADER_NBWORDS                       // Header
                   + RD((S->graph)[1], NB_IOS_GR1)              // PIO settings
                   + RD((S->graph)[1], SCRIPTSSZW32_GR1)        // Scripts
                   + RD((S->graph)[2], LINKEDLISTSZW32_GR2);    // linked list
    
    RAMsplit = (uint8_t)RD(((S->graph)[0]),RAMSPLIT_GR0);
    
    switch (RAMsplit)
    {   default :
            graph_src = &((S->graph)[0]);
            graph_words_copy_in_ram = 0;
            break;
        case COPY_CONF_GR0_COPY_ALL_IN_RAM: 
            graph_src = &((S->graph)[0]);
            graph_words_copy_in_ram = S->graph[-1]; 
            S->graph = graph_dst;                       // take the new address after copy 
            break;
        case COPY_CONF_GR0_FROM_PIO: 
            graph_src = &((S->graph)[PIOoffsetWords]);
            graph_words_copy_in_ram = S->graph[-1] - PIOoffsetWords; 
            break; 
    }
    
    /* finalize the copy by telling the other processors */   
    {   uint32_t i; 
        for (i=0;i<graph_words_copy_in_ram;i++)
        {   graph_dst[i]=graph_src[i];
        }
    }    
    
    /* copy the graph data to uint8_t platform_io_al_idx_to_graph[LAST_IO_FUNCTION_PLATFORM]; 
        to ease the translation from graph index to graph_io_idx used in arm_graph_interpreter_io_ack() 
    */
    {   uint32_t i_graph_io_idx, tmpi, tmpn;
        tmpn = RD((S->graph)[1], NB_IOS_GR1);
        for (i_graph_io_idx = 0; i_graph_io_idx < tmpn; i_graph_io_idx++)
        {   
            tmpi = (S->graph)[PIOoffsetWords + i_graph_io_idx];
            tmpi = RD(tmpi, FWIOIDX_IOFMT0);
            platform_io_al_idx_to_graph[tmpi] = i_graph_io_idx;
        }
    }
}


/**
  @brief        Initialization and start of the IOs 
  @param[in]    instance   global data of this instance
  @return       none

  @par          Read the IOs descriptor the instance is in charge of
                Interface to platform-specific stream controls (set, start)
  @remark
 */
# void platform_init_io(arm_stream_instance_t *S)
{
    uint32_t nio;
    uint8_t graph_io_idx;
    uint8_t *ongoing;
    uint32_t *pio_control;
    uint32_t stream_format_io_setting;
    uint32_t io_mask;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    const p_io_function_ctrl *io_func;
    const p_stream_al_services *al_func;

    extern arm_stream_instance_t * platform_io_callback_parameter;
    /*-------------------------------------------*/
    
    /* if cold start : clear the backup area */
    if (TEST_BIT(S->scheduler_control, BOOT_SCTRL_LSB) == STREAM_COLD_BOOT)
    {   al_func = &(S->al_services[0]);
        (*al_func)(PACK_AL_SERVICE(0,PLATFORM_CLEAR_BACKUP_MEM,0), 0,0,0,0);
    }
    
    /* wait all the process have initialized the graph */
    {   //uint8_t wait; 
        //do {
        //    platform_al (PLATFORM_MP_RESET_WAIT, &wait, 0, RD(S->scheduler_control, NBINSTAN_SCTRL));
        //} while (wait == 0);
    }
    
    /*------------------------------------------- 
        initialization of the graph IO ports 
    */     
    io_mask = S->iomask;
    
    nio = RD(S->graph[1],NB_IOS_GR1);
    all_arcs = S->all_arcs;


    /* the Graph interpreter instance holds the memory pointers */
    platform_io_callback_parameter = S;
    
    for (graph_io_idx = 0; graph_io_idx < nio; graph_io_idx++)
    {
        uint8_t *address;
        uint32_t size;
        pio_control = &(S->graph[GRAPH_HEADER_NBWORDS + graph_io_idx * STREAM_IOFMT_SIZE_W32]);
        ongoing = &(S->ongoing[graph_io_idx]);
    
        /* does this port is managed by the Graph interpreter instance ? */
        if (0 == (io_mask & (1U << graph_io_idx))) 
            continue; 
    
        /* default value settings */
        stream_format_io_setting = S->graph[GRAPH_HEADER_NBWORDS + graph_io_idx];
        io_func = &(S->platform_io[RD(*pio_control, FWIOIDX_IOFMT0)]);
        if (*io_func == 0) 
        {   continue;
        }
        (*io_func)(STREAM_RESET, (uint8_t *)&stream_format_io_setting, 0);
    
        /* 
            IO-Interface expects the buffer to be declared by the graph 
        */
        if (0 != TEST_BIT(*pio_control, FROMIOBUFF_IOFMT0_LSB))
        {
            iarc = RD(*pio_control, IOARCID_IOFMT0);
            iarc = SIZEOF_ARCDESC_W32 * iarc;
            address = (uint8_t *)pack2linaddr_ptr(S->long_offset, all_arcs[iarc + BUF_PTR_ARCW0], LINADDR_UNIT_W32);
            size = (uint32_t)RD(all_arcs[iarc + BUFSIZDBG_ARCW1], BUFF_SIZE_ARCW1);
    
            io_func = &(S->platform_io[RD(*pio_control, FWIOIDX_IOFMT0)]);
            (*io_func)(STREAM_SET_BUFFER, address, size);
        }
    } 
}