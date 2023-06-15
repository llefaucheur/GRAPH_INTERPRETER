c ------------------------------------------------------------------------------------------------------------
c CMSIS-STREAM graph for stream-based DSP/ML                                                                  
c   processing of installed software components :                                                             
c   FILTER, DETECTOR                                                                                          
c                                                                                                             
c               +----------------------+         +------------------------+                                   
c    ADC        | noise removal filter |         | detector with decision |       GPIO                        
c    +--------->+ IIR notch, 4th order +-------->+ hold for 1000ms        +------>                            
c               | (int16)              |         | (int16)                |                                   
c               +----------------------+         +------------------------+                                   
c                                                                                                             
c ------------------------------------------------------------------------------------------------------------
; HEADER 
o 0 sharedMEM; 0 AllInRam; 
; RAM address (bank,offset)
  1 format ih; h00000; graph RAM start address part 1 : DMEM internal
;
;------------------------BINARY OUTPUT-------------------------
;
;   - size of the graph in bytes
;   [0] 27b RAM address of part/all the graph, HW-shared MEM configuration, which part is going in RAM
;   [1] number of FORMAT, number of IO, size of SCRIPTS
;   [2] size of LINKEDLIST, number of STREAM_INSTANCES
;   [3] number of ARCS, number of DEBUG registers
;
; --------------------------------------------------
h 3 NBIO; 
1 arc; 0 fw_io_idx; 0 rx0tx1; 1 servant; 1 0_set_1_copy; 0 ext; 0 inst; 0 domain; 0 settings; rx from application
3 arc; 1 fw_io_idx; 1 rx0tx1; 1 servant; 1 0_set_1_copy; 0 ext; 0 inst; 0 domain; 0 settings; tx to loudspeaker
5 arc; 2 fw_io_idx; 1 rx0tx1; 1 servant; 1 0_set_1_copy; 0 ext; 0 inst; 0 domain; 0 settings; debug
;
;
; --------------------------------------------------
f 2 FORMATS;
4 frameSize; 17 raw; 1 nchan; 16000 FS; 0 timestp; 0 intlv; 0 multiframe; 0 specific; input format
6 frameSize; 17 raw; 1 nchan; 16000 FS; 0 timestp; 0 intlv; 0 multiframe; 0 specific; output format
;
;
; --------------------------------------------------
s  12;  length of scripts;
    hD091BB44;
    hE7E1FAFA;
    h2082353F;
    hE9D30006;
    hA1E24BA9;
    h18F86871;
    h474BA8C6;
    h8C006D5F;
    hF51F2AFF;
    hF702EF5E;
    h285954B7;
    hF878C4BE;
;
; --------------------------------------------------
l 9 FILTER; 1 arch; 0 proc; 0 verbose                   ; FILTER arc 1->2 with TCM
    2 nb arcs; 2 nb streaming arcs; 1  index of TX arc used for locking; 
    0; 1; RX
    1; 2; TX
;-  memory banks:  index (3bits NB_MEMINST_OFFSET) shift (2bits), base (22bits); check of overlays
    2 nbmem max=6; 1 TCM pointer loaded by scheduler; 
    1; h00250; main instance 
    1; h002A0; TCM address is dynamic
;-  parameter settings - preset:  2 stages [B,A]=cheby2(2,0.5,1/11), dec2hex(round(16384*[B,A]));
    1 preset; 11 skip words; 255 all params; 21 Bytes; 
    h02;    1; numstage
    h5678 ; 2;  b0
    h2E5B ; 2;  b1
    h71DD ; 2;  b2
    h2166 ; 2;  a1
    h70B0 ; 2;  a2   
    h71DD ; 2;  b0
    h2E5B ; 2;  b1
    h71DD ; 2;  b2
    h2166 ; 2;  a1
    hdef0 ; 2;  a2
;-------
l 10 DETECTOR; 1 arch; 0 proc; 0 verbose                ; DETECTOR arc 2->3
    2 nb arcs; 2 nb streaming arcs; 1 index of TX arc used for locking;
    0; 2; RX
    1; 3; TX
;-  memory banks:  index (3bits NB_MEMINST_OFFSET) shift (2bits), base (22bits); check of overlays
    1 nbmem max=6; 0 no TCM;
    1; h002C0; main instance
;-  parameter settings - preset:  2 stages [B,A]=cheby2(2,0.5,1/11), dec2hex(round(16384*[B,A]));
    1 preset; 0 skip words; 0 no params; 0 nbytes total; 
;
; --------------------------------------------------
i 1 INSTANCE;
7 iomask; 1 archID; 0 procID; 1 instanceID; 4 traceArc; single instance #1
;
; --------------------------------------------------
; ARC iiihhiiiiiiic
a 5 arcs;  
    0; 1 membank; h0140 base; h004 size; 0        ; 0        ; 0    ; 0     ; 0      ; 0      ; 0      ; ARC #0
    1; 1 membank; h0150 base; h020 size; 1 prodFMT; 0 consFMT; 0 reg; 0 comp; 0 overf; 0 under; 0 flush; input=>filter
    2; 1 membank; h0170 base; h020 size; 1 prodFMT; 0 consFMT; 0 reg; 0 comp; 0 overf; 0 under; 0 flush; filter=>detector
    3; 1 membank; h0190 base; h020 size; 1 prodFMT; 0 consFMT; 0 reg; 0 comp; 0 overf; 0 under; 0 flush; detect=>output
    4; 1 membank; h01D0 base; h080 size; 1 prodFMT; 0 consFMT; 0 reg; 0 comp; 0 overf; 0 under; 0 flush; debug trace
;
; --------------------------------------------------
; DEBUG registers
d 4;
;
; --------------------------------------------------
; RAM (for debug) in Bytes
r h0200;
;
; --------------------------------------------------
ENDGRAPH;
