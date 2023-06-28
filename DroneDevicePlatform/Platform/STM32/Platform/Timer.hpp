//
// Timer.hpp
//
//  Created on: Jun 2, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32_TIMER_HPP_
#define PLATFORM_STM32_TIMER_HPP_

#include "Platform/TimerBase.hpp"
#include <functional>

template<unsigned int number, bool continuous>
class Timer : public TimerBase<number> {
	using BaseType = TimerBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};
	static constexpr auto kIrq{BaseType::numberToIrq()};

	std::function<void ()> callback;

public:
	static constexpr auto kResolution{BaseType::numberToResolution()};

	enum class CompareChannel : uint8_t {
		OC1,
		OC2,
		OC3,
		OC4
	};

	enum class MasterMode : uint8_t {
		Reset,
		Enable,
		Update,
		ComparePulse,
		CompareOC1Ref,
		CompareOC2Ref,
		CompareOC3Ref,
		CompareOC4Ref
	};

private:
	constexpr tim_oc_id compareOutputFromChannel(CompareChannel aChannel)
	{
		switch (aChannel) {
			case CompareChannel::OC1:
				return TIM_OC1;
			case CompareChannel::OC2:
				return TIM_OC2;
			case CompareChannel::OC3:
				return TIM_OC3;
			default:
				return TIM_OC4;
		}
	}

public:
	Timer(uint32_t aFrequency, uint32_t aPeriod = kResolution + 1, std::function<void ()> aCallback = nullptr) :
		BaseType{},
		callback{aCallback}
	{
		assert(aFrequency > 0);

		BaseType::setHandler(this);
		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		timer_set_mode(kPeriph, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
		timer_set_prescaler(kPeriph, BaseType::clock() / aFrequency - 1);
		timer_set_period(kPeriph, aPeriod - 1);
		timer_disable_preload(kPeriph);

		if (continuous) {
			timer_continuous_mode(kPeriph);
		} else {
			timer_one_shot_mode(kPeriph);
		}

		if (callback) {
			timer_enable_irq(kPeriph, TIM_DIER_UIE);
		}

		// Enable interrupt
		nvic_clear_pending_irq(kIrq);
		nvic_enable_irq(kIrq);
	}

	~Timer() override
	{
		disable();

		nvic_disable_irq(kIrq);
		timer_disable_irq(kPeriph, TIM_DIER_UIE);

		rcc_periph_clock_disable(BaseType::numberToClockBranch());
		BaseType::setHandler(nullptr);
	}

	void disable()
	{
		timer_disable_counter(kPeriph);
	}

	void enable()
	{
		timer_enable_counter(kPeriph);
	}

	void start(uint32_t aPeriod)
	{
		timer_set_counter(kPeriph, 0);
		timer_set_period(kPeriph, std::max(uint32_t{1}, aPeriod - 1));
		timer_enable_counter(kPeriph);
	}

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;

		if (callback) {
			timer_enable_irq(kPeriph, TIM_DIER_UIE);
		} else {
			timer_disable_irq(kPeriph, TIM_DIER_UIE);
		}
	}

	uint32_t getFlags()
	{
		return TIM_SR(kPeriph);
	}

	uint32_t getValue()
	{
		return timer_get_counter(kPeriph);
	}

	void setMasterMode(MasterMode aMode)
	{
		timer_set_master_mode(kPeriph, static_cast<uint32_t>(aMode) << 4);
	}

	void enableCompareChannel(CompareChannel aChannel)
	{
		const auto output = compareOutputFromChannel(aChannel);

		timer_disable_oc_output(kPeriph, output);
		BaseType::enableMainOutput();

		timer_set_oc_mode(kPeriph, output, TIM_OCM_TOGGLE);
		timer_set_oc_value(kPeriph, output, 0);
		timer_set_oc_polarity_high(kPeriph, output);

		timer_enable_oc_output(kPeriph, output);
	}

protected:
	void handler() override
	{
		timer_clear_flag(kPeriph, TIM_SR_UIF);

		if (callback != nullptr) {
			callback();
		}
	}
};

#endif // PLATFORM_STM32_TIMER_HPP_
