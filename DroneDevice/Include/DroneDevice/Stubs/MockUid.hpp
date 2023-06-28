//
// MockUid.hpp
//
//  Created on: Dec 2, 2021
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKUID_HPP_
#define DRONEDEVICE_STUBS_MOCKUID_HPP_

namespace Device {

class MockUid {
public:
	MockUid() = delete;
	MockUid(const MockUid &) = delete;
	MockUid &operator=(const MockUid &) = delete;

	static const void *data()
	{
		static const uint8_t desig[12] = {
			0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC
		};

		return desig;
	}

	static constexpr size_t length()
	{
		return 12;
	}
};

} // namespace Device

#endif // DRONEDEVICE_STUBS_MOCKUID_HPP_
