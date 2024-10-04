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
void stream_tool_read_assembler(char **pt_line, struct stream_script *script)
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
void one_register (uint32_t *INST, char S[cNFIELDS][cASM], int offset, 
                   uint32_t msb, uint32_t lsb, int canBeConstant, 
                   int32_t *dtype)             
{   char *s;
    int idx, type;
    int32_t iK;
    union floatb {
        uint32_t i;
        float f;
    } fK;
    int64_t llK;

    *dtype = -1;                     // not a constant 
    s = &(S[offset][0]);

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

    /* s[0]='#' => read one more field for the constant */
    idx = 0;
    type = DTYPE_INT32;

    if (s[0] == '#')
    {   if ((0 == strcmp(s, "#ptruint8"))  || (0 == strcmp(s, "#uint8")) ) { type = DTYPE_UINT8 ; }
        if ((0 == strcmp(s, "#ptruint16")) || (0 == strcmp(s, "#uint16"))) { type = DTYPE_UINT16; }
        if ((0 == strcmp(s, "#ptrint16"))  || (0 == strcmp(s, "#int16")) ) { type = DTYPE_INT16 ; }
        if ((0 == strcmp(s, "#ptruint32")) || (0 == strcmp(s, "#uint32"))) { type = DTYPE_UINT32; }
        if ((0 == strcmp(s, "#ptrint32"))  || (0 == strcmp(s, "#int32")) ) { type = DTYPE_INT32 ; }
        if ((0 == strcmp(s, "#ptrint64"))  || (0 == strcmp(s, "#int64")) ) { type = DTYPE_INT64 ; }
        if ((0 == strcmp(s, "#ptrfp16"))   || (0 == strcmp(s, "#fp16"))  ) { type = DTYPE_FP16  ; }
        if ((0 == strcmp(s, "#ptrfloat"))  || (0 == strcmp(s, "#float")) ) { type = DTYPE_FP32  ; }
        if ((0 == strcmp(s, "#ptrdouble")) || (0 == strcmp(s, "#double"))) { type = DTYPE_FP64  ; }
        if ( 0 == strcmp(s, "#time16")  ) { type = DTYPE_TIME16; }
        if ( 0 == strcmp(s, "#time32")  ) { type = DTYPE_TIME32; }
        if ( 0 == strcmp(s, "#time64")  ) { type = DTYPE_TIME64; }
        if ( 0 == strcmp(s, "#ptr")     ) { type = DTYPE_PTR28B; }
        idx = 1;    /* switch to next field as a constant */
    }

    if (isdigit(s[idx]) && canBeConstant)
    {   
        ST(INST[0], OP_SRC2_INST, RegK);                                 /* SRC2 = RK 15 */

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
        case DTYPE_INT32  : scanf (s,"%d", &iK); 
                            if (iK < MAX_LITTLE_K && iK > MIN_LITTLE_K) 
                            {   ST(INST[0], OP_K_INST, iK + MAX_LITTLE_K);  /* small constant */
                            }
                            else
                            {   INST[1] = iK;  
                                INST [INST_WORDS-1] = 2;            /* two words */
                            }
                            break;

        case DTYPE_FP16   : 
        case DTYPE_FP32   : 
        case DTYPE_FP64   : scanf (s,"%f", &(fK.f));                /* double converted to float */
                            INST[1] = fK.i;  
                            INST [INST_WORDS-1] = 2;                /* two words */
                            break;
                            INST[1] = iK;
                            INST [INST_WORDS-1] = 2;                /* two words */
                            break;

        case DTYPE_TIME64 :
        case DTYPE_INT64  : scanf (s,"%lld", &llK); 
                            INST[1] = (int)((int64_t)0xFFFFFFFF & llK);  
                            INST[2] = (int)((int64_t)0xFFFFFFFF & (llK >> 32));  
                            INST [INST_WORDS-1] = 3;                /* three words */
                            break;
        }

        /* constant detection: 0x3FF0 */
        if (INST [INST_WORDS-1] > 1)
        {   ST(INST[0], OP_K_INST, (1 << (1+OP_K_INST_MSB))-1); /* start filling with all-ones */
            ST(INST[0], OP_K_DTYPE_INST, type);      /* DTYPE when there are several words */
            ST(INST[0], OP_RKEXT_INST, 1);           /* RK extension */
        }
    }
    else
    {   fprintf(stderr, "constants start with a number !"); exit(-4);
    }

    if (dtype)
    {   *dtype = type;
    }
}

