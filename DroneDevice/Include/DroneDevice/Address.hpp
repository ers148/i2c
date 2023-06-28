//
// Address.hpp
//
//  Created on: Dec 18, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_ADDRESS_HPP_
#define DRONEDEVICE_ADDRESS_HPP_

#include <functional>
#include <utility>
#include <DroneDevice/AbstractDevice.hpp>

namespace Device {

template<DeviceId value, typename T>
struct StaticHubNode {
	T *nodePtr;

	StaticHubNode(T *aNode) :
		nodePtr{aNode}
	{
	}

	DeviceId address() const
	{
		return value;
	}

	T *node()
	{
		return nodePtr;
	}
};

template<typename T>
struct DynamicHubNode {
	T *nodePtr;
	DeviceId &value;

	DynamicHubNode(T *aNode, DeviceId &aAddress) :
		nodePtr{aNode},
		value{aAddress}
	{
	}

	DeviceId address() const
	{
		return value;
	}

	T *node()
	{
		return nodePtr;
	}
};

template<typename T>
struct HubNodeFunctor {
	std::function<T *()> func;

	HubNodeFunctor(std::function<T *()> aFunc) :
		func{aFunc}
	{
	}

	DeviceId address() const
	{
		return func() ? func()->getBusAddress() : Device::kDeviceReservedId;
	}

	T *node()
	{
		return func();
	}
};

template<typename T>
constexpr auto makeDynamicHubNode(T *node, DeviceId &address)
{
	return DynamicHubNode<T>(node, address);
}

template<DeviceId value, typename T>
constexpr auto makeStaticHubNode(T *node)
{
	return StaticHubNode<value, T>(node);
}

} // namespace Device

#endif // DRONEDEVICE_ADDRESS_HPP_
