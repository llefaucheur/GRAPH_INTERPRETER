/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
 * Description:  
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

#ifdef __cplusplus
 extern "C" {
#endif

#include "stream_const.h" 
#include "stream_types.h"
#include "stream_extern.h"
#include "dsp/filtering_functions.h"

SECTION_START

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
    case STREAM_S1: case STREAM_U1: return 1;
    case STREAM_S2: case STREAM_U2: case STREAM_Q1: return 2;
    case STREAM_S4: case STREAM_U4: case STREAM_Q3: return 4;
    default:
    case STREAM_S8:   case STREAM_U8:   case STREAM_Q7:  case STREAM_FP8_E4M3: case STREAM_FP8_E5M2: return 8;
    case STREAM_S16:  case STREAM_U16:  case STREAM_Q15: case STREAM_FP16:     case STREAM_BF16:     return 16;
    case STREAM_Q23:  return 24;
    case STREAM_S32:  case STREAM_U32:  case STREAM_Q31: case STREAM_CQ15:     case STREAM_FP32:    case STREAM_CFP16: return 32;
    case STREAM_S64:  case STREAM_U64:  case STREAM_Q63: case STREAM_CQ31:     case STREAM_FP64:    case STREAM_CFP32: return 64;
    case STREAM_FP128:case STREAM_CFP64: return 128;
    case STREAM_FP256: return 256;
    }
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
static void arm_stream_services_internal(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint32_t n)
{

    switch (RD(command, SWC_TAG_CMD))
    {
    case STREAM_SERVICE_INTERNAL_NODE_REGISTER: /* called during STREAM_NODE_DECLARATION to register the SWC callback */
    {
#ifndef _MSC_VER 
        //rtn_addr = __builtin_return_address(0); // check the lr matches with the node 
#endif
        break;
    }

    /* ----------------------------------------------------------------------------------
        arm_stream_services(PACK_SERVICE(instance index, STREAM_SERVICE_INTERNAL_DEBUG_TRACE), *int8_t, 0, nb bytes);
            the Stream instance index
        arm_stream_services(DEBUG_TRACE_STAMPS, disable_0 / enable_1 time stamps);

        used to share the SWC version numbers, authors, .., real-time trace data
     */
    case STREAM_SERVICE_INTERNAL_DEBUG_TRACE:
    {
        //uint8_t arcid;
        //uint32_t* arc;
        //uint32_t free_area;
        //uint32_t debugBufferLength = RD(command, SWC_TAG_CMD);

        ///* extraction of the arc index used for the traces of this Stream instance */
        //arcid = RD(stream_instance->parameters, TRACE_ARC_PARINST);
        //arc = &(stream_instance->all_arcs[arcid * SIZEOF_ARCDESC_W32]);
        //free_area = RD(arc[1], BUFF_SIZE_ARCW1) - RD(arc[3], WRITE_ARCW3);
        //if (free_area < debugBufferLength)
        //{
        //    platform_al(PLATFORM_ERROR, 0, 0, 0); /* overflow issue */
        //    debugBufferLength = free_area;
        //}

        //arc_data_operations((arm_stream_instance_t*)&stream_instance, arc, arc_IO_move_to_arc, ptr1, debugBufferLength);
        break;
    }

    /* toggle a flag to insert/remove the time-stamps on each data pushed in the debug trace */
    case STREAM_SERVICE_INTERNAL_DEBUG_TRACE_STAMPS:
    {   break;
    }

    /* return the list of services available to let the SWC decide between its library or the accelerated ones
       by MVE architecture or specific acceleration schemes like coprocessors and custom instructions */
    //case STREAM_SERVICE_AVAILABLE:
    //{   break;
    //}

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
static void arm_stream_services_flow (uint32_t command, uint8_t* ptr1, uint8_t* ptr2, uint32_t n) 
{
    // SECTIONS OF ARC APIs
    /*
    * Test-pattern injection from scripts and result verification
    * Test availability of data (R/W)
    * Read/write to arcs, return the pointer to read/write for scripts
    * Data moves w/wo DMA
    */
}

/**
  @brief        data conversion services entry point
  @param[in]    instance   pointers to the Stream instance and graph data
  @param[in]    command    Bit-field of command (see enum stream_command)
  @param[in]    ptr1       data pointer
  @param[in]    ptr2       data pointer
  

  @return       none
  @par
  @remark
 */
static void arm_stream_services_conversion (uint32_t command, uint8_t* ptr1, uint8_t* ptr2, uint32_t n) 
{

    switch (RD(command, SWC_TAG_CMD))
    {
    case STREAM_SERVICE_CONVERSION_INT16_FP32: 
    {
        /* convert ptr1 => ptr2 [ndata3] */
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
void arm_stream_services_stdlib (uint32_t command, uint8_t* ptr1, uint8_t* ptr2, uint32_t n) 
{
#if STREAM_SERVICE_EXTSTDLIB

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
void arm_stream_services_math (uint32_t command, uint8_t* ptr1, uint8_t* ptr2, uint32_t n) 
{
    /* 
        Permanent APIs whatever "STREAM_SERVICE_EXTMATH" are 
    //STREAM_SIN_Q15 STREAM_COS_Q15 STREAM_LOG10_Q15, STREAM_SQRT_Q15,
    */
#if STREAM_SERVICE_EXTMATH
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
void arm_stream_services_mm_audio (uint32_t command, uint8_t* ptr1, uint8_t* ptr2, uint32_t n) 
{
#if STREAM_SERVICE_EXTAUDIO

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
void arm_stream_services_mm_image (uint32_t command, uint8_t* ptr1, uint8_t* ptr2, uint32_t n) 
{
#if STREAM_SERVICE_EXTIMAGE

#endif
}



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
                For example a SWC delivered in binary for Armv7-M architecture profile will 
                    automatically scale in DSP performance when executed on Armv8.1-M through services.
  @remark
 */

void arm_stream_services (uint32_t service_command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3, uint32_t n)
{   
    //arm_stream_instance_t *pinst;

    /* max 16 groups of commands */
	switch (RD(service_command, GROUP_SSRV))
    {
    //enum stream_service_group
    case STREAM_SERVICE_INTERNAL:
        
        if ((RD(service_command, GROUP_SSRV)) == STREAM_SERVICE_INTERNAL_RESET)
        {   /* arm_stream_services(*ID, STREAM_SERVICE_INTERNAL_RESET, stream_instance, 0, 0); */
            //stream_instance = *(arm_stream_instance_t*)ptr1;
        } 
        else
        {   /* arm_stream_services(*ID, PACK_COMMAND(TAG,PRESET,NARC,INST,STREAM_SERVICE_INTERNAL_XXXXX), pta, ptb); */
            arm_stream_services_internal(RD(service_command, FUNCTION_SSRV), ptr1, ptr2, n);
        }
        break;
    case STREAM_SERVICE_FLOW:
        arm_stream_services_flow (service_command, ptr1, ptr2, 0);
        break;
    case STREAM_SERVICE_CONVERSION:
        arm_stream_services_conversion(service_command, ptr1, ptr2, 0);
        break;
    case STREAM_SERVICE_STDLIB:
        arm_stream_services_stdlib(service_command, ptr1, ptr2, 0);
        break;
    case STREAM_SERVICE_MATH:
        arm_stream_services_math(service_command, ptr1, ptr2, 0);
        break;

    case STREAM_SERVICE_DSP_ML:
            /*  
                - IIR-DF1 biquad filter cascade, Cortex-M0's CMSIS-DSP arm_biquad_cascade_df1_q15
                - The spectral comuputation (cFFT, rFFT, DFT, window, module, dB)
                - Raw datatype conversion (int/float)
                - Matrix operations
            */

            switch (RD(service_command, FUNCTION_SSRV))
            {
            /* ------------------------- */
            case STREAM_SERVICE_CASCADE_DF1_Q15:          /* IIR filters */
                arm_biquad_cascade_df1_fast_q15(
                    (const arm_biquad_casd_df1_inst_q15 *) ptr3,
                    (const q15_t *) ptr1,
                    (q15_t *) ptr2,
                    (uint32_t)n);
                break;
            case STREAM_SERVICE_CASCADE_DF1_F32:          /* IIR filters */
                //arm_biquad_cascade_df1_f32(
                //    (const arm_biquad_casd_df1_inst_f32 *) ptr3,
                //    (const float32_t *) ptr1,
                //    (float32_t *) ptr2,
                //    (uint32_t)n);
                break;
            /* ------------------------- */
            case 0:              
                // #if STREAM_SERVICE_LOW_MEMORY        /* RADIX2 with tables recomputed */
                // 
                // STREAM_SERVICE_INIT_rFFT_Q15        /* RFFT + windowing, module, dB */
                // STREAM_SERVICE_rFFT_Q15
                // STREAM_SERVICE_INIT_rFFT_F32        
                // STREAM_SERVICE_rFFT_F32
                // 
                // STREAM_SERVICE_INIT_cFFT_Q15        /* cFFT + windowing, module, dB */
                // STREAM_SERVICE_cFFT_Q15
                // STREAM_SERVICE_INIT_cFFT_F32             
                // STREAM_SERVICE_cFFT_F32
                // 
                // STREAM_SERVICE_INIT_DFT_Q15         /* DFT/Goertzel + windowing, module, dB */
                // STREAM_SERVICE_DFT_Q15
                // STREAM_SERVICE_INIT_DFT_F32             
                // STREAM_SERVICE_DFT_F32
            /* ------------------------- */
                // STREAM_SERVICE_SQRT_Q15 
                // STREAM_SERVICE_SQRT_F32 
                // STREAM_SERVICE_LOG_Q15  
                // STREAM_SERVICE_LOG_F32  
            /* ------------------------- */
                // STREAM_SERVICE_SINE_Q15 
                // STREAM_SERVICE_SINE_F32 
                // STREAM_SERVICE_COS_Q15  
                // STREAM_SERVICE_COS_F32  
                // STREAM_SERVICE_ATAN2_Q15
                // STREAM_SERVICE_ATAN2_F32
            /* ------------------------- */

            default: 
                break;
            }
        break;
    case STREAM_SERVICE_MM_AUDIO:
        arm_stream_services_mm_audio(service_command, ptr1, ptr2, 0);
        break;
    case STREAM_SERVICE_MM_IMAGE:
        arm_stream_services_mm_image(service_command, ptr1, ptr2, 0);
        break;


        /*----------------------------------------------------------------------------
           arm_graph_interpreter interface is used for "special" services       
           examples : 
           - access to compute libraries, data converters and compression
           - access to time, stdlib, stdio for SWC delivered in binary
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
SECTION_STOP
#ifdef __cplusplus
}
#endif
