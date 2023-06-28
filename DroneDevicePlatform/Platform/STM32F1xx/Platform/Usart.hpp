//
// Usart.hpp
//
//  Created on: Mar 22, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_USART_HPP_
#define PLATFORM_STM32F1XX_USART_HPP_

#include "Platform/UsartV1.hpp"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class UsartBase {
	static_assert(number >= 1 && number <= 3, "Incorrect number");

public:
	virtual ~UsartBase() = default;

	static void handleIrq()
	{
		instance->handler();
	}

protected:
	static constexpr uint32_t numberToPeriph()
	{
		switch (number) {
			case 1:
				return USART1;
			case 2:
				return USART2;
			case 3:
				return USART3;
		}
	}

	static constexpr uint8_t numberToIrq()
	{
		switch (number) {
			case 1:
				return NVIC_USART1_IRQ;
			case 2:
				return NVIC_USART2_IRQ;
			case 3:
				return NVIC_USART3_IRQ;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (number) {
			case 1:
				return RCC_USART1;
			case 2:
				return RCC_USART2;
			case 3:
				return RCC_USART3;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_USART1;
			case 2:
				return RST_USART2;
			case 3:
				return RST_USART3;
		}
	}

	virtual void handler() = 0;

	static void setHandler(UsartBase *base)
	{
		instance = base;
	}

private:
	static UsartBase *instance;
};

#endif // PLATFORM_STM32F1XX_USART_HPP_
