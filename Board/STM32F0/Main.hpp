//! @file Main.hpp
//! @author Aleksei Drovnenkov
//! @date Jun 08, 2023

#ifndef BOARD_STM32F0_MAIN_HPP_
#define BOARD_STM32F0_MAIN_HPP_

#include "Board.hpp"
#include "Version.hpp"

#include <Platform/I2cIrq.hpp>
#include <Platform/Gpio.hpp>
#include <Platform/Flash.hpp>
#include <Platform/LocalTime.hpp>

#include <chrono>

using namespace std::chrono_literals;

struct BoardImpl : Board<Flash<CONFIG_ROM_PAGE>, Flash<CONFIG_ROM_PAGE>> {
private:
	using PowerPin = Gpio<GpioPort::B, 2>;
	using I2cScl = Gpio<GpioPort::B, 6>;
	using I2cSda = Gpio<GpioPort::B, 7>;
	using WakeupPin = Gpio<GpioPort::B, 10>;

public:
	using I2c = ::I2c<1>;
	using Clock = ::LocalTime;

	static constexpr uint8_t kAddress = 0x0B;
	static constexpr std::array<uint8_t, 1> kRequest = {0x22};
	static constexpr std::array<uint8_t, 5> kResponse = {0x04, 'L', 'I', 'O', 'N'};


	static void configure() {
		Clock::init();

		I2cScl::init(GpioMode::AF, GpioPull::PULLUP, GpioType::OPEN_DRAIN, GpioSpeed::SPEED_HIGH);
		I2cScl::setAltFunc(GPIO_AF1);
		I2cSda::init(GpioMode::AF, GpioPull::PULLUP, GpioType::OPEN_DRAIN, GpioSpeed::SPEED_HIGH);
		I2cSda::setAltFunc(GPIO_AF1);

		PowerPin::init(GpioMode::INPUT, GpioPull::NONE);
		PowerPin::reset();

		WakeupPin::init(GpioMode::OUTPUT, GpioPull::NONE);
		WakeupPin::set();
		Clock::delay(10ms);
		WakeupPin::reset();
		WakeupPin::init(GpioMode::INPUT, GpioPull::NONE);
	}
};


#endif // BOARD_STM32F0_MAIN_HPP_
