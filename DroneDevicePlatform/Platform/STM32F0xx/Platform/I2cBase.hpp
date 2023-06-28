//! @file I2cBase.hpp
//! @author Aleksei Drovenkov
//! @date Nov 07, 2022

#ifndef PLATFORM_STM32F0XX_I2CBASE_HPP_
#define PLATFORM_STM32F0XX_I2CBASE_HPP_

#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

template<unsigned int number>
class I2cBase {
	static_assert(number > 0 && number < 3, "Incorrect number");

public:
	virtual ~I2cBase() = default;

protected:
	I2cBase(const I2cBase&) = delete;
	I2cBase& operator=(const I2cBase&) = delete;

	I2cBase()
	{
		instance = this;
	}

	virtual void handler() {};

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

	static constexpr uint8_t numberToIrq()
	{
		switch (number) {
			case 1:
				return NVIC_I2C1_IRQ;
			case 2:
				return NVIC_I2C2_IRQ;
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

private:
	static I2cBase *instance;

	friend void i2c1_isr();
	friend void i2c2_isr();
};

#endif // PLATFORM_STM32F0XX_I2CBASE_HPP_
