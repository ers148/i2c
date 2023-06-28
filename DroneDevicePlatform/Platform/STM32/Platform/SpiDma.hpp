//
// SpiDma.hpp
//
//  Created on: Jun 2, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32_SPIDMA_HPP_
#define PLATFORM_STM32_SPIDMA_HPP_

#include "Platform/Dma.hpp"
#include "Platform/SpiBase.hpp"

template<unsigned int number, unsigned int dma = 0, SpiDmaRemap remap = SpiDmaRemap::Default>
class SpiDma : public SpiBase<number, dma, remap> {
	using BaseType = SpiBase<number, dma, remap>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};

	DmaChannel<BaseType::numberToDmaController(), BaseType::numberToDmaRxChannel()> rxDma;
	DmaChannel<BaseType::numberToDmaController(), BaseType::numberToDmaRxChannel()> rxDmaStub;
	DmaChannel<BaseType::numberToDmaController(), BaseType::numberToDmaTxChannel()> txDma;
	DmaChannel<BaseType::numberToDmaController(), BaseType::numberToDmaTxChannel()> txDmaStub;
	std::function<void ()> callback;
	uint32_t stub;
	uint8_t invoked;

public:
	SpiDma(uint8_t aMode, uint32_t aRate = 0, std::function<void ()> aCallback = nullptr) :
		BaseType{},
		rxDma{
			Dma::Dir::PeriphToMem,
			Dma::Width::Byte,
			false,
			Dma::Width::Byte,
			true,
			BaseType::numberToDmaRxEvent(),
			[this](uint32_t aFlags){ handler(aFlags); }},
		rxDmaStub{
			Dma::Dir::PeriphToMem,
			Dma::Width::Byte,
			false,
			Dma::Width::Byte,
			false,
			BaseType::numberToDmaRxEvent(),
			[this](uint32_t aFlags){ handler(aFlags); }},
		txDma{
			Dma::Dir::MemToPeriph,
			Dma::Width::Byte,
			true,
			Dma::Width::Byte,
			false,
			BaseType::numberToDmaTxEvent(),
			[this](uint32_t aFlags){ handler(aFlags); }},
		txDmaStub{
			Dma::Dir::MemToPeriph,
			Dma::Width::Byte,
			false,
			Dma::Width::Byte,
			false,
			BaseType::numberToDmaTxEvent(),
			[this](uint32_t aFlags){ handler(aFlags); }},
		callback{aCallback},
		stub{},
		invoked{0}
	{
		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		BaseType::init(calcHighestPrescaler(aRate), aMode);
		spi_set_unidirectional_mode(kPeriph);

		// Enable SPI
		spi_enable_software_slave_management(kPeriph);
		spi_set_nss_high(kPeriph);
		spi_enable_rx_dma(kPeriph);
		spi_enable_tx_dma(kPeriph);

		spi_enable(kPeriph);
	}

	~SpiDma()
	{
		spi_disable(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());
	}

	void read(void *aBuffer, size_t aLength, bool aBlocking = true)
	{
		invoked = 0;
		rxDma.start(aBuffer, BaseType::dataRegAddress(), aLength);
		txDmaStub.start(BaseType::dataRegAddress(), &stub, aLength);

		if (aBlocking) {
			while (invoked != 2) {
				barrier();
			}
		}
	}

	template<typename T>
	void read(T &aBuffer)
	{
		read(&aBuffer, sizeof(T));
	}

	void write(const void *aBuffer, size_t aLength, bool aBlocking = true)
	{
		invoked = 0;
		rxDmaStub.start(&stub, BaseType::dataRegAddress(), aLength);
		txDma.start(BaseType::dataRegAddress(), aBuffer, aLength);

		if (aBlocking) {
			while (invoked != 2) {
				barrier();
			}
		}
	}

	template<typename T>
	void write(const T &aBuffer)
	{
		write(&aBuffer, sizeof(T));
	}

	void exchange(void *aRxBuffer, const void *aTxBuffer, size_t aLength, bool aBlocking = true)
	{
		invoked = 0;
		rxDma.start(aRxBuffer, BaseType::dataRegAddress(), aLength);
		txDma.start(BaseType::dataRegAddress(), aTxBuffer, aLength);

		if (aBlocking) {
			while (invoked != 2) {
				barrier();
			}
		}
	}

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;
	}

private:
	void handler(uint32_t aFlags)
	{
		if (aFlags & Dma::Flags::TransferComplete) {
			if (callback && invoked == 1) {
				callback();
			}

			++invoked;
		}
	}

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

#endif // PLATFORM_STM32_SPIDMA_HPP_
