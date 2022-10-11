#include <cstdint>
namespace BootloaderConf
{
static constexpr bool UseGateInThreshold = true;
static constexpr uint32_t SampleRate = 48000;

constexpr inline uint32_t ReceiveSectorSize = 128 * 1024;

struct FskEncoding {
	uint32_t blank;
	uint32_t one;
	uint32_t zero;
};
static constexpr FskEncoding Encoding{16, 8, 4};

constexpr inline uint32_t TICKS_PER_MS = 1;

}; // namespace BootloaderConf
