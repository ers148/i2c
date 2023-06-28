//
// Reloader.hpp
//
//  Created on: Mar 28, 2018
//      Author: andrey
//

#ifndef PLATFORM_STM32F4XX_RELOADER_HPP_
#define PLATFORM_STM32F4XX_RELOADER_HPP_

#include <Platform/AsmHelpers.hpp>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>

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
		rcc_periph_clock_enable(RCC_PWR);
		rcc_periph_reset_pulse(RST_PWR);
	}

	static void deinit()
	{
		rcc_periph_clock_disable(RCC_PWR);
	}

	static bool isBootRequested()
	{
		const auto word1 = RTC_BKPXR(1);
		const auto word2 = RTC_BKPXR(2);

		return word1 == kBootConstant1 && (word2 == kBootConstant2 || word2 == kFastBootConstant2);
	}

	static bool isColdStart()
	{
		return !(RCC_CSR & (RCC_CSR_WWDGRSTF | RCC_CSR_IWDGRSTF | RCC_CSR_SFTRSTF));
	}

	static bool isFastBootRequested()
	{
		return RTC_BKPXR(1) == kBootConstant1 && RTC_BKPXR(2) == kFastBootConstant2;
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
		return RTC_BKPXR(3);
	}

	static void setRestartCount(uint32_t aCount)
	{
		unlock();
		RTC_BKPXR(3) = aCount;
		lock();
	}

	static void setBootSignature()
	{
		RCC_CSR |= RCC_CSR_RMVF;
		setBootRegisters(kBootConstant1, kBootConstant2);
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
		unlock();
		RTC_BKPXR(1) = aData1;
		RTC_BKPXR(2) = aData2;
		lock();
	}

	static void lock()
	{
		pwr_enable_backup_domain_write_protect();
		rtc_lock();
	}

	static void unlock()
	{
		rtc_unlock();
		pwr_disable_backup_domain_write_protect();
	}
};

#endif // PLATFORM_STM32F4XX_RELOADER_HPP_
