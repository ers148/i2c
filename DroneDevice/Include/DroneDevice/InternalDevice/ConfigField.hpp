//
// ConfigField.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_CONFIGFIELD_HPP_
#define DRONEDEVICE_INTERNALDEVICE_CONFIGFIELD_HPP_

#include <DroneDevice/ConfigStorage.hpp>
#include <DroneDevice/InternalDevice/AbstractField.hpp>
#include <DroneDevice/InternalDevice/Limits.hpp>
#include <DroneDevice/TypeHelpers.hpp>

namespace Device {

template<typename Storage, typename T, bool writethrough = true, bool readonly = false,
	FieldScale magnitude = 0, FieldDimension elements = 1, bool important = true>
class ConfigField : public AbstractField {
protected:
	using ContainerType = decltype(TypeHelpers::getClassType(T{}));
	using MemberType = decltype(TypeHelpers::getMemberType(T{}));

public:
	using Type = T;
	static constexpr Device::FieldType typeValue{typeToFieldType<MemberType>()};

	ConfigField(Storage &aStorage, MemberType ContainerType::*aField, const char *aName) :
		storage{aStorage},
		field{aField},
		nameString{aName}
	{
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
		return Limits::max<MemberType>();
	}

	const void *min() const override
	{
		return Limits::min<MemberType>();
	}

	Result read(void *aOutput) const override
	{
		memcpy(aOutput, &(getStorageEntry<ContainerType>(storage).*field), sizeof(MemberType));
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

	auto &operator=(const MemberType &aBuffer)
	{
		memcpy(&(getStorageEntry<ContainerType>(storage).*field), &aBuffer, sizeof(MemberType));
		return *this;
	}

	operator MemberType() const
	{
		return Device::Field::as<MemberType>(&(getStorageEntry<ContainerType>(storage).*field));
	}

protected:
	Storage &storage;
	MemberType ContainerType::*field;

private:
	const char *nameString;

	template<bool selector>
	typename std::enable_if_t<!selector, Result> writeImpl(const void *aInput)
	{
		const auto next = Device::Field::as<MemberType>(aInput);
		const auto current = Device::Field::as<MemberType>(&(getStorageEntry<ContainerType>(storage).*field));

		if (next != current) {
			memcpy(&(getStorageEntry<ContainerType>(storage).*field), aInput, sizeof(MemberType));

			if (writethrough) {
				storage.store();
			}
		}
		return Result::SUCCESS;
	}

	template<bool selector>
	typename std::enable_if_t<selector, Result> writeImpl(const void *)
	{
		return Result::FIELD_READ_ONLY;
	}
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_CONFIGFIELD_HPP_
