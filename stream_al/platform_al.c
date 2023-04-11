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

#include <stdlib.h>
#include <stdint.h>
#include <string.h> 

#include "stream_const.h"      /* graph list */

#include "platform_const.h"

#include "stream_types.h"  
#include "platform_types.h"

/*-------------------PLATFORM MANIFEST-----------------------
                  +-----------------+
                  | static |working |
   +--------------------------------+
   |external RAM  |        |        |
   +--------------------------------+
   |internal RAM  |        |        |
   +--------------------------------+
   |fast TCM      |  N/A   |        |
   +--------------+--------+--------+
*/
#define SIZE_MBANK_DMEM_EXT   0x20000  /* external */
#define SIZE_MBANK_DMEM       0x8000    /* internal */
#define SIZE_MBANK_DMEMFAST   0x4000    /* TCM */
#define SIZE_MBANK_BACKUP     0x10      /* BACKUP */
#define SIZE_MBANK_HWIODMEM   0x1000    /* DMA buffer */
#define SIZE_MBANK_PMEM       0x100     /* patch */

uint32_t MEXT[SIZE_MBANK_DMEM_EXT];
uint32_t RAM1[SIZE_MBANK_DMEM/4];
uint32_t RAM2[SIZE_MBANK_DMEM/4];
uint32_t RAM3[SIZE_MBANK_DMEM/4];
uint32_t RAM4[SIZE_MBANK_DMEM/4];
uint32_t TCM1[SIZE_MBANK_DMEMFAST]; 
uint32_t TCM2[SIZE_MBANK_DMEMFAST]; 
uint32_t BKUP[SIZE_MBANK_BACKUP]; 
uint32_t HWIO[SIZE_MBANK_HWIODMEM];
uint32_t PMEM[SIZE_MBANK_PMEM];

#define PROC_ID 0 
extern const uint32_t graph_input[];

#if PROC_ID == 0
intPtr_t long_offset[NB_MEMINST_OFFSET] = 
{
    (intPtr_t)&(MEXT[10]), // MBANK_DMEM_EXT
    (intPtr_t)&(RAM1[11]), // MBANK_DMEM    
    (intPtr_t)&(RAM1[12]), // MBANK_DMEMPRIV
    (intPtr_t)&(TCM1[13]), // MBANK_DMEMFAST
    (intPtr_t)&(BKUP[14]), // MBANK_BACKUP  
    (intPtr_t)&(HWIO[15]), // MBANK_HWIODMEM
    (intPtr_t)&(PMEM[16]), // MBANK_PMEM    
    (intPtr_t)graph_input, // MBANK_FLASH   ideally 16Bytes-aligned for arc shifter 1
};
#endif
  

#if MULTIPROCESS == 1
static uint32_t WR_BYTE_AND_CHECK_MP_(uint8_t *pt8b, uint8_t code)
{   volatile uint8_t *pt8 = pt8b;
    *pt8 = code;
    INSTRUCTION_SYNC_BARRIER;

    /* no need to use LDREX, don't wait and escape if collision occurs */
    DATA_MEMORY_BARRIER;

    return (*pt8 == code);
}
#else
#define WR_BYTE_AND_CHECK_MP_(pt8b, code) 1
#endif

/*
 * --- platform abstraction layer -------------------------------------------------------
 */

