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

#include "stream_tool_include.h"

#ifndef cSTREAM_TOOL_GRAPH_H
#define cSTREAM_TOOL_GRAPH_H


        graph format structure = 
                
                struct 
                    /*--------------- WORD 0 - producer frame size, raw format, interleaving --------------- */
                    #define SIZSFTRAW_FMT0   0
                        #define MULTIFRME_FMT0_MSB 31 
                        #define       RAW_FMT0_LSB 25 
                        #define INTERLEAV_FMT0_MSB 24 
                        #define FRAMESIZE_FMT0_LSB  0 
                    /*--------------- WORD 1 - sampling rate , nchan  -------------*/
                    #define   SAMPINGNCHANM1_FMT1  U( 1)
                        #define  TIMSTAMP_FMT1_LSB U(25) 
                        #define  SAMPLING_FMT1_MSB U(24) /* 20 mantissa 16b + exponent 4b, FS= 0 <=> ASYNCHRONOUS/SERVANT */
                        #define   NCHANM1_FMT1_LSB U( 0) /* 5  nb channels-1 [1..32] */
                    /*--------------- WORD 2 -  direction, channel mapping (depends on the "stream_io_domain")------*/
                        #define  BORDER_FMT2_LSB U(18)
                        #define   RATIO_FMT2_MSB U(17) /* 4 ratio to height : 1:1 4:3 16:9 16:10 5:4  */
                        #define   WIDTH_FMT2_LSB U( 0) 




                struct stream_format_io 
                    IO stream descriptors,
                       DOMAIN_IOFMT
                     INSTANCE_IOFMT
                      EXDTCMD_IOFMT
                    IOCOMMAND_IOFMT
                      SERVANT_IOFMT
                       RX0TX1_IOFMT
                    FW_IO_IDX_IOFMT
                      IOARCID_IOFMT

                     SETTINGS_IOFMT2_


                struct stream_local_instance
                    stream instances : IO mapping and debug arcs
                    #define STREAM_INSTANCE_SIZE 20         /* 4 words + 16words offset maximum (8 x 64bits addresses) */
                    #define STREAM_INSTANCE_WHOAMI_PORTS 0  /* _PARCH_ fields */
                    #define STREAM_INSTANCE_PARAMETERS 1    /* _PARINST_ fields */
                    #define STREAM_INSTANCE_IOMASK 2        /* _IOMASK_ fields */
                    #define STREAM_INSTANCE_IOREQ 3         /* _IOREQ_ fields */
                    #define STREAM_INSTANCE_OFFSETS 4       /* _OFFSET_ fields */

                              INSTANCE_PARCH
                                PROCID_PARCH
                            SWC_W32OFF_PARCH
                            TRACE_ARC_PARINST
                            BOUNDARY_IOMASK
                            REQMADE_IO

                
                struct IO nodes [] (" from the initial graph TXT = desired domain)
                
                
                Nodes : instanceID, indexed memory banks, debug info, default processors/arch, 
                    parameter preset/size/tags, list of arcs/streaming-arcs


                          PROCID_LW0
                          ARCHID_LW0
                          NBARCW_LW0
                         ARCLOCK_LW0
                         ARCSRDY_LW0
                         SWC_IDX_LW0

                         DBGB1_LW1
                         DBGB1_LW1
                          ARC1_LW1
                          ARC1_LW1
                
                          TCM_INST_LW2
                           NBALLOC_LW2
                         BASEIDXOFFLW2
                           DATAOFF_LW2
                           BASEIDX_LW2
                         BASEIDXOFFLW2

                          SELPARAM_LW3
                         W32LENGTH_LW3
                           VERBOSE_LW3
                          NEWPARAM_LW3
                            PRESET_LW3



                Scripts code, pointer/size + malloc/stdlib
                table of pointer to scripts
                


                memory banks size accumulators, physical address, speed
                table of struct 
                

                struct stream_platform_manifest
                    struct stream_interfaces
                    digital platform : processors ID/arch/IO-affinity
                        domain ios, fwioIDX, platform name


                list of arcs: buffer memory bank (base,size), debug, node src/dst +arc idx
                    uint8_t crossref_arc_using_fmt [maxNb arc] <= FMT ID

                                                                
    struct arcStruct arc_flow[MAX_NB_ARC_STREAM]; /* rx0tx1, domain, digital format and FS accuracy */


#endif /* #ifndef cSTREAM_TOOL_GRAPH_H */
#ifdef __cplusplus
}
#endif
