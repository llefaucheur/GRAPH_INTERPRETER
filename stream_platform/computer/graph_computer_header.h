//--------------------------------------
//  DATE Fri Sep  5 18:25:38 2025
//  AUTOMATICALLY GENERATED CODES
//  DO NOT MODIFY !
//--------------------------------------
//  Source ../../../stream_platform/computer/graph_computer.txt
//--------------------------------------
                        // GRAPH_PIO_HW to MEMID 0OFFSET 0 PTstatic 0x0000 MAXWorking 0x0000 Nbytes 0028 
                        // GRAPH_PIO_GRAPH to MEMID 0OFFSET 0 PTstatic 0x0028 MAXWorking 0x0000 Nbytes 0020 
                        // GRAPH_SCRIPTS to MEMID 0OFFSET 0 PTstatic 0x0048 MAXWorking 0x0000 Nbytes 0014 

#define arm_stream_filter__0       0x2C // node position in the graph
#define arm_stream_filter__0_arc_0 0x2D // node arc offset in linkedList
#define arm_stream_filter__0_arc_4 0x2D // node arc offset in linkedList

#define sigp_stream_detector__0       0x39 // node position in the graph
#define sigp_stream_detector__0_arc_4 0x3A // node arc offset in linkedList
#define sigp_stream_detector__0_arc_1 0x3A // node arc offset in linkedList

#define arm_stream_script__0       0x44 // node position in the graph
#define arm_stream_script__0_arc_2 0x45 // node arc offset in linkedList
                        // GRAPH_LINKED_LIST to MEMID 0OFFSET 0 PTstatic 0x005C MAXWorking 0x0000 Nbytes 00F0 
                        // GRAPH_ONGOING to MEMID 0OFFSET 0 PTstatic 0x014C MAXWorking 0x0000 Nbytes 0004 
                        // GRAPH_FORMATS to MEMID 0OFFSET 0 PTstatic 0x0150 MAXWorking 0x0000 Nbytes 0020 
                        // GRAPH_ARCS to MEMID 0OFFSET 0 PTstatic 0x0170 MAXWorking 0x0000 Nbytes 00A0 

#define _arc_0      0x71 // arc descriptor position W32 in the graph
                        // iarc 0 OFFSET 0 PTstatic 0x0210 MAXWorking 0x0000 Nbytes 0000 
#define _arc_buf_0  0x210 // arc buffer address
#define _arc_1      0x79 // arc descriptor position W32 in the graph
                        // iarc 1 OFFSET 0 PTstatic 0x0210 MAXWorking 0x0000 Nbytes 0000 
#define _arc_buf_1  0x210 // arc buffer address
                        // arm_stream_script  format 0 w32length 30OFFSET 0 PTstatic 0x0210 MAXWorking 0x0000 Nbytes 00E0 
// arm_stream_script  format 0 w32length 30 
//       nregs+r12 11 x2   stack 6 x2  heap 16h 
#define _arc_buf_2  0x0 // arc buffer address
                        // Script 0 format 0 w32length 5OFFSET 0 PTstatic 0x02F0 MAXWorking 0x0000 Nbytes 0068 
// Script 0 format 0 w32length 5 
//       nregs+r12 7 x2   stack 6 x2  heap 0h 
#define _arc_buf_3  0x0 // arc buffer address
#define _arc_4      0x91 // arc descriptor position W32 in the graph
                        // iarc 4 OFFSET 0 PTstatic 0x0358 MAXWorking 0x0000 Nbytes 0020 
#define _arc_buf_4  0x358 // arc buffer address


                        // inode 0 arm_stream_filter imem 0 Size 24 h18OFFSET 0 PTstatic 0x0378 MAXWorking 0x0000 Nbytes 0018 
#define arm_stream_filter_inode 0 arm_stream_filter imem 0 Size 24 h18 0x378 // node static memory address
                        // inode 0 arm_stream_filter imem 1 Size 78 h4eOFFSET 0 PTstatic 0x0390 MAXWorking 0x0000 Nbytes 004E 
#define arm_stream_filter_inode 0 arm_stream_filter imem 1 Size 78 h4e 0x390 // node static memory address
                        // inode 1 sigp_stream_detector imem 0 Size 54 h36OFFSET 0 PTstatic 0x03E0 MAXWorking 0x0000 Nbytes 0036 
#define sigp_stream_detector_inode 1 sigp_stream_detector imem 0 Size 54 h36 0x3E0 // node static memory address
                        // inode 1 sigp_stream_detector imem 1 Size 30 h1eOFFSET 0 PTstatic 0x0418 MAXWorking 0x0000 Nbytes 001E 
#define sigp_stream_detector_inode 1 sigp_stream_detector imem 1 Size 30 h1e 0x418 // node static memory address
                        // inode 2 arm_stream_script imem 0 Size 32 h20OFFSET 0 PTstatic 0x0438 MAXWorking 0x0000 Nbytes 0020 
#define arm_stream_script_inode 2 arm_stream_script imem 0 Size 32 h20 0x438 // node static memory address
                        // inode 1 imem 2 (Scratch) Size 16  h10OFFSET 0 PTstatic 0x0458 MAXWorking 0x0000 Nbytes 0010 
#define sigp_stream_detector_ node scratch memory address offset 0 index 1112 0x458 //inode 1 imem 2 (Scratch) Size 16  h10
