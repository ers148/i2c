//! @file I2cV1.hpp
//! @author Aleksei Drovenkov
//! @date Nov 31, 2022

#ifndef PLATFORM_STM32_I2CV1_HPP_
#define PLATFORM_STM32_I2CV1_HPP_

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
	using Callback = std::function<void (bool)>;

	static constexpr auto kEvIrq{BaseType::numberToEvIrq()};
	static constexpr auto kErrIrq{BaseType::numberToErrIrq()};
	static constexpr auto kPeriph{BaseType::numberToPeriph()};
	static constexpr auto kClockBranch{BaseType::numberToClockBranch()};
	static constexpr auto kResetSignal{BaseType::numberToResetSignal()};

	static constexpr auto kAttemptLimit = 100'000;

public:
	I2c(const I2c &) = delete;
	I2c &operator=(const I2c &) = delete;

	//! @brief Constructor
	//! @param[in] aRate - bus rate
	//! @param[in] aCallback = callback
	I2c(uint32_t aRate, Callback aCallback = nullptr) :
		rate{aRate},
		callback{aCallback}
	{
		rcc_periph_clock_enable(kClockBranch);
		i2c_set_clock_frequency(kPeriph, static_cast<uint8_t>(clock() / 1000000));
		i2c_set_ccr(kPeriph, static_cast<uint16_t>(clock() / (2 * rate)));
		i2c_set_trise(kPeriph, static_cast<uint16_t>(clock() / 1000000 + 1));
		i2c_peripheral_enable(kPeriph);
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
	bool send(const uint8_t aAddr, const void *aTxData, size_t aTxSize)
	{
		return exchange(aAddr, aTxData, aTxSize, nullptr, 0);
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
		// Transmit data
		bool result = start(false);

		if (result) {
			result = sendAddress(static_cast<uint8_t>(aAddr << 1), false);
		}

		for (auto data = reinterpret_cast<const uint8_t *>(aTxData); result && (aTxSize > 0); --aTxSize) {
			result = sendByte(*data++);
		}

		// Receive data
		if (result && (aRxSize > 0)) {
			result = start(aRxSize == 2);

			if (result) {
				result = sendAddress(static_cast<uint8_t>((aAddr << 1) | 1), aRxSize == 1);
			}

			if (result) {
				auto buffer = reinterpret_cast<uint8_t *>(aRxBuf);

				if (aRxSize == 1) {
					i2c_send_stop(kPeriph);
					result = receiveByte(buffer);
				} else if (aRxSize == 2) {
					result = waitForFlag(I2C_SR1_RxNE);
					i2c_disable_ack(kPeriph);
					i2c_send_stop(kPeriph);
					while (result && (aRxSize > 0)) {
						result = receiveByte(buffer++);
						--aRxSize;
					}
				} else {
					while (result && (aRxSize > 3)) {
						result = receiveByte(buffer++);
						--aRxSize;
					}

					if (result) {
						result = waitForFlag(I2C_SR1_BTF);
					}

					if (result) {
						i2c_disable_ack(kPeriph);
						result = receiveByte(buffer++);
						--aRxSize;
					}

					i2c_send_stop(kPeriph);

					while (result && (aRxSize > 0)) {
						result = receiveByte(buffer++);
						--aRxSize;
					}
				}
			} else {
				i2c_send_stop(kPeriph);
			}
		} else {
			if (result) {
				for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
					if (I2C_SR1(kPeriph) & (I2C_SR1_TxE | I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR)) {
						break;
					}
				}
			}
			i2c_send_stop(kPeriph);
		}

		// Wait for bus ready
		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			if ((I2C_SR2(kPeriph) & I2C_SR2_BUSY) == 0) {
				break;
			}
		}

		return result;
	}

private:
	uint32_t rate;
	Callback callback;

	//! @brief Send START condition
	//! @param[in] aSetPos - flag for POS bit set
	//! @return true on success, false on error
	bool start(bool aSetPos)
	{
		I2C_SR1(kPeriph) = 0;

		auto regValue = I2C_CR1(kPeriph) | I2C_CR1_ACK;
		I2C_CR1(kPeriph) = (aSetPos)? (regValue | I2C_CR1_POS) : (regValue & ~I2C_CR1_POS);

		i2c_send_start(kPeriph);

		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			auto status = I2C_SR1(kPeriph);

			if (status & I2C_SR1_SB) {
				return true;
			}

			if (status & (I2C_SR1_ARLO | I2C_SR1_BERR)) {
				break;
			}
		}

		return false;
	}

	//! @brief Send address
	//! @param[in] aResetAck - flag for ACK bit reset
	//! @return true on success, false on error
	bool sendAddress(uint8_t aAddress, bool aResetAck)
	{
		I2C_DR(kPeriph) = aAddress;

		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			auto status = I2C_SR1(kPeriph);

			if (status & I2C_SR1_ADDR) {
				if (aResetAck) {
					i2c_disable_ack(kPeriph);
				}

				return I2C_SR2(kPeriph) & I2C_SR2_MSL;  // We just need to read SR2 in order to reset ADDR bit
			}

			if (status & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR)) {
				break;
			}
		}

		return false;
	}

	//! @brief Send data byte
	//! @param[in] aByte - byte for send
	//! @return true on success, false on error
	bool sendByte(uint8_t aByte)
	{
		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			auto status = I2C_SR1(kPeriph);

			if (status & I2C_SR1_TxE) {
				I2C_DR(kPeriph) = aByte;
				return true;
			}

			if (status & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR)) {
				break;
			}
		}

		return false;
	}

	//! @brief Receive data byte
	//! @param[in] aBuffer - buffer for received byte
	//! @return true on success, false on error
	bool receiveByte(uint8_t *aBuffer)
	{
		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			auto status = I2C_SR1(kPeriph);

			if (status & I2C_SR1_RxNE) {
				*aBuffer = static_cast<uint8_t>(I2C_DR(kPeriph));
				return true;
			}

			if (status & (I2C_SR1_ARLO | I2C_SR1_BERR)) {
				break;
			}
		}

		return false;
	}

	//! @brief Wait for certain flag
	//! @param[in] aFlag - expected flag
	//! @return true if the flag is set
	bool waitForFlag(uint32_t aFlag)
	{
		for (auto attempt = 0; attempt < kAttemptLimit; ++attempt) {
			auto status = I2C_SR1(kPeriph);

			if (status & aFlag) {
				return true;
			}

			if (status & (I2C_SR1_ARLO | I2C_SR1_BERR)) {
				break;
			}
		}

		return false;
	}

	//! @brief Interrupt handler
	void handler() override
	{
	}

	static constexpr uint32_t clock()
	{
		return rcc_apb1_frequency;
	}
};

#endif // PLATFORM_STM32_I2CV1_HPP_
