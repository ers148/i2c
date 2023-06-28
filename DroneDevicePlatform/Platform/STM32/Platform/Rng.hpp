//
// Rng.hpp
//
//  Created on: Jan 15, 2018
//      Author: Alexander
//

#ifndef PLATFORM_STM32_RNG_HPP_
#define PLATFORM_STM32_RNG_HPP_

#include <cstdint>
#include <cstdlib>

template<typename Converter, typename Checksum>
class Rng {
public:
	static void init()
	{
		Converter adc;
		uint32_t seed = 0;

		for (unsigned int i = 0; i < 16; ++i) {
			const uint16_t high = adc.read(Converter::Channel::IN_TEMP);
			const uint16_t low = adc.read(Converter::Channel::IN_VREF);
			const uint32_t word = (high << 16) | low;

			seed = Checksum::update(seed, &word, sizeof(word));
		}

		srand(seed);
	}

	static void deinit()
	{
	}

	static uint32_t random()
	{
		return rand();
	}
};

#endif // PLATFORM_STM32_RNG_HPP_
