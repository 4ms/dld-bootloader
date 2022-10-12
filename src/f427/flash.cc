#include "flash_layout.hh"
#include "stm32f4xx.h"
#include <cstddef>
#include <cstdint>

void flash_copy_memory(uint32_t src_addr, uint32_t dst_addr, size_t size) {
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |
					FLASH_FLAG_PGSERR);

	for (size_t written = 0; written < size; written += 4) {

		//check if dst_addr is the start of a sector (in which case we should erase the sector)
		for (int32_t i = 0; i < 12; ++i) {
			if (dst_addr == get_sector_addr(i)) {
				FLASH_EraseSector(i * 8, VoltageRange_3);
				break;
			}
		}

		//Boundary check
		if (dst_addr > (BootloaderReceiveAddr - 4)) //Do not overwrite receive buffer
			break;

		//Program the word
		FLASH_ProgramWord(dst_addr, *(uint32_t *)src_addr);

		src_addr += 4;
		dst_addr += 4;
	}
}

bool flash_program_page(uint32_t current_address, const uint8_t *data, size_t size) {
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |
					FLASH_FLAG_PGSERR);
	for (int32_t i = 0; i < 12; ++i) {
		if (current_address == get_sector_addr(i)) {
			FLASH_EraseSector(i * 8, VoltageRange_3);
		}
	}
	const uint32_t *words = static_cast<const uint32_t *>(static_cast<const void *>(data));
	for (size_t written = 0; written < size; written += 4) {
		FLASH_ProgramWord(current_address, *words++);
		current_address += 4;
		if (current_address >= get_sector_addr(NumFlashSectors))
			return false;
	}
	return true;
}

bool flash_write_page(const uint8_t *data, uint32_t current_address, size_t size) {
	return flash_program_page(current_address, data, size);
}
