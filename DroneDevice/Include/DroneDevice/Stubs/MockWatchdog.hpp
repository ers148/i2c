//
// MockWatchdog.hpp
//
//  Created on: Mar 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKWATCHDOG_HPP_
#define DRONEDEVICE_STUBS_MOCKWATCHDOG_HPP_

#include <cstdint>

namespace Device {

class MockWatchdog {
public:
	MockWatchdog(uint32_t)
	{
	}

	void reset()
	{
	}
};

} // namespace Device

#endif // DRONEDEVICE_STUBS_MOCKWATCHDOG_HPP_
