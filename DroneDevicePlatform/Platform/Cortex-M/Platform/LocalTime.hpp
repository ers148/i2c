//
// LocalTime.hpp
//
//  Created on: Nov 14, 2017
//      Author: Alexander
//

#ifndef PLATFORM_CORTEX_M_LOCALTIME_HPP_
#define PLATFORM_CORTEX_M_LOCALTIME_HPP_

#include <chrono>
#include <tuple>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

class LocalTime {
	friend void sys_tick_handler();

	static constexpr uint32_t kReloadValue = 0x1000000ULL;

	static uint32_t prescaler;
	static volatile uint32_t s;
	static volatile uint32_t us;

	static auto timestamp()
	{
		uint32_t currentS;
		uint32_t currentUS;
		uint32_t timerTicks;

		do {
			currentS = s;
			currentUS = us;
			timerTicks = kReloadValue - systick_get_value();
		} while (currentS != s || currentUS != us);

		return std::make_tuple(currentS, currentUS + timerTicks / prescaler);
	}

public:
	LocalTime() = delete;
	LocalTime(const LocalTime &) = delete;
	LocalTime &operator=(const LocalTime &) = delete;

	static void init();
	static void deinit();

	static std::chrono::microseconds microseconds()
	{
		const auto ts = timestamp();
		return std::chrono::microseconds{static_cast<uint64_t>(std::get<0>(ts)) * 1000000 + std::get<1>(ts)};
	}

	static std::chrono::milliseconds milliseconds()
	{
		const auto ts = timestamp();
		return std::chrono::milliseconds{static_cast<uint64_t>(std::get<0>(ts)) * 1000 + std::get<1>(ts) / 1000};
	}

	static std::chrono::seconds seconds()
	{
		const auto ts = timestamp();
		return std::chrono::seconds{std::get<0>(ts) + std::get<1>(ts) / 1000000};
	}

	static void delay(std::chrono::microseconds aValue)
	{
		const auto start = microseconds();
		while (microseconds() <= start + aValue);
	}
};

#endif // PLATFORM_CORTEX_M_LOCALTIME_HPP_
