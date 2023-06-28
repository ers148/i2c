//
// QspiFlash.hpp
//
//  Created on: Nov 12, 2019
//      Author: N.Makarova
//

#ifndef PLATFORM_STM32F76X_QSPI_FLASH_HPP_
#define PLATFORM_STM32F76X_QSPI_FLASH_HPP_

#include <cstddef>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/quadspi.h>

namespace W25Defs {

static constexpr uint8_t BLOCK_64KB_ERASE = 0xD8;
static constexpr uint8_t QUAD_READ = 0x6B;
static constexpr uint8_t READ_STATUS_REG_1 = 0x05;
static constexpr uint8_t READ_STATUS_REG_2 = 0x35;
static constexpr uint8_t QUAD_PAGE_PROGRAM = 0x32;
static constexpr uint8_t WRITE_STATUS_REG = 0x01;
static constexpr uint8_t REGISTER_QUAD_ENABLE = 0x02;

static constexpr size_t kPageSize{0x100};
static constexpr size_t kSizeLimit{0x1000000};
static constexpr size_t kBlock64kbSize{0x010000};

} // namespace W25Defs

class QspiFlash {
	// Quadspi lightweight driver
public:
	static bool protect()
	{
		return true;
	}

	static bool isProtected()
	{
		return true;
	}

	static void lock()
	{
	}

	static void unlock()
	{
	}

