//
// UsartV2.hpp
//
//  Created on: Jun 2, 2021
//      Author: Alexander
//

#ifndef PLATFORM_STM32_PLATFORM_USARTV2_HPP_
#define PLATFORM_STM32_PLATFORM_USARTV2_HPP_

#include <DroneDevice/Queue.hpp>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>
#include <functional>

enum class UsartParityType : uint8_t {
	None,
	Even,
	Odd
};

template<unsigned int>
class UsartBase;

template<unsigned int number, size_t rxSize, size_t txSize>
class Usart : public UsartBase<number> {
	using BaseType = UsartBase<number>;

protected:
	static constexpr auto kPeriph{BaseType::numberToPeriph()};
	static constexpr auto kIrq{BaseType::numberToIrq()};

	Queue<uint8_t, rxSize> rxQueue;
	Queue<uint8_t, txSize> txQueue;
	std::function<void ()> callback;

public:
	Usart(uint32_t aRate, std::function<void ()> aCallback = nullptr) :
		BaseType{},
		rxQueue{},
		txQueue{},
		callback{aCallback}
	{
		BaseType::setHandler(this);

		rcc_periph_clock_enable(BaseType::numberToClockBranch());
		rcc_periph_reset_pulse(BaseType::numberToResetSignal());

		// Set standard UART parameters
		usart_set_baudrate(kPeriph, aRate);
		usart_set_databits(kPeriph, 8);
		usart_set_stopbits(kPeriph, USART_STOPBITS_1);
		usart_set_mode(kPeriph, USART_MODE_TX_RX);
		usart_set_parity(kPeriph, USART_PARITY_NONE);
		usart_set_flow_control(kPeriph, USART_FLOWCONTROL_NONE);

		// Enable USART Receive and IDLE interrupt
		USART_CR1(kPeriph) |= USART_CR1_RXNEIE | USART_CR1_IDLEIE;
		// Disable overrun
		USART_CR3(kPeriph) |= USART_CR3_OVRDIS;

		// Finally enable the USART
		usart_enable(kPeriph);
		nvic_clear_pending_irq(kIrq);

		// Enable the USART interrupt
		nvic_enable_irq(kIrq);
	}

	~Usart() override
	{
		USART_CR1(kPeriph) &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE | USART_CR1_IDLEIE);
		nvic_disable_irq(kIrq);
		usart_disable(kPeriph);
		rcc_periph_clock_disable(BaseType::numberToClockBranch());

		BaseType::setHandler(nullptr);
	}

	void setBaudrate(uint32_t aRate)
	{
		usart_disable(kPeriph);
		usart_set_baudrate(kPeriph, aRate);
		usart_enable(kPeriph);
	}

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;
	}

	void setParity(UsartParityType aParity)
	{
		uint32_t parity;

		switch (aParity) {
			case UsartParityType::None:
				parity = USART_PARITY_NONE;
				break;

			case UsartParityType::Even:
				parity = USART_PARITY_EVEN;
				break;

			case UsartParityType::Odd:
				parity = USART_PARITY_ODD;
				break;
		}

		if (aParity != UsartParityType::None) {
			parity |= USART_CR1_M0;
		}

		usart_disable(kPeriph);
		USART_CR1(kPeriph) = (USART_CR1(kPeriph) & ~(USART_PARITY_MASK | USART_CR1_M0)) | parity;
		usart_enable(kPeriph);
	}

	size_t read(void *aBuffer, size_t aLength)
	{
		if (!aLength) {
			return 0;
		}

		uint8_t *buffer = static_cast<uint8_t *>(aBuffer);

		while (!rxQueue.empty() && aLength--) {
			nvic_disable_irq(kIrq);
			*buffer++ = rxQueue.pop();
			nvic_enable_irq(kIrq);
		}

		return static_cast<size_t>(buffer - static_cast<uint8_t *>(aBuffer));
	}

	size_t write(const void *aBuffer, size_t aLength)
	{
		if (!aLength) {
			return 0;
		}

		const uint8_t *buffer = static_cast<const uint8_t *>(aBuffer);

		nvic_disable_irq(kIrq);
		while (!txQueue.full() && aLength--)
			txQueue.push(*buffer++);
		if (!(USART_CR1(kPeriph) & USART_CR1_TXEIE))
			USART_CR1(kPeriph) |= USART_CR1_TXEIE;
		nvic_enable_irq(kIrq);

		return static_cast<size_t>(buffer - static_cast<const uint8_t *>(aBuffer));
	}

	void startLineBreak()
	{
		usart_disable(kPeriph);
		usart_enable_tx_inversion(kPeriph);
		usart_enable(kPeriph);
	}

	void stopLineBreak()
	{
		usart_disable(kPeriph);
		usart_disable_tx_inversion(kPeriph);
		usart_enable(kPeriph);
	}

protected:
	void handler() override
	{
		const uint32_t isr = USART_ISR(kPeriph);
		bool event = false;

		// Receive byte
		if (isr & USART_ISR_RXNE) {
			const uint8_t data = static_cast<uint8_t>(USART_RDR(kPeriph));

			if (!rxQueue.full())
				rxQueue.push(data);

			if (rxQueue.size() >= rxSize / 2)
				event = true;
		}

		// Handle end of message
		if (isr & USART_ISR_IDLE) {
			USART_ICR(kPeriph) = USART_ICR_IDLECF;
			event = true;
		}

		// Transmit byte
		if (USART_CR1(kPeriph) & USART_CR1_TXEIE) {
			if (isr & USART_ISR_TXE) {
				if (txQueue.empty()) {
					USART_CR1(kPeriph) &= ~USART_CR1_TXEIE;
				} else {
					USART_TDR(kPeriph) = txQueue.pop();
				}
			}
		}

		if (event && callback != nullptr)
			callback();
	}
};

#endif // PLATFORM_STM32_PLATFORM_USARTV2_HPP_
