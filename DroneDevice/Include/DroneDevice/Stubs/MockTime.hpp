//
// MockTime.hpp
//
//  Created on: Sep 28, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKTIME_HPP_
#define DRONEDEVICE_STUBS_MOCKTIME_HPP_

#include <cstdint>

namespace Device {

struct MockTime {
	MockTime() = delete;
	MockTime(const MockTime &) = delete;
	MockTime &operator=(const MockTime &) = delete;

	static uint64_t time()
	{
		return 0;
	}
};

} // namespace Device

#endif // DRONEDEVICE_STUBS_MOCKTIME_HPP_
