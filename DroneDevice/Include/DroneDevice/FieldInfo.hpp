//
// FieldInfo.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_FIELDINFO_HPP_
#define DRONEDEVICE_FIELDINFO_HPP_

#include <DroneDevice/CoreTypes.hpp>

namespace Device {

struct FieldInfo {
	FieldId index;
	FieldDimension dimension;
	FieldFlags flags;
	FieldScale scale;
	FieldType type;
	const char *name;
	const char *unit;
	const void *min;
	const void *max;
};

} // namespace Device

#endif // DRONEDEVICE_FIELDINFO_HPP_
