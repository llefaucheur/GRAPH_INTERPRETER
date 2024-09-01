/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        main.c
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

#include "platform.h"
#include <stdlib.h>
#include <stdio.h>

/*   mask = ~((1 << (7 & mem_req_2bytes_alignment) -1) */

extern void main_init(void);
extern void main_run(void);
extern void main_stop(void);

int main(void)
{   int i;

    main_init();

    for (i = 0; i < 100000; i++)
    {   main_run();
    }

    main_stop();

    fprintf(stderr, "------------------\n end of simulation \n------------------");
}

//#endif