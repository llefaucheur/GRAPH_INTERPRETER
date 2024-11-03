/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Tools
 * Title:        generic graph translation to platform specific graph
 * Description:  translates the IO domain to specifics of the platform
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
#include "../stream_nodes/arm/script/arm_stream_script_instructions.h"

#include <ctype.h>  // isdigit


/*
;----------------------------------------------------------------------
    forget  { } \ /     comment ; 
    register r0..r11 sp sp1
    ALU     nop add addmod sub submod mul duv or nor and xor shr shl set clr max min amax amin norm 

    token   #  <registers>   [ ind ]   | bits | 

    instructions testleq..testlt if_yes if_no 
            swap delete save restore call callsys jump banz return

    set_op  type typeptr base size 

    type    int8 int16 int32 float16 float double ptrfloat ptruint8 ptrint32.. 


script 0
    script_code

    <register/number> = <register>
                      = #type number
                      = number
    TEST
        s[0]      1          2                  3           4        
        test<xx> <register>  <register/number>
                             <ALU>             <register> <register/number>
    LD
        s[0]      1          2                3           4        
        <register>  '='  <register/number>
                         <ALU>               <register> <register/number>
    JMOV
        s[0]        1            2            3           4        
        "set"      <register>   <set_op>   #type 
                                        <register/number>

        s[0]        1     2         3   4            5
        <register> '['  <register> ']' '='        <register>
        <register> '='  <register> '[' <register>   ']' 

        s[0]        1     2         3      4    5     6
        <register> '|'   lenK      posK   '|'  '='    <register>
        <register> '=' <register>  '|'   lenK posK   '|'

        s[0]        1             2
        "swap"      <register>   <register>

        s[0]        1           
        "delete"    <register/number>

        s[0]        1           2         3            4         5
        "save"      <register> <register> <register> <register> <register>
        "restore"   <register> <register> <register> <register> <register>
        "jump"      <Label>    <register> <register> <register> 
        "banz"      <Label>    <register> <register> <register> 
        "call"      <Label>    <register> <register> <register> 
        "callsys"   K          <register> <register> <register> 
        "return"
;----------------------------------------------------------------------
*/


#define INST_WORDS 4



/* ====================================================================================   
    Read and pack the script until finding "end" / SECTION_END

    script_assembler
    ....
    end               end of byte codes  
*/
void stream_tool_read_assembler(char **pt_line, struct stream_platform_manifest *platform,
                            struct stream_graph_linkedlist *graph, struct stream_script *script)
{
//    uint8_t raw_type;
//    uint32_t nb_raw, nbytes, nbits;
//
//    while (1)
//    {
//        read_binary_param(pt_line, &(script->script_program[script->script_nb_instruction]), &raw_type, &nb_raw);
//        if (nb_raw == 0)
//            break;
//        nbits = stream_bitsize_of_raw(raw_type);
//        nbytes = (nbits * nb_raw)/8;
//        script->script_nb_instruction += nbytes;
//    }
//
//    script->script_nb_instruction = (3+(script->script_nb_instruction)) & 0xFFFFFFFC;   // round it to W32
//
}

/* --------------------------- instruction coding ----------------------------*/
#define CONDF 0
#define INSTF 1
#define DSTF  2
#define OPCODEF 3
#define SRC1F 4
#define SRC2F 5
#define KF    6

// COND
#define M_IF "if"           // if yes / no
// INST
#define M_TEST "te"         // test
#define M_LD "ld"
#define M_JUMP "ju"
#define M_CALL "ca"
#define M_LABEL "la"
#define M_RETURN "re"
// REGS
#define M_REG "r"
#define M_STACK "sp"

#define cASM 32         // characters per symbol/instructions
#define cNFIELDS 7   



/* ==================================================================================== */
void dst_srcx_register (uint32_t *INST, char *s, uint32_t msb, uint32_t lsb, uint32_t not_sp1)             
{  
    /*  is it a register ?  => update INST and return -----------------------------*/
    if (0 == strncmp(s, RN, 2))     // none
    {   INSERT_BITS(INST[0], msb, lsb, RegNone);
    }

    if (0 == strncmp(s, "s", 1))    // DST = S0 or S1 
    {   if (0 == strchr(s,'1'))     // if '1' not found
        {   INSERT_BITS(INST[0], msb, lsb, RegSP0);         // DST = S0   
        }   else
        {   if (not_sp1)
            {   fprintf(stderr, "SP1 cannot be used on instruction %d !", INST[0]); exit(-4);
            } 
            else
            {   INSERT_BITS(INST[0], msb, lsb, RegSP1);      // DST = S1
            }
        }
        return;
    }

    if (0 == strncmp(s, "r", 1))    // DST = Rm
    {   int i = atoi(&(s[1])); 
        INSERT_BITS(INST[0], msb, lsb, i); // read R.. 
        return;
    } 
}


