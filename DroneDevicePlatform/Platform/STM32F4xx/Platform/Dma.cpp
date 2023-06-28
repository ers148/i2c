//
// Dma.cpp
//
//  Created on: Jun 4, 2021
//      Author: Alexander
//

#include "Dma.hpp"

template<> DmaBase<1, 0> *DmaBase<1, 0>::instance = nullptr;
template<> DmaBase<1, 1> *DmaBase<1, 1>::instance = nullptr;
template<> DmaBase<1, 2> *DmaBase<1, 2>::instance = nullptr;
template<> DmaBase<1, 3> *DmaBase<1, 3>::instance = nullptr;
template<> DmaBase<1, 4> *DmaBase<1, 4>::instance = nullptr;
template<> DmaBase<1, 5> *DmaBase<1, 5>::instance = nullptr;
template<> DmaBase<1, 6> *DmaBase<1, 6>::instance = nullptr;
template<> DmaBase<1, 7> *DmaBase<1, 7>::instance = nullptr;

template<> DmaBase<2, 0> *DmaBase<2, 0>::instance = nullptr;
template<> DmaBase<2, 1> *DmaBase<2, 1>::instance = nullptr;
template<> DmaBase<2, 2> *DmaBase<2, 2>::instance = nullptr;
template<> DmaBase<2, 3> *DmaBase<2, 3>::instance = nullptr;
template<> DmaBase<2, 4> *DmaBase<2, 4>::instance = nullptr;
template<> DmaBase<2, 5> *DmaBase<2, 5>::instance = nullptr;
template<> DmaBase<2, 6> *DmaBase<2, 6>::instance = nullptr;
template<> DmaBase<2, 7> *DmaBase<2, 7>::instance = nullptr;

template<unsigned int number, unsigned int channel>
static void genericDmaHandler()
{
	if (channel < 4) {
		const uint32_t status = DMA_LISR(number == 1 ? DMA1 : DMA2);
		DMA_LIFCR(number == 1 ? DMA1 : DMA2) = status;

		DmaBase<number, channel>::handleIrq(status >> (channel * 8));
	} else {
		const uint32_t status = DMA_HISR(number == 1 ? DMA1 : DMA2);
		DMA_HIFCR(number == 1 ? DMA1 : DMA2) = status;

		DmaBase<number, channel>::handleIrq(status >> ((channel - 4) * 8));
	}
}

void dma1_stream0_isr()
{
	genericDmaHandler<1, 0>();
}

void dma1_stream1_isr()
{
	genericDmaHandler<1, 1>();
}

void dma1_stream2_isr()
{
	genericDmaHandler<1, 2>();
}

void dma1_stream3_isr()
{
	genericDmaHandler<1, 3>();
}

void dma1_stream4_isr()
{
	genericDmaHandler<1, 4>();
}

void dma1_stream5_isr()
{
	genericDmaHandler<1, 5>();
}

void dma1_stream6_isr()
{
	genericDmaHandler<1, 6>();
}

void dma1_stream7_isr()
{
	genericDmaHandler<1, 7>();
}

void dma2_stream0_isr()
{
	genericDmaHandler<2, 0>();
}

void dma2_stream1_isr()
{
	genericDmaHandler<2, 1>();
}

void dma2_stream2_isr()
{
	genericDmaHandler<2, 2>();
}

void dma2_stream3_isr()
{
	genericDmaHandler<2, 3>();
}

void dma2_stream4_isr()
{
	genericDmaHandler<2, 4>();
}

void dma2_stream5_isr()
{
	genericDmaHandler<2, 5>();
}

void dma2_stream6_isr()
{
	genericDmaHandler<2, 6>();
}

void dma2_stream7_isr()
{
	genericDmaHandler<2, 7>();
}
