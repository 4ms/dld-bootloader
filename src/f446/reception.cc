#include "codec_CS4271.h"
#include "dig_pins.h"
#include "encoding/fsk/demodulator.h"
#include "i2s.h"
#include "ui_state.hh"

static void process_audio_block(int16_t *input, int16_t *output, uint16_t size);

void start_reception() {
	codec_reset_pin_init();
	codec_gpio_init();
	codec_sai_init(48000);

	Init_I2SDMA_Channel();

	codec_register_setup();
	set_codec_callback(process_audio_block);

	Start_I2SDMA_Channel();
}

extern stm_audio_bootloader::Demodulator demodulator;
extern volatile UiState ui_state;

void process_audio_block(int16_t *input, int16_t *output, uint16_t size) {
	static uint16_t discard_samples = 8000;
	static bool last_sample = false;
	bool sample;
	int32_t t;

	while (size) {
		size -= 4;

		*input++;	//Return
		*input++;	//Return
		t = *input; //Return

		*input++; //Main in
		*input++; //Main in

		// Anti-hysteresis: set threshold for true->false low
		// And set the threshold for false->true high
		if (last_sample == true)
			sample = (t > -300);
		else
			sample = (t > 400);

		last_sample = sample;

		// if (sample)
		// 	CLKOUT_ON;
		// else
		// 	CLKOUT_OFF;

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
			*output++ = 0;
			*output++ = 0;
			*output++ = sample ? 0x4000 : 0xC000;
			*output++ = 0;
		}
	}
}
