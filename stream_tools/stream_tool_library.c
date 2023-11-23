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
    
#include "stream_tool_include.h"

int stream_bitsize_of_raw(uint8_t raw)
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
  @brief            LCM / GCD of two integers
  @param[in/out]    none
  @return           int

  @par             
  @remark
 */

// Recursive function to return gcd of a and b 
uint32_t gcd(uint32_t a, uint32_t b) 
{ 
    if (b == 0) 
        return a; 
    return gcd(b, a % b); 
} 
  
// Function to return LCM of two numbers 
uint32_t lcm(uint32_t a, uint32_t b) { return (a / gcd(a, b)) * b; } 


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

/* 
    find a line which does not start with ';'
    return : line is pointing to the next valid character 
*/
int jump2next_valid_line(char **line)
{
    int i;
    char *p;

L_jump2next_valid_line:
    jump2next_line(line);
    p = *line;
    for (i = 0; i < NBCHAR_LINE; i++)
    {   if (' ' != (*p)) break;
        p++;
    }
    if ((*p) == ';' || (*p) == '\n')
    {   goto L_jump2next_valid_line;
    }

    *line = p;

    if (0 == strncmp (*line,"_END_",strlen("_END_")))
        return FOUND_END_OF_FILE;
    else
        return NOT_YET_END_OF_FILE;
}


/**
  @brief            read a line 
  @param[in/out]    none
  @return           

  @par             
  @remark
        2  u8;  0 255                       new preset + tag (all)
        1 u16;  22                          byte length of the parameters from next line
        1  u8;  2                           Two biquads
        1  u8;  0                           postShift
        5 h16; 5678 2E5B 71DD 2166 70B0     b0/b1/b2/a1/a2 
        5 h16; 5678 2E5B 71DD 2166 70B0     second biquad
 */
