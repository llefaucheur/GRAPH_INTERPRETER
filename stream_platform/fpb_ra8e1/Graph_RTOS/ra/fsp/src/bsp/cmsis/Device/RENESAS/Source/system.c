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
#include <string.h>
#if defined(__GNUC__) && defined(__llvm__) && !defined(__ARMCC_VERSION) && !defined(__CLANG_TIDY__)
 #include <picotls.h>
#endif
#if defined(__ARMCC_VERSION)
 #if defined(__ARMCC_USING_STANDARDLIB)
  #include <rt_misc.h>
 #endif
#endif
#include "bsp_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Mask to select CP bits( 0xF00000 ) */
#define CP_MASK                                       (0xFU << 20)

/* Startup value for CCR to enable instruction cache, branch prediction and LOB extension */
#define CCR_CACHE_ENABLE                              (0x000E0201)

/* Value to write to OAD register of MPU stack monitor to enable NMI when a stack overflow is detected. */
#define BSP_STACK_POINTER_MONITOR_NMI_ON_DETECTION    (0xA500U)

/* Key code for writing PRCR register. */
#define BSP_PRV_PRCR_KEY                              (0xA500U)
#define BSP_PRV_PRCR_PRC1_UNLOCK                      ((BSP_PRV_PRCR_KEY) | 0x2U)
#define BSP_PRV_PRCR_LOCK                             ((BSP_PRV_PRCR_KEY) | 0x0U)
#define BSP_PRV_STACK_LIMIT                           ((uint32_t) &g_main_stack[0])
#define BSP_PRV_STACK_TOP                             ((uint32_t) (uint32_t) &g_main_stack[BSP_CFG_STACK_MAIN_BYTES])
#define BSP_TZ_STACK_SEAL_VALUE                       (0xFEF5EDA5)

#define ARMV8_MPU_REGION_MIN_SIZE                     (32U)

#if (BSP_CFG_CPU_CORE == 0) && (BSP_FEATURE_BSP_HAS_ITCM || BSP_FEATURE_BSP_HAS_DTCM)
 #define BSP_PRV_ITCM_START_ADDRESS                   (0x00000000UL)
 #define BSP_PRV_DTCM_START_ADDRESS                   (0x20000000UL)
 #define BSP_PRV_VTOR_FIRST_PROJECT                   (0x02000000UL)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/

/** System Clock Frequency (Core Clock) */
uint32_t SystemCoreClock BSP_SECTION_EARLY_INIT;

#if defined(__GNUC__)

/* Nested in __GNUC__ because LLVM generates both __GNUC__ and __llvm__*/
 #if defined(__llvm__) && !defined(__CLANG_TIDY__)
extern uint32_t __tls_base;
 #endif

#endif

/* Initialize static constructors */
#if defined(__GNUC__)

extern void (* __init_array_start[])(void);

extern void (* __init_array_end[])(void);
#elif defined(__ICCARM__)
extern void __call_ctors(void const *, void const *);

 #pragma section = "SHT$$PREINIT_ARRAY" const
 #pragma section = "SHT$$INIT_ARRAY" const
#endif

extern void  * __VECTOR_TABLE[];
extern uint8_t g_main_stack[];

extern void R_BSP_SAUInit(void);
extern void R_BSP_SecurityInit(void);

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/

#if BSP_FEATURE_BSP_RESET_TRNG
static void bsp_reset_trng_circuit(void);

#endif

#if defined(__ICCARM__)

void R_BSP_WarmStart(bsp_warm_start_event_t event);

 #pragma weak R_BSP_WarmStart

#elif defined(__GNUC__) || defined(__ARMCC_VERSION)

void R_BSP_WarmStart(bsp_warm_start_event_t event) __attribute__((weak));

#endif

#if BSP_CFG_EARLY_INIT
static void bsp_init_uninitialized_vars(void);

#endif

#if (BSP_CFG_CPU_CORE == 0) && (BSP_FEATURE_BSP_HAS_ITCM || BSP_FEATURE_BSP_HAS_DTCM)
 #if !BSP_TZ_NONSECURE_BUILD
static void memset_64(uint64_t * destination, const uint64_t value, size_t count);

 #endif
#endif

#if BSP_CFG_DCACHE_ENABLED
static void bsp_init_mpu(void);

#endif

#if BSP_CFG_C_RUNTIME_INIT
static void SystemRuntimeInit(const uint32_t external);

#endif

