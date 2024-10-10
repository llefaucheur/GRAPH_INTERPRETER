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


#define INST_WORDS 4
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


    testlt r6 3	       	    ; test r6 < 3
    testlt r6 add r5 3	    ; test r6 < ( r5 + 3 )
    testlt r6 sub r5 r4	    ; test r6 < ( r5 - r4 )
    if_yes r6 = #float 3	; conditional load of r6 with 3.0
    r6 = 3	       			; r6 = 3  (the default litterals type is int32)
    r6 = add r5 3	        ; r6 = ( r5 + 3 )
    r6 = sub r5 r4	        ; r6 = ( r5 - r4 )
    set r2 type #float 
    set r2 typeptr #float 
    set r4 base r5 
    set r4 size r5 
    r2 [ r4 ] = r3 
    r2 = r3 [ r4 ] 
    r2 | lenK posK | = r3 
    r2 = r3 | lenK posK | 
    swap sp r3 
    pop r4 
    push r0 r3 r12 
    restore r0 r3 r12 
    jump r5 r1 
    banz L_replaced_Label r1 
    call L_replaced_Label r1 
    callsys 17 r1 r2 r3 
    return 

;end
;----------------------------------------------------------------------

*/


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
void dst_srcx_register (uint32_t *INST, char *s, uint32_t msb, uint32_t lsb)             
{  
    /*  is it a register ?  => update INST and return -----------------------------*/
    if (0 == strncmp(s, "s", 1))    // DST = S0 or S1 
    {   if (0 == strchr(s,'1'))
        {   INSERT_BITS(INST[0], msb, lsb, RegSP1);       // DST = S1
        }   else
        {   INSERT_BITS(INST[0], msb, lsb, RegSP0);       // DST = S0
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
        if ((0 == strcmp(S[idx], "#ptrint64"))  || (0 == strcmp(S[idx], "#int64")) ) { type = DTYPE_INT64 ; }
        if ((0 == strcmp(S[idx], "#ptrfp16"))   || (0 == strcmp(S[idx], "#fp16"))  ) { type = DTYPE_FP16  ; }
        if ((0 == strcmp(S[idx], "#ptrfloat"))  || (0 == strcmp(S[idx], "#float")) ) { type = DTYPE_FP32  ; }
        if ((0 == strcmp(S[idx], "#ptrdouble")) || (0 == strcmp(S[idx], "#double"))) { type = DTYPE_FP64  ; }
        if ( 0 == strcmp(S[idx], "#time16")  ) { type = DTYPE_TIME16; }
        if ( 0 == strcmp(S[idx], "#time32")  ) { type = DTYPE_TIME32; }
        if ( 0 == strcmp(S[idx], "#time64")  ) { type = DTYPE_TIME64; }
        if ( 0 == strcmp(S[idx], "#ptr")     ) { type = DTYPE_PTR28B; }
        idx ++;   /* switch to next field as a constant */
    }

    if (dtype) { *dtype = type; }
}

/* ==================================================================================== */
void K_register (uint32_t *INST, char S[cNFIELDS][cASM], int offset, uint32_t msb, uint32_t lsb)             
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
        if ((0 == strcmp(S[idx], "#ptrint64"))  || (0 == strcmp(S[idx], "#int64")) ) { type = DTYPE_INT64 ; }
        if ((0 == strcmp(S[idx], "#ptrfp16"))   || (0 == strcmp(S[idx], "#fp16"))  ) { type = DTYPE_FP16  ; }
        if ((0 == strcmp(S[idx], "#ptrfloat"))  || (0 == strcmp(S[idx], "#float")) ) { type = DTYPE_FP32  ; }
        if ((0 == strcmp(S[idx], "#ptrdouble")) || (0 == strcmp(S[idx], "#double"))) { type = DTYPE_FP64  ; }
        if ( 0 == strcmp(S[idx], "#time16")  ) { type = DTYPE_TIME16; }
        if ( 0 == strcmp(S[idx], "#time32")  ) { type = DTYPE_TIME32; }
        if ( 0 == strcmp(S[idx], "#time64")  ) { type = DTYPE_TIME64; }
        if ( 0 == strcmp(S[idx], "#ptr")     ) { type = DTYPE_PTR28B; }
        idx ++;   /* switch to next field as a constant */
    }
   
    /* default = long constant */
    ST(INST[0], SRC2LONGK_PATTERN_INST, 0);     /* 0 = decoded pattern for SRC2 / long_K */ 
    ST(INST[0], OP_K_DTYPE_INST, type);         /* DTYPE coded on 4 bits */

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
    case DTYPE_INT32  : tmp = sscanf (S[idx],"%d", &iK); 
                        if (iK <= MAX_LITTLE_K && iK >= MIN_LITTLE_K) 
                        {   
                            /* small constant coded on K14 
                                8191 coded as 8191+8192 = 16383 = 3FFF 
                                  -1 coded as -1+8192   =  8191 = 1FFF 
                               -8160 coded as -8160+8192=    32 = 0020
                               Interpreter does K = unsignedK14-8192
                             */
                            ST(INST[0], OP_K_INST, iK + UNSIGNED_K_OFFSET);   
                        }
                        else
                        {   INST[1] = iK;  
                            INST [INST_WORDS-1] = 2;                /* two words */
                        }
                        break;
    case DTYPE_FP16   : 
    case DTYPE_FP32   : 
    case DTYPE_FP64   : tmp = sscanf (S[idx],"%f", &(fK.f));    /* double converted to float */
                        INST[1] = fK.i;  
                        INST [INST_WORDS-1] = 2;                /* two words */
                        break;

    case DTYPE_TIME64 :
    case DTYPE_INT64  : tmp = sscanf (S[idx],"%lld", &llK); 
                        INST[1] = (int)((int64_t)0xFFFFFFFF & llK);  
                        INST[2] = (int)((int64_t)0xFFFFFFFF & (llK >> 32));  
                        INST [INST_WORDS-1] = 3;                /* three words */
                        break;
    }
}

