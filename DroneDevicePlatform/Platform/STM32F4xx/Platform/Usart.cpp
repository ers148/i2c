//
// Usart.cpp
//
//  Created on: Mar 27, 2018
//      Author: andrey
//

#include "Usart.hpp"

template<> UsartBase<1> *UsartBase<1>::instance{nullptr};
template<> UsartBase<2> *UsartBase<2>::instance{nullptr};
template<> UsartBase<3> *UsartBase<3>::instance{nullptr};
template<> UsartBase<4> *UsartBase<4>::instance{nullptr};
template<> UsartBase<5> *UsartBase<5>::instance{nullptr};
template<> UsartBase<6> *UsartBase<6>::instance{nullptr};

void usart1_isr(void)
{
	UsartBase<1>::handleIrq();
}

void usart2_isr(void)
{
	UsartBase<2>::handleIrq();
}

void usart3_isr(void)
{
	UsartBase<3>::handleIrq();
}

void uart4_isr(void)
{
	UsartBase<4>::handleIrq();
}

void uart5_isr(void)
{
	UsartBase<5>::handleIrq();
}

void usart6_isr(void)
{
	UsartBase<6>::handleIrq();
}
