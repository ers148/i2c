//! @file I2cV2Irq.hpp
//! @author Aleksei Drovenkov
//! @date Jun 16, 2023

#ifndef PLATFORM_STM32_I2CV2IRQ_HPP_
#define PLATFORM_STM32_I2CV2IRQ_HPP_

#include <Platform/I2cV2Helpers.hpp>

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/cm3/nvic.h>
#include <functional>

template<unsigned int>
class I2cBase;

template<unsigned int number>
class I2c : public I2cBase<number> {
	using BaseType = I2cBase<number>;
	using TransferDirection = typename BaseType::TransferDirection;
	using Callback = std::function<void (bool)>;

	static constexpr auto kIrq{BaseType::numberToIrq()};
	static constexpr auto kPeriph{BaseType::numberToPeriph()};
	static constexpr auto kClockBranch{BaseType::numberToClockBranch()};
	static constexpr auto kResetSignal{BaseType::numberToResetSignal()};

	static constexpr auto kAttemptLimit = 100'000;
	static constexpr auto kIntEnMask = I2C_CR1_ERRIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_RXIE | I2C_CR1_TXIE;
	static constexpr auto kIntClrMask = I2C_ICR_ARLOCF | I2C_ICR_BERRCF | I2C_ICR_NACKCF | I2C_ICR_STOPCF;
	static constexpr auto kErrMask = I2C_ISR_NACKF | I2C_ISR_BERR | I2C_ISR_ARLO;

public:
	I2c(const I2c&) = delete;
	I2c& operator=(const I2c&) = delete;

	//! @brief Constructor
	//! @param[in] aTimingRegValue - timing register value
	I2c(TimingRegValue aTimingRegValue)
	{
		rcc_set_i2c_clock_sysclk(kPeriph);
		rcc_periph_clock_enable(kClockBranch);
		i2c_enable_analog_filter(kPeriph);
		I2C_TIMINGR(kPeriph) = aTimingRegValue;
		i2c_enable_stretching(kPeriph);
		i2c_peripheral_enable(kPeriph);
		nvic_clear_pending_irq(kIrq);
		nvic_enable_irq(kIrq);
		i2c_enable_interrupt(kPeriph, kIntEnMask);
	}

	//! @brief Destructor
	~I2c()
	{
		i2c_peripheral_disable(kPeriph);
		rcc_periph_clock_disable(kClockBranch);
	}

	//! @brief Set callback
	//! @param[in] aCallback - callback
	void setCallback(Callback aCallback)
	{
		callback = aCallback;
	}

	//! @brief Bus reset
	void reset()
	{
		for (auto attempt = 0; (attempt < kAttemptLimit) && ((I2C_CR1(kPeriph) & I2C_CR1_PE) == 0); ++attempt) {
			i2c_peripheral_disable(kPeriph);
		}
		i2c_peripheral_enable(kPeriph);
	}

	//! @brief Send data
	//! @param[in] aAddr - 7-bit device address
	//! @param[in] aTxData - data for send
	//! @param[in] aTxSize - size of tx data
	//! @return true on success, false on error
	bool send(const uint8_t aAddr, const void *aTxData, uint8_t aTxSize)
	{
		return exchange(aAddr, aTxData, aTxSize, nullptr, 0);
	}

	//! @brief Receive data
	//! @param[in] aAddr - 7-bit device address
	//! @param[in] aRxBuf - buffer for received data
	//! @param[in] aRxSize - size of data to recive
	//! @return true on success, false on error
	bool receive(const uint8_t aAddr, void *aRxBuf, uint8_t aRxSize)
	{
		return exchange(aAddr, nullptr, 0, aRxBuf, aRxSize);
	}

	//! @brief Exchange data
	//! @param[in] aAddr - 7-bit device address
	//! @param[in] aTxData - data for send
	//! @param[in] aTxSize - size of tx data
	//! @param[in] aRxBuf - buffer for received data
	//! @param[in] aRxSize - size of data to recive
	//! @return true on success, false on error
	bool exchange(const uint8_t aAddr, const void *aTxData, uint8_t aTxSize, void* aRxBuf, uint8_t aRxSize)
	{
		if (I2C_ISR(kPeriph) & I2C_ISR_BUSY) {
			return false;
		}

		txPtr = static_cast<const uint8_t *>(aTxData);
		rxPtr = static_cast<uint8_t *>(aRxBuf);
		txCount = aTxSize;
		rxCount = aRxSize;
		address = aAddr;

		start((txCount > 0)? TransferDirection::Write : TransferDirection::Read);
		return true;
	}

private:
	Callback callback;
	const uint8_t *txPtr;
	uint8_t *rxPtr;
	uint8_t address;
	uint8_t txCount;
	uint8_t rxCount;

	//! @brief Start transfer
	//! @param[in] direction - transfer direction
	void start(TransferDirection direction)
	{
		I2C_ICR(kPeriph) = kIntClrMask;
		if (direction == TransferDirection::Write) {
			i2c_set_bytes_to_transfer(kPeriph, txCount);
			i2c_set_write_transfer_dir(kPeriph);
			if (rxCount) {
				i2c_disable_autoend(kPeriph);
			} else {
				i2c_enable_autoend(kPeriph);
			}
		} else {
			i2c_set_bytes_to_transfer(kPeriph, rxCount);
			i2c_set_read_transfer_dir(kPeriph);
			i2c_enable_autoend(kPeriph);
		}
		i2c_set_7bit_address(kPeriph, address);
		i2c_send_start(kPeriph);
	}

	//! @brief Interrupt handler
	void handler() override
	{
		uint32_t status = I2C_ISR(kPeriph);
		auto err = static_cast<bool>(status & kErrMask);

		if (err) {
			I2C_ICR(kPeriph) = kIntClrMask;
		} else if (status & I2C_ISR_TXIS) {
			I2C_TXDR(kPeriph) = *txPtr++;
			txCount--;
		} else if (status & I2C_ISR_TC) {
			start(TransferDirection::Read);
		} else if (status & I2C_ISR_RXNE) {
			*rxPtr++ = static_cast<uint8_t>(I2C_RXDR(kPeriph));
			rxCount--;
		}

		if (status & I2C_ISR_STOPF) {
			i2c_clear_stop(kPeriph);
			if (callback) {
				callback(!err && (txCount == 0) && (rxCount == 0));
			}
		}
	}
};

#endif // PLATFORM_STM32_I2CV2IRQ_HPP_
