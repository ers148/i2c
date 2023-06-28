//
// Flash.hpp
//
//  Created on: Nov 13, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_FLASH_HPP_
#define PLATFORM_STM32F76X_FLASH_HPP_

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

	static bool erase(uintptr_t aOffset, size_t aLength)
	{
		const auto begin = addressToSector(aOffset);
		const auto end = addressToSector(aOffset + aLength);

		flash_clear_status_flags();

		for (size_t i = begin; i < end; ++i) {
			if (!blankCheckSector(indexToEntry(i))) {
				flash_erase_sector(static_cast<uint8_t>(i), FLASH_CR_PROGRAM_X32);

				if (FLASH_SR & (FLASH_SR_ERSERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR)) {
					flash_lock();
					return false;
				}
			}
		}

		return true;
	}

	static bool read(uintptr_t aOffset, void *aBuffer, size_t aLength)
	{
		memcpy(aBuffer, reinterpret_cast<const void *>(aOffset), aLength);
		return true;
	}

	static bool write(uintptr_t aOffset, const void *aBuffer, size_t aLength)
	{
		assert(aOffset % 4 == 0);

		const auto buffer = static_cast<const uint8_t *>(aBuffer);

		flash_clear_status_flags();

		for (size_t i = 0; i < aLength; i += 4) {
			uint32_t word = std::numeric_limits<uint32_t>::max();

			memcpy(&word, buffer + i, std::min(aLength - i, sizeof(word)));
			flash_program_word(aOffset + i, word);

			if (FLASH_SR & (FLASH_SR_ERSERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR)) {
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

	static const Entry &indexToEntry(size_t aIndex)
	{
		static const std::array<Entry, 13> kSectors = {{
			{0x08000000, 1024 * 32},
			{0x08008000, 1024 * 32},
			{0x08010000, 1024 * 32},
			{0x08018000, 1024 * 32},
			{0x08020000, 1024 * 128},
			{0x08040000, 1024 * 256},
			{0x08080000, 1024 * 256},
			{0x080C0000, 1024 * 256},
			{0x08100000, 1024 * 256},
			{0x08140000, 1024 * 256},
			{0x08180000, 1024 * 256},
			{0x081C0000, 1024 * 256},
			{0x08200000, 0}}}; // Mock sector

		assert(aIndex < kSectors.size());
		return kSectors[aIndex];
	}

	static size_t addressToSector(uintptr_t aAddress)
	{
		size_t i = 0;

		while (indexToEntry(i).address != aAddress && indexToEntry(i).size != 0) {
			++i;
		}

		return i;
	}

	static bool blankCheckSector(const Entry &aEntry)
	{
		for (uintptr_t address = aEntry.address; address < aEntry.address + aEntry.size; address += 4) {
			if (*reinterpret_cast<const uint32_t *>(address) != 0xFFFFFFFFUL) {
				return false;
			}
		}

		return true;
	}
};

#endif // PLATFORM_STM32F76X_FLASH_HPP_
