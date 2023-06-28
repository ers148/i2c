//
// Gpio.hpp
//
//  Created on: Dec 22, 2017
//      Author: Yuriy Lebedov
//

#ifndef PLATFORM_STM32F1XX_GPIO_HPP_
#define PLATFORM_STM32F1XX_GPIO_HPP_

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

enum class GpioMode : uint8_t {
	INPUT         = 0x00,
	OUTPUT_10_MHZ = 0x01,
	OUTPUT_2_MHZ  = 0x02,
	OUTPUT_50_MHZ = 0x03
};

enum class GpioConfig : uint8_t {
	ANALOG          = 0x00,
	FLOAT           = 0x01,
	UPDOWN          = 0x02,
	PUSHPULL        = 0x00,
	OPENDRAIN       = 0x01,
	ALTFN_PUSHPULL  = 0x02,
	ALTFN_OPENDRAIN = 0x03
};

enum class GpioPort : uint32_t {
	A,
	B,
	C,
	D,
	E
};

template<GpioPort port, unsigned int pin>
class Gpio {
	static_assert(port >= GpioPort::A && port <= GpioPort::E, "Incorrect port");
	static_assert(pin < 16, "Incorrect pin");

public:
	static constexpr auto kPort{port};
	static constexpr auto kPin{pin};

	static bool read()
	{
		return gpio_get(kPeriph, 1U << kPin) != 0;
	}

	static void write(bool value)
	{
		if (value) {
			set();
		} else {
			reset();
		}
	}

	static void set()
	{
		gpio_set(kPeriph, 1U << kPin);
	}

	static void reset()
	{
		gpio_clear(kPeriph, 1U << kPin);
	}

	static void toggle()
	{
		gpio_toggle(kPeriph, 1U << kPin);
	}

	static void init(GpioMode mode, GpioConfig config)
	{
		if (!rcc_is_periph_clock_enabled(numberToClockBranch())) {
			rcc_periph_clock_enable(numberToClockBranch());
			rcc_periph_reset_pulse(numberToResetSignal());
		}

		gpio_set_mode(kPeriph, static_cast<uint8_t>(mode), static_cast<uint8_t>(config), 1U << kPin);
	}

	Gpio() = delete;
	Gpio(const Gpio &) = delete;
	Gpio &operator=(const Gpio &) = delete;

private:
	static constexpr uint32_t numberToPort()
	{
		switch (port) {
			case GpioPort::A:
				return GPIOA;
			case GpioPort::B:
				return GPIOB;
			case GpioPort::C:
				return GPIOC;
			case GpioPort::D:
				return GPIOD;
			case GpioPort::E:
				return GPIOE;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (port) {
			case GpioPort::A:
				return RCC_GPIOA;
			case GpioPort::B:
				return RCC_GPIOB;
			case GpioPort::C:
				return RCC_GPIOC;
			case GpioPort::D:
				return RCC_GPIOD;
			case GpioPort::E:
				return RCC_GPIOE;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (port) {
			case GpioPort::A:
				return RST_GPIOA;
			case GpioPort::B:
				return RST_GPIOB;
			case GpioPort::C:
				return RST_GPIOC;
			case GpioPort::D:
				return RST_GPIOD;
			case GpioPort::E:
				return RST_GPIOE;
		}
	}

public:
	static constexpr auto kPeriph{numberToPort()};
};

#endif // PLATFORM_STM32F1XX_GPIO_HPP_
