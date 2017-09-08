/*
 * ws2812.c
 *
 *  Created on: May 21, 2017
 *      Author: erakhorst
 */

#include "ws2812.h"

static void fillLed(uint8_t *buffer, uint8_t *color)
{
    int i;

    for(i=0; i<8; i++) // RED data
    {
        buffer[i] = ((color[0]<<i) & 0x80)?65:26;
    }
    for(i=0; i<8; i++) // GREEN
    {
        buffer[8+i] = ((color[1]<<i) & 0x80)?65:26;
    }
    for(i=0; i<8; i++) // BLUE
    {
        buffer[16+i] = ((color[2]<<i) & 0x80)?65:26;
    }
}

void ws2812Send(uint8_t (*color)[3], int len, Ws2812DmaAdmin *admin)
{
    int i;
    if(len<1) return;

    //Wait for previous transfer to be finished
//  xSemaphoreTake(allLedDone, portMAX_DELAY);

    // Set interrupt context ...
    admin->current_led = 0;
    admin->total_led = len;
    admin->color_led = color;

    for(i=0; (i<LED_PER_HALF) && (admin->current_led<admin->total_led+2); i++, admin->current_led++) {
        if (admin->current_led<admin->total_led)
            fillLed(admin->buffer.begin+(24*i), admin->color_led[admin->current_led]);
        else
            bzero(admin->buffer.begin+(24*i), 24);
    }

    for(i=0; (i<LED_PER_HALF) && (admin->current_led<admin->total_led+2); i++, admin->current_led++) {
        if (admin->current_led<admin->total_led)
            fillLed(admin->buffer.end+(24*i), admin->color_led[admin->current_led]);
        else
            bzero(admin->buffer.end+(24*i), 24);
    }

//  DMA1_Channel2->CNDTR = sizeof(led_dma.buffer); // load number of bytes to be transferred
//  DMA_Cmd(DMA1_Channel2, ENABLE);             // enable DMA channel 2
//  __HAL_DMA_ENABLE(&hdma_tim1_ch2);
//  HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_CC2],(uint32_t)&led_dma.buffer, (uint32_t)&TIM1->CCR2, len);
//    HAL_TIM_PWMN_Start_DMA(admin->TimerPtr, admin->DMAChannel, (uint32_t *)admin->buffer.buffer, sizeof(admin->buffer.buffer));
    HAL_TIMEx_PWMN_Start_DMA(admin->TimerPtr, admin->DMAChannel, (uint32_t *)admin->buffer.buffer, sizeof(admin->buffer.buffer));
   // Go!!!
//    __HAL_DMA_ENABLE_IT
//    __HAL_TIM_ENABLE_DMA(admin->TimerPtr, admin->DMACaptureCompare);
}

void ws2812DmaIsr(DMA_HandleTypeDef *hdma, Ws2812DmaAdmin *admin)
{
//    portBASE_TYPE xHigherPriorityTaskWoken;
    uint8_t * buffer;
    int i;

    if (admin->total_led == 0)
    {
//        HAL_TIM_PWM_Stop_DMA(admin->TimerPtr, admin->DMAChannel);
        HAL_TIMEx_PWMN_Stop_DMA(admin->TimerPtr, admin->DMAChannel);
//        TIM_Cmd(TIM1, DISABLE);
//      DMA_Cmd(DMA1_Channel2, DISABLE);
    }

    if (hdma->State == HAL_DMA_STATE_READY_HALF)
    {
        buffer = admin->buffer.begin;
    }

    if (hdma->State == HAL_DMA_STATE_READY)
    {
        buffer = admin->buffer.end;
    }

    for(i=0; (i<LED_PER_HALF) && (admin->current_led<admin->total_led+2); i++) {
        if (admin->current_led<admin->total_led)
            fillLed(buffer+(24*i), admin->color_led[admin->current_led]);
        else
            bzero(buffer+(24*i), 24);
    }
    admin->current_led++;
    if (admin->current_led >= admin->total_led+2) {
//        xSemaphoreGiveFromISR(allLedDone, &xHigherPriorityTaskWoken);

//      TIM_Cmd(TIM1, DISABLE);                     // disable Timer 1
//      DMA_Cmd(DMA1_Channel2, DISABLE);            // disable DMA channel 2
//        HAL_TIM_PWM_Stop_DMA(admin->TimerPtr, admin->DMAChannel);
        HAL_TIMEx_PWMN_Stop_DMA(admin->TimerPtr, admin->DMAChannel);

        admin->total_led = 0;
    }
}

