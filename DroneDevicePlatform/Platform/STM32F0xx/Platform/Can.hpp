//
// Can.hpp
//
//  Created on: Dec 8, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_CAN_HPP_
#define PLATFORM_STM32F0XX_CAN_HPP_

#include "Platform/CanV1.hpp"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class CanBase {
	static_assert(number == 1, "Incorrect number");

public:
	virtual ~CanBase() = default;

	static void handleIrq()
	{
		if (CAN_RF0R(numberToPeriph()) & CAN_RF0R_FMP0_MASK) {
			instance->rxHandler();
		}
		instance->txHandler();
	}

protected:
	static constexpr uint32_t numberToPeriph()
	{
		return CAN1;
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		return RCC_CAN1;
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		return RST_CAN1;
	}

	static constexpr uint8_t numberToRxIrq()
	{
		return NVIC_CEC_CAN_IRQ;
	}

	static constexpr uint8_t numberToTxIrq()
	{
		return NVIC_CEC_CAN_IRQ;
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

private:
	static CanBase *instance;
};

#endif // PLATFORM_STM32F0XX_CAN_HPP_
