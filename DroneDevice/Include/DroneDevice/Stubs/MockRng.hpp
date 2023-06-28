//
// MockRng.hpp
//
//  Created on: Mar 16, 2021
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKRNG_HPP_
#define DRONEDEVICE_STUBS_MOCKRNG_HPP_

#include <cstdint>

namespace Device {

class MockRng {
public:
	MockRng() = delete;
	MockRng(const MockRng &) = delete;
	MockRng &operator=(const MockRng &) = delete;

	static void init()
	{
	}

	static uint32_t random()
	{
		return 0;
	}
};

}

#endif // DRONEDEVICE_STUBS_MOCKRNG_HPP_