void read_binary_param(char **pt_line, void *X, uint8_t *raw_type, uint32_t *nbfields)
{
    char c, *ptstart, *ptend, stype[9], *ptchar, inputchar[200];
    uint8_t *ptu8;
    uint16_t *ptu16;
    uint32_t ifield, nfield, *ptu32;
    uint64_t *ptu64;
    int64_t i;
    float  f32, *ptf32;
    double f64, *ptf64;
    #define LL NBCHAR_LINE

    jump2next_valid_line(pt_line);

    if (0 == strncmp(*pt_line, SECTION_END, strlen(SECTION_END)))
    {   *nbfields = 0;
        return;
    }

    /* read the header of the line : number of fields and type */
    ptstart = *pt_line;   
    ptend = strchr(ptstart, ';');
    i = ptend - ptstart;
    strncpy(inputchar, ptstart, (int)i); inputchar[i] = '\0';
    c = sscanf(inputchar, "%d %s;", &nfield, stype);
    stype[8] = '\0';

    /* find the start of the fields */
    ptstart = strchr(ptstart, ';');
    ptstart++; 
    while (*ptstart == ' ') ptstart++;  // find the first non-white space character

    if (0 == strncmp(stype, "u8", LL) || 0 == strncmp(stype, "i8", LL))
    {   ptu8 = (uint8_t*)X; *raw_type = STREAM_U8; 
        for (ifield = 0; ifield < nfield; ifield++)
        {   c = sscanf(ptstart, "%lld", &i);  *ptu8++ = (uint8_t)i;  
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "u16") || 0 == strcmp(stype, "i16") || 0 == strcmp(stype, "h16"))
    {   ptu16 = (uint16_t*)X; *raw_type = STREAM_U16;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h16")) c = sscanf(ptstart, "%llx", &i);
            else                           c = sscanf(ptstart, "%lld", &i);  
            *ptu16++ = (uint16_t)i; 
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "u32") || 0 == strcmp(stype, "i32") || 0 == strcmp(stype, "h32"))
    {   ptu32 = (uint32_t*)X; *raw_type = STREAM_U32;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h32")) c = sscanf(ptstart, "%llx", &i);
            else                           c = sscanf(ptstart, "%lld", &i);
            *ptu32++ = (uint32_t)i;  
        }
    }

    if (0 == strcmp(stype, "u64") || 0 == strcmp(stype, "i64") || 0 == strcmp(stype, "h64"))
    {   ptu64 = (uint64_t*)X; *raw_type = STREAM_U64;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h64")) c = sscanf(ptstart, "%llx", &i);
            else                           c = sscanf(ptstart, "%lld", &i);
            *ptu64++ = i; 
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }
    
    if (0 == strcmp(stype, "f32"))
    {   ptf32 = (float*)X; *raw_type = STREAM_FP32;
        for (ifield = 0; ifield < nfield; ifield++)
        {   c = sscanf(ptstart, "%f", &f32);  *ptf32++ = f32; 
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "f64"))
    {   ptf64 = (double*)X; *raw_type = STREAM_FP64;
        for (ifield = 0; ifield < nfield; ifield++)
        {   c = sscanf(ptstart, "%lf", &f64);  *ptf64++ = f64;  
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "c"))
    {   ptchar = (char*)X;
        for (ifield = 0; ifield < nfield; ifield++)
        {   c = sscanf(ptstart, "%s", &inputchar);  
            strcpy(ptchar, inputchar);  
            ptchar = ptchar + strlen(inputchar) + 1;
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    /* return nb and type information */
    *nbfields = nfield;

    /* skip the end of line comments */
    *pt_line = ptstart;
}
/* ---------------------------------------------------------------------- */
//uint32_t INTTOFPE4M6(uint32_t x)
//{
//    uint32_t E, M;
//
//    for (E = 0; E <= PARAM_MAX_EXPONENT; E++)
//    {   for (M = 0; M <= PARAM_MAX_MANTISSA; M++)
//        {   if ((M << E) >= x)
//                return (E<<6) | M;
//        }
//    }
//    return 0;
//}


/* ---------------------------------------------------------------------- */
uint32_t quantized_FS (float FS)
{
    ///* 20bits : mantissa [U18], exponent [U2], FS = (Mx2)/2^(8xE), 0<=>ASYNCHRONOUS/SLAVE */
    ///* 20 range = (E=0,1,2,3) 262142/2^0 .. 2/2^24 [262kHz .. 3 months] */    
    ///* 524kHz  ..  1/8.388.608 Hz  [~1MHz .. 3months] */    
    //uint32_t E, M;
    //float x;

    //for (E = FMT_FS_MAX_EXPONENT; E >= 0; E--)
    //{   for (M = 0; M <= FMT_FS_MAX_MANTISSA; M++)
    //    {   x = (float)FMTQ2FS(E,M);
    //        if (x >= FS)
    //        {   
    //            return (E<<FMT_FS_EXPSHIFT) | M;
    //        }
    //    }
    //}
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
    int ifield, I, *vaI, nchar, n, nfields;
    long IL,*vaIL;
    va_list vl;
    float F, *vaF;
#define Characters 'c'
#define CHARACTERS 'C'
#define Float 'f'
#define FLOAT 'F'
#define Integer 'i'
#define INTEGER 'I'
#define Hexadecimal 'h'
#define HEXADECIMAL 'H'

    va_start(vl,types);

    jump2next_valid_line(pt_line); 
    ptstart = *pt_line;
    nfields = (int)strlen(types);

    for (ifield = 0; ifield < nfields; ifield++)
    {
        ptstart0 = strchr (ptstart, ' ');   // find the next white space
        while (ptstart0[1] == ' ') 
            ptstart0++;
        //ptstart1 = strchr (ptstart, '\n');   // find the next end of line
        //if (ptstart0-ptstart > ptstart1-ptstart)
        //{   ptstart0 = ptstart1;        // ptstart0 = next blank or end of line
        //}

        switch(types[ifield])
        {
            case Characters:
            case CHARACTERS:
                //nchar = (int)((uint64_t)ptstart0 - (uint64_t)ptstart);
                //vaS = va_arg (vl,char *);
                //strncpy(vaS, ptstart, nchar);
                //vaS[nchar] = 0;
                n = sscanf (ptstart,"%s",&S);
                vaS = va_arg (vl,char *);
                nchar = (int)strlen(S);
                strncpy(vaS, ptstart, nchar);
                vaS[nchar] = 0;
                break;

            case Float:
            case FLOAT:
                n = sscanf (ptstart,"%f",&F);
                vaF = va_arg (vl,float *);
                *vaF = F;
                break;

            default:
            case Integer:
                n = sscanf (ptstart,"%d",&I);
                vaI = va_arg (vl,int *);
                *vaI = I;
                break;
            case INTEGER:
                n = sscanf (ptstart,"%ld",&IL);
                vaIL = va_arg (vl,long *);
                *vaIL = IL;
                break;

            case Hexadecimal:
                n = sscanf (ptstart,"%s",S);
                n = sscanf(&(S[1]),"%X",&I); /* remove the 'h' */
                vaI = va_arg (vl,int *);
                *vaI = I;
                break;
            case HEXADECIMAL:
                n = sscanf (ptstart,"%s",S);
                n = sscanf(&(S[1]),"%lX",&IL); 
                vaIL = va_arg (vl,long *);
                *vaIL = IL;
                break;
        }

        ptstart = ptstart0 + 1;      /* skip the ';\n' separators */
    }
    *pt_line = ptstart;
    va_end(vl);
}


/**
  @brief            (main)
  @param[in/out]    none
  @return           int

  @par              translates the graph intermediate format GraphTxt to GraphBin to be reused
                    in CMSIS-Stream/stream_graph/*.txt
  @remark
 */

void fields_list(char **pt_line, struct options *opt)
{
    char *ptstart, *pend;
    int nfields;
    float F;

    jump2next_valid_line(pt_line); 
    ptstart = *pt_line;
    nfields = 0;
    sscanf (ptstart,"%d",&(opt->default_index));

    while (1)
    {   pend = strchr (ptstart, ' ');   
        while (pend[1] == ' ')          // find the next non white space
        {   pend++;
        }
        if (pend[1] == ';')
        {   break;
        }
        else
        {   nfields++;
        }

        sscanf (pend,"%f",&F);
        opt->options[nfields-1] = F;
        ptstart = pend + 1; 
    }
    *pt_line = pend;
    opt->nb_option = nfields;
}

/**
  @brief            read the file to a table of char
  @param[in/out]    none
  @return           int

  @par
  @remark
 */
void read_binary_paramector(char** pt_line, char* types, ...)
{
//    char* ptstart, * ptstart0, S[200], * vaS;
//    uint32_t ifield, I, * vaI, nchar, n, nfields;
//    va_list vl;
//    float F, * vaF;
//#define COMMENTS 'c'
//#define FLOAT 'f'
//#define INTEGER 'i'
//#define HEXADECIMAL 'h'
//
//    va_start(vl, types);
//    ptstart = *pt_line;
//    nfields = (uint32_t)strlen(types);
//
//    while (*(*pt_line) == ';')
//    {
//        jump2next_line(pt_line);
//        ptstart = *pt_line;
//    }
//
//    for (ifield = 0; ifield < nfields; ifield++)
//    {
//        if (types[ifield] == COMMENTS)
//        {
//            ptstart0 = strchr(ptstart, '\n');
//        }
//        else
//        {
//            ptstart0 = strchr(ptstart, ';');
//        }
//
//        switch (types[ifield])
//        {
//        case COMMENTS:
//            nchar = (uint32_t)((uint64_t)ptstart0 - (uint64_t)ptstart);
//            vaS = va_arg(vl, char*);
//            strncpy(vaS, ptstart, nchar);
//            vaS[nchar] = 0;
//            break;
//
//        case FLOAT:
//            n = sscanf(ptstart, "%f", &F);
//            vaF = va_arg(vl, float*);
//            *vaF = F;
//            break;
//
//        default:
//        case INTEGER:
//            n = sscanf(ptstart, "%d", &I);
//            vaI = va_arg(vl, uint32_t*);
//            *vaI = I;
//            break;
//
//        case HEXADECIMAL:
//            n = sscanf(ptstart, "%s", S);
//            n = sscanf(&(S[1]), "%X", &I); /* remove the 'h' */
//            vaI = va_arg(vl, uint32_t*);
//            *vaI = I;
//            break;
//        }
//
//        ptstart = ptstart0 + 1;      /* skip the ';\n' separators */
//    }
//    *pt_line = ptstart;
//    va_end(vl);
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



#ifdef __cplusplus
}
#endif