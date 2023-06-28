//
// VolatileField.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_VOLATILEFIELD_HPP_
#define DRONEDEVICE_INTERNALDEVICE_VOLATILEFIELD_HPP_

#include <DroneDevice/InternalDevice/AbstractField.hpp>
#include <DroneDevice/InternalDevice/Limits.hpp>

namespace Device {

template<typename T, bool readonly = false, FieldScale magnitude = 0, FieldDimension elements = 1,
	bool important = true>
class VolatileField : public AbstractField {
	using CurrentClass = VolatileField<T, readonly, magnitude, elements, important>;

public:
	using Type = T;
	static constexpr FieldType typeValue{typeToFieldType<T>()};

	constexpr VolatileField(const char *aName, T aValue) :
		nameString{aName},
		value{aValue}
	{
	}

	constexpr VolatileField(const CurrentClass &aOther) :
		nameString{aOther.nameString},
		value{aOther.value}
	{
	}

	CurrentClass &operator=(const CurrentClass &aOther)
	{
		nameString = aOther.nameString;
		value = aOther.value;
		return *this;
	}

	FieldDimension dimension() const override
	{
		return elements;
	}

	FieldFlags flags() const override
	{
		return kFieldReadable | (readonly ? 0 : kFieldWritable) | (important ? kFieldImportant : 0);
	}

	const char *name() const override
	{
		return nameString;
	}

	const void *max() const override
	{
		return Limits::max<T>();
	}

	const void *min() const override
	{
		return Limits::min<T>();
	}

	Result read(void *aOutput) const override
	{
		memcpy(aOutput, &value, sizeof(T));
		return Result::SUCCESS;
	}

	FieldScale scale() const override
	{
		return magnitude;
	}

	FieldType type() const override
	{
		return typeValue;
	}

	const char *unit() const override
	{
		return nullptr;
	}

	Result write(const void *aInput) override
	{
		return writeImpl<readonly>(aInput);
	}

	auto &operator=(const T &aInput)
	{
		value = aInput;
		return *this;
	}

	operator T() const
	{
		return value;
	}

protected:
	const char * const nameString;
	T value;

private:
	template<bool SELECTOR>
	typename std::enable_if_t<!SELECTOR, Result> writeImpl(const void *aInput)
	{
		memcpy(&value, aInput, sizeof(T));
		return Result::SUCCESS;
	}

	template<bool SELECTOR>
	typename std::enable_if_t<SELECTOR, Result> writeImpl(const void *)
	{
		return Result::FIELD_READ_ONLY;
	}
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_VOLATILEFIELD_HPP_
