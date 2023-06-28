//
// ProxyRequestDescriptor.hpp
//
//  Created on: Jul 12, 2019
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_PROXYREQUESTDESCRIPTOR_HPP_
#define DRONEDEVICE_PLAZCAN_PROXYREQUESTDESCRIPTOR_HPP_

#include <chrono>
#include <DroneDevice/RefCounter.hpp>

namespace Device {
class DeviceObserver;
}

namespace PlazCan {

struct ProxyRequestDescriptor: Device::RefCounter {
	Device::DeviceObserver *observer;
	Device::RefCounter *token;
	std::chrono::microseconds deadline;
	uint16_t type;
	uint8_t number;

	union {
		struct {
			uint8_t value[Device::kFieldMaxSize];
			Device::FieldId id;
		} field;
		struct {
			uint32_t position;
			Device::FileId id;
		} file;
	} context;

	constexpr ProxyRequestDescriptor() :
		RefCounter{},
		observer{},
		token{},
		deadline{},
		type{},
		number{},
		context{}
	{
	}

	ProxyRequestDescriptor(Device::DeviceObserver *aObserver, Device::RefCounter *aToken,
		std::chrono::microseconds aDeadline, uint16_t aType, uint8_t aNumber, Device::DeviceId aDestination,
		Device::FieldId aField) :
		RefCounter{static_cast<Device::DeviceId>(aDestination)},
		observer{aObserver},
		token{aToken},
		deadline{aDeadline},
		type{aType},
		number{aNumber}
	{
		context.field.id = aField;
	}

	ProxyRequestDescriptor(Device::DeviceObserver *aObserver, Device::RefCounter *aToken,
		std::chrono::microseconds aDeadline, uint16_t aType, uint8_t aNumber, Device::DeviceId aDestination,
		Device::FileId aFile, uint32_t aPosition) :
		RefCounter{static_cast<Device::DeviceId>(aDestination)},
		observer{aObserver},
		token{aToken},
		deadline{aDeadline},
		type{aType},
		number{aNumber}
	{
		context.file.id = aFile;
		context.file.position = aPosition;
	}
};

}

#endif // DRONEDEVICE_PLAZCAN_PROXYREQUESTDESCRIPTOR_HPP_
