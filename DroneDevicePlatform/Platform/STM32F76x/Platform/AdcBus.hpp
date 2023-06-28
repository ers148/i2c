//
// AdcBus.hpp
//
//  Created on: Apr 13, 2022
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_ADCBUS_HPP_
#define PLATFORM_STM32F76X_ADCBUS_HPP_

#include "Platform/AdcBase.hpp"
#include "Platform/Dma.hpp"
#include <array>
#include <chrono>

template<unsigned int number, size_t channels, typename Time,
	AdcPrescaler prescaler = AdcPrescaler::NonConfigured, AdcDmaStreamRemap stream = AdcDmaStreamRemap::NonConfigured>
class AdcBus : public AdcBase<number, stream> {
	static_assert(prescaler != AdcPrescaler::NonConfigured, "Prescaler not configured");

	using BaseType = AdcBase<number, stream>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};

public:
	using Channel = typename BaseType::Channel;
	using Trigger = typename BaseType::Trigger;

	AdcBus(std::array<Channel, channels> aChannels, Trigger aTrigger = Trigger::None,
		std::function<void()> aCallback = nullptr):
		BaseType{},
		dma{
			Dma::Dir::PeriphToMem,
			Dma::Width::HalfWord,
			false,
			Dma::Width::HalfWord,
			true,
			BaseType::numberToDmaEvent(),
			[this](uint32_t aFlags) { handler(aFlags); },
			true},
		callback{aCallback},
		buffer{}
	{
		// Reset Control and Status registers in case of previous use this ADC periphery
		// CR2 reset first to disable conversion and go to power down (clear ADON bit)
		ADC_CR2(kPeriph) = 0u;
		ADC_CR1(kPeriph) = 0u;
		ADC_SR(kPeriph) = 0u;

		Time::delay(std::chrono::microseconds{3});

		rcc_periph_clock_enable(BaseType::numberToClockBranch());

		// ADC Set Clock Prescale The ADC clock can be prescaled.
		adc_set_clk_prescale(static_cast<uint32_t>(prescaler));

		// ADC Enable Single Conversion Mode.
		// In this mode the ADC performs a conversion of one channel or a channel group and stops.
		adc_enable_scan_mode(kPeriph);

		// The EOC is set at the end of each sequence rather than after each conversion in the sequence.
		adc_eoc_after_group(kPeriph);

		adc_set_sample_time_on_all_channels(kPeriph, ADC_SMPR_SMP_480CYC);

		uint8_t channelArray[channels];

		for (size_t i = 0; i < channels; ++i) {
			channelArray[i] = static_cast<uint8_t>(aChannels[i]);
		}

		adc_set_regular_sequence(kPeriph, channels, channelArray);

		if (aTrigger == Trigger::None) {
			adc_disable_external_trigger_regular(kPeriph);
		} else {
			const uint32_t trigger = static_cast<uint32_t>(aTrigger);
			adc_enable_external_trigger_regular(kPeriph, trigger << ADC_CR2_EXTSEL_SHIFT, ADC_CR2_EXTEN_RISING_EDGE);
		}

		adc_enable_dma(kPeriph);

		// ADC Set DMA to Continue.
		// This must be set to allow DMA to continue to operate after the last conversion in the DMA sequence.
		// This allows DMA to be used in continuous circular mode.
		adc_set_dma_continue(kPeriph);

		// Enable converter
		adc_power_on(kPeriph);
		// Wait 10 us for ADC power-up
		Time::delay(std::chrono::microseconds{10});
	}

	~AdcBus() override
	{
		adc_power_off(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	void setCallback(std::function<void()> aCallback)
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
	DmaChannel<BaseType::numberToDmaController(), BaseType::numberToDmaStream()> dma;
	std::function<void()> callback;
	std::array<uint16_t, channels> buffer;

	void handler(uint32_t aFlags)
	{
		if ((aFlags & Dma::Flags::TransferComplete) && callback) {
			callback();
		}
	}
};

#endif // PLATFORM_STM32F76X_ADCBUS_HPP_
