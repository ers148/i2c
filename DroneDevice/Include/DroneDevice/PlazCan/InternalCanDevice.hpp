//
// InternalCanDevice.hpp
//
//  Created on: Dec 15, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_INTERNALCANDEVICE_HPP_
#define DRONEDEVICE_PLAZCAN_INTERNALCANDEVICE_HPP_

#include <DroneDevice/PlazCan/CanDevice.hpp>
#include <DroneDevice/PlazCan/ParamSlave.hpp>
#include <DroneDevice/PlazCan/PlazCanSlave.hpp>
#include <DroneDevice/Stubs/MockReloader.hpp>
#include <iterator>

namespace PlazCan {

template<typename T>
struct MockSlave {
	constexpr MockSlave(T &)
	{
	}

	constexpr bool process(const CanardRxTransfer *)
	{
		return false;
	}
};

template<typename T, typename U, bool SELECTOR>
struct SlaveTypeSelector {
	using Type = T;
};

template<typename T, typename U>
struct SlaveTypeSelector<T, U, false> {
	using Type = U;
};

template<typename Component, typename UavCanHandler, bool enablePlazCan = true, bool enableUavCan = true>
class InternalCanDevice : public CanDevice {
	static_assert(sizeof(Device::DeviceHash) == sizeof(UidType), "Incorrect type");
	static_assert(sizeof(Device::DeviceHash) == kUidLength, "Incorrect type");

	using microseconds = std::chrono::microseconds;
	using seconds = std::chrono::seconds;
	using CurrentType = InternalCanDevice<Component, UavCanHandler, enablePlazCan, enableUavCan>;

protected:
	using PlatformType = typename UavCanHandler::PlatformType;
	using ReloaderType = typename PlatformType::ReloaderType;
	using RngType = typename PlatformType::RngType;
	using TimeType = typename PlatformType::TimeType;

private:
	friend class ParamSlave<CurrentType>;
	friend class PlazCanSlave<CurrentType>;

	// clang-format off
	static constexpr uint32_t kAllocTimeout{400};
	static constexpr uint32_t kMinAllocInterval{600};
	static constexpr uint32_t kMaxAllocInterval{1000};
	// clang-format on

	static microseconds makeAllocInterval()
	{
		return microseconds{kMinAllocInterval * 1000
			+ RngType::random() % ((kMaxAllocInterval - kMinAllocInterval) * 1000)};
	}

	InternalCanDevice(const InternalCanDevice &) = delete;
	InternalCanDevice operator=(const InternalCanDevice &) = delete;

public:
	InternalCanDevice(Component &aDevice, UavCanHandler &aHandler,
		Device::DeviceId aDesiredAddress = kAddressUnallocated,
		bool aDynamicAllocation = true,
		microseconds aNodeStatusPeriod = seconds{1}):
		device{aDevice},
		handler{aHandler},
		status{Mode::OPERATIONAL, Health::HEALTHY, 0},
		currentAddress{aDynamicAllocation ? kAddressUnallocated : aDesiredAddress},
		desiredAddress{aDesiredAddress},
		paramSlave{*this},
		plazCanSlave{*this},
		deviceStartTime{TimeType::seconds()},
		nextAllocationTime{makeAllocInterval()},
		nextNodeStatusTime{TimeType::microseconds()},
		prevAllocationTime{0},
		nodeStatusPeriod{aNodeStatusPeriod},
		nodeStatusSent{true},
		sequences{0, 0}
	{
	}

	Device::DeviceId getBusAddress() const override
	{
		return currentAddress;
	}

	Status getStatus() const override
	{
		return status;
	}

	UidType getUID() const override
	{
		return device.deviceHash();
	}

