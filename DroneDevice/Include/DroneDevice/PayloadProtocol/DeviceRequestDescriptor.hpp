//
// DeviceRequestDescriptor.hpp
//
//  Created on: Jul 12, 2019
//      Author: Alexander
//

#ifndef DRONEDEVICE_PAYLOADPROTOCOL_DEVICEREQUESTDESCRIPTOR_HPP_
#define DRONEDEVICE_PAYLOADPROTOCOL_DEVICEREQUESTDESCRIPTOR_HPP_

#include <DroneDevice/RefCounter.hpp>

namespace PayloadProtocol {

struct DeviceRequestDescriptor: Device::RefCounter {
	uint8_t number;

	constexpr DeviceRequestDescriptor() :
		RefCounter{},
		number{}
	{
	}

	constexpr DeviceRequestDescriptor(uint8_t aNumber, uint8_t aDestination) :
		RefCounter{static_cast<Device::DeviceId>(aDestination)},
		number{aNumber}
	{
	}
};

}

#endif // DRONEDEVICE_PAYLOADPROTOCOL_DEVICEREQUESTDESCRIPTOR_HPP_
