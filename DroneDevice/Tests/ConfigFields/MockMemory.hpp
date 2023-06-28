//
// MockMemory.hpp
//
//  Created on: Nov 19, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_TESTS_CONFIGFIELDS_MOCKMEMORY_HPP_
#define DRONEDEVICE_TESTS_CONFIGFIELDS_MOCKMEMORY_HPP_

#include <algorithm>
#include <cstddef>
#include <cstdint>

template<size_t size>
class MockMemory {
public:
	MockMemory() = delete;
	MockMemory(const MockMemory &) = delete;
	MockMemory &operator=(const MockMemory &) = delete;

	static void lock()
	{
	}

	static void unlock()
	{
	}

	static bool erase()
	{
		std::fill(arena(), arena() + size, 0xFF);
		return true;
	}

	static bool read(uintptr_t aOffset, void *aBuffer, size_t aLength)
	{
		if (aOffset + aLength <= size) {
			uint8_t * const buffer = static_cast<uint8_t *>(aBuffer);
			std::copy(arena() + aOffset, arena() + aOffset + aLength, buffer);
			return true;
		} else {
			return false;
		}
	}

	static bool write(uintptr_t aOffset, const void *aBuffer, size_t aLength)
	{
		if (aOffset + aLength <= size && aLength % sizeof(uint32_t) == 0) {
			const uint8_t * const buffer = static_cast<const uint8_t *>(aBuffer);
			std::copy(buffer, buffer + aLength, arena() + aOffset);
			return true;
		} else {
			return false;
		}
	}

	static uint8_t *arena()
	{
		static uint8_t memory[size];
		return memory;
	}
};

#endif // DRONEDEVICE_TESTS_CONFIGFIELDS_MOCKMEMORY_HPP_
