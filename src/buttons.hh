#include "dig_pins.h"

static inline void init_buttons() {
	// init_dig_inouts();
	// DigIO::PingBut pingbutinit(mdrivlib::PinPull::Up);
	// DigIO::CycleBut cyclebutinit(mdrivlib::PinPull::Up);
}

enum class Button { RevA, RevB, Ping, InfA, InfB };

static inline bool button_pushed(Button button) {
	switch (button) {
		case Button::RevA:
			return REV1BUT;
		case Button::RevB:
			return REV2BUT;
		case Button::Ping:
			return PINGBUT;
		case Button::InfA:
			return INF1BUT;
		case Button::InfB:
			return INF2BUT;
		default:
			return false;
	}
}
