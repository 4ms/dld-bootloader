/*
 * dig_pins.c - initialize general purpose input/output pins
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */

#include "dig_pins.h"
#include "stm32f4xx.h"

void init_dig_inouts(void) {

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	GPIO_InitTypeDef gpio = {0};

	// Configure outputs
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
	gpio.Pull = GPIO_NOPULL;

	// LEDs
	gpio.Pin = LED_LOOP1;
	HAL_GPIO_Init(LED_LOOP1_GPIO, &gpio);
	gpio.Pin = LED_LOOP2;
	HAL_GPIO_Init(LED_LOOP2_GPIO, &gpio);

	gpio.Pin = LED_PINGBUT_pin;
	HAL_GPIO_Init(LED_PINGBUT_GPIO, &gpio);

	gpio.Pin = LED_INF1_pin;
	HAL_GPIO_Init(LED_INF1_GPIO, &gpio);

	gpio.Pin = LED_INF2_pin;
	HAL_GPIO_Init(LED_INF2_GPIO, &gpio);

	gpio.Pin = LED_REV1_pin;
	HAL_GPIO_Init(LED_REV1_GPIO, &gpio);

	gpio.Pin = LED_REV2_pin;
	HAL_GPIO_Init(LED_REV2_GPIO, &gpio);


	// DEBUG pins

	gpio.Pin = DEBUG0;
	HAL_GPIO_Init(DEBUG0_GPIO, &gpio);
	gpio.Pin = DEBUG1;
	HAL_GPIO_Init(DEBUG1_GPIO, &gpio);
	gpio.Pin = DEBUG2;
	HAL_GPIO_Init(DEBUG2_GPIO, &gpio);
	gpio.Pin = DEBUG3;
	HAL_GPIO_Init(DEBUG3_GPIO, &gpio);
	DEBUG0_OFF;
	DEBUG1_OFF;
	DEBUG2_OFF;
	DEBUG3_OFF;

	// Configure inputs
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_PULLUP;

	// Reverse buttons
	gpio.Pin = REV2BUT_pin;
	HAL_GPIO_Init(REV2BUT_GPIO, &gpio);
	gpio.Pin = REV1BUT_pin;
	HAL_GPIO_Init(REV1BUT_GPIO, &gpio);

	// Ping button 
	gpio.Pin = PINGBUT_pin;
	HAL_GPIO_Init(PINGBUT_GPIO, &gpio);

	// Inf Repeat buttons 
	gpio.Pull = GPIO_PULLUP;

	gpio.Pin = INF1BUT_pin;
	HAL_GPIO_Init(INF1BUT_GPIO, &gpio);
	gpio.Pin = INF2BUT_pin;
	HAL_GPIO_Init(INF2BUT_GPIO, &gpio);
}