void platform_al(uint32_t command, uint8_t *ptr1, uint8_t *ptr2, uint8_t *ptr3)
{
    static uint8_t stream_boot_lock;        
    static uint8_t stream_initialization_done;
    static uint8_t stream_reset_synchro[MAX_NB_STREAM_INSTANCES];

    static uint32_t platform_time;

    switch (command)            /*  */
    {
    case PLATFORM_PROC_ID:  /* platform_al (PLATFORM_PROC_ID, uint_8_t *procID, uint_8_t *archID,0); */
    {   
        uint8_t *proc;
        uint8_t *arch;
        proc=ptr1;
        arch=ptr2;
        
        /* [1,2,3] processor architectures 0="any" 1="master processor architecture" */
        *arch = 1;

        /* processor index [0..7] for this architecture 0="master processor" */  
        *proc = 0;
        break;
    }

    case PLATFORM_OFFSETS: /* platform_al (PLATFORM_OFFSETS, intPtr_t **,0,0); */
    {
        ptr1 = (uint8_t *) &(long_offset[0]);
        break;
    }



    case PLATFORM_MP_GRAPH_SHARED: /* platform_al (PLATFORM_MP_GRAPH_SHARED, start_address,end_address,0); */
    {
        /* define MPU_RBAR[Region] */
        /* set MPU_RASR[18] = 1 : the memory area is shared*/
        break;
    }
    case PLATFORM_MP_BOOT_SYNCHRO: /* platform_al (PLATFORM_MP_SYNCHRO, *instance index, *copy allowed for you, 0); */
    {
        uint8_t check;
        uint8_t instance_index;
        uint8_t *copy_to_do;
        instance_index = *(uint8_t *)ptr1;
        copy_to_do=ptr2;

        /* if the graph copy is on-going, return*/
        RD_BYTE_MP_(check, &stream_boot_lock);
        if (check != 0u)
        {   *copy_to_do = 0;
            break;
        }

        /* reservation attempt */
        if (0 == WR_BYTE_AND_CHECK_MP_(&stream_boot_lock, instance_index))
        {   *copy_to_do = 0;
            break;
        }

        /* no collision */
        *copy_to_do = 1;
        break;
    }
    case PLATFORM_MP_BOOT_WAIT: /* platform_al (PLATFORM_MP_BOOT_WAIT, uint_8_t *wait, 0,0); */
    {   
        uint8_t *wait=ptr1;
        *wait = (stream_initialization_done == 0u)? 1u : 0u;
        break;
    }
    case PLATFORM_MP_BOOT_DONE: /* platform_al (PLATFORM_MP_BOOT_DONE,0,0,0); */
    {   
        DATA_MEMORY_BARRIER;
        stream_initialization_done = 1;
        break;
    }
    case PLATFORM_MP_RESET_DONE: /* platform_al (PLATFORM_MP_RESET_DONE, &stream_instance,0,0); */
    {
        uint8_t *instance=ptr1;
        stream_reset_synchro[*instance] = 0;
        DATA_MEMORY_BARRIER;
        break;
    }
    case PLATFORM_MP_RESET_WAIT: /* platform_al (PLATFORM_MP_RESET_WAIT, &wait, &total_nb_stream_instance, 0); */
    {
        uint8_t *wait;
        uint8_t *nb;
        uint8_t i;
        wait = ptr1;
        nb = ptr2;
                
        DATA_MEMORY_BARRIER;

        /* all the instances must have set their flag at "1" to tell "reset completed" */
        *wait = 1;
        for (i = 0; i < *nb; i++)
        { *wait &= stream_reset_synchro[i];
        }
        break;
    }
    case PLATFORM_IO_ACK:  /* interface callback to arm_stream_io : platform_al(PLATFORM_IO_ACK, fx_idx, *,int);*/
    {
        uint32_t fw_idx;
        uint32_t size;
        uint32_t *graph;
        fw_idx = (uint32_t)convert_ptr_to_int(ptr1);
        size = (uint32_t)convert_ptr_to_int(ptr3);
        graph = platform_io_callback_parameter[fw_idx];
        arm_stream_io (fw_idx, graph, ptr2, size);
        break;
    }

    case PLATFORM_IO_SET_STREAM :   /* platform_al (PLATFORM_IO_SET_STREAM, struct platform_control_stream *, 0,0); */
    case PLATFORM_IO_DATA :
    case PLATFORM_IO_STOP_STREAM : 
	{   
        struct platform_control_stream *parameters;
        extern struct platform_io_manifest platform_io [LAST_IO_FUNCTION_PLATFORM];
        struct platform_io_manifest *io_manifest;
        data_buffer_t *buffer;
        io_function_control_ptr io_func;
        uint32_t fw_idx;
        uint32_t *settings;
        
        parameters = (struct platform_control_stream *)ptr1; 
        buffer = &(parameters->buffer);
        fw_idx = parameters->fw_idx;
        io_manifest = &(platform_io[fw_idx]);
        settings = &(parameters->domain_settings[0]);

        switch (command)
        {
        case PLATFORM_IO_SET_STREAM:
            platform_io_callback_parameter [parameters->fw_idx] = parameters->graph;
            io_func = io_manifest->io_set;
            (*io_func)(settings, buffer->address, (uint32_t)(buffer->size));
            break;
        case PLATFORM_IO_DATA :
            io_func = io_manifest->io_start;
            (*io_func)(settings, buffer->address, (uint32_t)(buffer->size));
            break;
        case PLATFORM_IO_STOP_STREAM:
            io_func = io_manifest->io_stop;
            (*io_func)(settings, buffer->address, (uint32_t)(buffer->size));
            break;
        default:
            break;
        }
	    break; 
    }
    case PLATFORM_BACKUP_MEM:       /* platform_al (PLATFORM_BACKUP_MEM, memory area to backup now, size,0); */
        break;
    case PLATFORM_RESTORE_MEM:      /* restore memory areas before warm boot */
        break;
    case PLATFORM_EXEC_TIME:        /* platform_al (PLATFORM_EXEC_TIME, *delta_time, 0,0); */
    {
        break;
    }
    case PLATFORM_ERROR:  /* platform_al (PLATFORM_ERROR, uint_8_t type, uint_8_t data,0); */
    {   
        uint8_t *type;
        uint8_t *error;
        type=ptr1;
        error=ptr2;
        break;
    }

    //------------------------------------------------------------------------------------------------------
    /* Time format - 40 bits - SYSTICK 1ms increments, global register
       GLOBALS : G_INCTICK (64bits), INV_RVR (32bits), G_SYST0 (32bits) interval in Q32 second

       G_INCTICK : incremented in the SYSTICK interrupt, SYST0=1ms or 10ms
       FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
       ________________________qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq with 1ms increment => 35 years (32bits gives only 50 days)
  
       SYST_CVR (current value), SYST_CALIB (reload value for 10ms), SYST_RVR (reload value), INV_RVR = 0x10000.0000/SYST_RVR
       Time sequence : G_INCTICK=>T0  CVR=>C0  G_INCTICK=>T1  CVR=>C1  

       Algorithm : if T0==T1 (no collision) then T=T0,C=C0  else T=T1,C=C1 endif
        the fraction of SYST0 is (RVR - C) x INV_RVR = xx...xxx
        the time (64bits Q16)  is  (T<<16) + (((RVR - C) x INV_RVR) >> 16)  => used for time-stamps 
        FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
        ________qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqXXXXXXXXXXXXXXXX 

       stream_time_seconds in 32bits :
       bits 31-30 = format   0: seconds from reset, 1:seconds from Jan 1st 2023, 2:format U14.16 for IMU time-stamps differences
       bit  29- 0 = time     (30bits seconds = 68years)
     */
    //------------------------------------------------------------------------------------------------------
    case PLATFORM_TIME_SET:
        break;
    case PLATFORM_TIMER_SET:
        break;
    case PLATFORM_TIME_READ:
    {   /* if the HAL is not ready then use a counter of calls */
        break;
    }
    default:
        break;
    }
}

