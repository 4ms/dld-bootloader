/*
 * flash.c - mid-level Flash r/w fuctions
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */

#include "flash_layout.hh"
#include "stm32f4xx.h"

typedef unsigned FlashStatus;

static FlashStatus _erase_sector(uint32_t sector) {
	FLASH_EraseInitTypeDef erase_conf = {
		.TypeErase = FLASH_TYPEERASE_SECTORS,
		.Banks = FLASH_BANK_1,
		.Sector = sector,
		.NbSectors = 1,
		.VoltageRange = FLASH_VOLTAGE_RANGE_3,
	};

	uint32_t err;
	return HAL_FLASHEx_Erase(&erase_conf, &err);
}

FlashStatus flash_erase_sector(uint32_t address) {
	HAL_FLASH_Unlock();
	for (uint32_t i = 0; i < 8; ++i) {
		if (address == kSectorBaseAddress[i]) {
			return _erase_sector(i);
		}
	}
	HAL_FLASH_Lock();

	return 99; // address out of range
}

FlashStatus flash_open_erase_sector(uint32_t address) {
	for (uint32_t i = 0; i < 8; ++i) {
		if (address == kSectorBaseAddress[i]) {
			return _erase_sector(i);
		}
	}
	return 99; // address out of range
}

void flash_begin_open_program(void) {
	HAL_FLASH_Unlock();
}

FlashStatus flash_open_program_byte(uint8_t byte, uint32_t address) {
	if (address < kSectorBaseAddress[1])
		return HAL_FLASH_ERROR_PGP;
	else
		return HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, byte);
}

FlashStatus flash_open_program_word(uint32_t word, uint32_t address) {
	if (address < kSectorBaseAddress[1])
		return HAL_FLASH_ERROR_PGP;
	else
		return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word);
}

void flash_end_open_program(void) {
	HAL_FLASH_Lock();
}

// size is in # of bytes
FlashStatus flash_open_program_array(uint8_t *arr, uint32_t address, uint32_t size) {
	FlashStatus status;

	while (size--) {
		if (address >= kSectorBaseAddress[1])
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, *arr);
		*arr++;
		address++;
	}
	return status;
}

// size in # of bytes
void flash_read_array(uint8_t *arr, uint32_t address, uint32_t size) {

	while (size--) {
		*arr++ = (uint8_t)(*(__IO uint32_t *)address);
		address++;
	}
}

uint32_t flash_read_word(uint32_t address) {
	return (*(__IO uint32_t *)address);
}

uint8_t flash_read_byte(uint32_t address) {
	return ((uint8_t)(*(__IO uint32_t *)address));
}

void flash_copy_memory(uint32_t src_addr, uint32_t dst_addr, size_t size) {
	// FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |
	// 				FLASH_FLAG_PGSERR);

	for (size_t written = 0; written < size; written += 4) {

		//check if dst_addr is the start of a sector (in which case we should erase the sector)
		for (int32_t i = 0; i < 12; ++i) {
			if (dst_addr == kSectorBaseAddress[i]) {
				FLASH_EraseSector(i * 8, VoltageRange_3);
				break;
			}
		}

		//Boundary check
		if (dst_addr > (kStartReceiveAddress - 4)) //Do not overwrite receive buffer
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
		if (current_address == kSectorBaseAddress[i]) {
			FLASH_EraseSector(i * 8, VoltageRange_3);
		}
	}
	const uint32_t *words = static_cast<const uint32_t *>(static_cast<const void *>(data));
	for (size_t written = 0; written < size; written += 4) {
		FLASH_ProgramWord(current_address, *words++);
		current_address += 4;
		if (current_address >= EndOfMemory)
			return false;
	}
	return true;
}
