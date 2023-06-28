//
// Spi.hpp
//
//  Created on: Jun 4, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32_SPI_HPP_
#define PLATFORM_STM32_SPI_HPP_

#include "Platform/SpiBase.hpp"

template<unsigned int number>
class Spi : public SpiBase<number> {
	using BaseType = SpiBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};

public:
	Spi(uint8_t aMode, uint32_t aRate = 0) :
		BaseType{}
	{
		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		BaseType::init(calcHighestPrescaler(aRate), aMode);
		spi_set_unidirectional_mode(kPeriph);

		// Enable SPI
		spi_enable_software_slave_management(kPeriph);
		spi_set_nss_high(kPeriph);

		spi_enable(kPeriph);
	}

	~Spi()
	{
		spi_disable(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	void read(void *aBuffer, size_t aLength)
	{
		const auto rxBuffer = static_cast<uint8_t *>(aBuffer);

		for (size_t i = 0; i < aLength; ++i) {
			BaseType::sendByte(0xFF);
			while (!(SPI_SR(kPeriph) & SPI_SR_RXNE));
			rxBuffer[i] = BaseType::receiveByte();
		}
	}

	template<typename T>
	void read(T &buffer)
	{
		read(&buffer, sizeof(T));
	}

	void write(const void *aBuffer, size_t aLength)
	{
		const auto txBuffer = static_cast<const uint8_t *>(aBuffer);

		for (size_t i = 0; i < aLength; ++i) {
			BaseType::sendByte(txBuffer[i]);
			while (!(SPI_SR(kPeriph) & SPI_SR_RXNE));
			BaseType::receiveByte();
		}
	}

	template<typename T>
	void write(const T &aBuffer)
	{
		write(&aBuffer, sizeof(T));
	}

	void exchange(void *aRxBuffer, const void *aTxBuffer, size_t aLength)
	{
		const auto rxBuffer = static_cast<uint8_t *>(aRxBuffer);
		const auto txBuffer = static_cast<const uint8_t *>(aTxBuffer);

		for (size_t i = 0; i < aLength; ++i) {
			BaseType::sendByte(txBuffer[i]);
			while (!(SPI_SR(kPeriph) & SPI_SR_RXNE));
			rxBuffer[i] = BaseType::receiveByte();
		}
	}

	void setCallback(std::function<void ()>)
	{
	}

private:
	static uint32_t calcHighestPrescaler(uint32_t aRate)
	{
		if (aRate) {
			const uint32_t clock = BaseType::clock();
			const uint32_t prescaler = (clock + (aRate - 1)) / aRate;
			return prescaler;
		} else {
			return 2;
		}
	}
};

#endif // PLATFORM_STM32_SPI_HPP_
