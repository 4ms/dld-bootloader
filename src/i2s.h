/*
 * i2s.h - I2S feeder routines
 */

#ifndef __i2s__
#define __i2s__

void DeInit_I2SDMA(void);
void Init_I2SDMA_Channel(void);
void Start_I2SDMA_Channel(void);

void set_codec_callback(void (*cb)(int16_t *, int16_t *, uint16_t));

#endif
