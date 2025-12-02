/* ----------------------------------------------------------------------
 * top_manifest_computer.h
 * -------------------------------------------------------------------- */


#ifndef cPLATFORM_COMPUTER_H
#define cPLATFORM_COMPUTER_H
#ifdef PLATFORM_COMPUTER

 /*----- PLATFORM DEFINITIONS FOR "COMPUTER" ------------------------------------------------------------------------*/

#define GRAPH_FROM_PLATFORM   2         /* the graph comes from the platform */

/*------ Floating point allowed ------*/
#define STREAM_FLOAT_ALLOWED 1          /* selection of integer processing with float_t and double_t */

/* max number of nodes installed at compilation time */
#define NB_NODE_ENTRY_POINTS 30

/* max number of application callbacks used from NODE and scripts */
#define MAX_NB_APP_CALLBACKS 8

// to test the multiprocessing macro
#define MULTIPROCESSING 1     

/*
 * --- maximum number of processors using STREAM in parallel - read by the graph compiler
 */
#define STREAM_NB_PROCESSES 1           /* Max number of process(or) reeading the graph simultaneously */

#define STREAM_MAXNB_THREADS_PER_PROC 1
#define STREAM_MAXNB_PROCESSOR_PER_ARCH 2
#define STREAM_NB_ARCHITECTURES 1

 // MEMORY BANKS

#define MBANK_GRAPH     0               /* share graph base address */
#define MBANK_DMEMFAST  1               /* not shared DTCM Cortex-M/LLRAM Cortex-R, swapped between NODE calls if static */

#define SIZE_MBANK_DMEM_EXT    500000   /* general purpose          */
#define SIZE_MBANK_DTCM         1000    /* simulates DTCM           */
#define SIZE_MBANK_ITCM         1000    /* simulates ITCM           */
#define SIZE_MBANK_RETENTION     100    /* simulates retention      */


        /* warning : changing the indexes impacts the "top_graph_interface" of each graph.txt */
#define IO_PLATFORM_DATA_SINK        0 
#define IO_PLATFORM_DATA_IN_1        1 
#define IO_PLATFORM_SENSOR_0         2 
#define IO_PLATFORM_MOTION_IN_0      3 
#define IO_PLATFORM_AUDIO_IN_0       4 
#define IO_PLATFORM_2D_IN_0          5 
#define IO_PLATFORM_LINE_OUT_0       6 
#define IO_PLATFORM_GPIO_OUT_0       7 
#define IO_PLATFORM_GPIO_OUT_1       8 
#define IO_PLATFORM_DATA_OUT_0       9


//#define LAST_IO_FUNCTION_PLATFORM (IO_PLATFORM_DATA_OUT_0+1)  /* table of platform_io[io_al_idx] */

//#define MAX_IO_FUNCTION_PLATFORM 128     /* table of platform_io[io_al_idx] */

/*===========================================================================
 in platform_computer.c : 
    void platform_specific_long_offset(intptr_t long_offset[])
        long_offset[MBANK_GRAPH]    = (const intptr_t)&(MEXT[10]); 
        long_offset[MBANK_DMEMFAST] = (const intptr_t)&(TCM1[10]); 
*/
//#define MAX_NB_MEMORY_OFFSET 4
//#define CRITICAL_FAST_SEGMENT_IDX (MAX_NB_MEMORY_OFFSET-1)


