
#ifdef PLATFORM_RA8E1FPB
#include "FreeRTOS.h"
#include "task.h"
#include "presets.h"
#include "stream_common_const.h"
#include "stream_const.h"


const int16_t  io_test_audio_in_0 [] = 
{
    #include "io_test_audio_in_0.txt"
};

extern TaskHandle_t new_thread0;


extern void graph_test_scheduler(uint64_t time64);

#define IO_TEST_DATA_IN_1      1 
#define IO_TEST_ANALOG_IN_0    2 
#define IO_TEST_MOTION_IN_0    3 
#define IO_TEST_AUDIO_IN_0     4 
#define IO_TEST_SENSOR_2D_IN_0 5 
#define IO_TEST_LINE_OUT_0     6 
#define IO_TEST_GPIO_OUT_0     7 
#define IO_TEST_GPIO_OUT_1     8 
#define IO_TEST_DATA_OUT_0     9 

#define NBGRAPHIO (IO_TEST_DATA_OUT_0+1)

typedef struct 
{
    uint32_t IOIDX;
    uint8_t  *data;
    uint32_t frame_length;
    uint32_t data_size;
    uint64_t period;
    
} io_test_struct_t;

const io_test_struct_t ios[NBGRAPHIO] = 
{ //   IO                    data                   N   Size period
  {     0,                    0,                    0,  0,   1 },                        
  { IO_PLATFORM}_DATA_SINK ,  0,                    0,  0,   1 }, // IO_TEST_DATA_IN_1      1
  { IO_PLATFORM_DATA_IN_1  ,  0,                    0,  0,   1 }, // IO_TEST_ANALOG_IN_0    2
  { IO_PLATFORM_SENSOR_0   ,  0,                    0,  0,   1 }, // IO_TEST_MOTION_IN_0    3
  { IO_PLATFORM_MOTION_IN_0,  io_test_audio_in_0,   0,  0,   1 }, // IO_TEST_AUDIO_IN_0     4
  { IO_PLATFORM_AUDIO_IN_0 ,  0,                    0,  0,   1 }, // IO_TEST_SENSOR_2D_IN_0 5
  { IO_PLATFORM_2D_IN_0    ,  0,                    0,  0,   1 }, // IO_TEST_LINE_OUT_0     6
  { IO_PLATFORM_LINE_OUT_0 ,  0,                    0,  0,   1 }, // IO_TEST_GPIO_OUT_0     7
  { IO_PLATFORM_GPIO_OUT_0 ,  0,                    0,  0,   1 }, // IO_TEST_GPIO_OUT_1     8
  { IO_PLATFORM_GPIO_OUT_1 ,  0,                    0,  0,   1 }  // IO_TEST_DATA_OUT_0     9
};


void graph_test_scheduler(uint64_t time64)
{
    static int64_t io_counter[NBGRAPHIO];
    static uint64_t previous_time64;
    static uint8_t initialization;
    uint32_t i;
    

    if (0 == initialization)
    {   for (i = 1; i < NBGRAPHIO; i++)
        {   io_counter[i] = io_counter_reload[i];
        }
        initialization = 1;
    }

    for (i = 1; i < NBGRAPHIO; i++)
    {   int32_t time_difference = (int64_t)time64 - io_counter[i];

        if (time_difference <= 0)
        {   
            arm_stream_io_ack(IO_PLATFORM_SENSOR_0, buffer_analog_0, size_analog_0);
            xTaskResumeFromISR(new_thread0);
            previous_time64 = time64;
        }
    }
    time_difference = time64 - previous_time64;
    if (time_difference > GTIME_ONE_SECOND)
    {
    	xTaskResumeFromISR(new_thread0);
	    previous_time64 = time64;
    }
}
#endif
