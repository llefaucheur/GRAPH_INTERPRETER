/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_services.c
 * Description:  computing services offered to computing nodes 
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "stream_common_const.h"
#include "presets.h"

#ifdef __cplusplus
 extern "C" {
#endif



#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"      
#include "stream_types.h"


void generic_biquad_cascade_df1_init_q15(
    generic_biquad_cascade_df1_inst_q15* S,
    uint8_t numStages,
    const int16_t* pCoeffs,
    int16_t* pState,
    int8_t postShift)
{
}
void generic_biquad_cascade_df1_fast_q15(
    const generic_biquad_cascade_df1_inst_q15* S,
    const int16_t* pSrc,
    int16_t* pDst,
    uint32_t blockSize)
{
}

//void generic_biquad_cascade_df1_init_f32(
//    biquad_cascade_df1_inst_f32* S,
//    uint8_t numStages,
//    const float* pCoeffs,
//    float* pState)
//{
//}
void generic_biquad_cascade_df1_fast_f32(
    generic_biquad_cascade_df1_inst_f32* S,
    const float* pSrc,
    float* pDst,
    uint32_t blockSize)
{
}

/* ------------------------------------------------------------------------------------------------------------
  @brief        Size of raw data
  @param[in]    raw type
  @return       size in bits
      
  @remark
 */

int32_t stream_bitsize_of_raw(uint8_t raw)
{
    switch (raw)
    {
        /* one bit per data */
    case STREAM_S2:     case STREAM_U2:     return 2;
    case STREAM_S4:     case STREAM_U4:     return 4;
    case STREAM_S8:     case STREAM_U8:     case STREAM_FP8_E4M3:   case STREAM_FP8_E5M2:   return 8;
    case STREAM_S16:    case STREAM_U16:    case STREAM_FP16:       case STREAM_BF16:       return 16;
    case STREAM_S23:    return 24;
    case STREAM_S32:    case STREAM_U32:    case STREAM_CS16:       case STREAM_FP32:       case STREAM_CFP16: return 32;
    case STREAM_S64:    case STREAM_U64:    case STREAM_FP64:       case STREAM_CFP32:      return 64;
    case STREAM_FP128:  case STREAM_CFP64:  return 128;
    case STREAM_FP256:  return 256;
    default: return 0;
    }
}


/* ------------------------------------------------------------------------------------------------------------
  @brief        ITOAB integer to ASCII with Base (binary, octal, decimal, hexadecimal)
  @param[in]    integer
  @param[out]   string of char
  @return       strlen      

  @remark       usage : char string[10]; string[itaob(string,1234,C_BASE10)]='/0';
 */

uint8_t itoab(char *s, int32_t n, int base)
{
    uint8_t sign, nc, i, j, c;

     if ((sign = (uint8_t)n) < 0)  /* save the sign */
     {    n = -n;
     }
    
     nc = 0;    /* generate digits in reverse order */
     do {       
         s[nc++] = "0123456789ABCDEF"[n % base];
     } while ((n /= base) > 0);     

     if (sign < 0)
     {  s[nc++] = '-';
     }

     /* reverse the charracters order */
     for (i = 0, j = nc-1; i < j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
     return nc; /* s[nc] = '\0'; to do out of the subroutine*/
}    



/**
  @brief        Internal services entry point 
  @param[in]    instance   pointers to the Stream instance and graph data
  @param[in]    command    Bit-field of command (see enum stream_command)
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  
  @return       none
       
  @remark
 */
static void arm_stream_services_internal(uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{
    switch (RD(command, FUNCTION_SSRV))
    {   
        //  multiprocessing mutual exclusion services 
        //  (*al_func)(PACK_SERVICE(0,0,0,SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP,SERV_GROUP_INTERNAL), 
        //    S->pt8b_collision_arc, &check, &whoAmI, 0);
        //   
        case SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP:
        {
            #ifdef PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_AND_CHECK_MP

            #else
            volatile uint8_t *pt8 = (uint8_t *)ptr1;
            volatile uint8_t *returned_flag = (uint8_t *)ptr2;
            volatile uint8_t *whoAmI = (uint8_t *)ptr3;

            /* attempt to reserve the node */
            *pt8 = *whoAmI;    

            /* check collision with all the running processes using the equivalent of lock() and unlock() 
               Oyama Lock, "Towards more scalable mutual exclusion for multicore architectures" by Jean-Pierre Lozi */
            INSTRUCTION_SYNC_BARRIER;
            DATA_MEMORY_BARRIER;

            *returned_flag = (*pt8 == *whoAmI);
            #endif
            break;
        }

        case SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP:
        {
            #ifdef PLATFORM_SERV_INTERNAL_MUTUAL_EXCLUSION_WR_BYTE_MP

            #else
            volatile uint8_t *pt8 = (uint8_t *)ptr1;
            volatile uint8_t *data = (uint8_t *)ptr2;
            
            *(volatile uint8_t *)(pt8) = (*data); 
            DATA_MEMORY_BARRIER; 
            #endif
            break;
        }

        case SERV_INTERNAL_MUTUAL_EXCLUSION_RD_BYTE_MP:
        {   volatile uint8_t *pt8 = (uint8_t *)ptr1;
            volatile uint8_t *data = (uint8_t *)ptr2;
         
            DATA_MEMORY_BARRIER; 
            (*data) = *(volatile uint8_t *)(pt8);
            break;
        }

        case SERV_INTERNAL_MUTUAL_EXCLUSION_CLEAR_BIT_MP:
        {   volatile uint8_t *pt8b = (uint8_t *)ptr1;
         
            ((*pt8b) = U(*pt8b) & U(~(U(1) << U(n)))); 
            DATA_MEMORY_BARRIER;
            break;
        }

        /* at reset time : key exchanges to deobfuscate node's firmware + 
            graph/user activation key to activate specific features  
         */
        case SERV_INTERNAL_KEYEXCHANGE : 
        {   const uint32_t platform_private_key [2] = { 12, 13 }; //{ 2452526671,  1812651256 };
            uint32_t **dst;
            dst = (uint32_t **)ptr1;
            *dst = (uint32_t *) platform_private_key;
            break;
        }
    }
}

/**
  @brief        data flow services entry point 
  @param[in]    instance   pointers to the Stream instance and graph data
  @param[in]    command    Bit-field of command (see enum stream_command)
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  

  @return       none
  @par          
  @remark
 */
static void arm_stream_services_script (uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{
    // SECTIONS OF ARC APIs
    /*
    * Test-pattern injection from scripts and result verification
    * Test availability of data (R/W)
    * Read/write to arcs, return the pointer to read/write for scripts
    * Data moves w/wo DMA
    */
    switch (RD(command, FUNCTION_SSRV))
    {   case SERV_SCRIPT_NODE:          /* called during STREAM_NODE_DECLARATION to register the NODE callback */
        {   
            #ifndef _MSC_VER 
            //rtn_addr = __builtin_return_address(0); // check the lr matches with the node 
            #endif
            break;
        }

        case SERV_SCRIPT_SCRIPT:
        {
            break; //"""""""""""""""""""""
        }
        /* ----------------------------------------------------------------------------------
            arm_stream_services(PACK_SERVICE(instance index, NOTAG_SSRV,  SERV_SCRIPT_DEBUG_TRACE), *string_address, 0, nb bytes);

            1: trace type bits 0,1,2(3) = OPTION_SSRV
                bit0: bit-data on bit3, 
                bit1: byte-data = TAG_SSRV 
                bit2:string of characters at the address of parameter

            2: address of the string of characters

         */
        case SERV_SCRIPT_DEBUG_TRACE:
        {   break;
        }

        /* toggle a flag to insert/remove the time-stamps on each data pushed in the debug trace */
        case SERV_SCRIPT_DEBUG_TRACE_STAMP:
        {   break;
        }

        /* stream format of an OUTPUT arc is changed on-the-fly : 
            update bit-fields of nchan, FS, units, interleaving, audio mapping, RAW format */
        case SERV_SCRIPT_FORMAT_UPDATE:
        {   /* checks the index of the NODE arc and update the format for the format converter or the next consumer */ 
            break;
        }

    }
}


/**
  @brief        subset of the standard library services entry point
  @param[in]    instance   pointers to the Stream instance and graph data
  @param[in]    command    Bit-field of command (see enum stream_command)
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  

  @return       none
  @par
  @remark
 */

void arm_stream_services_stdlib (uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{
    switch (RD(command, FUNCTION_SSRV))
    {
    case STREAM_MALLOC: /* (STREAM_MALLOC + OPTION_SSRV(align, static/w/retention, speed), **ptr1, 0, 0, n) */
        break;
    }
#ifdef SERV_STDLIB
	switch (RD(command, FUNCTION_SSRV))
    {
    case STREAM_FREE:
    case STREAM_RAND:   /* (STREAM_RAND + OPTION_SSRV(seed), *ptr1, 0, 0, n) */
    case STREAM_SRAND:
    case STREAM_ATOF:
    case STREAM_ATOI:
    case STREAM_MEMSET:
    case STREAM_STRCHR:
    case STREAM_STRLEN:
    case STREAM_STRNCAT:
    case STREAM_STRNCMP:
    case STREAM_STRNCPY:
    case STREAM_STRSTR:
    case STREAM_STRTOK:
        break;
    }
#endif
}

/**
  @brief        MATH services entry point
  @param[in]    instance   pointers to the Stream instance and graph data
  @param[in]    command    Bit-field of command (see enum stream_command)
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  

  @return       none
  @par
  @remark
 */
void arm_stream_services_math (uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{
    /* 
        Permanent APIs whatever "SERV_EXTMATH" are 
    //STREAM_SIN_Q15 STREAM_COS_Q15 STREAM_LOG10_Q15, STREAM_SQRT_Q15,
    */
    /* From Android CHRE  https://source.android.com/docs/core/interaction/contexthub
    String/array utilities: memcmp, memcpy, memmove, memset, strlen
    Math library: Commonly used single-precision floating-point functions:
    Basic operations: ceilf, fabsf, floorf, fmaxf, fminf, fmodf, roundf, lroundf, remainderf
    Exponential/power functions: expf, log2f, powf, sqrtf
    Trigonometric/hyperbolic functions: sinf, cosf, tanf, asinf, acosf, atan2f, tanhf
    */
#ifdef SERV_EXTMATH
    //STREAM_SIN_FP32,  STREAM_COS_FP32, STREAM_ASIN_FP32, STREAM_ACOS_FP32, 
    //STREAM_TAN_FP32,  STREAM_ATAN_FP32, STREAM_ATAN2_FP32, 
    //STREAM_LOG10_FP32,STREAM_LOG2_FP32, STREAM_POW_FP32, STREAM_SQRT_FP32, 
#endif
}


/**
  @brief        multimedia audio services entry point
  @param[in]    instance   pointers to the Stream instance and graph data
  @param[in]    command    Bit-field of command (see enum stream_command)
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  

  @return       none
  @par
  @remark
 */
void arm_stream_services_mm_audio (uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{
#ifdef SERV_EXTAUDIO

#endif
}


/**
  @brief        image processing services entry point
  @param[in]    instance   pointers to the Stream instance and graph data
  @param[in]    command    Bit-field of command (see enum stream_command)
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  

  @return       none
  @par
  @remark
 */
void arm_stream_services_mm_image (uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{
#ifdef SERV_EXTIMAGE

#endif
}


/**
  @brief        remote debugger command interpreter (set/read parameters)
  @param[in]    
  @param[in]    
  @param[in]    
  @param[in]    
 
  @return       none
  @par
  @remark       Un/Pack data from UART 7bits format, interleave long/short answers (<100Bytes) 
                header, address, function, data[], LRCcheck, end
                2 channels interleaving, time-stamps, 6bits ASCII format payload = 192bits x n
 */
void arm_stream_command_interpreter (uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{
}





/* 
    services
    - FFT : parameters tell if tweedle factors need to be moved in TCM
    - IIR : if the number of samples to filter is small, then take the hypothesis the caller
            made the good choices for the memory mapping of memory and coefficients
            else check the address are in TCM, apply a swap to a Stream scratch are execute
            the filtering from TCM, restore / swap
    - Matrices / dotProduct : strategy to define

*/

/**
  @brief        Service entry point for nodes

  @param[in]    command    Bit-field of domain, function, options, instance (see stream_service_command) 
  @param[in]    ptr1       input data pointer or stream_xdmbuffer pointer
  @param[in]    ptr2       output data pointer
  @param[in]    ptr3       parameters data pointer
  @param[in]    n          length of data
  

  @return       none

  @par          Services of DSP/ML computing, access to stdlib, advanced DSP operations (Codec)
                and data stream interface (debug trace, access to additional arcs used for 
                control and metadata reporting).
                arm_stream_services() uses a static memory area, "SERVICES_RAM", placed after the buffers

                There are 16 families of services(COMMAND_CMD) and 256 functions per family (TAG_CMD).
                Services are an abstraction layer to CMSIS-DSP/NN and the device implementation
                   using accelerators and custom instructions. 
                For example a NODE delivered in binary for Armv7-M architecture profile will 
                    automatically scale in DSP performance when executed on Armv8.1-M through services.

                Compute services can be called from any place in the code
                Sensitive services (data moves, key exchanges, spinlock access, ..) must be called from 
                    the same placed registered at reset time with SERV_SCRIPT_SECURE_ADDRESS

  @remark
 */
void arm_stream_services (uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n) 
{   
    //arm_stream_instance_t *pinst;

    /* max 16 groups of commands   PACK_SERVICE(COMMAND,OPTION,TAG,FUNC,GROUP) */
	switch (RD(command, GROUP_SSRV))
    {
    //enum stream_service_group
    case SERV_GROUP_INTERNAL:
        /* ----------------------------------------------------------
              Command                     C           COMMAND_SSRV
              Option                       o           OPTION_SSRV
              TAG                           tt            TAG_SSRV
              sub Function                    f      SUBFUNCT_SSRV
              Function                         FF    FUNCTION_SSRV
              Service Group                      g      GROUP_SSRV
           ----------------------------------------------------------
        */
        arm_stream_services_internal(command, ptr1, ptr2, ptr3, n);
        break;

    case SERV_GROUP_SCRIPT:
        arm_stream_services_script (command, ptr1, ptr2, ptr3, n);
        break;

    case SERV_GROUP_STDLIB:
        arm_stream_services_stdlib(command, ptr1, ptr2, ptr3, n);
        break;

    case SERV_GROUP_MATH:
        arm_stream_services_math(command, ptr1, ptr2, ptr3, n);
        break;

    case SERV_GROUP_DSP_ML:
            /*  
                - IIR-DF1 biquad filter cascade, Cortex-M0's CMSIS-DSP arm_biquad_cascade_df1_q15
                - The spectral comuputation (cFFT, rFFT, DFT, window, module, dB)
                - Raw datatype conversion (int/float)
                - Matrix operations
            */
            switch (RD(command, FUNCTION_SSRV))
            {
            case SERV_DSP_CHECK_END_COMP:
                *(uint8_t *)ptr1 = 1;                      /* return a completion flag */
                break;

            case SERV_DSP_CASCADE_DF1_Q15:          /* IIR filters arm_biquad_cascade_df1_fast_q15*/
                #ifdef PLATFORM_SERV_DSP_CASCADE_DF1_Q15

                    if (RD(command,  COMMAND_SSRV) == SERV_DSP_INIT)
                    {   
                        extern df1_q15_init platform_biquad_cascade_df1_init_q15;
                        //typedef void    (biquad_cascade_df1_q15_init) (
                        //        biquad_cascade_df1_inst_q15 * S,
                        //        uint8_t numStages,                n >> 8
                        //  const int16_t * pCoeffs,
                        //        int16_t * pState,
                        //        int8_t postShift );               n & 0xFF
                        platform_biquad_cascade_df1_init_q15((generic_biquad_cascade_df1_inst_q15*)ptr1, (uint8_t)(n >> 8), (const int16_t *)ptr2, (int16_t *)ptr3, (int8_t)n);

                    } else //(RD(command,  COMMAND_SSRV) == SERV_RUN)
                    {   
                        extern df1_q15 platform_biquad_cascade_df1_q15;
                        //typedef void    (biquad_cascade_df1_q15) (
                        //  const biquad_cascade_df1_inst_q15 * S,
                        //  const int16_t * pSrc,
                        //        int16_t * pDst,
                        //        uint32_t blockSize );
                        platform_biquad_cascade_df1_q15((generic_biquad_cascade_df1_inst_q15*)ptr1, (int16_t *)ptr2, (int16_t *)ptr3, (uint32_t)n);
                    }
                #else
                    if (RD(command,  COMMAND_SSRV) == SERV_DSP_INIT)
                    {
                        extern df1_q15_init generic_biquad_cascade_df1_init_q15;
                        generic_biquad_cascade_df1_init_q15(                // void arm_biquad_cascade_df1_init_q15(
                            (generic_biquad_cascade_df1_inst_q15 *) ptr1,   //         biquad_cascade_df1_inst_q15 * S,
                            n & 0xFF,                                       //         uint8_t numStages,
                            (const int16_t *) ptr2,                         //   const q15_t * pCoeffs,
                            (int16_t*) ptr3,                                //         q15_t * pState,
                            (uint8_t)(n >> 8));                             //         int8_t postShift)

                    } else //(RD(command,  COMMAND_SSRV) == SERV_DSP_RUN)
                    {
                        extern df1_q15 generic_biquad_cascade_df1_fast_q15;
                        generic_biquad_cascade_df1_fast_q15(                // void stream_filter_arm_biquad_cascade_df1_fast_q15(
                            (const generic_biquad_cascade_df1_inst_q15*) ptr3, //   const arm_biquad_cascade_df1_inst_q15 * S,
                            (const int16_t*) ptr1,                          //   const q15_t * pSrc,
                            (int16_t*) ptr2,                                //         q15_t * pDst,
                            (uint32_t)n);                                   //         uint32_t blockSize)
                    }
                #endif
                break;
            case SERV_DSP_CASCADE_DF1_F32:          /* IIR filters arm_biquad_cascade_df1_f32*/
              #ifdef PLATFORM_SERV_DSP_CASCADE_DF1_F32
                    if (RD(command,  COMMAND_SSRV) == SERV_DSP_INIT)
                    {   //typedef void    (biquad_cascade_df1_f32_init) (
                        //        biquad_cascade_df1_inst_f32 * S,
                        //        uint8_t numStages,                n >> 8
                        //  const int16_t * pCoeffs,
                        //        int16_t * pState,
                        //        int8_t postShift );               n & 0xFF
                        platform_biquad_cascade_df1_init_f32((biquad_cascade_df1_inst_f32 *)ptr1, (uint8_t)(n >> 8), (uint16_t *)ptr2, (int16_t *)ptr3, (int8_t)n);

                    } else //(RD(command,  COMMAND_SSRV) == SERV_RUN)
                    {   //typedef void    (biquad_cascade_df1_f32) (
                        //  const biquad_cascade_df1_inst_f32 * S,
                        //  const int16_t * pSrc,
                        //        int16_t * pDst,
                        //        uint32_t blockSize );
                        platform_biquad_cascade_df1_f32((biquad_cascade_df1_inst_q15 *)ptr1, (int16_t *)ptr2, (int16_t *)ptr3, (uint32_t)n);
                    }
                #else
                    if (RD(command,  COMMAND_SSRV) == SERV_DSP_INIT)
                    {   extern df1_init_f32 generic_biquad_cascade_df1_init_f32;
                        generic_biquad_cascade_df1_init_f32(                 // void arm_biquad_cascade_df1_init_f32(
                            (generic_biquad_cascade_df1_inst_f32 *) ptr1,                   //         biquad_cascade_df1_inst_q15 * S,
                            n & 0xFF,                                               //         uint8_t numStages,
                            (const float *) ptr2,                                   //   const q15_t * pCoeffs,
                            (float *) ptr3);                                        //         q15_t * pState)

                    } else //(RD(command,  COMMAND_SSRV) == SERV_DSP_RUN)
                    {   extern df1_f32 generic_biquad_cascade_df1_f32;
                        generic_biquad_cascade_df1_f32(                     // void arm_biquad_cascade_df1_fast_f32(
                            (const generic_biquad_cascade_df1_inst_f32 *) ptr3,             //   const arm_biquad_cascade_df1_inst_f32 * S,
                            (const float *) ptr1,                                   //   const q15_t * pSrc,
                            (float *) ptr2,                                         //         q15_t * pDst,
                            (uint32_t)n);                                           //         uint32_t blockSize)
                    }
                #endif


                break;
            /* ------------------------- */
            case 0:              
                // SERV_LOW_MEMORY_rFFT      /* inplace RFFT with sin/cos recomputed in each loop */
                // 
                // SERV_INIT_rFFT_Q15        /* RFFT + windowing, module, dB */
                // SERV_rFFT_Q15
                // SERV_INIT_rFFT_F32        
                // SERV_rFFT_F32
                // 
                // SERV_INIT_cFFT_Q15        /* cFFT + windowing, module, dB */
                // SERV_cFFT_Q15
                // SERV_INIT_cFFT_F32             
                // SERV_cFFT_F32
                // 
                // SERV_INIT_DFT_Q15         /* DFT/Goertzel + windowing, module, dB */
                // SERV_DFT_Q15
                // SERV_INIT_DFT_F32             
                // SERV_DFT_F32
            /* ------------------------- */
                // SERV_SQRT_Q15 
                // SERV_SQRT_F32 
                // SERV_LOG_Q15  
                // SERV_LOG_F32  
            /* ------------------------- */
                // SERV_SINE_Q15 
                // SERV_SINE_F32 
                // SERV_COS_Q15  
                // SERV_COS_F32  
                // SERV_ATAN2_Q15
                // SERV_ATAN2_F32
            /* ------------------------- */

            default: 
                break;
            }
        break;
    case SERV_GROUP_DEEPL:
        arm_stream_services_mm_audio(command, ptr1, ptr2, ptr3, n);
        break;
        
    case SERV_GROUP_MM_AUDIO:
        arm_stream_services_mm_audio(command, ptr1, ptr2, ptr3, n);
        break;

    case SERV_GROUP_MM_IMAGE:
        arm_stream_services_mm_image(command, ptr1, ptr2, ptr3, n);
        break;

        /*----------------------------------------------------------------------------
           arm_graph_interpreter interface is used for "special" services       
           examples : 
           - access to compute libraries, data converters and compression
           - access to time, stdlib, stdio for NODE delivered in binary
           - report information of change in format of the output stream (MPEG decoder)
           - access to platform IOs, data interfaces and associated services 
           - report error and metadata

            To avoid to have initialization steps when calling a complex LINK 
            service (rfft, ssrc, ..) the call to services is made with an int32 of value 
            zero at first call. STREAM will detect this value as a request for allocation 
            of memory for this instance, and make the corresponding initializations and 
            return a tag used to address the same instance on the next call. To save memory, 
            it is recommended to free this memory with FREE_INSTANCE.

            For example : int32_t ssrc_instance_id;
            arm_stream_services(SSRC_CONVERT, &(ssrc_instance_id = 0), xdmdata, parameters);
                ssrc_intance = index to an internal memory area managed by STREAM
            arm_stream_services(SSRC_CONVERT,  &ssrc_instance_id, xdmdata, parameters);
            ..
            Terminated by arm_stream_services(FREE_INSTANCE, ssrc_instance_id);  free STREAM internal memory
        */

        default:
            break;
    }
}

//
///**
//  @brief        provision for time functions
//  @param[in]    none
//  @return       none
//
//  @par          
//
//  @remark       
// */
//void SERV_time_functions (uint32_t service_command, uint8_t *pt8b, uint8_t *data, uint8_t *flag, uint32_t n)
//{   volatile uint8_t *pt8 = pt8b;
//
//    switch (service_command)
//    {   case SERV_READ_TIME64:
//        case SERV_READ_TIME32:
//        case SERV_READ_TIME16:
//        {   break;
//        }
//    }
//}
             

//
///**
//  @brief        Platform abstraction layer (time, spinlock, IOs)
//  @param[in]    none
//  @return       none
//
//  @par          Usage:
//                
//  @remark       
// */
//void al_services (uint32_t service_command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, uint32_t n)
//{   
//    /* max 16 groups of commands {SERV_INTERNAL .. SERV_MM_IMAGE} */
//	switch (RD(service_command, FUNCTION_SSRV))
//    {
//    case SERV_READ_TIME:
//        SERV_time_functions(RD(service_command, FUNCTION_SSRV), (uint8_t *)ptr1, (uint8_t *)ptr2, (uint8_t *)ptr3, n);
//        break;
//    case SERV_SLEEP_CONTROL:
//        break;
//    case SERV_READ_MEMORY:
//        break;
//    case SERV_SERIAL_COMMUNICATION:
//        break;
//    //enum stream_service_group
//    case SERV_MUTUAL_EXCLUSION:
//        SERV_mutual_exclusion(RD(service_command, FUNCTION_SSRV), (uint8_t *)ptr1, (uint8_t *)ptr2, (uint8_t *)ptr3, n);
//        break;
//    case SERV_CHANGE_IO_SETTING:
//        break;
//    }
//}


/**
  @brief         TEA 
  @param[in]     64bits data to encrypt and Key

  @return        none

  @par           

  @remark
 */
//void encrypt (uint32_t v[2], const uint32_t k[4]) {
//    uint32_t v0=v[0], v1=v[1], sum=0, i;           /* set up */
//    uint32_t delta=0x9E3779B9;                     /* a key schedule constant */
//    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
//    for (i=0; i<32; i++) {                         /* basic cycle start */
//        sum += delta;
//        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
//        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
//    }                                              /* end cycle */
//    v[0]=v0; v[1]=v1;
//}
//
//void decrypt (uint32_t v[2], const uint32_t k[4]) {
//    uint32_t v0=v[0], v1=v[1], sum=0xC6EF3720, i;  /* set up; sum is (delta << 5) & 0xFFFFFFFF */
//    uint32_t delta=0x9E3779B9;                     /* a key schedule constant */
//    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
//    for (i=0; i<32; i++) {                         /* basic cycle start */
//        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
//        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
//        sum -= delta;
//    }                                              /* end cycle */
//    v[0]=v0; v[1]=v1;
//}
/*
 * --- 4K platform signatures -------------------------------------------------------
//fwd_print_coef_col( floorf((2^16) * rand(4096/16,1)), 1, 16 ); */
/*
extern const uint16_t platform_private_key_4Kbits [256] = {
 63789, 40809,  4164, 24478, 10895, 15157,  3421, 59097, 51989, 24445, 54529, 49403, 40754, 25827, 23545,  5823,
 22392, 35957, 30182, 42300, 33654, 53374,  6368, 30389, 38654, 12266, 40064,  3404, 37730, 55203, 32750, 28771,
  9768,  1853, 49589, 52173, 19238,  7550, 24582, 54322, 55166, 43597, 62923, 61808,  7385, 42486, 31509,  4359,
 58836, 32586, 50548,  3955, 17200, 42668,  8755, 41847, 25227, 50180, 42789, 25001, 19662, 22291, 60222, 29901,
 28999, 29765, 61950, 14360, 57829,  1302, 22397, 50202, 22465, 40554, 29689,   666, 39261, 39424, 42560, 22460,
 32328, 45991, 58183,  3608,  6446, 42584, 50074, 64746,  8213, 23886, 44317, 24625, 56587, 19134,  8747, 44082,
 13276, 56919, 49227, 27484,    15,  9795, 17946, 57175, 39403, 21049, 18631, 28528, 59228, 60627, 33114, 41129,
 47137,  1567, 37678,  3049, 27691, 30653,  1482,  4264, 60552, 35005, 24038, 23851,  9920,  9804, 22990, 22017,
 51382, 31898, 30461,  8601, 58090, 44207, 54733, 43023, 64481, 64211, 16394, 40931, 47726, 32648, 55694, 12512,
  8135,   182, 10023, 35006, 33464, 25245, 20355,   233, 53427, 41839, 29382, 15996, 52650, 53999, 55849, 30621,
 63615, 55131,  5147, 15571, 53580, 26596, 30560, 62359, 63242, 50153, 37652, 60026, 32468, 10879, 21364, 19427,
 36588,  4422,  4520, 10930, 62091, 53155, 46560, 63586, 65432, 64713,  9836, 62814, 34764,  4855, 20435, 58665,
 54707,   153, 41956, 52637, 16064,  4202, 17245,  6731, 31701, 27452, 24988, 58115, 27561, 18602,  3157, 14363,
 15674,  1917, 46026,   500, 40037, 26744, 16314, 42759, 20989,  6794, 35098, 10804, 57897, 43676, 55557, 49981,
 52888, 41481, 46558, 45132, 21033, 34842, 57225,  3574, 32794, 28361, 59263, 41299, 64424, 38351, 55091, 30724,
 35731, 11737, 41580, 63108, 34997, 31431, 52013,  6076, 57724,   253, 33523, 44463, 37076, 31356, 21005, 39425,

};

extern const uint16_t platform_public_key_4Kbits [256] = {
 59844, 44729, 62045,  6493, 33490,  7217, 35734, 45140,  9661, 50958, 26152, 58870, 20122,  4001, 14383,  5428,
 62285,  1072,  7514,   813, 14170,   748, 42103, 33881, 16091, 12697,  5955, 24146,   510, 39498, 31382, 20192,
 48787, 55007, 17198, 33701, 29279, 22361, 54993, 64388, 41055, 11880,  8062, 38008, 21530, 17575, 36060, 11830,
 44465,  3649,  2231, 18777,  5071, 59019, 55483, 25932, 11089, 28210, 27277, 47760, 26638, 62377, 59767, 62351,
 22675, 19021, 58110, 13764,  8577, 34112, 59340, 26380, 14140,  5160, 61149, 39509, 24739, 43576, 51916, 21855,
 45394, 13357, 62830, 46650, 10938, 29017, 41483, 60946, 34690, 41056, 44618, 60502, 10016, 26589, 20478, 45475,
 58372, 32156, 52810, 21393, 36036, 25479, 58774, 44310, 54289,  7214, 18299, 50307, 14159,  2232, 28609, 61398,
 17176, 37338, 23563,  1758, 32795, 54199, 16972,  3007, 16152, 43301, 21588, 43220,   852, 47059, 25632,  2195,
 26608, 46943, 60380, 64488, 64449, 58740, 56734, 52491, 36371, 27451,  8331, 42901, 56620, 17996, 55060,  4637,
 24824, 17574, 10021, 41353, 20733, 62856, 32681, 48405,   835, 39672, 37778, 52912, 42923, 57555, 59137,  9976,
 12620, 51837,  3978, 25547, 19658, 48115,  6829, 51942, 51296, 34891, 16603,  4650, 41012,  1617,  4066,  8494,
 29531, 44062, 56106, 32666,  3197, 20567, 42049, 51536, 18949, 32628, 53636, 39002, 35155, 21684, 26980, 52035,
 22492, 30317, 24105, 44536, 37209, 42714, 32185, 26113, 31292,  4363, 26937, 63510, 51165, 47776, 50177, 49583,
 55264, 50472, 64137,  7298, 25957, 32247, 16914,  2422, 63857, 47607,  9697,  9692, 46191, 24968,  5007, 26924,
  9371, 52358, 60964,   309, 42601, 44468, 16621, 55258, 19264,  1760,  6114, 52290, 46622, 51341, 40889, 54094,
  2295, 26573, 16362, 31516, 57726, 18394, 39265,  1718, 10171, 54651, 12772, 54380, 22156, 43982,  3431, 48123,
};
*/

//fwd_print_coef_col( floorf((2^32) * rand(4096/32,1)), 1, 8 );
/* extern const uint32_t platform_private_key_4Kbits [128] = {
  2452526671,  1812651256,  3097342766,   314126654,  2554912484,  3702164590,  1927585877,  2802798550,
  1303407119,  2608934723,  1197876299,  3434094102,  3419520323,  4097772047,  1908417888,  1962296738,
  2576194992,  3619033209,   134004925,   804319133,  4052707085,  4071294585,  1945553145,  3482499289,
  3989505788,  2889298721,  1599154964,  1742449724,  1884734392,  2914776122,  1997461852,  4094238426,
  1523416233,  1456083514,  3847689602,  2342622301,  3218149061,   536343486,  1946628721,   321046393,
  2849026241,  3022160789,  3946862637,  2834987259,  2963978059,  3666714983,  2009622596,  1969150944,
  3462191122,  3542348306,   817916835,   110265590,   244018397,   613903642,   736237871,  2687982868,
   126766851,  2028651632,  2913889510,   493014868,  1013886833,  1241591732,   741982634,  1390307210,
  3440741675,  1286916938,  3331287114,  2374132639,  2382382803,  3138106201,  3322693311,  3869108853,
   593423417,  3410533956,   813524247,   124444025,   547123825,   574390398,   551091375,  4016910224,
  1173514151,  4048776575,  2740835533,  3747491340,  1576567553,  1014479457,   804525706,  2343545747,
  1095716831,  1313502393,    66780808,  2523242149,  4134156465,  3650067967,    34103316,  2723118955,
  1543143675,   489882317,  2322795306,  1788516222,  2220921135,  3805868774,   641650540,  1866921449,
   253569750,  1636511633,  3102553242,   408533977,  2865560937,  1273030687,  2570807496,   652199330,
  1874225401,    54470065,   983646066,  1132495488,  2196382847,   923782560,  1486530232,  3211849949,
  1776482024,   239455428,  1675158111,  2037893006,  3544725485,  1304139207,  3529561772,  2429576083,
};
  extern const uint32_t platform_public_key_4Kbits [128] = {
   233567265,  1116752402,  2530180486,  2060440189,   853197903,  1026551567,  3350905477,  2651325914,
   619028480,  3075763606,  1724456980,  1985930031,  3037753086,  1723186174,    61784824,   320578255,
  2538618481,  1915561943,  3979805183,   407563859,  1612411398,  2345047264,   479654964,  3884640012,
  2719903998,  3888713398,  2708204253,    61109983,  1359261744,   480481162,  2703523561,   260745093,
  2894786147,  2050585262,  1312309229,  2217633272,  3036663210,  3494432564,  1356386492,  1337032687,
  1481664379,  2861757725,  3698274489,  3271816983,  3761693748,  3741927968,   742129130,  3651684197,
  4121400693,  3308013829,  3758162314,   289538676,  2777951414,  1391984256,  2750269047,  3778501906,
  1604761596,  3293112734,   721926701,  2232200518,  2694901321,  3066235932,  1315961252,  1132527511,
  3934205106,  2641552048,   400183835,  2695938954,   824754078,  3337037933,  3713001398,  1432722113,
   581598698,  3287656411,  1368435565,  1083961746,   859313912,   296395058,  2370438557,  1734357715,
  3221722832,  2092437210,  1652643813,   263747543,   917797964,  2335918313,  1763705179,  3869607504,
   241761720,  1904857713,  2309887215,   575785894,  2323350636,  3682368347,   850478099,   668336001,
   263615505,  2839291162,    79897738,  1250277592,  4182541487,  3284105744,  1046614510,  2929666174,
   592080613,  2705020512,  3680850077,  3864604414,  1496230894,  2088686375,  2918505406,  3024180495,
  1979481950,  1564537603,  1203729972,   327295549,  1909649709,   711703124,  1712613800,  3953881054,
  2196164463,  3926211412,   394853260,  4265062187,   414179256,  1344958243,  3373160417,  2587289407,
}; */

#ifdef __cplusplus
}
#endif
