//! @file I2cV2Helpers.hpp
//! @author Kershis Sergey
//! @date March 28, 2018

#ifndef PLATFORM_STM32_I2CV2_HELPERS_HPP_
#define PLATFORM_STM32_I2CV2_HELPERS_HPP_

#include <cstdint>
#include <cstring>

enum class DataTimeSetup : uint16_t {
	TIME_000_NS = 0,
	TIME_250_NS = 250,
	TIME_500_NS = 500,
	TIME_750_NS = 750,
	TIME_1000_NS = 1000,
	TIME_1250_NS = 1250,
	TIME_1500_NS = 1500,
	TIME_1750_NS = 1750,
	TIME_2000_NS = 2000,
	TIME_2250_NS = 2250,
	TIME_2500_NS = 2500,
	TIME_2750_NS = 2750,
	TIME_3000_NS = 3000,
	TIME_3250_NS = 3250,
	TIME_3500_NS = 3500,
	TIME_3750_NS = 3750,
};

enum class I2CRate : uint16_t {
	RATE_10kHz = 10,
	RATE_25kHz = 25,
	RATE_50kHz = 50,
	RATE_100kHz = 100,
	RATE_400kHz = 400
};

using TimingRegValue = uint32_t;

//! @brief I2C timing register value calculation
//! @param[in] aApbClock - APB clock, Hz
//! @param[in] aRate - I2C bus rate
//! @param[in] aDataSetupTime - data setup time
//! @param[in] aDataHoldTime - data hold time
//! @return timing register value
constexpr TimingRegValue calcTimingRegValue(uint32_t aApbClock, I2CRate aRate, DataTimeSetup aDataSetupTime, DataTimeSetup aDataHoldTime)
{
	uint32_t apbFreq{aApbClock / 1'000};
	uint32_t prescaler{1};
	uint32_t dataSetupTime{0};
	uint32_t dataHoldTime{0};
	uint32_t halfPeriod{0};

	// Define prescalor factor
	// Since registers SCLH and SCLL defining i2c speed are 8 bit long
	// we define prescalor such as calculated value of halfPeriod(SCLH and SCLL) fits there
	// calculated value of data setup time must also fit SCLDEL[3:0]
	// calculated value of data hold time must also fit SDADEL[3:0]
	do {
		uint32_t i2cfreq = apbFreq / prescaler;
		halfPeriod = i2cfreq / static_cast<uint32_t>(2 * static_cast<uint16_t>(aRate)) - 1;

		// Define data setup time
		dataSetupTime = (static_cast<uint32_t>(aDataSetupTime) * i2cfreq) / 1000000UL - 1;

		// Define data hold time
		dataHoldTime = (static_cast<uint32_t>(aDataHoldTime) * i2cfreq) / 1000000UL - 1;

		prescaler = prescaler * 2;
	} while ((halfPeriod > 0x00FF || dataSetupTime > 0x0F || dataHoldTime > 0x0F) && prescaler < 32U);

	prescaler /= 2;

	// Correct calculated values
	if (prescaler == 16) {
		// If we finished while() because prescaler exceeds maximum value PRESC[3:0] = 0x0F

		// Check dataHoldTime if it is greater than the maximum
		if (dataHoldTime > 0x0F) {
			// Set to the maximum value
			dataHoldTime = 0x0F;
		}

		// Check dataHoldTime if it is greater than the maximum
		if (dataSetupTime > 0x0F) {
			// Set to the maximum value
			dataSetupTime = 0x0F;
		}
	}

	prescaler -= 1;
	return (prescaler << 28) + (dataSetupTime << 20) + (dataHoldTime << 16) + (halfPeriod << 8) + halfPeriod;
}

#endif // PLATFORM_STM32_I2CV2_HELPERS_HPP_