	bool onAcceptanceRequest(uint16_t aDataTypeId, CanardTransferType aTransferType,
		Device::DeviceId /*aSourceNodeId*/, Device::DeviceId aDestinationNodeId) override
	{
		// Accept only a limited range of messages by default
		static const DataType::Message acceptedMessages[] = {
			DataType::Message::PLAZ_ALLOCATION,
			DataType::Message::ALLOCATION,
			DataType::Message::GLOBAL_TIME_SYNC,
		};
		static const DataType::Service acceptedServices[] = {
			DataType::Service::GET_NODE_INFO,
			DataType::Service::GET_TRANSPORT_STATS,
			DataType::Service::RESTART_NODE,
			DataType::Service::PARAM_GET_SET,
			DataType::Service::GET_FIELDS_SUMMARY,
			DataType::Service::GET_FIELD_INFO,
			DataType::Service::FIELD_READ,
			DataType::Service::FIELD_WRITE,
			DataType::Service::GET_FILE_INFO,
			DataType::Service::FILE_READ,
			DataType::Service::FILE_WRITE,
		};

		// TODO Use lower_bound/upper_bound, verify in debug mode that the arrays are sorted
		if (aTransferType == CanardTransferTypeBroadcast) {
			for (auto iter = std::cbegin(acceptedMessages); iter != std::cend(acceptedMessages); ++iter) {
				if (*iter == static_cast<DataType::Message>(aDataTypeId)) {
					return true;
				}
			}
		} else if (aTransferType == CanardTransferTypeRequest && aDestinationNodeId == currentAddress) {
			for (auto iter = std::cbegin(acceptedServices); iter != std::cend(acceptedServices); ++iter) {
				if (*iter == static_cast<DataType::Service>(aDataTypeId)) {
					return true;
				}
			}
		}

		return false;
	}

	void onMessageReceived(const CanardRxTransfer *aTransfer) override
	{
		if (aTransfer->transfer_type == CanardTransferTypeRequest && aTransfer->destination_node_id == currentAddress) {
			if (!processStandardMessage(aTransfer)) {
				if (!plazCanSlave.process(aTransfer)) {
					paramSlave.process(aTransfer);
				}
			}
		} else if (aTransfer->transfer_type == CanardTransferTypeBroadcast) {
			processAllocationMessage(aTransfer);
		}
	}

	microseconds onTimeoutOccurred() override
	{
		microseconds nextWakeTime = microseconds::max();
		const auto currentTime = TimeType::microseconds();

		if (currentAddress != kAddressUnallocated && nodeStatusPeriod.count() != 0) {
			if (currentTime >= nextNodeStatusTime) {
				nextNodeStatusTime = currentTime + nodeStatusPeriod;
				sendNodeStatus();
			}

			if (nextWakeTime > nextNodeStatusTime) {
				nextWakeTime = nextNodeStatusTime;
			}
		}

		if (handler.isNextAllocatee(this)) {
			if (currentTime >= nextAllocationTime) {
				resetAllocationSequence(currentTime);
				prevAllocationTime = currentTime;
				sendAllocationRequest0(desiredAddress);
			}

			if (nextWakeTime > nextAllocationTime) {
				nextWakeTime = nextAllocationTime;
			}
		}

		return nextWakeTime;
	}

	uint16_t getFlags() const
	{
		return getStatus().flags;
	}

	Health getHealth() const
	{
		return getStatus().health;
	}

	Mode getMode() const
	{
		return getStatus().mode;
	}

	seconds getUptime() const
	{
		return TimeType::seconds() - deviceStartTime;
	}

	void setFlags(uint16_t aFlags)
	{
		if (nodeStatusSent) {
			status.flags = aFlags;
			nodeStatusSent = false;
		} else {
			status.flags |= aFlags;
		}
	}

	void setHealth(Health aHealth)
	{
		status.health = aHealth;
	}

	void setMode(Mode aMode)
	{
		status.mode = aMode;
	}

	void setNodeStatusPeriod(microseconds aNodeStatusPeriod)
	{
		nodeStatusPeriod = aNodeStatusPeriod;
	}

protected:
	Component &device;
	UavCanHandler &handler;

	Status status;
	Device::DeviceId currentAddress;
	Device::DeviceId desiredAddress;

private:
	typename SlaveTypeSelector<ParamSlave<CurrentType>, MockSlave<CurrentType>, enableUavCan>::Type paramSlave;
	typename SlaveTypeSelector<PlazCanSlave<CurrentType>, MockSlave<CurrentType>, enablePlazCan>::Type plazCanSlave;

