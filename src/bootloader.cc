#include "stm32f4xx.h"

#define USING_FSK
// #define USING_QPSK

#include "animation.hh"
#include "bl_utils.hh"
#include "bootloader_settings.hh"
#include "bootloader_utils.h"
#include "buttons.hh"
#include "dig_pins.h"
#include "flash.hh"
#include "flash_layout.hh"
#include "nvic.h"
#include "reception.hh"
#include "system.hh"
#include "ui_state.hh"

#ifdef USING_QPSK
#include "encoding/qpsk/demodulator.h"
#include "encoding/qpsk/packet_decoder.h"
#else
#include "encoding/fsk/demodulator.h"
#include "encoding/fsk/packet_decoder.h"
#endif

using namespace stmlib;
using namespace stm_audio_bootloader;

#ifdef USING_QPSK
constexpr float kModulationRate = 6000.0;
constexpr float kBitRate = 12000.0;
constexpr float kSampleRate = 48000.0;
#else
constexpr uint32_t kSampleRate = BootloaderConf::SampleRate;	  //-s
constexpr uint32_t kPausePeriod = BootloaderConf::Encoding.blank; //-b
constexpr uint32_t kOnePeriod = BootloaderConf::Encoding.one;	  //-n
constexpr uint32_t kZeroPeriod = BootloaderConf::Encoding.zero;	  //-z
																  //-p must be 256 (set in fsk/packet_decoder.h)
#endif

constexpr uint32_t kStartExecutionAddress = AppFlashAddr;
constexpr uint32_t kStartReceiveAddress = BootloaderReceiveAddr;
constexpr uint32_t kBlkSize = BootloaderConf::ReceiveSectorSize; //Flash page size, -g

constexpr uint16_t kPacketsPerBlock = kBlkSize / kPacketSize; //kPacketSize=256
uint8_t recv_buffer[kBlkSize];

volatile uint32_t systmr = 0;
PacketDecoder decoder;
Demodulator demodulator;

uint16_t packet_index;
uint16_t discard_samples = 8000;
uint32_t current_flash_address;

UiState ui_state;

static void animate_until_button_pushed(Animations animation_type, Button button);
static void update_LEDs();
static void init_reception();
static void delay(uint32_t tm);
static bool write_buffer();
static void new_block();
static void new_packet();

void main() {
	uint32_t symbols_processed = 0;
	uint32_t dly = 0, button_debounce = 0;
	uint8_t do_bootloader;
	uint8_t symbol;
	PacketDecoderState state;
	bool rcv_err;
	uint8_t exit_updater = false;

	set_vect_table(BootloaderFlashAddr);
	system_init();

	delay(300);

	init_dig_inouts();

	animate(ANI_RESET);

	dly = 32000;
	while (dly--) {
		if (button_pushed(Button::RevA) && button_pushed(Button::RevB) && button_pushed(Button::Ping) &&
			!button_pushed(Button::InfA) && !button_pushed(Button::InfB))
			button_debounce++;
		else
			button_debounce = 0;
	}
	do_bootloader = (button_debounce > 15000) ? 1 : 0;

	delay(100);

	//Debug:
	// DigIO::ClockBusOut init;
	// mdrivlib::FPin<mdrivlib::GPIO::E, mdrivlib::PinNum::_3, mdrivlib::PinMode::Output> debug2;
	// debug2.low();
	// DigIO::ClockBusOut::low();

	if (do_bootloader) {
		init_reception();

		start_reception();
		// start_reception(kSampleRate, []() {
		// 	bool sample = gate_in_read(gatein_threshold);
		// 	if (!discard_samples) {
		// 		demodulator.PushSample(sample ? 1 : 0);
		// 	} else {
		// 		--discard_samples;
		// 	}
		// });

		uint32_t button1_exit_armed = 0;
		uint32_t cycle_but_armed = 0;

		while (button_pushed(Button::RevA))
			;

		delay(300);

		while (!exit_updater) {
			rcv_err = false;

			while (demodulator.available() && !rcv_err && !exit_updater) {
				symbol = demodulator.NextSymbol();
				state = decoder.ProcessSymbol(symbol);
				symbols_processed++;

				switch (state) {
					case PACKET_DECODER_STATE_SYNCING:
						animate(ANI_SYNC);
						break;

					case PACKET_DECODER_STATE_OK:
						ui_state = UI_STATE_RECEIVING;
						memcpy(recv_buffer + (packet_index % kPacketsPerBlock) * kPacketSize,
							   decoder.packet_data(),
							   kPacketSize);
						++packet_index;
						if ((packet_index % kPacketsPerBlock) == 0) {
							ui_state = UI_STATE_WRITING;
							bool write_ok = write_buffer();
							if (!write_ok) {
								ui_state = UI_STATE_ERROR;
								rcv_err = true;
							}
							new_block();
						} else {
							new_packet();
						}
						break;

					case PACKET_DECODER_STATE_ERROR_SYNC:
						rcv_err = true;
						break;

					case PACKET_DECODER_STATE_ERROR_CRC:
						rcv_err = true;
						break;

					case PACKET_DECODER_STATE_END_OF_TRANSMISSION:
						// If our block size is as big as our transmission,
						// then we might get to the end and have never written anything to
						// flash (we know this is the case if we never incremented current_flash_address)
						// So, write out our buffer now
						if (current_flash_address == kStartReceiveAddress) {
							if (!write_buffer()) {
								ui_state = UI_STATE_ERROR;
								rcv_err = true;
								new_block();
								break;
							}
						}

						// Copy the received data to the execution sectors if needed:
						if (BootloaderReceiveAddr != AppFlashAddr) {
							flash_copy_memory(
								BootloaderReceiveAddr, AppFlashAddr, (current_flash_address - BootloaderReceiveAddr));
						}

						exit_updater = true;
						ui_state = UI_STATE_DONE;
						animate_until_button_pushed(ANI_SUCCESS, Button::RevA);
						animate(ANI_RESET);
						delay(100);
						break;

					default:
						break;
				}
			}
			if (rcv_err) {
				ui_state = UI_STATE_ERROR;
				animate_until_button_pushed(ANI_FAIL_ERR, Button::RevA);
				animate(ANI_RESET);
				delay(100);
				init_reception();
				exit_updater = false;
			}

			if (button_pushed(Button::Ping)) {
				if (button1_exit_armed) {
					if (ui_state == UI_STATE_WAITING) {
						exit_updater = true;
					}
				}
				button1_exit_armed = 0;
			} else
				button1_exit_armed = 1;
		}
		ui_state = UI_STATE_DONE;
		while (button_pushed(Button::Ping) || button_pushed(Button::RevA)) {
			;
		}
	}

	reset_buses();
	reset_RCC();
	jump_to(kStartExecutionAddress);
}

