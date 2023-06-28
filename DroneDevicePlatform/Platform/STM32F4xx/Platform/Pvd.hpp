//
// Pvd.hpp
//
//  Created on: Jan 11, 2018
//      Author: Alexander
//

#ifndef PLATFORM_STM32F4XX_PVD_HPP_
#define PLATFORM_STM32F4XX_PVD_HPP_

#include <cstdint>
#include <functional>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/pwr.h>

class Pvd {
	friend void pvd_isr();

	static constexpr uint32_t kExti{EXTI16};
	static constexpr uint8_t kIrq{NVIC_PVD_IRQ};

public:
	enum Level: uint32_t {
		LEVEL_2V2 = PWR_CR_PLS_2V2,
		LEVEL_2V3 = PWR_CR_PLS_2V3,
		LEVEL_2V4 = PWR_CR_PLS_2V4,
		LEVEL_2V5 = PWR_CR_PLS_2V5,
		LEVEL_2V6 = PWR_CR_PLS_2V6,
		LEVEL_2V7 = PWR_CR_PLS_2V7,
		LEVEL_2V8 = PWR_CR_PLS_2V8,
		LEVEL_2V9 = PWR_CR_PLS_2V9
	};

	Pvd(Level level) :
		callback{nullptr}
	{
		pwr_enable_power_voltage_detect(level);

		exti_set_trigger(kExti, EXTI_TRIGGER_RISING);
		exti_enable_request(kExti);

		nvic_clear_pending_irq(kIrq);
	}

	~Pvd()
	{
		nvic_disable_irq(kIrq);
		pwr_disable_power_voltage_detect();
	}

	bool isVoltageHigher() const
	{
		return pwr_voltage_high();
	}

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;

		if (callback != nullptr) {
			nvic_enable_irq(kIrq);
		} else {
			nvic_disable_irq(kIrq);
		}
	}

private:
	Pvd(const Pvd &) = delete;
	Pvd &operator=(const Pvd &) = delete;

	std::function<void ()> callback;

	static void handleIrq()
	{
		instance->callback();
		exti_reset_request(kExti);
	}

	static Pvd *instance;
};

#endif // PLATFORM_STM32F4XX_PVD_HPP_
