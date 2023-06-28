//
// SpiBase.hpp
//
//  Created on: Jun 4, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F76X_SPIBASE_HPP_
#define PLATFORM_STM32F76X_SPIBASE_HPP_

#include "Platform/AsmHelpers.hpp"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

enum class SpiDmaRemap {
	Default,
	// SPI1
	Spi1Rx0Tx3,
	Spi1Rx0Tx5,
	Spi1Rx2Tx3,
	Spi1Rx2Tx5,
	// SPI2
	Spi2Rx3Tx4,
	Spi2Rx3Tx6,
	Spi2Rx1Tx4,
	Spi2Rx1Tx6,
	// SPI3
	Spi3Rx0Tx5,
	Spi3Rx0Tx7,
	Spi3Rx2Tx5,
	Spi3Rx2Tx7,
	// SPI4
	Spi4Rx0Tx1,
	Spi4Rx3Tx1,
	Spi4Rx0Tx4,
	Spi4Rx3Tx4,
	Spi4Rx0Tx2,
	Spi4Rx3Tx2,
	// SPI5
	Spi5Rx3Tx4,
	Spi5Rx3Tx6,
	Spi5Rx5Tx4,
	Spi5Rx5Tx6
};

template<unsigned int number, unsigned int dma = 0, SpiDmaRemap remap = SpiDmaRemap::Default>
class SpiBase {
	static_assert(number >= 1 && number <= 6, "Incorrect number");
	static_assert(dma == 0
		|| (dma == 1 && number >= 2 && number <= 3)
		|| (dma == 2 && number >= 4 && number <= 6)
		|| (dma == 2 && number == 1),
		"Incorrect DMA controller configuration");
	static_assert(remap == SpiDmaRemap::Default
		|| (number == 1 && remap >= SpiDmaRemap::Spi1Rx0Tx3 && remap <= SpiDmaRemap::Spi1Rx2Tx5)
		|| (number == 2 && remap >= SpiDmaRemap::Spi2Rx3Tx4 && remap <= SpiDmaRemap::Spi2Rx1Tx6)
		|| (number == 3 && remap >= SpiDmaRemap::Spi3Rx0Tx5 && remap <= SpiDmaRemap::Spi3Rx2Tx7)
		|| (number == 4 && remap >= SpiDmaRemap::Spi4Rx0Tx1 && remap <= SpiDmaRemap::Spi4Rx3Tx2)
		|| (number == 5 && remap >= SpiDmaRemap::Spi5Rx3Tx4 && remap <= SpiDmaRemap::Spi5Rx5Tx6),
		"Incorrect DMA remap configuration");

private:
	struct StreamEvent {
		unsigned int stream;
		unsigned int event;
	};

	static constexpr StreamEvent numberToDmaRxPair()
	{
		switch (number) {
			case 1: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi1Rx0Tx3
					|| remap == SpiDmaRemap::Spi1Rx0Tx5) ?
						StreamEvent{0, 3} : StreamEvent{2, 3};
			}
			case 2: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi2Rx3Tx4
					|| remap == SpiDmaRemap::Spi2Rx3Tx6) ?
						StreamEvent{3, 0} : StreamEvent{1, 9};
			}
			case 3: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi3Rx0Tx5
					|| remap == SpiDmaRemap::Spi3Rx0Tx7) ?
						StreamEvent{0, 0} : StreamEvent{2, 0};
			}
			case 4: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi4Rx0Tx1
					|| remap == SpiDmaRemap::Spi4Rx0Tx4
					|| remap == SpiDmaRemap::Spi4Rx0Tx2) ?
						StreamEvent{0, 4} : StreamEvent{3, 5};
			}
			case 5: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi3Rx0Tx5
					|| remap == SpiDmaRemap::Spi3Rx0Tx7) ?
						StreamEvent{3, 2} : StreamEvent{5, 7};
			}
			case 6: {
				return StreamEvent{6, 1};
			}
		}
	}

	static constexpr StreamEvent numberToDmaTxPair()
	{
		switch (number) {
			case 1: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi1Rx0Tx3
					|| remap == SpiDmaRemap::Spi1Rx2Tx3) ?
						StreamEvent{3, 3} : StreamEvent{5, 3};
			}
			case 2: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi2Rx3Tx4
					|| remap == SpiDmaRemap::Spi2Rx1Tx4) ?
						StreamEvent{4, 0} : StreamEvent{6, 9};
			}
			case 3: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi3Rx0Tx5
					|| remap == SpiDmaRemap::Spi3Rx2Tx5) ?
						StreamEvent{5, 0} : StreamEvent{7, 0};
			}
			case 4: {
				switch (remap) {
					case SpiDmaRemap::Spi4Rx0Tx4:
					case SpiDmaRemap::Spi4Rx3Tx4:
						return StreamEvent{4, 5};
					case SpiDmaRemap::Spi4Rx0Tx2:
					case SpiDmaRemap::Spi4Rx3Tx2:
						return StreamEvent{2, 9};
					default:
						return StreamEvent{1, 4};
				}
			}
			case 5: {
				return (remap == SpiDmaRemap::Default
					|| remap == SpiDmaRemap::Spi5Rx3Tx4
					|| remap == SpiDmaRemap::Spi5Rx5Tx4) ?
						StreamEvent{4, 2} : StreamEvent{6, 7};
			}
			case 6: {
				return StreamEvent{5, 1};
			}
		}
	}

protected:
	static constexpr uint32_t numberToPeriph()
	{
		switch (number) {
			case 1:
				return SPI1;
			case 2:
				return SPI2;
			case 3:
				return SPI3;
			case 4:
				return SPI4;
			case 5:
				return SPI5;
			case 6:
				return SPI6;
		}
	}

	static constexpr rcc_periph_clken numberToClockBranch()
	{
		switch (number) {
			case 1:
				return RCC_SPI1;
			case 2:
				return RCC_SPI2;
			case 3:
				return RCC_SPI3;
			case 4:
				return RCC_SPI4;
			case 5:
				return RCC_SPI5;
			case 6:
				return RCC_SPI6;
		}
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		switch (number) {
			case 1:
				return RST_SPI1;
			case 2:
				return RST_SPI2;
			case 3:
				return RST_SPI3;
			case 4:
				return RST_SPI4;
			case 5:
				return RST_SPI5;
			case 6:
				return RST_SPI6;
		}
	}

	static constexpr unsigned int numberToDmaController()
	{
		if (dma == 0) {
			return (number == 2 || number == 3) ? 1 : 2;
		} else {
			return dma;
		}
	}

	static constexpr unsigned int numberToDmaRxChannel()
	{
		return numberToDmaRxPair().stream;
	}

	static constexpr unsigned int numberToDmaRxEvent()
	{
		return numberToDmaRxPair().event;
	}

	static constexpr unsigned int numberToDmaTxChannel()
	{
		return numberToDmaTxPair().stream;
	}

	static constexpr unsigned int numberToDmaTxEvent()
	{
		return numberToDmaTxPair().event;
	}

	static uint32_t clock()
	{
		if (number >= 2 && number <= 3) {
			return rcc_apb1_frequency;
		} else {
			return rcc_apb2_frequency;
		}
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

#endif // PLATFORM_STM32F76X_SPIBASE_HPP_
