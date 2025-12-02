
#include "presets.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "stream_common_const.h"
#include "stream_const.h"
#include "ra8e1/top_manifest_ra8e1.h"

extern void arm_stream_io_ack (uint8_t io_al_idx, void *data,  uint32_t size);
extern void graph_test_scheduler(uint64_t time64);


const int16_t  tstaudio_in_m1 [] = 
{
    #include "io_test_audio_in_0.txt"
};
const int16_t  tstaudio_in_0 [] = 
{
    #include "io_test_audio_in_0.txt"
};
const int16_t  tstaudio_in_1 [] = 
{
    #include "io_test_audio_in_0.txt"
};


extern TaskHandle_t new_thread0;
extern TaskHandle_t new_thread1;
extern TaskHandle_t new_thread2;


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


/*  graph_interpreter_time64" using a global variable
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s]  140 Y + Q28 [s]
 *
 *  increments for  1ms systick =  0x00041893 =  1ms x 2^28
 *  increments for 10ms systick =  0x0028F5C2 = 10ms x 2^28
 */

#define GTIMESEC(x) ((uint64_t)((x)*((float)(1L<<28))))

typedef struct 
{
    uint32_t IOIDX;
    uint8_t  *data;
    uint32_t buffer_size;
    uint32_t frame_length;
    uint64_t period;
    uint8_t thread_affinity_bitfield;
    
} io_test_struct_t;

const io_test_struct_t ios[NBGRAPHIO] =
{ //   IO                    data                buffer_size    frame     period    thread_affinity
  { IO_PLATFORM_DATA_SINK  ,  0,                        0,        0,       0,            0 },
  { IO_PLATFORM_DATA_IN_1  ,  0,                        0,        0,       0,            0 }, // IO_TEST_DATA_IN_1      1
  { IO_PLATFORM_SENSOR_0   ,  0,                        0,        0,       0,            0 }, // IO_TEST_ANALOG_IN_0    2
  { IO_PLATFORM_MOTION_IN_0,  0,                        0,        0,       0,            0 }, // IO_TEST_MOTION_IN_0    3
  { IO_PLATFORM_AUDIO_IN_0 ,  (uint8_t *)tstaudio_in_0, 160000,  320,   GTIMESEC(0.02),  1 }, // IO_TEST_AUDIO_IN_0     4
  { IO_PLATFORM_2D_IN_0    ,  0,                        0,        0,       0,            0 }, // IO_TEST_SENSOR_2D_IN_0 5
  { IO_PLATFORM_LINE_OUT_0 ,  0,                        0,        0,       0,            0 }, // IO_TEST_LINE_OUT_0     6
  { IO_PLATFORM_GPIO_OUT_0 ,  0,                        0,        0,       0,            0 }, // IO_TEST_GPIO_OUT_0     7
  { IO_PLATFORM_GPIO_OUT_1 ,  0,                        0,        0,       0,            0 }, // IO_TEST_GPIO_OUT_1     8
  { IO_PLATFORM_DATA_OUT_0 ,  0,                        0,        0,       0,            0 }  // IO_TEST_DATA_OUT_0     9
};                                                                

    

void graph_test_scheduler(uint64_t time64)
{
    static int64_t io_counter[NBGRAPHIO];
    static int32_t read_index[NBGRAPHIO];
    static uint8_t initialization;
    uint8_t *pt8;
    uint32_t i, threads;
    

    if (0 == initialization)
    {   
        for (i = 1; i < NBGRAPHIO; i++)
        {	io_counter[i] = time64 + ios[i].period;
        }
        initialization = 1;
    }

    /* call the graph scheduler if this time to exchange new data */
    threads = 0;
    for (i = 1; i < NBGRAPHIO; i++)
    {   
        if (ios[i].frame_length == 0)
        	continue;

        if (time64 > io_counter[i])
        {   
            pt8 = ios[i].data + read_index[i];
            arm_stream_io_ack(ios[i].IOIDX, pt8, ios[i].frame_length);
            read_index[i] += ios[i].frame_length;
            if (read_index[i] >= (ios[i].buffer_size - ios[i].frame_length))
            {   read_index[i] = 0;
            }
           
            io_counter[i] += ios[i].period;
            threads |= ios[i].thread_affinity_bitfield;
        }
    }

    /* awake the thread with affinity to IOs */
    if (threads & 1)
    {	xTaskResumeFromISR(new_thread0);
    }
    if (threads & 2)
    {	xTaskResumeFromISR(new_thread1);
    }
    if (threads & 4)
    {	xTaskResumeFromISR(new_thread2);
    }

}


#ifdef __cplusplus
}
#endif

