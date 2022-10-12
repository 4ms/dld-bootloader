/*
 * codec_CS4271.c - Setup for the Cirrus Logic CS4271 codec
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
#ifndef __codec__
#define __codec__

#include <stm32f4xx.h>

/* I2C clock speed configuration (in Hz)  */
#define CODEC_I2C_SPEED 50000

#define CODECA_I2C I2C1
#define CODECA_I2C_CLK __HAL_RCC_I2C1_CLK_ENABLE
#define CODECA_I2C_GPIO_CLOCK RCC_AHB1Periph_GPIOB
#define CODECA_I2C_GPIO_AF GPIO_AF4_I2C1
#define CODECA_I2C_GPIO GPIOB
#define CODECA_I2C_SCL_PIN GPIO_PIN_8
#define CODECA_I2C_SDA_PIN GPIO_PIN_9

#define CODECB_I2C I2C2
#define CODECB_I2C_CLK __HAL_RCC_I2C2_CLK_ENABLE
#define CODECB_I2C_GPIO_CLOCK __HAL_RCC_GPIOB_CLK_ENABLE
#define CODECB_I2C_GPIO_AF GPIO_AF4_I2C2
#define CODECB_I2C_GPIO GPIOB
#define CODECB_I2C_SCL_PIN GPIO_PIN_10
#define CODECB_I2C_SDA_PIN GPIO_PIN_11

// CODEC A: SAI1_SD_A/B: DLD Left Side
#define CODECA_RESET_RCC_ENABLE __HAL_RCC_GPIOD_CLK_ENABLE
#define CODECA_RESET_pin GPIO_PIN_2
#define CODECA_RESET_GPIO GPIOD
#define CODECA_RESET_HIGH CODECA_RESET_GPIO->BSRR = CODECA_RESET_pin
#define CODECA_RESET_LOW CODECA_RESET_GPIO->BSRR = (CODECA_RESET_pin << 16)

// CODEC B (DLD side B)
// CODECB is SAI2, synced to SAI1 [on sch: I2S2 synced to I2S3]
#define CODECB_RESET_RCC_ENABLE __HAL_RCC_GPIOG_CLK_ENABLE
#define CODECB_RESET_pin GPIO_PIN_3
#define CODECB_RESET_GPIO GPIOG
#define CODECB_RESET_HIGH CODECB_RESET_GPIO->BSRR = CODECB_RESET_pin
#define CODECB_RESET_LOW CODECB_RESET_GPIO->BSRR = (CODECB_RESET_pin << 16)

#define CODEC_FLAG_TIMEOUT ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))

void Codec_A_CtrlInterface_Init(void);
void Codec_B_CtrlInterface_Init(void);

void Codec_A_AudioInterface_Init(uint32_t AudioFreq);
void Codec_B_AudioInterface_Init(uint32_t AudioFreq);

uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue, I2C_TypeDef *CODEC);

void Codecs_Deinit(void);

void Codec_GPIO_Init(void);

void init_i2s_clkin(void);

#endif
