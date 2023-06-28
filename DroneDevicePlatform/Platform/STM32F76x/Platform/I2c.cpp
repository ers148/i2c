//! @file I2c.cpp
//! @author Aleksei Drovnenkov
//! @date Jun 05, 2023

#include "I2c.hpp"

template<> I2cBase<1> *I2cBase<1>::instance = nullptr;
template<> I2cBase<2> *I2cBase<2>::instance = nullptr;
template<> I2cBase<3> *I2cBase<3>::instance = nullptr;
template<> I2cBase<4> *I2cBase<4>::instance = nullptr;

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

void i2c4_ev_isr(void)
{
	I2cBase<4>::handleIrq();
}

void i2c4_er_isr(void)
{
	I2cBase<4>::handleIrq();
}
