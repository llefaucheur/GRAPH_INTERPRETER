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
#ifndef cSTREAM_NODES_H
#define cSTREAM_NODES_H

/*
 *  SWC manifest :
 */ 

//enum stream_node_status {
#define SWC_BUFFERS_PROCESSED 0
#define SWC_NEED_RUN_AGAIN 1         /* SWC completion type */

//enum mem_mapping_type {
#define MEM_TYPE_STATIC          0    /* (LSB) memory content is preserved (default ) */
#define MEM_TYPE_WORKING         1    /* scratch memory content is not preserved between two calls */
#define MEM_TYPE_PSEUDO_WORKING  2    /* static only during the uncompleted execution state of the SWC, see “NODE_RUN” */
#define MEM_TYPE_PERIODIC_BACKUP 3    /* static parameters to reload for warm boot after a crash, holding for example 
           long-term estimators. This memory area is cleared at cold NODE_RESET and 
           refreshed for warm NODE_RESET. The SWC should not reset it (there is 
           no "warm-boot reset" entry point.
           the period of backup depends on platform capabilities, 10 seconds for example */
  

//enum mem_speed_type                         /* memory requirements associated to enum memory_banks */
#define MEM_SPEED_REQ_ANY           0    /* best effort */
#define MEM_SPEED_REQ_NORMAL        1    /* can be external memory */
#define MEM_SPEED_REQ_FAST          2    /* will be internal SRAM when possible */
#define MEM_SPEED_REQ_CRITICAL_FAST 3    /* will be TCM when possible
           When a SWC is declaring this segment as relocatable ("RELOC_MEMREQ") it will use 
           physical address different from one TCM to an other depending on the processor running the SWC.
           The design constraint is to have ONE segment and a pointer to this segment placed 
           at the beginning of the SWC instance. To let the scheduler modify the TCM address dynamically
           before calling the SWC. 
           In all other cases the TCM area will be allocated to one processor designated in 
           the linked-list parameter "PROCID_GI".
           */
                                

//enum buffer_alignment_type            
#define MEM_REQ_4BYTES_ALIGNMENT   0   /* 0 placed here as default value */
#define MEM_REQ_8BYTES_ALIGNMENT   1
#define MEM_REQ_16BYTES_ALIGNMENT  2
#define MEM_REQ_32BYTES_ALIGNMENT  3
#define MEM_REQ_64BYTES_ALIGNMENT  4
#define MEM_REQ_128BYTES_ALIGNMENT 5
#define MEM_REQ_NOALIGNMENT_REQ    6   /* address binary mask : */
#define MEM_REQ_2BYTES_ALIGNMENT   7   /*   mask = ~((1 << (7&(mem_req_2bytes_alignment+2)) -1) */


#define SWC_CONTROLS U(4)
#define SWC_CONTROLS_NAME U(8)


//enum buffer_relocation_type
#define NOT_RELOCATABLE U(0)
#define RELOCATABLE U(1)

/*---------------------------------------------------------------------------------------------------*/
#define  UNUSED_SWCID_MSB U(31)
#define  UNUSED_SWCID_LSB U(31) /* 1 */
#define      ID_SWCID_MSB U(30)
#define      ID_SWCID_LSB U(19) /* 12 SWC ID : developer can produce 4K SWC   */
#define   IDVER_SWCID_MSB U(18)
#define   IDVER_SWCID_LSB U(15) /* 4 version    */
#define   MARCH_SWCID_MSB U(14)
#define   MARCH_SWCID_LSB U(14) /* 1 multi architecture fat binary delivery, offsets in fatbin_offsets[]    */
#define SUBARCH_SWCID_MSB U(13)
#define SUBARCH_SWCID_LSB U( 8) /* 6 (stream_processor_sub_arch_fpu) mapped on 4b from platform_manifest */
#define   UARCH_SWCID_MSB U( 7)
#define   UARCH_SWCID_LSB U( 0) /* 8 (stream_processor_architectures) mapped on 6b from platform_manifest */

/*---------------------------------------------------------------------------------------------------*/
/* SWC memory allocation constraint 32bits : memory allocation description */
#define   OFFSET_MEMREQ_MSB U(31)
#define   OFFSET_MEMREQ_LSB U(31) /* 1  followed by an offset (working memory in SMP) */
#define    RELOC_MEMREQ_MSB U(30)
#define    RELOC_MEMREQ_LSB U(30) /* 1  memory segement is relocatble */
#define  INPLACE_MEMREQ_MSB U(29)
#define  INPLACE_MEMREQ_LSB U(29) /* 1  in-place processing : output buffer can be mapped on input buffer  */
#define    INMOD_MEMREQ_MSB U(28)     
#define    INMOD_MEMREQ_LSB U(28) /* 1  warning : "1" means input buffer can be modified by the component  */
#define     TYPE_MEMREQ_MSB U(27)     
#define     TYPE_MEMREQ_LSB U(26) /* 2  mem_mapping_type  static, working, pseudo_working, hot_static      */
#define    SPEED_MEMREQ_MSB U(25)     
#define    SPEED_MEMREQ_LSB U(23) /* 3  mem_speed_type "idx_memory_base_offset"   */
#define  ALIGNMT_MEMREQ_MSB U(22)     
#define  ALIGNMT_MEMREQ_LSB U(20) /* 3  buffer_alignment_type */
#define    SHIFT_MEMREQ_MSB U(19)     
#define    SHIFT_MEMREQ_LSB U(18) /* 2  size shifter : 0=0, 1=4, 2=8, 3=12 => max size 256k<<12 = 1G */
#define     SIZE_MEMREQ_MSB U(17)     
#define     SIZE_MEMREQ_LSB U( 0) /*18 size in bytes, max = 256kB */

/*---------------------------------------------------------------------------------------------------*/
#define   UNUSED_SWPARAM_MSB U(31)
#define   UNUSED_SWPARAM_LSB U(24) /* 7 */
#define   PARMSZ_SWPARAM_MSB U(23)
#define   PARMSZ_SWPARAM_LSB U( 6) /*18 parameter array size <256kB */
#define NBPRESET_SWPARAM_MSB U( 5)
#define NBPRESET_SWPARAM_LSB U( 0) /* 6 nb parameters presets (max 64) */

#define PACKSWCPARAM(PSZ,NBPST) (((PSZ)<<6)|(NBPST))

//enum idx_memory_base_offset   /* maximum 4, see DATAOFF_ARCW0 */
#define MBANK_DMEM_EXT  U(0)    /* external shared memory space, used for arcs offsets */
#define MBANK_DMEM      U(1)    /* internal shared memory space, used for arcs offsets */

#define NB_ARC_OFFSET   U(2) /* arc offsets for the shared internal / external mem */

#define MBANK_DMEMPRIV  U(2)    /* not shared memory space, (MP without cache) */
#define MBANK_DMEMFAST  U(3)    /* DTCM Cortex-M/LLRAM Cortex-R, swapped between SWC calls if static */
#define MBANK_BACKUP    U(4)    /* backup SRAM addressed only by STREAM */
#define MBANK_HWIODMEM  U(5)    /* memory space for I/O and DMA buffers */
#define MBANK_PMEM      U(6)    /* program RAM */

#define NB_MEMINST_OFFSET U(7)  /* offsets */

/* upon STREAM_MEMREQ commands the swc can return up to 6 memory requests for its instance 
    scratch fast, DTCM, static fast, internal L2, external */

#define MAX_NB_MEM_REQ_PER_NODE U(6)  /* table of PACKSWCMEM requirements (speed, size, alignment..) see MAXNB_MEMORY_BASE_MALLOC */

#define GRAPH_MEM_REQ (U(MEM_WORKING_INTERNAL)+U(1)) 

#define MAX_NB_STREAM_SWC U(4)             


/*
 * -----------------------------------------------------------------------
 */


#endif /* #ifndef cSTREAM_NODES_H */

/*
 * -----------------------------------------------------------------------
 */
