#include "codec_CS4271.h"

void start_reception() {
	Codec_Init_Reset_GPIO();
	Codec_Deinit();

	uint32_t x = 1000000;
	while (x--)
		__asm__ __volatile__("nop\n\t" ::: "memory");

	Codec_Init(48000);

	NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);
}

extern "C" void process_audio_block(int16_t *input, int16_t *output, uint16_t ht, uint16_t size) {
	static uint16_t discard_samples = 8000;
	bool sample;
	static bool last_sample = false;
	int32_t t;

	while (size) {
		size -= 4;

		*input++; //Return
		*input++; //Return

		t = *input; //Main in
		*input++;	//Main in
		*input++;	//Main in

		if (last_sample == true) {
			if (t < -300)
				sample = false;
			else
				sample = true;
		} else {
			if (t > 400)
				sample = true;
			else
				sample = false;
		}
		last_sample = sample;

		if (sample)
			CLKOUT_ON;
		else
			CLKOUT_OFF;

		if (!discard_samples) {
			demodulator.PushSample(sample);
		} else {
			--discard_samples;
		}

		if (ui_state == UI_STATE_ERROR) {
			*output++ = 0;
			*output++ = 0;
			*output++ = 0;
			*output++ = 0;
		} else {
			*output++ = t;
			*output++ = 0;
			*output++ = t;
			*output++ = 0;
		}
	}
}
