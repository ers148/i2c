//
// TimerBase.hpp
//
//  Created on: Jan 10, 2019
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_TIMERBASE_HPP_
#define PLATFORM_STM32F76X_TIMERBASE_HPP_

#include <cassert>
#include <cstdint>
#include <functional>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

template<unsigned int number>
class TimerBase {
	static_assert(number >= 1 && number <= 14, "Incorrect number");

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
			case 9:
				return TIM9;
			case 10:
				return TIM10;
			case 11:
				return TIM11;
			case 12:
				return TIM12;
			case 13:
				return TIM13;
			case 14:
				return TIM14;
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
			case 9:
				return RCC_TIM9;
			case 10:
				return RCC_TIM10;
			case 11:
				return RCC_TIM11;
			case 12:
				return RCC_TIM12;
			case 13:
				return RCC_TIM13;
			case 14:
				return RCC_TIM14;
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
			case 9:
				return RST_TIM9;
			case 10:
				return RST_TIM10;
			case 11:
				return RST_TIM11;
			case 12:
				return RST_TIM12;
			case 13:
				return RST_TIM13;
			case 14:
				return RST_TIM14;
		}
	}

	static constexpr uint8_t numberToIrq()
	{
		switch (number) {
			case 1:
				return NVIC_TIM1_UP_TIM10_IRQ;
			case 2:
				return NVIC_TIM2_IRQ;
			case 3:
				return NVIC_TIM3_IRQ;
			case 4:
				return NVIC_TIM4_IRQ;
			case 5:
				return NVIC_TIM5_IRQ;
			case 6:
				return NVIC_TIM6_DAC_IRQ;
			case 7:
				return NVIC_TIM7_IRQ;
			case 8:
				return NVIC_TIM8_UP_TIM13_IRQ;
			case 9:
				return NVIC_TIM1_BRK_TIM9_IRQ;
			case 10:
				return NVIC_TIM1_UP_TIM10_IRQ;
			case 11:
				return NVIC_TIM1_TRG_COM_TIM11_IRQ;
			case 12:
				return NVIC_TIM8_BRK_TIM12_IRQ;
			case 13:
				return NVIC_TIM8_UP_TIM13_IRQ;
			case 14:
				return NVIC_TIM4_IRQ;
		}
	}

	static constexpr uint32_t numberToResolution()
	{
		switch (number) {
			case 2:
			case 5:
				return 0xFFFFFFFFUL;
			default:
				return 0xFFFFUL;
		}
	}

	static uint32_t clock()
	{
		if ((number >= 2 && number <= 7) || (number >= 12 && number <= 14)) {
			if (rcc_get_ppre1() == RCC_CFGR_PPRE_DIV_NONE) {
				return rcc_apb1_frequency;
			} else {
				return rcc_apb1_frequency * 2;
			}
		} else {
			if (rcc_get_ppre2() == RCC_CFGR_PPRE_DIV_NONE) {
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

#endif // PLATFORM_STM32F76X_TIMERBASE_HPP_
