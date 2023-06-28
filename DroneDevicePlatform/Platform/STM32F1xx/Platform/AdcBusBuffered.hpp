//
// AdcBusBuffered.hpp
//
//  Created on: Dec 16, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_ADCBUSBUFFERED_HPP_
#define PLATFORM_STM32F1XX_ADCBUSBUFFERED_HPP_

#include "Platform/AdcBase.hpp"
#include "Platform/Dma.hpp"
#include <array>
#include <cassert>

template<unsigned int number, size_t channels, typename Time, unsigned kSampleRate = ADC_SMPR_SMP_13DOT5CYC>
class AdcBusBuffered : public AdcBase<number> {
	using BaseType = AdcBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};

public:
	using Channel = typename BaseType::Channel;
	using Trigger = typename BaseType::Trigger;

	AdcBusBuffered(std::array<Channel, channels> aChannels, uint16_t *aBuffer, size_t aSampleCount,
		Trigger aTrigger = Trigger::NONE, std::function<void (const uint16_t *, size_t)> aCallback = nullptr) :
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
		buffer{aBuffer},
		samples{aSampleCount}
	{
		assert(aSampleCount >= channels * 2);

		dma.enableHalfTransferInterrupt();

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

	~AdcBusBuffered() override
	{
		adc_power_off(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	void setCallback(std::function<void (const uint16_t *, size_t)> aCallback)
	{
		callback = aCallback;
	}

	void start()
	{
		auto dst = reinterpret_cast<void *>(const_cast<uint32_t *>(&ADC_DR(kPeriph)));
		dma.start(buffer, dst, samples);
		adc_start_conversion_regular(kPeriph);
	}

private:
	DmaChannel<BaseType::numberToDmaController(), BaseType::numberToDmaChannel()> dma;
	std::function<void (const uint16_t *, size_t)> callback;
	uint16_t * const buffer;
	const size_t samples;

	void handler(uint32_t aFlags)
	{
		if (!callback) {
			return;
		}

		if (aFlags & Dma::Flags::HalfTransfer) {
			callback(buffer, samples >> 1);
		} else if (aFlags & Dma::Flags::TransferComplete) {
			callback(buffer + (samples >> 1), samples - (samples >> 1));
		}
	}
};

#endif // PLATFORM_STM32F1XX_ADCBUSBUFFERED_HPP_
