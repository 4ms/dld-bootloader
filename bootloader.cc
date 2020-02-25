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

#include "stm32f4xx.h"

#include "system.h"

#include <cstring>

#include "dsp.h"
#include "ring_buffer.h"
#include "bootloader_utils.h"
#include "flash_programming.h"
#include "system_clock.h"

#include "encoding/fsk/packet_decoder.h"
#include "encoding/fsk/demodulator.h"

extern "C" {
#include <stddef.h> 
#include "dig_inouts.h"
#include "codec.h"
#include "i2s.h"

#define delay(x)						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < x; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)

}

using namespace stmlib;
using namespace stm_audio_bootloader;

const float kSampleRate = 48000.0;
uint32_t kStartExecutionAddress =		0x08008000;
uint32_t kStartReceiveAddress = 		0x08080000;
uint32_t EndOfMemory =					0x080FFFFC;

extern "C" {
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void NMI_Handler(void) { }
void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }
}

smr::System sys;
PacketDecoder decoder;
Demodulator demodulator;

uint16_t packet_index;
uint16_t old_packet_index=0;

bool g_error;

enum UiState {
	UI_STATE_WAITING,
	UI_STATE_RECEIVING,
	UI_STATE_ERROR,
	UI_STATE_WRITING
};
volatile UiState ui_state;

uint16_t manual_exit_primed;
bool exit_updater;

//extern "C" {

inline void *memcpy(void *dest, const void *src, size_t n)
{
    char *dp = (char *)dest;
    const char *sp = (const char *)src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

void update_LEDs(void){
	static uint16_t dly=0;
	uint16_t fade_speed=800;
	uint8_t pck_ctr=0;

	if (ui_state == UI_STATE_RECEIVING){
		if (dly++>400){
			dly=0;
			LED_OVLD1_OFF;

		} else if (dly==200){
			LED_OVLD1_ON;
		}

	} else if (ui_state == UI_STATE_WRITING){

		if (dly++>400){
			dly=0;
			LED_OVLD2_OFF;

		} else if (dly==200){
			LED_OVLD2_ON;
		}

	} else if (ui_state == UI_STATE_WAITING){

		if (dly==(fade_speed>>1)){
			LED_INF1_ON;
			LED_OVLD2_ON;
		}
		if (dly++==fade_speed) {dly=0;
			LED_INF1_OFF;
		}
	}
}


void check_button(void){
	static uint16_t State=0;
	uint16_t t;

	//Depressed adds a 0, released adds a 1

	if (INF1BUT) t=0xe000; else t=0xe001; //1110 0000 0000 000(0|1)
	State=(State<<1) | t;

	if (State == 0xff00)  	//Released event (depressed followed by released)
		manual_exit_primed=1;

	if (State == 0xe00f){ 				 //Depressed event (released followed by a depressed)
		if (packet_index==0 && manual_exit_primed==1)
			exit_updater=1;
	}

}

extern "C" {

void SysTick_Handler() {
	system_clock.Tick();  // Tick global ms counter.
	update_LEDs();
	check_button();
}

void process_audio_block(int16_t *input, int16_t *output, uint16_t ht, uint16_t size){
	static uint16_t discard_samples = 8000;
	bool sample;
	static bool last_sample=false;
	int32_t t;
	//int32_t sample;

	while (size) {
		size-=4;

		*input++; //Return
		*input++; //Return

		t=*input; //Main in
		*input++;//Main in
		*input++;//Main in

		if (last_sample==true){
			if (t < -300)
				sample=false;
			else
				sample=true;
		} else {
			if (t > 400)
				sample=true;
			else
				sample=false;
		}
		last_sample=sample;


		if (sample) CLKOUT_ON;
		else CLKOUT_OFF;

		if (!discard_samples) {
			demodulator.PushSample(sample);
		} else {
			--discard_samples;
		}


		if (ui_state == UI_STATE_ERROR)
		{
			*output++=0;
			*output++=0;
			*output++=0;
			*output++=0;
		}
		else
		{
			*output++=t;
			*output++=0;
			*output++=t;
			*output++=0;
		}


	}

}

} //extern C

static uint32_t current_address;
static uint32_t kSectorBaseAddress[] = {
  0x08000000,
  0x08004000,
  0x08008000,
  0x0800C000,
  0x08010000,
  0x08020000,
  0x08040000,
  0x08060000,
  0x08080000,
  0x080A0000,
  0x080C0000,
  0x080E0000
};
const uint32_t kBlockSize = 16384;
const uint16_t kPacketsPerBlock = kBlockSize / kPacketSize;
uint8_t recv_buffer[kBlockSize];


inline void CopyMemory(uint32_t src_addr, uint32_t dst_addr, size_t size) {

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
				  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);


	for (size_t written = 0; written < size; written += 4) {

		//check if dst_addr is the start of a sector (in which case we should erase the sector)
		for (int32_t i = 0; i < 12; ++i) {
			if (dst_addr == kSectorBaseAddress[i]) {

				LED_INF1_OFF;	LED_INF2_OFF;	LED_PINGBUT_OFF;	LED_OVLD1_OFF;	LED_OVLD2_OFF;
				FLASH_EraseSector(i * 8, VoltageRange_3);
				LED_INF1_ON;	LED_INF2_ON;	LED_PINGBUT_ON;	LED_OVLD1_ON;	LED_OVLD2_ON;

			}
		}

		//Boundary check
		if (dst_addr > (kStartReceiveAddress-4)) //Do not overwrite receive buffer
			break;

		//Program the word
		FLASH_ProgramWord(dst_addr, *(uint32_t*)src_addr);

		src_addr += 4;
		dst_addr += 4;
	}

}


