/*
 * i2s.h - I2S feeder routines
 */

#ifndef __i2s__
#define __i2s__

void I2S_Block_Init(void);
void I2S_Block_PlayRec(void);
void DeInit_I2SDMA(void);
void Init_I2SDMA_Channel(void);

#endif
