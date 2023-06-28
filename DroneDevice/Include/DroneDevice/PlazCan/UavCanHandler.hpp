//
// UavcanHandler.hpp
//
//  Created on: Dec 12, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_UAVCANHANDLER_HPP_
#define DRONEDEVICE_PLAZCAN_UAVCANHANDLER_HPP_

#include <algorithm>
#include <array>
#include <DroneDevice/Can.hpp>
#include <DroneDevice/PlazCan/CanardWrapper.hpp>
#include <DroneDevice/PlazCan/CanDevice.hpp>
#include <DroneDevice/PlazCan/HashFinder.hpp>
#include <DroneDevice/PlazCan/UavCanPacket.hpp>
#include <DroneDevice/Stubs/MockMutex.hpp>

namespace PlazCan {

template<typename Platform, size_t maxNodeCount>
class UavCanHandler {
	using seconds = std::chrono::seconds;
	using microseconds = std::chrono::microseconds;

	using BusType = typename Platform::BusType;
	using MutexType = typename Platform::MutexType;
	using TimeType = typename Platform::TimeType;

	static void onReceptionCallback(CanardInstance *instance, CanardRxTransfer *transfer)
	{
		UavCanHandler *const handler = reinterpret_cast<UavCanHandler *>(instance->user_reference);

		handler->mutex.unlock();
		for (auto entry : handler->nodes) {
			if (entry != nullptr) {
				entry->onMessageReceived(transfer);
			}
		}
		handler->mutex.lock();
	}

	static bool shouldAcceptCallback(const CanardInstance *instance, uint64_t *outDataTypeSignature,
		uint16_t dataTypeId, CanardTransferType transferType, uint8_t sourceNodeId, uint8_t destinationNodeId)
	{
		UavCanHandler *const handler = reinterpret_cast<UavCanHandler *>(instance->user_reference);
		bool shouldAccept = false;

		for (auto entry : handler->nodes) {
			if (entry != nullptr) {
				if (entry->onAcceptanceRequest(dataTypeId, transferType, sourceNodeId, destinationNodeId)) {
					shouldAccept = true;
					break;
				}
			}
		}

		if (shouldAccept) {
			// Message has passed acceptance filtering so it is assumed that signature is known
			*outDataTypeSignature = handler->findHashById(transferType, dataTypeId);
			return true;
		} else {
			return false;
		}
	}

	static constexpr seconds kDefaultCleanupInterval{1};

	UavCanHandler(const UavCanHandler &) = delete;

	UavCanHandler operator=(const UavCanHandler &) = delete;

public:
	using PlatformType = Platform;

	UavCanHandler(BusType &aBus, void *aArena, size_t aArenaSize,
		uint64_t (*aHashFinder)(CanardTransferType, uint16_t) = nullptr):
		bus{aBus},
		canard{}, // Suppress warning, object must be initialized with canardInit
		nodes{},
		mutex{},
		hashFinder{aHashFinder},
		nextCleanupTime{kDefaultCleanupInterval + TimeType::microseconds()}
	{
		canardInit(&canard, aArena, aArenaSize, onReceptionCallback, shouldAcceptCallback, this);

		for (auto &entry : nodes) {
			entry = nullptr;
		}
	}

	bool attach(CanDevice *aDevice)
	{
		auto iter = std::find(nodes.begin(), nodes.end(), nullptr);

		if (iter != nodes.end()) {
			*iter = aDevice;
			return true;
		} else {
			return false;
		}
	}

	void detach(const CanDevice *aDevice)
	{
		auto iter = std::find(nodes.begin(), nodes.end(), aDevice);

		if (iter != nodes.end()) {
			*iter = nullptr;
		}
	}

	CanStatistics getInterfaceStatistics() const
	{
		return bus.getStatistics();
	}

	CanardPoolAllocatorStatistics getPoolStatistics() const
	{
		return canardGetPoolAllocatorStatistics(&canard);
	}

	CanardTransferStatistics getTransferStatistics() const
	{
		return canardGetTransferStatistics(&canard);
	}

	bool isAddressFree(Device::DeviceId aAddress) const
	{
		for (auto entry : nodes) {
			if (entry != nullptr && entry->getBusAddress() == aAddress) {
				return false;
			}
		}

		return true;
	}

	bool isNextAllocatee(const CanDevice *aDevice) const
	{
		for (auto entry : nodes) {
			if (entry != nullptr && entry->getBusAddress() == kAddressUnallocated) {
				return entry == aDevice;
			}
		}

		return false;
	}

	void onMessageReceived(const CanMessage *aMessage, size_t aCount)
	{
		while (aCount--) {
			CanardCANFrame frame;
			memcpy(frame.data, aMessage->data, aMessage->length);
			frame.data_len = aMessage->length;
			frame.id = aMessage->id | CANARD_CAN_FRAME_EFF;

			mutex.lock();
			canardHandleRxFrame(&canard, &frame, aMessage->timestamp);
			mutex.unlock();

			aMessage++;
		}
	}

	microseconds onTimeoutOccurred()
	{
		const auto currentTime = TimeType::microseconds();

		if (currentTime >= nextCleanupTime) {
			cleanupStaleTransfers(currentTime);
			nextCleanupTime = currentTime + kDefaultCleanupInterval;
		}

		microseconds nextWakeTime = nextCleanupTime;

		for (auto entry : nodes) {
			if (entry != nullptr) {
				nextWakeTime = std::min(nextWakeTime, entry->onTimeoutOccurred());
			}
		}

		return nextWakeTime;
	}

