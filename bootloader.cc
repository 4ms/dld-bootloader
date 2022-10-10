// Bootloader.cc
// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
// Modified for DLD project: Dan Green (danngreen1@gmail.com) 2016

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

#include "flash.hh"
#include "flash_layout.hh"
#include "stm32f4xx.h"
#include "system.h"

#include <cstring>

#include "bootloader_utils.h"
#include "dsp.h"
#include "flash_programming.h"
#include "ring_buffer.h"
#include "system_clock.h"

#include "encoding/fsk/demodulator.h"
#include "encoding/fsk/packet_decoder.h"

extern "C" {
#include "codec_CS4271.h"
#include "dig_pins.h"
#include "i2s.h"
#include <stddef.h>
}

static inline void delay(uint32_t x) {
	while (x--)
		__asm__ __volatile__("nop\n\t" ::: "memory");
}

using namespace stmlib;
using namespace stm_audio_bootloader;

const float kSampleRate = 48000.0;

static inline void handle_fault(void) {
	//TODO: Reset system if not in debug mode
	while (1)
		;
}

extern "C" {
void HardFault_Handler(void) {
	handle_fault();
}
void MemManage_Handler(void) {
	handle_fault();
}
void BusFault_Handler(void) {
	handle_fault();
}
void UsageFault_Handler(void) {
	handle_fault();
}
void NMI_Handler(void) {
	handle_fault();
}
void SVC_Handler(void) {
	handle_fault();
}
void DebugMon_Handler(void) {
	handle_fault();
}
void PendSV_Handler(void) {
	handle_fault();
}
}

smr::System sys;
PacketDecoder decoder;
Demodulator demodulator;

uint16_t packet_index;
uint16_t old_packet_index = 0;

bool g_error;

enum UiState { UI_STATE_WAITING, UI_STATE_RECEIVING, UI_STATE_ERROR, UI_STATE_WRITING };
volatile UiState ui_state;

uint16_t manual_exit_primed;
bool exit_updater;

extern "C" inline void *memcpy(void *dest, const void *src, size_t n) {
	char *dp = (char *)dest;
	const char *sp = (const char *)src;
	while (n--)
		*dp++ = *sp++;
	return dest;
}

void update_LEDs(void) {
	static uint16_t dly = 0;
	uint16_t fade_speed = 800;

	if (ui_state == UI_STATE_RECEIVING) {
		if (dly++ > 400) {
			dly = 0;
			LED_OVLD1_OFF;

		} else if (dly == 200) {
			LED_OVLD1_ON;
		}

	} else if (ui_state == UI_STATE_WRITING) {

		if (dly++ > 400) {
			dly = 0;
			LED_OVLD2_OFF;

		} else if (dly == 200) {
			LED_OVLD2_ON;
		}

	} else if (ui_state == UI_STATE_WAITING) {

		if (dly == (fade_speed >> 1)) {
			LED_INF1_ON;
			LED_OVLD2_ON;
		}
		if (dly++ == fade_speed) {
			dly = 0;
			LED_INF1_OFF;
		}
	}
}

void check_button(void) {
	static uint16_t State = 0;
	uint16_t t;

	//Depressed adds a 0, released adds a 1

	if (INF1BUT)
		t = 0xe000;
	else
		t = 0xe001; //1110 0000 0000 000(0|1)
	State = (State << 1) | t;

	if (State == 0xff00) //Released event (depressed followed by released)
		manual_exit_primed = 1;

	if (State == 0xe00f) { //Depressed event (released followed by a depressed)
		if (packet_index == 0 && manual_exit_primed == 1)
			exit_updater = 1;
	}
}

extern "C" void SysTick_Handler() {
	system_clock.Tick(); // Tick global ms counter.
	update_LEDs();
	check_button();
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

static uint32_t current_address;

const uint32_t kBlockSize = 16384;
const uint16_t kPacketsPerBlock = kBlockSize / kPacketSize;
uint8_t recv_buffer[kBlockSize];

inline void CopyMemory(uint32_t src_addr, uint32_t dst_addr, size_t size) {
	LED_INF1_OFF;
	LED_INF2_OFF;
	LED_PINGBUT_OFF;
	LED_OVLD1_OFF;
	LED_OVLD2_OFF;
	flash_copy_memory(src_addr, dst_addr, size);
	LED_INF1_ON;
	LED_INF2_ON;
	LED_PINGBUT_ON;
	LED_OVLD1_ON;
	LED_OVLD2_ON;
}

inline void ProgramPage(const uint8_t *data, size_t size) {
	LED_PINGBUT_ON;
	if (!flash_program_page(current_address, data, size)) {
		g_error = true;
		ui_state = UI_STATE_ERROR;
	}
	LED_PINGBUT_OFF;
}

void init_audio_in() {

	Codec_Init_Reset_GPIO();
	Codec_Deinit();
	delay(1000000);

	//QPSK or Codec
	Codec_Init(48000);

	//TODO: Arch-specific
	NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);
}

