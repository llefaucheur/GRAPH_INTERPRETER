;----------------------------------------------------------------------------------------------------------------------
; IO MANIFEST TEMPLATE
;----------------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------OPTIONS SYNTAX --------------------------
;   options sets : { index  list } { index  list } 
;
;   when the list has one single element "X", this is the value to consider : {X} <=> {1 X} <=> X
;
;   when index == 0 it means "any", the list can be empty, the default value is not changed from reset   
;
;   when index > 0 the list gives the allowed values the scheduler can select
;       The Index tells the default "value" to take at reset time and to put in the graph 
;           the combination of index give the second word of stream_format_io[]
;        At reset : (*io_func)(STREAM_RESET, (uint8_t *)&stream_format_io_setting, 0);
;        For tuning : (*io_func)(STREAM_SET_IO_CONFIG, (uint8_t *)&stream_format_io_setting, 0);
;        Example 2  5 6 7 8 9    ; index 2, default = 6 (index starts at 1)
;        
;
;   when index < 0 a list of triplets follows to describe a combination of data intervals :  A1 B1 C1  A2 B2 C2 ... 
;       A is starting value, B is the increment step, C is the included maximum value 
;       The absolute index value selects the default value in this range





io_platform_sensor_in_0         ; IO name for the tools
analog_in                       ; domain name, among the list below :  

                                ; domain name               description and examples
                                ;------------               ------------------------
                                ; general                   (a)synchronous sensor , electrical, chemical, color, .. remote data  
                                ; audio_in                  microphone, line-in, I2S, PDM RX  
                                ; audio_out                 line-out, earphone / speaker, PDM TX, I2S,  
                                ; gpio_in                   generic digital IO , control of relay,  
                                ; gpio_out                  generic digital IO , control of relay,  
                                ; motion                    accelerometer, combined or not with pressure and gyroscope  
                                ; 2d_in                     camera sensor  
                                ; 2d_out                    display, led matrix,  
                                ; analog_in                 with aging control  
                                ; analog_out                D/A, position piezzo, PWM converter   
                                ; rtc                       ticks sent from a programmable timer  
                                ; user_interface_in         button, slider, rotary button  
                                ; user_interface_out        LED, digits, display,  
                                ; platform_x                platform-specific #x, decoded with callbacks                               

                                ; ---------------------------------------------------------------------------------
                                ;  General information of the digital frame format 


io_commander0_servant1  1       ;  commander=0 servant=1 (default is servant)
                                ;  IO stream are managed from the graph scheduler with the help of one subroutine per IO using 
                                ;    the template : typedef void (*p_io_function_ctrl) (uint32_t command, uint8_t *data, uint32_t length);  
                                ;    The "command" parameter can be : STREAM_SET_PARAMETER, STREAM_DATA_START, STREAM_STOP, STREAM_SET_BUFFER.
                                ;  
                                ;  When the IO is "Commander" it calls arm_graph_interpreter_io_ack() when data is read
                                ;  When the IO is "Servant" the scheduler call p_io_function_ctrl(STREAM_RUN, ..) to ask for 
                                ;   data move. Once the move is done the IO driver calls arm_graph_interpreter_io_ack()


io_buffer_allocation    2.0     ;  default is 0, which means the buffer is declared outside of the graph
                                ;  The floating-point number is a multiplication factor of the frame size (here 2 frames), 
                                ;   the buffer size is computed with rounding (n = floor(X+0.5))
                                ;  
                                ;  When more than one byte are exchanged, the IO driver needs a temporary buffer. This buffer
                                ;   can be allocated "outside(0)" by the IO driver, or ">1" during the graph memory mapping preparation
                                ;   The memora mapping of this allocation is decided in the graph and can be in general-purpose or 
                                ;   any RAM "0" or specific memory bank for speed reason or reserved for DMA processing, etc ..

io_set0_copy1      1            ; 

io_direction_rx0tx1   1         ; direction of the stream  0:input 1:output from graph point of view
io_raw_format      S16          ; options for the raw arithmetics computation format here  STREAM_S16 
io_interleaving    1            ; multichannel intleaved (0), deinterleaved by frame-size (1) 
io_nb_channels     1            ; options for the number of channels 
io_frame_length   {1 1 2 16 }   ; options of possible frame_size in number of sample (can mono or multi-channel). 
io_frame_duration {1 10 22.5}   ; options of possible frame_size in [milliseconds].     The default frame length is 1 sample
io_subtype_units  VRMS          ; depending on the domain. Here Units_Vrms of the "general" domain (0 = any or underfined)
io_subtype_multiple {DPS GAUSS} ; example of multi domain : motion can have up to 4 data units for accelerometer, gyroscope, magnetometer, temperature
io_power_mode     0             ; to set the device at boot time in stop / off (0)
                                ; running mode(1) : digital conversion (BIAS always powered for analog peripherals )
                                ; running mode(2) : digital conversion BIAS shut-down between conversions
                                ; Sleep (3) Bias still powered but not digital conversions
io_position meter 1.1 -2.2 0.01 ; unit and relative XYZ position with the platform reference point
io_euler_angles  10 20 90       ; Euler angles with respect to the platform reference orientation, in degrees
io_sampling_rate       {1 16000 44100 48000} ; sampling rate options (enumeration in Hz)  
io_sampling_period_s   {1 1 60 120 }         ; sampling period options (enumeration in [second])   
io_sampling_period_day {1 0.25 1 7 }         ; sampling period options (enumeration in [day])   
io_sampling_rate_accuracy       0.1             ; in percentage

io_time_stamp_format {1 1}      ; 0 no time-stamp, 1 absolute time, 2 relative time from last frame, 3 frame counter
io_time_stamp_length {1 1}      ; 0/1/2/3 corresponding to  16/32/64/64 bits time formats (default : STREAM_TIME32)


