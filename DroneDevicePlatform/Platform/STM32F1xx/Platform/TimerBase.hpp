//
// TimerBase.hpp
//
//  Created on: Dec 22, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_TIMERBASE_HPP_
#define PLATFORM_STM32F1XX_TIMERBASE_HPP_

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

template<unsigned int number>
class TimerBase {
	static_assert(number >= 1 && number <= 8, "Incorrect number");

public:
	virtual ~TimerBase() = default;

	static void handleIrq()
	{
		instance->handler();
	}

protected:
	static constexpr uint32_t numberToPeriph()
	{
		switch (number) {
			case 1:
				return TIM1;
			case 2:
				return TIM2;
			case 3:
				return TIM3;
			case 4:
				return TIM4;
			case 5:
				return TIM5;
			case 6:
				return TIM6;
			case 7:
				return TIM7;
			case 8:
				return TIM8;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (number) {
			case 1:
				return RCC_TIM1;
			case 2:
				return RCC_TIM2;
			case 3:
				return RCC_TIM3;
			case 4:
				return RCC_TIM4;
			case 5:
				return RCC_TIM5;
			case 6:
				return RCC_TIM6;
			case 7:
				return RCC_TIM7;
			case 8:
				return RCC_TIM8;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_TIM1;
			case 2:
				return RST_TIM2;
			case 3:
				return RST_TIM3;
			case 4:
				return RST_TIM4;
			case 5:
				return RST_TIM5;
			case 6:
				return RST_TIM6;
			case 7:
				return RST_TIM7;
			case 8:
				return RST_TIM8;
		}
	}

	static constexpr uint8_t numberToIrq()
	{
		switch (number) {
			case 1:
				return NVIC_TIM1_UP_IRQ;
			case 2:
				return NVIC_TIM2_IRQ;
			case 3:
				return NVIC_TIM3_IRQ;
			case 4:
				return NVIC_TIM4_IRQ;
			case 5:
				return NVIC_TIM5_IRQ;
			case 6:
				return NVIC_TIM6_IRQ;
			case 7:
				return NVIC_TIM7_IRQ;
			case 8:
				return NVIC_TIM8_UP_IRQ;
		}
	}

	static constexpr uint32_t numberToResolution()
	{
		return 0xFFFFUL;
	}

	static uint32_t clock()
	{
		if (number >= 2 && number <= 7) {
			if (((RCC_CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_SHIFT) == RCC_CFGR_PPRE1_HCLK_NODIV) {
				return rcc_apb1_frequency;
			} else {
				return rcc_apb1_frequency * 2;
			}
		} else {
			if (((RCC_CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_SHIFT) == RCC_CFGR_PPRE2_HCLK_NODIV) {
				return rcc_apb2_frequency;
			} else {
				return rcc_apb2_frequency * 2;
			}
		}
	}

	virtual void handler() = 0;

	static void enableMainOutput()
	{
		if ((number == 1) || (number == 8)) {
			timer_enable_break_main_output(numberToPeriph());
		}
	}

	static void setHandler(TimerBase *base)
	{
		instance = base;
	}

private:
	static TimerBase *instance;
};

#endif // PLATFORM_STM32F1XX_TIMERBASE_HPP_
