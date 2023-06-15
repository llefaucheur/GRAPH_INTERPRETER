/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Tools
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

#define _CRT_SECURE_NO_DEPRECATE 1
#include <stdio.h>    
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

#include "../stream_src/stream_const.h"
#include "../stream_src/stream_types.h"

#define SZ_LINE 200
char current_line[SZ_LINE];

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
void jump2next_line(char **line)
{
    while (*(*line) != '\n') 
    {   (*line)++;
    };
    (*line)++;
}
/* ---------------------------------------------------------------------- */
static uint32_t jump2next_valid_line (char **line)
{
    while ((*line)[0] == ';')     /* skip lines starting with ';' */
        jump2next_line(line);

#define NOT_YET_END_OF_GRAPH 1
#define FOUND_END_OF_GRAPH 2

    if (0 == strncmp (*line,"ENDGRAPH",8))
        return FOUND_END_OF_GRAPH;
    else
        return NOT_YET_END_OF_GRAPH;
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

/* ---------------------------------------------------------------------- */
// fields_extract(&pt_line, 3, "III", &ARC_ID, &IFORMAT, &SIZE);

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
    nfields = strlen(types);

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
                nchar = ptstart0 - ptstart;
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


/* 
    the number of coefficients per nodes can be large
*/
static uint32_t parameters[MAX_FPE4M6];


/* 
    list of all the memory banks, for debug
*/
#define MAX_MEMORY_BANK_DBG 1000
static uint32_t memory_banks[MAX_MEMORY_BANK_DBG], nb_memory_banks;


/**
  @brief            (main) 
  @param[in/out]    none
  @return           int

  @par              translates the graph intermediate format GraphTxt to GraphBin to be reused
                    in CMSIS-Stream/stream_graph/*.txt
  @remark
 */

void arm_stream_graphTxt2Bin (char *graph_txt, FILE *ptf_graph_bin)
{
    char *pt_line;
    uint32_t addrBytes, FMT0, FMT1, FMT2;
    char comments[SZ_LINE];
    char comment1[SZ_LINE];
    char comment2[SZ_LINE];
    char comment3[SZ_LINE];
    static fpos_t pos_NWords, pos_PACKFORMATIOSSCRIPT, pos_PACKLINKEDLNBINSTANCE, pos_PACKNBARCDEBUG;
    static uint32_t nFMT, LENscript, LinkedList, LinkedList0, NbInstance, nIOs, NBarc, SizeDebug;

    pt_line = graph_txt;
    addrBytes = 0;

    while (NOT_YET_END_OF_GRAPH == jump2next_valid_line(&pt_line))
    {
        switch (*pt_line)
        {   
            /* ============================================================= */
            case _comments:
            {   pt_line++;     // skip the "c" field
                fields_extract(&pt_line, "c", &(comments[0])); pt_line--;
                fprintf (ptf_graph_bin, "//%s\n", comments);
                break;
            }
            /* ============================================================= */
            case _offsets:
            {   uint32_t share, RAMsplit, off, base;

                /* save the address for the last operation : saving the number of bytes in the file */
                fgetpos (ptf_graph_bin, &pos_NWords);
                fprintf (ptf_graph_bin, "0x%08X, // number of Words in this graph \n", 0); 

                pt_line++;     // skip the "o" field
                fields_extract(&pt_line, "iic", &share, &RAMsplit, &comment1);
                fields_extract(&pt_line, "ihc", &off,  &base, &comment2);

                FMT0 = 0;
                ST(FMT0, DATAOFF_ARCW0, off);
                ST(FMT0, BASEIDX_ARCW0, base);
                ST(FMT0, PRODUCFMT_ARCW0, PACKSHARERAMSPLIT(share,RAMsplit));
                fprintf(ptf_graph_bin, "0x%08X, // %03X %03X [0] 27b RAM address, HW-shared MEM & RAM copy config\n", FMT0, addrBytes, addrBytes/4); addrBytes += 4;

                fgetpos (ptf_graph_bin, &pos_PACKFORMATIOSSCRIPT);
                fprintf(ptf_graph_bin, "0x%08X, // %03X %03X [1] Scripts size, NB IOs, number of FORMAT, \n", 0, addrBytes, addrBytes/4); addrBytes += 4;

                fgetpos (ptf_graph_bin, &pos_PACKLINKEDLNBINSTANCE);
                fprintf(ptf_graph_bin, "0x%08X, // %03X %03X [2] size of LINKEDLIST, number of STREAM_INSTANCES\n", 0, addrBytes, addrBytes/4); addrBytes += 4;

                fgetpos (ptf_graph_bin, &pos_PACKNBARCDEBUG);
                fprintf(ptf_graph_bin, "0x%08X, // %03X %03X [3] number of ARCS, number of DEBUG registers\n", 0, addrBytes, addrBytes/4); addrBytes += 4;
                break;
            }

            /* ============================================================= */
            case _header:  /* Header and IOs stream_format_io */
            {   uint32_t idxio, arc, fw_io_idx, rx0tx1, follower, iocommand, ext, inst, domain, settings;

                pt_line++;     // skip the "h" field
                fields_extract(&pt_line, "i", &nIOs);
                for (idxio = 0; idxio < nIOs; idxio++)
                {
                    fields_extract(&pt_line, "iiiiiiiiic",  
                        &arc, &fw_io_idx, &rx0tx1, &follower, &iocommand, &ext, &inst, &domain, &settings, comments);

                    FMT0 = 0;
                    ST(FMT0,   IOARCID_IOFMT, ARC_RX0TX1_CLEAR & arc);
                    ST(FMT0, FW_IO_IDX_IOFMT, fw_io_idx);
                    ST(FMT0,    RX0TX1_IOFMT, rx0tx1);
                    ST(FMT0,   SERVANT_IOFMT, follower);
                    ST(FMT0, IOCOMMAND_IOFMT, iocommand);
                    ST(FMT0,   EXDTCMD_IOFMT, ext);
                    ST(FMT0,  INSTANCE_IOFMT, inst);
                    ST(FMT0,    DOMAIN_IOFMT, domain);
                
                    FMT1 = settings;

                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X IO [%d] %s\n", FMT0, addrBytes, addrBytes/4, idxio, comments); addrBytes += 4;
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X    \n", FMT1, addrBytes, addrBytes/4); addrBytes += 4;

                    FMT1 = 0;
                }
                break;
            }


            /* ============================================================= */
            /* the list of formats includes the ones selected found in the IO manifests */
            case _format:
            {   uint32_t idxfmt, frameSize, raw, iFS, nchan, timestp, intlv, multiframe, specific;
                uint32_t addrBytes_start;
                float FS;
                
                pt_line++;     // skip the "f" field
                fields_extract(&pt_line, "i", &nFMT);
                addrBytes_start = addrBytes;

                for (idxfmt = 0; idxfmt < nFMT; idxfmt++)
                {
                    fields_extract(&pt_line, "iiifiiiic",  
                        &frameSize, &raw, &nchan, &FS, &timestp, &intlv, &multiframe, &specific, comments);

                    FMT0 = 0;
                    ST(FMT0, MULTIFRME_FMT0, multiframe);
                    ST(FMT0,       RAW_FMT0, raw);
                    ST(FMT0, INTERLEAV_FMT0, intlv);
                    ST(FMT0, FRAMESIZE_FMT0, frameSize);

                    FMT1 = 0;
                    ST(FMT1, NCHANM1_FMT1,      nchan-1 );
                    iFS = quantized_FS (FS);
                    ST(FMT1, SAMPLING_FMT1,     iFS);
                    ST(FMT1, TIMSTAMP_FMT1,     timestp);
                    //iFS = quantized_FS (1.653e-6); // 1 week
                    //iFS = quantized_FS (1.157e-5); // 1 day
                    //iFS = quantized_FS (2.778e-4); // 1 hour

                    FMT2 = 0;
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X Format[%d] %s\n", FMT0, addrBytes, addrBytes/4, idxfmt, comments); addrBytes += 4;
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X \n", FMT1, addrBytes, addrBytes/4); addrBytes += 4;
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X \n", FMT2, addrBytes, addrBytes/4); addrBytes += 4;
                }

                break;
            }

            /* ============================================================= */
            case _script:
            {   uint32_t iscript, p;

                pt_line++;     // skip the "s" field
                fields_extract(&pt_line, "ic", &LENscript, comments);

                for (iscript = 0; iscript < LENscript; iscript++)
                {   fields_extract(&pt_line, "h", &p);
                    PRINTF(p)
                }
                break;
            }

            /* ============================================================= */
            case _linked_list:
            {   uint32_t swcID, TCM, verb, narc, narc_streamed, arc[MAX_NB_STREAM_PER_SWC], iarc, rx0tx1;   
                uint32_t nbmem, m[MAX_NB_MEM_REQ_PER_NODE], imem, off, base;
                uint32_t preset, skip, arch, proc, nparam, nb, accubytes, locking_arc;
                uint32_t iparam, p, nbytes;
                uint8_t *p8;
                
                if (LinkedList0 == 0)
                {   LinkedList0 = addrBytes;
                }
                pt_line++;     // skip the "l" field
                fields_extract(&pt_line, "iiiic",  
                    &swcID, &arch, &proc, &verb, comment1);      

                fields_extract(&pt_line, "iiic",  
                   &narc, &narc_streamed, &locking_arc, comment2);     
                   
                memset(arc, 0, MAX_NB_STREAM_PER_SWC * sizeof(uint32_t));
                for (iarc = 0; iarc < narc; iarc++)
                {   fields_extract(&pt_line, "iic",  &rx0tx1, &(arc[iarc]), comments);  
                    arc[iarc] |= (rx0tx1 << ARC0_LW1_MSB);  // bit10 is the rx/tx direction
                }

                /* forced maximum value of ARCs = 4 */
                if (narc >= MAX_NB_STREAM_PER_SWC)
                {   fprintf(stderr, "\n TOO MUCH ARCS \n");
                }

                narc = MIN(narc, MAX_NB_STREAM_PER_SWC);

                fields_extract(&pt_line, "iic",  
                   &nbmem, &TCM, comment3);           
                   
                for (imem = 0; imem < nbmem; imem++)
                {   fields_extract(&pt_line, "ihc",  &off, &base, comments);  
                    FMT0 = 0;
                    ST(FMT0, DATAOFF_ARCW0, off);
                    ST(FMT0, BASEIDX_ARCW0, base);
                    m[imem] = FMT0;
                }

                /* parameters */
                fields_extract(&pt_line, "iiiic",  
                    &preset, &skip,  &nparam, &nbytes, comment3);

                //ST(linked_list_27b, BASEIDX_ARCW0, addrBytes); /* patch the base address, keep the memory bank */

        /* word 1 - main Header */
                FMT0 = 0;
                ST(FMT0,  PROCID_LW0, proc);
                ST(FMT0,  ARCHID_LW0, arch);
                ST(FMT0,  NBARCW_LW0, narc);
                ST(FMT0, ARCLOCK_LW0, locking_arc);
                ST(FMT0, ARCSRDY_LW0, narc_streamed);
                ST(FMT0, SWC_IDX_LW0, swcID);
                fprintf (ptf_graph_bin, "0x%08X, // %03X -------%s\n", FMT0, addrBytes, comment1); addrBytes += 4;

        /* word 2 - arcs */
                for (iarc = 0; iarc < narc; iarc+=2)
                {
                    FMT0 = 0;
                    ST(FMT0, ARC1_LW1, arc[iarc+1]);
                    ST(FMT0, ARC0_LW1, arc[iarc]);
                    fprintf (ptf_graph_bin, "0x%08X, // %03X        %s \n", FMT0, addrBytes, comment2); addrBytes += 4;
                }

        /* word 3 - memory banks */
                for (imem = 0; imem < nbmem; imem++) 
                {
                    memory_banks[nb_memory_banks++] = (swcID<<28) | m[imem];      // for debug

                    FMT0 = 0;
                    if (imem == 0) ST(FMT0, NBALLOC_LW2,  nbmem);  // instance pointer has the number of memory banks on MSBs
                    ST(FMT0, BASEIDXOFFLW2, m[imem]);

                    if (imem == 0)
                    {   ST(FMT0, TCM_INST_LW2, TCM);
                        fprintf (ptf_graph_bin, "0x%08X, // %03X %03X SWC MEM %d %s\n", FMT0, addrBytes, addrBytes/4, imem, comment3); addrBytes += 4;
                    }
                    else
                    {   fprintf (ptf_graph_bin, "0x%08X, // %03X %03X SWC MEM %d \n", FMT0, addrBytes, addrBytes/4, imem); addrBytes += 4;
                    }
                }

        /* word 4 - parameters */
                FMT0 = 0;
                ST(FMT0, SELPARAM_LW3,  0); // @@@ all parameters only

                //{   int i, f, s; float fs = 1; s = 1;
                //    for (i = 0; i < 2e6; i+=s)
                //    {   fs = fs * 1.1; s = (int)fs; f = INTTOFP844(i);
                //        printf("%9d %3X [%3d %3d] err = %9d\n", i, f, f>>6, f&0x3F, ((f&0x3F) << (f>>6))-i);
                //    } }

                //FPskip = INTTOFPE4M6(skip);
                //skip = FPE4M6TOINT(FPskip);      // parameter size is increased by ~2%

                ST(FMT0, W32LENGTH_LW3, skip);
                ST(FMT0,   VERBOSE_LW3, verb);
                ST(FMT0,  NEWPARAM_LW3, 0);
                ST(FMT0,    PRESET_LW3, preset);

                PRINTF(FMT0)

                parameters[0] = 0;
                p8 = (uint8_t *)parameters;

#define NIBBLE3_MSB U(31)
#define NIBBLE3_LSB U(24) 
#define NIBBLE2_MSB U(23)
#define NIBBLE2_LSB U(16) 
#define NIBBLE1_MSB U(15)
#define NIBBLE1_LSB U( 8) 
#define NIBBLE0_MSB U( 7)
#define NIBBLE0_LSB U( 0) 

                /* 
                    @@@ TODO @@@ 
                    allow setting of few parameters instead of 0 or all
                */

                accubytes = 0;
                while (nbytes > 0)
                {   fields_extract(&pt_line, "hic", &p, &nb, comments);
                    if (nb == 1)
                    {   *p8++ = RD(p, NIBBLE0);
                    }
                    if (nb == 2)
                    {   *p8++ = RD(p, NIBBLE0); *p8++ = RD(p, NIBBLE1);
                    }
                    if (nb == 4)
                    {   *p8++ = RD(p, NIBBLE0); *p8++ = RD(p, NIBBLE1);
                        *p8++ = RD(p, NIBBLE2); *p8++ = RD(p, NIBBLE3);
                    }
                    nbytes = nbytes -nb;
                    accubytes = accubytes + nb;
                }

                for (iparam = 0; iparam < skip; iparam++)
                {   PRINTF(parameters[iparam])
                }
                break;
             }


            /* ============================================================= */
            case _stream_inst:
            {   uint32_t idxinst, iomask, archID, procID, instanceID, traceArc;
                uint32_t last_word;
                
                /* === END OF LINKED_LIST ===*/
                LinkedList = (addrBytes - LinkedList0 + 3)/4;
                last_word = 0;
                ST(last_word, SWC_IDX_LW0, GRAPH_LAST_WORD);
                fprintf(ptf_graph_bin, "0x%08X, // %03X %03X\n", last_word, addrBytes, addrBytes/4); addrBytes += 4;
                /* === ================== ===*/

                pt_line++;     // skip the "i" field
                fields_extract(&pt_line, "i", &NbInstance);
                for (idxinst = 0; idxinst < NbInstance; idxinst++)
                {
                    fields_extract(&pt_line, "iiiiic",  
                        &iomask, &archID, &procID, &instanceID, &traceArc, comments);

                    /* STREAM_INSTANCE_WHOAMI_PORTS */
                    FMT0 = 0;
                    ST(FMT0, INSTANCE_PARCH,    instanceID);
                    ST(FMT0, PROCID_PARCH,      procID);
                    ST(FMT0, ARCHID_PARCH,      archID);
                    ST(FMT0, BOUNDARY_PARCH,    iomask);

                    /* STREAM_INSTANCE_PARAMETERS */
                    FMT1 = 0;
                    ST(FMT1, TRACE_ARC_PARINST, traceArc);

                    /* STREAM_INSTANCE_DYNAMIC */
                    FMT2 = 0;

                    //ST(linked_list_sharedRAM27b, BASEIDX_ARCW0, addrBytes); /* patch the base address, keep the memory bank */

                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X Stream Instance [%d] %s\n", FMT0, addrBytes, addrBytes/4, idxinst, comments); addrBytes += 4;
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X  \n", FMT1, addrBytes, addrBytes/4); addrBytes += 4;
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X  \n", FMT1, addrBytes, addrBytes/4); addrBytes += 4;
                }
                break;
            }
            /* ============================================================= */
            case _arc:
            {   uint32_t size, off, base, prodFMT, consFMT, reg, compute, overflow, underflow, MPflush;
                uint32_t iarc, idxarc;

                pt_line++;     // skip the "a" field
                fields_extract(&pt_line, "ic", &NBarc);

                for (iarc = 0; iarc < NBarc; iarc++)
                {                             
                    fields_extract(&pt_line, "iihhiiiiiiic", 
                        &idxarc, &off, &base, &size, &prodFMT, &consFMT, &reg, &compute, &overflow, &underflow, &MPflush, comments);

                    FMT0 = 0;
                    ST(FMT0, PRODUCFMT_ARCW0, prodFMT);
                    ST(FMT0,   DATAOFF_ARCW0, off);
                    ST(FMT0,   BASEIDX_ARCW0, base);
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X ARC[%d]%s\n", FMT0, addrBytes, addrBytes/4, idxarc, comments); addrBytes += 4;

                    FMT1 = 0;
                    ST(FMT1, CONSUMFMT_ARCW1, consFMT);
                    ST(FMT1,   MPFLUSH_ARCW1, MPflush);
                    ST(FMT1, DEBUG_REG_ARCW1, reg);
                    ST(FMT1, BUFF_SIZE_ARCW1, size);

                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X\n", FMT1, addrBytes, addrBytes/4); addrBytes += 4;

                    FMT2 = 0;
                    ST(FMT2, COMPUTCMD_ARCW2, compute);
                    ST(FMT2, UNDERFLRD_ARCW2, underflow);
                    ST(FMT2,  OVERFLRD_ARCW2, overflow);
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X\n", FMT2, addrBytes, addrBytes/4); addrBytes += 4;

                    FMT0 = 0;
                    fprintf (ptf_graph_bin, "0x%08X, // %03X %03X\n", FMT0, addrBytes, addrBytes/4); addrBytes += 4;
                    
                }
                break;
            };

            /* ============================================================= */
            case _debug:
            {   uint32_t i;
                pt_line++;     // skip the "d" field
                fields_extract(&pt_line, "ic", &SizeDebug);

                for (i = 0; i < SizeDebug; i++)
                {   PRINTF(0)
                }
                break;
            }
            /* ============================================================= */
            case _RAM:
            {   uint32_t idxram, nbram, imem, membank;

                pt_line++;     // skip the "r" field
                fields_extract(&pt_line, "hc", &nbram, comments);

 for (imem = 0; imem < nb_memory_banks; imem++)
 {   
     fprintf (stderr, "\n%8X %8X",memory_banks[imem], RD(memory_banks[imem],BASEIDX_LW2));
 }

                for (idxram = 0; idxram < nbram/4; idxram++)
                {   
                    membank = 0;
                    for (imem = 0; imem < nb_memory_banks; imem++)
                    {   if (RD(memory_banks[imem],BASEIDX_LW2) == addrBytes)
                        {   membank = 0x11110000 | (0Xffff & memory_banks[imem]);
                        }
                    }
                    PRINTF(membank)
                }
                break;
             }

            /* ============================================================= */
            default:
                break;
        }
        jump2next_line(&pt_line);
    }

    fsetpos (ptf_graph_bin, &pos_NWords);
    fprintf (ptf_graph_bin, "0x%08X", addrBytes/4); 
    
    fsetpos (ptf_graph_bin, &pos_PACKFORMATIOSSCRIPT);
    fprintf (ptf_graph_bin, "0x%08X", PACKFORMATIOSSCRIPT(LENscript,nIOs,nFMT));

    fsetpos (ptf_graph_bin, &pos_PACKLINKEDLNBINSTANCE);
    fprintf (ptf_graph_bin, "0x%08X", PACKLINKEDLNBINSTANCE(LinkedList,NbInstance) );

    fsetpos (ptf_graph_bin, &pos_PACKNBARCDEBUG);
    fprintf (ptf_graph_bin, "0x%08X", PACKNBARCDEBUG(SizeDebug,NBarc));
}
