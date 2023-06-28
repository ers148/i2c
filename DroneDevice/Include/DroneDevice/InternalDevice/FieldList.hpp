//
// FieldList.hpp
//
//  Created on: Aug 30, 2018
//      Author: Alexander
//

#ifndef DRONDEVICE_INTERNALDEVICE_FIELDLIST_HPP_
#define DRONDEVICE_INTERNALDEVICE_FIELDLIST_HPP_

#include <DroneDevice/InternalDevice/AbstractField.hpp>
#include <tuple>
#include <type_traits>

namespace Device {

template<typename... Fields>
class FieldList {
	static constexpr size_t kCount{sizeof...(Fields)};
	std::tuple<Fields&...> elements;

public:
	constexpr FieldList(Fields&... aFields) :
		elements{std::tie(aFields...)}
	{
	}

	static constexpr size_t count()
	{
		return kCount;
	}

	constexpr Device::AbstractField *get(Device::FieldId aField)
	{
		return getImpl<0, Fields...>(aField);
	}

private:
	// Get field by index, non-constant version

	template<Device::FieldId N, typename T>
	constexpr Device::AbstractField *getImpl(Device::FieldId aField)
	{
		return aField == N ? &std::get<N>(elements) : nullptr;
	}

	template<Device::FileId N, typename T1, typename T2, typename... Ts>
	constexpr Device::AbstractField *getImpl(Device::FieldId aField)
	{
		auto * const pointer = getImpl<N, T1>(aField);
		return pointer != nullptr ? pointer : getImpl<N + 1, T2, Ts...>(aField);
	}
};

template<typename... Ts>
static constexpr auto makeFieldList(Ts&&... aArgs)
{
	return FieldList<std::remove_reference_t<Ts>...>(std::forward<Ts>(aArgs)...);
}

}

#endif // DRONDEVICE_INTERNALDEVICE_FIELDLIST_HPP_
