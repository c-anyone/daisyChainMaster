/*
 * led_commands.h
 *
 *  Created on: Oct 24, 2016
 *      Author: Faebsn
 */

#ifndef LED_COMMANDS_H_
#define LED_COMMANDS_H_

typedef enum {
	LED_COMMAND_ON,
	LED_COMMAND_OFF,
	LED_COMMAND_SET,
	LED_COMMAND_GET_TEMP,
	LED_COMMAND_GET_TYPES,
	LED_COMMAND_GET_PWM_SETTINGS
}led_command_t;

typedef struct {
	uint16_t led1;
	uint16_t led2;
	uint16_t led3;
} PWM_SETTINGS_t;

typedef struct {
	uint16_t wavelength;
	uint8_t  leds;
	uint16_t amps;
} ledtype_t;

#endif /* LED_COMMANDS_H_ */
