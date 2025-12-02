/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************************************************//**
 * @addtogroup BSP_MCU
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/
#include "bsp_api.h"
#include <stdint.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#if BSP_TZ_SECURE_BUILD
 #define BSP_TZ_STACK_SEAL_SIZE    (8U)
#else
 #define BSP_TZ_STACK_SEAL_SIZE    (0U)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/* Defines function pointers to be used with vector table. */
typedef void (* exc_ptr_t)(void);

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/
void    Reset_Handler(void);
void    Default_Handler(void);
int32_t main(void);

extern void SysTickSetup (void);
extern void Graph_SysTick_Handler (void);


/* ------------------------------ TWO LEDS ------------------------------------*/
extern bsp_leds_t g_bsp_leds;

void hal_set_led0_low(void)
{	R_BSP_PinAccessEnable();
    R_BSP_PinWrite((bsp_io_port_pin_t) g_bsp_leds.p_leds[0], BSP_IO_LEVEL_LOW);
    R_BSP_PinAccessDisable();
}
void hal_set_led0_high(void)
{	R_BSP_PinAccessEnable();
	R_BSP_PinWrite((bsp_io_port_pin_t) g_bsp_leds.p_leds[0], BSP_IO_LEVEL_HIGH);
	R_BSP_PinAccessDisable();
}

void hal_set_led1_low(void)
{	R_BSP_PinAccessEnable();
    R_BSP_PinWrite((bsp_io_port_pin_t) g_bsp_leds.p_leds[1], BSP_IO_LEVEL_LOW);
    R_BSP_PinAccessDisable();
}
void hal_set_led1_high(void)
{	R_BSP_PinAccessEnable();
	R_BSP_PinWrite((bsp_io_port_pin_t) g_bsp_leds.p_leds[1], BSP_IO_LEVEL_HIGH);
	R_BSP_PinAccessDisable();
}


/*******************************************************************************************************************//**
 * MCU starts executing here out of reset. Main stack pointer is set up already.
 **********************************************************************************************************************/
void Reset_Handler (void)
{
    /* Initialize system using BSP. */
    SystemInit();
    SysTickSetup();
    

    /* Call user application. */
    hal_set_led0_low();
    hal_set_led0_high();
    hal_set_led0_low();
    hal_set_led0_high();

    main();

    while (1)
    {
        /* Infinite Loop. */
    }
}


/* ============================================  TIME  ============================================== */
#define TIME_BASE_1MS 1
#define PROCESSOR_CLOCK 350000000L
static volatile uint64_t graph_interpreter_time64;              /* one global variable */

/*  graph_interpreter_time64" using a global variable
 *  FEDCBA987654321 FEDCBA987654321 FEDCBA987654321 FEDCBA9876543210
 *  ____ssssssssssssssssssssssssssssssssqqqqqqqqqqqqqqqqqqqqqqqqqqqq q32.28 [s]  140 Y + Q28 [s]
 *
 *  increments for  1ms systick =  0x00041893 =  1ms x 2^28
 *  increments for 10ms systick =  0x0028F5C2 = 10ms x 2^28
 */

#if TIME_BASE_1MS == 1
#define SYSTICK_LOAD_1MS (PROCESSOR_CLOCK/1000)
#define GRAPHTIME64INC 0x00041893L
#endif
#if TIME_BASE_10MS == 1
#define SYSTICK_LOAD_1MS (PROCESSOR_CLOCK/100)
#define GRAPHTIME64INC 0x0028F5C2L
#endif

#define INV_RVR (0x100000000/GRAPHTIME64INC)

/* 	SysTick IRQ handler
 */
extern void graph_systick_scheduler(uint64_t time64);
extern void Graph_SysTick_Handler (void);
	   void Graph_SysTick_Handler (void)
{
    extern void graph_test_scheduler(uint64_t time64);

    uint32_t *pt_DBG = (uint32_t *)0x22060760;

    if (0xFFFFFFFF == *pt_DBG) {
    	graph_interpreter_time64 = graph_interpreter_time64;
    }


    graph_interpreter_time64 = graph_interpreter_time64 + GRAPHTIME64INC;
    graph_test_scheduler(graph_interpreter_time64);
}


