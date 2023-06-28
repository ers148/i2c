//
// AdcBase.hpp
//
//  Created on: Apr 13, 2022
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_ADCBASE_HPP_
#define PLATFORM_STM32F76X_ADCBASE_HPP_

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>

enum class AdcDmaStreamRemap : uint8_t {
	Stream0,
	Stream1,
	Stream2,
	Stream3,
	Stream4,
	NonConfigured
};

enum class AdcPrescaler : uint32_t {
	By2 = (0u << 16u),
	By4 = (1u << 16u),
	By6 = (2u << 16u),
	By8 = (3u << 16u),
	NonConfigured
};

template<unsigned int number, AdcDmaStreamRemap stream = AdcDmaStreamRemap::NonConfigured>
class AdcBase {
	static_assert(number >= 1 && number <= 3, "Incorrect number");
	static_assert(number != 1
		|| stream == AdcDmaStreamRemap::NonConfigured
		|| stream == AdcDmaStreamRemap::Stream0
		|| stream == AdcDmaStreamRemap::Stream4,
		"Incorrect stream configuration");
	static_assert(number != 2
		|| stream == AdcDmaStreamRemap::NonConfigured
		|| stream == AdcDmaStreamRemap::Stream2
		|| stream == AdcDmaStreamRemap::Stream3,
		"Incorrect stream configuration");
	static_assert(number != 3
		|| stream == AdcDmaStreamRemap::NonConfigured
		|| stream == AdcDmaStreamRemap::Stream0
		|| stream == AdcDmaStreamRemap::Stream1,
		"Incorrect stream configuration");

protected:
	static constexpr uint32_t numberToPeriph()
	{
		if (number == 1) {
			return ADC1;
		} else if (number == 2) {
			return ADC2;
		} else {
			return ADC3;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		if (number == 1) {
			return RCC_ADC1;
		} else if (number == 2) {
			return RCC_ADC2;
		} else {
			return RCC_ADC3;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		return RST_ADC;
	}

	static constexpr unsigned int numberToDmaController()
	{
		// In STM32F76x family all ADC connected to DMA2 controller
		return 2u;
	}

	static constexpr unsigned int numberToDmaStream()
	{
		switch (number) {
			case 1:
				return stream == AdcDmaStreamRemap::NonConfigured ?
					4 : static_cast<unsigned int>(stream);

			case 2:
				return stream == AdcDmaStreamRemap::NonConfigured ?
					3 : static_cast<unsigned int>(stream);

			case 3:
				return stream == AdcDmaStreamRemap::NonConfigured ?
					1 : static_cast<unsigned int>(stream);
		}
		return static_cast<unsigned int>(stream);
	}

	static constexpr unsigned int numberToDmaEvent()
	{
		if (number == 1) {
			return 0u;
		} else if (number == 2) {
			return 1u;
		} else {
			return 2u;
		}
	}

public:
	enum class Channel : uint8_t {
		In0 = 0,
		In1 = 1,
		In2 = 2,
		In3 = 3,
		In4 = 4,
		In5 = 5,
		In6 = 6,
		In7 = 7,
		In8 = 8,
		In9 = 9,
		In10 = 10,
		In11 = 11,
		In12 = 12,
		In13 = 13,
		In14 = 14,
		In15 = 15,
		InVref = 17,
		InVbat = 18,
	};

	enum class Trigger : uint8_t {
		Tim1CC1,
		Tim1CC2,
		Tim1CC3,
		Tim2CC2,
		Tim5TRGO,
		Tim4CC4,
		Tim3CC4,
		Tim8TRGO,
		Tim8TRGO2,
		Tim1TRGO,
		Tim1TRGO2,
		Tim2TRGO,
		Tim4TRGO,
		Tim6TRGO,
		Exti11,
		None
	};

	virtual ~AdcBase() = default;
};

#endif // PLATFORM_STM32F76X_ADCBASE_HPP_
