//
// Dma.hpp
//
//  Created on: Jun 4, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F4XX_DMA_HPP_
#define PLATFORM_STM32F4XX_DMA_HPP_

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/rcc.h>
#include <cassert>
#include <functional>
#include <limits>

namespace Dma {

enum class Dir {
	PeriphToMem,
	MemToPeriph,
	MemToMem
};

enum class Priority {
	Low,
	Medium,
	High,
	VeryHigh
};

enum class Width {
	Byte,
	HalfWord,
	Word
};

namespace Flags {

// clang-format off
enum : uint32_t {
	TransferComplete = 0x01,
	HalfTransfer     = 0x02,
	TransferError    = 0x04
};
// clang-format on

} // namespace Flags

} // namespace Dma

template<unsigned int controller, unsigned int stream>
class DmaBase {
	static_assert(controller >= 1 && controller <= 2, "Incorrect controller");
	static_assert(stream >= 0 && stream <= 7, "Incorrect stream");

public:
	virtual ~DmaBase() = default;

	DmaBase(std::function<void (uint32_t)> aCallback) :
		callback{aCallback}
	{
	}

	static void handleIrq(uint32_t aStatus)
	{
		instance->callback(statusToFlags(aStatus));
	}

protected:
	std::function<void (uint32_t)> callback;

	static void setHandler(DmaBase *base)
	{
		instance = base;
	}

private:
	static DmaBase *instance;

	static uint32_t statusToFlags(uint32_t aStatus)
	{
		uint32_t result = 0;

		if (aStatus & 0x20) {
			result |= Dma::Flags::TransferComplete;
		}
		if (aStatus & 0x10) {
			result |= Dma::Flags::HalfTransfer;
		}
		if (aStatus & 0x0D) {
			result |= Dma::Flags::TransferError;
		}

		return result;
	}
};

template<unsigned int controller, unsigned int stream>
class DmaChannel : public DmaBase<controller, stream> {
private:
	static constexpr rcc_periph_clken numberToClockBranch()
	{
		return controller == 1 ? RCC_DMA1 : RCC_DMA2;
	}

	static constexpr rcc_periph_rst numberToResetSignal()
	{
		return controller == 1 ? RST_DMA1 : RST_DMA2;
	}

	static constexpr uint32_t numberToPeriph()
	{
		return controller == 1 ? DMA1 : DMA2;
	}

	static constexpr uint8_t numberToIrq()
	{
		if (controller == 1) {
			if (stream < 7) {
				return NVIC_DMA1_STREAM0_IRQ + stream;
			} else {
				return NVIC_DMA1_STREAM7_IRQ;
			}
		} else {
			if (stream < 5) {
				return NVIC_DMA2_STREAM0_IRQ + stream;
			} else {
				return NVIC_DMA2_STREAM5_IRQ + (stream - 5);
			}
		}
	}

	static constexpr uint8_t numberToStream()
	{
		return DMA_STREAM0 + stream;
	}

	static constexpr uint32_t dirToValue(Dma::Dir aDir)
	{
		switch (aDir) {
			case Dma::Dir::PeriphToMem:
				return DMA_SxCR_DIR_PERIPHERAL_TO_MEM;

			case Dma::Dir::MemToPeriph:
				return DMA_SxCR_DIR_MEM_TO_PERIPHERAL;

			default:
				return DMA_SxCR_DIR_MEM_TO_MEM;
		}
	}

	static constexpr uint32_t priorityToValue(Dma::Priority aPriority)
	{
		switch (aPriority) {
			case Dma::Priority::Low:
				return DMA_SxCR_PL_LOW;

			case Dma::Priority::Medium:
				return DMA_SxCR_PL_MEDIUM;

			case Dma::Priority::High:
				return DMA_SxCR_PL_HIGH;

			default:
				return DMA_SxCR_PL_VERY_HIGH;
		}
	}

