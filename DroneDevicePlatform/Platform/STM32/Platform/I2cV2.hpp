//! @file I2cV2.hpp
//! @author Aleksei Drovenkov
//! @date Jun 05, 2023

#ifndef PLATFORM_STM32_I2CV2_HPP_
#define PLATFORM_STM32_I2CV2_HPP_

#include <Platform/I2cV2Helpers.hpp>

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include <functional>

template<unsigned int>
class I2cBase;

template<unsigned int number>
class I2c : public I2cBase<number> {
	using BaseType = I2cBase<number>;
	using TransferDirection = typename BaseType::TransferDirection;

	static constexpr auto kIrq{BaseType::numberToIrq()};
	static constexpr auto kPeriph{BaseType::numberToPeriph()};
	static constexpr auto kClockBranch{BaseType::numberToClockBranch()};
	static constexpr auto kResetSignal{BaseType::numberToResetSignal()};

	static constexpr auto kAttemptLimit = 100'000;
	static constexpr auto kIntClrMask = I2C_ICR_ARLOCF | I2C_ICR_BERRCF | I2C_ICR_STOPCF | I2C_ICR_NACKCF;

public:
	I2c(const I2c&) = delete;
	I2c& operator=(const I2c&) = delete;

	//! @brief Constructor
	//! @param[in] aTimingRegValue - timing register value
	I2c(TimingRegValue aTimingRegValue)
	{
		rcc_periph_clock_enable(kClockBranch);
		i2c_enable_analog_filter(kPeriph);
		I2C_TIMINGR(kPeriph) = aTimingRegValue;
		i2c_enable_stretching(kPeriph);
		i2c_peripheral_enable(kPeriph);
	}

	//! @brief Destructor
	~I2c()
	{
		i2c_peripheral_disable(kPeriph);
		rcc_periph_clock_disable(kClockBranch);
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
	bool send(const uint8_t aAddr, const void *aTxData, size_t aTxSize)
	{
		return exchange(aAddr, aTxData, aTxSize, nullptr, 0);
	}

	//! @brief Receive data
	//! @param[in] aAddr - 7-bit device address
	//! @param[in] aRxBuf - buffer for received data
	//! @param[in] aRxSize - size of data to recive
	//! @return true on success, false on error
	bool receive(const uint8_t aAddr, void *aRxBuf, size_t aRxSize)
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
	bool exchange(const uint8_t aAddr, const void *aTxData, size_t aTxSize, void* aRxBuf, size_t aRxSize)
	{
		bool result = true;

		// Transmit data
		if (aTxSize > 0) {
			result = start(aAddr, aTxSize, TransferDirection::Write, aRxSize == 0);

			for (auto ptr = reinterpret_cast<const uint8_t* >(aTxData); result && (aTxSize > 0); --aTxSize) {
				result = sendByte(*ptr++);
			}

			if (result && (aRxSize > 0)) {
				result = waitFlag(I2C_ISR_TC);
			}
		}

		// Receive data
		if (result && (aRxSize > 0)) {
			result = start(aAddr, aRxSize, TransferDirection::Read, true);

			for (auto ptr = reinterpret_cast<uint8_t* >(aRxBuf); result && (aRxSize > 0); --aRxSize) {
				result = readByte(ptr);
				++ptr;
			}
		}

		// Wait for bus ready
		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			if ((I2C_ISR(kPeriph) & I2C_ISR_BUSY) == 0) {
				break;
			}
		}

		return result;
	}

private:
	//! @brief Start transfer
	//! @param[in] aAddress - 7-bit device address
	//! @param[in] aSize - size of data
	//! @param[in] aDirection - transfer direction
	//! @param[in] aAutoEnd - flag for automatic stop generation
	//! @return true on success, false on error
	bool start(uint8_t aAddress, size_t aSize, TransferDirection aDirection, bool aAutoEnd)
	{
		I2C_ICR(kPeriph) = kIntClrMask;
		i2c_set_7bit_address(kPeriph, aAddress);
		i2c_set_bytes_to_transfer(kPeriph, aSize);

		if (aAutoEnd) {
			i2c_enable_autoend(kPeriph);
		} else {
			i2c_disable_autoend(kPeriph);
		}

		if (aDirection == TransferDirection::Write) {
			i2c_set_write_transfer_dir(kPeriph);
		} else {
			i2c_set_read_transfer_dir(kPeriph);
		}

		i2c_send_start(kPeriph);

		auto flag = (aDirection == TransferDirection::Write) ? I2C_ISR_TXIS : I2C_ISR_RXNE;
		return waitFlag(flag);
	}

	//! @brief Send byte
	//! @param[in] aByte - byte for send
	//! @return true on success, false on error
	bool sendByte(uint8_t aByte)
	{
		if (waitFlag(I2C_ISR_TXIS)) {
			I2C_TXDR(kPeriph) = aByte;
			return true;
		}
		return false;
	}

	//! @brief Read byte
	//! @param[in] aBuffer - buffer for byte
	//! @return true on success, false on error
	bool readByte(uint8_t *aBuffer)
	{
		if (waitFlag(I2C_ISR_RXNE)) {
			*aBuffer = static_cast<uint8_t>(I2C_RXDR(kPeriph));
			return true;
		}
		return false;
	}

	//! @brief Wait for required flag
	//! @param[in] aFlag - required flag
	//! @return true on success, false on error
	bool waitFlag(uint32_t aFlag)
	{
		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			auto isr = I2C_ISR(kPeriph);

			if (isr & aFlag) {
				return true;
			}

			if (isr & (I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_NACKF)) {
				break;
			}
		}
		return false;
	}
};

#endif // PLATFORM_STM32_PLATFORM_I2CV2_HPP_
