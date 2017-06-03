/*
 * ws2812.h
 *
 *  Created on: May 21, 2017
 *      Author: erakhorst
 */

#ifndef WS2812_H_
#define WS2812_H_


#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_tim.h"



// The minimum is to have 2 leds (1 per half buffer) in the buffer, this
// consume 42Bytes and will trigger the DMA interrupt at ~2KHz.
// Putting 2 there will divide by 2 the interrupt frequency but will also
// double the memory consumption (no free lunch ;-)
#define LED_PER_HALF 1

typedef union {
    uint8_t buffer[2*LED_PER_HALF*24];
    struct {
        uint8_t begin[LED_PER_HALF*24];
        uint8_t end[LED_PER_HALF*24];
    } __attribute__((packed));
} led_dma_buffer;


typedef struct {
  uint32_t current_led;         //runtime
  uint32_t total_led;           //runtime
  uint8_t (*color_led)[3];      //runtime

  uint32_t DMAChannel;          //config
  uint32_t DMACaptureCompare;   //config
  led_dma_buffer buffer;        //runtime
  TIM_HandleTypeDef *TimerPtr;  //config

}Ws2812DmaAdmin;

static void fillLed(uint8_t *buffer, uint8_t *color);
void ws2812Send(uint8_t (*color)[3], int len, Ws2812DmaAdmin *admin);
void ws2812DmaIsr(DMA_HandleTypeDef *hdma, Ws2812DmaAdmin *admin);

#endif /* WS2812_H_ */