	seconds deviceStartTime;
	microseconds nextAllocationTime;
	microseconds nextNodeStatusTime;
	microseconds prevAllocationTime;
	microseconds nodeStatusPeriod;

	bool nodeStatusSent;

	struct {
		uint8_t allocation;
		uint8_t status;
	} sequences;

	void processAllocationMessage(const CanardRxTransfer *aTransfer)
	{
		if (aTransfer->data_type_id == DataType::Message::ALLOCATION) {
			resetAllocationSequence();

			if (TimeType::microseconds() - prevAllocationTime > microseconds{kAllocTimeout * 1000}) {
				return;
			}

			if (aTransfer->payload_len == kAllocationStage0) {
				uint8_t payload[kAllocationStage0 - 1];
				CanHelpers::decodeBlobField(aTransfer, 1,
					static_cast<size_t>(aTransfer->payload_len - 1), payload);

				if (!memcmp(device.deviceHash().data(), payload, sizeof(payload))) {
					sendAllocationRequest1(desiredAddress);
				}
			} else if (aTransfer->payload_len == kAllocationStage1) {
				uint8_t payload[kAllocationStage1 - 1];
				CanHelpers::decodeBlobField(aTransfer, 1,
					static_cast<size_t>(aTransfer->payload_len - 1), payload);

				if (!memcmp(device.deviceHash().data(), payload, sizeof(payload))) {
					sendAllocationRequest2(desiredAddress);
				}
			} else if (aTransfer->payload_len == kAllocationStage2) {
				uint8_t payload[kAllocationStage2 - 1];
				uint8_t proposedAddress = 0;

				CanHelpers::decodeBlobField(aTransfer, 1,
					static_cast<size_t>(aTransfer->payload_len - 1), payload);
				canardDecodeScalar(aTransfer, 0, 7, false, &proposedAddress);

				if (!memcmp(device.deviceHash().data(), payload, sizeof(payload)) && proposedAddress != 0) {
					currentAddress = static_cast<Device::DeviceId>(proposedAddress);
				}
			}
		}
	}

	bool processStandardMessage(const CanardRxTransfer *aTransfer)
	{
		switch (aTransfer->data_type_id) {
			case DataType::Service::GET_NODE_INFO:
				sendNodeInfo(aTransfer);
				return true;

			case DataType::Service::GET_TRANSPORT_STATS:
				sendTransportStats(aTransfer);
				return true;

			case DataType::Service::RESTART_NODE:
				restartNode<ReloaderType>(aTransfer);
				return true;

			default:
				return false;
		}
	}

	void resetAllocationSequence(microseconds aCurrentTime = TimeType::microseconds())
	{
		nextAllocationTime = aCurrentTime + makeAllocInterval();
	}

	template<typename U>
	typename std::enable_if_t<std::is_same<U, Device::MockReloader>::value> restartNode(
		const CanardRxTransfer *)
	{
	}

	template<typename U>
	typename std::enable_if_t<!std::is_same<U, Device::MockReloader>::value> restartNode(
		const CanardRxTransfer *aTransfer)
	{
		uint64_t magicNumber{0};
		canardDecodeScalar(aTransfer, 0, 40, false, &magicNumber);

		// TODO From little-endian to machine type
		if (magicNumber == static_cast<uint64_t>(RestartNodeMagicNumber::MAGIC_DFU)) {
			ReloaderType::resetToBootloader();
		} else if (magicNumber == static_cast<uint64_t>(RestartNodeMagicNumber::MAGIC_RESTART)) {
			ReloaderType::reset();
		}
	}

	void fillNodeStatusMessage(NodeStatusMessage &aMessage)
	{
		const auto nodeStatus = getStatus();

		aMessage.uptime_sec = static_cast<uint32_t>(getUptime().count());
		aMessage.health = static_cast<uint8_t>(nodeStatus.health) & 0x03;
		aMessage.mode = static_cast<uint8_t>(nodeStatus.mode) & 0x07;
		aMessage.sub_mode = 0; // Reserved
		aMessage.vendor_specific_status_code = nodeStatus.flags;
	}