/* ==================================================================================== */
void dtype_register (uint32_t *INST, char S[cNFIELDS][cASM], int offset, int32_t *dtype)             
{   int idx, type;

    idx = offset;
    type = DTYPE_INT32; /*  it a constant w/wo #type  : SRC2=RK and check if we need 2 words */

    /* s[0]='#' => read one more field for the constant */
    if (S[idx][0] == '#')
    {   if ((0 == strcmp(S[idx], "#ptruint8"))  || (0 == strcmp(S[idx], "#uint8")) ) { type = DTYPE_UINT8 ; }
        if ((0 == strcmp(S[idx], "#ptruint16")) || (0 == strcmp(S[idx], "#uint16"))) { type = DTYPE_UINT16; }
        if ((0 == strcmp(S[idx], "#ptrint16"))  || (0 == strcmp(S[idx], "#int16")) ) { type = DTYPE_INT16 ; }
        if ((0 == strcmp(S[idx], "#ptruint32")) || (0 == strcmp(S[idx], "#uint32"))) { type = DTYPE_UINT32; }
        if ((0 == strcmp(S[idx], "#ptrint32"))  || (0 == strcmp(S[idx], "#int32")) ) { type = DTYPE_INT32 ; }
        //if ((0 == strcmp(S[idx], "#ptrint64"))  || (0 == strcmp(S[idx], "#int64")) ) { type = DTYPE_INT64 ; }
        if ((0 == strcmp(S[idx], "#ptrfp16"))   || (0 == strcmp(S[idx], "#fp16"))  ) { type = DTYPE_FP16  ; }
        if ((0 == strcmp(S[idx], "#ptrfloat"))  || (0 == strcmp(S[idx], "#float")) ) { type = DTYPE_FP32  ; }
        //if ((0 == strcmp(S[idx], "#ptrdouble")) || (0 == strcmp(S[idx], "#double"))) { type = DTYPE_FP64  ; }
        if ( 0 == strcmp(S[idx], "#time16")  ) { type = DTYPE_TIME16; }
        if ( 0 == strcmp(S[idx], "#time32")  ) { type = DTYPE_TIME32; }
        if ( 0 == strcmp(S[idx], "#time64")  ) { type = DTYPE_TIME64; }
        if ( 0 == strcmp(S[idx], "#ptr")     ) { type = DTYPE_PTR28B; }
        idx ++;   /* switch to next field as a constant */
    }

    if (dtype) { *dtype = type; }
}

/* ==================================================================================== */
void K_register (uint32_t *INST, char S[cNFIELDS][cASM], int offset, uint32_t msb, uint32_t lsb, uint32_t not_sp1)             
{   int idx, type, tmp;
    int32_t iK;
    union floatb {
        uint32_t i;
        float f;
    } fK;
    int64_t llK;

    idx = offset;
    type = DTYPE_INT32; /*  it a constant w/wo #type  : SRC2=RK and check if we need 2 words */

    /* s[0]='#' => read one more field for the constant */
    if (S[idx][0] == '#')
    {   if ((0 == strcmp(S[idx], "#ptruint8"))  || (0 == strcmp(S[idx], "#uint8")) ) { type = DTYPE_UINT8 ; }
        if ((0 == strcmp(S[idx], "#ptruint16")) || (0 == strcmp(S[idx], "#uint16"))) { type = DTYPE_UINT16; }
        if ((0 == strcmp(S[idx], "#ptrint16"))  || (0 == strcmp(S[idx], "#int16")) ) { type = DTYPE_INT16 ; }
        if ((0 == strcmp(S[idx], "#ptruint32")) || (0 == strcmp(S[idx], "#uint32"))) { type = DTYPE_UINT32; }
        if ((0 == strcmp(S[idx], "#ptrint32"))  || (0 == strcmp(S[idx], "#int32")) ) { type = DTYPE_INT32 ; }
        //if ((0 == strcmp(S[idx], "#ptrint64"))  || (0 == strcmp(S[idx], "#int64")) ) { type = DTYPE_INT64 ; }
        if ((0 == strcmp(S[idx], "#ptrfp16"))   || (0 == strcmp(S[idx], "#fp16"))  ) { type = DTYPE_FP16  ; }
        if ((0 == strcmp(S[idx], "#ptrfloat"))  || (0 == strcmp(S[idx], "#float")) ) { type = DTYPE_FP32  ; }
        //if ((0 == strcmp(S[idx], "#ptrdouble")) || (0 == strcmp(S[idx], "#double"))) { type = DTYPE_FP64  ; }
        if ( 0 == strcmp(S[idx], "#time16")  ) { type = DTYPE_TIME16; }
        if ( 0 == strcmp(S[idx], "#time32")  ) { type = DTYPE_TIME32; }
        if ( 0 == strcmp(S[idx], "#time64")  ) { type = DTYPE_TIME64; }
        if ( 0 == strcmp(S[idx], "#ptr")     ) { type = DTYPE_PTR28B; }
        idx ++;   /* switch to next field as a constant */
    }
   
    /* default = long constant */
    ST(INST[0], SRC2LONGK_PATTERN_INST, 0);     /* 0 = decoded pattern for SRC2 / long_K */ 
    ST(INST[0], OP_K_INST, type + UNSIGNED_K_OFFSET); /* DTYPE coded on 4 bits */
    ST(INST[0], OP_RKEXT_INST, 1);              /* default = extended constant */

    switch (type)
    {
    default:
    case DTYPE_PTR28B : /* TODO */
    case DTYPE_TIME16 : 
    case DTYPE_TIME32 : 
    case DTYPE_UINT8  : 
    case DTYPE_UINT16 : 
    case DTYPE_INT16  : 
    case DTYPE_UINT32 : 
    case DTYPE_INT32  : if (0 ==  strstr(S[idx], "0x"))
                        {   tmp = sscanf (S[idx],"%d", &iK); 
                        } else
                        {   tmp = sscanf (S[idx],"%x", &iK); 
                        }
                        if (iK <= MAX_LITTLE_K && iK >= MIN_LITTLE_K) 
                        {   
                            /* small constant coded on K12
                                2047 coded as 2047+2048 = 4095 = FFF 
                                  -1 coded as -1+2048   = 8191 = 7FF 
                               -2016 coded as -2016+2048=   32 = 020
                               Interpreter does K = unsignedK12-2048
                             */
                            ST(INST[0], OP_K_INST, iK + UNSIGNED_K_OFFSET);   
                        }
                        else
                        {   INST[1] = iK;  
                            INST [INST_WORDS-1] = 2;                /* two words */
                        }
                        break;
    //case DTYPE_FP64   :
    case DTYPE_FP32   : 
    case DTYPE_FP16   : tmp = sscanf (S[idx],"%f", &(fK.f));    /* double converted to float */
                        INST[1] = fK.i;  
                        INST [INST_WORDS-1] = 2;                /* two words */
                        break;

    //case DTYPE_INT64  : 
    case DTYPE_TIME64 : tmp = sscanf (S[idx],"%lld", &llK); 
                        INST[1] = (int)((int64_t)0xFFFFFFFF & llK);  
                        INST[2] = (int)((int64_t)0xFFFFFFFF & (llK >> 32));  
                        INST [INST_WORDS-1] = 3;                /* three words */
                        break;
    }
}

