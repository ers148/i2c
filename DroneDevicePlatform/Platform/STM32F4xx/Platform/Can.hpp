//
// Can.hpp
//
//  Created on: Dec 8, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F4XX_CAN_HPP_
#define PLATFORM_STM32F4XX_CAN_HPP_

#include "Platform/CanV1.hpp"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class CanBase {
	static_assert(number >= 1 && number <= 2, "Incorrect number");

public:
	virtual ~CanBase() = default;

	static void handleRxIrq()
	{
		instance->rxHandler();
	}

	static void handleTxIrq()
	{
		instance->txHandler();
	}

protected:
	static constexpr uint32_t numberToPeriph()
	{
		return number == 1 ? CAN1 : CAN2;
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		return number == 1 ? RCC_CAN1 : RCC_CAN2;
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		return number == 1 ? RST_CAN1 : RST_CAN2;
	}

	static constexpr uint8_t numberToRxIrq()
	{
		return number == 1 ? NVIC_CAN1_RX0_IRQ : NVIC_CAN2_RX0_IRQ;
	}

	static constexpr uint8_t numberToTxIrq()
	{
		return number == 1 ? NVIC_CAN1_TX_IRQ : NVIC_CAN2_TX_IRQ;
	}

	static constexpr uint32_t prescaler(uint32_t rate, uint8_t tseg1, uint8_t tseg2)
	{
		return rcc_apb1_frequency / (1 + tseg1 + tseg2) / rate;
	}

	virtual void rxHandler() = 0;
	virtual void txHandler() = 0;

	static void setHandler(CanBase *base)
	{
		instance = base;
	}

	CanBase()
	{
		// Enable peripheral clocks
		if (number == 2) {
			// Clock to CAN1 should be enabled in all cases
			rcc_periph_clock_enable(RCC_CAN1);
		}
	}

private:
	static CanBase *instance;
};

#endif // PLATFORM_STM32F4XX_CAN_HPP_
