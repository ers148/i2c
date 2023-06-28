//
// AdcBase.hpp
//
//  Created on: Jul 12, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_ADCBASE_HPP_
#define PLATFORM_STM32F0XX_ADCBASE_HPP_

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>

enum class AdcDmaRemap {
	Default,
	// ADC requests remapped from DMA1 stream 1 to stream 2
	AdcAlternate
};

template<unsigned int number, AdcDmaRemap remap = AdcDmaRemap::Default>
class AdcBase {
	static_assert(number == 1, "Incorrect number");

protected:
	static constexpr uint32_t numberToPeriph()
	{
		return ADC1;
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		return RCC_ADC1;
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		return RST_ADC1;
	}

	static constexpr unsigned int numberToDmaController()
	{
		return 1;
	}

	static constexpr unsigned int numberToDmaChannel()
	{
		return remap == AdcDmaRemap::Default ? 1 : 2;
	}

	static constexpr unsigned int numberToDmaEvent()
	{
		return 0;
	}

public:
	enum class Channel : uint8_t {
		IN0,
		IN1,
		IN2,
		IN3,
		IN4,
		IN5,
		IN6,
		IN7,
		IN8,
		IN9,
		IN10,
		IN11,
		IN12,
		IN13,
		IN14,
		IN15,
		IN_TEMP,
		IN_VREF
	};

	enum class Trigger : uint8_t {
		TIM1_TRGO,
		TIM1_CC4,
		TIM2_TRGO,
		TIM3_TRGO,
		TIM15_TRGO,
		NONE
	};

	virtual ~AdcBase() = default;
};

#endif // PLATFORM_STM32F0XX_ADCBASE_HPP_