/* ==================================================================================== */
void check_JMOV_opar (char *s, int *oparf)
{
    *oparf = -1;
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
void check_alu_opar (char *s, int *oparf, int *opar0reg1)
{
    *oparf = OPAR_NOP;
    *opar0reg1 = 0;

    if (0 != strncmp(s, "sp", 2)) { *opar0reg1 = 1; }   /* is it a register ? */
    if (0 != strncmp(s, "r", 1))  { *opar0reg1 = 1; }   /* SPx / Rx */
    *oparf = OPAR_NOP;
    if (s[0] = '-')               { *oparf = OPAR_NOP ; } 
    if (s[0] = '_')               { *oparf = OPAR_NOP ; } 
    if (0 != strstr(s, "nop"))    { *oparf = OPAR_NOP ; } 
    if (0 != strstr(s, "add"))    { *oparf = OPAR_ADD ; } 
    if (0 != strstr(s, "sub"))    { *oparf = OPAR_SUB ; } 
    if (0 != strstr(s, "mul"))    { *oparf = OPAR_MUL ; } 
    if (0 != strstr(s, "div"))    { *oparf = OPAR_DIV ; } 
    if (0 != strstr(s, "or"))     { *oparf = OPAR_OR  ; } 
    if (0 != strstr(s, "nor"))    { *oparf = OPAR_NOR ; } 
    if (0 != strstr(s, "and"))    { *oparf = OPAR_AND ; } 
    if (0 != strstr(s, "xor"))    { *oparf = OPAR_XOR ; } 
    if (0 != strstr(s, "shr"))    { *oparf = OPAR_SHR ; } 
    if (0 != strstr(s, "shl"))    { *oparf = OPAR_SHL ; } 
    if (0 != strstr(s, "set"))    { *oparf = OPAR_SET ; } 
    if (0 != strstr(s, "clr"))    { *oparf = OPAR_CLR ; } 
    if (0 != strstr(s, "max"))    { *oparf = OPAR_MAX ; } 
    if (0 != strstr(s, "min"))    { *oparf = OPAR_MIN ; } 
    if (0 != strstr(s, "amax"))   { *oparf = OPAR_AMAX; } 
    if (0 != strstr(s, "amin"))   { *oparf = OPAR_AMIN; } 
    if (0 != strstr(s, "norm"))   { *oparf = OPAR_NORM; } 
    if (0 != strstr(s, "addmod")) { *oparf = OPAR_ADDMOD; } 
    if (0 != strstr(s, "submod")) { *oparf = OPAR_SUBMOD; } 
}


/* ==================================================================================== */
void clean_line (char **pt_line, uint32_t *INST, 
                       uint8_t *thereIsHash, uint8_t *thereAreBrackets, uint8_t *thereAreVerticals,
                       char *comments, char s[cNFIELDS][cASM])
{   char *pch, test;
    char current_line[NBCHAR_LINE];
    int line_length;

    jump2next_valid_line(pt_line);
    pch = strchr(*pt_line,'\n');
    line_length = (int)(pch - *pt_line);
    strncpy(current_line, *pt_line, line_length);
    thereIsHash = thereAreBrackets = 0; strcpy (comments, "");

    pch = strchr(current_line,';');
    if (0 != pch)   // search ';' copy comments 
    {   strcpy(comments, pch);
        do {
            *pch++ = ' ';                   // replace comments by ' ' 
        } while (*pch != '\n');
    }

    /* -------------------- CLEAN THE LINE FROM { } \ /  and detect '#' '[' ----- */
    thereIsHash = strchr(current_line,'#');   
    thereAreBrackets = strchr(current_line,'[');   
    thereAreVerticals = strchr(current_line,'|');   

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


void stream_tool_read_code(char **pt_line, struct stream_script *script)
{
    char s[cNFIELDS][cASM];
    uint32_t INST[INST_WORDS], nWord;
    uint8_t thereIsHash, thereAreBrackets, thereAreVerticals;
    char script_comment[NBCHAR_LINE];
    int idx_label;
    struct { int position; char symbol[NBCHAR_LINE]; } Label_positions[100];
    char LabelName[NBCHAR_LINE];
    int oparf, opar0reg1, dtype;

    while (1)
    {
        INST[0] = INST[1] = INST[2] = 0; 
        INST[INST_WORDS-1] = 1;                        // one word = one instruction 
        nWord = 1; idx_label = 0;

        /* remove { } \ / =  */
        clean_line (pt_line, &(INST[0]), &thereIsHash, &thereAreBrackets, &thereAreVerticals, script_comment, s);

        /* -------------------- SEPARATE THE LINE IN 7 FIELDS --------------------- */
        fields_extract(pt_line, "CCCCCCC", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[cNFIELDS-1]); 

        /* -------------------- CONDITIONAL FIELD --------------------- */
        if (0 == strncmp(s[0], M_IF, 2))        
        {   int i;

            if (0 == strchr(s[0],'y')) { ST(INST[0], OP_COND_INST, IF_NOT); }   // "if_yes"
            else                       { ST(INST[0], OP_COND_INST, IF_YES); }   // "if_not"

            for (i = cNFIELDS-1; i > 0; i--)      // all the instruction fields are left-shifted
            {   strcpy (s[i], s[i-1]);            //  so next field starts on s[0]
            }
        }

        /* --- TEST --- 
            s[0]         1            2         3       4
            test<xx> + <register> + <ALU> + <register> <register/number>
                                  + <register/number>
        */
        if (0 == strncmp(s[0], M_TEST, 2))
        { 
            if (0 != strstr(s[0], "equ")) { ST(INST[0], OP_INST, OP_TESTEQU); } 
            if (0 != strstr(s[0], "leq")) { ST(INST[0], OP_INST, OP_TESTLEQ); } 
            if (0 != strstr(s[0], "lt"))  { ST(INST[0], OP_INST, OP_TESTLT ); } 
            if (0 != strstr(s[0], "neq")) { ST(INST[0], OP_INST, OP_TESTNEQ); } 
            if (0 != strstr(s[0], "geq")) { ST(INST[0], OP_INST, OP_TESTGEQ); } 
            if (0 != strstr(s[0], "gt"))  { ST(INST[0], OP_INST, OP_TESTGT ); } 

            one_register (INST, s, 1,  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0);     /* register to compare to */
            check_alu_opar (s[2], &oparf, &opar0reg1);                   /* is it ALU or Register */

            if (opar0reg1 == 0)
            {   ST(INST[0], OP_OPAR_INST, oparf);
                one_register (INST, s, 3, OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0, 0); /* <ALU> + <register> */
                one_register (INST, s, 4, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 1, 0); /*       + <register/number> */
            } else
            {   ST(INST[0], OP_OPAR_INST, OPAR_NOP);                            /* NOP */
                one_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 1, 0); /*         <register/number> */
            }
            
        }
        else
        /* ----------------------------------------- OP_LD -------------------- */
        { 

        /* --- LD --- 
        s[0]         1         2               3           4        
        <register>  '='  <register/number>
                         <ALU>              <register> <register/number>
        */
            check_alu_opar (s[0], &oparf, &opar0reg1);                        /* does s[0] is a Register */
            if (opar0reg1 == 1 && thereAreBrackets == 0 && thereAreVerticals == 0)   /* without | and [] */
            {
                ST(INST[0], OP_INST, OP_LD);                                        /* OP_LD instruction family */
                one_register (INST, s, 0,  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0);    /* DST register to load */
                
                if (0 == strstr(s[1], "="))  
                {   fprintf(stderr, "LD missing '='  !"); exit(-4);
                } 
                
                check_alu_opar (s[2], &oparf, &opar0reg1);                  /* does s[2] is a Register or ALU */
                if (opar0reg1 == 1)
                {   one_register (INST, s, 2,  OP_SRC2_INST_MSB,  OP_SRC2_INST_LSB, 1, 0); /* s[2] = SRC2 or K */
                }
                else
                {   ST(INST[0], OP_OPAR_INST, oparf);
                    one_register (INST, s, 3,  OP_SRC1_INST_MSB,  OP_SRC1_INST_LSB, 0, 0); /* s[3] = SRC1 */
                    one_register (INST, s, 4,  OP_SRC2_INST_MSB,  OP_SRC2_INST_LSB, 1, 0); /* s[4] = SRC2 or K */
                }
            }
            else
            {
            /* ----------------------------------------- JMOV -------------------- */
            
            /* --- JMOV --- 
            s[0]        1             2
            "swap"   <register>   <register>
            
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
            */
            ST(INST[0], OP_INST, OP_JMOV);
            
            check_JMOV_opar (s[2], &oparf);  
            ST(INST[0], OP_OPAR_INST, oparf);
            
            //s[0]    1        2                3
            //"set" <register> <type/typeptr>  #type              OPLJ_CASTPTR / OPLJ_CAST
            //                 <base/size>     <register/number>  OPLJ_BASE   / OPLJ_SIZE
            if (OPLJ_CAST    == oparf)
            {   one_register (INST, s, 1, OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); 
                one_register (INST, s, 3, 0,  0,  0, &dtype); 
                ST(INST[0], OP_K_DTYPE_INST, dtype);
            }
            if (OPLJ_CASTPTR == oparf)
            {   one_register (INST, s, 1, OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, &dtype); 
                one_register (INST, s, 3, 0,  0,  0, &dtype); 
                ST(INST[0], OP_K_DTYPE_INST, dtype);
            }
            if (OPLJ_BASE    == oparf)
            {   one_register (INST, s, 1, OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); 
                one_register (INST, s, 3, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); 
            }
            if (OPLJ_SIZE    == oparf)
            {   one_register (INST, s, 1, OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); 
                one_register (INST, s, 3, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); 
            }
            
            //s[0]        1             2
            //"swap"      <register>   <register>
            if (OPLJ_SWAP    == oparf)
            {   one_register (INST, s, 1, OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); 
                one_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); 
            }
            
            //s[0]        1           
            //"delete"    <register/number>
            if (OPLJ_DELETE  == oparf)
            {   one_register (INST, s, 1, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); 
            }
            
            //s[0]        1           2         3            4         5
            //"jump"      <Label>    <register> <register> <register> 
            //"banz"      <Label>    <register> <register> <register> 
            //"call"      <Label>    <register> <register> <register> 
            //"callsys"   K          <register> <register> <register> 
            if (OPLJ_JUMP == oparf || OPLJ_BANZ == oparf || OPLJ_CALL == oparf || OPLJ_CALLSYS == oparf)
            {   
                // Save the instruction offset and the Symbol
                Label_positions[idx_label].position = script->script_nb_instruction;
                strcpy(Label_positions[idx_label].symbol, LabelName);
                idx_label++;
                if (0 != strcmp("", s[2])) {one_register (INST, s, 2, OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); }
                if (0 != strcmp("", s[3])) {one_register (INST, s, 2, OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0, 0); }
                if (0 != strcmp("", s[4])) {one_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); }
            }
            if (OPLJ_CALLSYS == oparf)
            {   int service;
                idx_label --;   // there was no label
                sscanf(s[1], "%d", &service); 
                ST(INST[0], OP_K_INST, service);
            }
            

            //s[0]        1           2         3            4         5
            //"save"      <register> <register> <register> <register> <register>
            //"restore"   <register> <register> <register> <register> <register>
            if ((OPLJ_SAVE    == oparf) || (OPLJ_RESTORE == oparf))
            {
                if (0 != strcmp("", s[1])) {one_register (INST, s, 2, OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); }
                if (0 != strcmp("", s[2])) {one_register (INST, s, 2, OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0, 0); }
                if (0 != strcmp("", s[3])) {one_register (INST, s, 2, OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); }
                if (0 != strcmp("", s[4])) {one_register (INST, s, 2, OP_SRC3_INST_MSB, OP_SRC3_INST_LSB, 0, 0); }
                if (0 != strcmp("", s[5])) {one_register (INST, s, 2, OP_SRC4_INST_MSB, OP_SRC4_INST_LSB, 0, 0); }
            }
            if (OPLJ_RETURN  == oparf) { /* nothing .. */}
            
            
            if (thereAreBrackets)
            {   //s[0]        1     2         3   4            5
                //<register> '['  <register> ']'  '='       <register>  OPLJ_SCATTER
                //<register> '='  <register> '[' <register>   ']'       OPLJ_GATHER
                one_register (INST, s, 0,  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); 
                one_register (INST, s, 2,  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0, 0); 
                if (s[1][0] == '[')
                {   if (s[3][0] != ']' || s[4][0] != '=')  
                    {   fprintf(stderr, " missing ']' or '='  !"); exit(-4);
                    }
                    one_register (INST, s, 5,  OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_SCATTER);
                    if (s[3][1] == '+')
                    {   ST(INST[0], SCGA_POSTINC_INST, 1);
                    }
                }
                else
                {   if (s[3][0] != ']' || s[1][0] != '=')  
                    {   fprintf(stderr, " missing ']' or '='  !"); exit(-4);
                    }
                    one_register (INST, s, 4,  OP_SRC2_INST_MSB, OP_SRC2_INST_LSB, 0, 0); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_GATHER);
                    if (s[5][1] == '+')
                    {   ST(INST[0], SCGA_POSTINC_INST, 1);
                    }
                }
            }
            if (thereAreVerticals)
            {   int len, pos, tmp; 
                //s[0]        1     2         3      4    5     6
                //<register> '|'   lenK      posK   '|'  '='  <register>   OPLJ_WR2BF
                //<register> '=' <register>  '|'   lenK posK   '|'         OPLJ_RDBF
                one_register (INST, s, 0,  OP_DST_INST_MSB,  OP_DST_INST_LSB, 0, 0); 
                if (s[1][0] == '|')
                {   if (s[4][0] != '|' || s[5][0] != '=')  
                    {   fprintf(stderr, " missing '|' or '='  !"); exit(-4);
                    }
                    tmp = sscanf(s[2], "%d", &len); tmp = sscanf(s[3], "%d", &pos);
                    ST(INST[0], BITFIELD_LEN_INST, len);
                    ST(INST[0], BITFIELD_POS_INST, pos);
                    one_register (INST, s, 6,  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0, 0); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_WR2BF);
                }
                else
                {   if (s[3][0] != '|' || s[1][0] != '=')  
                    {   fprintf(stderr, " missing '|' or '='  !"); exit(-4);
                    }
                    tmp = sscanf(s[4], "%d", &len); tmp = sscanf(s[5], "%d", &pos);
                    ST(INST[0], BITFIELD_LEN_INST, len);
                    ST(INST[0], BITFIELD_POS_INST, pos);
                    one_register (INST, s, 2,  OP_SRC1_INST_MSB, OP_SRC1_INST_LSB, 0, 0); 
                    ST(INST[0], OP_OPAR_INST, OPLJ_RDBF);
                }
            }

            }   /* else not OP_LD */
        }
        

        /* -------------------- label N ---------------------------------------label L_symbol      no code-------- */
        if (0 != strstr(s[INSTF],"label"))  
        {   
        } 
        
        // save byte-codes and corresponding comments        
        script->script_program[script->script_nb_instruction] = INST[0];     script->script_nb_instruction += 1;
        strcpy(script->script_comments[script->script_nb_instruction], script_comment);

        if (INST[INST_WORDS-1] == 2)
        {   script->script_program[script->script_nb_instruction] = INST[1];  script->script_nb_instruction += 1;
        }

        if (INST[INST_WORDS-1] == 3)
        {   script->script_program[script->script_nb_instruction] = INST[2];  script->script_nb_instruction += 1;
        }

        if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
        {   jump2next_valid_line(pt_line);
            break;
        }
    }
    /*
        Second pass : find the L_symbol labels
                Label_positions[idx_label].position = script->script_nb_instruction;
                strcpy(Label_positions[idx_label].symbol, LabelName);
    */
}