/* ==================================================================================== */
void check_JMOV_opar (char *s, int *oparf)
{
    *oparf = OPLJ_NONE;
    if ((0 != strstr(s, "set")) && (0 == strstr(s, "ptr")))  { *oparf = OPLJ_CAST    ; } 
    if ((0 != strstr(s, "set")) && (0 != strstr(s, "ptr")))  { *oparf = OPLJ_CASTPTR ; } 
    if ((0 != strstr(s, "set")) && (0 != strstr(s, "base"))) { *oparf = OPLJ_BASE    ; } 
    if ((0 != strstr(s, "set")) && (0 != strstr(s, "size"))) { *oparf = OPLJ_SIZE    ; } 
    if ((0 != strstr(s, "swap")))   { *oparf = OPLJ_SWAP; } 
    if ((0 != strstr(s, "del")))    { *oparf = OPLJ_DELETE; } 
    if ((0 != strstr(s, "jump")))   { *oparf = OPLJ_JUMP; } 
    if ((0 != strstr(s, "banz")))   { *oparf = OPLJ_BANZ; } 
    if ((0 != strstr(s, "call")))   { *oparf = OPLJ_CALL; } 
    if ((0 != strstr(s,"callsys"))) { *oparf = OPLJ_CALLSYS; } 
    if ((0 != strstr(s, "save")))   { *oparf = OPLJ_SAVE; } 
    if ((0 != strstr(s, "rest")))   { *oparf = OPLJ_RESTORE; } 
    if ((0 != strstr(s, "ret")))    { *oparf = OPLJ_RETURN; } 

    /* missing : OPLJ_SCATTER  OPLJ_GATHER  OPLJ_WR2BF  OPLJ_RDBF */
}

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


/* ==================================================================================== */
void clean_line (char **pt_line, uint32_t *INST, 
                       uint8_t *thereIsHash, uint8_t *thereAreBrackets, uint8_t *thereAreVerticals,
                       char *comments, char s[cNFIELDS][cASM])
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
        memset(pch, '\n', line_length);
    }

    /* -------------------- CLEAN THE LINE FROM { } \ /  and detect '#' '[' ----- */
    *thereIsHash = (0 != strchr(current_line,'#'));   
    *thereAreBrackets = (0 != strchr(current_line,'['));   
    *thereAreVerticals = (0 != strchr(current_line,'|'));   

    test = '{'; pch = strchr(current_line,test); while (pch != NULL) { *pch = ' ';  pch = strchr(current_line, test); }
    test = '}'; pch = strchr(current_line,test); while (pch != NULL) { *pch = ' ';  pch = strchr(current_line, test); }
    test = '\\';pch = strchr(current_line,test); while (pch != NULL) { *pch = ' ';  pch = strchr(current_line, test); }
    test = '/'; pch = strchr(current_line,test); while (pch != NULL) { *pch = ' ';  pch = strchr(current_line, test); }

    strncpy(*pt_line, current_line, line_length);
}

