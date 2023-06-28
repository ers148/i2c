//
// AdcSingleChannel.hpp
//
//  Created on: Jan 15, 2018
//      Author: Alexander
//

#ifndef PLATFORM_STM32F4XX_ADCSINGLECHANNEL_HPP_
#define PLATFORM_STM32F4XX_ADCSINGLECHANNEL_HPP_

#include "AdcBase.hpp"
#include <chrono>
#include <cstdint>

template<unsigned int number, typename Time, AdcPrescaler prescaler = AdcPrescaler::NonConfigured>
class AdcSingleChannel : public AdcBase<number> {
	static_assert(prescaler != AdcPrescaler::NonConfigured, "Prescaler not configured");
	using BaseType = AdcBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};

public:
	using Channel = typename BaseType::Channel;

	AdcSingleChannel()
	{
		// Reset Control and Status registers in case of previous use this ADC periphery
		// CR2 resets first to disable conversion and go to power down (clear ADON bit)
		ADC_CR2(kPeriph) = 0u;
		ADC_CR1(kPeriph) = 0u;
		ADC_SR(kPeriph) = 0u;

		Time::delay(std::chrono::microseconds{3});

		rcc_periph_clock_enable(BaseType::numberToClockBranch());

		adc_set_clk_prescale(static_cast<uint32_t>(prescaler));

		adc_set_single_conversion_mode(kPeriph);

		adc_set_sample_time_on_all_channels(kPeriph, ADC_SMPR_SMP_480CYC);

		// Enable temperature sensor and Vrefint
		adc_enable_temperature_sensor();

		// Enable converter
		adc_power_on(kPeriph);
		// Wait 10 us for ADC power-up
		Time::delay(std::chrono::microseconds{10});
	}

	~AdcSingleChannel()
	{
		adc_power_off(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	uint16_t read(Channel channel) const
	{
		uint8_t channelArray[] = {static_cast<uint8_t>(channel)};
		adc_set_regular_sequence(kPeriph, sizeof(channelArray), channelArray);

		// Start the conversion directly (not trigger mode)
		adc_start_conversion_regular(kPeriph);
		// Wait for end of conversion
		while (!adc_eoc(kPeriph));
		return static_cast<uint16_t>(adc_read_regular(kPeriph));
	}
};

#endif // PLATFORM_STM32F4XX_ADCSINGLECHANNEL_HPP_