	static bool init()
	{
		uint32_t sr;
		uint32_t ccr;

		// Clocking quadspi
		rcc_periph_clock_enable(RCC_QSPI);
		// Set control registers and turn on quadspi stm32
		setCrDcrRegister();
		writeEnable();

		// Turn on quadspi nor flash
		ccr = ((W25Defs::WRITE_STATUS_REG & QUADSPI_CCR_INST_MASK) << QUADSPI_CCR_INST_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_IMODE_MASK) << QUADSPI_CCR_IMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_FMODE_IWRITE & QUADSPI_CCR_FMODE_MASK) << QUADSPI_CCR_FMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_NONE & QUADSPI_CCR_ADMODE_MASK) << QUADSPI_CCR_ADMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_DMODE_MASK) << QUADSPI_CCR_DMODE_SHIFT);
		QUADSPI_CCR = ccr;
		QUADSPI_DR = 0x200;
		do {
			sr = QUADSPI_SR;
		} while (sr & QUADSPI_SR_BUSY);
		QUADSPI_FCR = 0x1F;

		// Check quadspi switched on
		return isQuadModeEnable();
	}

	static void deinit()
	{
		QUADSPI_CR = 0;
		rcc_periph_clock_disable(RCC_QSPI);
	}

	static bool read(uintptr_t aOffset, void *aBuffer, size_t aLength)
	{
		// Adress + buf length are inside nor flash limits
		if (aOffset + aLength > W25Defs::kSizeLimit) {
			return false;
		}
		if (aLength == 0) {
			return true;
		}
		if (readStatusReg1() == 0xFF) {
			return false;
		}

		uint32_t ccr;
		uint32_t sr;
		uint8_t flashReg;

		auto buffer = static_cast<uint8_t *>(aBuffer);
		// Data buf length
		QUADSPI_DLR = aLength - 1;
		// 1-1-4 reading; 8 dummy clock
		ccr = ((QUADSPI_CCR_FMODE_IREAD & QUADSPI_CCR_FMODE_MASK) << QUADSPI_CCR_FMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_4LINE & QUADSPI_CCR_DMODE_MASK) << QUADSPI_CCR_DMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_IMODE_MASK) << QUADSPI_CCR_IMODE_SHIFT);
		ccr |= ((2 & QUADSPI_CCR_ADSIZE_MASK) << QUADSPI_CCR_ADSIZE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_ADMODE_MASK) << QUADSPI_CCR_ADMODE_SHIFT);
		ccr |= ((W25Defs::QUAD_READ & QUADSPI_CCR_INST_MASK) << QUADSPI_CCR_INST_SHIFT);
		ccr |= ((8 & QUADSPI_CCR_DCYC_MASK) << QUADSPI_CCR_DCYC_SHIFT);
		QUADSPI_CCR = ccr;
		QUADSPI_AR = aOffset;

		// Reading process
		do {
			sr = QUADSPI_SR;
			if (sr & QUADSPI_SR_FTF) {
				*buffer = QUADSPI_BYTE_DR;
				++buffer;
			}
		} while (sr & QUADSPI_SR_BUSY);
		QUADSPI_FCR = 0x1F;

		// Waiting for completion
		do {
			flashReg = readStatusReg1();
			if (flashReg == 0xFF) {
				return false;
			}
		} while (flashReg & 0x01);

		return true;
	}

	static bool ready()
	{
		const auto reg = readStatusReg1();
		return (reg != 0xFF) && ((reg & 0x01) == 0);
	}

	// Erase block 64 Kb
	static bool erase(uintptr_t aOffset, size_t aLength)
	{
		// Adress + buf length are inside nor flash limits
		if (aLength < W25Defs::kBlock64kbSize || aOffset > W25Defs::kSizeLimit - W25Defs::kBlock64kbSize) {
			return false;
		}
		if (aOffset % W25Defs::kBlock64kbSize != 0 || aLength % W25Defs::kBlock64kbSize != 0) {
			return false;
		}
		if (aOffset + aLength > W25Defs::kSizeLimit) {
			return false;
		}

		uint32_t ccr;
		uint32_t sr;
		uint8_t flashReg;
		uintptr_t currentOffset = aOffset;

		do {
			writeEnable();
			// 1-1 write mode; no data
			ccr = ((QUADSPI_CCR_FMODE_IWRITE & QUADSPI_CCR_FMODE_MASK) << QUADSPI_CCR_FMODE_SHIFT);
			ccr |= ((2 & QUADSPI_CCR_ADSIZE_MASK) << QUADSPI_CCR_ADSIZE_SHIFT);
			ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_ADMODE_MASK) << QUADSPI_CCR_ADMODE_SHIFT);
			ccr |= ((W25Defs::BLOCK_64KB_ERASE & QUADSPI_CCR_INST_MASK) << QUADSPI_CCR_INST_SHIFT);
			ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_IMODE_MASK) << QUADSPI_CCR_IMODE_SHIFT);
			QUADSPI_CCR = ccr;
			QUADSPI_AR = currentOffset;
			do {
				sr = QUADSPI_SR;
			} while (sr & QUADSPI_SR_BUSY);
			QUADSPI_FCR = 0x1F;

			// Waiting for cleaning completion
			do {
				flashReg = readStatusReg1();
				if (flashReg == 0xFF) {
					return false;
				}
			} while (flashReg & 0x01);

			currentOffset += W25Defs::kBlock64kbSize;
			if (currentOffset >= aOffset + aLength) {
				return true;
			}
			// Until the end of NOR flash will be reached or set value
		} while (currentOffset < W25Defs::kSizeLimit);

		return true;
	}

	static bool write(uintptr_t aOffset, const void *aBuffer, size_t aLength)
	{
		// Adress + buf length are inside nor flash limits
		if (aOffset + aLength > W25Defs::kSizeLimit) {
			return false;
		}

		auto buffer = static_cast<const uint8_t *>(aBuffer);
		size_t bytesToWrite(aLength);
		uintptr_t currentOffset(aOffset);

		if (readStatusReg1() == 0xFF) {
			return false;
		}

		// Split buffer and write page by page
		while (bytesToWrite > 0) {
			// Address goes to the top of the page
			if (currentOffset % W25Defs::kPageSize == 0) {
				if (bytesToWrite <= W25Defs::kPageSize) {
					return writePage(currentOffset, buffer, bytesToWrite);
				} else {
					// But buffer length is larger than page
					writePage(currentOffset, buffer, W25Defs::kPageSize);
					buffer = buffer + W25Defs::kPageSize;
					bytesToWrite -= W25Defs::kPageSize;
					currentOffset += W25Defs::kPageSize;
				}
			} else { // currentOffset % FLASH_PAGE_SIZE != 0
				// Address does NOT go to the top of the page
				size_t bytesToNextPage = (currentOffset / W25Defs::kPageSize + 1) * W25Defs::kPageSize - currentOffset;
				// But inside it
				if (bytesToWrite < bytesToNextPage) {
					return writePage(currentOffset, buffer, bytesToWrite);
				} else {
					writePage(currentOffset, buffer, bytesToNextPage);
					buffer = buffer + bytesToNextPage;
					bytesToWrite -= bytesToNextPage;
					currentOffset += bytesToNextPage;
				}
			}
		}
		return true;
	}

	static void setInterface(QspiFlash *aInterface)
	{
		// Singleton pattern implementation
		*interfaceImpl() = aInterface;
	}

