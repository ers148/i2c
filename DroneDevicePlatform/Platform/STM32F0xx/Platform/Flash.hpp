//
// Flash.hpp
//
//  Created on: Nov 3, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_FLASH_HPP_
#define PLATFORM_STM32F0XX_FLASH_HPP_

#include <cassert>
#include <cstddef>
#include <libopencm3/stm32/flash.h>

template<size_t PAGE_SIZE>
class Flash {
public:
	Flash() = delete;
	Flash(const Flash &) = delete;
	Flash &operator=(const Flash &) = delete;

	static bool protect()
	{
		flash_unlock();
		flash_erase_option_bytes();
		flash_program_option_bytes(reinterpret_cast<uint32_t>(&FLASH_OPTION_BYTE_0), 0xFF00);
		flash_lock();
		return true;
	}

	static bool isProtected()
	{
		return (FLASH_OBR & FLASH_OBR_RDPRT) != 0;
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
		assert(offset % PAGE_SIZE == 0);

		flash_clear_status_flags();

		for (uintptr_t address = offset; address < offset + length; address += PAGE_SIZE) {
			flash_erase_page(address);

			if (flash_get_status_flags() != FLASH_SR_EOP) {
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

			if (flash_get_status_flags() != FLASH_SR_EOP) {
				flash_lock();
				return false;
			}
		}

		return true;
	}
};

#endif // PLATFORM_STM32F0XX_FLASH_HPP_
