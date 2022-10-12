#include "animation.hh"
#include "bootloader_settings.hh"
#include "dig_pins.h"
#include <cstdint>

extern volatile uint32_t systmr;

void animate(Animations animation_type) {
	auto TICKS_PER_MS = BootloaderConf::TICKS_PER_MS;

	uint32_t cur_tm = systmr;
	static uint32_t last_tm = 0;
	static uint8_t ctr = 0;
	uint32_t step_time = 500 * TICKS_PER_MS;

	switch (animation_type) {

		case ANI_RESET:
			last_tm = cur_tm;
			ctr = 0;
			LED_LOOP1_OFF;
			LED_LOOP2_OFF;
			LED_INF1_OFF;
			LED_INF2_OFF;
			LED_REV1_OFF;
			LED_REV2_OFF;
			LED_PINGBUT_OFF;
			break;

		case ANI_SUCCESS:
			step_time = 200 * TICKS_PER_MS;
			if (ctr == 0) {
				LED_PINGBUT_ON;

				LED_REV2_OFF;
				LED_REV1_ON;
			} else if (ctr == 1) {
				LED_REV1_OFF;
				LED_INF1_ON;
			} else if (ctr == 2) {
				LED_PINGBUT_OFF;

				LED_INF1_OFF;
				LED_INF2_ON;
			} else if (ctr == 3) {
				LED_INF2_OFF;
				LED_REV2_ON;
			} else
				ctr = 0;
			break;

		case ANI_WAITING:
			LED_LOOP1_OFF;
			LED_LOOP2_OFF;
			if (ctr == 0) {
				LED_PINGBUT_ON;
			} else if (ctr == 1) {
				LED_PINGBUT_OFF;
			} else
				ctr = 0;
			break;

		case ANI_RECEIVING:
			step_time = 100 * TICKS_PER_MS;
			if (ctr == 0) {
				LED_LOOP1_ON;
			} else if (ctr == 1) {
				LED_LOOP1_OFF;
			} else
				ctr = 0;
			break;

		case ANI_SYNC:
			step_time = 50 * TICKS_PER_MS;
			if (ctr == 0) {
				LED_LOOP1_ON;
				LED_LOOP2_ON;
			} else if (ctr == 1) {
				LED_LOOP1_OFF;
				LED_LOOP2_OFF;
			} else
				ctr = 0;
			break;

		case ANI_WRITING:
			step_time = 50 * TICKS_PER_MS;
			if (ctr == 0) {
				LED_LOOP2_ON;
			} else if (ctr == 1) {
				LED_LOOP2_OFF;
			} else
				ctr = 0;
			break;

		case ANI_FAIL_ERR:
			LED_LOOP1_ON;
			LED_LOOP2_ON;
			LED_INF1_ON;
			LED_INF2_ON;
			LED_REV2_ON;
			LED_PINGBUT_ON;
			if (ctr == 0) {
				LED_REV1_ON;
			} else if (ctr == 1) {
				LED_REV1_OFF;
			} else
				ctr = 0;
			break;
			break;

		default:
			break;
	}

	if ((cur_tm - last_tm) > step_time) {
		ctr++;
		last_tm = cur_tm;
	}
}