/* ====================================================================================   
    Read and pack the macro assembler
        script_byte_code
        ...
        end               end of byte codes  
*/
typedef union
{   char    c;       char v_c[8];
    int8_t  i8;    int8_t v_i8[8];
    int16_t i16;  int16_t v_i16[4];
    int32_t i32;  int32_t v_i32[2];
    int64_t i64;
    sfloat  f32;   sfloat v_f32[2]; 
    #define REGS_DATA 0
    #define REGS_TYPE 1
    sdouble f64;
} regdata_t;


void stream_tool_read_code(char **pt_line, struct stream_platform_manifest *platform,
                            struct stream_graph_linkedlist *graph, 
                            struct stream_script *script)
{
    char s[cNFIELDS][cASM];
    uint32_t INST[INST_WORDS], nWord;
    uint8_t thereIsHash, thereAreBrackets, thereAreVerticals;
    char script_comment[NBCHAR_LINE];
    int idx_label;
    struct { int position; char symbol[NBCHAR_STREAM_NAME]; int Label0Jump1;} Label_positions[100];
    char LabelName[NBCHAR_STREAM_NAME];
    int oparf, opar0src1orKm1, dtype, tmp;

    jump2next_valid_line(pt_line);                  // remove   "script_code"
    idx_label = 0;

