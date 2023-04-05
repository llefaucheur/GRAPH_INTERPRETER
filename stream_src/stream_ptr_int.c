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

#include "stream_const.h"  
#include "stream_types.h"
/*
   MISRA 2012 RULES 11.XX
 */

void * convert_intp_to_voidp (intPtr_t *in)
{   
    return (void *) in;
}

struct stream_local_instance * convert_intp_to_instancep (intPtr_t *in)
{   
    return (struct stream_local_instance *) in;
}

void * convert_int_to_voidp (intPtr_t in)
{   
    return (void *) in;
}

void * convert_sdata_to_voidp (data_buffer_t *in)
{   
    return (void *) in;
}

stream_parameters_t * convert_voidp_to_sparam (void *in)
{
    return (stream_parameters_t *)in;
}

intPtr_t convert_voidp_to_int (void *in)
{
    return (intPtr_t)in;
}

intPtr_t convert_charp_to_int (uint8_t *in)
{
    return (intPtr_t)in;
}

data_buffer_t * convert_charp_to_xdm (uint8_t *in)
{
    return (data_buffer_t *)in;
}
