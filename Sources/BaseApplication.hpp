//
// BaseApplication.hpp
//
//  Created on: Aug 30, 2018
//      Author: Alexander
//

#ifndef SOURCES_BASEAPPLICATION_HPP_
#define SOURCES_BASEAPPLICATION_HPP_

#include <cstddef>
#include <cstdint>

namespace Dfu {

template<typename T>
static bool checkFirmware()
{
	auto contains = [](uintptr_t offset, size_t size, uint32_t address) {
		return address >= offset && address <= offset + size;
	};

	const uint32_t * const vectors = reinterpret_cast<const uint32_t *>(T::kFirmwareOffset);

	return contains(T::kRamOffset, T::kRamSize, vectors[0])
		&& contains(T::kFirmwareOffset, T::kFirmwareSize, vectors[1]);
}

template<typename T>
static void startFirmware()
{
	const uint32_t * const vectors = reinterpret_cast<const uint32_t *>(T::kFirmwareOffset);
	T::Reloader::relocateAndStart(T::kFirmwareOffset, vectors[0], vectors[1]);
}

static constexpr size_t kArenaSize = 2048;
extern uint8_t arena[kArenaSize];

} // namespace Dfu


#endif // SOURCES_BASEAPPLICATION_HPP_