#if BSP_CFG_C_RUNTIME_INIT
static void SystemRuntimeInit (const uint32_t external)
{
    /* Initialize C runtime environment. */
    for (uint32_t i = 0; i < g_init_info.zero_count; i++)
    {
        if (external == g_init_info.p_zero_list[i].type.external)
        {
            memset(g_init_info.p_zero_list[i].p_base, 0U,
                   ((uint32_t) g_init_info.p_zero_list[i].p_limit - (uint32_t) g_init_info.p_zero_list[i].p_base));
        }
    }

    for (uint32_t i = 0; i < g_init_info.copy_count; i++)
    {
        if (external == g_init_info.p_copy_list[i].type.external)
        {
            memcpy(g_init_info.p_copy_list[i].p_base, g_init_info.p_copy_list[i].p_load,
                   ((uint32_t) g_init_info.p_copy_list[i].p_limit - (uint32_t) g_init_info.p_copy_list[i].p_base));
        }
    }
}

#endif

/*******************************************************************************************************************//**
 * Initialize the MCU and the runtime environment.
 **********************************************************************************************************************/
void SystemInit (void)
{
    /* Enable the instruction cache, branch prediction, and the branch cache (required for Low Overhead Branch (LOB) extension).
     * See sections 6.5, 6.6, and 6.7 in the Arm Cortex-M85 Processor Technical Reference Manual (Document ID: 101924_0002_05_en, Issue: 05)
     * See section D1.2.9 in the Armv8-M Architecture Reference Manual (Document number: DDI0553B.w, Document version: ID07072023) */
    SCB->CCR = (uint32_t) CCR_CACHE_ENABLE;
    __DSB();
    __ISB();


    /* If Cortex-M85 revision is r1p1 or newer. */
    const uint32_t cpuid          = SCB->CPUID;
    const uint32_t cpuid_variant  = ((cpuid & SCB_CPUID_VARIANT_Msk) >> SCB_CPUID_VARIANT_Pos);
    const uint32_t cpuid_revision = ((cpuid & SCB_CPUID_REVISION_Msk) >> SCB_CPUID_REVISION_Pos);

    {
        /* Apply Arm Cortex-M85 errata workarounds for D-Cache.
         * See erratum 3175626 and 3190818 in the Cortex-M85 AT640 and Cortex-M85 with FPU AT641 Software Developer Errata Notice (Date of issue: March 07, 2024, Document version: 13.0, Document ID: SDEN-2236668). */
        MEMSYSCTL->MSCR |= MEMSYSCTL_MSCR_FORCEWT_Msk;
        __DSB();
        __ISB();
        ICB->ACTLR |= (1U << 16U);
        __DSB();
        __ISB();
    }


    /* Enable the FPU only when it is used.
     * Code taken from Section 7.1, Cortex-M4 TRM (DDI0439C) */

    /* Set bits 20-23 (CP10 and CP11) to enable FPU. */
    SCB->CPACR = (uint32_t) CP_MASK;

    /* Call pre clock initialization hook. */
    R_BSP_WarmStart(BSP_WARM_START_RESET);

    /* Configure system clocks. */
    bsp_clock_init();

    /* Unlock LVOCR register. */
    R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_PRC1_UNLOCK;

    /* Set LVOCR according to BSP configuration.
     * Configure prior to warm start post clock, since OSPI_B may initialize within and begin using I/O. */
    R_SYSTEM->LVOCR = ((BSP_CFG_IOPORT_VOLTAGE_MODE_VCC2 << R_SYSTEM_LVOCR_LVO1E_Pos) & R_SYSTEM_LVOCR_LVO1E_Msk) |
                      ((BSP_CFG_IOPORT_VOLTAGE_MODE_VCC << R_SYSTEM_LVOCR_LVO0E_Pos) & R_SYSTEM_LVOCR_LVO0E_Msk);

    /* Lock LVOCR register. */
    R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_LOCK;

    /* Call post clock initialization hook. */
    R_BSP_WarmStart(BSP_WARM_START_POST_CLOCK);

    __set_MSPLIM(BSP_PRV_STACK_LIMIT);


    /* Zero initialize all available Cortex-M85 TCM memory if ECC is enabled for it and the very first project is executing.
     * This may be either a bootloader if present, or a Flat or Secure application. */
    if ((MEMSYSCTL->MSCR & MEMSYSCTL_MSCR_ECCEN_Msk) &&
        (SCB->VTOR == BSP_PRV_VTOR_FIRST_PROJECT))
    {
        const size_t itcm_num_doublewords =
            (1U << (((MEMSYSCTL->ITCMCR & MEMSYSCTL_ITCMCR_SZ_Msk) >> MEMSYSCTL_ITCMCR_SZ_Pos) + 9U)) /
            sizeof(uint64_t);
        const size_t dtcm_num_doublewords =
            (1U << (((MEMSYSCTL->DTCMCR & MEMSYSCTL_DTCMCR_SZ_Msk) >> MEMSYSCTL_DTCMCR_SZ_Pos) + 9U)) /
            sizeof(uint64_t);
        memset_64((uint64_t *) BSP_PRV_ITCM_START_ADDRESS, 0, itcm_num_doublewords);
        memset_64((uint64_t *) BSP_PRV_DTCM_START_ADDRESS, 0, dtcm_num_doublewords);
    }


    /* Initialize data placed in internal memories. */
    SystemRuntimeInit(0);

    /* Initialize SystemCoreClock variable. */
    SystemCoreClockUpdate();


    /* Perform RTC reset sequence to avoid unintended operation. */
    R_BSP_Init_RTC();


    /* Ensure that the PMSAR registers are set to their default value. */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);

    for (uint32_t i = 0; i < BSP_FEATURE_BSP_NUM_PMSAR; i++)
    {
        R_PMISC->PMSAR[i].PMSAR = 0U;
    }
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);


    /* Initialize peripherals to secure mode for flat projects */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SAR);
    R_PSCU->PSARB = 0;
    R_PSCU->PSARC = 0;
    R_PSCU->PSARD = 0;
    R_PSCU->PSARE = 0;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SAR);

    /* Call Post C runtime initialization hook. */
    R_BSP_WarmStart(BSP_WARM_START_POST_C);

    /* Initialize data placed in external memories. */
    SystemRuntimeInit(1);

    /* Initialize TLS memory. */
    _init_tls(&__tls_base);
    _set_tls(&__tls_base);

    /* Invalidate I-Cache after initializing the .ram_from_flash section. */
    SCB_InvalidateICache();

    /* Initialize static constructors */
    int32_t count = __init_array_end - __init_array_start;
    for (int32_t i = 0; i < count; i++)
    {
        __init_array_start[i]();
    }

    /* Initialize ELC events that will be used to trigger NVIC interrupts. */
    bsp_irq_cfg();

    /* Call any BSP specific code. No arguments are needed so NULL is sent. */
    bsp_init(NULL);
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.
 * This function is declared as a weak symbol higher up in this file because it is meant to be overridden by a user
 * implemented version. One of the main uses for this function is to call functional safety code during the startup
 * process. To use this function just copy this function into your own code and modify it to meet your needs.
 *
 * @param[in]  event    Where the code currently is in the start up process
 **********************************************************************************************************************/
