// Copyright 2020 Dan Green
//
// Author: Dan Green (danngreen1@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------

#include "bootloader_utils.h"
#include "stm32f4xx_conf.h"

void reset_buses() {
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOA, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOB, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOC, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOD, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOE, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOF, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOG, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOG, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOI, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOJ, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOK, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_CRC, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2D, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_ETH_MAC, DISABLE);
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_OTG_HS, DISABLE);
#ifdef RCC_AHB1Periph_RNG
	RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_RNG, DISABLE);
#endif
	RCC_AHB2PeriphResetCmd(RCC_AHB2Periph_DCMI, DISABLE);
	RCC_AHB2PeriphResetCmd(RCC_AHB2Periph_CRYP, DISABLE);
	RCC_AHB2PeriphResetCmd(RCC_AHB2Periph_HASH, DISABLE);
	RCC_AHB2PeriphResetCmd(RCC_AHB2Periph_RNG, DISABLE);

#ifdef RCC_AHB3Periph_FMC
	RCC_AHB3PeriphResetCmd(RCC_AHB3Periph_FMC, DISABLE);
#endif
#ifdef RCC_AHB3Periph_FSMC
	RCC_AHB3PeriphResetCmd(RCC_AHB3Periph_FSMC, DISABLE);
#endif
#ifdef RCC_AHB3Periph_QSPI
	RCC_AHB3PeriphResetCmd(RCC_AHB3Periph_QSPI, DISABLE);
#endif

	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM2, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM4, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM5, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM6, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM7, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM12, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM13, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM14, DISABLE);
#ifdef RCC_APB1Periph_LPTIM1
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_LPTIM1, DISABLE);
#endif
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_WWDG, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, DISABLE);
#ifdef RCC_APB1Periph_SPDIF
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPDIF, DISABLE);
#endif
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART4, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, DISABLE);
#ifdef RCC_APB1Periph_FMPI2C1
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_FMPI2C1, DISABLE);
#endif
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN2, DISABLE);
#ifdef RCC_APB1Periph_CEC
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_CEC, DISABLE);
#endif
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_DAC, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART7, DISABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART8, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM1, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM8, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART6, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC2, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI4, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM9, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM10, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_TIM11, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI5, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI6, DISABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SAI1, DISABLE);
#ifdef RCC_APB2Periph_SAI2
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SAI2, DISABLE);
#endif
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_LTDC, DISABLE);
#ifdef RCC_APB2Periph_DSI
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_DSI, DISABLE);
#endif
}

void reset_RCC() {
  //Clear the CTRL bits that SysTick_Config() set
  SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk |
                     SysTick_CTRL_ENABLE_Msk);

  RCC_DeInit();

  //Simplification of SystemCoreClockUpdate()
//  SystemCoreClock = HSI_VALUE;
}

typedef void (*EntryPoint)(void);

void JumpTo(uint32_t address) {
  uint32_t application_address = *(__IO uint32_t*)(address + 4);
  EntryPoint application = (EntryPoint)(application_address);
  __set_MSP(*(__IO uint32_t*)address);
  application();
}

