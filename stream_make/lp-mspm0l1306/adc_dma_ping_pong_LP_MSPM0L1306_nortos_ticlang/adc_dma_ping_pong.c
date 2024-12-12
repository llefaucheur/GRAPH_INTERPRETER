/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"

#define ADC_SAMPLE_SIZE (32)

/* When FIFO is enabled 2 samples are compacted in a single word */
#define ADC_FIFO_SAMPLES (ADC_SAMPLE_SIZE / 2)

uint16_t gADCSamplesPing[ADC_SAMPLE_SIZE];
uint16_t gADCSamplesPong[ADC_SAMPLE_SIZE];


volatile bool gCheckADC;
volatile bool gPing;

int main(void) {

  gPing = true;

  SYSCFG_DL_init();

/* LED */
  DL_GPIO_initDigitalOutput(GPIO_IOMUX_RGB_GREEN);

  /* Configure DMA source, destination and size */
  DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)DL_ADC12_getFIFOAddress(ADC12_0_INST));
  DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&gADCSamplesPing[0]);
  DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, ADC_FIFO_SAMPLES);
  DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);

  /* Setup interrupts on device */
  NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);

  gCheckADC = false;

  /* The ADC is configured for Repeat Single Conversion,
   * so the ADC will continue until DL_ADC12_disableConversions() is called */
  DL_ADC12_startConversion(ADC12_0_INST);

  { extern void main_init(void);
    main_init();
  }

  while (1) {
    extern void main_run(void);
    while (false == gCheckADC) {
        __WFE();
    }
    /* Breakpoint to check the buffers and watch the Ping Pong operation.
     * The data will be alternating between each buffer.
     */
   // __BKPT(0);

    {   extern void Push_Ping_Pong(uint32_t *data, uint32_t size);
        extern void main_run(void);
        if (gPing) {
            Push_Ping_Pong ((uint32_t *)gADCSamplesPing, sizeof(gADCSamplesPing));
        }
        else       {
            Push_Ping_Pong ((uint32_t *)gADCSamplesPong, sizeof(gADCSamplesPong));
        }
    }
    {
      main_run();
    }
    /* Switch to send ADC Results to Pong Buffer */
    if (gPing) {

      DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&gADCSamplesPong[0]);
      DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, ADC_FIFO_SAMPLES);
      DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
      DL_ADC12_enableDMA(ADC12_0_INST);

      gPing = false;
    }
    /* Switch to send ADC Results to Ping Buffer */
    else {

      DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)&gADCSamplesPing[0]);
      DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, ADC_FIFO_SAMPLES);
      DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
      DL_ADC12_enableDMA(ADC12_0_INST);

      gPing = true;
    }
  }

  gCheckADC = false;
}

void ADC12_0_INST_IRQHandler(void) {
  switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)) {
  case DL_ADC12_IIDX_DMA_DONE:
    gCheckADC = true;
    break;
  default:
    break;
  }
}


/* called from stream_platform/lp-mspm0l1306/platform_lp-mspm0l1306_io_services.c :
 *      gpio_out_1 (uint32_t command, stream_xdmbuffer_t *data)
 */
void platform_gpio_out_1(uint8_t bit)
{
    if (0 == bit)
    {   DL_GPIO_clearPins(GPIO_RGB_PORT, GPIO_RGB_GREEN_PIN);
        DL_GPIO_enableOutput(GPIO_RGB_PORT, GPIO_RGB_GREEN_PIN);
    }
    else
    {   DL_GPIO_setPins(GPIO_RGB_PORT, GPIO_RGB_GREEN_PIN);
        DL_GPIO_enableOutput(GPIO_RGB_PORT, GPIO_RGB_GREEN_PIN);
    }
}



void toggle_LED (uint32_t service_command, void *ptr1, void *ptr2, void *ptr3, uint32_t n)
{
    /* GPIO defines for LED1 */
    #define GPIO_LEDS_PORT                                                  (GPIOA)
    #define GPIO_LEDS_USER_LED_1_PIN                                (DL_GPIO_PIN_0)
    #define GPIO_LEDS_USER_LED_1_IOMUX                               (IOMUX_PINCM1)


    if (service_command == 0)
    {   DL_GPIO_clearPins(GPIOA, GPIO_LEDS_USER_LED_1_PIN);
    }
    else
    {   DL_GPIO_setPins  (GPIOA, GPIO_LEDS_USER_LED_1_PIN);
    }

    //DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
}
