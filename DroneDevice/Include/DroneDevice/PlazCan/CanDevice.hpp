//
// CanDevice.hpp
//
//  Created on: Dec 18, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_CANDEVICE_HPP_
#define DRONEDEVICE_PLAZCAN_CANDEVICE_HPP_

#include <DroneDevice/Address.hpp>
#include <DroneDevice/PlazCan/CanardWrapper.hpp>
#include <DroneDevice/PlazCan/UavCanPacket.hpp>
#include <chrono>

namespace PlazCan {

// clang-format off
enum class Health: uint8_t {
	HEALTHY  = 0,
	WARNING  = 1,
	ERROR    = 2,
	CRITICAL = 3
};
// clang-format on

// clang-format off
enum class Mode: uint8_t {
	OPERATIONAL     = 0,
	INITIALIZATION  = 1,
	MAINTENANCE     = 2,
	SOFTWARE_UPDATE = 3,
	OFFLINE         = 7
};
// clang-format on

// clang-format off
enum class InitState : uint8_t {
	NEED_ALLOCATION,
	NEED_REQUEST_INFO,
	NEED_REQUEST_FIELDS_SUMMARY,
	NEED_REQUEST_FIELDS,
	DEVICE_READY,
	DEVICE_UNSUPPORTED
};
// clang-format on

static constexpr Device::DeviceId kAddressUnallocated{0};

struct Status {
	Mode mode;
	Health health;
	uint16_t flags;
	Status(Mode aMode = Mode::OFFLINE, Health aHealth = Health::CRITICAL, uint16_t aFlags = 0):
		mode{aMode},
		health{aHealth},
		flags{aFlags}
	{
	}
};

class CanDevice {
public:
	virtual ~CanDevice() = default;
	virtual Device::DeviceId getBusAddress() const = 0;
	virtual Status getStatus() const = 0;
	virtual UidType getUID() const = 0;
	virtual bool onAcceptanceRequest(uint16_t aDataTypeId, CanardTransferType aTransferType,
		Device::DeviceId aSourceNodeId, Device::DeviceId aDestinationNodeId) = 0;
	virtual void onMessageReceived(const CanardRxTransfer *aTransfer) = 0;
	virtual std::chrono::microseconds onTimeoutOccurred() = 0;
};
} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_CANDEVICE_HPP_
