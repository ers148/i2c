//
// I2c.cpp
//
//  Created on: Sep 9, 2021
//      Author: p.dudrov
//

#include "I2cBase.hpp"

template<>
I2cBase<1> *I2cBase<1>::instance = nullptr;

template<>
I2cBase<2> *I2cBase<2>::instance = nullptr;

void i2c1_isr(void)
{
	I2cBase<1>::instance->handler();
}

void i2c2_isr(void)
{
	I2cBase<2>::instance->handler();
}
