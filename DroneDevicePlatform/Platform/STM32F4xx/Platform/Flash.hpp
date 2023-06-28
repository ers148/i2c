//
// Flash.hpp
//
//  Created on: Mar 27, 2018
//      Author: andrey
//

#ifndef PLATFORM_STM32F4XX_FLASH_HPP_
#define PLATFORM_STM32F4XX_FLASH_HPP_

#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <libopencm3/stm32/flash.h>
#include "Helpers.hpp"

enum class BorLevel: uint8_t {
	NONE  = 0x03,
	VBOR1 = 0x02,
	VBOR2 = 0x01,
	VBOR3 = 0x00
};

template<BorLevel LEVEL = BorLevel::NONE>
class Flash {
public:
	Flash() = delete;
	Flash(const Flash &) = delete;
	Flash &operator=(const Flash &) = delete;

	static bool protect()
	{
		static constexpr uint32_t kOptValue = 0x0000FF00UL | (static_cast<uint32_t>(LEVEL) << 2);

		flash_unlock_option_bytes();
		flash_program_option_bytes((FLASH_OPTCR & 0xFFFF00F0UL) | kOptValue);
		flash_lock_option_bytes();
		return true;
	}

	static bool isProtected()
	{
		return (FLASH_OPTCR & 0x0000FF00UL) != 0x0000AA00UL;
	}

	static void lock()
	{
		flash_lock();
	}

	static void unlock()
	{
		flash_unlock();
	}

	static bool erase(uintptr_t offset, size_t length)
	{
		const size_t begin = addressToSector(offset);
		const size_t end = addressToSector(offset + length);

		flash_clear_status_flags();

		for (size_t i = begin; i < end; ++i) {
			flash_erase_sector(static_cast<uint8_t>(i), FLASH_CR_PROGRAM_X32);

			if (FLASH_SR & (FLASH_SR_PGSERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR)) {
				flash_lock();
				return false;
			}
		}

		return true;
	}

	static bool read(uintptr_t offset, void *bufferPtr, size_t length)
	{
		memcpy(bufferPtr, reinterpret_cast<const void *>(offset), length);
		return true;
	}

	static bool write(uintptr_t offset, const void *bufferPtr, size_t length)
	{
		assert(length % 4 == 0);
		assert(offset % 4 == 0);

		const uint8_t * const buffer = static_cast<const uint8_t *>(bufferPtr);

		flash_clear_status_flags();

		for (size_t i = 0; i < length; i += 4) {
			uint32_t word;

			memcpy(&word, buffer + i, sizeof(word));
			flash_program_word(offset + i, word); // TODO Add parallelism configuration

			if (FLASH_SR & (FLASH_SR_PGSERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR)) {
				flash_lock();
				return false;
			}
		}

		return true;
	}

private:
	struct Entry {
		uintptr_t address;
		size_t size;
	};

	static size_t addressToSector(uintptr_t address)
	{
		static const std::array<Entry, 26> kSectors = {{
			{0x08000000, 1024 * 16},
			{0x08004000, 1024 * 16},
			{0x08008000, 1024 * 16},
			{0x0800C000, 1024 * 16},
			{0x08010000, 1024 * 64},
			{0x08020000, 1024 * 128},
			{0x08040000, 1024 * 128},
			{0x08060000, 1024 * 128},
			{0x08080000, 1024 * 128},
			{0x080A0000, 1024 * 128},
			{0x080C0000, 1024 * 128},
			{0x080E0000, 1024 * 128},
			{0x08100000, 1024 * 16},
			{0x08104000, 1024 * 16},
			{0x08108000, 1024 * 16},
			{0x0810C000, 1024 * 16},
			{0x08110000, 1024 * 64},
			{0x08120000, 1024 * 128},
			{0x08140000, 1024 * 128},
			{0x08160000, 1024 * 128},
			{0x08180000, 1024 * 128},
			{0x081A0000, 1024 * 128},
			{0x081C0000, 1024 * 128},
			{0x081E0000, 1024 * 128},
			{0x08200000, 0}}}; // Mock sector

		ssize_t result = -1;

		for (size_t i = 0; i < kSectors.size(); ++i) {
			if (kSectors[i].address == address) {
				result = static_cast<ssize_t>(i);
				break;
			}
		}

		assert(result != -1);
		return static_cast<size_t>(result);
	}
};

#endif // PLATFORM_STM32F4XX_FLASH_HPP_
