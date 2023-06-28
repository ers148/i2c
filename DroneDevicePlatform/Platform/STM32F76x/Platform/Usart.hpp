//
// Usart.hpp
//
//  Created on: Nov 13, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_USART_HPP_
#define PLATFORM_STM32F76X_USART_HPP_

#include "Platform/UsartV2.hpp"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class UsartBase {
	static_assert(number >= 1 && number <= 8, "Incorrect number");

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
			case 4:
				return UART4;
			case 5:
				return UART5;
			case 6:
				return USART6;
			case 7:
				return UART7;
			case 8:
				return UART8;
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
			case 4:
				return NVIC_UART4_IRQ;
			case 5:
				return NVIC_UART5_IRQ;
			case 6:
				return NVIC_USART6_IRQ;
			case 7:
				return NVIC_UART7_IRQ;
			case 8:
				return NVIC_UART8_IRQ;
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
			case 4:
				return RCC_UART4;
			case 5:
				return RCC_UART5;
			case 6:
				return RCC_USART6;
			case 7:
				return RCC_UART7;
			case 8:
				return RCC_UART8;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_USART1;
			case 2:
				return RST_UART2;
			case 3:
				return RST_UART3;
			case 4:
				return RST_UART4;
			case 5:
				return RST_UART5;
			case 6:
				return RST_USART6;
			case 7:
				return RST_UART7;
			case 8:
				return RST_UART8;
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

#endif // PLATFORM_STM32F76X_USART_HPP_
