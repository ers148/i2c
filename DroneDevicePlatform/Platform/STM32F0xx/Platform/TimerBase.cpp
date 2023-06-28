//
// TimerBase.cpp
//
//  Created on: Dec 22, 2017
//      Author: Alexander
//

#include "TimerBase.hpp"

template<> TimerBase<1> *TimerBase<1>::instance = nullptr;
template<> TimerBase<2> *TimerBase<2>::instance = nullptr;
template<> TimerBase<3> *TimerBase<3>::instance = nullptr;
template<> TimerBase<6> *TimerBase<6>::instance = nullptr;
template<> TimerBase<7> *TimerBase<7>::instance = nullptr;
template<> TimerBase<14> *TimerBase<14>::instance = nullptr;
template<> TimerBase<15> *TimerBase<15>::instance = nullptr;
template<> TimerBase<16> *TimerBase<16>::instance = nullptr;
template<> TimerBase<17> *TimerBase<17>::instance = nullptr;

void tim1_brk_up_trg_com_isr()
{
	TimerBase<1>::handleIrq();
}

void tim2_isr()
{
	TimerBase<2>::handleIrq();
}

void tim3_isr()
{
	TimerBase<3>::handleIrq();
}

void tim6_dac_isr()
{
	TimerBase<6>::handleIrq();
}

void tim7_isr()
{
	TimerBase<7>::handleIrq();
}

void tim14_isr()
{
	TimerBase<14>::handleIrq();
}

void tim15_isr()
{
	TimerBase<15>::handleIrq();
}

void tim16_isr()
{
	TimerBase<16>::handleIrq();
}

void tim17_isr()
{
	TimerBase<17>::handleIrq();
}