private:
	static QspiFlash **interfaceImpl()
	{
		// Singleton pattern implementation
		static QspiFlash *instance;
		return &instance;
	}

	static QspiFlash *interface()
	{
		// Singleton pattern implementation
		return *interfaceImpl();
	}

	static void writeEnable()
	{
		// NOR flash write enable
		uint32_t sr;
		QUADSPI_CCR = 0x106;
		do {
			sr = QUADSPI_SR;
		} while (sr & QUADSPI_SR_BUSY);
		QUADSPI_FCR = 0x1F;
	}

	static bool isQuadModeEnable()
	{
		// Reading NOR flash register status 2 and checking if quad bit is set
		uint32_t sr;
		uint32_t ccr;
		uint8_t reg2{0xFF};

		QUADSPI_DLR = 0x00;
		ccr = ((W25Defs::READ_STATUS_REG_2 & QUADSPI_CCR_INST_MASK) << QUADSPI_CCR_INST_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_IMODE_MASK) << QUADSPI_CCR_IMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_FMODE_IREAD & QUADSPI_CCR_FMODE_MASK) << QUADSPI_CCR_FMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_NONE & QUADSPI_CCR_ADMODE_MASK) << QUADSPI_CCR_ADMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_DMODE_MASK) << QUADSPI_CCR_DMODE_SHIFT);
		QUADSPI_CCR = ccr;

		do {
			sr = QUADSPI_SR;
			if (sr & QUADSPI_SR_FTF) {
				reg2 = QUADSPI_BYTE_DR;
			}
		} while (sr & QUADSPI_SR_BUSY);
		QUADSPI_FCR = 0x1F;

		if (reg2 == 0xFF) {
			return false;
		}

		return reg2 & W25Defs::REGISTER_QUAD_ENABLE ? true : false;
	}

	static uint8_t readStatusReg1()
	{
		uint8_t reg{0xFF};
		uint32_t ccr;
		uint32_t sr;

		QUADSPI_DLR = 0x00;
		ccr = ((W25Defs::READ_STATUS_REG_1 & QUADSPI_CCR_INST_MASK) << QUADSPI_CCR_INST_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_IMODE_MASK) << QUADSPI_CCR_IMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_FMODE_IREAD & QUADSPI_CCR_FMODE_MASK) << QUADSPI_CCR_FMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_NONE & QUADSPI_CCR_ADMODE_MASK) << QUADSPI_CCR_ADMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_DMODE_MASK) << QUADSPI_CCR_DMODE_SHIFT);
		QUADSPI_CCR = ccr;
		do {
			sr = QUADSPI_SR;
			if (sr & QUADSPI_SR_FTF) {
				reg = QUADSPI_BYTE_DR;
			}
		} while (sr & QUADSPI_SR_BUSY);
		QUADSPI_FCR = 0x1F;

		return reg;
	}

	static void setCrDcrRegister()
	{
		// STM32 control register: frequency prescaler, fifo threshold level, sample shift
		QUADSPI_CR = ((1 & QUADSPI_CR_PRESCALE_MASK) << QUADSPI_CR_PRESCALE_SHIFT);
		QUADSPI_CR |= QUADSPI_CR_FSEL;
		QUADSPI_CR |= QUADSPI_CR_SSHIFT;

		// Device configuration register: flash memory size, chip select high time,
		// High/low between comands (mode 3/0)
		QUADSPI_DCR = ((23 & QUADSPI_DCR_FSIZE_MASK) << QUADSPI_DCR_FSIZE_SHIFT);
		QUADSPI_DCR |= ((1 & QUADSPI_DCR_CSHT_MASK) << QUADSPI_DCR_CSHT_SHIFT);
		QUADSPI_DCR |= QUADSPI_DCR_CKMODE;

		// Switch on
		QUADSPI_CR |= QUADSPI_CR_EN;
	}

	static bool writePage(uintptr_t aOffset, const uint8_t *aBuffer, size_t aLength)
	{
		// Writing within page
		uint32_t ccr;
		uint32_t sr;
		uint8_t flashReg;

		writeEnable();

		QUADSPI_DLR = aLength - 1;
		ccr = ((W25Defs::QUAD_PAGE_PROGRAM & QUADSPI_CCR_INST_MASK) << QUADSPI_CCR_INST_SHIFT);
		ccr |= ((QUADSPI_CCR_FMODE_IWRITE & QUADSPI_CCR_FMODE_MASK) << QUADSPI_CCR_FMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_4LINE & QUADSPI_CCR_DMODE_MASK) << QUADSPI_CCR_DMODE_SHIFT);
		ccr |= ((2 & QUADSPI_CCR_ADSIZE_MASK) << QUADSPI_CCR_ADSIZE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_ADMODE_MASK) << QUADSPI_CCR_ADMODE_SHIFT);
		ccr |= ((QUADSPI_CCR_MODE_1LINE & QUADSPI_CCR_IMODE_MASK) << QUADSPI_CCR_IMODE_SHIFT);
		QUADSPI_CCR = ccr;
		QUADSPI_AR = aOffset;

		do {
			sr = QUADSPI_SR;
			if (sr & QUADSPI_SR_TCF) {
				break;
			}
			QUADSPI_BYTE_DR = *aBuffer;
			aBuffer++;
		} while (QUADSPI_SR & QUADSPI_SR_BUSY);
		QUADSPI_FCR = 0x1F;

		// Waiting for process completion
		do {
			flashReg = readStatusReg1();
			if (flashReg == 0xFF) {
				return false;
			}
		} while (flashReg & 0x01);

		return true;
	}
};

#endif // PLATFORM_STM32F76X_QSPI_FLASH_HPP_
