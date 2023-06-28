//
// AbstractField.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_ABSTRACTFIELD_HPP_
#define DRONEDEVICE_INTERNALDEVICE_ABSTRACTFIELD_HPP_

#include <DroneDevice/CoreTypes.hpp>

namespace Device {

class AbstractField {
public:
	virtual ~AbstractField() = default;

	virtual FieldDimension dimension() const = 0;
	virtual FieldFlags flags() const = 0;
	virtual const void *max() const = 0;
	virtual const void *min() const = 0;
	virtual const char *name() const = 0;
	virtual Result read(void *aOutput) const = 0;
	virtual FieldScale scale() const = 0;
	virtual FieldType type() const = 0;
	virtual const char *unit() const = 0;
	virtual Result write(const void *aInput) = 0;
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_ABSTRACTFIELD_HPP_
