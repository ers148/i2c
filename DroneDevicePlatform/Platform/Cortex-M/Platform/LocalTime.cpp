/*
 * LocalTime.cpp
 *
 *  Created on: Nov 16, 2017
 *      Author: Alexander
 */

#include "LocalTime.hpp"
#include <libopencm3/stm32/rcc.h>

uint32_t LocalTime::prescaler = 1;
volatile uint32_t LocalTime::s = 0;
volatile uint32_t LocalTime::us = 0;

void sys_tick_handler()
{
	LocalTime::us += LocalTime::kReloadValue / LocalTime::prescaler;

	if (LocalTime::us >= 1000000) {
		LocalTime::s += 1;
		LocalTime::us -= 1000000;
	}
}

void LocalTime::init()
{
  prescaler = rcc_ahb_frequency / 1000000;

  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_set_reload(kReloadValue - 1);
  systick_interrupt_enable();

  // Start counting
  systick_counter_enable();
}

void LocalTime::deinit()
{
  systick_counter_disable();
  systick_interrupt_disable();
}
