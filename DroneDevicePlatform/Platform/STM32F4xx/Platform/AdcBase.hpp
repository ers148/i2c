//
// AdcBase.hpp
//
//  Created on: Jul 12, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F4XX_ADCBASE_HPP_
#define PLATFORM_STM32F4XX_ADCBASE_HPP_

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
	NonConfigured,
};

template<unsigned int number, AdcDmaStreamRemap stream = AdcDmaStreamRemap::NonConfigured>
class AdcBase {
	static_assert(number >= 1 && number <= 3, "Incorrect number");

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
		// In STM32F4xx family all ADC connected to DMA2 controller
		return 2u;
	}

	static constexpr unsigned int numberToDmaStream()
	{
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
		InTemp = 16,
		// Redefinition for use in methods of RNG class
		IN_TEMP = InTemp,
		InVref = 17,
		// Redefinition for use in methods of RNG class
		IN_VREF = InVref,
		InVbat = 18,
		// For the STM32F42x and STM32F43x devices IN_TEMP connected to ADC1_IN18 (RM0090, p.391)
		InTemp_InVbat_Stm32F42x_43x = InVbat,
		InVbat_InTemp_Stm32F42x_43x = InVbat,
	};

	enum class Trigger : uint8_t {
		AdcTim1CC1,
		AdcTim1CC2,
		AdcTim1CC3,
		AdcTim2CC2,
		AdcTim2CC3,
		AdcTim2CC4,
		AdcTim2TRGO,
		AdcTim3CC1,
		AdcTim3TRGO,
		AdcTim4CC4,
		AdcTim5CC1,
		AdcTim5CC2,
		AdcTim5CC3,
		AdcTim8CC1,
		AdcTim8TRGO,
		None
	};

	virtual ~AdcBase() = default;
};

#endif // PLATFORM_STM32F4XX_ADCBASE_HPP_
