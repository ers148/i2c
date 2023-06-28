//
// ExtInterrupt.hpp
//
//  Created on: Jul 2, 2019
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_EXTINTERRUPT_HPP_
#define PLATFORM_STM32F76X_EXTINTERRUPT_HPP_

#include "Platform/Gpio.hpp"
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/f7/nvic.h>
#include <functional>

enum class ExtInterruptType: uint8_t {
	RISING  = 0x00,
	FALLING = 0x01,
	BOTH    = 0x02
};

class ExtInterruptHandler {
public:
	virtual ~ExtInterruptHandler() = default;

	static void handleIrq(uint32_t aIndex)
	{
		instances[aIndex]->handler();
	}

protected:
	virtual void handler() = 0;

	static void setHandler(uint32_t aIndex, ExtInterruptHandler *aInstance)
	{
		instances[aIndex] = aInstance;
	}

private:
	static ExtInterruptHandler *instances[16];
};

template<typename Pin, ExtInterruptType type>
class ExtInterrupt: public ExtInterruptHandler {
	static constexpr auto numberToIrq()
	{
		switch (Pin::kPin) {
			case 0:
				return NVIC_EXTI0_IRQ;
			case 1:
				return NVIC_EXTI1_IRQ;
			case 2:
				return NVIC_EXTI2_IRQ;
			case 3:
				return NVIC_EXTI3_IRQ;
			case 4:
				return NVIC_EXTI4_IRQ;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				return NVIC_EXTI9_5_IRQ;
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				return NVIC_EXTI15_10_IRQ;
		}
	}

	static constexpr auto makeEventType()
	{
		switch (type) {
			case ExtInterruptType::RISING:
				return EXTI_TRIGGER_RISING;

			case ExtInterruptType::FALLING:
				return EXTI_TRIGGER_FALLING;

			case ExtInterruptType::BOTH:
				return EXTI_TRIGGER_BOTH;
		}
	}

	static constexpr auto kEvent{makeEventType()};
	static constexpr auto kIsr{numberToIrq()};

public:
	ExtInterrupt(std::function<void ()> aCallback = nullptr) :
		callback{aCallback}
	{
		ExtInterruptHandler::setHandler(Pin::kPin, this);
		exti_select_source(1UL << Pin::kPin, Pin::kPeriph);
		exti_set_trigger(1UL << Pin::kPin, kEvent);
		nvic_enable_irq(kIsr);
	}

	~ExtInterrupt() override
	{
		ExtInterruptHandler::setHandler(Pin::kPin, nullptr);
	}

	void enable()
	{
		exti_enable_request(1UL << Pin::kPin);
	}

	void disable()
	{
		exti_disable_request(1UL << Pin::kPin);
	}

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;
	}

protected:
	virtual void handler() override
	{
		if (callback != nullptr) {
			callback();
		}
	}

	std::function<void ()> callback;
};

#endif // PLATFORM_STM32F76X_EXTINTERRUPT_HPP_
