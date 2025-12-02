#include "FreeRTOS.h"
#include "task.h"

#include "stream_common_const.h"
#include "stream_common_types.h"
#include "stream_const.h"
#include "stream_types.h"
#include "stream_extern.h"


/*
    global variables : all the instances of the graph interpreter
*/
#define STREAM_CURRENT_INSTANCE 0
#define STREAM_NB_INSTANCE 3

    /* GRAPH INSTANCES FOR THREAD 0 / 1 / 2 */
    arm_stream_instance_t instance[STREAM_NB_INSTANCE];

    extern TaskHandle_t new_thread0;
    //volatile uint32_t wakeup_thread0;



/* pvParameters contains TaskHandle_t */
void new_thread0_entry(void *pvParameters)
{
	static int f_init_made;
	FSP_PARAMETER_NOT_USED(pvParameters);
    
    if (f_init_made == 0)
    {
    	vTaskPrioritySet( new_thread0, tskIDLE_PRIORITY + 1 );
		extern void platform_init_stream_instance(arm_stream_instance_t *instance);

		instance[STREAM_CURRENT_INSTANCE].scheduler_control =
			PACK_STREAM_PARAM(
				STREAM_INSTANCE_ANY_PRIORITY,       // any priority
				STREAM_MAIN_INSTANCE,               // this interpreter instance is the main one (multi-thread)
				STREAM_NB_INSTANCE,                 // total number of instances executing this graph
				COMMDEXT_COLD_BOOT,                 // is it a warm or cold boot
				STREAM_SCHD_NO_SCRIPT,              // debugging scheme used during execution
				STREAM_SCHD_RET_END_ALL_PARSED      // interpreter returns after all nodes are parsed
				);

		/* provision protocol for situation when the graph comes from the application */
		instance[STREAM_CURRENT_INSTANCE].graph = 0;

		/* reset the graph */
		arm_graph_interpreter (STREAM_RESET, &(instance[STREAM_CURRENT_INSTANCE]), 0, 0);
        
        //wakeup_thread0 = 0;
        f_init_made = 1;
    }


	while (1 /* wakeup_thread0 */)
	{
        /* here test the need for memory recovery/swap 
            does the application modified the memory banks used by the graph ? 
        arm_memory_swap(&(instance[STREAM_CURRENT_INSTANCE]));  */

        arm_graph_interpreter (STREAM_RUN, &(instance[STREAM_CURRENT_INSTANCE]), 0, 0);

        /* here test the need for memory recovery/swap
            does the application intend to modify memory banks used by the graph ?
        arm_memory_swap(&(instance[STREAM_CURRENT_INSTANCE])); */

        //wakeup_thread0 = 0;
        vTaskSuspend( new_thread0 );
	}
}
