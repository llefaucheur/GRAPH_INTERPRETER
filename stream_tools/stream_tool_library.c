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

uint8_t globalEndFile;
uint8_t FoundEndSection;
 
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
  @brief            compute the memory consumption of a script => ARC buffer size in Bytes
 */

void script_memory_consumption(struct stream_script *script)
{
    /* minimum state memory = (0 register + 0 pointer + 1 special) x 8 Bytes = 8 Bytes + stack  */
    script->nbw32_allocated = SCRIPT_REGSIZE;   /* special registers */
    script->nbw32_allocated += script->nb_reg * SCRIPT_REGSIZE;
    script->nbw32_allocated += script->nb_ptr * SCRIPT_REGSIZE;
    script->nbw32_allocated += script->nb_stack * SCRIPT_REGSIZE;
}


/**
  @brief            MALLOC with VIDs
  @param[in/out]    
  @return           

  @par             
  @remark
  parse the membank

 */
int vid_malloc (uint32_t VID, intPtr_t size, uint32_t alignment, 
                uint32_t *pack27b, int working,       
                char *comments, 
                struct stream_platform_manifest *platform,       
                struct stream_graph_linkedlist *graph) 
{ 
    uint32_t found, ibank, offset, offsetID;
    uint64_t alignmask;
    char tmpstring[NBCHAR_LINE];

    /*  platform->membank[ibank].base32;    
        find the ibank associated to VID(input) = virtualID(processor_memory_bank)
        increment its ptalloc_static with the corresponding alignment
    */

    for (found = ibank = 0; ibank < platform->nbMemoryBank_detailed; ibank++)
    {
        if (platform->membank[ibank].virtualID == VID)
        {   found = 1;
            offsetID = platform->membank[ibank].offsetID;
            offset = (uint32_t)(platform->membank[ibank].ptalloc_static);
            break;
        }
    }

    if (found == 0) 
        exit(-5);

    sprintf(tmpstring, " OFF %d BASE 0x%04X PT 0x%04X MAXW 0x%04X SIZE %04X", 
        offsetID, offset, platform->membank[ibank].ptalloc_static, platform->membank[ibank].max_working_booking, size);

    alignmask =  ~((1 << (7&alignment)) -1);
    size = (size + 3) & alignmask;
     
    *pack27b = 0;
    ST(*pack27b, DATAOFF_ARCW0, offsetID);
    ST(*pack27b, BASEIDX_ARCW0, offset);

    if (working == MEM_TYPE_WORKING)
    {   if (size > platform->membank[ibank].max_working_booking)
    {   {   platform->membank[ibank].max_working_booking = size;
        }
    }
    } else // MEM_TYPE_STATIC, MEM_TYPE_PERIODIC_BACKUP
    {   platform->membank[ibank].ptalloc_static += size;
    }
    
    if (platform->membank[ibank].ptalloc_static +
        platform->membank[ibank].max_working_booking > 
        platform->membank[ibank].size)
    {   /* check overflow */
        exit(-7);
    }

    strcpy(platform->membank[ibank].comments, comments);
    strcat(platform->membank[ibank].comments, tmpstring);

    fprintf(graph->ptf_header,"                        // %s \n",platform->membank[ibank].comments);

    return 0;   // success
} 
  
/**
  @brief            read the file to a table of char
  @param[in/out]    none
  @return           int

  @par             
  @remark
 */
void jump2next_line(char **pt_line)
{
    while (*(*pt_line) != '\n') 
    {   (*pt_line)++;
    };
    (*pt_line)++;
}

/* 
    find a line which does not start with ';'
    return : line is pointing to the next valid character 
*/
void jump2next_valid_line(char **pt_line)
{
    int i;
    char *p;

    FoundEndSection = 0;

L_jump2next_valid_line:
    jump2next_line(pt_line);

    p = *pt_line;
    for (i = 0; i < NBCHAR_LINE; i++)
    {   if (' ' != (*p)) break;
        p++;
    }

    if ((*p) == ';' || (*p) == '\n')
    {   goto L_jump2next_valid_line;
    }

    *pt_line = p;

    if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
        globalEndFile = FOUND_END_OF_FILE;
    else
        globalEndFile = NOT_YET_END_OF_FILE;
}


