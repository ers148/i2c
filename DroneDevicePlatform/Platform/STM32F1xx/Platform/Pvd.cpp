//
// Pvd.cpp
//
//  Created on: Jan 11, 2018
//      Author: Alexander
//

#include "Pvd.hpp"

Pvd *Pvd::instance = nullptr;

void pvd_isr()
{
	Pvd::handleIrq();
}