;--------------------------------------------------------------------------COMMENTS SECTION --------------------------
; INFORMATION :
;   jumpers to set on the board
;   manufacturer references and internet URLs
;   any other system integration warning and recommendations
;


;--------------------------------------------------------------------------DOMAIN-SPECIFIC OPTIONS --------------------
; general   domain is using subtype_units "stream_unit_physical"

io_analog_scale 0.55           ; 0.55V is corresponding to full-scale (0x7FFF or 1.0f) with the default setting


;----------------------------------------------------------------------------------------------------------------------
; audio_in  domain is using subtype_units "stream_unit_physical"

io_subtype_units    DBSPL       ; STREAM_SUBT_ANA_DBSPL of the domain
io_analog_scale     90.1        ; 90.1 dBSPL is corresponding to full-scale with the default setting

                                ;------------------------------------------------------------------------------------------------------
                                ; audio_in  domain SPECIFIC (digital format) 
                                ; Samples = Full_Scale_Digital x (interpa x analog_input + interpoff)
                                ; analog_input = invinterpa x ((samples/Full_Scale_Digital) - interpoff)
io_rescale_factor   12.24  -44.3 ; [1/a off] analog_input = invinterpa x ((samples/Full_Scale_Digital) - interpoff)

io_sampling_rate       {1 16000 44100 48000} ; sampling rate options (enumeration in Hz)  
io_sampling_period_s   {1 1 60 120 }         ; sampling period options (enumeration in [second])   
io_sampling_period_day {1 0.25 1 7 }         ; sampling period options (enumeration in [day])   

io_nb_channels          {1 1 2} ; options for the number of channels 
io_channel_mapping  1           ; mono (Front Left), 18 channels can be controlled :
                                ;   Front Left                FL     bit0
                                ;   Front Right               FR        1
                                ;   Front Center              FC        2
                                ;   Low Frequency             LFE       3
                                ;   Back Left                 BL        4
                                ;   Back Right                BR        5
                                ;   Front Left of Center      FLC       6
                                ;   Front Right of Center     FRC       7
                                ;   Back Center               BC        8
                                ;   Side Left                 SL        9
                                ;   Side Right                SR        10
                                ;   Top Center                TC        11
                                ;   Front Left Height         TFL       12
                                ;   Front Center Height       TFC       13
                                ;   Front Right Height        TFR       14
                                ;   Rear Left Height          TBL       15
                                ;   Rear Center Height        TBC       16
                                ;   Rear Right Height         TBR       17

    subtype_units  104


io_analog_gain     {1  0 12 24       }   ; analog gain (PGA)
io_digital_gain    {-1 -12 1 12      }   ; digital gain range
io_hp_filter       {1 1 20 50 300    }   ; high-pass filter (DC blocker) ON(1)/OFF(0) followed by cut-off frequency options
io_agc              0                    ; agc automatic gain control, ON(1)/OFF(0) 
io_router          {1  0 1 2 3       }   ; router  from AMIC0 DMIC1 HS2 LINE3 BT/FM4 LOOP/TEST5
io_gbass_filter    {1  1  1  0 -3 3 6}   ; ON(1)/OFF(0) options for gains in dB
io_fbass_filter    {1  20 100 200    }   ; options for frequencies
io_gmid_filter     {1  1  1  0 -3 3 6}   ; ON(1)/OFF(0) options for gains in dB
io_fmid_filter     {1  500 1000      }   ; options for frequencies
io_ghigh_filter    {1  1  0 -3 3 6   }   ; ON(1)/OFF(0) options for gains in dB 
io_fhigh_filter    {1  4000 8000     }   ; options for frequencies

;----------------------------------------------------------------------------------------------------------------------
; audio_out          

io_subtype_units   87                    ; Units is [mV] 
io_analog_scale    1400                  ; 1400nV is corresponding to full-scale with the default setting
io_sampling_rate   {1 16000 44100 48000} ; sampling rate options (enumeration in Hz)  
io_nb_channels     {   1    1 1 2    }      ; multichannel intleaved (0), deinterleaved by frame-size (1) + options for the number of channels 
io_channel_mapping  1                    ; mono (Front Left), 18 channels can be controlled :
io_router          {1  0 1 2 3       }   ; router  from AMIC0 DMIC1 HS2 LINE3 BT/FM4 LOOP/TEST5
io_analog_gain     {1  0 12 24       }   ; analog gain (Class-D)
io_digital_gain    {-1 -12 1 12      }   ; digital gain range
io_hp_filter       {1 1 20 50 300    }   ; high-pass filter (DC blocker) ON(1)/OFF(0) followed by cut-off frequency options
io_agc              0                    ; agc automatic gain control, ON(1)/OFF(0) 
io_gbass_filter    {1  1  1  0 -3 3 6}   ; ON(1)/OFF(0) options for gains in dB
io_fbass_filter    {1  20 100 200    }   ; options for frequencies
io_gmid_filter     {1  1  1  0 -3 3 6}   ; ON(1)/OFF(0) options for gains in dB
io_fmid_filter     {1  500 1000      }   ; options for frequencies
io_ghigh_filter    {1  1  0 -3 3 6   }   ; ON(1)/OFF(0) options for gains in dB 
io_fhigh_filter    {1  4000 8000     }   ; options for frequencies


;----------------------------------------------------------------------------------------------------------------------
; gpio_in            
    0                       ; time-stamp (none)
    0   5                   ; samp. rate format (1:Hz 2:s 3:days) accuracy in % 
    1   1 10 20             ; options : sampling rate (in Hz, index <0 for a range [min, step, max])  

;----------------------------------------------------------------------------------------------------------------------
; gpio_out           

    0                       ; time-stamp (none)