/**
  @brief            read a line 
  @param[in/out]    none
  @return           

  @par             
  @remark
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
    int32_t ifield, nfield, *ptu32;
    uint64_t *ptu64;
    int64_t i;
    float  f32, *ptf32;
    double f64, *ptf64;
    #define LL NBCHAR_LINE

    if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
    {   *nbfields = 0;
        jump2next_valid_line (pt_line);
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

    if (0 == strcmp(stype, "u8") || 0 == strcmp(stype, "i8") || 0 == strcmp(stype, "s8") || 0 == strcmp(stype, "h8"))
    {   ptu8 = (uint8_t*)X; *raw_type = STREAM_U8; 
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h8")) c = sscanf(ptstart, "%llx", &i);
            else                          c = sscanf(ptstart, "%lld", &i);  
            *ptu8++ = (uint8_t)i;  
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "u16") || 0 == strcmp(stype, "i16") ||  0 == strcmp(stype, "s16") || 0 == strcmp(stype, "h16"))
    {   ptu16 = (uint16_t*)X; *raw_type = STREAM_U16;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h16")) c = sscanf(ptstart, "%llx", &i);
            else                           c = sscanf(ptstart, "%lld", &i);  
            *ptu16++ = (uint16_t)i; 
            ptstart = strchr(ptstart, ' '); while (*ptstart == ' ') ptstart++;
        }
    }

    if (0 == strcmp(stype, "u32") || 0 == strcmp(stype, "i32") || 0 == strcmp(stype, "s32") || 0 == strcmp(stype, "h32"))
    {   ptu32 = (uint32_t*)X; *raw_type = STREAM_U32;
        for (ifield = 0; ifield < nfield; ifield++)
        {   if (0 == strcmp(stype, "h32")) c = sscanf(ptstart, "%llx", &i);
            else                           c = sscanf(ptstart, "%lld", &i);
            *ptu32++ = (uint32_t)i;  
        }
    }

    if (0 == strcmp(stype, "u64") || 0 == strcmp(stype, "i64") || 0 == strcmp(stype, "s64") || 0 == strcmp(stype, "h64"))
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

    jump2next_valid_line (pt_line);
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

    if (0 == (ptf_platform_manifest_file = fopen(file_name, "rt")))
        exit(-1);
    idx = 0;
    while (1) {
        if (0 == fread(&(inputFile[idx++]), 1, 1, ptf_platform_manifest_file)) 
            break;
    }
    fclose(ptf_platform_manifest_file);
}



/**
  @brief            search a word in a long string
  @param[in/out]    strings
  @return           int

  @par              
  @remark
 */

