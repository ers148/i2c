//
// SpiBase.hpp
//
//  Created on: Jun 4, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_SPIBASE_HPP_
#define PLATFORM_STM32F0XX_SPIBASE_HPP_

#include "Platform/AsmHelpers.hpp"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

enum class SpiDmaRemap {
	Default,
	// SPI1 requests remapped from DMA1 streams 4, 5 to streams 6, 7
	Spi2Alternate
};

template<unsigned int number, unsigned int dma = 0, SpiDmaRemap remap = SpiDmaRemap::Default>
class SpiBase {
	static_assert(number >= 1 && number <= 2, "Incorrect number");
	static_assert(dma == 0
		|| (number == 1 && dma >= 1 && dma <= 2)
		|| (number == 2 && dma == 1),
		"Incorrect DMA controller configuration");
	static_assert((number == 1 && remap == SpiDmaRemap::Default) || number == 2,
		"Incorrect DMA remap configuration");

protected:
	static constexpr uint32_t numberToPeriph()
	{
		switch (number) {
			case 1:
				return SPI1;
			case 2:
				return SPI2;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (number) {
			case 1:
				return RCC_SPI1;
			case 2:
				return RCC_SPI2;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_SPI1;
			case 2:
				return RST_SPI2;
		}
	}

	static constexpr unsigned int numberToDmaController()
	{
		if (dma == 0) {
			return 1;
		} else {
			return dma;
		}
	}

	static constexpr unsigned int numberToDmaRxChannel()
	{
		switch (number) {
			case 1:
				return numberToDmaController() == 1 ? 2 : 3;
			case 2:
				return remap == SpiDmaRemap::Spi2Alternate ? 6 : 4;
		}
	}

	static constexpr unsigned int numberToDmaRxEvent()
	{
		return 0;
	}

	static constexpr unsigned int numberToDmaTxChannel()
	{
		switch (number) {
			case 1:
				return numberToDmaController() == 1 ? 3 : 4;
			case 2:
				return remap == SpiDmaRemap::Spi2Alternate ? 7 : 5;
		}
	}

	static constexpr unsigned int numberToDmaTxEvent()
	{
		return 0;
	}

	static uint32_t clock()
	{
		return rcc_apb1_frequency;
	}

	static void init(uint32_t aPrescaler, uint8_t aMode)
	{
		assert(aPrescaler >= 2 && aPrescaler <= 256);

		const uint32_t cpha = (aMode == 1 || aMode == 3) ? SPI_CR1_CPHA : 0;
		const uint32_t cpol = (aMode == 2 || aMode == 3) ? SPI_CR1_CPOL : 0;
		uint32_t prescaler = 30 - countLeadingZeros32(aPrescaler);

		if ((aPrescaler & (aPrescaler - 1)) != 0) {
			++prescaler;
		}

		spi_init_master(numberToPeriph(), prescaler << 3, cpol, cpha, SPI_CR1_MSBFIRST);
		spi_set_crcl_8bit(numberToPeriph());
		SPI_CR2(numberToPeriph()) |= 1 << 12;
	}

	static uint8_t receiveByte()
	{
		return SPI_DR8(numberToPeriph());
	}

	static void sendByte(uint8_t aData)
	{
		SPI_DR8(numberToPeriph()) = aData;
	}

	static void *dataRegAddress()
	{
		return reinterpret_cast<void *>(const_cast<uint8_t *>(&SPI_DR8(numberToPeriph())));
	}
};

#endif // PLATFORM_STM32F0XX_SPIBASE_HPP_
