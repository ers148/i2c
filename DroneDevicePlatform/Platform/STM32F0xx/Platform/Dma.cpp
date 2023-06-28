//
// Dma.cpp
//
//  Created on: Jun 2, 2021
//      Author: Alexander
//

#include "Dma.hpp"

template<> DmaBase<1, 1> *DmaBase<1, 1>::instance = nullptr;
template<> DmaBase<1, 2> *DmaBase<1, 2>::instance = nullptr;
template<> DmaBase<1, 3> *DmaBase<1, 3>::instance = nullptr;
template<> DmaBase<1, 4> *DmaBase<1, 4>::instance = nullptr;
template<> DmaBase<1, 5> *DmaBase<1, 5>::instance = nullptr;
template<> DmaBase<1, 6> *DmaBase<1, 6>::instance = nullptr;
template<> DmaBase<1, 7> *DmaBase<1, 7>::instance = nullptr;

void dma1_channel1_isr()
{
	const uint32_t flags = DMA1_ISR & DMA_ISR_MASK(1);
	DMA1_IFCR = flags;

	DmaBase<1, 1>::handleIrq(flags >> DMA_FLAG_OFFSET(1));
}

void dma1_channel2_3_dma2_channel1_2_isr()
{
	const uint32_t flags = DMA1_ISR & (DMA_ISR_MASK(2) | DMA_ISR_MASK(3));
	DMA1_IFCR = flags;

	if ((flags & DMA_ISR_MASK(2)) != 0) {
		DmaBase<1, 2>::handleIrq((flags >> DMA_FLAG_OFFSET(2)) & DMA_FLAGS);
	}

	if ((flags & DMA_ISR_MASK(3)) != 0) {
		DmaBase<1, 3>::handleIrq((flags >> DMA_FLAG_OFFSET(3)) & DMA_FLAGS);
	}
}

void dma1_channel4_7_dma2_channel3_5_isr()
{
	const uint32_t flags = DMA1_ISR & (DMA_ISR_MASK(4) | DMA_ISR_MASK(5) | DMA_ISR_MASK(6) | DMA_ISR_MASK(7));
	DMA1_IFCR = flags;

	if ((flags & DMA_ISR_MASK(4)) != 0) {
		DmaBase<1, 4>::handleIrq((flags >> DMA_FLAG_OFFSET(4)) & DMA_FLAGS);
	}

	if ((flags & DMA_ISR_MASK(5)) != 0) {
		DmaBase<1, 5>::handleIrq((flags >> DMA_FLAG_OFFSET(5)) & DMA_FLAGS);
	}

	if ((flags & DMA_ISR_MASK(6)) != 0) {
		DmaBase<1, 6>::handleIrq((flags >> DMA_FLAG_OFFSET(6)) & DMA_FLAGS);
	}

	if ((flags & DMA_ISR_MASK(7)) != 0) {
		DmaBase<1, 7>::handleIrq((flags >> DMA_FLAG_OFFSET(7)) & DMA_FLAGS);
	}
}