;----------------------------------------------------------------------------------------------------------------------
; motion             
io_sampling_rate        1 16000 44100 48000   ; sampling rate options (enumeration in Hz)  
io_sampling_period_s    1 0.01 0.02 0.04 ; sampling period options (enumeration in [second])   

io_sampling_accuracy    0.8             ; sampling rate accuracy in percent
    
                                        ; subtype_motion aXg0m0 1                            /* only accelerometer */
                                        ; subtype_motion a0gXm0 2                            /* only gyroscope */
                                        ; subtype_motion a0g0mX 3                            /* only magnetometer */
                                        ; subtype_motion aXgXm0 4                            /* A + G */
                                        ; subtype_motion aXg0mX 5                            /* A + M */
                                        ; subtype_motion a0gXmX 6                            /* G + M */
                                        ; subtype_motion aXgXmX 7                            /* A + G + M */
io_motion_format   4                    ; imu_channel_format

io_motion_sensitivity acc   1  2 4 8 16 ; sensitivity options of accelerometer 
io_motion_sensitivity gyro  1  2 4 8 16 ; sensitivity options of gyroscope [dps]
io_motion_sensitivity mag   1  2 4 8 16 ; sensitivity options of magnetometer

io_motion_averaging   acc   1  1 4 16 32; averaging in nb of samples
io_motion_averaging   gyro  1  1 4 16 32; averaging in nb of samples

io_high_pass   0 /1                     ; remove clicks 
io_DC-canceller

;----------------------------------------------------------------------------------------------------------------------
; 2d_in              

io_raw_format_2d           (U16 + RGB16) (U8 + Grey) (U8 + YUV422)  
io_trigger flash
io_synchronize with IR transmitter    https://developer.android.com/reference/android/hardware/HardwareBuffer
io_frame rate per second
io_exposure time           The amount of time the photosensor is capturing light, in seconds.
io_image size 
io_modes                   portrait, landscape, barcode, night modes
io_Gain	                Amplification factor applied to the captured light. 1.0 is the default gain; more than 1.0 is brighter; less than 1.0 is darker.
io_WhiteBalanceColorTemp	Temperature parameter when using the regular HDRP color balancing.
io_WhiteBalanceColorTint	Tint parameter when using the regular HDRP color balancing.
io_MosaicPattern	        Color Filter Array pattern for the colors.
io_WhiteBalanceRGBCoefficients	Custom RGB scaling values for white balance, used only if EnableWhiteBalanceRGBCoefficients is selected.
io_EnableWhiteBalanceRGBCoefficients	Enable using custom RGB scaling values for white balance instead of temperature and tint.
io_Auto White Balance	    Assumes the camera is looking at a white reference, and calibrates the WhiteBalanceRGBCoefficients. Refer to the API for more details.
io_time-stamp (none)
io_wdr;                    wide dynamic range flag (tuya)
io_watermark;              watermark insertion flag (tuya)
io_flip;                   image format (portrait, panoramic)
io_night_mode;             motion detection sensitivity (low, medium, high)
io_detection_zones;        + {center pixel (in %) radius}, {}, {}
io_focus_area
io_auto exposure on focus area
io_focus_distance          forced focus to infinity or xxx meters
io_get_distance            from focus area
io_zoom_area
io_time_stamp;             detection time-stamp format 
io_light_detection;        
io_jpeg_quality  
io_sound_detection;        sound level 
io_other sensors;          humidity, battery%

    /* 6b Focus control and white balance  
        2   0.05 0.0 -0.05      ; relative position to the center of the light reference
        2   0.05 0.0 -0.05      ; relative position to the center of the lens focus point
    */

            

