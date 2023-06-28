//
// Board.hpp
//
//  Created on: Nov 29, 2017
//      Author: Alexander
//

#ifndef BOARD_HPP_
#define BOARD_HPP_

#include <DroneDevice/MemoryRegion.hpp>
#include <DroneDevice/Stubs/MockMemoryInterface.hpp>

template<typename Flash, typename Eeprom = Device::MockMemoryInterface>
struct Board {
	Board() = delete;
	Board(const Board &) = delete;
	Board &operator=(const Board &) = delete;

	static constexpr uintptr_t kSpecOffset     = ${SPEC_OFFSET};
	static constexpr size_t kSpecSize          = ${SPEC_SIZE};
	static constexpr uintptr_t kEepromOffset   = ${EEPROM_OFFSET};
	static constexpr size_t kEepromSize        = ${EEPROM_SIZE};
	static constexpr uintptr_t kFirmwareOffset = ${FIRMWARE_OFFSET};
	static constexpr size_t kFirmwareSize      = ${FIRMWARE_SIZE};
	static constexpr uintptr_t kRamOffset      = ${RAM_OFFSET};
	static constexpr size_t kRamSize           = ${RAM_SIZE};

	using FirmwareRegion = Device::MemoryRegion<Flash, kFirmwareOffset, kFirmwareSize, true, false>;
	using SpecRegion = Device::MemoryRegion<Flash, kSpecOffset, kSpecSize, true, false>;
	using EepromRegion = Device::MemoryRegion<Eeprom, kEepromOffset, kEepromSize, true, true>;
};

#endif // BOARD_HPP_
