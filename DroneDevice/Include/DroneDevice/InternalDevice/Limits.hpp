//
// Limits.hpp
//
//  Created on: Aug 17, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_LIMITS_HPP_
#define DRONEDEVICE_INTERNALDEVICE_LIMITS_HPP_

#include <limits>
#include <type_traits>

namespace Device {

namespace Limits {

	template<typename T>
	static const void *max()
	{
		static const T maxValue = std::numeric_limits<T>::max();
		return &maxValue;
	}

	template<typename T>
	static typename std::enable_if_t<std::is_floating_point<T>::value, const void *> min()
	{
		static const T minValue = std::numeric_limits<T>::lowest();
		return &minValue;
	}

	template<typename T>
	static typename std::enable_if_t<!std::is_floating_point<T>::value, const void *> min()
	{
		static const T minValue = std::numeric_limits<T>::min();
		return &minValue;
	}

} // namespace Limits

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_LIMITS_HPP_
