//
// AdcBase.hpp
//
//  Created on: Jul 12, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_ADCBASE_HPP_
#define PLATFORM_STM32F1XX_ADCBASE_HPP_

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
		ADC12_TIM1_CC1,
		ADC12_TIM1_CC2,
		ADC12_TIM1_CC3,
		ADC12_TIM2_CC2,
		ADC12_TIM3_TRGO,
		ADC12_TIM4_CC4,
		ADC12_EXTI11,
		ADC3_TIM3_CC1,
		ADC3_TIM2_CC3,
		ADC3_TIM1_CC3,
		ADC3_TIM8_CC1,
		ADC3_TIM8_TRGO,
		ADC3_TIM5_CC1,
		ADC3_TIM5_CC3,
		NONE
	};

	virtual ~AdcBase() = default;
};

#endif // PLATFORM_STM32F1XX_ADCBASE_HPP_
