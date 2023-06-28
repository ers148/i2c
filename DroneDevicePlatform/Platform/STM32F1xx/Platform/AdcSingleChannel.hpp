//
// AdcSingleChannel.hpp
//
//  Created on: Jan 15, 2018
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_ADCSINGLECHANNEL_HPP_
#define PLATFORM_STM32F1XX_ADCSINGLECHANNEL_HPP_

#include "Platform/AdcBase.hpp"

template<unsigned int number, typename Time, unsigned kSampleRate = ADC_SMPR_SMP_239DOT5CYC>
class AdcSingleChannel : AdcBase<number> {
	using BaseType = AdcBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};

public:
	using Channel = typename BaseType::Channel;

	AdcSingleChannel()
	{
		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		// Make sure the ADC doesn't run during configuration
		adc_power_off(kPeriph);

		// Configure everything for one single conversion
		// adc_set_clk_source(kPeriph, ADC_CLKSOURCE_PCLK_DIV4);
		adc_set_single_conversion_mode(kPeriph);
		adc_disable_external_trigger_regular(kPeriph);
		adc_set_right_aligned(kPeriph);
		adc_set_sample_time_on_all_channels(kPeriph, kSampleRate);

		// Enable temperature sensor and Vrefint
		adc_enable_temperature_sensor();

		// Enable converter
		adc_power_on(kPeriph);
		// Wait 10 us for ADC power-up
		Time::delay(std::chrono::microseconds{10});

		// ADC must be powered before starting a calibration
		adc_calibrate(kPeriph);
	}

	~AdcSingleChannel() override
	{
		adc_power_off(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	uint16_t read(Channel channel) const
	{
		uint8_t channelArray[] = {static_cast<uint8_t>(channel)};
		adc_set_regular_sequence(kPeriph, sizeof(channelArray), channelArray);

		// Start the conversion directly (not trigger mode)
		adc_start_conversion_direct(kPeriph);
		// // Start the conversion
		// adc_start_conversion_regular(kPeriph);
		// Wait for end of conversion
		while (!adc_eoc(kPeriph));

		return static_cast<uint16_t>(adc_read_regular(kPeriph));
	}
};

#endif // PLATFORM_STM32F1XX_ADCSINGLECHANNEL_HPP_
