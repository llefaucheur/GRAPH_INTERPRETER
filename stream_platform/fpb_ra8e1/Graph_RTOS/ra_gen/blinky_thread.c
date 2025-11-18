/* generated thread source file - do not edit */
#include <all_threads.h>


static uint8_t lower_priority_thread_stack[512] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.blinky_thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
static StaticTask_t lower_priority_thread_memory;
static void lower_priority_thread_func(void *pvParameters);

static StaticTask_t higher_priority_thread_memory;
static uint8_t higher_priority__thread_stack[512] BSP_PLACE_IN_SECTION(BSP_UNINIT_SECTION_PREFIX ".stack.blinky_thread") BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);
static void higher_priority_thread_func(void *pvParameters);

TaskHandle_t blinky_thread;
void blinky_thread_create(void);
void rtos_startup_err_callback(void *p_instance, void *p_data);
void rtos_startup_common_init(void);
extern uint32_t g_fsp_common_thread_count;

const rm_freertos_port_parameters_t blinky_thread_parameters =
{ .p_context = (void*) NULL, };

void blinky_thread_create(void)
{
    /* Increment count so we will know the number of threads created in the RA Configuration editor. */
    g_fsp_common_thread_count++;

    /* Initialize each kernel object. */

    blinky_thread = xTaskCreateStatic (
            lower_priority_thread_func,
            (const char*) "Blinky Thread", 512 / 4, // In words, not bytes
            (void*) &blinky_thread_parameters, //pvParameters
            1,
            (StackType_t*) &lower_priority_thread_stack,
            (StaticTask_t*) &lower_priority_thread_memory
    );

    if (NULL == blinky_thread)
    {
        rtos_startup_err_callback (blinky_thread, 0);
    }
}
static void low-lower_priority_thread_func(void *pvParameters)
{
    /* Initialize common components */
    rtos_startup_common_init ();


    /* Enter user code for this thread. Pass task handle. */
    blinky_thread_entry (pvParameters);
}
