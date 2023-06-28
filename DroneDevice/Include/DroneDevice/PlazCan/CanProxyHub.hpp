//
// CanProxyHub.hpp
//
//  Created on: Jun 26, 2019
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_CANPROXYHUB_HPP_
#define DRONEDEVICE_PLAZCAN_CANPROXYHUB_HPP_

#include <DroneDevice/PlazCan/CanDevice.hpp>
#include <DroneDevice/PlazCan/CanProxy.hpp>
#include <algorithm>
#include <type_traits>

namespace PlazCan {

class CanProxyHubObserver {
public:
	virtual ~CanProxyHubObserver() = default;
	virtual void onDeviceAttached(const UidType &aUid) = 0;
	virtual void onDeviceDetached(const UidType &aUid) = 0;
};

template<typename Component, size_t maxNodeCount, size_t maxRequestCount = 8, size_t maxFieldCount = 64>
class CanProxyHub : public Component {
	using microseconds = std::chrono::microseconds;
	using milliseconds = std::chrono::milliseconds;

	using Component::handler;
	using CurrentType = CanProxyHub<Component, maxNodeCount, maxRequestCount, maxFieldCount>;

public:
	using HandlerType = std::remove_reference_t<decltype(handler)>;
	using ProxyType = CanProxy<CurrentType, maxRequestCount, maxFieldCount>;

private:
	using PlatformType = typename HandlerType::PlatformType;
	using MutexType = typename PlatformType::MutexType;
	using TimeType = typename PlatformType::TimeType;

	CanProxyHub(const CanProxyHub &) = delete;
	CanProxyHub operator=(const CanProxyHub &) = delete;

public:
	static constexpr Device::DeviceId kFirstAddress{125};
	static constexpr milliseconds kOfflineTimeout{3000};
	static constexpr milliseconds kRequestTimeout{100};

	template<typename... Args>
	CanProxyHub(bool aAllocationEnabled, Args &&...aArgs):
		Component{std::forward<Args>(aArgs)...},
		proxies{},
		allocation{aAllocationEnabled},
		numbers{}
	{
	}

	virtual ~CanProxyHub() = default;

	bool onAcceptanceRequest(uint16_t aDataTypeId, CanardTransferType aTransferType,
		Device::DeviceId aSourceNodeId, Device::DeviceId aDestinationNodeId) override
	{
		static const DataType::Message acceptedMessages[] = {
			DataType::Message::PLAZ_ALLOCATION,
			DataType::Message::ALLOCATION,
			DataType::Message::NODE_STATUS,
		};

		if (aTransferType == CanardTransferTypeBroadcast) {
			for (auto iter = std::cbegin(acceptedMessages); iter != std::cend(acceptedMessages); ++iter) {
				if (*iter == static_cast<DataType::Message>(aDataTypeId)) {
					return true;
				}
			}
		}

		return Component::onAcceptanceRequest(aDataTypeId, aTransferType, aSourceNodeId, aDestinationNodeId);
	}

	void onMessageReceived(const CanardRxTransfer *aTransfer) override
	{
		if (Component::currentAddress != kAddressUnallocated
			&& aTransfer->transfer_type == CanardTransferTypeBroadcast) {
			switch (static_cast<DataType::Message>(aTransfer->data_type_id)) {
				case DataType::Message::PLAZ_ALLOCATION:
				case DataType::Message::ALLOCATION:
					if (allocation.enabled) {
						processAllocationMessage(aTransfer);
					}
					return;

				case DataType::Message::NODE_STATUS:
					processNodeStatus(aTransfer);
					return;

				default:
					break;
			}
		}

		Component::onMessageReceived(aTransfer);
	}

	microseconds onTimeoutOccurred() override
	{
		purgeOfflineNodes();
		return Component::onTimeoutOccurred();
	}

	// TODO Access, make proxy a friend?
	void onDeviceReady(ProxyType *aProxy)
	{
		if (observer != nullptr) {
			observer->onDeviceAttached(aProxy->getUID());
		}
	}

