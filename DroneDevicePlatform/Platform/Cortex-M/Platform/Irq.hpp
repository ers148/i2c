//
// Irq.hpp
//
//  Created on: Dec 14, 2021
//      Author: Alexander
//

#ifndef PLATFORM_CORTEX_M_IRQ_HPP_
#define PLATFORM_CORTEX_M_IRQ_HPP_

#include <libopencm3/cm3/cortex.h>

using IrqState = uint32_t;

static inline void irqDisable()
{
	cm_disable_interrupts();
}

static inline void irqEnable()
{
	cm_enable_interrupts();
}

static inline IrqState irqSave()
{
	return cm_mask_interrupts(1);
}

static inline void irqRestore(IrqState aState)
{
	cm_mask_interrupts(aState);
}

#endif // PLATFORM_CORTEX_M_IRQ_HPP_
