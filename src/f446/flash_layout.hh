#include <cstdint>

constexpr inline uint32_t kSectorBaseAddress[] = {0x08000000,
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
												  0x080E0000};
constexpr inline uint32_t kStartExecutionAddress = 0x08008000;
constexpr inline uint32_t kStartReceiveAddress = 0x08080000;
constexpr inline uint32_t EndOfMemory = 0x080FFFFC;
