//! @file I2c.hpp
//! @author Aleksei Drovnenkov
//! @date Nov 07, 2022

#ifndef PLATFORM_STM32F1XX_I2C_HPP_
#define PLATFORM_STM32F1XX_I2C_HPP_

#include <Platform/I2cV1.hpp>

#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class I2cBase {
	static_assert(number > 0 && number < 3, "Incorrect number");

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
		}
	}

	static constexpr uint8_t numberToEvIrq()
	{
		switch (number) {
			case 1:
				return NVIC_I2C1_EV_IRQ;
			case 2:
				return NVIC_I2C2_EV_IRQ;
		}
	}

	static constexpr uint8_t numberToErrIrq()
	{
		switch (number) {
			case 1:
				return NVIC_I2C1_ER_IRQ;
			case 2:
				return NVIC_I2C2_ER_IRQ;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (number) {
			case 1:
				return RCC_I2C1;
			case 2:
				return RCC_I2C2;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_I2C1;
			case 2:
				return RST_I2C2;
		}
	}

	static void setHandler(I2cBase *base)
	{
		instance = base;
	}

private:
	static I2cBase *instance;
};

#endif // PLATFORM_STM32F1XX_I2C_HPP_
