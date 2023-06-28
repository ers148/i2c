//
// Reloader.hpp
//
//  Created on: Nov 3, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F1XX_RELOADER_HPP_
#define PLATFORM_STM32F1XX_RELOADER_HPP_

#include <Platform/AsmHelpers.hpp>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/f1/bkp.h>
#include <libopencm3/stm32/rcc.h>

class Reloader {
	static constexpr uint32_t kBootConstant1{0x1071};
	static constexpr uint32_t kBootConstant2{0x1701};
	static constexpr uint32_t kFastBootConstant2{0x1771};
	static constexpr uint32_t kMaxRestartCount{4};

public:
	Reloader() = delete;
	Reloader(const Reloader &) = delete;
	Reloader &operator=(const Reloader &) = delete;

	static void init()
	{
		rcc_periph_clock_enable(RCC_BKP);
		rcc_periph_clock_enable(RCC_PWR);
		rcc_periph_reset_pulse(RST_PWR);
	}

	static void deinit()
	{
		rcc_periph_clock_disable(RCC_PWR);
		rcc_periph_clock_disable(RCC_BKP);
	}

	static bool isBootRequested()
	{
		const auto word1 = BKP_DR1;
		const auto word2 = BKP_DR2;

		return word1 == kBootConstant1 && (word2 == kBootConstant2 || word2 == kFastBootConstant2);
	}

	static bool isColdStart()
	{
		return !(RCC_CSR & (RCC_CSR_WWDGRSTF | RCC_CSR_IWDGRSTF | RCC_CSR_SFTRSTF));
	}

	static bool isFastBootRequested()
	{
		return BKP_DR1 == kBootConstant1 && BKP_DR2 == kFastBootConstant2;
	}

	static bool isFwStartFailed()
	{
		return getRestartCount() >= kMaxRestartCount && !isColdStart();
	}

	static bool isWdtResetOccurred()
	{
		return (RCC_CSR & (RCC_CSR_WWDGRSTF | RCC_CSR_IWDGRSTF)) != 0;
	}

	static uint32_t getRestartCount()
	{
		return BKP_DR3;
	}

	static void setRestartCount(uint32_t aCount)
	{
		pwr_disable_backup_domain_write_protect();
		BKP_DR3 = aCount;
		pwr_enable_backup_domain_write_protect();
	}

	static void clearBootSignature()
	{
		RCC_CSR |= RCC_CSR_RMVF;
		setBootRegisters(0, 0);
	}

	static void relocateAndStart(uint32_t aFirmwareAddress, uint32_t aStackAddress, uint32_t aVectorAddress)
	{
		SCB_VTOR = aFirmwareAddress;
		__setMainStackPointerAndStart(aStackAddress, aVectorAddress);
	}

	static void reset()
	{
		clearBootSignature();
		scb_reset_system();
	}

	static void resetToBootloader()
	{
		setBootRegisters(kBootConstant1, kBootConstant2);
		scb_reset_system();
	}

private:
	static void setBootRegisters(uint32_t aData1, uint32_t aData2)
	{
		pwr_disable_backup_domain_write_protect();
		BKP_DR1 = aData1;
		BKP_DR2 = aData2;
		pwr_enable_backup_domain_write_protect();
	}
};

#endif // PLATFORM_STM32F1XX_RELOADER_HPP_
