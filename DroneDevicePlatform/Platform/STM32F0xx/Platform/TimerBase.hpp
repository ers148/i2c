//
// TimerBase.hpp
//
//  Created on: Dec 22, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_TIMERBASE_HPP_
#define PLATFORM_STM32F0XX_TIMERBASE_HPP_

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

template<unsigned int number>
class TimerBase {
	static_assert(number >= 1 && number <= 17, "Incorrect number");

public:
	virtual ~TimerBase() = default;

	static void handleIrq()
	{
		if (instance) {
			instance->handler();
		}
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
			case 6:
				return TIM6;
			case 7:
				return TIM7;
			case 14:
				return TIM14;
			case 15:
				return TIM15;
			case 16:
				return TIM16;
			case 17:
				return TIM17;
		}
	}

	static constexpr uint8_t numberToIrq()
	{
		switch (number) {
			case 1:
				return NVIC_TIM1_BRK_UP_TRG_COM_IRQ;
			case 2:
				return NVIC_TIM2_IRQ;
			case 3:
				return NVIC_TIM3_IRQ;
			case 6:
				return NVIC_TIM6_DAC_IRQ;
			case 7:
				return NVIC_TIM7_IRQ;
			case 14:
				return NVIC_TIM14_IRQ;
			case 15:
				return NVIC_TIM15_IRQ;
			case 16:
				return NVIC_TIM16_IRQ;
			case 17:
				return NVIC_TIM17_IRQ;
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
			case 6:
				return RCC_TIM6;
			case 7:
				return RCC_TIM7;
			case 14:
				return RCC_TIM14;
			case 15:
				return RCC_TIM15;
			case 16:
				return RCC_TIM16;
			case 17:
				return RCC_TIM17;
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
			case 6:
				return RST_TIM6;
			case 7:
				return RST_TIM7;
			case 14:
				return RST_TIM14;
			case 15:
				return RST_TIM15;
			case 16:
				return RST_TIM16;
			case 17:
				return RST_TIM17;
		}
	}

	static constexpr uint32_t numberToResolution()
	{
		switch (number) {
			case 2:
				return 0xFFFFFFFFUL;

			default:
				return 0xFFFFUL;
		}
	}

	static uint32_t clock()
	{
		if (((RCC_CFGR & RCC_CFGR_PPRE) >> RCC_CFGR_PPRE_SHIFT) == RCC_CFGR_PPRE_NODIV) {
			return rcc_apb1_frequency;
		} else {
			return rcc_apb1_frequency * 2;
		}
	}

	virtual void handler() = 0;

	static void enableMainOutput()
	{
		if ((number == 1) || (number == 15) || (number == 16) || (number == 17)) {
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

#endif // PLATFORM_STM32F0XX_TIMERBASE_HPP_
