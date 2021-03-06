/*
 * dig_inouts.c
 */


#include "dig_inouts.h"


void init_dig_inouts(void){
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);

	//Configure outputs
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_25MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//LEDs
	RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_OVLD1 | LED_OVLD2;	GPIO_Init(LED_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(PINGBUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_PINGBUT_pin;	GPIO_Init(LED_PINGBUT_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF1_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF1_pin;	GPIO_Init(LED_INF1_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF2_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF2_pin;	GPIO_Init(LED_INF2_GPIO, &gpio);


	//CLKOUT
	RCC_AHB1PeriphClockCmd(CLKOUT_RCC, ENABLE);
	gpio.GPIO_Pin = CLKOUT_pin;	GPIO_Init(CLKOUT_GPIO, &gpio);
	CLKOUT_OFF;

	//DEBUG pins
	/*
	RCC_AHB1PeriphClockCmd(DEBUG_RCC, ENABLE);

	gpio.GPIO_Pin = DEBUG0;	GPIO_Init(DEBUG0_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG1;	GPIO_Init(DEBUG1_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG2;	GPIO_Init(DEBUG2_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG3;	GPIO_Init(DEBUG3_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG4;	GPIO_Init(DEBUG4_GPIO, &gpio);

	 DEBUG0_OFF;
	 DEBUG1_OFF;
	 DEBUG2_OFF;
	 DEBUG3_OFF;
	 DEBUG4_OFF;
*/

	//Configure inputs
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;
/*
	//Div/Mult switches
	RCC_AHB1PeriphClockCmd(TIMESW_RCC, ENABLE);

	gpio.GPIO_Pin = TIMESW_CH1_T1_pin;	GPIO_Init(TIMESW_CH1_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH1_T2_pin;	GPIO_Init(TIMESW_CH1_T2_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T1_pin;	GPIO_Init(TIMESW_CH2_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T2_pin;	GPIO_Init(TIMESW_CH2_T2_GPIO, &gpio);
*/

	//Reverse buttons
	RCC_AHB1PeriphClockCmd(REVSW_RCC, ENABLE);

	gpio.GPIO_Pin = REV2SW_pin;	GPIO_Init(REV2SW_GPIO, &gpio);
	gpio.GPIO_Pin = REV1SW_pin;	GPIO_Init(REV1SW_GPIO, &gpio);


	//Ping button and jack
	RCC_AHB1PeriphClockCmd(PING_RCC, ENABLE);

	gpio.GPIO_Pin = PINGBUT_pin;	GPIO_Init(PINGBUT_GPIO, &gpio);
/*
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Pin = PINGJACK_pin;	GPIO_Init(PINGJACK_GPIO, &gpio);
	gpio.GPIO_Pin = GPIO_Pin_10;	GPIO_Init(GPIOC, &gpio);
*/

	// Inf Repeat buttons and jacks
	RCC_AHB1PeriphClockCmd(INF_RCC, ENABLE);

	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	gpio.GPIO_Pin = INF1BUT_pin;	GPIO_Init(INF1BUT_GPIO, &gpio);
	gpio.GPIO_Pin = INF2BUT_pin;	GPIO_Init(INF2BUT_GPIO, &gpio);

}