	void sendAllocationRequest0(Device::DeviceId aAddress)
	{
		AllocationMessage<kAllocationPart0> packet;

		packet.node_id = static_cast<uint8_t>(aAddress) & 0x7F;
		packet.first_part_of_unique_id = 1;
		memcpy(packet.unique_id, device.deviceHash().data(), kAllocationPart0);

		handler.sendCanMessage(kAddressUnallocated, DataType::ALLOCATION, &sequences.allocation,
			&packet, sizeof(packet));
	}

	void sendAllocationRequest1(Device::DeviceId aAddress)
	{
		AllocationMessage<kAllocationPart1> packet;

		packet.node_id = static_cast<uint8_t>(aAddress) & 0x7F;
		packet.first_part_of_unique_id = 0;
		memcpy(packet.unique_id, device.deviceHash().data() + kAllocationOffset1, kAllocationPart1);

		handler.sendCanMessage(kAddressUnallocated, DataType::ALLOCATION, &sequences.allocation,
			&packet, sizeof(packet));
	}

	void sendAllocationRequest2(Device::DeviceId aAddress)
	{
		AllocationMessage<kAllocationPart2> packet;

		packet.node_id = static_cast<uint8_t>(aAddress) & 0x7F;
		packet.first_part_of_unique_id = 0;
		memcpy(packet.unique_id, device.deviceHash().data() + kAllocationOffset2, kAllocationPart2);

		handler.sendCanMessage(kAddressUnallocated, DataType::ALLOCATION, &sequences.allocation,
			&packet, sizeof(packet));
	}

	void sendNodeInfo(const CanardRxTransfer *aTransfer)
	{
		GetNodeInfoResponse<0, kMaxNameLength> response;
		const auto version = device.deviceVersion();
		const size_t nameLength = strlen(device.deviceName());
		const size_t responseLength = sizeof(response) - sizeof(response.name) + nameLength;

		fillNodeStatusMessage(response.status);

		response.software_version.major = version.sw.major;
		response.software_version.minor = version.sw.minor;
		response.software_version.optional_field_flags = 1; // VCS commit field is initialized, Image CRC is not
		response.software_version.image_crc = 0;
		response.software_version.vcs_commit = version.sw.revision;

		response.hardware_version.major = version.hw.major;
		response.hardware_version.minor = version.hw.minor;
		memcpy(response.hardware_version.unique_id, device.deviceHash().data(),
			sizeof(response.hardware_version.unique_id));
		response.hardware_version.certificate_of_authenticity_length = 0;

		memcpy(response.name, device.deviceName(), nameLength);

		handler.sendCanServiceResponse(currentAddress, aTransfer->source_node_id,
			static_cast<DataType::Service>(aTransfer->data_type_id), aTransfer->transfer_id,
			&response, responseLength);
	}

	void sendNodeStatus()
	{
		NodeStatusMessage message;
		fillNodeStatusMessage(message);
		handler.sendCanMessage(currentAddress, DataType::NODE_STATUS, &sequences.status, &message, sizeof(message));
		nodeStatusSent = true;
	}

	static void fillUint48(uint8_t *buffer, uint64_t value)
	{
		memcpy(buffer, &value, 6);
	}

	void sendTransportStats(const CanardRxTransfer *aTransfer)
	{
		const auto interfaceStats = handler.getInterfaceStatistics();
		const auto transferStats = handler.getTransferStatistics();
		TransportStatsResponse<1> response;

		fillUint48(response.transfers_tx, transferStats.transfers_tx);
		fillUint48(response.transfers_rx, transferStats.transfers_rx);
		fillUint48(response.transfer_errors, transferStats.dropped_transfers + transferStats.transfer_errors);

		fillUint48(response.can_iface_stats[0].frames_tx, interfaceStats.tx);
		fillUint48(response.can_iface_stats[0].frames_rx, interfaceStats.rx);
		fillUint48(response.can_iface_stats[0].errors, interfaceStats.errors);

		handler.sendCanServiceResponse(currentAddress, aTransfer->source_node_id,
			static_cast<DataType::Service>(aTransfer->data_type_id), aTransfer->transfer_id,
			&response, sizeof(response));
	}
};

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_INTERNALCANDEVICE_HPP_
