//
// AdcBus.hpp
//
//  Created on: Jul 12, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_ADCBUS_HPP_
#define PLATFORM_STM32F1XX_ADCBUS_HPP_

#include "Platform/AdcBase.hpp"
#include "Platform/Dma.hpp"
#include <array>

template<unsigned int number, size_t channels, typename Time, unsigned kSampleRate = ADC_SMPR_SMP_13DOT5CYC>
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

		adc_disable_analog_watchdog_regular(kPeriph);
		adc_enable_scan_mode(kPeriph);
		adc_set_single_conversion_mode(kPeriph);
		adc_set_sample_time_on_all_channels(kPeriph, kSampleRate);

		uint8_t channelArray[channels];

		for (size_t i = 0; i < channels; ++i) {
			channelArray[i] = static_cast<uint8_t>(aChannels[i]);
		}
		adc_set_regular_sequence(kPeriph, channels, channelArray);

		if (aTrigger == Trigger::NONE) {
			adc_disable_external_trigger_regular(kPeriph);
		} else if (aTrigger < Trigger::ADC3_TIM3_CC1) {
			const uint32_t trigger = static_cast<uint32_t>(aTrigger);
			adc_enable_external_trigger_regular(kPeriph, trigger << ADC_CR2_EXTSEL_SHIFT);
		} else {
			const uint32_t trigger = static_cast<uint32_t>(aTrigger) - static_cast<uint32_t>(Trigger::ADC3_TIM3_CC1);
			adc_enable_external_trigger_regular(kPeriph, trigger << ADC_CR2_EXTSEL_SHIFT);
		}

		adc_enable_dma(kPeriph);

		// Enable converter
		adc_power_on(kPeriph);
		// Wait 10 us for ADC power-up
		Time::delay(std::chrono::microseconds{10});

		// ADC must be powered before starting a calibration
		adc_calibrate(kPeriph);
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

#endif // PLATFORM_STM32F1XX_ADCBUS_HPP_