void Init() {
	sys.Init(false);
	system_clock.Init();
	init_dig_inouts();
}

void InitializeReception() {
	//FSK

	decoder.Init();
	decoder.Reset();

	demodulator.Init(16, 8, 4);
	demodulator.Sync();

	current_address = kStartReceiveAddress;
	packet_index = 0;
	old_packet_index = 0;
	ui_state = UI_STATE_WAITING;
}

#define BOOTLOADER_BUTTON (REVSW_CH1 && REVSW_CH2 && !INF1BUT && !INF2BUT && PINGBUT)

void main(void) {
	//TODO: Arch-specific:
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);

	uint32_t symbols_processed = 0;
	uint32_t dly = 0, button_debounce = 0;

	bool codec_did_init = false;

	delay(25000);

	Init();
	InitializeReception(); //FSK

	LED_OVLD2_OFF;
	LED_OVLD1_ON;

	dly = 32000;
	while (dly--) {
		if (BOOTLOADER_BUTTON)
			button_debounce++;
		else
			button_debounce = 0;
	}
	exit_updater = (button_debounce > 15000) ? 0 : 1;
	LED_OVLD1_OFF;

	if (!exit_updater) {
		LED_INF1_ON;
		LED_INF2_ON;
		init_audio_in(); //QPSK or Codec
		codec_did_init = true;
		sys.StartTimers();
	}

	/*
	LED_OVLD2_ON;
	dly=4000;
	while(dly--){
		if (BOOTLOADER_BUTTON) button_debounce++;
		else button_debounce=0;
	}
	exit_updater = (button_debounce>2000) ? 0 : 1;
	*/

	manual_exit_primed = 0;
	LED_INF2_OFF;
	LED_OVLD2_OFF;

	while (!exit_updater) {
		g_error = false;

		while (demodulator.available() && !g_error && !exit_updater) {
			uint8_t symbol = demodulator.NextSymbol();
			PacketDecoderState state = decoder.ProcessSymbol(symbol);
			symbols_processed++;

			switch (state) {
				case PACKET_DECODER_STATE_OK: {
					ui_state = UI_STATE_RECEIVING;
					memcpy(recv_buffer + (packet_index % kPacketsPerBlock) * kPacketSize,
						   decoder.packet_data(),
						   kPacketSize);
					++packet_index;
					if ((packet_index % kPacketsPerBlock) == 0) {
						ui_state = UI_STATE_WRITING;
						ProgramPage(recv_buffer, kBlockSize);
						decoder.Reset();
						demodulator.Sync(); //FSK
					} else {
						decoder.Reset(); //FSK
					}
				} break;

				case PACKET_DECODER_STATE_ERROR_SYNC:
					LED_OVLD1_ON;
					g_error = true;
					break;

				case PACKET_DECODER_STATE_ERROR_CRC:
					LED_OVLD2_ON;
					g_error = true;
					break;

				case PACKET_DECODER_STATE_END_OF_TRANSMISSION:
					exit_updater = true;

					CopyMemory(kStartReceiveAddress, kStartExecutionAddress, (current_address - kStartReceiveAddress));
					break;

				default:
					break;
			}
		}
		if (g_error) {
			ui_state = UI_STATE_ERROR;

			while (!REVSW_CH1) {
				;
			}

			while (REVSW_CH1) {
				;
			}

			LED_INF1_OFF;
			LED_INF2_OFF;
			LED_PINGBUT_OFF;
			LED_OVLD1_OFF;
			LED_OVLD2_OFF;

			InitializeReception();
			manual_exit_primed = 0;
			exit_updater = false;
		}
	}

	LED_INF1_OFF;
	LED_INF2_OFF;
	LED_PINGBUT_OFF;
	LED_OVLD1_OFF;
	LED_OVLD2_OFF;

	if (codec_did_init) {
		Codec_PowerDown();
		Codec_Deinit();
	}
	delay(25000);

	//TODO: Arch-specific
	reset_buses();
	reset_RCC();

	JumpTo(kStartExecutionAddress);
}
