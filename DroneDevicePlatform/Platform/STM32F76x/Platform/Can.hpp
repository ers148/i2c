//
// Can.hpp
//
//  Created on: Jan 10, 2019
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_CAN_HPP_
#define PLATFORM_STM32F76X_CAN_HPP_

#include "Platform/CanV1.hpp"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class CanBase {
	static_assert(number >= 1 && number <= 3, "Incorrect number");

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
		switch (number) {
			case 1:
				return CAN1;
			case 2:
				return CAN2;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (number) {
			case 1:
				return RCC_CAN1;
			case 2:
				return RCC_CAN2;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_CAN1;
			case 2:
				return RST_CAN2;
		}
	}

	static constexpr uint8_t numberToRxIrq()
	{
		switch (number) {
			case 1:
				return NVIC_CAN1_RX0_IRQ;
			case 2:
				return NVIC_CAN2_RX0_IRQ;
		}
	}

	static constexpr uint8_t numberToTxIrq()
	{
		switch (number) {
			case 1:
				return NVIC_CAN1_TX_IRQ;
			case 2:
				return NVIC_CAN2_TX_IRQ;
		}
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

#endif // PLATFORM_STM32F76X_CAN_HPP_
