//! @file Main.cpp
//! @author Aleksei Drovnenkov
//! @date Jun 08, 2023

#include "Main.hpp"
#include "Application.hpp"

constexpr uint8_t BoardImpl::kAddress;
constexpr std::array<uint8_t, 1> BoardImpl::kRequest;
constexpr std::array<uint8_t, 5> BoardImpl::kResponse;

constexpr auto kAhbClock {48'000'000};
constexpr auto kApbClock {48'000'000};
constexpr auto kI2cRate {I2CRate::RATE_50kHz};
constexpr auto kI2cDataSetupTime {DataTimeSetup::TIME_1000_NS};
constexpr auto kI2cDataHoldTime {DataTimeSetup::TIME_000_NS};
constexpr auto kI2cTimingRegValue {calcTimingRegValue(kApbClock, kI2cRate, kI2cDataSetupTime, kI2cDataHoldTime)};

//! @brief Clock system initialization
static void
initClock()
{
	rcc_osc_on(RCC_HSE);
	rcc_wait_for_osc_ready(RCC_HSE);
	rcc_set_sysclk_source(RCC_HSE);

	rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
	rcc_set_ppre(RCC_CFGR_PPRE_NODIV);

	flash_prefetch_enable();
	flash_set_ws(FLASH_ACR_LATENCY_024_048MHZ);

	// PLL: 16MHz / 1 * 3 = 48MHz
	rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_MUL3);
	rcc_set_pll_source(RCC_CFGR_PLLSRC_HSE_CLK);
	rcc_set_pllxtpre(RCC_CFGR_PLLXTPRE_HSE_CLK);

	rcc_osc_on(RCC_PLL);
	rcc_wait_for_osc_ready(RCC_PLL);
	rcc_set_sysclk_source(RCC_PLL);

	rcc_apb1_frequency = kApbClock;
	rcc_ahb_frequency = kAhbClock;

	rcc_periph_clock_enable(RCC_SYSCFG_COMP);

	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_reset_pulse(RST_GPIOB);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_reset_pulse(RST_GPIOA);
}

//! @brief Main fucntion
int main()
{
	initClock();
	BoardImpl::configure();
	BoardImpl::I2c i2c {kI2cTimingRegValue};
	Application<BoardImpl> app {i2c};
	app.run();
	return 0;
}
