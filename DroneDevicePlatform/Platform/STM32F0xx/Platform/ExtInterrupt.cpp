//
// ExtInterrupt.cpp
//
//  Created on: Oct 12, 2020
//      Author: y.lebedov
//

#include "ExtInterrupt.hpp"
#include "Platform/AsmHelpers.hpp"

ExtInterruptHandler *ExtInterruptHandler::instances[] = {};

void exti0_1_isr()
{
	const auto status = exti_get_flag_status(EXTI0 | EXTI1);
	exti_reset_request(status);

	if (status & EXTI0) {
		ExtInterruptHandler::handleIrq(0);
	}
	if (status & EXTI1) {
		ExtInterruptHandler::handleIrq(1);
	}
}

void exti2_3_isr()
{
	const auto status = exti_get_flag_status(EXTI2 | EXTI3);
	exti_reset_request(status);

	if (status & EXTI2) {
		ExtInterruptHandler::handleIrq(2);
	}
	if (status & EXTI3) {
		ExtInterruptHandler::handleIrq(3);
	}
}

void exti4_15_isr()
{
	static constexpr auto kMask = ((EXTI15 << 1) - 1) & ~(EXTI4 - 1);

	auto status = exti_get_flag_status(kMask);
	exti_reset_request(kMask);

	while (status) {
		const auto index = countTrailingZeros32(status);

		ExtInterruptHandler::handleIrq(index);
		status -= 1UL << index;
	}
}
