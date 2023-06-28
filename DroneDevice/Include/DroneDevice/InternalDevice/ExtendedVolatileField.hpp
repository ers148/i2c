//
// ExtendedVolatileField.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_EXTENDEDVOLATILEFIELD_HPP_
#define DRONEDEVICE_INTERNALDEVICE_EXTENDEDVOLATILEFIELD_HPP_

#include <DroneDevice/InternalDevice/Limits.hpp>
#include <DroneDevice/InternalDevice/VolatileField.hpp>

namespace Device {

template<class T, bool readonly = false, FieldScale magnitude = 0, FieldDimension elements = 1, bool important = true>
class ExtendedVolatileField : public VolatileField<T, readonly, magnitude, elements, important> {
	using BaseClass = VolatileField<T, readonly, magnitude, elements, important>;
	using CurrentClass = ExtendedVolatileField<T, readonly, magnitude, elements, important>;
	using BaseClass::value;

public:
	using BaseClass::Type;
	using BaseClass::typeValue;
	using BaseClass::operator=;

	constexpr ExtendedVolatileField(const char *aName, T aValue, const char *aUnit = nullptr,
		T aMinValue = *reinterpret_cast<const T *>(Limits::min<T>()),
		T aMaxValue = *reinterpret_cast<const T *>(Limits::max<T>())) :
		BaseClass{aName, aValue},
		maxValue{aMaxValue},
		minValue{aMinValue},
		unitString{aUnit}
	{
	}

	const void *max() const override
	{
		return &maxValue;
	}

	const void *min() const override
	{
		return &minValue;
	}

	const char *unit() const override
	{
		return unitString;
	}

	Result write(const void *aInput) override
	{
		return writeImpl<readonly>(aInput);
	}

protected:
	const T maxValue;
	const T minValue;
	const char * const unitString;

private:
	template<bool selector>
	typename std::enable_if_t<!selector, Result> writeImpl(const void *aInput)
	{
		T buffer;
		memcpy(&buffer, aInput, sizeof(T));

		if (buffer >= minValue && buffer <= maxValue) {
			memcpy(&value, aInput, sizeof(T));
			return Result::SUCCESS;
		} else {
			return Result::FIELD_RANGE_ERROR;
		}
	}

	template<bool selector>
	typename std::enable_if_t<selector, Result> writeImpl(const void *)
	{
		return Result::FIELD_READ_ONLY;
	}
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_EXTENDEDVOLATILEFIELD_HPP_
