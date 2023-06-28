//! @file I2c.hpp
//! @author Aleksei Drovnenkov
//! @date Jun 05, 2023

#ifndef PLATFORM_STM32F7XX_I2C_HPP_
#define PLATFORM_STM32F7XX_I2C_HPP_

#include <Platform/I2cV3.hpp>

#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class I2cBase {
	static_assert(number > 0 && number < 5, "Incorrect number");

public:
	virtual ~I2cBase() = default;

	static void handleIrq()
	{
		if (instance != nullptr) {
			instance->handler();
		}
	}

protected:
	virtual void handler() = 0;

	enum class TransferDirection : uint8_t {
		Write = 0,
		Read = 1,
	};

	static constexpr uint32_t numberToPeriph()
	{
		switch (number) {
			case 1:
				return I2C1;
			case 2:
				return I2C2;
			case 3:
				return I2C3;
			case 4:
				return I2C4;
		}
	}

	static constexpr uint8_t numberToIrq()
	{
		switch (number) {
			case 1:
				return NVIC_I2C1_EV_IRQ;
			case 2:
				return NVIC_I2C2_EV_IRQ;
			case 3:
				return NVIC_I2C3_EV_IRQ;
			case 4:
				return NVIC_I2C4_EV_IRQ;
		}
	}

	static constexpr uint8_t numberToErrIrq()
	{
		switch (number) {
			case 1:
				return NVIC_I2C1_ER_IRQ;
			case 2:
				return NVIC_I2C2_ER_IRQ;
			case 3:
				return NVIC_I2C3_ER_IRQ;
			case 4:
				return NVIC_I2C4_ER_IRQ;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (number) {
			case 1:
				return RCC_I2C1;
			case 2:
				return RCC_I2C2;
			case 3:
				return RCC_I2C3;
			case 4:
				return RCC_I2C4;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_I2C1;
			case 2:
				return RST_I2C2;
			case 3:
				return RST_I2C3;
			case 4:
				return RST_I2C4;
		}
	}

	static void setHandler(I2cBase *base)
	{
		instance = base;
	}

private:
	static I2cBase *instance;
};

#endif // PLATFORM_STM32F7XX_I2C_HPP_
