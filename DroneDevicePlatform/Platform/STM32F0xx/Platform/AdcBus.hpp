//
// AdcBus.hpp
//
//  Created on: Jul 12, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_ADCBUS_HPP_
#define PLATFORM_STM32F0XX_ADCBUS_HPP_

#include "Platform/AdcBase.hpp"
#include "Platform/Dma.hpp"
#include <array>
#include <chrono>

template<unsigned int number, size_t channels, typename Time, unsigned kSampleRate = ADC_SMPR_SMP_013DOT5>
class AdcBus : public AdcBase<number> {
	using BaseType = AdcBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};

public:
	using Channel = typename BaseType::Channel;
	using Trigger = typename BaseType::Trigger;

	AdcBus(std::array<Channel, channels> aChannels, Trigger aTrigger = Trigger::NONE,
		std::function<void ()> aCallback = nullptr) :
		BaseType{},
		dma{
			Dma::Dir::PeriphToMem,
			Dma::Width::HalfWord,
			false,
			Dma::Width::HalfWord,
			true,
			BaseType::numberToDmaEvent(),
			[this](uint32_t aFlags){ handler(aFlags); },
			true},
		callback{aCallback},
		buffer{}
	{
		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		// Make sure the ADC doesn't run during configuration
		adc_power_off(kPeriph);

		adc_set_clk_source(kPeriph, ADC_CLKSOURCE_PCLK_DIV4);
		adc_calibrate_async(kPeriph);
		adc_disable_discontinuous_mode(kPeriph);
		adc_set_single_conversion_mode(kPeriph);
		adc_disable_analog_watchdog(kPeriph);
		adc_set_sample_time_on_all_channels(kPeriph, kSampleRate);

		uint8_t channelArray[channels];

		for (size_t i = 0; i < channels; ++i) {
			channelArray[i] = static_cast<uint8_t>(aChannels[i]);
		}
		adc_set_regular_sequence(kPeriph, channels, channelArray);

		if (aTrigger == Trigger::NONE) {
			adc_disable_external_trigger_regular(kPeriph);
		} else {
			adc_enable_external_trigger_regular(kPeriph,
				ADC_CFGR1_EXTSEL_VAL(static_cast<uint32_t>(aTrigger)),
				ADC_CFGR1_EXTEN_RISING_EDGE);
		}

		// Calibration can only be initiated when the ADC is disabled
		adc_calibrate(kPeriph);

		do {
			// Enable converter
			adc_power_on_async(kPeriph);
			// Wait for ADC power-up
			Time::delay(std::chrono::microseconds{10});
		} while (!adc_is_power_on(kPeriph));

		adc_enable_dma(kPeriph);
		adc_enable_dma_circular_mode(kPeriph);
	}

	~AdcBus() override
	{
		adc_power_off(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;
	}

	void start()
	{
		auto dst = reinterpret_cast<void *>(const_cast<uint32_t *>(&ADC_DR(kPeriph)));
		dma.start(buffer.data(), dst, channels);
		adc_start_conversion_regular(kPeriph);
	}

	const std::array<uint16_t, channels> &values() const
	{
		return buffer;
	}

private:
	DmaChannel<BaseType::numberToDmaController(), BaseType::numberToDmaChannel()> dma;
	std::function<void ()> callback;
	std::array<uint16_t, channels> buffer;

	void handler(uint32_t aFlags)
	{
		if ((aFlags & Dma::Flags::TransferComplete) && callback) {
			callback();
		}
	}
};

#endif // PLATFORM_STM32F0XX_ADCBUS_HPP_
