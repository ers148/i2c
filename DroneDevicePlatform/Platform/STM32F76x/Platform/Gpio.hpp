//
// Gpio.hpp
//
//  Created on: Jan 15, 2018
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_GPIO_HPP_
#define PLATFORM_STM32F76X_GPIO_HPP_

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

enum class GpioMode : uint8_t {
	INPUT  = 0x00,
	OUTPUT = 0x01,
	AF     = 0x02,
	ANALOG = 0x03
};

enum class GpioPull : uint8_t {
	NONE     = 0x00,
	PULLUP   = 0x01,
	PULLDOWN = 0x02
};

enum class GpioSpeed : uint8_t {
	SPEED_2MHZ   = 0x0,
	SPEED_25MHZ  = 0x1,
	SPEED_50MHZ  = 0x2,
	SPEED_100MHZ = 0x3
};

enum class GpioType : uint8_t {
	PUSH_PULL  = 0x00,
	OPEN_DRAIN = 0x01
};

enum class GpioPort : uint32_t {
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K
};

template<GpioPort port, unsigned int pin>
class Gpio {
	static_assert(port >= GpioPort::A && port <= GpioPort::K, "Incorrect port");
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

	static void init(GpioMode mode, GpioPull pull = GpioPull::NONE, GpioType type = GpioType::PUSH_PULL,
		GpioSpeed speed = GpioSpeed::SPEED_100MHZ)
	{
		if (!rcc_is_periph_clock_enabled(numberToClockBranch())) {
			rcc_periph_clock_enable(numberToClockBranch());
			rcc_periph_reset_pulse(numberToResetSignal());
		}

		gpio_mode_setup(kPeriph, static_cast<uint8_t>(mode), static_cast<uint8_t>(pull), 1U << kPin);
		gpio_set_output_options(kPeriph, static_cast<uint8_t>(type), static_cast<uint8_t>(speed), 1U << kPin);
	}

	static void setAltFunc(uint8_t func)
	{
		gpio_set_af(kPeriph, func, 1U << kPin);
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
			case GpioPort::F:
				return GPIOF;
			case GpioPort::G:
				return GPIOG;
			case GpioPort::H:
				return GPIOH;
			case GpioPort::I:
				return GPIOI;
			case GpioPort::J:
				return GPIOJ;
			case GpioPort::K:
				return GPIOK;
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
			case GpioPort::F:
				return RCC_GPIOF;
			case GpioPort::G:
				return RCC_GPIOG;
			case GpioPort::H:
				return RCC_GPIOH;
			case GpioPort::I:
				return RCC_GPIOI;
			case GpioPort::J:
				return RCC_GPIOJ;
			case GpioPort::K:
				return RCC_GPIOK;
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
			case GpioPort::F:
				return RST_GPIOF;
			case GpioPort::G:
				return RST_GPIOG;
			case GpioPort::H:
				return RST_GPIOH;
			case GpioPort::I:
				return RST_GPIOI;
			case GpioPort::J:
				return RST_GPIOJ;
			case GpioPort::K:
				return RST_GPIOK;
		}
	}

public:
	static constexpr auto kPeriph{numberToPort()};
};

#endif // PLATFORM_STM32F76X_GPIO_HPP_
