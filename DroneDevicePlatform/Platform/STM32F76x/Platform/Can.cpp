//
// Can.cpp
//
//  Created on: Jan 10, 2019
//      Author: Alexander
//

#include "Can.hpp"

template<> CanBase<1> *CanBase<1>::instance = nullptr;
template<> CanBase<2> *CanBase<2>::instance = nullptr;

void can1_tx_isr()
{
	CanBase<1>::handleTxIrq();
}

void can1_rx0_isr()
{
	CanBase<1>::handleRxIrq();
}

void can2_tx_isr()
{
	CanBase<2>::handleTxIrq();
}

void can2_rx0_isr()
{
	CanBase<2>::handleRxIrq();
}
