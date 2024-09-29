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


#define isLabel         1   // R = L_<sym> 
#define isRange_dst     2   // R [ a b ] = R
#define isIndexR_dst    3   // R [ Ru ] = R
#define isIndexK_dst    4   // R [ k ] = R
#define isRange_src     5   // R = R [ a b ] 
#define isIndexR_src    6   // R = R [ Ru ] 
#define isIndexK_src    7   // R = R [ k ] 
#define is_LDK11        8   // R = alu R #int K11
#define is_LDR          9   // R = alu R R
#define is_LDK_long    10   // R = alu R #type K
#define is_LDK_double  11   // R = alu R #type K (int64 / double / time64)
#define isLdtype       12   // ld r2 type #float
#define isLdtypeptr    13   // ld r2 typeptr #float
#define isLdBase       14   // ld r4 base r5         
#define isLdSize       15   // ld r4 size r5         
#define isLdBaseK      16   // ld r4 base r5         
#define isLdSizeK      17   // ld r4 size r5         
#define isLdIncptr     18   // ld r4 incptr r5 #INCTYPE
#define isLdSwap       19   // ld r2 swap r3
#define isJump         20   // jump L_symbol
#define isJumpR        21   // jump Rii
#define isJumpK        22   // jump k11
#define isCall         23   // call L_symbol
#define isCallR        24   // call R
#define isCallK        25   // call K11 
#define isCallSys      26   // callsys K11
#define isCallScript   27   // callscript K11
#define isCallApp      28   // callapp K11
#define isBanz         29   // bank L_symbol
#define isSave         30   // save 3 4 5
#define isRestore      31   // restore 3 4 5
#define isReturn       32   // return 3 4 5

/*
;script 0
;    script_code
;        ifyes   testequ r1          add            r2    r5 
;                ld      r2          nop            r3    # 
;                ld      r2          nop            r3    r6 
;
;end
;        ; COND   INST    DST  LD-TST/ JMP  / MOV     SRC1  SRC2/#K
;        ; 0      1       2     ------- 3 -------     4     5  : field index
;        ; if yes ld/k    0     add    jmp    wr2bf   0     0   + # and constant type iufdptc (first letter)
;        ; if no  test/k  r1    min    cal    ptrinc  r1    r1
;        ;        jmp     r10   set    label          r10   r10
;        ;        mov     sp           ret            sp    sp
;        ;                sp1                         sp1   sp1
;        ;                                            rk    rk
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
void one_register (uint32_t *INST, char *s, uint32_t msb, uint32_t lsb)
{
    if (0 == strncmp(s, "0", 1))    // DST = R0
    {   INSERT_BITS(*INST, msb, lsb, Reg0); 
    } 
    if (0 == strncmp(s, "s", 1))    // DST = SP or SP1
    {   if (0 == strchr(s,'1'))
        {   INSERT_BITS(*INST, msb, lsb, RegSP1);       // DSP = SP1
        }   else
        {   INSERT_BITS(*INST, msb, lsb, RegSP);        // DST = SP
        }
    }
    if (0 == strncmp(s, "r", 1))    // DST = Rm
    {   int i = atoi(&(s[1])); 
        INSERT_BITS(*INST, msb, lsb, i); // read R.. 
    } 
}

/* ==================================================================================== */
void check_alu_opar (char *s, int *oparf)
{
    *oparf = -1;    // not found 
    if (s[0] = '-')              { *oparf = OPAR_NOP ; } 
    if (s[0] = '_')              { *oparf = OPAR_NOP ; } 
    if (0 != strstr(s, "nop"))   { *oparf = OPAR_NOP ; } 
    if (0 != strstr(s, "add"))   { *oparf = OPAR_ADD ; } 
    if (0 != strstr(s, "sub"))   { *oparf = OPAR_SUB ; } 
    if (0 != strstr(s, "mul"))   { *oparf = OPAR_MUL ; } 
    if (0 != strstr(s, "div"))   { *oparf = OPAR_DIV ; } 
    if (0 != strstr(s, "or"))    { *oparf = OPAR_OR  ; } 
    if (0 != strstr(s, "nor"))   { *oparf = OPAR_NOR ; } 
    if (0 != strstr(s, "and"))   { *oparf = OPAR_AND ; } 
    if (0 != strstr(s, "xor"))   { *oparf = OPAR_XOR ; } 
    if (0 != strstr(s, "shr"))   { *oparf = OPAR_SHR ; } 
    if (0 != strstr(s, "shl"))   { *oparf = OPAR_SHL ; } 
    if (0 != strstr(s, "set"))   { *oparf = OPAR_SET ; } 
    if (0 != strstr(s, "clr"))   { *oparf = OPAR_CLR ; } 
    if (0 != strstr(s, "max"))   { *oparf = OPAR_MAX ; } 
    if (0 != strstr(s, "min"))   { *oparf = OPAR_MIN ; } 
    if (0 != strstr(s, "amax"))  { *oparf = OPAR_AMAX; } 
    if (0 != strstr(s, "amin"))  { *oparf = OPAR_AMIN; } 
    if (0 != strstr(s, "norm"))  { *oparf = OPAR_NORM; } 
}
        //if (0 != strstr(s[INSTF],"cast"   ))     { ST(INST, OP_OPAR, OPMV_CAST      ); ST(INST, OP_INST, OP_MOV); } 
        //if (0 != strstr(s[INSTF],"castptr"))     { ST(INST, OP_OPAR, OPMV_CASTPTR   ); ST(INST, OP_INST, OP_MOV); } 
        //if (0 != strstr(s[INSTF],"base"   ))     { ST(INST, OP_OPAR, OPMV_BASE      ); ST(INST, OP_INST, OP_MOV); }         
        //if (0 != strstr(s[INSTF],"size"   ))     { ST(INST, OP_OPAR, OPMV_SIZE      ); ST(INST, OP_INST, OP_MOV); } 
        //if (0 != strstr(s[INSTF],"ptrinc" ))     { ST(INST, OP_OPAR, OPMV_PTRINC    ); ST(INST, OP_INST, OP_MOV); } 
        //if (0 != strstr(s[INSTF],"scatter"))     { ST(INST, OP_OPAR, OPMV_SCATTER   ); ST(INST, OP_INST, OP_MOV); }         
        //if (0 != strstr(s[INSTF],"gather" ))     { ST(INST, OP_OPAR, OPMV_GATHER    ); ST(INST, OP_INST, OP_MOV); } 
        //if (0 != strstr(s[INSTF],"wr2bf"  ))     { ST(INST, OP_OPAR, OPMV_WR2BF     ); ST(INST, OP_INST, OP_MOV); } 
        //if (0 != strstr(s[INSTF],"rdbf"   ))     { ST(INST, OP_OPAR, OPMV_RDBF      ); ST(INST, OP_INST, OP_MOV); } 
        //if (0 != strstr(s[INSTF],"swap"   ))     { ST(INST, OP_OPAR, OPMV_SWAP      ); ST(INST, OP_INST, OP_MOV); }  

