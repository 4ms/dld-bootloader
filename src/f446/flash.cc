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

void flash_copy_memory(uint32_t src_addr, uint32_t dst_addr, size_t size) {
	for (size_t written = 0; written < size; written += 4) {
		//check if dst_addr is the start of a sector (in which case we should erase the sector)
		for (int32_t i = 0; i < NumFlashSectors; ++i) {
			if (dst_addr == get_sector_addr(i)) {
				_erase_sector(i);
				break;
			}
		}

		//Program the word
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, dst_addr, *(uint32_t *)src_addr);

		src_addr += 4;
		dst_addr += 4;
	}
}

bool flash_program_page(uint32_t current_address, const uint8_t *data, size_t size) {
	HAL_FLASH_Unlock();

	for (int32_t i = 0; i < NumFlashSectors; ++i) {
		if (current_address == get_sector_addr(i)) {
			_erase_sector(i);
		}
	}
	const uint32_t *words = static_cast<const uint32_t *>(static_cast<const void *>(data));
	for (size_t written = 0; written < size; written += 4) {
		FLASH_ProgramWord(current_address, *words++);
		current_address += 4;
		if (current_address >= get_sector_addr(NumFlashSectors)) {
			HAL_FLASH_Lock();
			return false;
		}
	}
	HAL_FLASH_Lock();
	return true;
}

bool flash_write_page(const uint8_t *data, uint32_t current_address, size_t size) {
	return flash_program_page(current_address, data, size);
}