	static constexpr uint32_t widthToMemValue(Dma::Width aWidth)
	{
		switch (aWidth) {
			case Dma::Width::HalfWord:
				return DMA_SxCR_MSIZE_16BIT;

			case Dma::Width::Word:
				return DMA_SxCR_MSIZE_32BIT;

			default:
				return DMA_SxCR_MSIZE_8BIT;
		}
	}

	static constexpr uint32_t widthToPeriphValue(Dma::Width aWidth)
	{
		switch (aWidth) {
			case Dma::Width::HalfWord:
				return DMA_SxCR_PSIZE_16BIT;

			case Dma::Width::Word:
				return DMA_SxCR_PSIZE_32BIT;

			default:
				return DMA_SxCR_PSIZE_8BIT;
		}
	}

	static constexpr auto kStream{numberToStream()};
	static constexpr auto kPeriph{numberToPeriph()};

	using BaseType = DmaBase<controller, stream>;
	using BaseType::callback;

public:
	DmaChannel(Dma::Dir aDir, Dma::Width aSrcWidth, bool aSrcIncrement, Dma::Width aDstWidth, bool aDstIncrement,
		unsigned int aEvent, std::function<void (uint32_t)> aCallback, bool aCircular = false,
		Dma::Priority aPriority = Dma::Priority::VeryHigh) :
		BaseType{aCallback},
		dir{aDir == Dma::Dir::MemToPeriph}
	{
		if (!rcc_is_periph_clock_enabled(numberToClockBranch())) {
			rcc_periph_clock_enable(numberToClockBranch());
			rcc_periph_reset_pulse(numberToResetSignal());
		}

		nvic_clear_pending_irq(numberToIrq());
		nvic_enable_irq(numberToIrq());

		dma_stream_reset(kPeriph, kStream);
		dma_channel_select(kPeriph, kStream, DMA_SxCR_CHSEL(aEvent));
		dma_set_transfer_mode(kPeriph, kStream, dirToValue(aDir));

		if (dir) {
			dma_set_memory_size(kPeriph, kStream, widthToMemValue(aSrcWidth));
			dma_set_peripheral_size(kPeriph, kStream, widthToPeriphValue(aDstWidth));
		} else {
			dma_set_memory_size(kPeriph, kStream, widthToMemValue(aDstWidth));
			dma_set_peripheral_size(kPeriph, kStream, widthToPeriphValue(aSrcWidth));
		}

		if ((aSrcIncrement && dir) || (aDstIncrement && !dir)) {
			dma_enable_memory_increment_mode(kPeriph, kStream);
		}
		if ((aSrcIncrement && !dir) || (aDstIncrement && dir)) {
			dma_enable_peripheral_increment_mode(kPeriph, kStream);
		}

		if (aCircular) {
			dma_enable_circular_mode(kPeriph, kStream);
		}
		if (callback) {
			dma_enable_transfer_complete_interrupt(kPeriph, kStream);
		}
		dma_set_priority(kPeriph, kStream, priorityToValue(aPriority));

		config = DMA_SCR(kPeriph, kStream);
	}

	void start(void *aDst, const void *aSrc, size_t aCount)
	{
		assert(aCount <= std::numeric_limits<uint16_t>::max());

		DMA_SCR(kPeriph, kStream) = 0;

		if (dir) {
			dma_set_peripheral_address(kPeriph, kStream, reinterpret_cast<uint32_t>(aDst));
			dma_set_memory_address(kPeriph, kStream, reinterpret_cast<uint32_t>(aSrc));
		} else {
			dma_set_peripheral_address(kPeriph, kStream, reinterpret_cast<uint32_t>(aSrc));
			dma_set_memory_address(kPeriph, kStream, reinterpret_cast<uint32_t>(aDst));
		}
		dma_set_number_of_data(kPeriph, kStream, static_cast<uint16_t>(aCount));

		BaseType::setHandler(this);
		DMA_SCR(kPeriph, kStream) = config | DMA_SxCR_EN;
	}

	void stop()
	{
		DMA_SCR(kPeriph, kStream) &= ~DMA_SxCR_EN;
	}

private:
	uint32_t config;
	bool dir;
};

#endif // PLATFORM_STM32F4XX_DMA_HPP_
