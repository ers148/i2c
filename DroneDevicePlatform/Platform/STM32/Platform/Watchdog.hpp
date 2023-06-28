//
// Watchdog.hpp
//
//  Created on: Feb 8, 2018
//      Author: Arseniy Soytu
//

#ifndef PLATFORM_STM32_WATCHDOG_HPP_
#define PLATFORM_STM32_WATCHDOG_HPP_

#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/rcc.h>

class Watchdog {
public:
	Watchdog(uint32_t aPeriod)
	{
		rcc_osc_on(RCC_LSI);
		rcc_wait_for_osc_ready(RCC_LSI);

		iwdg_set_period_ms(aPeriod);
		iwdg_start();

		while (iwdg_prescaler_busy() || iwdg_reload_busy());
	}

	void reset()
	{
		iwdg_reset();
	}
};

#endif // PLATFORM_STM32_WATCHDOG_HPP_
