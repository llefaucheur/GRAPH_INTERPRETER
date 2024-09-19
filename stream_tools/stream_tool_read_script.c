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

/*
;script 0
;    script_code
;        ifyes   testequ r1          add            r2    r5 
;                ld      r2          nop            r3    # 
;                ld      r2          nop            r3    r6 
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

/*  Encoded instructions : 
     COND   INST    DST  LD-TST/ JMP  / MOV     SRC1  SRC2/#Ktype  K
     0      1       2     ------- 3 -------     4     5            6  :    field index
     if yes ld/k    0     add    jmp    wr2bf   0     0   + # and constant type iufdptc (first letter)
     if no  test/k  r1    min    cal    ptrinc  r1    r1
            jmp     r10   set    label          r10   r10
            mov     sp           ret            sp    sp
                    sp1                         sp1   sp1
                                                rk    rk


    remove '{' and '}' characters before text analysis

        ld      r6   r3    
        ld      r2   mul { r4    r6 }	
        testequ r2   add { r2    #15 }
        ifyes callsys 14 
        return
        
        data_section
        label 1
        1  i8; 0                 ; TAG= 0 "load all parameters"
        7  i8; 2 3 4 5 6 7 8     ; parameters
        label 2
        include 1 binary.txt     ; path ID and file name
*/

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
    if (0 != strstr(s, "-"))     { *oparf = OPAR_NOP ; } 
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
}