void R_BSP_WarmStart (bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
        /* C runtime environment has not been setup so you cannot use globals. System clocks are not setup. */
    }

    if (BSP_WARM_START_POST_CLOCK == event)
    {
        /* C runtime environment has not been setup so you cannot use globals. Clocks have been initialized. */
    }
    else if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment, system clocks, and pins are all setup. */
    }
    else
    {
        /* Do nothing */
    }
}


/*******************************************************************************************************************//**
 * 64-bit memory set for Armv8.1-M using low overhead loop instructions.
 *
 * @param[in] destination set destination start address, word aligned
 * @param[in] value value to set
 * @param[in] count number of doublewords to set
 **********************************************************************************************************************/
static void memset_64 (uint64_t * destination, const uint64_t value, size_t count)
{
    __asm volatile (
        "wls lr, %[count], memset_64_loop_end_%=\n"
  #if (defined(__ARMCC_VERSION) || defined(__GNUC__))

        /* Align the branch target to a 64-bit boundary, a CM85 specific optimization. */
        /* IAR does not support alignment control within inline assembly. */
        ".balign 8\n"
  #endif
        "memset_64_loop_start_%=:\n"
        "strd %Q[value], %R[value], [%[destination]], #+8\n"
        "le lr, memset_64_loop_start_%=\n"
        "memset_64_loop_end_%=:"
        :[destination] "+&r" (destination)
        :[count] "r" (count), [value] "r" (value)
        : "lr", "memory"
        );
}



/** @} (end addtogroup BSP_MCU) */
