/*
 * dig_inouts.c
 */

#include "dig_pins.h"

void init_dig_inouts(void) {
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);

	//Configure outputs
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_25MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//LEDs
	RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_OVLD1 | LED_OVLD2;
	GPIO_Init(LED_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(PINGBUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_PINGBUT_pin;
	GPIO_Init(LED_PINGBUT_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF1_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF1_pin;
	GPIO_Init(LED_INF1_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF2_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF2_pin;
	GPIO_Init(LED_INF2_GPIO, &gpio);

	//CLKOUT
	RCC_AHB1PeriphClockCmd(CLKOUT_RCC, ENABLE);
	gpio.GPIO_Pin = CLKOUT_pin;
	GPIO_Init(CLKOUT_GPIO, &gpio);
	CLKOUT_OFF;


	//Configure inputs
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	//Reverse buttons
	RCC_AHB1PeriphClockCmd(REVSW_RCC, ENABLE);

	gpio.GPIO_Pin = REV2SW_pin;
	GPIO_Init(REV2SW_GPIO, &gpio);
	gpio.GPIO_Pin = REV1SW_pin;
	GPIO_Init(REV1SW_GPIO, &gpio);

	//Ping button and jack
	RCC_AHB1PeriphClockCmd(PING_RCC, ENABLE);

	gpio.GPIO_Pin = PINGBUT_pin;
	GPIO_Init(PINGBUT_GPIO, &gpio);

	// Inf Repeat buttons and jacks
	RCC_AHB1PeriphClockCmd(INF_RCC, ENABLE);

	gpio.GPIO_PuPd = GPIO_PuPd_UP;
	gpio.GPIO_Pin = INF1BUT_pin;
	GPIO_Init(INF1BUT_GPIO, &gpio);
	gpio.GPIO_Pin = INF2BUT_pin;
	GPIO_Init(INF2BUT_GPIO, &gpio);
}