	void sendCanMessage(Device::DeviceId aSourceNodeId, uint16_t aDataTypeId,
		uint8_t *aTransferId, const void *aData, size_t aLength)
	{
		mutex.lock();
		canardBroadcast(&canard,
			static_cast<uint8_t>(aSourceNodeId),
			findHashById(CanardTransferTypeBroadcast, aDataTypeId),
			aDataTypeId,
			aTransferId,
			0,
			aData,
			static_cast<uint16_t>(aLength));
		mutex.unlock();

		enqueuePackets();
	}

	void sendCanServiceRequest(Device::DeviceId aSourceNodeId, Device::DeviceId aDestinationNodeId,
		uint16_t aDataTypeId, uint8_t *aTransferId, const void *aData, size_t aLength)
	{
		mutex.lock();
		canardRequestOrRespond(&canard,
			static_cast<uint8_t>(aSourceNodeId),
			static_cast<uint8_t>(aDestinationNodeId),
			findHashById(CanardTransferTypeRequest, aDataTypeId),
			static_cast<uint8_t>(aDataTypeId),
			aTransferId,
			0,
			CanardRequest,
			aData,
			static_cast<uint16_t>(aLength));
		mutex.unlock();

		enqueuePackets();
	}

	void sendCanServiceResponse(Device::DeviceId aSourceNodeId, Device::DeviceId aDestinationNodeId,
		DataType::Service aDataTypeId, uint8_t aTransferId, const void *aData, size_t aLength)
	{
		mutex.lock();
		canardRequestOrRespond(&canard,
			static_cast<uint8_t>(aSourceNodeId),
			static_cast<uint8_t>(aDestinationNodeId),
			findHashById(CanardTransferTypeResponse, aDataTypeId),
			static_cast<uint8_t>(aDataTypeId),
			&aTransferId,
			0,
			CanardResponse,
			aData,
			static_cast<uint16_t>(aLength));
		mutex.unlock();

		enqueuePackets();
	}

	size_t getNodeCount() const
	{
		size_t count{0};

		for (auto &entry : nodes) {
			if (entry != nullptr) {
				++count;
			}
		}

		return count;
	}

	size_t getNodeTotal() const
	{
		return maxNodeCount;
	}

private:
	uint64_t findHashById(CanardTransferType aType, uint16_t aTransferId)
	{
		// clang-format off
		using MessageFinder = HashFinder<
				HashId<DataType::Message::PLAZ_ALLOCATION,        0x0B2A812620A11D40ULL>,
				HashId<DataType::Message::ALLOCATION,             0x0B2A812620A11D40ULL>,
				HashId<DataType::Message::GLOBAL_TIME_SYNC,       0x20271116A793C2DBULL>,
				HashId<DataType::Message::NODE_STATUS,            0x0F0868D0C1A7C6F1ULL>,
				HashId<DataType::Message::COMPOSITE_FIELD_VALUES, 0xC0D0FE11431D2AC1ULL>
			>;
		// clang-format on

		// clang-format off
		using ServiceFinder = HashFinder<
				HashId<DataType::Service::GET_NODE_INFO,       0xEE468A8121C46A9EULL>,
				HashId<DataType::Service::GET_TRANSPORT_STATS, 0xBE6F76A7EC312B04ULL>,
				HashId<DataType::Service::RESTART_NODE,        0x569E05394A3017F0ULL>,
				HashId<DataType::Service::PARAM_GET_SET,       0xA7B622F939D1A4D5ULL>,
				HashId<DataType::Service::GET_FIELDS_SUMMARY,  0x2583CECACCA99058ULL>,
				HashId<DataType::Service::GET_FIELD_INFO,      0xEA771CFDBB20F747ULL>,
				HashId<DataType::Service::FIELD_READ,          0x2E94C3535FDA9115ULL>,
				HashId<DataType::Service::FIELD_WRITE,         0x0F77ADF1898B26C9ULL>,
				HashId<DataType::Service::GET_FILE_INFO,       0xB1C21238E6C7883AULL>,
				HashId<DataType::Service::FILE_READ,           0x4CE2161B8D089C57ULL>,
				HashId<DataType::Service::FILE_WRITE,          0x0666D4CB855D3403ULL>
			>;
		// clang-format on

		if (aType != CanardTransferTypeBroadcast) {
			const auto result = ServiceFinder::findHashById(aTransferId);

			if (result != 0) {
				return result;
			}
		} else {
			const auto result = MessageFinder::findHashById(aTransferId);

			if (result != 0) {
				return result;
			}
		}

		assert(hashFinder != nullptr);
		return hashFinder(aType, aTransferId);
	}

private:
	BusType &bus;
	CanardInstance canard;
	std::array<CanDevice *, maxNodeCount> nodes;
	MutexType mutex;

	uint64_t (*hashFinder)(CanardTransferType, uint16_t);
	microseconds nextCleanupTime;

	void cleanupStaleTransfers(microseconds aCurrentTime)
	{
		mutex.lock();
		canardCleanupStaleTransfers(&canard, static_cast<uint64_t>(aCurrentTime.count()));
		mutex.unlock();
	}

	void enqueuePackets()
	{
		const CanardCANFrame *frame;

		mutex.lock();
		while ((frame = canardPeekTxQueue(&canard)) != nullptr) {
			CanMessage message; // TODO Use sole type for all packets
			message.id = frame->id;
			message.flags = CanMessage::EXT;
			message.length = frame->data_len;
			memcpy(message.data, frame->data, frame->data_len);
			bus.write(&message, 1);

			canardPopTxQueue(&canard);
		}
		mutex.unlock();
	}
};

template<typename Platform, size_t maxNodeCount>
constexpr std::chrono::seconds UavCanHandler<Platform, maxNodeCount>::kDefaultCleanupInterval;

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_UAVCANHANDLER_HPP_
