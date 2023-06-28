//
// Dma.hpp
//
//  Created on: Jun 4, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_DMA_HPP_
#define PLATFORM_STM32F1XX_DMA_HPP_

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
	MemToMem,
	PeriphToPeriph
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

template<unsigned int controller, unsigned int channel>
class DmaBase {
	static_assert(controller >= 1 && controller <= 2, "Incorrect controller");
	static_assert((controller == 1 && channel >= 1 && channel <= 7)
		|| (controller == 2 && channel >= 1 && channel <= 5),
		"Incorrect channel");

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

		if (aStatus & 0x02) {
			result |= Dma::Flags::TransferComplete;
		}
		if (aStatus & 0x04) {
			result |= Dma::Flags::HalfTransfer;
		}
		if (aStatus & 0x08) {
			result |= Dma::Flags::TransferError;
		}

		return result;
	}
};

template<unsigned int controller, unsigned int channel>
class DmaChannel : public DmaBase<controller, channel> {
private:
	static constexpr rcc_periph_clken numberToClockBranch()
	{
		return controller == 1 ? RCC_DMA1 : RCC_DMA2;
	}

	static constexpr uint32_t numberToPeriph()
	{
		return controller == 1 ? DMA1 : DMA2;
	}

	static constexpr uint8_t numberToIrq()
	{
		if (controller == 1) {
			return NVIC_DMA1_CHANNEL1_IRQ + (channel - 1);
		} else {
			return NVIC_DMA2_CHANNEL1_IRQ + (channel - 1);
		}
	}

	static constexpr uint8_t numberToChannel()
	{
		return DMA_CHANNEL1 + (channel - 1);
	}

	static constexpr uint32_t priorityToValue(Dma::Priority aPriority)
	{
		switch (aPriority) {
			case Dma::Priority::Low:
				return DMA_CCR_PL_LOW;

			case Dma::Priority::Medium:
				return DMA_CCR_PL_MEDIUM;

			case Dma::Priority::High:
				return DMA_CCR_PL_HIGH;

			default:
				return DMA_CCR_PL_VERY_HIGH;
		}
	}

	static constexpr uint32_t widthToMemValue(Dma::Width aWidth)
	{
		switch (aWidth) {
			case Dma::Width::HalfWord:
				return DMA_CCR_MSIZE_16BIT;

			case Dma::Width::Word:
				return DMA_CCR_MSIZE_32BIT;

			default:
				return DMA_CCR_MSIZE_8BIT;
		}
	}

	static constexpr uint32_t widthToPeriphValue(Dma::Width aWidth)
	{
		switch (aWidth) {
			case Dma::Width::HalfWord:
				return DMA_CCR_PSIZE_16BIT;

			case Dma::Width::Word:
				return DMA_CCR_PSIZE_32BIT;

			default:
				return DMA_CCR_PSIZE_8BIT;
		}
	}

	static constexpr auto kChannel{numberToChannel()};
	static constexpr auto kPeriph{numberToPeriph()};

	using BaseType = DmaBase<controller, channel>;
	using BaseType::callback;

public:
	DmaChannel(Dma::Dir aDir, Dma::Width aSrcWidth, bool aSrcIncrement, Dma::Width aDstWidth, bool aDstIncrement,
		unsigned int, std::function<void (uint32_t)> aCallback, bool aCircular = false,
		Dma::Priority aPriority = Dma::Priority::VeryHigh) :
		BaseType{aCallback},
		dir{aDir == Dma::Dir::PeriphToPeriph || aDir == Dma::Dir::MemToPeriph}
	{
		if (!rcc_is_periph_clock_enabled(numberToClockBranch())) {
			rcc_periph_clock_enable(numberToClockBranch());
		}

		nvic_clear_pending_irq(numberToIrq());
		nvic_enable_irq(numberToIrq());

		if (controller == 2 && channel == 5) {
			nvic_clear_pending_irq(NVIC_DMA2_CHANNEL4_5_IRQ);
			nvic_enable_irq(NVIC_DMA2_CHANNEL4_5_IRQ);
		}

		dma_channel_reset(kPeriph, kChannel);

		if (aDir == Dma::Dir::MemToMem) {
			dma_enable_mem2mem_mode(kPeriph, kChannel);
		}
		if (aDir == Dma::Dir::MemToPeriph) {
			dma_set_read_from_memory(kPeriph, kChannel);
		} else {
			dma_set_read_from_peripheral(kPeriph, kChannel);
		}

		if (dir) {
			dma_set_memory_size(kPeriph, kChannel, widthToMemValue(aSrcWidth));
			dma_set_peripheral_size(kPeriph, kChannel, widthToPeriphValue(aDstWidth));
		} else {
			dma_set_memory_size(kPeriph, kChannel, widthToMemValue(aDstWidth));
			dma_set_peripheral_size(kPeriph, kChannel, widthToPeriphValue(aSrcWidth));
		}

		if ((aSrcIncrement && dir) || (aDstIncrement && !dir)) {
			dma_enable_memory_increment_mode(kPeriph, kChannel);
		}
		if ((aSrcIncrement && !dir) || (aDstIncrement && dir)) {
			dma_enable_peripheral_increment_mode(kPeriph, kChannel);
		}

		if (aCircular) {
			dma_enable_circular_mode(kPeriph, kChannel);
		}
		if (callback) {
			dma_enable_transfer_complete_interrupt(kPeriph, kChannel);
		}
		dma_disable_half_transfer_interrupt(kPeriph, kChannel);
		dma_set_priority(kPeriph, kChannel, priorityToValue(aPriority));

		config = DMA_CCR(kPeriph, kChannel);
	}

	void start(void *aDst, const void *aSrc, size_t aCount)
	{
		assert(aCount <= std::numeric_limits<uint16_t>::max());

		DMA_CCR(kPeriph, kChannel) = 0;

		if (dir) {
			dma_set_peripheral_address(kPeriph, kChannel, reinterpret_cast<uint32_t>(aDst));
			dma_set_memory_address(kPeriph, kChannel, reinterpret_cast<uint32_t>(aSrc));
		} else {
			dma_set_peripheral_address(kPeriph, kChannel, reinterpret_cast<uint32_t>(aSrc));
			dma_set_memory_address(kPeriph, kChannel, reinterpret_cast<uint32_t>(aDst));
		}
		dma_set_number_of_data(kPeriph, kChannel, static_cast<uint16_t>(aCount));

		BaseType::setHandler(this);
		DMA_CCR(kPeriph, kChannel) = config | DMA_CCR_EN;
	}

	void stop()
	{
		DMA_CCR(kPeriph, kChannel) &= ~DMA_CCR_EN;
	}

	void disableHalfTransferInterrupt()
	{
		config &= ~DMA_CCR_HTIE;
	}

	void enableHalfTransferInterrupt()
	{
		config |= DMA_CCR_HTIE;
	}

private:
	uint32_t config;
	bool dir;
};

#endif // PLATFORM_STM32F1XX_DMA_HPP_
