//
// Uid.hpp
//
//  Created on: Sep 27, 2018
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_UID_HPP_
#define PLATFORM_STM32F0XX_UID_HPP_

#include <libopencm3/stm32/desig.h>

class Uid {
public:
	Uid() = delete;
	Uid(const Uid &) = delete;
	Uid &operator=(const Uid &) = delete;

	static const void *data()
	{
		return const_cast<const uint32_t *>(&DESIG_UNIQUE_ID0);
	}

	static constexpr size_t length()
	{
		return 12;
	}
};

#endif // PLATFORM_STM32F0XX_UID_HPP_
