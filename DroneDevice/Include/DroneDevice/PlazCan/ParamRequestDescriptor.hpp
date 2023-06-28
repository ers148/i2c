//
// ParamRequestDescriptor.hpp
//
//  Created on: Feb 5, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_PARAMREQUESTDESCRIPTOR_HPP_
#define DRONEDEVICE_PLAZCAN_PARAMREQUESTDESCRIPTOR_HPP_

#include <DroneDevice/PlazCan/DeviceRequestDescriptor.hpp>

namespace PlazCan {

struct ParamRequestDescriptor: DeviceRequestDescriptor {
	static constexpr size_t kMaxParamSize = sizeof(int64_t);

	Device::FieldType paramType;
	uint8_t min[kMaxParamSize];
	uint8_t max[kMaxParamSize];
	uint8_t currentValue[kMaxParamSize];
	uint8_t desiredValue[kMaxParamSize];
	char name[CanHelpers::kMaxFieldNameLength];

	constexpr ParamRequestDescriptor() :
		DeviceRequestDescriptor{},
		paramType{Device::FieldType::UNDEFINED},
		min{},
		max{},
		currentValue{},
		desiredValue{},
		name{}
	{
	}

	constexpr ParamRequestDescriptor(uint16_t aType, uint8_t aNumber, uint8_t aSource, uint8_t aDestination) :
		DeviceRequestDescriptor{aType, aNumber, aSource, aDestination},
		paramType{Device::FieldType::UNDEFINED},
		min{},
		max{},
		currentValue{},
		desiredValue{},
		name{}
	{
	}
};

}

#endif // DRONEDEVICE_PLAZCAN_PARAMREQUESTDESCRIPTOR_HPP_
