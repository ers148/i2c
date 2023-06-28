//
// Pwm.hpp
//
//  Created on: Jun 2, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32_PWM_HPP_
#define PLATFORM_STM32_PWM_HPP_

#include "Platform/TimerBase.hpp"
#include <cassert>

template<unsigned int number>
class Pwm : public TimerBase<number> {
	using BaseType = TimerBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};
	static constexpr auto kIrq{BaseType::numberToIrq()};

public:
	template<unsigned int channel, bool inversion = false, bool complementary = false>
	class Channel {
		using ParentType = Pwm<number>;

		static_assert((!complementary && channel == 4) || (channel >= 1 && channel <= 3),
			"Incorrect PWM channel");

		static constexpr tim_oc_id channelToComplementaryOutput()
		{
			switch (channel) {
				case 1:
					return complementary ? TIM_OC1N : TIM_OC1;
				case 2:
					return complementary ? TIM_OC2N : TIM_OC2;
				case 3:
					return complementary ? TIM_OC3N : TIM_OC3;
				case 4:
					return TIM_OC4;
			}
		}

		static constexpr tim_oc_id channelToOutput()
		{
			switch (channel) {
				case 1:
					return TIM_OC1;
				case 2:
					return TIM_OC2;
				case 3:
					return TIM_OC3;
				case 4:
					return TIM_OC4;
			}
		}

		static constexpr tim_oc_id kComplementaryOutput{channelToComplementaryOutput()};
		static constexpr tim_oc_id kOutput{channelToOutput()};

	public:
		Channel(ParentType &aParent, uint32_t aValue) :
			parent{aParent}
		{
			timer_disable_oc_output(ParentType::kPeriph, kComplementaryOutput);

			if (inversion) {
				timer_set_oc_mode(ParentType::kPeriph, kOutput, TIM_OCM_FORCE_HIGH);
			} else {
				timer_set_oc_mode(ParentType::kPeriph, kOutput, TIM_OCM_FORCE_LOW);
			}

			timer_set_oc_value(ParentType::kPeriph, kOutput, aValue);
			timer_set_oc_polarity_high(ParentType::kPeriph, kOutput);

			timer_enable_oc_output(ParentType::kPeriph, kComplementaryOutput);
		}

		~Channel()
		{
			disable();
		}

		void set(uint32_t aValue)
		{
			timer_set_oc_value(ParentType::kPeriph, kOutput, aValue);
		}

		void enable()
		{
			if (inversion) {
				timer_set_oc_mode(ParentType::kPeriph, kOutput, TIM_OCM_PWM2);
			} else {
				timer_set_oc_mode(ParentType::kPeriph, kOutput, TIM_OCM_PWM1);
			}
		}

		void disable()
		{
			if (inversion) {
				timer_set_oc_mode(ParentType::kPeriph, kOutput, TIM_OCM_FORCE_HIGH);
			} else {
				timer_set_oc_mode(ParentType::kPeriph, kOutput, TIM_OCM_FORCE_LOW);
			}
		}

	private:
		ParentType &parent;
	};

	Pwm(uint32_t aFrequency, uint32_t aPeriod)
	{
		assert(aFrequency > 0);

		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		timer_set_mode(kPeriph, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
		timer_set_prescaler(kPeriph, BaseType::clock() / aFrequency - 1);
		timer_set_period(kPeriph, aPeriod - 1);

		BaseType::enableMainOutput();
		timer_enable_preload(kPeriph);
		timer_continuous_mode(kPeriph);
	}

	~Pwm() override
	{
		disable();
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	void enable()
	{
		timer_enable_counter(kPeriph);
	}

	void disable()
	{
		timer_disable_counter(kPeriph);
		timer_clear_flag(kPeriph, TIM_SR_UIF);
	}

protected:
	void handler() override
	{
	}
};

#endif // PLATFORM_STM32_PWM_HPP_