/* ==================================================================================== */
void check_JMOV_opar (char *s0, char *s2, int *oparf)
{
    *oparf = OPLJ_NONE;
    if ((0 != strstr(s0, "set")) && (0 == strstr(s2, "ptr")))  { *oparf = OPLJ_CAST    ; } 
    if ((0 != strstr(s0, "set")) && (0 != strstr(s2, "ptr")))  { *oparf = OPLJ_CASTPTR ; } 
    if ((0 != strstr(s0, "set")) && (0 != strstr(s2, "base"))) { *oparf = OPLJ_BASE    ; } 
    if ((0 != strstr(s0, "set")) && (0 != strstr(s2, "size"))) { *oparf = OPLJ_SIZE    ; } 
    if ((0 != strstr(s0, "set")) && (0 != strstr(s2, "param"))){ *oparf = OPLJ_PARAM   ; } 
    if ((0 != strstr(s0, "set")) && (0 != strstr(s2, "heap"))) { *oparf = OPLJ_HEAP    ; } 
    if ((0 != strstr(s0, "set")) && (0 != strstr(s2, "graph"))){ *oparf = OPLJ_GRAPH   ; } 
    if ((0 != strstr(s0, "swap")))   { *oparf = OPLJ_SWAP; } 
    if ((0 != strstr(s0, "del")))    { *oparf = OPLJ_DELETE; } 
    if ((0 != strstr(s0, "jump")))   { *oparf = OPLJ_JUMP; } 
    if ((0 != strstr(s0, "banz")))   { *oparf = OPLJ_BANZ; } 
    if ((0 != strstr(s0, "call")))   { *oparf = OPLJ_CALL; } 
    if ((0 != strstr(s0,"callsys"))) { *oparf = OPLJ_CALLSYS; } 
    if ((0 != strstr(s0, "save")))   { *oparf = OPLJ_SAVE; } 
    if ((0 != strstr(s0, "rest")))   { *oparf = OPLJ_RESTORE; } 
    if ((0 != strstr(s0, "ret")))    { *oparf = OPLJ_RETURN; } 

    /* missing : OPLJ_SCATTER  OPLJ_GATHER  OPLJ_WR2BF  OPLJ_RDBF */
}

//char s[cNFIELDS][cASM]
/* ==================================================================================== */
void check_alu_opar (char *s, int *oparf, int *opar0reg1RKm1)
{
#define opar_K (-1) 
#define opar_ALU (0) 
#define opar_reg (1) 

    *oparf = OPAR_NOP;
    *opar0reg1RKm1 = opar_K;   /* "else"  # or number */
    if (0 == strncmp(s, "sp", 2)) { *opar0reg1RKm1 = opar_reg; }   /* is it a register ? */
    if (0 == strncmp(s, "r",  1)) { *opar0reg1RKm1 = opar_reg; }   /* SPx / Rx */
    *oparf = OPAR_NOP;

    if (0 != strstr(s, "ret"))    { *oparf = OPAR_NOP ;   *opar0reg1RKm1 = opar_ALU; }  /* confusion with registers */

    if (0 != strstr(s, "nop"))    { *oparf = OPAR_NOP ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "add"))    { *oparf = OPAR_ADD ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "sub"))    { *oparf = OPAR_SUB ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "mul"))    { *oparf = OPAR_MUL ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "div"))    { *oparf = OPAR_DIV ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "or"))     { *oparf = OPAR_OR  ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "nor"))    { *oparf = OPAR_NOR ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "and"))    { *oparf = OPAR_AND ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "xor"))    { *oparf = OPAR_XOR ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "shr"))    { *oparf = OPAR_SHR ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "shl"))    { *oparf = OPAR_SHL ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "set"))    { *oparf = OPAR_SET ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "clr"))    { *oparf = OPAR_CLR ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "max"))    { *oparf = OPAR_MAX ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "min"))    { *oparf = OPAR_MIN ;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "amax"))   { *oparf = OPAR_AMAX;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "amin"))   { *oparf = OPAR_AMIN;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "norm"))   { *oparf = OPAR_NORM;   *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "addmod")) { *oparf = OPAR_ADDMOD; *opar0reg1RKm1 = opar_ALU; } 
    if (0 != strstr(s, "submod")) { *oparf = OPAR_SUBMOD; *opar0reg1RKm1 = opar_ALU; } 
}