void init_reception() {
#ifdef USING_QPSK
	//QPSK
	decoder.Init((uint16_t)20000);
	demodulator.Init(
		kModulationRate / kSampleRate * 4294967296.0f, kSampleRate / kModulationRate, 2.f * kSampleRate / kBitRate);
	demodulator.SyncCarrier(true);
	decoder.Reset();
#else
	//FSK
	decoder.Init();
	decoder.Reset();
	demodulator.Init(kPausePeriod, kOnePeriod, kZeroPeriod); // pause_thresh = 24. one_thresh = 6.
	demodulator.Sync();
#endif

	current_flash_address = kStartReceiveAddress;
	packet_index = 0;
	ui_state = UI_STATE_WAITING;
}

bool write_buffer() {
	if ((current_flash_address + kBlkSize) <= get_sector_addr(NumFlashSectors)) {
		// flash_write_page(recv_buffer, current_flash_address, kBlkSize);
		flash_program_page(current_flash_address, recv_buffer, kBlkSize);
		current_flash_address += kBlkSize;
		return true;
	} else {
		return false;
	}
}

void update_LEDs() {
	if (ui_state == UI_STATE_RECEIVING)
		animate(ANI_RECEIVING);

	else if (ui_state == UI_STATE_WRITING)
		animate(ANI_WRITING);

	else if (ui_state == UI_STATE_WAITING)
		animate(ANI_WAITING);

	else //if (ui_state == UI_STATE_DONE)
	{}
}

void new_block() {
	decoder.Reset();
#ifdef USING_FSK
	demodulator.Sync(); //FSK
#else
	demodulator.SyncCarrier(false); //QPSK
#endif
}

void new_packet() {
#ifdef USING_FSK
	decoder.Reset(); //FSK
#else
	demodulator.SyncDecision();		//QPSK
#endif
}

void animate_until_button_pushed(Animations animation_type, Button button) {
	animate(ANI_RESET);

	while (!button_pushed(button)) {
		delay(1);
		animate(animation_type);
	}
	while (button_pushed(button)) {
		delay(1);
	}
}

void delay(uint32_t ticks) {
	uint32_t i = systmr;
	while ((systmr - i) < ticks) {
		;
	}
}

extern "C" void SysTick_Handler(void) {
	systmr = systmr + 1;
	update_LEDs();
}

void panic() {
	// NVIC_SystemReset();
	panic();
}

extern "C" void NMI_Handler() {
	panic();
}
extern "C" void HardFault_Handler() {
	panic();
}
extern "C" void MemManage_Handler() {
	panic();
}
extern "C" void BusFault_Handler() {
	panic();
}
extern "C" void UsageFault_Handler() {
	panic();
}
extern "C" void SVC_Handler() {
	panic();
}
extern "C" void DebugMon_Handler() {
	panic();
}
extern "C" void PendSV_Handler() {
	panic();
}