extern void SysTickSetup (void);
       void SysTickSetup (void)
{   SysTick->LOAD = SYSTICK_LOAD_1MS;
    SysTick->CTRL = 7;
    SysTick->VAL  = 0U;
    graph_interpreter_time64  = 0U;     // graph interpreter global counter
    NVIC_SetPriority(SysTick_IRQn, 2);
    __enable_irq();
}


/*******************************************************************************************************************//**
 * Default exception handler.
 **********************************************************************************************************************/
void Default_Handler (void)
{
    /** A error has occurred. The user will need to investigate the cause. Common problems are stack corruption
     *  or use of an invalid pointer. Use the Fault Status window in e2 studio or manually check the fault status
     *  registers for more information.
     */
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}

/* Main stack */
uint8_t g_main_stack[BSP_CFG_STACK_MAIN_BYTES + BSP_TZ_STACK_SEAL_SIZE] BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);

/* Heap */
BSP_DONT_REMOVE uint8_t g_heap[BSP_CFG_HEAP_BYTES] BSP_ALIGN_VARIABLE(BSP_STACK_ALIGNMENT);

/* All system exceptions in the vector table are weak references to Default_Handler. If the user wishes to handle
 * these exceptions in their code they should define their own function with the same name.
 */
#if defined(__ICCARM__)
 #define WEAK_REF_ATTRIBUTE

 #pragma weak HardFault_Handler                        = Default_Handler
 #pragma weak MemManage_Handler                        = Default_Handler
 #pragma weak BusFault_Handler                         = Default_Handler
 #pragma weak UsageFault_Handler                       = Default_Handler
 #pragma weak SecureFault_Handler                      = Default_Handler
 #pragma weak SVC_Handler                              = Default_Handler
 #pragma weak DebugMon_Handler                         = Default_Handler
 #pragma weak PendSV_Handler                           = Default_Handler
 #pragma weak SysTick_Handler                          = Default_Handler
#elif defined(__GNUC__)

 #define WEAK_REF_ATTRIBUTE    __attribute__((weak, alias("Default_Handler")))
#endif

void NMI_Handler(void);                // NMI has many sources and is handled by BSP
void HardFault_Handler(void) WEAK_REF_ATTRIBUTE;
void MemManage_Handler(void) WEAK_REF_ATTRIBUTE;
void BusFault_Handler(void) WEAK_REF_ATTRIBUTE;
void UsageFault_Handler(void) WEAK_REF_ATTRIBUTE;
void SecureFault_Handler(void) WEAK_REF_ATTRIBUTE;
void SVC_Handler(void) WEAK_REF_ATTRIBUTE;
void DebugMon_Handler(void) WEAK_REF_ATTRIBUTE;
void PendSV_Handler(void) WEAK_REF_ATTRIBUTE;
void SysTick_Handler(void) WEAK_REF_ATTRIBUTE;

/* Vector table. */
BSP_DONT_REMOVE const exc_ptr_t __VECTOR_TABLE[BSP_CORTEX_VECTOR_TABLE_ENTRIES] BSP_PLACE_IN_SECTION(
    BSP_SECTION_FIXED_VECTORS) =
{
    (exc_ptr_t) (&g_main_stack[0] + BSP_CFG_STACK_MAIN_BYTES), /*      Initial Stack Pointer     */
    Reset_Handler,                                             /*      Reset Handler             */
    NMI_Handler,                                               /*      NMI Handler               */
    HardFault_Handler,                                         /*      Hard Fault Handler        */
    MemManage_Handler,                                         /*      MPU Fault Handler         */
    BusFault_Handler,                                          /*      Bus Fault Handler         */
    UsageFault_Handler,                                        /*      Usage Fault Handler       */
    SecureFault_Handler,                                       /*      Secure Fault Handler      */
    0,                                                         /*      Reserved                  */
    0,                                                         /*      Reserved                  */
    0,                                                         /*      Reserved                  */
    SVC_Handler,                                               /*      SVCall Handler            */
    DebugMon_Handler,                                          /*      Debug Monitor Handler     */
    0,                                                         /*      Reserved                  */
    PendSV_Handler,                                            /*      PendSV Handler            */
    Graph_SysTick_Handler,                                     /*      SysTick Handler + soft timer */
};

/** @} (end addtogroup BSP_MCU) */
