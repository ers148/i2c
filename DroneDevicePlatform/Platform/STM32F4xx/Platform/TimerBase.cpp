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
template<> TimerBase<4> *TimerBase<4>::instance = nullptr;
template<> TimerBase<5> *TimerBase<5>::instance = nullptr;
template<> TimerBase<6> *TimerBase<6>::instance = nullptr;
template<> TimerBase<7> *TimerBase<7>::instance = nullptr;
template<> TimerBase<8> *TimerBase<8>::instance = nullptr;
template<> TimerBase<9> *TimerBase<9>::instance = nullptr;
template<> TimerBase<10> *TimerBase<10>::instance = nullptr;
template<> TimerBase<11> *TimerBase<11>::instance = nullptr;
template<> TimerBase<12> *TimerBase<12>::instance = nullptr;
template<> TimerBase<13> *TimerBase<13>::instance = nullptr;
template<> TimerBase<14> *TimerBase<14>::instance = nullptr;

void tim1_up_tim10_isr()
{
	TimerBase<1>::handleIrq();
	TimerBase<10>::handleIrq();
}

void tim2_isr()
{
	TimerBase<2>::handleIrq();
}

void tim3_isr()
{
	TimerBase<3>::handleIrq();
}

void tim4_isr()
{
	TimerBase<4>::handleIrq();
}

void tim5_isr()
{
	TimerBase<5>::handleIrq();
}

void tim6_dac_isr()
{
	TimerBase<6>::handleIrq();
}

void tim7_isr()
{
	TimerBase<7>::handleIrq();
}

void tim8_up_tim13_isr()
{
	TimerBase<8>::handleIrq();
	TimerBase<13>::handleIrq();
}

void tim1_brk_tim9_isr()
{
	TimerBase<1>::handleIrq();
	TimerBase<9>::handleIrq();
}

void tim8_brk_tim12_isr()
{
	TimerBase<8>::handleIrq();
	TimerBase<12>::handleIrq();
}

void tim8_trg_com_tim14_isr()
{
	TimerBase<8>::handleIrq();
	TimerBase<14>::handleIrq();
}