#ifdef __cplusplus
}
#endif










            /* -------------------------------------------------OP_MOV ------------------------------------------------*/
            //switch (whatIsIt)
            //{
            //case isRange_dst:                   // DST [ a b ] = SRC1      OP_MOV | OPMV_WR2BF | DST | SRC1 | Pos+Len
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_WR2BF);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, BITFIELD_POS, BFpos);
            //    ST(INST, BITFIELD_LENM1, BFlen -1);
            //    break;
            //case isIndexR_dst:                  // DST [SRC2] = SRC1       OP_MOV | OPMV_SCATTER | DST | SRC1 | SRC2
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_SCATTER);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isIndexK_dst:                  // DST [ K ] = SRC1        OP_MOV | OPMV_SCATTERK | DST | SRC1 | K11
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_SCATTERK);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isRange_src:                   // DST = SRC [ a b ]       OP_MOV | OPMV_RDBF | DST | SRC1 | Pos+Len
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_RDBF);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, BITFIELD_POS, BFpos);
            //    ST(INST, BITFIELD_LENM1, BFlen -1);
            //    break;
            //case  isIndexR_src:                 // DST = SRC1 [ SRC2 ]     OP_MOV | OPMV_GATHER | DST | SRC1 | SRC2
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_GATHER);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isIndexK_src:                  // DST = SRC1 [ k ]         OP_MOV | OPMV_GATHERK | DST | SRC1 | K11
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_GATHERK);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case  isLabel:                      // R = alu R #type K        OP_MOV | OPBR_LABEL | DST | 0 | K11
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPBR_LABEL);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, 0);
            //    ST(INST, OP_K11, K11);
            //    break;


            //case is_LDR:                        // R = alu R R              OP_LD | ALU OPAR | DST | SRC1 | SRC2
            //    ST(INST, OP_INST, OP_LD);
            //    ST(INST, OP_OPAR_INST, ALUopar);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case is_LDK11:                      // R = alu R K11            OP_LDK | ALU OPAR | DST | SRC1 | K11
            //    ST(INST, OP_INST, OP_LDK);
            //    ST(INST, OP_OPAR_INST, ALUopar);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case is_LDK_long:                   // R = alu R #type K        OP_LDK | ALU OPAR | DST | SRC1 | K-DTYPE + EXTRA WORDS
            //    ST(INST, OP_INST, OP_LDK);
            //    ST(INST, OP_OPAR_INST, ALUopar);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_K11, K_MAX + K_dtype);
            //    INST2 = K11;
            //    nWord = 2;
            //    break;
            //case is_LDK_double:                 // R = alu R #type K        OP_LDK | ALU OPAR | DST | SRC1 | K-DTYPE + 2 EXTRA WORDS
            //    ST(INST, OP_INST, OP_LDK);
            //    ST(INST, OP_OPAR_INST, ALUopar);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_K11, K_MAX + K_dtype);
            //    INST2 = K11;
            //    INST3 = Kdouble;
            //    nWord = 2;
            //    break;
            //

            //case isLdtype    :                  // ld r2 type #float        OP_MOV | OPMV_CAST | DST | 0 | K-DTYPE 
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_CAST);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_K11, K_dtype);
            //    break;
            //case isLdtypeptr :                  // ld r2 typeptr #float     OP_MOV | OPMV_CASTPTR | DST | 0 | K-DTYPE 
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_CASTPTR);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_K11, K_dtype);
            //    break;
            //case isLdBase    :                  // ld r4 base r5            OP_MOV | OPMV_BASE | DST | SRC1 | 0
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_BASE);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    break;
            //case isLdSize    :                  // ld r4 size r5            OP_MOV | OPMV_SIZE | DST | SRC1 | 0
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_SIZE);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    break;
            //case isLdBaseK   :                  // ld r4 base K             OP_MOV | OPMV_BASEK | DST | 0 | K11 
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_BASEK);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isLdSizeK   :                  // ld r4 size K             OP_MOV | OPMV_SIZEK | DST | 0 | K11
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_SIZEK);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isLdIncptr  :        // ld r4 incptr r5 #INCTYPE           OP_MOV | OPMV_PTRINC | DST | SRC1 | K11
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_PTRINC);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isLdSwap    :                  // ld r2 swap r3            OP_MOV | OPMV_SWAP | DST | SRC1 | 0
            //    ST(INST, OP_INST, OP_MOV);
            //    ST(INST, OP_OPAR_INST, OPMV_SWAP);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    break;

            //case isJump      :                  // jump L_symbol            OP_JMP | OPBR_JUMP | saveSRC1 | saveSRC2 | K11_decided_later
            //    // Save the instruction offset and the Symbol
            //    Label_positions[idx_label].position = script->script_nb_instruction;
            //    strcpy(Label_positions[idx_label].symbol, LabelName);
            //    idx_label++;
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_JUMP);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isJumpK     :                  // jump k11                 OP_JMP | OPBR_JUMP | saveSRC1 | saveSRC2 | K11
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_JUMP);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isJumpR     :                  // jump R                   OP_JMP | OPBR_JUMPR | DST | saveSRC1 | saveSRC2 | 0
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_JUMPR);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isCall      :                  // call L_symbol            OP_JMP | OPBR_CALL | saveSRC1 | saveSRC2 | K11_decided_later
            //    // Save the instruction offset and the Symbol
            //    Label_positions[idx_label].position = script->script_nb_instruction;
            //    strcpy(Label_positions[idx_label].symbol, LabelName);
            //    idx_label++;
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_CALL);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isCallR     :                  // call R                   OP_JMP | OPBR_CALL | DST | saveSRC1 | saveSRC2 | 0
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_CALLR);
            //    ST(INST, OP_DST_INST, DSTreg);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isCallK     :                  // call K11                OP_JMP | OPBR_CALL | saveSRC1 | saveSRC2 | K11_decided_later
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_CALL);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isCallSys   :                  // callsys index            OP_JMP | OPBR_CALLSYS | saveSRC1 | saveSRC2 | K11
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_CALLSYS);
            //    ST(INST, OP_K11, K11);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isCallScript:                  // callscript index         OP_JMP | OPBR_CALLSCRIPT | saveSRC1 | saveSRC2 | K11
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_CALLSCRIPT);
            //    ST(INST, OP_K11, K11);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isCallApp   :                  // callapp index            OP_JMP | OPBR_CALLAPP | saveSRC1 | saveSRC2 | K11
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_CALLAPP);
            //    ST(INST, OP_K11, K11);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;
            //case isBanz      :                  // banz label               OP_JMP | OPBR_BANZ | saveSRC1 | saveSRC2 | K11
            //    Label_positions[idx_label].position = script->script_nb_instruction;
            //    strcpy(Label_positions[idx_label].symbol, LabelName);
            //    idx_label++;
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_BANZ);
            //    ST(INST, OP_SRC1_INST, SRC1reg);
            //    ST(INST, OP_SRC2_INST, SRC2reg);
            //    break;

            //case isSave      :                  // save 3 4 5               OP_JMP | OPBR_SAVEREG | 0 | 0 | K11
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_SAVEREG);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isRestore   :                  // restore 3 4 5           OP_JMP | OPBR_RESTOREREG | 0 | 0 | K11
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_RESTOREREG);
            //    ST(INST, OP_K11, K11);
            //    break;
            //case isReturn    :                  // return 3 4 5            OP_JMP | OPBR_RETURN | 0 | 0 | K11
            //    ST(INST, OP_INST, OP_JMP);
            //    ST(INST, OP_OPAR_INST, OPBR_RETURN);
            //    ST(INST, OP_K11, K11);
            //    break;
            //
            //default :
            //    fprintf(stderr, "bad instruction !"); exit(-4);
            //}