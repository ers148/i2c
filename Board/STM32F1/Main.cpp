//! @file Main.cpp
//! @author Aleksei Drovnenkov
//! @date Jun 15, 2023

#include "Main.hpp"
#include "Application.hpp"

constexpr uint8_t BoardImpl::kAddress;
constexpr std::array<uint8_t, 1> BoardImpl::kRequest;
constexpr std::array<uint8_t, 5> BoardImpl::kResponse;

//! @brief Clock system initialization
void initClock()
{
	rcc_clock_setup_in_hse_16mhz_out_72mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_reset_pulse(RST_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_reset_pulse(RST_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_reset_pulse(RST_AFIO);
}

//! @brief Main fucntion
int main()
{
	initClock();
	BoardImpl::configure();
	BoardImpl::I2c i2c {50'000};
	Application<BoardImpl> app {i2c};
	app.run();
	return 0;
}
