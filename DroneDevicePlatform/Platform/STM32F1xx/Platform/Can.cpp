//
// Can.cpp
//
//  Created on: Dec 8, 2017
//      Author: Alexander
//

#include "Can.hpp"

template<> CanBase<1> *CanBase<1>::instance = nullptr;
template<> CanBase<2> *CanBase<2>::instance = nullptr;

void usb_hp_can_tx_isr()
{
	CanBase<1>::handleTxIrq();
}

void usb_lp_can_rx0_isr()
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
