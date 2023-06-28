//
// BlankField.hpp
//
//  Created on: Jan 13, 2021
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_BLANKFIELD_HPP_
#define DRONEDEVICE_INTERNALDEVICE_BLANKFIELD_HPP_

#include <DroneDevice/InternalDevice/AbstractField.hpp>
#include <DroneDevice/InternalDevice/Limits.hpp>

namespace Device {

template<typename T, bool readonly = false, FieldScale magnitude = 0, FieldDimension elements = 1,
	bool important = true>
class BlankField : public AbstractField {
	using CurrentClass = BlankField<T, readonly, magnitude, elements, important>;

public:
	using Type = T;
	static constexpr FieldType typeValue{typeToFieldType<T>()};

	constexpr BlankField(const char *aName) :
		nameString{aName}
	{
	}

	constexpr BlankField(const CurrentClass &aOther) :
		nameString{aOther.nameString}
	{
	}

	CurrentClass &operator=(const CurrentClass &aOther)
	{
		nameString = aOther.nameString;
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

protected:
	const char * const nameString;

private:
	template<bool SELECTOR>
	typename std::enable_if_t<!SELECTOR, Result> writeImpl(const void *)
	{
		return Result::FIELD_UNAVAILABLE;
	}

	template<bool SELECTOR>
	typename std::enable_if_t<SELECTOR, Result> writeImpl(const void *)
	{
		return Result::FIELD_READ_ONLY;
	}
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_BLANKFIELD_HPP_