#define NOT_SP1 1
/* ==================================================================================== */
void K_SRC2_register(uint32_t *INST, char s[cNFIELDS][cASM], int offset, uint32_t msb, uint32_t lsb, uint32_t not_sp1)     
{
    int oparf, opar0src1orKm1;

    check_alu_opar (s[offset], &oparf, &opar0src1orKm1);
    if (opar0src1orKm1 == opar_reg)  
    {   dst_srcx_register (INST, s[offset], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, not_sp1); 
    }
    else
    {   K_register (INST, s, offset, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, not_sp1); 
    }    
}

/* ==================================================================================== */
void clean_line (char **pt_line, 
                    uint8_t *thereIsHash, uint8_t *thereAreBrackets, uint8_t *thereAreVerticals,
                    char *comments)
{   char *pch, test;
    char current_line[NBCHAR_LINE];
    int line_length;
        
    pch = strchr(*pt_line,'\n');
    line_length = (int)(pch - *pt_line);        
    strncpy(current_line, *pt_line, line_length);
    current_line[line_length] = '\0';               // forced end of line
    *thereIsHash = *thereAreBrackets = *thereAreVerticals = 0; strcpy (comments, "");

    pch = strchr(current_line,';');
    if (0 != pch)   // search ';' copy comments 
    {   strncpy(comments, pch, line_length);
        pch[0] = '\0';
        pch[1] = '\n';
    }

    /* -------------------- CLEAN THE LINE FROM { } \ /  and detect '#' '[' ----- */
    *thereIsHash = (0 != strchr(current_line,'#'));   
    *thereAreBrackets = (0 != strchr(current_line,'['));   
    *thereAreVerticals = (0 != strchr(current_line,'|'));   

    test = '{'; pch = strchr(current_line,test); 
    while (pch != NULL) 
    { *pch = ' ';  
     pch = strchr(current_line, test); 
    }
    test = '}'; pch = strchr(current_line,test); while (pch != NULL) { *pch = ' ';  pch = strchr(current_line, test); }
    test = '\\';pch = strchr(current_line,test); while (pch != NULL) { *pch = ' ';  pch = strchr(current_line, test); }
    test = '/'; pch = strchr(current_line,test); while (pch != NULL) { *pch = ' ';  pch = strchr(current_line, test); }

    strncpy(*pt_line, current_line, line_length);
}


/* ==================================================================================== */
void read_para_heap_labels (char **pt_line, struct stream_script *script,
                            uint32_t *idx_l, labelPos_t *Label_positions,
                            uint32_t *total_nbytes, uint32_t Param1Heap2)

{   uint8_t raw_type, *ptr_param, *pt0;
    uint32_t nb_raw, nbits, tmp, idx_label, nbytes;
    char *ptstart, *ptend, inputchar[200];
#define MAX_HEAP_BUFFER 10000
    char dummy_buffer[MAX_HEAP_BUFFER];
    char LabelName[NBCHAR_STREAM_NAME];

    idx_label = *idx_l;

    /* parameter data follows the code */
    if (Param1Heap2 == 1) 
    {   pt0 = ptr_param = (uint8_t *)&(script->script_program[script->script_nb_instruction]);
    }
    else
    {   pt0 = ptr_param = dummy_buffer;
    }
    
    jump2next_valid_line(pt_line);
    nbytes = 0;

    /* same code as stream_tool_read_parameters () but with Label checks */
    

    while (1)
    {   char *Label, c[10], *p; 
        int i;

        /* read the header of the line : number of fields and type */
        ptstart = *pt_line;   
        ptend = strchr(ptstart, '\n');
        i = ptend - ptstart;
        strncpy(inputchar, ptstart, (int)i); inputchar[i] = '\0'; inputchar[i+1] = '\n';
        
        Label = strstr(inputchar, script_label);

        if (Label)
        {   tmp = sscanf(Label, "%s %s", c, LabelName); LabelName[NBCHAR_STREAM_NAME-1] = '\0';
            strcpy(Label_positions[idx_label].symbol, LabelName);  

            Label_positions[idx_label].offset = (int)(ptr_param - pt0);
            if (Param1Heap2 == 1)
            {   Label_positions[idx_label].label_type = LABEL_PARAM_DECLARE;
            } else
            {   Label_positions[idx_label].label_type = LABEL_HEAP_DECLARE;
            }

            idx_label++;
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
        } 

        read_binary_param(pt_line, ptr_param, &raw_type, &nb_raw);
        if (nb_raw == 0)
            break;

        nbits = stream_bitsize_of_raw(raw_type);
        nbytes = (nbits * nb_raw)/8;
        ptr_param = &(ptr_param[nbytes]);               // increment ptr_param write pointer

        if (0 == strncmp (*pt_line,script_parameters,strlen(script_parameters)) ||
            0 == strncmp (*pt_line,script_heap,strlen(script_heap)) || 
            0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)) )
        {   break;
        }
    }

    *idx_l = idx_label;
    *total_nbytes = (int)(ptr_param - pt0); 
}
/* ====================================================================================   
    Read and pack the macro assembler
        script_byte_code
        ...
        end               end of byte codes  
*/
//typedef union
//{   char    c;       char v_c[8];
//    int8_t  i8;    int8_t v_i8[8];
//    int16_t i16;  int16_t v_i16[4];
//    int32_t i32;  int32_t v_i32[2];
//    int64_t i64;
//    sfloat  f32;   sfloat v_f32[2]; 
//    #define REGS_DATA 0
//    #define REGS_TYPE 1
//    sdouble f64;
//} regdata_t;


