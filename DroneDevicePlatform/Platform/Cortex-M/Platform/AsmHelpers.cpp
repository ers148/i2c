//
// AsmHelpers.cpp
//
//  Created on: Nov 9, 2017
//      Author: Alexander
//

#include "AsmHelpers.hpp"

void __setMainStackPointerAndStart(uint32_t stack, uint32_t function)
{
	__asm__ volatile (
			"MSR MSP, %[stack]\n"
			"BX %[function]\n"
			:
			: [stack] "r" (stack), [function] "r" (function)
	);
}