int search_word(char line[], char word[])
{
    int i, j, found, L;

    L = (int)strlen(line);
    L = L - (int)strlen(word);
    for (i = 0; i <= L; i++) {
        found = 1;
        for (j = 0; j < strlen(word); j++) {
            if (line[i + j] != word[j]) {
                found = 0;
                break;
            }
        }
        if (found == 1) {
            return i;
        }
    }

    return -1;
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
    options sets : { index  list } { index  list } 
 
    when index == 0 it means "any", the list can be empty, the default value is not changed from reset   
 
    when index > 0 the list gives the allowed values the scheduler can select
        The Index tells the default "value" to take at reset time and to put in the graph 
            the combination of index give the second word of stream_format_io[]
         At reset : (*io_func)(STREAM_RESET, (uint8_t *)&stream_format_io_setting, 0);
         For tuning : (*io_func)(STREAM_SET_IO_CONFIG, (uint8_t *)&stream_format_io_setting, 0);
         Example 2  5 6 7 8 9    ; index 2, default = 6 (index starts at 1)
 
    when index < 0 a list of triplets follows to describe a combination of data intervals :  A1 B1 C1  A2 B2 C2 ... 
        A is starting value, B is the increment step, C is the included maximum value 
        The absolute index value selects the default value in this range   

    extract the search range to the next ';' or '\n'  search for '{' and '}' search the non-blank segments
    number of sets from '{}' 
    start with only considering {n X}
     (when the list has one single element "X", this is the value to consider : {X} <=> {1 X} <=> X)
*/

int fields_options_extract(char **pt_line, struct options *opt)
{
    int i, iBrace, nBrace, option_index;
    char *p;
    int bracePositions[MAX_NBOPTIONS_SETS * 2]; // open and close braces
    float A, B, C;

    p = *pt_line;

    for (iBrace = i = 0; i < NBCHAR_LINE; i++)
    {   if (*p == ';' || *p == '\n')  break;
        if (*p == '{')  
        {   bracePositions[iBrace++] = i;
        }
        p++;
    }

    nBrace = iBrace;
    if (nBrace == 0) return 1;  /* nothing */

#define NON_BLKSPACE()  {int i; for(i=0;i<NBCHAR_LINE;i++){if(*p != ' ') break; p++;}}  /* now p points to the start of data */ 
#define BLKSPACE() {int i; for(i=0;i<NBCHAR_LINE;i++){if(*p == '}') break; if(*p == ' ') break; p++;}} /* now p points to the end of data */ 

    /* loop on option sets */
    for (opt->nb_option = iBrace = 0; iBrace < nBrace; iBrace ++, opt->nb_option ++)
    {
        p = *pt_line;
        p = p + bracePositions[iBrace++] + 1;       /* p { x   idx */ 

        NON_BLKSPACE()  
        sscanf(p, "%d", &option_index);  BLKSPACE() NON_BLKSPACE()   
        opt->default_index[opt->nb_option] = option_index;

        if (option_index < 0)   /* is it an option range ? */
        {   
            sscanf(p, "%f", &A);    BLKSPACE() NON_BLKSPACE()   
            sscanf(p, "%f", &B);    BLKSPACE() NON_BLKSPACE()   
            sscanf(p, "%f", &C);    BLKSPACE() NON_BLKSPACE()   
            opt->optionRange[opt->nb_option][0] = A;
            opt->optionRange[opt->nb_option][1] = B;
            opt->optionRange[opt->nb_option][2] = C;
        }

        if (option_index > 0)       /* is it an option list ? */
        {   do {   
            sscanf(p, "%f", &A);    BLKSPACE() NON_BLKSPACE()   
            
            opt->optionList[opt->nb_option][opt->nbElementsInList[opt->nb_option]] = A;
            opt->nbElementsInList[opt->nb_option] ++;
            } while (*p != '}');
        }
    }

    jump2next_valid_line (pt_line);
    return 1;
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

int fields_extract(char **pt_line, char *types,  ...)
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

    if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
    {   FoundEndSection = 1;
        return -1;
    }

    ptstart = *pt_line;
    nfields = (int)strlen(types);
    ptstart0 = ptstart;

    for (ifield = 0; ifield < nfields; ifield++)
    {
        while (*ptstart0 == ' ')            // find the next non-white space
        {   ptstart0++;
            if (*ptstart0 == '\n')
                break;
        }

        switch(types[ifield])
        {
            case Characters:
            case CHARACTERS:
                n = sscanf (ptstart0,"%s",&S);
                vaS = va_arg (vl,char *);
                nchar = (int)strlen(S);
                strncpy(vaS, ptstart0, nchar);
                vaS[nchar] = 0;
                break;

            case Float:
            case FLOAT:
                n = sscanf (ptstart0,"%f",&F);
                vaF = va_arg (vl,float *);
                *vaF = F;
                break;

            default:
            case Integer:
                n = sscanf (ptstart0,"%d",&I);
                vaI = va_arg (vl,int *);
                *vaI = I;
                break;
            case INTEGER:
                n = sscanf (ptstart0,"%ld",&IL);
                vaIL = va_arg (vl,long *);
                *vaIL = IL;
                break;

            case Hexadecimal:
                n = sscanf (ptstart0,"%s",S);
                n = sscanf(&(S[1]),"%X",&I); /* remove the 'h' */
                vaI = va_arg (vl,int *);
                *vaI = I;
                break;
            case HEXADECIMAL:
                n = sscanf (ptstart0,"%s",S);
                n = sscanf(&(S[1]),"%lX",&IL); 
                vaIL = va_arg (vl,long *);
                *vaIL = IL;
                break;
        }

        while (*ptstart0 != ' ')            // find the next non-white space
        {   if (*ptstart0 == '\n')
                break;
            ptstart0++;
        }
        ptstart = ptstart0;
    }

    *pt_line = ptstart;
    jump2next_valid_line (pt_line);
    if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
    {   globalEndFile = FOUND_END_OF_FILE;
    }
    va_end(vl);

    return 1;
}

#ifdef __cplusplus
}
#endif