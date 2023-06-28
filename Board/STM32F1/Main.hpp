//! @file Main.hpp
//! @author Aleksei Drovnenkov
//! @date Oct 14, 2022

#ifndef BOARD_STM32F1_MAIN_HPP_
#define BOARD_STM32F1_MAIN_HPP_

#include "Board.hpp"
#include "Version.hpp"

#include <Platform/I2c.hpp>
#include <Platform/Gpio.hpp>
#include <Platform/Flash.hpp>
#include <Platform/LocalTime.hpp>

#include <chrono>

using namespace std::chrono_literals;

struct BoardImpl : Board<Flash<CONFIG_ROM_PAGE>, Flash<CONFIG_ROM_PAGE>> {
private:
	using PowerPin = Gpio<GpioPort::A, 12>;
	using I2cScl = Gpio<GpioPort::B, 6>;
	using I2cSda = Gpio<GpioPort::B, 7>;
	using WakeupPin = Gpio<GpioPort::B, 8>;

public:
	using I2c = ::I2c<1>;
	using Clock = ::LocalTime;

	static constexpr uint8_t kAddress = 0x0B;
	static constexpr std::array<uint8_t, 1> kRequest = {0x22};
	static constexpr std::array<uint8_t, 5> kResponse = {0x04, 'L', 'I', 'O', 'N'};

	static void configure() {
		Clock::init();

		PowerPin::init(GpioMode::OUTPUT_50_MHZ, GpioConfig::PUSHPULL);
		PowerPin::set();

		I2cScl::init(GpioMode::OUTPUT_10_MHZ, GpioConfig::ALTFN_OPENDRAIN);
		I2cSda::init(GpioMode::OUTPUT_10_MHZ, GpioConfig::ALTFN_OPENDRAIN);

		WakeupPin::init(GpioMode::OUTPUT_50_MHZ, GpioConfig::PUSHPULL);
		WakeupPin::set();
		Clock::delay(10ms);
		WakeupPin::reset();
		WakeupPin::init(GpioMode::INPUT, GpioConfig::FLOAT);

		gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);
	}
};

#endif // BOARD_STM32F1_MAIN_HPP_

