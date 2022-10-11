#include <stddef.h>
#include <stdint.h>

void flash_copy_memory(uint32_t src_addr, uint32_t dst_addr, size_t size);
bool flash_program_page(uint32_t current_address, const uint8_t *data, size_t size);
