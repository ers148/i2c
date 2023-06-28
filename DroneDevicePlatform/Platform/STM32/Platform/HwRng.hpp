//
// HwRng.hpp
//
//  Created on: Jan 11, 2019
//      Author: Alexander
//

#ifndef PLATFORM_STM32_HWRNG_HPP_
#define PLATFORM_STM32_HWRNG_HPP_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rng.h>

class HwRng {
public:
	HwRng() = delete;
	HwRng(const HwRng &) = delete;
	HwRng &operator=(const HwRng &) = delete;

	static void init()
	{
		rcc_periph_clock_enable(RCC_RNG);
		rcc_periph_reset_pulse(RST_RNG);
		rng_enable();
	}

	static void deinit()
	{
		rng_disable();
		rcc_periph_clock_disable(RCC_RNG);
	}

	static uint32_t random()
	{
		return rng_get_random_blocking();
	}
};

#endif // PLATFORM_STM32_HWRNG_HPP_
