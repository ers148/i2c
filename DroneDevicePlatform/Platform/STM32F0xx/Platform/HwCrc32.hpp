//
// HwCrc32.hpp
//
//  Created on: Dec 22, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_HWCRC32_HPP_
#define PLATFORM_STM32F0XX_HWCRC32_HPP_

#include <cassert>
#include <cstdint>
#include <libopencm3/stm32/crc.h>
#include <libopencm3/stm32/rcc.h>

class HwCrc32 {
public:
	using Type = uint32_t;
	static constexpr size_t size{sizeof(uint32_t)};

	HwCrc32() = delete;
	HwCrc32(const HwCrc32 &) = delete;
	HwCrc32 &operator=(const HwCrc32 &) = delete;

	static void init()
	{
		rcc_periph_clock_enable(RCC_CRC);

		CRC_CR = CRC_CR_REV_OUT | CRC_CR_REV_IN_WORD;
		CRC_INIT = 0xFFFFFFFFUL;
	}

	static void deinit()
	{
		rcc_periph_clock_disable(RCC_CRC);
	}

	static uint32_t update(uint32_t aChecksum, const void *aBuffer, size_t aLength)
	{
		// Buffer address and size should be aligned along 4-byte boundary
		assert((aLength & 0x03) == 0);
		assert((reinterpret_cast<uintptr_t>(aBuffer) & 0x03) == 0);

		const uint32_t *data = static_cast<const uint32_t *>(aBuffer);
		size_t wordLength = aLength >> 2;

		if (!aChecksum)
			CRC_CR |= CRC_CR_RESET;

		while (wordLength--)
			CRC_DR = *data++;

		return ~CRC_DR;
	}
};

#endif // PLATFORM_STM32F0XX_HWCRC32_HPP_