/*----- NODES ENABLED FOR "COMPUTER" ------------------------------------------------------------------------*/
#define CODE_ARM_STREAM_SCRIPT          /* byte-code interpreter, index "arm_stream_script_INDEX" */
#define CODE_ARM_STREAM_ROUTER          /* copy input arcs and subchannel and output arcs and subchannels   */     
#define CODE_ARM_STREAM_MODULATOR       /* signal generator with modulation */
#define CODE_ARM_FIXEDBF                /* audio fixed direction beamformer */
#define CODE_ARM_STREAM_FILTER2D        /* Filter, rescale, rotate, exposure compensation */
#define CODE_ARM_STREAM_FILTER          /* cascade of DF1 filters */
#define CODE_ARM_STREAM_DEMODULATOR     /* signal demodulator, frequency estimator */
#define CODE_ARM_ANR                    /* audio noise reduction */
#define CODE_ARM_STREAM_AMPLIFIER       /* amplifier mute and un-mute with ramp and delay control */
#define CODE_SIGP_STREAM_COMPRESSOR     /* raw data compression with adaptive prediction */
#define CODE_SIGP_STREAM_DECOMPRESSOR   /* raw data decompression */
#define CODE_SIGP_STREAM_DETECTOR       /* estimates peaks/floor of the mono input and triggers a flag on high SNR */
#define CODE_SIGP_STREAM_DETECTOR2D     /* activity detection, pattern detection */
#define CODE_SIGP_STREAM_RESAMPLER      /* asynchronous sample-rate converter */
#define CODE_BITBANK_JPEGENC            /* bitbank_JPEGENC */
#define CODE_TJPGDEC                    /* TjpgDec */
#define CODE_SIGP_KWS                   /* yes/no keyword spotting */


/*----- SERVICES ENABLED FOR "COMPUTER" ------------------------------------------------------------------------*/
// SERV_GROUP_INTERNAL           /* 0  internal : Semaphores, DMA, Clocks */
    //#define PLATFORM_SERV_INTERNAL_SLEEP_CONTROL                              0
    //#define PLATFORM_SERV_INTERNAL_CPU_CLOCK_UPDATE                           1
    //#define PLATFORM_SERV_INTERNAL_READ_MEMORY                                2
    //#define PLATFORM_SERV_INTERNAL_READ_MEMORY_FAST_MEM_ADDRESS               3
    //#define PLATFORM_SERV_INTERNAL_SERIAL_COMMUNICATION                       4
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP      5
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_RD_BYTE_MP                6
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP                7
    //#define PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_CLEAR_BIT_MP              8
    //#define PLATFORM_SERV_INTERNAL_READ_TIME                                  9
    //#define PLATFORM_SERV_INTERNAL_READ_TIME64                               10
    //#define PLATFORM_SERV_INTERNAL_READ_TIME32                               11
    //#define PLATFORM_SERV_INTERNAL_READ_TIME16                               12
    //#define PLATFORM_SERV_INTERNAL_KEYEXCHANGE                               13

// SERV_GROUP_SCRIPT             /* 1  script : Node parameters  */

//#undef SERV_GROUP_CONVERSION         /* 2  Compute : raw conversions */
    //#undef PLATFORM_SERV_SERV_CONVERSION_INT16_FP32

//#undef SERV_GROUP_STDLIB             /* 3  Compute : malloc, string */
    //#undef PLATFORM_SERV_STREAM_ATOF    
    //#undef PLATFORM_SERV_STREAM_ATOI    
    //#undef PLATFORM_SERV_STREAM_MEMSET  
    //#undef PLATFORM_SERV_STREAM_STRCHR  
    //#undef PLATFORM_SERV_STREAM_STRLEN  
    //#undef PLATFORM_SERV_STREAM_STRNCAT 
    //#undef PLATFORM_SERV_STREAM_STRNCMP 
    //#undef PLATFORM_SERV_STREAM_STRNCPY 
    //#undef PLATFORM_SERV_STREAM_STRSTR  
    //#undef PLATFORM_SERV_STREAM_STRTOK  
    //#undef PLATFORM_SERV_STREAM_FREE    
    //#undef PLATFORM_SERV_STREAM_MALLOC  

