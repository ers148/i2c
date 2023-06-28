//
// PlatformWrapper.hpp
//
//  Created on: Mar 16, 2021
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_PLATFORMWRAPPER_HPP_
#define DRONEDEVICE_PLAZCAN_PLATFORMWRAPPER_HPP_

#include <DroneDevice/Stubs/MockMutex.hpp>
#include <DroneDevice/Stubs/MockReloader.hpp>
#include <DroneDevice/Stubs/MockRng.hpp>
#include <DroneDevice/Stubs/MockTime.hpp>

namespace PlazCan {

template<typename Bus, typename Time = Device::MockTime, typename Rng = Device::MockRng,
	typename Reloader = Device::MockReloader, typename Mutex = Device::MockMutex>
struct PlatformWrapper {
	using BusType = Bus;
	using MutexType = Mutex;
	using ReloaderType = Reloader;
	using RngType = Rng;
	using TimeType = Time;
};

}

#endif // DRONEDEVICE_PLAZCAN_PLATFORMWRAPPER_HPP_
