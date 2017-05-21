/*
 * config.h
 *
 *  Created on: May 21, 2017
 *      Author: erakhorst
 */

#ifndef ABRY_CONFIG_H_
#define ABRY_CONFIG_H_


#define NR_OF_OUTPUTS  7

typedef enum
{
  OT_OFF,
  OT_WS2811,
  OT_PWM,
  OT_UART
}abry_output_type;

typedef enum
{
  DMX,
  ABRYNET
}abry_input_type;


typedef enum
{
  OFF,
  IN,
  OUT
}abry_uart_type;

/*
 * config for each output
 * type determines output protocol
 */
typedef struct
{
  abry_output_type type;
  uint16_t  channel_start;
  uint16_t  channel_count;
} abry_outputconfig;


typedef struct
{
  abry_input_type type;
  uint8_t   input_device; //DC if spi, anders uart number for input.
  uint16_t  start; //start universe if dmx, start channel if abrynet
  uint16_t  count; //aantal universes or number of channels of abrynet.
} abry_inputconfig;


typedef struct
{
  abry_inputconfig input;
  abry_outputconfig outputs[NR_OF_OUTPUTS];


} abry_config;

#endif /* ABRY_CONFIG_H_ */