/* ==================================================================================== */
void read_fields(char s[cNFIELDS][cASM], int offset, int *whatIsIt, int *ALUopar, int *DSTreg, int *SRC1reg, int *SRC2reg, int *K11, 
                int *indexReg, int *Kdouble, int *K_dtype, int *BFpos, int *BFlen, char *LabelName)
{
    uint8_t brackets, hash, Label;
    uint8_t is = offset;
    char *pch;

    (0 == strchr(s,'1'))

#define isLabel         1   // R = L_<sym> 
#define isRange_dst     2   // R [ a b ] = R
#define isIndexR_dst    3   // R [ Ru ] = R
#define isIndexK_dst    4   // R [ k ] = R
#define isRange_src     5   // R = R [ a b ] 
#define isIndexR_src    6   // R = R [ Ru ] 
#define isIndexK_src    7   // R = R [ k ] 
#define is_LDK11        8   // R = alu R #int K11
#define is_LDR          9   // R = alu R R
#define is_LDK_long    10   // R = alu R #type K
#define is_LDK_double  11   // R = alu R #type K (int64 / double / time64)
#define isLdtype       12   // ld r2 type #float
#define isLdtypeptr    13   // ld r2 typeptr #float
#define isLdBase       14   // ld r4 base r5         
#define isLdSize       15   // ld r4 size r5         
#define isLdBaseK      16   // ld r4 base r5         
#define isLdSizeK      17   // ld r4 size r5         
#define isLdIncptr     18   // ld r4 incptr r5 #INCTYPE
#define isLdSwap       19   // ld r2 swap r3
#define isJump         20   // jump L_symbol
#define isJumpR        21   // jump Rii
#define isJumpK        22   // jump k11
#define isCall         23   // call L_symbol
#define isCallR        24   // call R
#define isCallK        25   // call K11 
#define isCallSys      26   // callsys K11
#define isCallScript   27   // callscript K11
#define isCallApp      28   // callapp K11
#define isBanz         29   // bank L_symbol
#define isSave         30   // save 3 4 5
#define isRestore      31   // restore 3 4 5
#define isReturn       32   // return 3 4 5
}