//#undef SERV_GROUP_MATH               /* 4  math.h */
    //#undef PLATFORM_SERV_SERV_SQRT_Q15  
    //#undef PLATFORM_SERV_SERV_SQRT_F32  
    //#undef PLATFORM_SERV_SERV_LOG_Q15   
    //#undef PLATFORM_SERV_SERV_LOG_F32   
    //#undef PLATFORM_SERV_SERV_SINE_Q15  
    //#undef PLATFORM_SERV_SERV_SINE_F32  
    //#undef PLATFORM_SERV_SERV_COS_Q15   
    //#undef PLATFORM_SERV_SERV_COS_F32   
    //#undef PLATFORM_SERV_SERV_ATAN2_Q15 
    //#undef PLATFORM_SERV_SERV_ATAN2_F32 
    //#undef PLATFORM_SERV_SERV_SORT      

// SERV_GROUP_DSP_ML             /* 5  cmsis-dsp */
    //#undef PLATFORM_SERV_CHECK_COPROCESSOR    /* check for services() */
    //#undef PLATFORM_SERV_SERV_CHECK_END_COMP       /* check completion for the caller */
    //#undef PLATFORM_SERV_SERV_DFT_Q15              /* DFT/Goertzel windowing, module, dB */
    //#undef PLATFORM_SERV_SERV_DFT_F32            

            
    #define PLATFORM_SERV_DSP_CASCADE_DF1_Q15       /* IIR filters, use SERV_CHECK_COPROCESSOR */
    //#define PLATFORM_SERV_DSP_CASCADE_DF1_F32         /* take the default implementation */

    //#undef PLATFORM_SERV_SERV_WINDOW                
    //#undef PLATFORM_SERV_SERV_WINDOW_DB             
    //#undef PLATFORM_SERV_SERV_rFFT_Q15             /* RFFT windowing, module, dB , use SERV_CHECK_COPROCESSOR */
    //#undef PLATFORM_SERV_SERV_rFFT_F32             /* default FFT with tables rebuilded */
    //#undef PLATFORM_SERV_SERV_cFFT_Q15             /* cFFT windowing, module, dB */
    //#undef PLATFORM_SERV_SERV_cFFT_F32           

//#undef SERV_GROUP_DEEPL              /* 6  cmsis-nn */
    //#undef PLATFORM_SERV_STREAM_FC                 /* fully connected layer Mat x Vec */
    //#undef PLATFORM_SERV_STREAM_CNN                /* convolutional NN : 3x3 5x5 fixed-weights */

//#undef PLATFORM_SERV_SERV_GROUP_MM_AUDIO           /* 7 speech/audio processing */

//#undef PLATFORM_SERV_SERV_GROUP_MM_IMAGE           /* 8 image processing */


///* conditional compilation */
//#define STREAM_PLATFORM_SERVICES        /* call the platform service with its fast libraries w/wo accelerators */
    //#undef PLATFORM_SERV_SERV_STDLIB
    //#undef PLATFORM_SERV_SERV_EXTMATH
    //#undef PLATFORM_SERV_SERV_EXTAUDIO
    //#undef PLATFORM_SERV_SERV_EXTIMAGE





#ifdef _MSC_VER 
#define DATA_MEMORY_BARRIER
#define INSTRUCTION_SYNC_BARRIER
#else
#if MULTIPROCESSING == 1
#define DATA_MEMORY_BARRIER DMB()
#define INSTRUCTION_SYNC_BARRIER ISB()
#else
#define DATA_MEMORY_BARRIER 
#define INSTRUCTION_SYNC_BARRIER
#endif
#endif

#define WR_BYTE_MP_(address,x) { *(volatile uint8_t *)(address) = (x); DATA_MEMORY_BARRIER; }
#define RD_BYTE_MP_(x,address) { DATA_MEMORY_BARRIER; (x) = *(volatile uint8_t *)(address);}
#define CLEAR_BIT_MP(arg, bit) {((arg) = U(arg) & U(~(U(1) << U(bit)))); DATA_MEMORY_BARRIER; } 

#endif
#endif /* ifndef cPLATFORM_COMPUTER_H */