;----------------------------------------------------------------------------------------------------------------------
; 2d_out             
    8b backlight brightness control


    ;----------------------------------------------------------------------------------------------------------------------
    ; analog_in          
    0                       ; time-stamp (none)
    0   5                   ; samp. rate format (1:Hz 2:s 3:days) accuracy in % 
    1   1 10 20             ; options : sampling rate (in Hz, index <0 for a range [min, step, max])  

    /* Environmental Sensing Service, Bluetooth Service Specification
      0x00: Unspecified 0x01: Air 0x02: Water 0x03: Barometric 0x04: Soil 0x05: Infrared 
      0x06: Map Database 0x07: Barometric Elevation Source 0x08: GPS only Elevation Source 
      0x09: GPS and Map database Elevation Source 0x0A: Vertical datum Elevation Source 
      0x0B: Onshore 0x0C: Onboard vessel or vehicle 0x0D: Front 0x0E: Back/Rear 0x0F: Upper 
      0x10: Lower 0x11: Primary 0x12: Secondary 0x13: Outdoor 0x14: Indoor 0x15: Top 0x16: Bottom 
      0x17: Main 0x18: Backup 0x19: Auxiliary 0x1A: Supplementary 0x1B: Inside 0x1C: Outside 
      0x1D: Left 0x1E: Right 0x1F: Internal 0x20: External 0x21: Solar 0x22-0xFF:Reserved for future use

    ; Physical_Unit = (A x Raw_input_sample + B)
    ;    Full_Scale_Digital = 1.0 / 32767 for samples in fp32 / q15 
    ;

    17                      ; rfc8428_V unit = volt (PEAK)
    3   0.9 0.5 0.11        ; A options to analog Volt input
    3   0.2 0.1 0.0         ; B options 


;----------------------------------------------------------------------------------------------------------------------
; analog_out         

    0                       ; time-stamp (none)
;----------------------------------------------------------------------------------------------------------------------
; rtc                
    0                       ; time-stamp (none)
;----------------------------------------------------------------------------------------------------------------------
; user_interface_in  
    0                       ; time-stamp (none)
;----------------------------------------------------------------------------------------------------------------------
; user_interface_out 
    0                       ; time-stamp (none)
     


    /*================================ STREAM ARITHMETICS DATA/TYPE ====================================================*/
    /* types fit in 6bits, arrays start with 0, stream_bitsize_of_raw() is identical */

    enum stream_raw_data 
    {
    #define STREAM_DATA_ARRAY 0 /* see stream_array: [0NNNTT00] 0, type, nb */
    #define STREAM_S1         1 /* S, one signed bit, "0" = +1 */                           /* one bit per data */
    #define STREAM_U1         2 /* one bit unsigned, boolean */
    #define STREAM_S2         3 /* SX  */                                                   /* two bits per data */
    #define STREAM_U2         4 /* XX  */
    #define STREAM_Q1         5 /* Sx ~stream_s2 with saturation management*/
    #define STREAM_S4         6 /* Sxxx  */                                                 /* four bits per data */
    #define STREAM_U4         7 /* xxxx  */
    #define STREAM_Q3         8 /* Sxxx  */
    #define STREAM_FP4_E2M1   9 /* Seem  micro-float [8 .. 64] */
    #define STREAM_FP4_E3M0  10 /* Seee  [8 .. 512] */
    #define STREAM_S8        11 /* Sxxxxxxx  */                                             /* eight bits per data */
    #define STREAM_U8        12 /* xxxxxxxx  ASCII char, numbers.. */
    #define STREAM_Q7        13 /* Sxxxxxxx  arithmetic saturation */
    #define STREAM_CHAR      14 /* xxxxxxxx  */
    #define STREAM_FP8_E4M3  15 /* Seeeemmm  NV tiny-float [0.02 .. 448] */                 
    #define STREAM_FP8_E5M2  16 /* Seeeeemm  IEEE-754 [0.0001 .. 57344] */
    #define STREAM_S16       17 /* Sxxxxxxx.xxxxxxxx  */                                    /* 2 bytes per data */
    #define STREAM_U16       18 /* xxxxxxxx.xxxxxxxx  Numbers, UTF-16 characters */
    #define STREAM_Q15       19 /* Sxxxxxxx.xxxxxxxx  arithmetic saturation */
    #define STREAM_FP16      20 /* Seeeeemm.mmmmmmmm  half-precision float */
    #define STREAM_BF16      21 /* Seeeeeee.mmmmmmmm  bfloat */
    #define STREAM_Q23       22 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx  24bits */                    /* 3 bytes per data */ 
    #define STREAM_Q23_32    23 /* SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxxx  */                  /* 4 bytes per data */
    #define STREAM_S32       24 /* one long word  */
    #define STREAM_U32       25 /* xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx UTF-32, .. */
    #define STREAM_Q31       26 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx  */
    #define STREAM_FP32      27 /* Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm  FP32 */             
    #define STREAM_CQ15      28 /* Sxxxxxxx.xxxxxxxx Sxxxxxxx.xxxxxxxx (I Q) */             
    #define STREAM_CFP16     29 /* Seeeeemm.mmmmmmmm Seeeeemm.mmmmmmmm (I Q) */             
    #define STREAM_S64       30 /* long long */                                             /* 8 bytes per data */
    #define STREAM_U64       31 /* unsigned  64 bits */
    #define STREAM_Q63       32 /* Sxxxxxxx.xxxxxx ....... xxxxx.xxxxxxxx  */
    #define STREAM_CQ31      33 /* Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxx..*/
    #define STREAM_FP64      34 /* Seeeeeee.eeemmmmm.mmmmmmm ... double  */
    #define STREAM_CFP32     35 /* Seeeeeee.mmmmmmmm.mmmmmmmm.mmmmmmmm Seee.. (I Q)  */
    #define STREAM_FP128     36 /* Seeeeeee.eeeeeeee.mmmmmmm ... quadruple precision */     /* 16 bytes per data */
    #define STREAM_CFP64     37 /* fp64 fp64 (I Q)  */
    #define STREAM_FP256     38 /* Seeeeeee.eeeeeeee.eeeeemm ... octuple precision  */      /* 32 bytes per data */
    #define STREAM_TIME16    39 /* ssssssssssssqqqq q14.2   1 hour + 8mn +/- 0.0625 */
    #define STREAM_TIME16D   40 /* qqqqqqqqqqqqqqqq q15 [s] time difference +/- 15us */
    #define STREAM_TIME32    41 /* ssssssssssssssssssssssssssssqqqq q28.4  [s] (8.5 years +/- 0.0625s) */ 
    #define STREAM_TIME32D   42 /* ssssssssssssssssqqqqqqqqqqqqqqqq q17.15 [s] (36h, +/- 30us) time difference */   
    #define STREAM_TIMESTMP  43 /* ssssssssssssssssssssqqqqqqqqqqqq q20.12 [s] (12 days, +/- 0.25ms) */   
    #define STREAM_TIME64    44 /* ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s] 140 Y +Q28 [s] */   
    #define STREAM_TIME64MS  45 /* ______________________mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm ms */   
    #define STREAM_TIME64ISO 46 /* ___..YY..YY..YY..YY..MM..MM..DD..DD..SS..SS.....offs..MM..MM..MM ISO8601 signed offset 2024-05-04T21:12:02+07:00  */   
    #define STREAM_WGS84     47 /* <--LATITUDE 32B--><--LONGITUDE 32B-->  lat="52.518611" 0x4252130f   lon="13.376111" 0x4156048d - dual IEEE754 */   
    #define STREAM_HEXBINARY 48 /* UTF-8 lower case hexadecimal byte stream */
    #define STREAM_BASE64    49 /* RFC-2045 base64 for xsd:base64Binary XML data */
    #define STREAM_STRING8   50 /* UTF-8 string of char terminated by 0 */
    #define STREAM_STRING16  51 /* UTF-16 string of char terminated by 0 */

    #define LAST_RAW_TYPE    64 /* coded on 6bits RAW_FMT0_LSB */
    };

