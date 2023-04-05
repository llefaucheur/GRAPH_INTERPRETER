/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        stream_const.h
 * Description:  public references for the application using CMSIS-STREAM
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
#ifndef cSTREAM_PUBLIC_H
#define cSTREAM_PUBLIC_H

#include <stdint.h>
#include <string.h> 

/*============================ SWITCHES =====================================*/
#define PLATFORM_ARCH_32BIT
#define MULTIPROCESS 0      /* one process, one processor */

#ifdef _MSC_VER 
#define RUN_ON_COMPUTER_ 1
#else
#define RUN_ON_COMPUTER_ 0
#endif
/*===========================================================================*/

#include "stream_macro.h"      /* defines */
#include "stream_nodes.h"      /* swc */
#include "stream_platform.h"   /* hw/sw dependencies */
#include "stream_arcs.h"       /* buffers */
#include "stream_graph.h"      /* graph list */


#endif /* cSTREAM_PUBLIC_H */
/*
 * -----------------------------------------------------------------------
 */
