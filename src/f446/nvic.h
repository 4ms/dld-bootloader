#include "stm32f4xx.h"

inline void set_vect_table(uint32_t addr) {
	SCB->VTOR = addr;
}
