//
// DeviceRequestDescriptor.hpp
//
//  Created on: Jul 12, 2019
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_DEVICEREQUESTDESCRIPTOR_HPP_
#define DRONEDEVICE_PLAZCAN_DEVICEREQUESTDESCRIPTOR_HPP_

#include <chrono>
#include <DroneDevice/RefCounter.hpp>

namespace PlazCan {

struct DeviceRequestDescriptor: Device::RefCounter {
	uint16_t type;
	uint8_t number;
	uint8_t source;

	constexpr DeviceRequestDescriptor() :
		RefCounter{},
		type{},
		number{},
		source{}
	{
	}

	constexpr DeviceRequestDescriptor(uint16_t aType, uint8_t aNumber, uint8_t aSource, uint8_t aDestination) :
		Device::RefCounter{static_cast<Device::DeviceId>(aDestination)},
		type{aType},
		number{aNumber},
		source{aSource}
	{
	}
};

}

#endif // DRONEDEVICE_PLAZCAN_DEVICEREQUESTDESCRIPTOR_HPP_
