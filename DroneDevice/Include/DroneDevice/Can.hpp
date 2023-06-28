//
// Can.hpp
//
//  Created on: Dec 12, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_CAN_HPP_
#define DRONEDEVICE_CAN_HPP_

#include <cstdint>
#include <cstddef>

struct CanFilter {
	uint32_t id;
	uint32_t mask;
};

struct CanMessage {
	static constexpr size_t kMaxLength{8};

	enum: uint8_t {
		EXT = 0x01,
		RTR = 0x02
	};

	uint64_t timestamp;
	uint32_t id;
	uint8_t flags;
	uint8_t length;
	uint8_t data[kMaxLength];
};

struct CanStatistics {
	uint64_t rx;
	uint64_t tx;
	uint64_t errors;
};

#endif // PLATFORM_STM32F1XX_USART_HPP_
