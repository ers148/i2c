//
// Dma.cpp
//
//  Created on: Jun 4, 2021
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

template<> DmaBase<2, 1> *DmaBase<2, 1>::instance = nullptr;
template<> DmaBase<2, 2> *DmaBase<2, 2>::instance = nullptr;
template<> DmaBase<2, 3> *DmaBase<2, 3>::instance = nullptr;
template<> DmaBase<2, 4> *DmaBase<2, 4>::instance = nullptr;
template<> DmaBase<2, 5> *DmaBase<2, 5>::instance = nullptr;

template<unsigned int number, unsigned int channel>
static void genericDmaHandler()
{
	const uint32_t flags = DMA_ISR(number == 1 ? DMA1 : DMA2) & DMA_ISR_MASK(channel);
	DMA_IFCR(number == 1 ? DMA1 : DMA2) = flags;

	DmaBase<number, channel>::handleIrq(flags >> DMA_FLAG_OFFSET(channel));
}

void dma1_channel1_isr()
{
	genericDmaHandler<1, 1>();
}

void dma1_channel2_isr()
{
	genericDmaHandler<1, 2>();
}

void dma1_channel3_isr()
{
	genericDmaHandler<1, 3>();
}

void dma1_channel4_isr()
{
	genericDmaHandler<1, 4>();
}

void dma1_channel5_isr()
{
	genericDmaHandler<1, 5>();
}

void dma1_channel6_isr()
{
	genericDmaHandler<1, 6>();
}

void dma1_channel7_isr()
{
	genericDmaHandler<1, 6>();
}

void dma2_channel1_isr()
{
	genericDmaHandler<2, 1>();
}

void dma2_channel2_isr()
{
	genericDmaHandler<2, 2>();
}

void dma2_channel3_isr()
{
	genericDmaHandler<2, 3>();
}

void dma2_channel4_5_isr()
{
	const uint32_t flags = DMA2_ISR;
	const uint32_t flags4 = flags & DMA_ISR_MASK(4);
	const uint32_t flags5 = flags & DMA_ISR_MASK(4);

	if (flags4 != 0) {
		DMA2_IFCR = flags4;
		DmaBase<2, 4>::handleIrq(flags4 >> DMA_FLAG_OFFSET(4));
	}

	if (flags5 != 0) {
		DMA2_IFCR = flags5;
		DmaBase<2, 5>::handleIrq(flags5 >> DMA_FLAG_OFFSET(5));
	}
}

void dma2_channel5_isr()
{
	genericDmaHandler<2, 5>();
}