inline void ProgramPage(const uint8_t* data, size_t size) {
	LED_PINGBUT_ON;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
				  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
	for (int32_t i = 0; i < 12; ++i) {
		if (current_address == kSectorBaseAddress[i]) {
		  FLASH_EraseSector(i * 8, VoltageRange_3);
		}
	}
	const uint32_t* words = static_cast<const uint32_t*>(static_cast<const void*>(data));
	for (size_t written = 0; written < size; written += 4) {
		FLASH_ProgramWord(current_address, *words++);
		current_address += 4;
		if (current_address>=EndOfMemory){
			ui_state = UI_STATE_ERROR;
			g_error=true;
			break;
		}
	}
	LED_PINGBUT_OFF;
}

void init_audio_in(){

	Codec_Init_Reset_GPIO();
	Codec_Deinit();
	do {register unsigned int i; for (i = 0; i < 1000000; ++i) __asm__ __volatile__ ("nop\n\t":::"memory");} while (0);

	//QPSK or Codec
	Codec_Init(48000);

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

#define BOOTLOADER_BUTTON (\
		REVSW_CH1 && \
		REVSW_CH2 && \
		!INF1BUT &&\
		!INF2BUT &&\
		PINGBUT\
		)

int main(void) {
	uint32_t symbols_processed=0;
	uint32_t dly=0, button_debounce=0;

	delay(25000);

	Init();
	InitializeReception(); //FSK

	LED_OVLD2_OFF;
	LED_OVLD1_ON;

	dly=32000;
	while(dly--){
		if (BOOTLOADER_BUTTON) button_debounce++;
		else button_debounce=0;
	}
	exit_updater = (button_debounce>15000) ? 0 : 1;
	LED_OVLD1_OFF;

	if (!exit_updater){
		LED_INF1_ON;
		LED_INF2_ON;
		init_audio_in(); //QPSK or Codec
		sys.StartTimers();
	}

	LED_OVLD2_ON;
	dly=4000;
	while(dly--){
		if (BOOTLOADER_BUTTON) button_debounce++;
		else button_debounce=0;
	}
	exit_updater = (button_debounce>2000) ? 0 : 1;

	manual_exit_primed=0;
	LED_INF2_OFF;
	LED_OVLD2_OFF;

	while (!exit_updater) {
		g_error = false;

		while (demodulator.available() && !g_error && !exit_updater) {
			uint8_t symbol = demodulator.NextSymbol();
			PacketDecoderState state = decoder.ProcessSymbol(symbol);
			symbols_processed++;

			switch (state) {
				case PACKET_DECODER_STATE_OK:
				{
					ui_state = UI_STATE_RECEIVING;
					memcpy(recv_buffer + (packet_index % kPacketsPerBlock) * kPacketSize, decoder.packet_data(), kPacketSize);
					++packet_index;
					if ((packet_index % kPacketsPerBlock) == 0) {
						ui_state = UI_STATE_WRITING;
						ProgramPage(recv_buffer, kBlockSize);
						decoder.Reset();
						demodulator.Sync(); //FSK
					} else {
						decoder.Reset(); //FSK
					}
				}
				break;

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

					CopyMemory(kStartReceiveAddress, kStartExecutionAddress, (current_address-kStartReceiveAddress));
					break;

				default:
					break;
			}
		}
		if (g_error) {
			ui_state = UI_STATE_ERROR;

			while (!REVSW_CH1){;}

			while (REVSW_CH1){;}

			LED_INF1_OFF;	LED_INF2_OFF;	LED_PINGBUT_OFF;	LED_OVLD1_OFF;	LED_OVLD2_OFF;

			InitializeReception();
			manual_exit_primed=0;
			exit_updater=false;
		}
	}

	LED_INF1_OFF;	LED_INF2_OFF;	LED_PINGBUT_OFF;	LED_OVLD1_OFF;	LED_OVLD2_OFF;

	Codec_PowerDown();
	Codec_Deinit();
	delay(25000);

	Uninitialize();

	JumpTo(kStartExecutionAddress);
}