void stream_tool_read_code(char **pt_line, struct stream_platform_manifest *platform,
                            struct stream_graph_linkedlist *graph, 
                            struct stream_script *script)
{
    char s[cNFIELDS][cASM], *pdbg, dbg;
    uint32_t INST[INST_WORDS], nWord;
    uint8_t thereIsHash, thereAreBrackets, thereAreVerticals;
    char script_comment[NBCHAR_LINE];
    char LabelName[NBCHAR_STREAM_NAME];
    uint32_t oparf, opar0src1orKm1, dtype, tmp, Param1Heap2, nbytes;

    jump2next_valid_line(pt_line);                  // remove   "script_code"
    Param1Heap2 = script->idx_label = 0;
    strcpy(LabelName, "");
    memset(&(script->Label_positions[0]), 0, sizeof(script->Label_positions));

    while (1)
    {
        INST[0] = 0;
        INST[1] = INST[2] = 0; 
        INST[INST_WORDS-1] = 1;                     // one word = one instruction 
        nWord = 1; 

        /* remove { } \ / =  */
        clean_line (pt_line, &thereIsHash, &thereAreBrackets, &thereAreVerticals, script_comment);

        /* -------------------- SEPARATE THE LINE IN 7 FIELDS --------------------- */
        fields_extract(pt_line, "CCCCCCC", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[cNFIELDS-1]); 


        /* -------------------- label N ---------------------label L_symbol      no code-------- 
          s[0]      1     
          Label   L_symbol
         */
        if (0 != strstr(s[0],script_label))  
        {   // Save the instruction offset and the Symbol
            tmp = sscanf(s[1], "%s", LabelName);
            script->Label_positions[script->idx_label].offset = script->script_nb_instruction;
            script->Label_positions[script->idx_label].label_type = LABEL_CODE_DECLARE;
            pdbg = strncpy((char *)&(script->Label_positions[script->idx_label].symbol[0]), LabelName, NBCHAR_STREAM_NAME);
            script->idx_label++;
            INST[INST_WORDS-1] = 0;
            continue;
        } 
            
        /* ----------------------------------------- CONDITIONAL FIELD ------------------------ */
        if (0 == strncmp(s[0], M_IF, 2))        
        {   int i;

            if (0 == strchr(s[0],'y')) { ST(INST[0], OP_COND_INST, IF_NOT); }   // strchr=0 'y' not found => "if not"
            else                       { ST(INST[0], OP_COND_INST, IF_YES); }   // "if_yes"

            for (i = 0; i < cNFIELDS-1; i++)      // all the instruction fields are left-shifted
            {   strcpy (s[i], s[i+1]);            //  so next field starts on s[0]
            }
        }

        /* -------------------------------------------- TEST --------------------------------------
          s[0]      1          2         3        4                        
          test<xx> <DST reg> <ALU>    <SRC1 reg> <SRC2 reg>          test_leq r2 add r4 r1
          test<xx> <DST reg> <ALU>    <SRC1 reg> <number>            test_leq r2 add r4 #int 3
          test<xx> <DST reg> <SRC2 reg>                              test_leq r2     r4
          test<xx> <DST reg> <number>                                test_leq r2     #float 3
        */
        if (0 == strncmp(s[0], M_TEST, 2))
        { 
            if (0 != strstr(s[0], "equ")) { ST(INST[0], OP_INST, OP_TESTEQU); } 
            if (0 != strstr(s[0], "leq")) { ST(INST[0], OP_INST, OP_TESTLEQ); } 
            if (0 != strstr(s[0], "lt"))  { ST(INST[0], OP_INST, OP_TESTLT ); } 
            if (0 != strstr(s[0], "neq")) { ST(INST[0], OP_INST, OP_TESTNEQ); } 
            if (0 != strstr(s[0], "geq")) { ST(INST[0], OP_INST, OP_TESTGEQ); } 
            if (0 != strstr(s[0], "gt"))  { ST(INST[0], OP_INST, OP_TESTGT ); } 

            dst_srcx_register (INST, s[1],  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0);     /* register to compare to */

            check_alu_opar (s[2], &oparf, &opar0src1orKm1);                         /* is it an operator ? */  
            ST(INST[0], OP_OPAR_INST, OPAR_NOP);                                    /* default operator = NOP */
            
            if (opar0src1orKm1 == opar_ALU)                                                /* operator ?  */
            {   ST(INST[0], OP_OPAR_INST, oparf);                                   /* next is SRC1  +SRC2/K */
                dst_srcx_register(INST, s[3], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0);  
                K_SRC2_register (INST, s, 4, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
            }
            else
            {   K_SRC2_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
            }
        }
        else
        /* ----------------------------------------- OP_LD ------------------------------------------------ */
        { 
            /* --- LD ---                                            r1 = <op> r2 r3/type K
            s[0]        1       2          3          4        
            <DST reg>  '='   <SRC2>
            <DST reg>  '='   <number>
            <DST reg>  '='   <ALU>        <SRC1>    <SRC2> 
            <DST reg>  '='   <ALU>        <SRC1>    <number> 

            check if s[0] is register, constant, or either set setptr swap del jump banz call callsys save rest ret */
            check_alu_opar (s[0], &oparf, &opar0src1orKm1);                         /* does s[0] is a Register */
            ST(INST[0], OP_OPAR_INST, oparf);                                       /* fill OPAR_INST */
                

            if (opar0src1orKm1 == opar_reg && thereAreBrackets == 0 && thereAreVerticals == 0)   /* without | and [] */
            {
                ST(INST[0], OP_INST, OP_LD);                                        /* OP_LD instruction family */
                dst_srcx_register(INST,s[0], OP_DST_INST_MSB,  OP_DST_INST_LSB, 0);    /* DST register to load */

                /* --- LD ---  r1 = op r2 r3/type K
                s[0]        1       2          3          4        
                <DST reg>  '='   <SRC2>
                <DST reg>  '='   <number>
                <DST reg>  '='   <ALU>        <SRC1>    <SRC2> 
                <DST reg>  '='   <ALU>        <SRC1>    <number>  */
                {
                    check_alu_opar (s[2], &oparf, &opar0src1orKm1);                     /* does s[2] is a Register or ALU */
                    if (opar0src1orKm1 == opar_ALU)
                    {   ST(INST[0], OP_OPAR_INST, oparf);                               /* LD DST ALU SRC1 + SRC2/K */
                        dst_srcx_register(INST, s[3], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0); 
                        K_SRC2_register (INST, s, 4, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0);           
                    }  
                    else
                    {   K_SRC2_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); // r1 = src2/K
                    }
                }
            }
            else
            {
            /* ----------------------------------------- JMOV ------------------------------------------------- */
            
            /* --- JMOV --- 
           
            s[0]        1           2             3           4         5
             return 
             save       <register> <register> <register> <register> <register>
             restore    <register> <register> <register> <register> <register>
             jump       <Label>    <register> <register> <register> 
             banz       <Label>    <register> <register> <register> 
             call       <Label>    <register> <register> <register> 
             callsys    K          <register> <register> <register> 
            
             set    <register>  <type/typeptr>    #type                  OPLJ_CASTPTR / OPLJ_CAST
             set    <register>  <base/size>       <register/number>      OPLJ_BASE   / OPLJ_SIZE
             set    <register>  <heap/param/graph> <register/number>     OPLJ_HEAP   / OPLJ_SIZE
            */      
            ST(INST[0], OP_INST, OP_JMOV);
            dst_srcx_register(INST, s[1], OP_DST_INST_MSB,  OP_DST_INST_LSB, 0); 

            /* check it is either set setptr swap del jump banz call callsys save rest ret */
            check_JMOV_opar (s[0], s[2], &oparf);     
            ST(INST[0], OP_OPAR_INST, oparf); 

            if (OPLJ_SWAP == oparf)
            {   dst_srcx_register(INST, s[2], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, NOT_SP1); 
            }
            if (OPLJ_RETURN == oparf) 
            {   ST(INST[0], OP_K_INST, 0 + UNSIGNED_K_OFFSET);  
            }
            if (OPLJ_DELETE == oparf)
            {   K_SRC2_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, NOT_SP1); 
            }
            if (OPLJ_CAST    == oparf)
            {   dtype_register (INST, s, 3, &dtype); 
                ST(INST[0], OP_K_INST, dtype + UNSIGNED_K_OFFSET);
            }
            if (OPLJ_CASTPTR == oparf)
            {   dtype_register (INST, s, 3, &dtype); 
                ST(INST[0], OP_K_INST, dtype + UNSIGNED_K_OFFSET);
            }
            if (OPLJ_BASE == oparf)
            {   K_SRC2_register (INST, s, 3, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
            }
            if (OPLJ_SIZE    == oparf)
            {   K_SRC2_register (INST, s, 3, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
            }

            // s[0]    1        2                3
            // set register <heap/param/graph>  <label>  
            // OP_JMOV OPLJ_PARAM / OPLJ_HEAP / OPLJ_GRAPH : 2 words (no risk of overflow)
            // IIyyy-OPARDST_______00000001_INT 
            // <--------------Long Offset----->

            if (OPLJ_PARAM == oparf || OPLJ_HEAP == oparf || OPLJ_GRAPH == oparf)
            {   // Save the instruction offset and the Symbol
                script->Label_positions[script->idx_label].offset = 
                    script->script_nb_instruction + 1;      // points to the extra w32 holding the address
                INST [INST_WORDS-1] = 2;
                if (OPLJ_GRAPH == oparf)
                {   script->Label_positions[script->idx_label].label_type = LABEL_GRAPH_USE;
                } 
                if (OPLJ_HEAP == oparf)
                {   script->Label_positions[script->idx_label].label_type = LABEL_HEAP_USE;
                }
                if (OPLJ_PARAM == oparf)
                {   script->Label_positions[script->idx_label].label_type = LABEL_PARAM_USE;
                }
                ST(INST[0], OP_K_INST, DTYPE_INT32);        // DTYPE and clears SRC2LONGK_PATTERN_INST
                ST(INST[0], OP_RKEXT_INST, 1);              // 2 words
                INST[1] = 0x12345678;                       // to fill at LINK time

                dbg = sscanf(s[3], "%s", LabelName);
                strcpy(script->Label_positions[script->idx_label].symbol, LabelName);
                script->idx_label++;
            }

            // s[0]        1           2          3          4          5 
            //  jump       <Label>    <dst>      <src1>      
            //  banz       <Label>    <dst>      <src1>      
            //  call       <Label>    <dst>      <src1>      
            //  callsys    K          <dst>      <src1>     <src3>     <src4>  
            if (OPLJ_JUMP == oparf || OPLJ_BANZ == oparf || OPLJ_CALL == oparf || OPLJ_CALLSYS == oparf)
            {   
                if ('\n' == s[2][0] || '\0' == s[2][0]) { strcpy(s[2], RN); }
                if ('\n' == s[3][0] || '\0' == s[3][0]) { strcpy(s[3], RN); }
                if ('\n' == s[4][0] || '\0' == s[4][0]) { strcpy(s[4], RN); }
                if ('\n' == s[5][0] || '\0' == s[5][0]) { strcpy(s[5], RN); }
                dst_srcx_register (INST, s[2], OP_DST_INST_MSB,  OP_DST_INST_LSB , NOT_SP1);
                dst_srcx_register (INST, s[3], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, NOT_SP1);
                dst_srcx_register (INST, s[4], OP_SRC3_INST_MSB, OP_SRC3_INST_LSB, NOT_SP1);
                dst_srcx_register (INST, s[5], OP_SRC4_INST_MSB, OP_SRC4_INST_LSB, NOT_SP1);

                if (OPLJ_CALLSYS == oparf)
                {   int service;
                    tmp = sscanf(s[1], "%d", &service);         /* 6bits service */
                    ST(INST[0], CALLSYS_K_INST, service);
                }
                else
                {
                    // Save the instruction offset and the Symbol
                    script->Label_positions[script->idx_label].offset = script->script_nb_instruction;
                    script->Label_positions[script->idx_label].label_type = LABEL_CODE_USE;
                    tmp = sscanf(s[1], "%s", LabelName);
                    strcpy(script->Label_positions[script->idx_label].symbol, LabelName);
                    script->idx_label++;
                }
            }
            
            //s[0]        1           2         3            4         5
            // save       <register> <register> <register> <register> <register>
            // restore    <register> <register> <register> <register> <register>
            if ((OPLJ_SAVE    == oparf) || (OPLJ_RESTORE == oparf))
            {   if ('\n' == s[1][0]) { strcpy(s[4], RN); }
                if ('\n' == s[2][0]) { strcpy(s[2], RN); }
                if ('\n' == s[3][0]) { strcpy(s[3], RN); }
                if ('\n' == s[4][0]) { strcpy(s[4], RN); }
                if ('\n' == s[5][0]) { strcpy(s[4], RN); }
                dst_srcx_register (INST, s[1], OP_DST_INST_MSB,  OP_DST_INST_LSB , NOT_SP1);
                dst_srcx_register (INST, s[2], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, NOT_SP1);
                dst_srcx_register (INST, s[3], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, NOT_SP1);
                dst_srcx_register (INST, s[4], OP_SRC3_INST_MSB, OP_SRC3_INST_LSB, NOT_SP1);
                dst_srcx_register (INST, s[5], OP_SRC4_INST_MSB, OP_SRC4_INST_LSB, NOT_SP1);
            }

            /*--------------------------------------------------------------------------------------------------------*/
            if (thereAreBrackets) 
            {   
                // SCATTER  pre-inc [ r K ]+ = r   post-inc  [ r ]+ K = r
                // s[0]  1     2    3    4    5
                //1  [   reg   ]    =    reg          [r] = r                   OPLJ_SCATTER null-post-increment 
                //1  [   reg   ]    K    =    reg     post-inc [ r ]+ K = r     OPLJ_SCATTER post-increment
                //1  [   reg   K    ]    =    reg     pre-inc [ r K ]+ = r      OPLJ_SCATTER pre-increment OP_EXT0_INST-1

                dst_srcx_register (INST, s[4],  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0); 
                if (s[0][0] == '[')
                {   ST(INST[0], OP_OPAR_INST, OPLJ_SCATTER);
                    dst_srcx_register (INST, s[1],  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0);     // destination [r]
                    if (s[3][0] == '=')                                                     
                    {   ST(INST[0], OP_K_INST, 0 + UNSIGNED_K_OFFSET);                      //1 case [DST + 0] = SRC1
                        dst_srcx_register (INST, s[4], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0); 
                    }
                    else
                    {   dst_srcx_register (INST, s[5], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0); 
                        if (s[2][0] == ']')                                                 //2 case post-inc [ r ]+ K = r
                        {   K_SRC2_register (INST, s, 3, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
                        }
                        else                                                                //3 case pre-inc [ r K ]+ = r 
                        {   K_SRC2_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
                            ST(INST[0], OP_EXT0_INST, PRE_INCREMENT);
                        }
                        if (s[2][1] == '+' || s[3][1] == '+')
                        {   ST(INST[0], OP_EXT1_INST, INDEX_UPDATE); 
                        }
                    }                
                }
                else
                // GATHER  pre-inc r = [r K]+   post-inc  r = [r]+ K
                // s[0]  1  2  3    4      5
                // reg   =  [ reg   ]            r = [r]
                // reg   =  [ reg   ](+)   K     post-inc r = [r]+ K
                // reg   =  [ reg   K      ]     pre-inc r = [r K]+ 
                {   
                    dst_srcx_register (INST, s[0],  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0);     // destination [r]

                    ST(INST[0], OP_OPAR_INST, OPLJ_GATHER);
                    dst_srcx_register (INST, s[3],  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0); 
                    if (s[5][0] == '\0')                                                     
                    {   ST(INST[0], OP_K_INST, 0 + UNSIGNED_K_OFFSET);                      //1 case DST = [SRC1]
                    }
                    else
                    {   if (s[5][0] != ']')                                                 //2 case post-inc r = [r]+ K
                        {   K_SRC2_register (INST, s, 5, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
                        }
                        else                                                                //3 case pre-inc r = [r K]+  
                        {   K_SRC2_register (INST, s, 4, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0); 
                            ST(INST[0], OP_EXT0_INST, PRE_INCREMENT);
                        }
                        if (s[5][1] == '+' || s[4][1] == '+')
                        {   ST(INST[0], OP_EXT1_INST, INDEX_UPDATE); 
                        }
                    }        
                }
            }
            /*--------------------------------------------------------------------------------------------------------*/
            if (thereAreVerticals)
            {   int lsb, msb; 
                //s[0]        1     2         3      4    5     6 
                //<register> '|'   lsb       msb   '|'  '='  <register>   OPLJ_WR2BF
                //<register> '=' <register>  '|'   lsb   msb   '|'         OPLJ_RDBF
                dst_srcx_register (INST, s[0],  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0); 
                if (s[1][0] == '|')
                {   if (s[4][0] != '|' || s[5][0] != '=')  
                    {   fprintf(stderr, " missing '|' or '='  !"); exit(-4);
                    }
                    tmp = sscanf(s[2], "%d", &lsb); tmp = sscanf(s[3], "%d", &msb);
                    ST(INST[0], BITFIELD_LSB_INST, lsb);
                    ST(INST[0], BITFIELD_MSB_INST, msb);
                    dst_srcx_register (INST, s[6],  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_WR2BF);
                }
                else
                {   if (s[3][0] != '|' || s[1][0] != '=')  
                    {   fprintf(stderr, " missing '|' or '='  !"); exit(-4);
                    }
                    tmp = sscanf(s[4], "%d", &lsb); tmp = sscanf(s[5], "%d", &msb);
                    ST(INST[0], BITFIELD_LSB_INST, lsb);
                    ST(INST[0], BITFIELD_MSB_INST, msb);
                    dst_srcx_register (INST, s[2],  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_RDBF);
                }
            }

            }   /* else not OP_LD */
        }


        // save byte-codes and corresponding comments        
        strcpy(script->script_comments[script->script_nb_instruction], script_comment);

        tmp = INST[INST_WORDS-1];
        memcpy (&(script->script_program[script->script_nb_instruction]), INST, tmp*4);
        script->script_nb_instruction += tmp;


        if (0 == strncmp (*pt_line,script_parameters,strlen(script_parameters)) ||
            0 == strncmp (*pt_line,script_heap,strlen(script_heap)) || 
            0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)) )
        {   break;
        }
    }       /* while (1) */

    /*--------------------------------------------------------------------------------------------------------*/

    /* here : either "end" or "param" or "heap" */
L_check_heap:
    if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
    {    jump2next_valid_line(pt_line);
    }
    if (0 == strncmp (*pt_line,script_parameters,strlen(script_parameters)) ||
        0 == strncmp (*pt_line,script_heap,strlen(script_heap))
       )    /* node_parameters / heap */
    {  
        if (0 == strncmp (*pt_line,script_parameters,strlen(script_parameters))) Param1Heap2 = 1;
        if (0 == strncmp (*pt_line,script_heap,strlen(script_heap))) Param1Heap2 = 2;

        read_para_heap_labels (pt_line, script, &(script->idx_label), script->Label_positions, &nbytes, Param1Heap2);
        
        if (Param1Heap2 == 1) 
        {   script->script_nb_instruction += (3 + nbytes) /4;   // parameters are after the code
        }   
        else
        {   if (nbytes > script->ram_heap_size)
            {   script->ram_heap_size = nbytes;
            }
        }

        while (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
        {   jump2next_valid_line(pt_line);
            break;
        }
        goto L_check_heap;
    }


    /*
        Second pass : find the L_symbol labels
                Label_positions[idx_label].position = script->script_nb_instruction;
                strcpy(Label_positions[idx_label].symbol, LabelName);
    */
    {   uint32_t ijump, ilabel;
        int label_position, instruction_position;
        uint32_t *instruction;

        /* replacement of Label of load jump call */
        for (ilabel = 0; ilabel < script->idx_label; ilabel++)
        {   
            if (script->Label_positions[ilabel].label_type != LABEL_CODE_DECLARE)
            {   continue; /* find Labels to place */
            }
            for (ijump = 0; ijump <script-> idx_label; ijump++)
            {   
                if (script->Label_positions[ijump].label_type == LABEL_CODE_USE)
                {   
                    // comparison of label name, then update the instruction 
                    if (0 == strcmp(script->Label_positions[ilabel].symbol, script->Label_positions[ijump].symbol))
                    {   label_position = script->Label_positions[ilabel].offset;
                        instruction_position = script->Label_positions[ijump].offset;
                        instruction = &(script->script_program[instruction_position]);
                        ST(*instruction, OP_K_INST, label_position - instruction_position + UNSIGNED_K_OFFSET);  
                    }
                }
            }
        }

        /* replacement of Label of set r label/heap , instruction using 2 words */
        for (ilabel = 0; ilabel < script->idx_label; ilabel++)
        {   
            if (script->Label_positions[ilabel].label_type == LABEL_HEAP_DECLARE  ||
                script->Label_positions[ilabel].label_type == LABEL_PARAM_DECLARE ) 
            {   
                /* label is LABEL_HEAP_DECLARE or LABEL_PARAM_DECLARE
                   find the instruction using it */
                for (ijump = 0; ijump < script->idx_label; ijump++)
                {   
                    if (script->Label_positions[ijump].label_type == LABEL_HEAP_USE ||
                        script->Label_positions[ijump].label_type == LABEL_PARAM_USE )
                    {   
                        if (0 == strcmp(script->Label_positions[ilabel].symbol, script->Label_positions[ijump].symbol))
                        {   label_position = script->Label_positions[ilabel].offset;
                            instruction_position = script->Label_positions[ijump].offset;
                            instruction = &(script->script_program[instruction_position]);
                            *instruction = label_position;  // 32bit address
                        }
                    }
                }
            }
        }
    }
}



#ifdef __cplusplus
}
#endif

