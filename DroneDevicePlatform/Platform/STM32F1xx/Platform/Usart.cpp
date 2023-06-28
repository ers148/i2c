//
// Usart.cpp
//
//  Created on: Oct 24, 2017
//      Author: Alexander
//

#include "Usart.hpp"

template<> UsartBase<1> *UsartBase<1>::instance = nullptr;
template<> UsartBase<2> *UsartBase<2>::instance = nullptr;
template<> UsartBase<3> *UsartBase<3>::instance = nullptr;

void usart1_isr()
{
	UsartBase<1>::handleIrq();
}

void usart2_isr()
{
	UsartBase<2>::handleIrq();
}

void usart3_isr()
{
	UsartBase<3>::handleIrq();
}
