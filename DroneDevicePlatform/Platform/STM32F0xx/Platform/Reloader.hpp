//
// Reloader.hpp
//
//  Created on: Nov 3, 2017
//      Author: Alexander
//

#ifndef PLATFORM_STM32F0XX_RELOADER_HPP_
#define PLATFORM_STM32F0XX_RELOADER_HPP_

#include <Platform/AsmHelpers.hpp>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/f0/syscfg.h>
#include <cstdint>
#include <cstddef>

template<uintptr_t TABLE_POSITION, size_t TABLE_SIZE>
class Reloader {
	static constexpr uint32_t kBootConstant1{0x1071};
	static constexpr uint32_t kBootConstant2{0x1701};
	static constexpr uint32_t kMaxRestartCount{4};

public:
	Reloader() = delete;
	Reloader(const Reloader &) = delete;
	Reloader &operator=(const Reloader &) = delete;

	static void init()
	{
		rcc_periph_clock_enable(RCC_RTC);
		rcc_periph_clock_enable(RCC_PWR);
		rcc_periph_reset_pulse(RST_PWR);
	}

	static void deinit()
	{
		rcc_periph_clock_disable(RCC_PWR);
		rcc_periph_clock_disable(RCC_RTC);
	}

	static bool isBootRequested()
	{
		return RTC_BKPXR(0) == kBootConstant1 && RTC_BKPXR(1) == kBootConstant2;
	}

	static bool isColdStart()
	{
		return !(RCC_CSR & (RCC_CSR_WWDGRSTF | RCC_CSR_IWDGRSTF | RCC_CSR_SFTRSTF));
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
		return RTC_BKPXR(2);
	}

	static void setRestartCount(uint32_t aCount)
	{
		pwr_disable_backup_domain_write_protect();
		RTC_BKPXR(2) = aCount;
		pwr_enable_backup_domain_write_protect();
	}

	static void clearBootSignature()
	{
		RCC_CSR |= RCC_CSR_RMVF;
		setBootRegisters(0, 0);
	}

	static void relocateAndStart(uint32_t aFirmwareAddress, uint32_t aStackAddress, uint32_t aVectorAddress)
	{
		uint32_t * const vectors = reinterpret_cast<uint32_t *>(TABLE_POSITION);

		/* Copy interrupt vectors from target firmware to the RAM section */
		memcpy(vectors, reinterpret_cast<const uint32_t *>(aFirmwareAddress), TABLE_SIZE);

		SYSCFG_CFGR1 = (SYSCFG_CFGR1 & ~SYSCFG_CFGR1_MEM_MODE) | SYSCFG_CFGR1_MEM_MODE_SRAM;
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
	static void setBootRegisters(uint32_t aData0, uint32_t aData1)
	{
		pwr_disable_backup_domain_write_protect();
		RTC_BKPXR(0) = aData0;
		RTC_BKPXR(1) = aData1;
		pwr_enable_backup_domain_write_protect();
	}
};

#endif // PLATFORM_STM32F0XX_RELOADER_HPP_
