#include <cstdint>

volatile uint32_t systmr = 0;

void delay(uint32_t ticks) {
	uint32_t i = systmr;
	while ((systmr - i) < ticks) {
		;
	}
}
void inc_systmr() {
	systmr = systmr + 1;
}
