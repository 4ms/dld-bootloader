#include <cstdint>

// constexpr inline uint32_t kSectorBaseAddress[] = {0x08000000,
// 												  0x08004000,
// 												  0x08008000,
// 												  0x0800C000,
// 												  0x08010000,
// 												  0x08020000,
// 												  0x08040000,
// 												  0x08060000,
// 												  0x08080000,
// 												  0x080A0000,
// 												  0x080C0000,
// 												  0x080E0000};
// constexpr inline uint32_t kStartExecutionAddress = 0x08008000;
// constexpr inline uint32_t kStartReceiveAddress = 0x08080000;
// constexpr inline uint32_t EndOfMemory = 0x080FFFFC;

///////////////////////

constexpr uint32_t NumFlashSectors = 12;

constexpr uint32_t get_sector_addr(uint32_t sector_num) {
	constexpr uint32_t SECTORS[] = {
		0x08000000, //Sector 0: 16kB
		0x08004000, //Sector 1: 16kB
		0x08008000, //Sector 2: 16kB
		0x0800C000, //Sector 3: 16kB
		0x08010000, //Sector 4: 64kB
		0x08020000, //Sector 5: 128kB
		0x08040000, //Sector 6: 128k
		0x08060000, //Sector 7: 128k
		0x08080000, //Sector 8: 128k
		0x080A0000, //Sector 9: 128k
		0x080C0000, //Sector 10: 128k
		0x080E0000, //Sector 11: 128k
		0x08100000, //end of memory + 1
	};

	return (sector_num <= NumFlashSectors) ? SECTORS[sector_num] : 0;
}

constexpr inline uint32_t BootloaderFlashAddr = get_sector_addr(0);
constexpr inline uint32_t SettingsFlashAddr = get_sector_addr(1);
constexpr inline uint32_t AppFlashAddr = get_sector_addr(2);  //0x08008000
constexpr inline uint32_t BootloaderReceiveAddr = 0x08080000; //get_sector_addr(8);