    while (1)
    {
        INST[0] = 0;
        INST[1] = INST[2] = 0; 
        INST[INST_WORDS-1] = 1;                     // one word = one instruction 
        nWord = 1; 

        /* remove { } \ / =  */
        clean_line (pt_line, &(INST[0]), &thereIsHash, &thereAreBrackets, &thereAreVerticals, script_comment, s);

        /* -------------------- SEPARATE THE LINE IN 7 FIELDS --------------------- */
        fields_extract(pt_line, "CCCCCCC", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[cNFIELDS-1]); 


        /* -------------------- label N ---------------------label L_symbol      no code-------- 
          s[0]      1     
          Label   L_symbol
         */
        if (0 != strstr(s[0],script_label))  
        {   // Save the instruction offset and the Symbol
            tmp = sscanf(s[1], "%s", LabelName);
            Label_positions[idx_label].position = script->script_nb_instruction;
            strcpy(Label_positions[idx_label].symbol, LabelName);
            Label_positions[idx_label].Label0Jump1 = 0;
            idx_label++;
            continue;
        } 
            
        /* ----------------------------------------- CONDITIONAL FIELD ------------------------ */
        if (0 == strncmp(s[0], M_IF, 2))        
        {   int i;

            if (0 == strchr(s[0],'y')) { ST(INST[0], OP_COND_INST, IF_NOT); }   // "if_yes"
            else                       { ST(INST[0], OP_COND_INST, IF_YES); }   // "if_not"

            for (i = cNFIELDS-1; i > 0; i--)      // all the instruction fields are left-shifted
            {   strcpy (s[i], s[i-1]);            //  so next field starts on s[0]
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

            dst_srcx_register (INST, s[1],  OP_DST_INST_MSB,  OP_DST_INST_LSB);     /* register to compare to */

            check_alu_opar (s[2], &oparf, &opar0src1orKm1);                         /* is it an operator ? */  
            ST(INST[0], OP_OPAR_INST, OPAR_NOP);                                    /* default operator = NOP */
            
            if (opar0src1orKm1 == opar_ALU)                                                /* operator ?  */
            {   ST(INST[0], OP_OPAR_INST, oparf);                                   /* next is SRC1  +SRC2/K */
                dst_srcx_register(INST, s[3], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB);  
                check_alu_opar (s[4], &oparf, &opar0src1orKm1);
                if (opar0src1orKm1 == opar_reg)  
                {   dst_srcx_register(INST, s[4], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); 
                } else
                {   K_register (INST, s, 4, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB);
                }
            }
            else
            {   check_alu_opar (s[2], &oparf, &opar0src1orKm1);                     /* no ALU : test<xx> <DST reg> <SRC2 reg> */
                if (opar0src1orKm1 == opar_reg)  
                {   dst_srcx_register(INST, s[2], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); /* reg SRC2 */
                } else
                {   K_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB);    /* constant */
                }
            }
        }
        else
        /* ----------------------------------------- OP_LD ------------------------------------------------ */
        { 
            /* --- LD ---                                                                 r1 = op r2 r3/type K
            s[0]        1       2          3          4        
            <DST reg>  '='   <SRC2>
            <DST reg>  '='   <number>
            <DST reg>  '='   <ALU>        <SRC1>    <SRC2> 
            <DST reg>  '='   <ALU>        <SRC1>    <number> 

            check if s[0] is register, constant, or either set setptr swap del jump banz call callsys save rest ret */
            check_alu_opar (s[0], &oparf, &opar0src1orKm1);                        /* does s[0] is a Register */

            if (opar0src1orKm1 == opar_reg && thereAreBrackets == 0 && thereAreVerticals == 0)   /* without | and [] */
            {
                ST(INST[0], OP_INST, OP_LD);                                        /* OP_LD instruction family */
                dst_srcx_register(INST,s[0], OP_DST_INST_MSB,  OP_DST_INST_LSB);    /* DST register to load */
                
                if (0 == strstr(s[1], "="))  
                {   fprintf(stderr, "LD missing '='  !"); exit(-4);
                } 
                
                check_alu_opar (s[2], &oparf, &opar0src1orKm1);                     /* does s[2] is a Register or ALU */
                if (opar0src1orKm1 == opar_reg)
                {   dst_srcx_register(INST, s[2], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); /* s[2] = SRC2 */
                }
                else if (opar0src1orKm1 == opar_K)
                {   K_register (INST, s, 2,  OP_SRC2_INST_MSB,  OP_SRC2_INST_LSB); /* s[2] = K */
                }
                else
                {   ST(INST[0], OP_OPAR_INST, oparf);                               /* LD DST ALU SRC1 + SRC2/K */
                    check_alu_opar (s[3], &oparf, &opar0src1orKm1);                
                    dst_srcx_register(INST, s[3], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB); 
                    check_alu_opar (s[4], &oparf, &opar0src1orKm1);

                    if (opar0src1orKm1 == opar_reg)  
                    {   dst_srcx_register(INST, s[4], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); /* reg SRC2 */
                    } else
                    {   K_register (INST, s, 4, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB);    /* constant */
                    }                
                }
            }
            else
            {
            /* ----------------------------------------- JMOV ------------------------------------------------- */
            
            /* --- JMOV --- 
           
            s[0]        1           2         3            4         5
            "return"
            "save"      <register> <register> <register> <register> <register>
            "restore"   <register> <register> <register> <register> <register>
            "jump"      <Label>    <register> <register> <register> 
            "banz"      <Label>    <register> <register> <register> 
            "call"      <Label>    <register> <register> <register> 
            "callsys"   K          <register> <register> <register> 
            */
            ST(INST[0], OP_INST, OP_JMOV);
            dst_srcx_register(INST, s[1], OP_DST_INST_MSB,  OP_DST_INST_LSB); 

            /* check it is either set setptr swap del jump banz call callsys save rest ret */
            check_JMOV_opar (s[0], &oparf);     
            ST(INST[0], OP_OPAR_INST, oparf); 

            // s[0]       1            2
            //"swap"    <DST>        <SRC1>
            if (OPLJ_SWAP == oparf)
            {   dst_srcx_register(INST, s[2], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB); 
            }
 
            if (OPLJ_RETURN == oparf) 
            {   /* nothing .. */
            }

            //s[0]        1           
            //"delete"    <SRC2/number> 
            if (OPLJ_DELETE == oparf)
            {   check_alu_opar (s[1], &tmp, &opar0src1orKm1);
                if (opar0src1orKm1 == opar_reg)
                {   dst_srcx_register (INST, s[1], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); 
                }
                else
                {   K_register (INST, s, 1,  OP_SRC2_INST_MSB,  OP_SRC2_INST_LSB); /* s[1] = K */
                }
            }
            
            //s[0]    1        2                3
            //"set" <register> <type/typeptr>  #type              OPLJ_CASTPTR / OPLJ_CAST
            //                 <base/size>     <register/number>  OPLJ_BASE   / OPLJ_SIZE
            if (OPLJ_CAST    == oparf)
            {   dtype_register (INST, s, 3, &dtype); 
                ST(INST[0], OP_K_DTYPE_INST, dtype);
            }
            if (OPLJ_CASTPTR == oparf)
            {   dtype_register (INST, s, 3, &dtype); 
                ST(INST[0], OP_K_DTYPE_INST, dtype);
            }
            if (OPLJ_BASE == oparf)
            {   check_alu_opar (s[3], &tmp, &opar0src1orKm1);
                if (opar0src1orKm1 == opar_reg)  
                {   dst_srcx_register (INST, s[3], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); 
                } else
                {   K_register (INST, s, 3, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB);
                }
            }
            if (OPLJ_SIZE    == oparf)
            {   check_alu_opar (s[3], &tmp, &opar0src1orKm1);
                if (opar0src1orKm1 == opar_reg)  
                {   dst_srcx_register (INST, s[3], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); 
                } else
                {   K_register (INST, s, 3, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB);
                }
            }
            
            //s[0]        1           2         3            4         5
            //"jump"      <Label>    <register> <register> <register> 
            //"banz"      <Label>    <register> <register> <register> 
            //"call"      <Label>    <register> <register> <register> 
            //"callsys"   K          <register> <register> <register> 
            if (OPLJ_JUMP == oparf || OPLJ_BANZ == oparf || OPLJ_CALL == oparf || OPLJ_CALLSYS == oparf)
            {   // Save the instruction offset and the Symbol
                Label_positions[idx_label].position = script->script_nb_instruction;
                tmp = sscanf(s[1], "%s", LabelName);
                strcpy(Label_positions[idx_label].symbol, LabelName);
                Label_positions[idx_label].Label0Jump1 = 1;
                idx_label++;
                if ('\n' == s[2][0]) { strcpy(s[2], RN); }
                if ('\n' == s[3][0]) { strcpy(s[3], RN); }
                if ('\n' == s[4][0]) { strcpy(s[4], RN); }
                dst_srcx_register (INST, s[2], OP_DST_INST_MSB,  OP_DST_INST_LSB );
                dst_srcx_register (INST, s[3], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB);
                dst_srcx_register (INST, s[4], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB);

                if (OPLJ_CALLSYS == oparf)
                {   int service;
                    idx_label --;   // there was no label
                    tmp = sscanf(s[1], "%d", &service);         /* 6bits service */
                    ST(INST[0], CALLSYSIDX_INST, service);
                }
            }
            
            //s[0]        1           2         3            4         5
            //"save"      <register> <register> <register> <register> <register>
            //"restore"   <register> <register> <register> <register> <register>
            if ((OPLJ_SAVE    == oparf) || (OPLJ_RESTORE == oparf))
            {   if ('\n' == s[1][0]) { strcpy(s[4], RN); }
                if ('\n' == s[2][0]) { strcpy(s[2], RN); }
                if ('\n' == s[3][0]) { strcpy(s[3], RN); }
                if ('\n' == s[4][0]) { strcpy(s[4], RN); }
                if ('\n' == s[5][0]) { strcpy(s[4], RN); }
                dst_srcx_register (INST, s[1], OP_DST_INST_MSB,  OP_DST_INST_LSB );
                dst_srcx_register (INST, s[2], OP_SRC1_INST_MSB, OP_SRC1_INST_LSB);
                dst_srcx_register (INST, s[3], OP_SRC2_INST_MSB, OP_SRC2_INST_LSB);
                dst_srcx_register (INST, s[4], OP_SRC3_INST_MSB, OP_SRC3_INST_LSB);
                dst_srcx_register (INST, s[5], OP_SRC4_INST_MSB, OP_SRC4_INST_LSB);
            }

            if (thereAreBrackets)
            {   //s[0]        1     2         3   4            5
                //<register> '['  <register> ']'  '='       <register>  OPLJ_SCATTER
                //<register> '='  <register> '[' <register>   ']'       OPLJ_GATHER
                dst_srcx_register (INST, s[0],  OP_DST_INST_MSB,  OP_DST_INST_LSB); 
                dst_srcx_register (INST, s[2],  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB); 
                if (s[1][0] == '[')
                {   if (s[3][0] != ']' || s[4][0] != '=')  
                    {   fprintf(stderr, " missing ']' or '='  !"); exit(-4);
                    }
                    dst_srcx_register (INST, s[5],  OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_SCATTER);
                    if (s[3][1] == '+')
                    {   ST(INST[0], SCGA_POSTINC_INST, 1);
                    }
                }
                else
                {   if (s[3][0] != '[')  
                    {   fprintf(stderr, " missing ']' or '='  !"); exit(-4);
                    }
                    dst_srcx_register (INST, s[4],  OP_SRC2_INST_MSB, OP_SRC2_INST_LSB); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_GATHER);
                    if (s[5][1] == '+')
                    {   ST(INST[0], SCGA_POSTINC_INST, 1);
                    }
                }
            }
            if (thereAreVerticals)
            {   int len, pos; 
                //s[0]        1     2         3      4    5     6
                //<register> '|'   lenK      posK   '|'  '='  <register>   OPLJ_WR2BF
                //<register> '=' <register>  '|'   lenK posK   '|'         OPLJ_RDBF
                dst_srcx_register (INST, s[0],  OP_DST_INST_MSB,  OP_DST_INST_LSB); 
                if (s[1][0] == '|')
                {   if (s[4][0] != '|' || s[5][0] != '=')  
                    {   fprintf(stderr, " missing '|' or '='  !"); exit(-4);
                    }
                    tmp = sscanf(s[2], "%d", &len); tmp = sscanf(s[3], "%d", &pos);
                    ST(INST[0], BITFIELD_LEN_INST, len);
                    ST(INST[0], BITFIELD_POS_INST, pos);
                    dst_srcx_register (INST, s[6],  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_WR2BF);
                }
                else
                {   if (s[3][0] != '|' || s[1][0] != '=')  
                    {   fprintf(stderr, " missing '|' or '='  !"); exit(-4);
                    }
                    tmp = sscanf(s[4], "%d", &len); tmp = sscanf(s[5], "%d", &pos);
                    ST(INST[0], BITFIELD_LEN_INST, len);
                    ST(INST[0], BITFIELD_POS_INST, pos);
                    dst_srcx_register (INST, s[2],  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_RDBF);
                }
            }

            }   /* else not OP_LD */
        }

