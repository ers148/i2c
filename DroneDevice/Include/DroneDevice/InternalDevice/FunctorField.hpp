//
// FunctorField.hpp
//
//  Created on: Nov 05, 2020
//      Author: Ilya
//

#ifndef DRONEDEVICE_INTERNALDEVICE_FUNCTORFIELD_HPP_
#define DRONEDEVICE_INTERNALDEVICE_FUNCTORFIELD_HPP_

#include <DroneDevice/InternalDevice/AbstractField.hpp>
#include <DroneDevice/InternalDevice/Limits.hpp>
#include <type_traits>

namespace Device {

template<typename T, FieldScale magnitude = 0, FieldDimension elements = 1, bool important = true, typename C = T>
class FunctorField : public AbstractField {
	using CurrentClass = FunctorField<T, magnitude, elements, important, C>;
	using ReadFunction = Result (*)(const typename std::remove_reference_t<C> &, void *);
	using WriteFunction = Result (*)(C &, const void *);

public:
	using Type = T;
	static constexpr FieldType typeValue{typeToFieldType<T>()};

	template<typename M = C>
	constexpr FunctorField(const char *aName,
		ReadFunction aRead,
		WriteFunction aWrite, M &&aContext = {}):
		nameString{aName},
		context{std::forward<M>(aContext)},
		readFunc{aRead},
		writeFunc{aWrite}
	{
	}

	constexpr FunctorField(const CurrentClass &aOther):
		nameString{aOther.nameString},
		context{aOther.context},
		readFunc{aOther.readFunc},
		writeFunc{aOther.writeFunc}
	{
	}

	CurrentClass &operator=(const CurrentClass &aOther)
	{
		nameString = aOther.nameString;
		context = aOther.context;
		return *this;
	}

	FieldDimension dimension() const override
	{
		return elements;
	}

	FieldFlags flags() const override
	{
		FieldFlags f = important ? kFieldImportant : 0;

		if (readFunc) {
			f |= kFieldReadable;
		}
		if (writeFunc) {
			f |= kFieldWritable;
		}

		return f;
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
		return readFunc ? readFunc(context, aOutput) : Result::COMMAND_ERROR;
	}

	FieldScale scale() const override
	{
		return magnitude;
	}

	FieldType type() const override
	{
		return typeToFieldType<T>();
	}

	const char *unit() const override
	{
		return nullptr;
	}

	Result write(const void *aInput) override
	{
		return writeFunc ? writeFunc(context, aInput) : Result::FIELD_READ_ONLY;
	}

protected:
	const char *const nameString;
	C context;
	ReadFunction readFunc;
	WriteFunction writeFunc;
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_FUNCTORFIELD_HPP_
