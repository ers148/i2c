//
// Uid.hpp
//
//  Created on: Dec 12, 2018
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_UID_HPP_
#define PLATFORM_STM32F76X_UID_HPP_

#include <libopencm3/stm32/desig.h>
#include <array>

class Uid {
private:
	static auto load()
	{
		std::array<uint32_t, 3> result;
		desig_get_unique_id(result.data());
		return result;
	}

public:
	Uid() = delete;
	Uid(const Uid &) = delete;
	Uid &operator=(const Uid &) = delete;

	static const void *data()
	{
		static auto uid = load();
		return uid.data();
	}

	static constexpr size_t length()
	{
		return 12;
	}
};

#endif // PLATFORM_STM32F76X_UID_HPP_
