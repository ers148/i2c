//
// I2cBase.cpp
//
//  Created on: Oct 20, 2021
//      Author: p.dudrov
//

#include "I2c.hpp"

template<> I2cBase<1> *I2cBase<1>::instance = nullptr;
template<> I2cBase<2> *I2cBase<2>::instance = nullptr;
template<> I2cBase<3> *I2cBase<3>::instance = nullptr;

void i2c1_ev_isr(void)
{
	I2cBase<1>::handleIrq();
}

void i2c1_er_isr(void)
{
	I2cBase<1>::handleIrq();
}

void i2c2_ev_isr(void)
{
	I2cBase<2>::handleIrq();
}

void i2c2_er_isr(void)
{
	I2cBase<2>::handleIrq();
}

void i2c3_ev_isr(void)
{
	I2cBase<3>::handleIrq();
}

void i2c3_er_isr(void)
{
	I2cBase<3>::handleIrq();
}
