//
// Can.cpp
//
//  Created on: Dec 8, 2017
//      Author: Alexander
//

#include "Can.hpp"

template<> CanBase<1> *CanBase<1>::instance = nullptr;

void cec_can_isr()
{
	CanBase<1>::handleIrq();
}
