//
// CanV1.hpp
//
//  Created on: Jun 2, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32_CANV1_HPP_
#define PLATFORM_STM32_CANV1_HPP_

#include <DroneDevice/Can.hpp>
#include <DroneDevice/Queue.hpp>
#include <DroneDevice/Stubs/MockTime.hpp>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/rcc.h>
#include <functional>

template<unsigned int>
class CanBase;

template<unsigned int number, size_t rxSize, size_t txSize, unsigned int tseg1, unsigned int tseg2,
	typename Time = Device::MockTime>
class Can : public CanBase<number> {
	using BaseType = CanBase<number>;
	static constexpr auto kPeriph{BaseType::numberToPeriph()};
	static constexpr auto kRxIrq{BaseType::numberToRxIrq()};
	static constexpr auto kTxIrq{BaseType::numberToTxIrq()};

protected:
	Queue<CanMessage, rxSize> rxQueue;
	Queue<CanMessage, txSize> txQueue;
	std::function<void ()> callback;
	CanStatistics stats;

public:
	Can(uint32_t aRate, std::function<void ()> aCallback = nullptr) :
		BaseType{},
		rxQueue{},
		txQueue{},
		callback{aCallback},
		stats{}
	{
		const uint32_t clockDivisor = BaseType::prescaler(aRate, tseg1, tseg2);

		BaseType::setHandler(this);

		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		// Set default parameters
		can_init(kPeriph,
			false,             // Time triggered communication mode
			true,              // Automatic bus-off management
			true,              // Automatic wakeup mode
			false,             // No automatic retransmission
			false,             // Receive FIFO locked mode
			true,              // Transmit FIFO priority
			CAN_BTR_SJW_1TQ,   // Resynchronization time quanta jump width
			tseg1Value(tseg1), // Time segment 1 time quanta width
			tseg2Value(tseg2), // Time segment 2 time quanta width
			clockDivisor,      // Baud rate prescaler
			false,             // Loopback
			false              // Silent mode
		);

		if (number == 2) {
			// Allocate second half of filters to CAN2
			CAN_FMR(CAN1) |= CAN_FMR_FINIT;
			CAN_FMR(CAN1) = (CAN_FMR(CAN1) & ~CAN_FMR_CAN2SB_MASK) | (14 << CAN_FMR_CAN2SB_SHIFT);
			CAN_FMR(CAN1) &= ~CAN_FMR_FINIT;
		}

		// Set default filtering settings: accept all
		can_filter_id_mask_32bit_init(number == 2 ? CAN1 : kPeriph,
			number == 2 ? 14 : 0, // Filter ID
			0, // CAN ID
			0, // CAN ID mask
			0, // Use FIFO0
			true // Enable the filter
		);

		nvic_clear_pending_irq(kRxIrq);
		nvic_enable_irq(kRxIrq);

		if (kTxIrq != kRxIrq) {
			nvic_clear_pending_irq(kTxIrq);
			nvic_enable_irq(kTxIrq);
		}

		can_enable_irq(kPeriph, CAN_IER_FMPIE0);
	}

	~Can() override
	{
		can_disable_irq(kPeriph, CAN_IER_FMPIE0 | CAN_IER_EPVIE | CAN_IER_TMEIE);

		nvic_disable_irq(kRxIrq);
		if (kTxIrq != kRxIrq) {
			nvic_disable_irq(kTxIrq);
		}
		rcc_periph_clock_disable(BaseType::numberToClockBranch());

		BaseType::setHandler(nullptr);
	}

