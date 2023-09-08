/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Tools
 * Title:        stream_tool_library.c
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
    

#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdarg.h>  /* for void fields_extract(char **pt_line, char *types,  ...) */
#include <stdlib.h>
#include <math.h>

#include "stream_tool_include.h"

/**
  @brief            read the file to a table of char
  @param[in/out]    none
  @return           int

  @par             
  @remark
 */
void jump2next_line(char **line)
{
    while (*(*line) != '\n') 
    {   (*line)++;
    };
    (*line)++;
}

/**
  @brief            read a line 
  @param[in/out]    none
  @return           int

  @par             
  @remark
 */
void read_data_v(char **pt_line, void *X, uint8_t *raw_type, uint32_t *nb_option)
{
    char* ptstart, *ptend, stype[9], *ptchar, inputchar[200];
    uint8_t *ptu8;
    uint16_t *ptu16;
    uint32_t ifield, nfield, *ptu32;
    uint64_t *ptu64;
    int64_t i;
    float  f32, *ptf32;
    double f64, *ptf64;

    while (*(*pt_line) == ';')      /* lines starting with ';' are skiped as comments */
    {   jump2next_line(pt_line);
    }

    /* read the header of the line : number of fields and type */
    ptstart = *pt_line;   
    ptend = strchr(ptstart, ';');
    i = ptend - ptstart;
    strncpy(inputchar, ptstart, (int)i); inputchar[i] = '\0';
    sscanf(inputchar, "%d %s;", &nfield, stype);

    /* find the start of the fields */
    ptstart = strchr(ptstart, ';');
    ptstart++; 
    while (*ptstart == ' ') ptstart++;  // find the first non-white space character

    if (0 == strcmp(stype, "u8") || 0 == strcmp(stype, "i8"))
    {   ptu8 = (uint8_t*)X; *raw_type = STREAM_U8;
        for (ifield = 0; ifield < nfield; ifield++)
        {   sscanf(ptstart, "%lld", &i);  
            *ptu8++ = i;  ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "u16") || 0 == strcmp(stype, "i16") || 0 == strcmp(stype, "h16"))
    {   ptu16 = (uint16_t*)X; *raw_type = STREAM_U16;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h16")) sscanf(ptstart, "%llx", &i);
            else                           sscanf(ptstart, "%lld", &i);  
            *ptu16++ = i;  ptstart = strchr(ptstart, ' ');  while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "u32") || 0 == strcmp(stype, "i32") || 0 == strcmp(stype, "h32"))
    {   ptu32 = (uint32_t*)X; *raw_type = STREAM_U32;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h32")) sscanf(ptstart, "%llx", &i);
            else                           sscanf(ptstart, "%lld", &i);
            *ptu32++ = i;  ptstart = strchr(ptstart, ' ');  while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "u64") || 0 == strcmp(stype, "i64") || 0 == strcmp(stype, "h64"))
    {   ptu64 = (uint64_t*)X; *raw_type = STREAM_U64;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h64")) sscanf(ptstart, "%llx", &i);
            else                           sscanf(ptstart, "%lld", &i);
            *ptu64++ = i;  ptstart = strchr(ptstart, ' ');  while (*ptstart == ' ') ptstart++;
        }
    }
    
    if (0 == strcmp(stype, "f32"))
    {   ptf32 = (float*)X; *raw_type = STREAM_FP32;
        for (ifield = 0; ifield < nfield; ifield++)
        {   sscanf(ptstart, "%f", &f32);  
            *ptf32++ = f32;  ptstart = strchr(ptstart, ' ');  while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "f64"))
    {   ptf64 = (double*)X; *raw_type = STREAM_FP64;
        for (ifield = 0; ifield < nfield; ifield++)
        {   sscanf(ptstart, "%lf", &f64);  
            *ptf64++ = f64;  ptstart = strchr(ptstart, ' ');  while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "c"))
    {   ptchar = (char*)X;
        for (ifield = 0; ifield < nfield; ifield++)
        {   sscanf(ptstart, "%s", &inputchar);  
            strcpy(ptchar, inputchar);  
            ptchar = ptchar + strlen(inputchar) + 1;
            ptstart = strchr(ptstart, ' ');
            while (*ptstart == ' ') ptstart++;
        }
    }

    /* return nb and type information */
    *nb_option = nfield;

    /* skip the end of line comments */
    *pt_line = ptstart;
    jump2next_line(pt_line);
}
/* ---------------------------------------------------------------------- */
uint32_t INTTOFPE4M6(uint32_t x)
{
    uint32_t E, M;

    for (E = 0; E <= PARAM_MAX_EXPONENT; E++)
    {   for (M = 0; M <= PARAM_MAX_MANTISSA; M++)
        {   if ((M << E) >= x)
                return (E<<6) | M;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------- */
uint32_t jump2next_valid_line (char **line)
{
    while ((*line)[0] == ';')     /* skip lines starting with ';' */
        jump2next_line(line);

    if (0 == strncmp (*line,"END",8))
        return FOUND_END_OF_FILE;
    else
        return NOT_YET_END_OF_FILE;
}    

/* ---------------------------------------------------------------------- */
uint32_t quantized_FS (float FS)
{
    /* 20bits : mantissa [U18], exponent [U2], FS = (Mx2)/2^(8xE), 0<=>ASYNCHRONOUS/SLAVE */
    /* 20 range = (E=0,1,2,3) 262142/2^0 .. 2/2^24 [262kHz .. 3 months] */    
    /* 524kHz  ..  1/8.388.608 Hz  [~1MHz .. 3months] */    
    uint32_t E, M;
    float x;

    for (E = FMT_FS_MAX_EXPONENT; E >= 0; E--)
    {   for (M = 0; M <= FMT_FS_MAX_MANTISSA; M++)
        {   x = (float)FMTQ2FS(E,M);
            if (x >= FS)
            {   
                return (E<<FMT_FS_EXPSHIFT) | M;
            }
        }
    }
    return 0;

}


/**
  @brief            read the file to a table of char
  @param[in/out]    none
  @return           int

  @par             
  @remark
 */
void read_input_file(char* file_name, char * inputFile)
{ 
    FILE * ptf_platform_manifest_file;
    uint32_t idx;
    if (0 == (ptf_platform_manifest_file = fopen(file_name, "rt"))) exit(-1);
    idx = 0;
    while (1) if (0 == fread(&(inputFile[idx++]), 1, 1, ptf_platform_manifest_file)) break;
    fclose(ptf_platform_manifest_file);
}


/**
  @brief            (main)
  @param[in/out]    none
  @return           int

  @par              translates the graph intermediate format GraphTxt to GraphBin to be reused
                    in CMSIS-Stream/stream_graph/*.txt
  @remark
 */

/* ----------------------------------------------------------------------
 example : fields_extract(&pt_line, "III", &ARC_ID, &IFORMAT, &SIZE);
*/

void fields_extract(char **pt_line, char *types,  ...)
{
    char *ptstart, *ptstart0, S[200], *vaS;
    uint32_t ifield, I, *vaI, nchar, n, nfields;
    va_list vl;
    float F, *vaF;
#define COMMENTS 'c'
#define FLOAT 'f'
#define INTEGER 'i'
#define HEXADECIMAL 'h'

    va_start(vl,types);
    ptstart = *pt_line;
    nfields = (uint32_t)strlen(types);

    while (*(*pt_line) == ';') 
    {   jump2next_line(pt_line); 
        ptstart = *pt_line;
    }

    for (ifield = 0; ifield < nfields; ifield++)
    {
        if (types[ifield] == COMMENTS)
        {   ptstart0 = strchr (ptstart, '\n'); 
        } else
        {   ptstart0 = strchr (ptstart, ';'); 
        }

        switch(types[ifield])
        {
            case COMMENTS:
                nchar = (uint32_t)((uint64_t)ptstart0 - (uint64_t)ptstart);
                vaS = va_arg (vl,char *);
                strncpy(vaS, ptstart, nchar);
                vaS[nchar] = 0;
                break;

            case FLOAT:
                n = sscanf (ptstart,"%f",&F);
                vaF = va_arg (vl,float *);
                *vaF = F;
                break;

            default:
            case INTEGER:
                n = sscanf (ptstart,"%d",&I);
                vaI = va_arg (vl,uint32_t *);
                *vaI = I;
                break;

            case HEXADECIMAL:
                n = sscanf (ptstart,"%s",S);
                n = sscanf(&(S[1]),"%X",&I); /* remove the 'h' */
                vaI = va_arg (vl,uint32_t *);
                *vaI = I;
                break;
        }

        ptstart = ptstart0 + 1;      /* skip the ';\n' separators */
    }
    *pt_line = ptstart;
    va_end(vl);
}


/**
  @brief            read the file to a table of char
  @param[in/out]    none
  @return           int

  @par
  @remark
 */
void read_data_vector(char** pt_line, char* types, ...)
{
    char* ptstart, * ptstart0, S[200], * vaS;
    uint32_t ifield, I, * vaI, nchar, n, nfields;
    va_list vl;
    float F, * vaF;
#define COMMENTS 'c'
#define FLOAT 'f'
#define INTEGER 'i'
#define HEXADECIMAL 'h'

    va_start(vl, types);
    ptstart = *pt_line;
    nfields = (uint32_t)strlen(types);

    while (*(*pt_line) == ';')
    {
        jump2next_line(pt_line);
        ptstart = *pt_line;
    }

    for (ifield = 0; ifield < nfields; ifield++)
    {
        if (types[ifield] == COMMENTS)
        {
            ptstart0 = strchr(ptstart, '\n');
        }
        else
        {
            ptstart0 = strchr(ptstart, ';');
        }

        switch (types[ifield])
        {
        case COMMENTS:
            nchar = (uint32_t)((uint64_t)ptstart0 - (uint64_t)ptstart);
            vaS = va_arg(vl, char*);
            strncpy(vaS, ptstart, nchar);
            vaS[nchar] = 0;
            break;

        case FLOAT:
            n = sscanf(ptstart, "%f", &F);
            vaF = va_arg(vl, float*);
            *vaF = F;
            break;

        default:
        case INTEGER:
            n = sscanf(ptstart, "%d", &I);
            vaI = va_arg(vl, uint32_t*);
            *vaI = I;
            break;

        case HEXADECIMAL:
            n = sscanf(ptstart, "%s", S);
            n = sscanf(&(S[1]), "%X", &I); /* remove the 'h' */
            vaI = va_arg(vl, uint32_t*);
            *vaI = I;
            break;
        }

        ptstart = ptstart0 + 1;      /* skip the ';\n' separators */
    }
    *pt_line = ptstart;
    va_end(vl);
}


/*  
    first letters of the lines in the input file 
*/
#define _comments    'c'
#define _offsets     'o'
#define _header      'h'
#define _format      'f'
#define _stream_inst 'i'
#define _linked_list 'l'
#define _script      's'
#define _arc         'a'
#define _RAM         'r'
#define _debug       'd'

#define PRINTF(d) fprintf(ptf_graph_bin, "0x%08X, // %03X %03X\n", (d), addrBytes, addrBytes/4); addrBytes += 4;





#ifdef __cplusplus
}
#endif