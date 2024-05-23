/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_filter.c
 * Description:  filters
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
   

#ifndef carm_stream_script_INSTRUCTIONS_H
#define carm_stream_script_INSTRUCTIONS_H

#define iBRANCHs 0x00
#define iCJUMP   0x01

#define iCOPY    0x01
#define iLOADK   0x02

#define iRET     0x00 
#define iRETP    0x01 
#define iPSHT    0x02 
#define iRESET   0x03 
#define iDUP     0x04 
#define iNOP     0x05 
#define iSWAPXY  0x06 
#define iCCOPY   0x07 
#define iEQU     0x08 
#define iLTE     0x09 
#define iLTS     0x0A 
#define iEQ0     0x0B 
#define iLE0     0x0C 
#define iLT0     0x0D 
#define iNEGC    0x0E 
#define iBIT0    0x0F 
#define iBIT1    0x10 
#define iADD     0x11 
#define iSUB     0x12 
#define iRSB     0x13 
#define iMUL     0x14 
#define iDIV     0x15 
#define iDIVI    0x16 
#define iMOD     0x17 
#define iASHFT   0x18 
#define iOR      0x19 
#define iXOR     0x1A 
#define iAND     0x1B 
#define iNOR     0x1C 
#define iNOT     0x1D 
#define iNEG     0x1E 
#define iNORM    0x1F 
#define iSQRA    0x20 
#define iINC     0x21 
#define iDEC     0x22 
#define iBANZ    0x23 
#define iBITSET  0x24 
#define iBITCLR  0x25 
#define iMAX     0x26 
#define iMIN     0x27 
#define iCONV    0x28 
#define iCAST    0x29 

#endif
#ifdef __cplusplus
}
#endif
 