/* ==================================================================================== */
void registers_fields (char **pt_line, uint32_t *INST, uint32_t *Kfield, 
                       char s[cNFIELDS][cASM])
{   
        jump2next_valid_line(pt_line);

        /* -------------------- CLEAN THE LINE FROM { } --------------------- */
        {   char *pch = strchr(*pt_line,'{');   // search '{' and replace by white space
            while (pch != NULL)
            {   *pch = ' ';  pch = strchr(*pt_line,'{');
            }
            pch = strchr(*pt_line,'}');         // search '}' and replace by white space
            while (pch != NULL)
            {   *pch = ' ';  pch = strchr(*pt_line,'}');
            }
        }

        /* -------------------- SEPARATE THE LINE IN 7 FIELDS --------------------- */
        fields_extract(pt_line, "CCCCCCC", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[cNFIELDS-1]); 

        /* -------------------- FILL THE INSTRUCTION WITH REGISTERS ID --------------------- */


    /* -------------------- CONDITIONAL FIELD --------------------- */
    if (0 == strncmp(s[CONDF], M_IF, 2))    
    {   ST(*INST, OP_INST, NO_TEST);    
        if (0 == strchr(s[CONDF],'y')) { ST(*INST, OP_COND, IF_NOT); } 
        else                           { ST(*INST, OP_COND, IF_YES); }
    }
    else    /* no condition : need to shift the instruction fields on the right  */
    {   int i;
        for (i = cNFIELDS-1; i > 0; i--)
        {   strcpy (s[i], s[i-1]);
        }
    }

    /* -------------------- NO OPERATION FOR LD AND TEST --------------------- */
    if ((0 == strncmp(s[INSTF], M_TEST, 2)) || (0 == strncmp(s[INSTF], M_LD, 2)))
    {   int oparf;
        check_alu_opar (s[OPCODEF], &oparf);
        // CONDF INSTF    DSTF OPCODEF   SRC1F SRC2F KF 
        //   ?   LD/TEST  dst  NOTFOUND  src1  src2  k
        //   ?   LD/TEST  dst            src1  src2  k
        if (oparf < 0)  /* no ALU field : need to shift the other fields on the right */
        {   int i;
            for (i = cNFIELDS-1; i > 0; i--)
            {   strcpy (s[i], s[i-1]);
            }
        }
    }

    /* -------------------- REGS FIELD --------------------- */
    one_register (INST, s[DSTF],  OP_DST_MSB,  OP_DST_LSB);
    one_register (INST, s[SRC1F], OP_SRC1_MSB, OP_SRC1_LSB);
    one_register (INST, s[SRC2F], OP_SRC2_MSB, OP_SRC2_LSB);
    
    /* -------------------- IS THERE A CONSTANT  --------------------- */
    if (0 == strchr(*pt_line,'#'))          // a literal constant to put in RegK ?
    {   *Kfield = 1; 
    } else 
    {   *Kfield = 0;
    }
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
    uint32_t INST, Kfield, optionK;

    while (1)
    {
        INST = 0;
        registers_fields (pt_line, &INST, &Kfield, s);

        /* -------------------- label N --------------------- */
        if (0 != strstr(s[INSTF],"label"))  
        {   
        } 

        /* -------------------- LD --------------------- */
        if (0 == strncmp(s[INSTF], M_LD, 2))
        {   if (Kfield)       // is it load with a literal constant to put in RegK ?
            {   ST(INST, OP_INST, OP_LD);
            } else 
            {   ST(INST, OP_INST, OP_LDK);
            }
        }

        /* -------------------- TEST --------------------- */
        if (0 == strncmp(s[INSTF], M_TEST, 2))
        {   if (Kfield)       // is it load with a literal constant to put in RegK ?
            {   optionK = 0;
            } else 
            {   optionK = LAST_TEST_OPCODE_WITH_K;
            }
            if (0 != strstr(s[INSTF], "equ"))    { ST(INST, OP_INST, OP_TESTEQU + optionK); } 
            if (0 != strstr(s[INSTF], "leq"))    { ST(INST, OP_INST, OP_TESTLEQ + optionK); } 
            if (0 != strstr(s[INSTF], "lt"))     { ST(INST, OP_INST, OP_TESTLT  + optionK); } 
            if (0 != strstr(s[INSTF], "neq"))    { ST(INST, OP_INST, OP_TESTNEQ + optionK); } 
            if (0 != strstr(s[INSTF], "geq"))    { ST(INST, OP_INST, OP_TESTGEQ + optionK); } 
            if (0 != strstr(s[INSTF], "gt"))     { ST(INST, OP_INST, OP_TESTGT  + optionK); } 
        }

        /* -------------------- ALU OPERATIONS FOR LD AND TEST --------------------- */
        if ((0 == strncmp(s[INSTF], M_TEST, 2)) || (0 == strncmp(s[INSTF], M_LD, 2)))
        {   int oparf;

            check_alu_opar (s[OPCODEF], &oparf);
            ST(INST, OP_OPAR, oparf); 
        }


        /* -------------------- JUMP --------------------- */
        if (0 != strstr(s[INSTF], "jmpa"))       { ST(INST, OP_OPAR, OPBR_JUMP      ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "cala"))       { ST(INST, OP_OPAR, OPBR_JUMPA     ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "pushctrl"))   { ST(INST, OP_OPAR, OPBR_JUMPOFF   ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "popctrl"))    { ST(INST, OP_OPAR, OPBR_BANZ      ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "pushlbl"))    { ST(INST, OP_OPAR, OPBR_CALL      ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "jmplabel"))   { ST(INST, OP_OPAR, OPBR_CALA      ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "calllabel"))  { ST(INST, OP_OPAR, OPBR_CALLSYS   ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "banz"))       { ST(INST, OP_OPAR, OPBR_CALLSCRIPT); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "jmpoff"))     { ST(INST, OP_OPAR, OPBR_CALLAPP   ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "call"))       { ST(INST, OP_OPAR, OPBR_SAVEREG   ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "callsys"))    { ST(INST, OP_OPAR, OPBR_RESTOREREG); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "callscript")) { ST(INST, OP_OPAR, OPBR_RETURN    ); ST(INST, OP_INST, OP_JMP); } 
        if (0 != strstr(s[INSTF], "callapp"))    { ST(INST, OP_OPAR, OPBR_LABEL     ); ST(INST, OP_INST, OP_JMP); } 

        /* -------------------- MOV --------------------- */
        if (0 != strstr(s[INSTF],"cast"   ))     { ST(INST, OP_OPAR, OPLD_CAST      ); ST(INST, OP_INST, OP_MOV); } 
        if (0 != strstr(s[INSTF],"castptr"))     { ST(INST, OP_OPAR, OPLD_CASTPTR   ); ST(INST, OP_INST, OP_MOV); } 
        if (0 != strstr(s[INSTF],"base"   ))     { ST(INST, OP_OPAR, OPLD_BASE      ); ST(INST, OP_INST, OP_MOV); }         
        if (0 != strstr(s[INSTF],"size"   ))     { ST(INST, OP_OPAR, OPLD_SIZE      ); ST(INST, OP_INST, OP_MOV); } 
        if (0 != strstr(s[INSTF],"ptrinc" ))     { ST(INST, OP_OPAR, OPLD_PTRINC    ); ST(INST, OP_INST, OP_MOV); } 
        if (0 != strstr(s[INSTF],"scatter"))     { ST(INST, OP_OPAR, OPLD_SCATTER   ); ST(INST, OP_INST, OP_MOV); }         
        if (0 != strstr(s[INSTF],"gather" ))     { ST(INST, OP_OPAR, OPLD_GATHER    ); ST(INST, OP_INST, OP_MOV); } 
        if (0 != strstr(s[INSTF],"wr2bf"  ))     { ST(INST, OP_OPAR, OPLD_WR2BF     ); ST(INST, OP_INST, OP_MOV); } 
        if (0 != strstr(s[INSTF],"rdbf"   ))     { ST(INST, OP_OPAR, OPLD_RDBF      ); ST(INST, OP_INST, OP_MOV); } 
        if (0 != strstr(s[INSTF],"norm"   ))     { ST(INST, OP_OPAR, OPLD_NORM      ); ST(INST, OP_INST, OP_MOV); }         
        if (0 != strstr(s[INSTF],"swap"   ))     { ST(INST, OP_OPAR, OPLD_SWAP      ); ST(INST, OP_INST, OP_MOV); }         
                                                                                   
        (script->script_program[script->script_nb_instruction]) = INST;
        script->script_nb_instruction += 1;

        if (0 == strncmp (*pt_line,SECTION_END,strlen(SECTION_END)))
        {   jump2next_valid_line(pt_line);
            break;
        }
    }
}



#ifdef __cplusplus
}
#endif