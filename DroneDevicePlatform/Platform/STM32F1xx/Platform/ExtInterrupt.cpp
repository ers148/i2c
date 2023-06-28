//
// ExtInterrupt.cpp
//
//  Created on: Mar 22, 2022
//      Author: Alexander
//

#include "ExtInterrupt.hpp"
#include "Platform/AsmHelpers.hpp"

ExtInterruptHandler *ExtInterruptHandler::instances[] = {};

void exti0_isr()
{
	exti_reset_request(EXTI0);
	ExtInterruptHandler::handleIrq(0);
}

void exti1_isr()
{
	exti_reset_request(EXTI1);
	ExtInterruptHandler::handleIrq(1);
}

void exti2_isr()
{
	exti_reset_request(EXTI2);
	ExtInterruptHandler::handleIrq(2);
}

void exti3_isr()
{
	exti_reset_request(EXTI3);
	ExtInterruptHandler::handleIrq(3);
}

void exti4_isr()
{
	exti_reset_request(EXTI4);
	ExtInterruptHandler::handleIrq(4);
}

void exti9_5_isr()
{
	static constexpr auto kMask = ((EXTI9 << 1) - 1) & ~(EXTI5 - 1);

	auto status = reverseBits32(exti_get_flag_status(kMask));
	exti_reset_request(kMask);

	while (status) {
		const auto index = countLeadingZeros32(status);

		ExtInterruptHandler::handleIrq(index);
		status -= 0x80000000UL >> index;
	}
}

void exti15_10_isr()
{
	static constexpr auto kMask = ((EXTI15 << 1) - 1) & ~(EXTI10 - 1);

	auto status = reverseBits32(exti_get_flag_status(kMask));
	exti_reset_request(kMask);

	while (status) {
		const auto index = countLeadingZeros32(status);

		ExtInterruptHandler::handleIrq(index);
		status -= 0x80000000UL >> index;
	}
}
