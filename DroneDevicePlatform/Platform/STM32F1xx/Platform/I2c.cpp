//! @file I2c.cpp
//! @author p.dudrov
//! @date Oct 20, 2021

#include "I2c.hpp"

template<> I2cBase<1> *I2cBase<1>::instance = nullptr;
template<> I2cBase<2> *I2cBase<2>::instance = nullptr;

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