	CanStatistics getStatistics() const
	{
		return stats;
	}

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;
	}

	void setFilters(const CanFilter *aFilters, size_t aCount)
	{
		for (size_t i = 0; i < aCount; ++i) {
			can_filter_id_mask_32bit_init(kPeriph,
				i,                // Filter ID
				aFilters[i].id,   // CAN ID
				aFilters[i].mask, // CAN ID mask
				0,                // Use FIFO0
				true              // Enable the filter
			);
		}
	}

	size_t read(CanMessage *aBuffer, size_t aLength)
	{
		nvic_disable_irq(kRxIrq);

		size_t initialLength = aLength;

		while (!rxQueue.empty() && aLength) {
			*aBuffer++ = rxQueue.pop();
			--aLength;
		}

		nvic_enable_irq(kRxIrq);
		return initialLength - aLength;
	}

	size_t write(const CanMessage *aBuffer, size_t aLength)
	{
		nvic_disable_irq(kTxIrq);

		const size_t initialLength = aLength;
		const bool pending = isTxPending() || !txQueue.empty();

		if (!pending) {
			can_enable_irq(kPeriph, CAN_IER_TMEIE | CAN_IER_EPVIE);

			while (isTxAvailable() && aLength > 0) {
				fillTxQueue(aBuffer++);
				--aLength;
			}
		}

		while (!txQueue.full() && aLength > 0) {
			txQueue.push(*aBuffer++);
			--aLength;
		}

		nvic_enable_irq(kTxIrq);
		return initialLength - aLength;
	}

protected:
	void rxHandler() override
	{
		CanMessage message;
		bool ext, rtr;
		uint8_t fmi;

		if (!std::is_same<Time, Device::MockTime>::value) {
			message.timestamp = static_cast<uint64_t>(Time::microseconds().count());
		}

		can_receive(kPeriph, 0, true, &message.id, &ext, &rtr, &fmi, &message.length, message.data, nullptr);
		++stats.rx;

		message.flags = static_cast<uint8_t>((ext ? CanMessage::EXT : 0) | (rtr ? CanMessage::RTR : 0));

		if (!rxQueue.full()) {
			rxQueue.push(message);
		}

		if (callback) {
			callback();
		}
	}

	void txHandler() override
	{
		bool queued = false;

		clearFailedTx();

		while (isTxAvailable() && !txQueue.empty()) {
			fillTxQueue(&txQueue.front());
			txQueue.pop();
			queued = true;
		}

		if (!queued && !isTxPending() && txQueue.empty()) {
			can_disable_irq(kPeriph, CAN_IER_TMEIE | CAN_IER_EPVIE);
		}
	}

	void clearFailedTx()
	{
		const uint32_t esr = CAN_ESR(kPeriph);
		const uint32_t tsr = CAN_TSR(kPeriph);

		if (!(esr & CAN_ESR_EPVF)) {
			return;
		}

		if ((tsr & (CAN_TSR_TERR0 | CAN_TSR_ALST0)) && !(tsr & CAN_TSR_ABRQ0)) {
			CAN_TSR(kPeriph) = CAN_TSR_ABRQ0;
			++stats.errors;
		}
		if ((tsr & (CAN_TSR_TERR1 | CAN_TSR_ALST1)) && !(tsr & CAN_TSR_ABRQ1)) {
			CAN_TSR(kPeriph) = CAN_TSR_ABRQ1;
			++stats.errors;
		}
		if ((tsr & (CAN_TSR_TERR2 | CAN_TSR_ALST2)) && !(tsr & CAN_TSR_ABRQ2)) {
			CAN_TSR(kPeriph) = CAN_TSR_ABRQ2;
			++stats.errors;
		}
	}

	bool isTxAvailable()
	{
		static constexpr uint32_t mailboxMask = CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2;
		return (CAN_TSR(kPeriph) & mailboxMask) != 0;
	}

	bool isTxPending()
	{
		static constexpr uint32_t mailboxMask = CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2;
		return (CAN_TSR(kPeriph) & mailboxMask) != mailboxMask;
	}

	void fillTxQueue(const CanMessage *aMessage)
	{
		can_transmit(kPeriph, aMessage->id, (aMessage->flags & CanMessage::EXT) != 0,
			(aMessage->flags & CanMessage::RTR) != 0, aMessage->length, aMessage->data);
		++stats.tx;
	}

	static constexpr uint32_t tseg1Value(uint8_t aValue)
	{
		assert(aValue >= 1 && aValue <= 16);
		return (aValue - 1) << CAN_BTR_TS1_SHIFT;
	}

	static constexpr uint32_t tseg2Value(uint8_t aValue)
	{
		assert(aValue >= 1 && aValue <= 8);
		return (aValue - 1) << CAN_BTR_TS2_SHIFT;
	}
};

#endif // PLATFORM_STM32_CANV1_HPP_
