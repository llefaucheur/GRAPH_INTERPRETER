---------------------------
[TOC]
--------------------------------------
# IO MANIFEST TEMPLATE

The graph interface manifests are detailing the stream of data flowing through, and the setting options (analog and digital).
Because of the variety of stream data types, the graph interpreter introduces the concept of "domains".
Next is a list of general information of the digital frame format, followed by specific information.

## IO manifest header 
The "IO manifest" starts with the name which will be used in the GUI design tool, followed by the "domain" (list below)
Example :
`   io_name io_platform_sensor_in_0 `  
`   io_domain   analog_in           `

## IO Domains
  | Domain name        | Code | Comments | 
  | ----            |   -- | -- | 
  | GENERAL            |   0 | (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings  | 
  | AUDIO_IN           |   1 | microphone, line-in, I2S, PDM RX                                                                                        | 
  | AUDIO_OUT          |   2 | line-out, earphone / speaker, PDM TX, I2S,                                                                              | 
  | GPIO_IN            |   3 | generic digital IO                                                                                   | 
  | GPIO_OUT           |   4 | generic digital IO , control of relay,                                                                                  | 
  | MOTION             |   5 | accelerometer, combined or not with pressure and gyroscope                                                              | 
  | 2D_IN              |   6 | camera sensor                                                                                                           | 
  | 2D_OUT             |   7 | display, led matrix,                                                                                                    | 
  | ANALOG_IN          |   8 | analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage        | 
  | ANALOG_OUT         |   9 | D/A, position piezzo, PWM converter                                                                                     | 
  | RTC                |  10 | ticks sent from a programmable timer                                                                                    | 
  | USER_INTERFACE_IN  |  11 | button, slider, rotary button                                                                                           | 
  | USER_INTERFACE_OUT |  12 | LED, digits, display,                                                                                                   | 
  | PLATFORM_3         |  13 | platform-specific #3            | 
  | PLATFORM_2         |  14 | platform-specific #2                                                                            | 
  | PLATFORM_1         |  15 | platform-specific #1                                                                         | 

## Common information of all digital stream

### io_commander0_servant1 "0/1"
The IO is "commander" when it start the data exchanges with the graph without the control from the scheduler. It is "servant" when the scheduler needs to pull or push the by calling the IO functions.
commander=0 servant=1 (default is servant)
IO stream are managed from the graph scheduler with the help of one subroutine per IO using the template : typedef void (*p_io_function_ctrl) (uint32_t command, uint8_t *data, uint32_t length); 
The "command" parameter can be : STREAM_SET_PARAMETER, STREAM_DATA_START, STREAM_STOP, STREAM_SET_BUFFER.

When the IO is "Commander" it calls arm_graph_interpreter_io_ack() when data is read
When the IO is "Servant" the scheduler call p_io_function_ctrl(STREAM_RUN, ..) to ask for data move. Once the move is done the IO driver calls arm_graph_interpreter_io_ack()

Example :
`   io_commander0_servant1  1  `

#### Graph interpreter implementation details
IO stream are managed from the graph scheduler with the help of one subroutine per IO using the template : typedef void (*p_io_function_ctrl) (uint32_t command, uint8_t *data, uint32_t length); 
The "command" parameter can be : STREAM_SET_PARAMETER, STREAM_DATA_START, STREAM_STOP, STREAM_SET_BUFFER.

When the IO is "Commander" it calls arm_graph_interpreter_io_ack() when data is read
When the IO is "Servant" the scheduler call p_io_function_ctrl(STREAM_RUN, ..) to ask for data move. Once the move is done the IO driver calls arm_graph_interpreter_io_ack()


### io_buffer_allocation "x"

io_buffer_allocation    2.0     ;  default is 0, which means the buffer is declared outside of the graph
                                ;  The floating-point number is a multiplication factor of the frame size (here 2 frames), 
                                ;   the buffer size is computed with rounding (n = floor(X+0.5))
                                ;  
                                ;  When more than one byte are exchanged, the IO driver needs a temporary buffer. This buffer
                                ;   can be allocated "outside(0)" by the IO driver, or ">1" during the graph memory mapping preparation
                                ;   The memora mapping of this allocation is decided in the graph and can be in general-purpose or 
                                ;   any RAM "0" or specific memory bank for speed reason or reserved for DMA processing, etc ..
### io_set0_copy1 "0/1"
io_set0_copy1      1            ; 

### io_direction_rx0tx1 "0/1"
io_direction_rx0tx1   1         ; direction of the stream  0:input 1:output from graph point of view

### io_raw_format "n"
io_raw_format      S16          ; options for the raw arithmetics computation format here  STREAM_S16 

### io_interleaving "0/1"
io_interleaving    1            ; multichannel intleaved (0), deinterleaved by frame-size (1) 

### io_nb_channels "n"
io_nb_channels     1            ; options for the number of channels 

### io_frame_length "o"
io_frame_length   {1 1 2 16 }   ; options of possible frame_size in number of sample (can mono or multi-channel). 

### io_frame_duration "o"
io_frame_duration {1 10 22.5}   ; options of possible frame_size in [milliseconds].     The default frame length is 1 sample

### io_subtype_units "x"
io_subtype_units  VRMS          ; depending on the domain. Here Units_Vrms of the "general" domain (0 = any or underfined)

### io_subtype_multiple "x"
io_subtype_multiple {DPS GAUSS} ; example of multi domain : motion can have up to 4 data units for accelerometer, gyroscope, magnetometer, temperature


### io_position "x"
io_position meter 1.1 -2.2 0.01 ; unit and relative XYZ position with the platform reference point

### io_euler_angles "x"
io_euler_angles  10 20 90       ; Euler angles with respect to the platform reference orientation, in degrees

### io_sampling_rate "x"
io_sampling_rate       {1 16000 44100 48000} ; sampling rate options (enumeration in Hz)  

### io_sampling_period_s "s"
io_sampling_period_s   {1 1 60 120 }         ; sampling period options (enumeration in [second])   

### io_sampling_period_day "s"
io_sampling_period_day {1 0.25 1 7 }         ; sampling period options (enumeration in [day])   

### io_sampling_rate_accuracy "p"
io_sampling_rate_accuracy       0.1             ; in percentage


### io_time_stamp_format "n"
io_time_stamp_format {1 1}      ; 0 no time-stamp, 1 absolute time, 2 relative time from last frame, 3 frame counter
                            ; 0/1/2/3 corresponding to  16/32/64/64 bits time formats (default : STREAM_TIME32)


--------------------------

## Domain-specific information of the stream

--------------------------------------

### Domain "general"

   domain is using subtype_units "stream_unit_physical"

io_analog_scale 0.55           ; 0.55V is corresponding to full-scale (0x7FFF or 1.0f) with the default setting

        #define STREAM_SUBT_GENERAL_COMP195X 0 /* compressed byte stream following RFC1950 / RFC1951 ("deflate") */
        #define STREAM_SUBT_GENERAL_DPCM     1 /* compressed byte stream */
        #define STREAM_SUBT_GENERAL_JSON     2 /* JSON */
        #define STREAM_SUBT_GENERAL_XFORMAT  3 /* SensorThings MultiDatastream extension */
--------------------------------------


### Domain audio_in  

domain is using subtype_units "stream_unit_physical"

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
io_channel_mapping  1           ; mono (Front Left), 20 channels can be controlled :
  | Channel name    | Name | Bit | 
  | ----            | -- | -- | 
 |  Front Left               | FL    | 0    |
 |  Front Right              | FR    |    1    |
 |  Front Center             | FC    |    2    |
 |  Low Frequency            | LFE   |    3    |
 |  Back Left                | BL    |    4    |
 |  Back Right               | BR    |    5    |
 |  Front Left of Center     | FLC   |    6    |
 |  Front Right of Center    | FRC   |    7    |
 |  Back Center              | BC    |    8    |
 |  Side Left                | SL    |    9    |
 |  Side Right               | SR    |    10   |
 |  Top Center               | TC    |    11   |
 |  Front Left Height        | TFL   |    12   |
 |  Front Center Height      | TFC   |    13   |
 |  Front Right Height       | TFR   |    14   |
 |  Rear Left Height         | TBL   |    15   |
 |  Rear Center Height       | TBC   |    16   |
 |  Rear Right Height        | TBR   |    17   |
 |  Channel 19        | C19   |    18   |
 |  Channel 20        | C20   |    19   |

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


​            
--------------------------------------


### Domain audio_out          

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
--------------------------------------


### Domain gpio_in            
    0                       ; time-stamp (none)
    0   5                   ; samp. rate format (1:Hz 2:s 3:days) accuracy in % 
    1   1 10 20             ; options : sampling rate (in Hz, index <0 for a range [min, step, max])  
    
        /* tuning of PLATFORM_GPIO_IN / OUT */
            //const int32_tgpio_out_0_settings [] = {    
            /* nb options nbbits */
            //State : High-Z, low, high, duration, frequency
            //type : PWM, motor control, GPIO
            //PWM duty, duration, frequency (buzzer)
            //Servo motor control -120 .. +120 deg
            //keep the servo position
--------------------------------------


### Domain gpio_out           

    0                       ; time-stamp (none)

--------------------------------------


### Domain motion             

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

--------------------------------------


### Domain 2d_in              

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


--------------------------------------


### Domain 2d_out             
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


--------------------------------------

### Domain analog_in
    /* IO_DOMAIN_ANALOG_IN  */ 
            sensitivity / PGA
            detection threshold
            detection attack time
            detection release time
            aging control 
### Domain analog_out         

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


### Domain rtc                
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


### Domain user_interface_in  
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



### Domain user_interface_out 
    0                       ; time-stamp (none)





/* ==========================================================================================

    IO_DOMAIN physical types and tuning

   ==========================================================================================
*/

--------------------------
## Comments section 
Information :
- jumpers to set on the board
- manufacturer references and internet URLs
- any other system integration warning and recommendations

C:\Work\GITHUB_LLF\GRAPH_INTERPRETER\stream_platform\windows\manifest\io_platform_analog_sensor_0.txt


; io_commander0_servant1 1                    ; commander=0 servant=1 (default is servant)
; io_buffer_allocation   2.0                  ; default is 0, which means the buffer is declared outside of the graph
; io_direction_rx0tx1    1                    ; direction of the stream  0:input 1:output from graph point of view
; io_raw_format          {1 17}               ; options for the raw arithmetics computation format here  STREAM_S16 
; io_nb_channels         {1 1 2}              ; multichannel intleaved (0), deinterleaved by frame-size (1) + options for the number of channels 
; io_frame_length        {1 2 16}             ; [ms]0/[samp]1  +  options of possible frame_size 
; io_subtype_units       104                  ; depending on the domain. Here Units_Vrms of the "general" domain (0 = any or underfined)
; io_analogscale         0.55                 ; 0.55V is corresponding to full-scale (0x7FFF or 1.0f) with the default setting
; io_sampling_rate       {1 16000 44100 48000} ; sampling rate options (enumeration in Hz)  
; io_rescale_factor      12.24  -44.3         ; [1/a off] analog_input = invinterpa x ((samples/Full_Scale_Digital) - interpoff)            