        // save byte-codes and corresponding comments        
        strcpy(script->script_comments[script->script_nb_instruction], script_comment);
        if (INST[INST_WORDS-1] >= 1)
        {   script->script_program[script->script_nb_instruction] = INST[0];  script->script_nb_instruction += 1;
        }
        if (INST[INST_WORDS-1] >= 2)
        {   script->script_program[script->script_nb_instruction] = INST[1];  script->script_nb_instruction += 1;
        }
        if (INST[INST_WORDS-1] >= 3)
        {   script->script_program[script->script_nb_instruction] = INST[2];  script->script_nb_instruction += 1;
        }

        if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))    /* end */
        {   jump2next_valid_line(pt_line);
            break;
        }
        if (0 == strncmp (*pt_line,node_parameters,strlen(node_parameters)))    /* node_parameters */
        {   jump2next_valid_line(pt_line);
            stream_tool_read_parameters(pt_line, platform, graph, &(script->ParameterSizeW32), &(script->PackedParameters[0])); 
            break;
        }
    }


    
    /*
        Second pass : find the L_symbol labels
                Label_positions[idx_label].position = script->script_nb_instruction;
                strcpy(Label_positions[idx_label].symbol, LabelName);
    */
    {   int ijump, ilabel, label_position, instruction_position;
        uint32_t *instruction;

        for (ilabel = 0; ilabel < idx_label; ilabel++)
        {   
            if (Label_positions[ilabel].Label0Jump1 == 0)
            {   
                for (ijump = 0; ijump < idx_label; ijump++)
                {   
                    if (Label_positions[ijump].Label0Jump1 == 1)
                    {   
                        if (0 == strcmp(Label_positions[ilabel].symbol, Label_positions[ijump].symbol))
                        {   label_position = Label_positions[ilabel].position;
                            instruction_position = Label_positions[ijump].position;
                            instruction = &(script->script_program[instruction_position]);
                            ST(*instruction, JUMPIDX_INST, label_position - instruction_position);
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