	ProxyType *getNextUninitializedNode()
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[](const auto &aProxy) { return aProxy.isUninitialized(); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	void purge(const UidType &aUid)
	{
		for (auto &entry : proxies) {
			if (entry.getUID() == aUid) {
				if (observer != nullptr) {
					observer->onDeviceDetached(entry.getUID());
				}

				handler.detach(&entry);
				entry = ProxyType{};
				break;
			}
		}
	}

	void purge()
	{
		for (auto &entry : proxies) {
			if (entry.isValid()) {
				if (observer != nullptr) {
					observer->onDeviceDetached(entry.getUID());
				}

				handler.detach(&entry);
				entry = ProxyType{};
			}
		}
	}

	void purgeOfflineNodes()
	{
		if (timeout != milliseconds{0}) {
			const auto minStatusTime = TimeType::milliseconds() - timeout;

			for (auto &entry : proxies) {
				if (entry.isValid() && entry.getLastStatusTime() < minStatusTime) {
					if (observer != nullptr) {
						observer->onDeviceDetached(entry.getUID());
					}

					handler.detach(&entry);
					entry = ProxyType{};
				}
			}
		}
	}

	void setFirstAddress(Device::DeviceId aAddress)
	{
		assert(aAddress <= kFirstAddress);
		allocation.first = aAddress;
	}

	void setPurgeTimeout(milliseconds aTimeout)
	{
		timeout = aTimeout;
	}

	void sendCanMessage(uint16_t aDataTypeId, uint8_t *aTransferId, const void *aData, size_t aLength)
	{
		handler.sendCanMessage(Component::getBusAddress(), aDataTypeId, aTransferId, aData, aLength);
	}

	void sendCanServiceRequest(Device::DeviceId aDestinationNodeId, uint16_t aDataTypeId, uint8_t *aTransferId,
		const void *aData, size_t aLength)
	{
		handler.sendCanServiceRequest(Component::getBusAddress(), aDestinationNodeId, aDataTypeId, aTransferId,
			aData, aLength);
	}

	Device::DeviceId findFreeAddress(Device::DeviceId aPreferredAddress = Device::kDeviceReservedId)
	{
		if (aPreferredAddress != Device::kDeviceReservedId
			&& isAddressFree(aPreferredAddress) && handler.isAddressFree(aPreferredAddress)) {
			return aPreferredAddress;
		}

		auto address{allocation.first};
		bool found{false};

		if (address == Component::getBusAddress()) {
			--address;
		}

		while (!found) {
			found = true;

			if (!isAddressFree(address) || !handler.isAddressFree(address)) {
				--address;
				found = false;
			}
		}

		return found ? address : Device::kDeviceReservedId;
	}

	size_t size() const
	{
		return std::count_if(proxies.begin(), proxies.end(), [](const auto &aProxy) { return aProxy.isReady(); });
	}

	ProxyType *find(std::function<bool (const ProxyType *)> aPredicate)
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[&aPredicate](const auto &aProxy) { return aProxy.isReady() && aPredicate(&aProxy); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	ProxyType *find(std::function<bool (const char *)> aPredicate)
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[&aPredicate](const auto &aProxy) { return aProxy.isReady() && aPredicate(aProxy.deviceName()); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	ProxyType *operator[](const UidType &aUid)
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[aUid](const auto &aProxy) { return aProxy.getUID() == aUid && aProxy.isReady(); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	ProxyType *operator[](const char *aName)
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[aName](const auto &aProxy) { return !strcmp(aName, aProxy.deviceName()) && aProxy.isReady(); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	ProxyType *operator[](size_t aIndex)
	{
		size_t count{0};

		for (auto &entry : proxies) {
			if (entry.isReady() && (count++ == aIndex)) {
				return &entry;
			}
		}

		return nullptr;
	}

	const ProxyType *find(std::function<bool (const ProxyType *)> aPredicate) const
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[&aPredicate](const auto &aProxy) { return aProxy.isReady() && aPredicate(&aProxy); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	const ProxyType *find(std::function<bool (const char *)> aPredicate) const
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[&aPredicate](const auto &aProxy) { return aProxy.isReady() && aPredicate(aProxy.deviceName()); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	const ProxyType *operator[](const UidType &aUid) const
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[aUid](const auto &aProxy) { return aProxy.getUID() == aUid && aProxy.isReady(); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	const ProxyType *operator[](const char *aName) const
	{
		auto iter = std::find_if(proxies.begin(), proxies.end(),
			[aName](const auto &aProxy) { return !strcmp(aName, aProxy.deviceName()) && aProxy.isReady(); });

		return iter != proxies.end() ? &(*iter) : nullptr;
	}

	const ProxyType *operator[](size_t aIndex) const
	{
		size_t count{0};

		for (auto &entry : proxies) {
			if (entry.isReady() && (count++ == aIndex)) {
				return &entry;
			}
		}

		return nullptr;
	}

	void subscribe(CanProxyHubObserver *aObserver)
	{
		assert(aObserver != nullptr);
		observer = aObserver;
	}

	void unsubscribe()
	{
		observer = nullptr;
	}

private:
	std::array<ProxyType, maxNodeCount> proxies;
	CanProxyHubObserver *observer{nullptr};
	milliseconds timeout{0};

	struct Allocation {
		UidType uid{};
		microseconds timestamp{};
		Device::DeviceId first{kFirstAddress};
		bool enabled;
		bool ongoing{false};

		constexpr Allocation(bool aEnabled):
			enabled{aEnabled}
		{
		}
	} allocation;

	struct Numbers {
		uint8_t allocation{0};
	} numbers;

	bool isAddressFree(Device::DeviceId aAddress) const
	{
		for (auto &entry : proxies) {
			if (entry.getBusAddress() == aAddress) {
				return false;
			}
		}

		return true;
	}

	bool processAllocationMessage(const CanardRxTransfer *aTransfer)
	{
		static constexpr milliseconds kAllocationTimeout{400};

		enum class Phase {
			PHASE_0,
			PHASE_1,
			PHASE_2,
		} phase;

		if (aTransfer->payload_len != 7 && aTransfer->payload_len != 5) {
			return false; // Incorrect message
		}

		const bool uavCanAlloc = aTransfer->data_type_id == DataType::Message::ALLOCATION;

		uint8_t payload[7];
		CanHelpers::decodeBlobField(aTransfer, 0, aTransfer->payload_len, payload);

		const Device::DeviceId preferredAddress = static_cast<Device::DeviceId>((payload[0] & 0xFE) >> 1);
		const bool firstPart = uavCanAlloc ? (payload[0] & 0x01) != 0 : (payload[0] & 0x80) != 0;
		const auto currentTime = TimeType::microseconds();

		if (firstPart) {
			if (allocation.ongoing) {
				allocation.ongoing = false;

				if (currentTime - allocation.timestamp < kAllocationTimeout) {
					return false;
				}
			}

			allocation.timestamp = currentTime;
			allocation.ongoing = true;
			std::fill(allocation.uid.begin(), allocation.uid.end(), 0);
			std::copy_n(payload + 1, 6, allocation.uid.data());
			phase = Phase::PHASE_0;
		} else {
			if (!allocation.ongoing || currentTime - allocation.timestamp >= kAllocationTimeout) {
				allocation.ongoing = false;
				return false; // Incorrect sequence
			} else {
				if (aTransfer->payload_len == 7) {
					std::copy_n(payload + 1, 6, allocation.uid.data() + 6);
					phase = Phase::PHASE_1;
				} else {
					std::copy_n(payload + 1, 4, allocation.uid.data() + 12);
					phase = Phase::PHASE_2;
				}
			}
		}

		if (allocation.ongoing) {
			Device::DeviceId address;
			size_t length;

			switch (phase) {
				case Phase::PHASE_0: {
					address = preferredAddress;
					length = 7;
					break;
				}
				case Phase::PHASE_1: {
					address = preferredAddress;
					length = 13;
					break;
				}
				case Phase::PHASE_2: {
					allocation.ongoing = false;
					address = attachDevice(allocation.uid);
					length = 17;
					break;
				}
				default:
					address = Device::kDeviceReservedId;
					break;
			}

			if (address != Device::kDeviceReservedId) {
				uint8_t response[1 + sizeof(allocation.uid)];

				response[0] = uavCanAlloc ? static_cast<uint8_t>(address << 1) : address;
				std::copy_n(allocation.uid.data(), sizeof(allocation.uid), response + 1);

				handler.sendCanMessage(Component::getBusAddress(),
					uavCanAlloc ? DataType::Message::ALLOCATION : DataType::Message::PLAZ_ALLOCATION,
					&numbers.allocation, response, length);
			}
		}

		return true;
	}

	void processNodeStatus(const CanardRxTransfer *aTransfer)
	{
		bool found{false};

		for (auto &entry : proxies) {
			if (entry.getBusAddress() == aTransfer->source_node_id) {
				found = true;
				break;
			}
		}

		if (!found) {
			for (auto &entry : proxies) {
				if (!entry.isValid()) {
					const auto packet = CanHelpers::decodeAs<NodeStatusMessage>(aTransfer);

					entry = ProxyType{
						this,
						aTransfer->source_node_id,
						Status{
							static_cast<Mode>(packet.mode),
							static_cast<Health>(packet.health),
							packet.vendor_specific_status_code,
						},
					};
					handler.attach(&entry);
					break;
				}
			}
		}
	}

	Device::DeviceId attachDevice(const UidType &aUid)
	{
		for (auto &entry : proxies) {
			if (entry.isValid() && entry.getUID() == aUid) {
				return entry.getBusAddress();
			}
		}

		Device::DeviceId address = findFreeAddress();

		if (address != Device::kDeviceReservedId) {
			for (auto &entry : proxies) {
				if (!entry.isValid()) {
					entry = ProxyType{this, address};
					handler.attach(&entry);
					break;
				}
			}
		}

		return address;
	}
};

template<typename Component, size_t maxNodeCount, size_t maxRequestCount, size_t maxFieldCount>
constexpr std::chrono::milliseconds
	CanProxyHub<Component, maxNodeCount, maxRequestCount, maxFieldCount>::kOfflineTimeout;

template<typename Component, size_t maxNodeCount, size_t maxRequestCount, size_t maxFieldCount>
constexpr std::chrono::milliseconds
	CanProxyHub<Component, maxNodeCount, maxRequestCount, maxFieldCount>::kRequestTimeout;

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_CANPROXYHUB_HPP_