/* ==========================================================================================

    IO_DOMAIN physical types and tuning

   ==========================================================================================
*/

    /* IO_DOMAIN_DATA_IN           : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
        #define STREAM_SUBT_GENERAL_COMP195X 0 /* compressed byte stream following RFC1950 / RFC1951 ("deflate") */
        #define STREAM_SUBT_GENERAL_DPCM     1 /* compressed byte stream */
        #define STREAM_SUBT_GENERAL_JSON     2 /* JSON */
        #define STREAM_SUBT_GENERAL_XFORMAT  3 /* SensorThings MultiDatastream extension */

    /* IO_DOMAIN_DATA_OUT          : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
    /* IO_DOMAIN_AUDIO_IN          : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */

        #define STREAM_SUBT_AUDIO_IN 0      /* no subtype_units : integer/ADC format  */
        #define STREAM_SUBT_AUDIO_MPG  0    /* compressed byte stream */

        /* tuning of PLATFORM_AUDIO_IN */
            //const float audio_settings [] = { 
            ///* [stream_units_physical_t, scaling to full-scale] */  
            //unit_pascal, 1.0f,
            ///* nb of bits */
            ///* 3  nchan */              3,   1, 2, 8,  
            ///* 4  FS */                 2,   16000.0f, 48000.0f, 
            ///* 2  framesize [s] */      2,   0.01f, 0.016f, 
            ///*    1 option = "fixed in HW and this is the setting" */
            ///*    0 option = "fixed in HW, and it depends" */
            ///* 4  PGA gain + digital gain option [dB]*/  0,          
            ///* 2  hpf set at 20Hz */    1,   20.0f,
            ///* 1  agc option */         0,
            ///* 2  router to MIC, LINE, BT, LOOP */ 0,          
            ///*14 bits remains */    
            //IO_REC_STREAM_SETTING
            // _UNUSED_MSB 31
            // _UNUSED_LSB 16
            //     AGC_MSB 15 
            //     AGC_LSB 15 /* O6 AGC on/off */
            //     HPF_MSB 14 
            //     HPF_LSB 14 /* O5 HPF on/off */
            //     PGA_MSB 13 
            //     PGA_LSB 10 /* O4 16 analog PGA + digital PGA settings [dB] */
            //   DBSPL_MSB  9 
            //   DBSPL_LSB  9 /* O3 2 max dBSPL options */
            // FRMSIZE_MSB  8 
            // FRMSIZE_LSB  7 /* O2 4 frame size options, in seconds */
            //      FS_MSB  6 
            //      FS_LSB  3 /* O1 16 sampling rates options  */
            //   NCHAN_MSB  2 
            //   NCHAN_LSB  0 /* O0 8 nchan options  */


    /* IO_DOMAIN_AUDIO_OUT         : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
        /* tuning of PLATFORM_AUDIO_OUT */

        #define STREAM_SUBT_AUDIO_OUT 0     /* no subtype_units : integer/DAC format  */

            //const int32_t audio_out_settings [] = { 
            ///* nb options nbbits */
            ///*  8  3  nchan */         3,   1, 2, 8,
            ///* 16  4  FS */            2,   16000, 48000, 
            ///*  4  2  framesize [ms] */2,   10, 16, 
            ///*  8  3  mVrms max */     2,   100, 700,
            ///* 16  4  PGA gain */      0,
            ///*  4  2  bass gain dB */  4,   0, -3, 3, 6,
            ///*  2  1  bass frequency */2,   80, 200,       
            ///*  4  2  mid gain */      4,   0, -3, 3, 6,
            ///*  2  1  mid frequency */ 2,   500, 2000,       
            ///*  4  2  high gain */     4,   0, -3, 3, 6,
            ///*  2  1  high frequency */2,   4000, 8000,       
            ///*  2  1  agc gain */      0,
            ///*  4  2  router */        0,  /* SPK, EAR, LINE, BT */
            ///*     2 bits remains */ 

    /* IO_DOMAIN_GPIO_IN           : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
    /* IO_DOMAIN_GPIO_OUT          : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */

            #define STREAM_SUBT_GPIO_IN 0      /* no subtype_units  */
            #define STREAM_SUBT_GPIO_OUT 0     /* no subtype_units  */


        /* tuning of PLATFORM_GPIO_IN / OUT */
            //const int32_tgpio_out_0_settings [] = {    
            /* nb options nbbits */
            //State : High-Z, low, high, duration, frequency
            //type : PWM, motor control, GPIO
            //PWM duty, duration, frequency (buzzer)
            //Servo motor control -120 .. +120 deg
            //keep the servo position

    /* IO_DOMAIN_MOTION_IN         : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
            #define STREAM_SUBT_MOTION_A     1
            #define STREAM_SUBT_MOTION_G     2
            #define STREAM_SUBT_MOTION_B     3
            #define STREAM_SUBT_MOTION_AG    4
            #define STREAM_SUBT_MOTION_AB    5
            #define STREAM_SUBT_MOTION_GB    6
            #define STREAM_SUBT_MOTION_AGB   7

    /* IO_DOMAIN_2D_IN             : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
    /* IO_DOMAIN_2D_OUT            : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
            /*
            ;------------------------------------------------------------------------------------------------------
            ; 2d_in SPECIFIC (mixed signal setting) 
            2   0.05 0.0 -0.05      ; relative position to the center of the light reference
            2   0.05 0.0 -0.05      ; relative position to the center of the lens focus point
            */
            /*======================================= 2D DOMAINS  */

            #define STREAM_SUBT_YUV420P     1  /* Luminance, Blue projection, Red projection, 6 bytes per 4 pixels, reordered */
            #define STREAM_SUBT_YUV422P     2  /* 8 bytes per 4 pixels, or 16bpp, Y0 Cb Y1 Cr (1 Cr & Cb sample per 2x1 Y samples) */
            #define STREAM_SUBT_YUV444P     3  /* 12 bytes per 4 pixels, or 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
            #define STREAM_SUBT_CYM24       4  /* cyan yellow magenta */
            #define STREAM_SUBT_CYMK32      5  /* cyan yellow magenta black */
            #define STREAM_SUBT_RGB8        6  /* RGB  3:3:2,  8bpp, (msb)2B 3G 3R(lsb) */
            #define STREAM_SUBT_RGB16       7  /* RGB  5:6:5, 16bpp, (msb)5R 6G 5B(lsb) */
            #define STREAM_SUBT_RGBA16      8  /* RGBA 4:4:4:4 32bpp (msb)4R */
            #define STREAM_SUBT_RGB24       9  /* BBGGRR 24bpp (msb)8B */
            #define STREAM_SUBT_RGBA32     10  /* BBGGRRAA 32bpp (msb)8B */
            #define STREAM_SUBT_RGBA8888   11  /* AABBRRGG OpenGL/PNG format R=lsb A=MSB ("ABGR32" little endian) */
            #define STREAM_SUBT_BW1B       12  /* Y, 1bpp, 0 is black, 1 is white */
            #define STREAM_SUBT_GREY2B     13  /* Y, 2bpp, 0 is black, 3 is white, ordered from lsb to msb  */
            #define STREAM_SUBT_GREY4B     14  /* Y, 4bpp, 0 is black, 15 is white, ordered from lsb to msb */
            #define STREAM_SUBT_GREY8B     15  /* Grey 8b, 0 is black, 255 is white */


    /* IO_DOMAIN_ANALOG_IN  */ 
            sensitivity / PGA
            detection threshold
            detection attack time
            detection release time
            aging control 

    /* IO_DOMAIN_ANALOG_OUT */ 
            // aging option (usage counter, time)
            // 

            // Physical_Unit = (A x Raw_input_sample + B)
            //    Full_Scale_Digital = 1.0 / 32767 for samples in fp32 / q15 
            // 17                      ; rfc8428_V unit = volt (PEAK)
            // 3   0.9 0.5 0.11        ; A options to analog Volt input
            // 3   0.2 0.1 0.0         ; B options 
            // aging option (usage counter, time)

            //                    /* int16_t format */            /* int32_t format */           
            //unit_linear,        /* PCM and default format */    /* PCM and default format */
            //unit_decibel,       /* Q11.4 :   1dB <> 0x0010      Q19.12 :   1dB <> 0x0000 1000  */
            //unit_percentage,    /* Q11.4 :   1 % <> 0x0010      Q19.12 :   1 % <> 0x0000 1000 */
            //unit_meter,         /* Q11.4 :  10 m <> 0x00A0      Q19.12 :  10 m <> 0x0000 A000 */
            //unit_g = 20,        /* Q11.4 :   1g <> 0x0010       Q19.12 :   1g <> 0x0000 1000  */   
            /*======================================= ANALOG SENDOR DOMAINS SUBTYPE_FMT1 */
            //enum stream_unit_physical
            //{   
                                        // RFC8428 Symbol  Description
            #define STREAM_SUBT_ANA_ANY             0 /*        any                        */        
            #define STREAM_SUBT_ANA_METER           1 /* m         meter                   */
            #define STREAM_SUBT_ANA_KGRAM           2 /* kg        kilogram                */
            #define STREAM_SUBT_ANA_GRAM            3 /* g         gram*                   */
            #define STREAM_SUBT_ANA_SECOND          4 /* s         second                  */
            #define STREAM_SUBT_ANA_AMPERE          5 /* A         ampere                  */
            #define STREAM_SUBT_ANA_KELVIB          6 /* K         kelvin                  */
            #define STREAM_SUBT_ANA_CANDELA         7 /* cd        candela                 */
            #define STREAM_SUBT_ANA_MOLE            8 /* mol       mole                    */
            #define STREAM_SUBT_ANA_HERTZ           9 /* Hz        hertz                   */
            #define STREAM_SUBT_ANA_RADIAN         10 /* rad       radian                  */
            #define STREAM_SUBT_ANA_STERADIAN      11 /* sr        steradian               */
            #define STREAM_SUBT_ANA_NEWTON         12 /* N         newton                  */
            #define STREAM_SUBT_ANA_PASCAL         13 /* Pa        pascal                  */
            #define STREAM_SUBT_ANA_JOULE          14 /* J         joule                   */
            #define STREAM_SUBT_ANA_WATT           15 /* W         watt                    */
            #define STREAM_SUBT_ANA_COULOMB        16 /* C         coulomb                 */
            #define STREAM_SUBT_ANA_VOLT           17 /* V         volt                    */
            #define STREAM_SUBT_ANA_FARAD          18 /* F         farad                   */
            #define STREAM_SUBT_ANA_OHM            19 /* Ohm       ohm                     */
            #define STREAM_SUBT_ANA_SIEMENS        20 /* S         siemens                 */
            #define STREAM_SUBT_ANA_WEBER          21 /* Wb        weber                   */
            #define STREAM_SUBT_ANA_TESLA          22 /* T         tesla                   */
            #define STREAM_SUBT_ANA_HENRY          23 /* H         henry                   */
            #define STREAM_SUBT_ANA_CELSIUSDEG     24 /* Cel       degrees Celsius         */
            #define STREAM_SUBT_ANA_LUMEN          25 /* lm        lumen                   */
            #define STREAM_SUBT_ANA_LUX            26 /* lx        lux                     */
            #define STREAM_SUBT_ANA_BQ             27 /* Bq        becquerel               */
            #define STREAM_SUBT_ANA_GRAY           28 /* Gy        gray                    */
            #define STREAM_SUBT_ANA_SIVERT         29 /* Sv        sievert                 */
            #define STREAM_SUBT_ANA_KATAL          30 /* kat       katal                   */
            #define STREAM_SUBT_ANA_METERSQUARE    31 /* m2        square meter (area)     */
            #define STREAM_SUBT_ANA_CUBICMETER     32 /* m3        cubic meter (volume)    */
            #define STREAM_SUBT_ANA_LITER          33 /* l         liter (volume)                               */
            #define STREAM_SUBT_ANA_M_PER_S        34 /* m/s       meter per second (velocity)                  */
            #define STREAM_SUBT_ANA_M_PER_S2       35 /* m/s2      meter per square second (acceleration)       */
            #define STREAM_SUBT_ANA_M3_PER_S       36 /* m3/s      cubic meter per second (flow rate)           */
            #define STREAM_SUBT_ANA_L_PER_S        37 /* l/s       liter per second (flow rate)*                */
            #define STREAM_SUBT_ANA_W_PER_M2       38 /* W/m2      watt per square meter (irradiance)           */
            #define STREAM_SUBT_ANA_CD_PER_M2      39 /* cd/m2     candela per square meter (luminance)         */
            #define STREAM_SUBT_ANA_BIT            40 /* bit       bit (information content)                    */
            #define STREAM_SUBT_ANA_BIT_PER_S      41 /* bit/s     bit per second (data rate)                   */
            #define STREAM_SUBT_ANA_LATITUDE       42 /* lat       degrees latitude[1]                          */
            #define STREAM_SUBT_ANA_LONGITUDE      43 /* lon       degrees longitude[1]                         */
            #define STREAM_SUBT_ANA_PH             44 /* pH        pH value (acidity; logarithmic quantity)     */
            #define STREAM_SUBT_ANA_DB             45 /* dB        decibel (logarithmic quantity)               */
            #define STREAM_SUBT_ANA_DBW            46 /* dBW       decibel relative to 1 W (power level)        */
            #define STREAM_SUBT_ANA_BSPL           47 /* Bspl      bel (sound pressure level; log quantity)     */
            #define STREAM_SUBT_ANA_COUNT          48 /* count     1 (counter value)                            */
            #define STREAM_SUBT_ANA_PER            49 /* /         1 (ratio e.g., value of a switch; [2])       */
            #define STREAM_SUBT_ANA_PERCENT        50 /* %         1 (ratio e.g., value of a switch; [2])*      */
            #define STREAM_SUBT_ANA_PERCENTRH      51 /* %RH       Percentage (Relative Humidity)               */
            #define STREAM_SUBT_ANA_PERCENTEL      52 /* %EL       Percentage (remaining battery energy level)  */
            #define STREAM_SUBT_ANA_ENERGYLEVEL    53 /* EL        seconds (remaining battery energy level)     */
            #define STREAM_SUBT_ANA_1_PER_S        54 /* 1/s       1 per second (event rate)                    */
            #define STREAM_SUBT_ANA_1_PER_MIN      55 /* 1/min     1 per minute (event rate, "rpm")*            */
            #define STREAM_SUBT_ANA_BEAT_PER_MIN   56 /* beat/min  1 per minute (heart rate in beats per minute)*/
            #define STREAM_SUBT_ANA_BEATS          57 /* beats     1 (Cumulative number of heart beats)*        */
            #define STREAM_SUBT_ANA_SIEMPERMETER   58 /* S/m       Siemens per meter (conductivity)             */
            #define STREAM_SUBT_ANA_BYTE           59 /* B         Byte (information content)                   */
            #define STREAM_SUBT_ANA_VOLTAMPERE     60 /* VA        volt-ampere (Apparent Power)                 */
            #define STREAM_SUBT_ANA_VOLTAMPERESEC  61 /* VAs       volt-ampere second (Apparent Energy)         */
            #define STREAM_SUBT_ANA_VAREACTIVE     62 /* var       volt-ampere reactive (Reactive Power)        */
            #define STREAM_SUBT_ANA_VAREACTIVESEC  63 /* vars      volt-ampere-reactive second (Reactive Energy)*/
            #define STREAM_SUBT_ANA_JOULE_PER_M    64 /* J/m       joule per meter (Energy per distance)        */
            #define STREAM_SUBT_ANA_KG_PER_M3      65 /* kg/m3     kg/m3 (mass density, mass concentration)     */
            #define STREAM_SUBT_ANA_DEGREE         66 /* deg       degree (angle)*                              */
            #define STREAM_SUBT_ANA_NTU            67 /* NTU       Nephelometric Turbidity Unit                 */

            // Secondary Unit (rfc8798)           Description          SenML Unit     Scale     Offset 
            #define STREAM_SUBT_ANA_MS             68 /* millisecond                  s      1/1000    0       1ms = 1s x [1/1000] */
            #define STREAM_SUBT_ANA_MIN            69 /* minute                       s      60        0        */
            #define STREAM_SUBT_ANA_H              70 /* hour                         s      3600      0        */
            #define STREAM_SUBT_ANA_MHZ            71 /* megahertz                    Hz     1000000   0        */
            #define STREAM_SUBT_ANA_KW             72 /* kilowatt                     W      1000      0        */
            #define STREAM_SUBT_ANA_KVA            73 /* kilovolt-ampere              VA     1000      0        */
            #define STREAM_SUBT_ANA_KVAR           74 /* kilovar                      var    1000      0        */
            #define STREAM_SUBT_ANA_AH             75 /* ampere-hour                  C      3600      0        */
            #define STREAM_SUBT_ANA_WH             76 /* watt-hour                    J      3600      0        */
            #define STREAM_SUBT_ANA_KWH            77 /* kilowatt-hour                J      3600000   0        */
            #define STREAM_SUBT_ANA_VARH           78 /* var-hour                     vars   3600      0        */
            #define STREAM_SUBT_ANA_KVARH          79 /* kilovar-hour                 vars   3600000   0        */
            #define STREAM_SUBT_ANA_KVAH           80 /* kilovolt-ampere-hour         VAs    3600000   0        */
            #define STREAM_SUBT_ANA_WH_PER_KM      81 /* watt-hour per kilometer      J/m    3.6       0        */
            #define STREAM_SUBT_ANA_KIB            82 /* kibibyte                     B      1024      0        */
            #define STREAM_SUBT_ANA_GB             83 /* gigabyte                     B      1e9       0        */
            #define STREAM_SUBT_ANA_MBIT_PER_S     84 /* megabit per second           bit/s  1000000   0        */
            #define STREAM_SUBT_ANA_B_PER_S        85 /* byteper second               bit/s  8         0        */
            #define STREAM_SUBT_ANA_MB_PER_S       86 /* megabyte per second          bit/s  8000000   0        */
            #define STREAM_SUBT_ANA_MV             87 /* millivolt                    V      1/1000    0        */
            #define STREAM_SUBT_ANA_MA             88 /* milliampere                  A      1/1000    0        */
            #define STREAM_SUBT_ANA_DBM            89 /* decibel rel. to 1 milliwatt  dBW    1       -30     0 dBm = -30 dBW       */
            #define STREAM_SUBT_ANA_UG_PER_M3      90 /* microgram per cubic meter    kg/m3  1e-9      0        */
            #define STREAM_SUBT_ANA_MM_PER_H       91 /* millimeter per hour          m/s    1/3600000 0        */
            #define STREAM_SUBT_ANA_M_PER_H        92 /* meterper hour                m/s    1/3600    0        */
            #define STREAM_SUBT_ANA_PPM            93 /* partsper million             /      1e-6      0        */
            #define STREAM_SUBT_ANA_PER_100        94 /* percent                      /      1/100     0        */
            #define STREAM_SUBT_ANA_PER_1000       95 /* permille                     /      1/1000    0        */
            #define STREAM_SUBT_ANA_HPA            96 /* hectopascal                  Pa     100       0        */
            #define STREAM_SUBT_ANA_MM             97 /* millimeter                   m      1/1000    0        */
            #define STREAM_SUBT_ANA_CM             98 /* centimeter                   m      1/100     0        */
            #define STREAM_SUBT_ANA_KM             99 /* kilometer                    m      1000      0        */
            #define STREAM_SUBT_ANA_KM_PER_H      100 /* kilometer per hour           m/s    1/3.6     0        */
                                                                                                          
            #define STREAM_SUBT_ANA_GRAVITY       101 /* earth gravity                m/s2   9.81      0       1g = m/s2 x 9.81     */
            #define STREAM_SUBT_ANA_DPS           102 /* degrees per second           1/s    360       0     1dps = 1/s x 1/360     */   
            #define STREAM_SUBT_ANA_GAUSS         103 /* Gauss                        Tesla  10-4      0       1G = Tesla x 1/10000 */
            #define STREAM_SUBT_ANA_VRMS          104 /* Volt rms                     Volt   0.707     0    1Vrms = 1Volt (peak) x 0.707 */
            #define STREAM_SUBT_ANA_MVPGAUSS      105 /* Hall effect, mV/Gauss        millivolt 1      0    1mV/Gauss                    */


    /* IO_DOMAIN_RTC               : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
            /* tuning of PLATFORM_RTC_IN  */
                /* a FIFO is filled from RTC on periodic basis : */
                //const int32_t timer_in_settings [] = {    
                //    /* nb options nbbits */
                //    /*  8  3  unit : Days Hours Minutes Seconds 1ms
                //    /* Flexible bit-fields implementation-dependent
                //    /*   period Q8.4 x [unit]
                //    /*   offset (Q8) from the start of the day (watering)
                // use-case = periodic comparison of ADC value, periodic activation of GPIO
                //   one FIFO is trigered by unit: days one other by minutes, a script can
                //   implement the state machine to read sensors or activate watering


    /* IO_DOMAIN_USER_INTERFACE_IN : subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
    /* IO_DOMAIN_USER_INTERFACE_OUT: subtypes and tuning  SUBTYPE_FMT1 and SETTINGS_IOFMT2 */
        /* tuning of PLATFORM_USER_INTERFACE_IN / OUT */
            /* a FIFO is filled from UI detection of a button */
            /* one FIFO per button to ease the routing to specific nodes */

            /* Default implementation using SYSTICK
               Time format - 40 bits - SYSTICK 1ms increments, global register
               GLOBALS : G_INCTICK (64bits), INV_RVR (32bits), G_SYST0 (32bits) interval in Q32 second
               G_INCTICK : incremented in the SYSTICK interrupt, SYST0=1ms or 10ms
               FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
               ________________________qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq with 1ms increment => 35 years (32bits gives only 50 days)
  
               SYST_CVR (current value), SYST_CALIB (reload value for 10ms), SYST_RVR (reload value), INV_RVR = 0x10000.0000/SYST_RVR
               Time sequence : G_INCTICK=>T0  CVR=>C0  G_INCTICK=>T1  CVR=>C1  

               Algorithm : if T0==T1 (no collision) then T=T0,C=C0  else T=T1,C=C1 endif
                the fraction of SYST0 is (RVR - C) x INV_RVR = xx...xxx
                the time (64bits Q16)  is  (T<<16) + (((RVR - C) x INV_RVR) >> 16)  => used for time-stamps 
                FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
                ________qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqXXXXXXXXXXXXXXXX 
            */