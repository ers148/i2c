//
// AsmHelpers.hpp
//
//  Created on: Nov 9, 2017
//      Author: Alexander
//

#ifndef PLATFORM_CORTEX_M_ASMHELPERS_HPP_
#define PLATFORM_CORTEX_M_ASMHELPERS_HPP_

#include <cstdint>

#define barrier() __asm__ volatile ("" : : : "memory")
#define sleep() __asm__ volatile ("wfi")

void __setMainStackPointerAndStart(uint32_t, uint32_t) __attribute__((naked));

static inline uint32_t countTrailingZeros32(uint32_t value)
{
	return __builtin_ctz(value);
}

static inline uint32_t countLeadingZeros32(uint32_t value)
{
	return __builtin_clz(value);
}

static inline uint32_t reverseBits32(uint32_t value)
{
	uint32_t result;

	__asm__ volatile (
		"RBIT %[result], %[value]"
		: [result] "=r" (result)
		: [value] "r" (value)
	);
	return result;
}

#endif // PLATFORM_CORTEX_M_ASMHELPERS_HPP_
