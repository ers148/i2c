//
// RefCounter.hpp
//
//  Created on: Dec 15, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_REFCOUNTER_HPP_
#define DRONEDEVICE_REFCOUNTER_HPP_

#include <DroneDevice/AbstractDevice.hpp>

namespace Device {

struct RefCounter {
public:
	DeviceId destination;
	uint8_t refs;

	constexpr RefCounter() :
		destination{kDeviceReservedId},
		refs{0}
	{
	}

	constexpr RefCounter(DeviceId aDestination) :
		destination{aDestination},
		refs{0}
	{
	}
};

} // namespace Device

#endif // DRONEDEVICE_REFCOUNTER_HPP_