/* ==================================================================================== */
void clean_line (char **pt_line, uint32_t *INST, 
                       uint8_t *thereIsHash, uint8_t *thereAreBrackets, char *comments,
                       char s[cNFIELDS][cASM])
{   char *pch;
    char current_line[NBCHAR_LINE];
    int line_length;

    jump2next_valid_line(pt_line);
    pch = strchr(*pt_line,'\n');
    line_length = (int)(pch - *pt_line);
    strncpy(current_line, *pt_line, line_length);
    thereIsHash = thereAreBrackets = 0; strcpy (comments, "");

    /* -------------------- CLEAN THE LINE FROM { } and detect '#' '[' ----- */
    pch = strchr(current_line,';');
    if (0 != pch)   // search ';' copy comments 
    {   strcpy(comments, pch);
        do {
            *pch++ = ' ';                   // replace comments by ' ' 
        } while (*pch != '\n');
    }
    thereIsHash = strchr(current_line,'#');   
    thereAreBrackets = strchr(current_line,'[');   

    pch = strchr(current_line,'{');   // search '{' and replace by white space
    while (pch != NULL)
    {   *pch = ' ';  pch = strchr(current_line,'{');
    }
    pch = strchr(current_line,'}');         // search '}' and replace by white space
    while (pch != NULL)
    {   *pch = ' ';  pch = strchr(current_line,'}');
    }

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
    uint32_t INST, INST2, INST3, nWord, optionK;
    uint8_t thereIsHash, thereAreBrackets;
    char script_comment[NBCHAR_LINE];
    int idx_label;
    struct { int position; char symbol[NBCHAR_LINE]; } Label_positions[100];
    char LabelName[NBCHAR_LINE];

    while (1)
    {
        INST = 0; nWord = 1; idx_label = 0;
        clean_line (pt_line, &INST, &thereIsHash, &thereAreBrackets, script_comment, s);

        /* -------------------- SEPARATE THE LINE IN 7 FIELDS --------------------- */
        fields_extract(pt_line, "CCCCCCC", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[cNFIELDS-1]); 

        /* -------------------- CONDITIONAL FIELD --------------------- */
        if (0 == strncmp(s[0], M_IF, 2))        
        {   int i;
            ST(INST, OP_INST, NO_TEST);    
            if (0 == strchr(s[0],'y')) { ST(INST, OP_COND, IF_NOT); }       // "ifyes"
            else                       { ST(INST, OP_COND, IF_YES); }   // "ifnot"

            for (i = cNFIELDS-1; i > 0; i--)
            {   strcpy (s[i], s[i-1]);
            }
        }

        /* -------------------- TEST ---------------testxx Rii arithmetic_OP Rjj + Ru / {#type K}------------ */
        if (0 == strncmp(s[0], M_TEST, 2))
        {   int oparf;
            
            if (thereIsHash)       // is it load with a literal constant to put in RegK ?
            {   optionK = 0;
            } else 
            {   optionK = LAST_TEST_OPCODE_WITH_K;
            }
            if (0 != strstr(s[0], "equ"))    { ST(INST, OP_INST, OP_TESTEQU + optionK); } 
            if (0 != strstr(s[0], "leq"))    { ST(INST, OP_INST, OP_TESTLEQ + optionK); } 
            if (0 != strstr(s[0], "lt"))     { ST(INST, OP_INST, OP_TESTLT  + optionK); } 
            if (0 != strstr(s[0], "neq"))    { ST(INST, OP_INST, OP_TESTNEQ + optionK); } 
            if (0 != strstr(s[0], "geq"))    { ST(INST, OP_INST, OP_TESTGEQ + optionK); } 
            if (0 != strstr(s[0], "gt"))     { ST(INST, OP_INST, OP_TESTGT  + optionK); } 

            check_alu_opar (s[OPCODEF], &oparf);  
            ST(INST, OP_OPAR, oparf);

            one_register (&INST, s[1],  OP_DST_MSB,  OP_DST_LSB);
            one_register (&INST, s[2], OP_SRC1_MSB, OP_SRC1_LSB);
            one_register (&INST, s[3], OP_SRC2_MSB, OP_SRC2_LSB);
        }
        else
        /* ------------------------------------------------------------ LD + ALU + JUMP + others -------------------- */
        {   int whatIsIt, ALUopar, K11, K_dtype, Kdouble, BFpos, BFlen, DSTreg, indexReg, SRC1reg, SRC2reg; 

            /* K11 = 11b/ LSB second word   Kdouble = MSB second word */
            read_fields(s, DSTF, &whatIsIt, &ALUopar, &DSTreg, &SRC1reg, &SRC2reg, &K11, 
                &indexReg, &Kdouble, &K_dtype, &BFpos, &BFlen, LabelName);

            /* -------------------------------------------------OP_MOV ------------------------------------------------*/
            switch (whatIsIt)
            {
            case isRange_dst:                   // DST [ a b ] = SRC1      OP_MOV | OPMV_WR2BF | DST | SRC1 | Pos+Len
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_WR2BF);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, BITFIELD_POS, BFpos);
                ST(INST, BITFIELD_LENM1, BFlen -1);
                break;
            case isIndexR_dst:                  // DST [SRC2] = SRC1       OP_MOV | OPMV_SCATTER | DST | SRC1 | SRC2
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_SCATTER);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isIndexK_dst:                  // DST [ K ] = SRC1        OP_MOV | OPMV_SCATTERK | DST | SRC1 | K11
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_SCATTERK);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_K11, K11);
                break;
            case isRange_src:                   // DST = SRC [ a b ]       OP_MOV | OPMV_RDBF | DST | SRC1 | Pos+Len
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_RDBF);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, BITFIELD_POS, BFpos);
                ST(INST, BITFIELD_LENM1, BFlen -1);
                break;
            case  isIndexR_src:                 // DST = SRC1 [ SRC2 ]     OP_MOV | OPMV_GATHER | DST | SRC1 | SRC2
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_GATHER);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isIndexK_src:                  // DST = SRC1 [ k ]         OP_MOV | OPMV_GATHERK | DST | SRC1 | K11
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_GATHERK);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_K11, K11);
                break;
            case  isLabel:                      // R = alu R #type K        OP_MOV | OPBR_LABEL | DST | 0 | K11
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPBR_LABEL);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, 0);
                ST(INST, OP_K11, K11);
                break;


            case is_LDR:                        // R = alu R R              OP_LD | ALU OPAR | DST | SRC1 | SRC2
                ST(INST, OP_INST, OP_LD);
                ST(INST, OP_OPAR, ALUopar);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case is_LDK11:                      // R = alu R K11            OP_LDK | ALU OPAR | DST | SRC1 | K11
                ST(INST, OP_INST, OP_LDK);
                ST(INST, OP_OPAR, ALUopar);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_K11, K11);
                break;
            case is_LDK_long:                   // R = alu R #type K        OP_LDK | ALU OPAR | DST | SRC1 | K-DTYPE + EXTRA WORDS
                ST(INST, OP_INST, OP_LDK);
                ST(INST, OP_OPAR, ALUopar);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_K11, K_MAX + K_dtype);
                INST2 = K11;
                nWord = 2;
                break;
            case is_LDK_double:                 // R = alu R #type K        OP_LDK | ALU OPAR | DST | SRC1 | K-DTYPE + 2 EXTRA WORDS
                ST(INST, OP_INST, OP_LDK);
                ST(INST, OP_OPAR, ALUopar);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_K11, K_MAX + K_dtype);
                INST2 = K11;
                INST3 = Kdouble;
                nWord = 2;
                break;
            

            case isLdtype    :                  // ld r2 type #float        OP_MOV | OPMV_CAST | DST | 0 | K-DTYPE 
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_CAST);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_K11, K_dtype);
                break;
            case isLdtypeptr :                  // ld r2 typeptr #float     OP_MOV | OPMV_CASTPTR | DST | 0 | K-DTYPE 
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_CASTPTR);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_K11, K_dtype);
                break;
            case isLdBase    :                  // ld r4 base r5            OP_MOV | OPMV_BASE | DST | SRC1 | 0
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_BASE);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                break;
            case isLdSize    :                  // ld r4 size r5            OP_MOV | OPMV_SIZE | DST | SRC1 | 0
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_SIZE);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                break;
            case isLdBaseK   :                  // ld r4 base K             OP_MOV | OPMV_BASEK | DST | 0 | K11 
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_BASEK);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_K11, K11);
                break;
            case isLdSizeK   :                  // ld r4 size K             OP_MOV | OPMV_SIZEK | DST | 0 | K11
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_SIZEK);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_K11, K11);
                break;
            case isLdIncptr  :        // ld r4 incptr r5 #INCTYPE           OP_MOV | OPMV_PTRINC | DST | SRC1 | K11
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_PTRINC);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_K11, K11);
                break;
            case isLdSwap    :                  // ld r2 swap r3            OP_MOV | OPMV_SWAP | DST | SRC1 | 0
                ST(INST, OP_INST, OP_MOV);
                ST(INST, OP_OPAR, OPMV_SWAP);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                break;

            case isJump      :                  // jump L_symbol            OP_JMP | OPBR_JUMP | saveSRC1 | saveSRC2 | K11_decided_later
                // Save the instruction offset and the Symbol
                Label_positions[idx_label].position = script->script_nb_instruction;
                strcpy(Label_positions[idx_label].symbol, LabelName);
                idx_label++;
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_JUMP);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isJumpK     :                  // jump k11                 OP_JMP | OPBR_JUMP | saveSRC1 | saveSRC2 | K11
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_JUMP);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                ST(INST, OP_K11, K11);
                break;
            case isJumpR     :                  // jump R                   OP_JMP | OPBR_JUMPR | DST | saveSRC1 | saveSRC2 | 0
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_JUMPR);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isCall      :                  // call L_symbol            OP_JMP | OPBR_CALL | saveSRC1 | saveSRC2 | K11_decided_later
                // Save the instruction offset and the Symbol
                Label_positions[idx_label].position = script->script_nb_instruction;
                strcpy(Label_positions[idx_label].symbol, LabelName);
                idx_label++;
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_CALL);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isCallR     :                  // call R                   OP_JMP | OPBR_CALL | DST | saveSRC1 | saveSRC2 | 0
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_CALLR);
                ST(INST, OP_DST, DSTreg);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isCallK     :                  // call K11                OP_JMP | OPBR_CALL | saveSRC1 | saveSRC2 | K11_decided_later
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_CALL);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                ST(INST, OP_K11, K11);
                break;
            case isCallSys   :                  // callsys index            OP_JMP | OPBR_CALLSYS | saveSRC1 | saveSRC2 | K11
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_CALLSYS);
                ST(INST, OP_K11, K11);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isCallScript:                  // callscript index         OP_JMP | OPBR_CALLSCRIPT | saveSRC1 | saveSRC2 | K11
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_CALLSCRIPT);
                ST(INST, OP_K11, K11);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isCallApp   :                  // callapp index            OP_JMP | OPBR_CALLAPP | saveSRC1 | saveSRC2 | K11
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_CALLAPP);
                ST(INST, OP_K11, K11);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;
            case isBanz      :                  // banz label               OP_JMP | OPBR_BANZ | saveSRC1 | saveSRC2 | K11
                Label_positions[idx_label].position = script->script_nb_instruction;
                strcpy(Label_positions[idx_label].symbol, LabelName);
                idx_label++;
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_BANZ);
                ST(INST, OP_SRC1, SRC1reg);
                ST(INST, OP_SRC2, SRC2reg);
                break;

            case isSave      :                  // save 3 4 5               OP_JMP | OPBR_SAVEREG | 0 | 0 | K11
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_SAVEREG);
                ST(INST, OP_K11, K11);
                break;
            case isRestore   :                  // restore 3 4 5           OP_JMP | OPBR_RESTOREREG | 0 | 0 | K11
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_RESTOREREG);
                ST(INST, OP_K11, K11);
                break;
            case isReturn    :                  // return 3 4 5            OP_JMP | OPBR_RETURN | 0 | 0 | K11
                ST(INST, OP_INST, OP_JMP);
                ST(INST, OP_OPAR, OPBR_RETURN);
                ST(INST, OP_K11, K11);
                break;
            
            default :
                fprintf(stderr, "bad instruction !"); exit(-4);
            }
        }

        /* -------------------- label N ---------------------------------------label L_symbol      no code-------- */
        if (0 != strstr(s[INSTF],"label"))  
        {   
        } 
        
        // save byte-codes and corresponding comments        
        script->script_program[script->script_nb_instruction] = INST;       script->script_nb_instruction += 1;
        strcpy(script->script_comments[script->script_nb_instruction], script_comment);

        if (nWord == 2)
        {   script->script_program[script->script_nb_instruction] = INST2;  script->script_nb_instruction += 1;
        }

        if (nWord == 3)
        {   script->script_program[script->script_nb_instruction] = INST3;  script->script_nb_instruction += 1;
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