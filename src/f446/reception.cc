#include "codec_CS4271.h"
#include "dig_pins.h"
#include "encoding/fsk/demodulator.h"
#include "ui_state.hh"

static void process_audio_block(int16_t *input, int16_t *output, uint16_t ht, uint16_t size);

void start_reception() {
	Codecs_Deinit();
	DeInit_I2S_Clock();
	DeInit_I2SDMA_Channel2();
	DeInit_I2SDMA_Channel1();

	Codec_GPIO_Init();
	Codec_A_AudioInterface_Init(48000);

	init_audio_dma();

	Codec_A_Register_Setup();

	set_codec_callback(process_audio_block);

	Start_I2SDMA_Channel1();
}

extern stm_audio_bootloader::Demodulator demodulator;
extern volatile UiState ui_state;

void process_audio_block(int16_t *input, int16_t *output, uint16_t ht, uint16_t size) {
	static uint16_t discard_samples = 8000;
	static bool last_sample = false;
	bool sample;
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
