//
// CanProxy.hpp
//
//  Created on: Jun 26, 2019
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_CANPROXY_HPP_
#define DRONEDEVICE_PLAZCAN_CANPROXY_HPP_

#include <DroneDevice/List.hpp>
#include <DroneDevice/PlazCan/CanDevice.hpp>
#include <DroneDevice/PlazCan/CanHelpers.hpp>
#include <DroneDevice/PlazCan/CanProxyFeature.hpp>
#include <DroneDevice/PlazCan/PlazCanPacket.hpp>
#include <DroneDevice/PlazCan/ProxyRequestDescriptor.hpp>
#include <DroneDevice/PlazCan/UavCanPacket.hpp>
#include <DroneDevice/RequestPool.hpp>

namespace PlazCan {

template<typename Master, size_t maxRequestCount, size_t maxFieldCount>
class CanProxy: public Device::AbstractDevice, public CanDevice {
	static_assert(sizeof(Device::DeviceHash) == sizeof(UidType), "Incorrect type");
	static_assert(sizeof(Device::DeviceHash) == kUidLength, "Incorrect type");

	using microseconds = std::chrono::microseconds;
	using seconds = std::chrono::seconds;

	using HandlerType = typename Master::HandlerType;
	using PlatformType = typename HandlerType::PlatformType;
	using MutexType = typename PlatformType::MutexType;
	using TimeType = typename PlatformType::TimeType;

public:
	CanProxy() :
		master{nullptr},
		feature{nullptr},
		mutex{},
		nextRequestTime{microseconds::max()},
		retries{0},
		info{}
	{
	}

	CanProxy(Master *aMaster, Device::DeviceId aAddress, Status aStatus = {Mode::INITIALIZATION, Health::HEALTHY, 0}) :
		master{aMaster},
		feature{nullptr},
		mutex{},
		nextRequestTime{TimeType::microseconds()},
		retries{kMaxRetries},
		info{TimeType::microseconds(), aAddress, aStatus}
	{
	}

	CanProxy(const CanProxy &aProxy) = delete;

	CanProxy &operator=(const CanProxy &aProxy)
	{
		purgeRequestDescriptors();
		delete feature;

		// TODO Rewrite
		master = aProxy.master;
		feature = aProxy.feature;
		nextRequestTime = TimeType::microseconds();
		retries = kMaxRetries;
		info = aProxy.info;
		state = State::REQUEST_NODE_INFO;
		numbers = Numbers{};

		return *this;
	}

	~CanProxy() override
	{
		purgeRequestDescriptors();
		delete feature;
	}

	// Synchronous functions inherited from Device::AbstractDevice

	Device::DeviceHash deviceHash() const override
	{
		return info.uid;
	}

	const char *deviceName() const override
	{
		return info.name.data();
	}

	Device::Version deviceVersion() const override
	{
		return info.version;
	}

	size_t getFieldCount() override
	{
		return info.fields + (feature != nullptr ? feature->getFieldCount() : 0);
	}

	// Asynchronous field functions inherited from Device::AbstractDevice

	void fieldRequestIndex(const char */*aField*/, Device::DeviceObserver */*aObserver*/,
		Device::RefCounter */*aToken*/) override
	{
		// TODO
	}

	void fieldRequestInfo(Device::FieldId aField, Device::DeviceObserver *aObserver,
		Device::RefCounter *aToken) override
	{
		auto result = Device::Result::SUCCESS;

		if (aField < info.fields) {
			if (master != nullptr) {
				mutex.lock();

				auto * const request = pool.alloc(aObserver, aToken, TimeType::microseconds() + Master::kRequestTimeout,
					static_cast<uint16_t>(DataType::Service::GET_FIELD_INFO), numbers.getFieldInfo, info.address,
					aField);

				if (request != nullptr) {
					const GetFieldInfoRequest payload{static_cast<uint8_t>(aField)};

					requests.pushBack(request);
					master->sendCanServiceRequest(info.address, DataType::Service::GET_FIELD_INFO,
						&numbers.getFieldInfo, &payload, sizeof(payload));
				} else {
					result = Device::Result::QUEUE_ERROR;
				}

				mutex.unlock();
			} else {
				result = Device::Result::COMMAND_ERROR;
			}
		} else if (feature != nullptr) {
			aField = static_cast<Device::FieldId>(aField - info.fields);
			feature->fieldRequestInfo(aField, aObserver, aToken);
		} else {
			result = Device::Result::FIELD_NOT_FOUND;
		}

		if (result != Device::Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFieldInfoRequestError(aField, result, aToken);
		}
	}

	void fieldRead(Device::FieldId aField, Device::DeviceObserver *aObserver, Device::RefCounter *aToken) override
	{
		auto result = Device::Result::SUCCESS;

		if (aField < info.fields) {
			if (master != nullptr) {
				mutex.lock();

				auto * const request = pool.alloc(aObserver, aToken, TimeType::microseconds() + Master::kRequestTimeout,
					static_cast<uint16_t>(DataType::Service::FIELD_READ), numbers.fieldRead, info.address, aField);

				if (request != nullptr) {
					const FieldReadRequest payload{static_cast<uint8_t>(aField)};

					requests.pushBack(request);
					master->sendCanServiceRequest(info.address, DataType::Service::FIELD_READ,
						&numbers.fieldRead, &payload, sizeof(payload));
				} else {
					result = Device::Result::QUEUE_ERROR;
				}

				mutex.unlock();
			} else {
				result = Device::Result::COMMAND_ERROR;
			}
		} else if (feature != nullptr) {
			aField = static_cast<Device::FieldId>(aField - info.fields);
			feature->fieldRead(aField, aObserver, aToken);
		} else {
			result = Device::Result::FIELD_NOT_FOUND;
		}

		if (result != Device::Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFieldRequestError(aField, result, aToken);
		}
	}

	void fieldWrite(Device::FieldId aField, const void *aBuffer, Device::DeviceObserver *aObserver,
		Device::RefCounter *aToken) override
	{
		auto result = Device::Result::SUCCESS;

		if (aField < info.fields) {
			if (master != nullptr) {
				mutex.lock();

				auto * const request = pool.alloc(aObserver, aToken, TimeType::microseconds() + Master::kRequestTimeout,
					static_cast<uint16_t>(DataType::Service::FIELD_WRITE), numbers.fieldWrite, info.address, aField);

				if (request != nullptr) {
					const auto fieldSize = sizeOfFieldType(info.types[aField]);
					const FieldWriteRequest<Device::kFieldMaxSize> payload{static_cast<uint8_t>(aField), aBuffer,
						fieldSize};

					requests.pushBack(request);
					master->sendCanServiceRequest(info.address, DataType::Service::FIELD_WRITE,
						&numbers.fieldWrite, &payload, sizeof(FieldWriteRequest<0>) + fieldSize);
				} else {
					result = Device::Result::QUEUE_ERROR;
				}

				mutex.unlock();
			} else {
				result = Device::Result::COMMAND_ERROR;
			}
		} else if (feature != nullptr) {
			aField = static_cast<Device::FieldId>(aField - info.fields);
			feature->fieldWrite(aField, aBuffer, aObserver, aToken);
		} else {
			result = Device::Result::FIELD_NOT_FOUND;
		}

		if (result != Device::Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFieldRequestError(aField, result, aToken);
		}
	}

	// Asynchronous file functions inherited from Device::AbstractDevice

	void fileRequestInfo(Device::FileId aFile, Device::FileFlags aFlags, Device::DeviceObserver *aObserver,
		Device::RefCounter *aToken) override
	{
		auto result = Device::Result::SUCCESS;

		if (master != nullptr) {
			mutex.lock();

			auto * const request = pool.alloc(aObserver, aToken, TimeType::microseconds() + Master::kRequestTimeout,
				static_cast<uint16_t>(DataType::Service::GET_FILE_INFO), numbers.getFileInfo, info.address, aFile, 0U);

			if (request != nullptr) {
				const GetFileInfoRequest payload{static_cast<uint8_t>(aFile), static_cast<uint8_t>(aFlags)};

				requests.pushBack(request);
				master->sendCanServiceRequest(info.address, DataType::Service::GET_FILE_INFO,
					&numbers.getFileInfo, &payload, sizeof(payload));
			} else {
				result = Device::Result::QUEUE_ERROR;
			}

			mutex.unlock();
		} else {
			result = Device::Result::COMMAND_ERROR;
		}

		if (result != Device::Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFileInfoRequestError(aFile, result, aToken);
		}
	}

	void fileRead(Device::FileId aFile, uint32_t aOffset, size_t aSize,
		Device::DeviceObserver *aObserver, Device::RefCounter *aToken) override
	{
		auto result = Device::Result::SUCCESS;

		if (master != nullptr) {
			mutex.lock();

			auto * const request = pool.alloc(aObserver, aToken, TimeType::microseconds() + Master::kRequestTimeout,
				static_cast<uint16_t>(DataType::Service::FILE_READ), numbers.fileRead, info.address, aFile, aOffset);

			if (request != nullptr) {
				const FileReadRequest payload{static_cast<uint8_t>(aFile), aOffset, static_cast<uint8_t>(aSize)};

				requests.pushBack(request);
				master->sendCanServiceRequest(info.address, DataType::Service::FILE_READ,
					&numbers.fileRead, &payload, sizeof(payload));
			} else {
				result = Device::Result::QUEUE_ERROR;
			}

			mutex.unlock();
		} else {
			result = Device::Result::COMMAND_ERROR;
		}

		if (result != Device::Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFileReadEnd(aFile, aOffset, nullptr, 0, result, aToken);
		}
	}

	void fileWrite(Device::FileId aFile, uint32_t aOffset, const void *aBuffer, size_t aSize,
		Device::DeviceObserver *aObserver, Device::RefCounter *aToken) override
	{
		auto result = Device::Result::SUCCESS;

		if (master != nullptr) {
			mutex.lock();

			auto * const request = pool.alloc(aObserver, aToken, TimeType::microseconds() + Master::kRequestTimeout,
				static_cast<uint16_t>(DataType::Service::FILE_WRITE), numbers.fileWrite, info.address, aFile, aOffset);

			if (request != nullptr) {
				const FileWriteRequest<Device::kFileMaxChunkLength> payload{
					static_cast<uint8_t>(aFile), aOffset, aBuffer, aSize};

				requests.pushBack(request);
				master->sendCanServiceRequest(info.address, DataType::Service::FILE_WRITE,
					&numbers.fileWrite, &payload, sizeof(FileWriteRequest<0>) + aSize);
			} else {
				result = Device::Result::QUEUE_ERROR;
			}

			mutex.unlock();
		} else {
			result = Device::Result::COMMAND_ERROR;
		}

		if (result != Device::Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFileWriteEnd(aFile, aOffset, result, aToken);
		}
	}

	// Inherited from CanDevice

	Device::DeviceId getBusAddress() const override
	{
		return info.address;
	}

	Status getStatus() const override
	{
		return info.status;
	}

	UidType getUID() const override
	{
		return info.uid;
	}

	bool onAcceptanceRequest(uint16_t aDataTypeId, CanardTransferType aTransferType,
		Device::DeviceId aSourceNodeId, Device::DeviceId aDestinationNodeId) override
	{
		assert(isValid());

		if (aSourceNodeId != info.address) {
			return false;
		}

		// Accept only a limited set of messages by default
		static const DataType::Message acceptedMessages[] = {
			DataType::Message::NODE_STATUS,
			DataType::Message::COMPOSITE_FIELD_VALUES
		};
		static const DataType::Service acceptedServices[] = {
			DataType::Service::GET_NODE_INFO,
			DataType::Service::GET_FIELD_INFO,
			DataType::Service::FIELD_READ,
			DataType::Service::FIELD_WRITE,
			DataType::Service::GET_FILE_INFO,
			DataType::Service::FILE_READ,
			DataType::Service::FILE_WRITE
		};

		// TODO Use lower_bound/upper_bound, verify in debug mode that the arrays are sorted
		if (aTransferType == CanardTransferTypeBroadcast) {
			for (auto iter = std::cbegin(acceptedMessages); iter != std::cend(acceptedMessages); ++iter) {
				if (*iter == static_cast<DataType::Message>(aDataTypeId)) {
					return true;
				}
			}

			if (feature != nullptr && feature->onAcceptanceRequest(aDataTypeId, aTransferType)) {
				return true;
			}
		} else if (aTransferType == CanardTransferTypeResponse && aDestinationNodeId == master->getBusAddress()) {
			for (auto iter = std::cbegin(acceptedServices); iter != std::cend(acceptedServices); ++iter) {
				if (*iter == static_cast<DataType::Service>(aDataTypeId)) {
					return true;
				}
			}

			if (feature != nullptr && feature->onAcceptanceRequest(aDataTypeId, aTransferType)) {
				return true;
			}
		}

		return false;
	}

	void onMessageReceived(const CanardRxTransfer *aTransfer) override
	{
		assert(isValid());

		if (aTransfer->source_node_id != info.address) {
			return;
		}

		if (aTransfer->transfer_type == CanardTransferTypeResponse
			&& aTransfer->destination_node_id == master->getBusAddress()) {
			processServiceResponse(aTransfer);
		} else if (aTransfer->transfer_type == CanardTransferTypeBroadcast) {
			processMessage(aTransfer);
		}

		if (feature != nullptr) {
			feature->onMessageReceived(aTransfer);
		}
	}

	microseconds onTimeoutOccurred() override
	{
		assert(isValid());

		// Remove timed out descriptors
		const auto currentTime = TimeType::microseconds();
		auto nextWakeTime = purgeTimedOutDescriptors(currentTime);

		// Handle node disabling
		if (currentTime >= info.timestamp + Master::kOfflineTimeout && info.status.mode != Mode::OFFLINE) {
			info.status.mode = Mode::OFFLINE;

			if (feature != nullptr) {
				feature->onNodeStatusReceived(info.status, info.uptime);
			}
		}

		// Handle device initialization logic
		if ((state != State::IDLE && state != State::ERROR) && this == master->getNextUninitializedNode()
			&& currentTime >= nextRequestTime) {
			switch (state) {
				case State::REQUEST_NODE_INFO: {
					if (retries > 0) {
						--retries;
						sendGetNodeInfo();
					} else {
						state = State::ERROR;
					}
					break;
				}
				case State::REQUEST_FIELDS: {
					if (retries > 0) {
						--retries;
						sendGetFieldInfo(info.fields);
					} else {
						// Initialization is incomplete, just forward the device
						state = State::IDLE;
						master->onDeviceReady(this);
					}
					break;
				}
				default:
					break;
			}

			nextRequestTime = currentTime + Master::kRequestTimeout;
			if (nextRequestTime < nextWakeTime) {
				nextWakeTime = nextRequestTime;
			}
		}

		// Handle special node features
		if (feature != nullptr) {
			const auto nextFeatureTime = feature->onTimeoutOccurred();

			if (nextFeatureTime < nextWakeTime) {
				nextWakeTime = nextFeatureTime;
			}
		}

		return nextWakeTime;
	}

	// Custom functions

	microseconds getLastStatusTime() const
	{
		return info.timestamp;
	}

	seconds getUptime() const
	{
		return std::chrono::duration_cast<seconds>(info.uptime);
	}

	bool isReady() const
	{
		return state == State::IDLE;
	}

	bool isUninitialized() const
	{
		return info.address != kAddressUnallocated
			&& (state == State::REQUEST_NODE_INFO || state == State::REQUEST_FIELDS);
	}

	bool isValid() const
	{
		return info.address != kAddressUnallocated;
	}

	void restart(bool aEnterDfu = false)
	{
		const RestartNodeRequest request{
			aEnterDfu ? RestartNodeMagicNumber::MAGIC_DFU : RestartNodeMagicNumber::MAGIC_RESTART};
		master->sendCanServiceRequest(info.address, DataType::Service::RESTART_NODE, &numbers.restartNode,
			&request, sizeof(request));
	}

	void setFeature(CanProxyFeature *aFeature)
	{
		delete feature;
		feature = aFeature;
	}

	void sendCanMessage(uint16_t aDataTypeId, uint8_t *aTransferId, const void *aData, size_t aLength)
	{
		master->sendCanMessage(aDataTypeId, aTransferId, aData, aLength);
	}

	void sendCanServiceRequest(Device::DeviceId aDestinationNodeId,
		uint16_t aDataTypeId, uint8_t *aTransferId, const void *aData, size_t aLength)
	{
		master->sendCanServiceRequest(aDestinationNodeId, aDataTypeId, aTransferId, aData, aLength);
	}

private:
	static constexpr unsigned int kMaxRetries{8};

	Master *master;
	CanProxyFeature *feature;
	MutexType mutex;
	microseconds nextRequestTime;
	unsigned int retries;

	struct Info {
		UidType uid{};
		Device::Version version{};
		Status status;
		microseconds timestamp;
		microseconds uptime{0};
		std::array<char, kMaxNameLength> name{0};
		std::array<Device::FieldType, maxFieldCount> types;
		Device::DeviceId address;
		Device::FieldId fields{0};

		Info() :
			status{Mode::INITIALIZATION, Health::HEALTHY, 0},
			timestamp{0},
			address{kAddressUnallocated}
		{
		}

		Info(microseconds aTimestamp, Device::DeviceId aAddress, Status aStatus) :
			status{aStatus},
			timestamp{aTimestamp},
			address{aAddress}
		{
		}
	} info;

	enum class CallbackResult: uint8_t {
		COMPONENT_DETACHED,
		GENERIC_TIMEOUT
	};

	enum class State: uint8_t {
		REQUEST_NODE_INFO,
		REQUEST_FIELDS,
		IDLE,
		ERROR
	} state{State::REQUEST_NODE_INFO};

	struct Numbers {
		uint8_t getNodeInfo{0};
		uint8_t getFieldInfo{0};
		uint8_t fieldRead{0};
		uint8_t fieldWrite{0};
		uint8_t getFileInfo{0};
		uint8_t fileRead{0};
		uint8_t fileWrite{0};
		uint8_t restartNode{0};
	} numbers{};

	Device::RequestPool<ProxyRequestDescriptor, maxRequestCount> pool{};
	List<ProxyRequestDescriptor *, maxRequestCount> requests{};

	ProxyRequestDescriptor *findRequestDescriptor(uint16_t aDataTypeId, uint8_t aTransferId)
	{
		mutex.lock();
		auto iter = requests.front();

		while (iter != nullptr) {
			if ((**iter)->type == aDataTypeId && (**iter)->number == aTransferId) {
				break;
			}

			iter = iter->next;
		}

		mutex.unlock();
		return iter != nullptr ? **iter : nullptr;
	}

	void invokeObserverCallback(const ProxyRequestDescriptor *aDescriptor, CallbackResult aResult)
	{
		if (aDescriptor->observer == nullptr) {
			return;
		}

		switch (static_cast<DataType::Service>(aDescriptor->type)) {
			case DataType::Service::GET_FIELD_INFO: {
				const auto result = (aResult == CallbackResult::COMPONENT_DETACHED) ?
					Device::Result::COMPONENT_NOT_FOUND : Device::Result::FIELD_TIMEOUT;

				aDescriptor->observer->onFieldInfoRequestError(aDescriptor->context.field.id, result,
					aDescriptor->token);
				break;
			}

			case DataType::Service::FIELD_READ:
			case DataType::Service::FIELD_WRITE: {
				const auto result = (aResult == CallbackResult::COMPONENT_DETACHED) ?
					Device::Result::COMPONENT_NOT_FOUND : Device::Result::FIELD_TIMEOUT;

				aDescriptor->observer->onFieldRequestError(aDescriptor->context.field.id, result,
					aDescriptor->token);
				break;
			}

			case DataType::Service::GET_FILE_INFO: {
				const auto result = (aResult == CallbackResult::COMPONENT_DETACHED) ?
					Device::Result::COMPONENT_NOT_FOUND : Device::Result::FILE_TIMEOUT;

				aDescriptor->observer->onFileInfoRequestError(aDescriptor->context.file.id, result,
					aDescriptor->token);
				break;
			}

			case DataType::Service::FILE_READ: {
				const auto result = (aResult == CallbackResult::COMPONENT_DETACHED) ?
					Device::Result::COMPONENT_NOT_FOUND : Device::Result::FILE_TIMEOUT;

				aDescriptor->observer->onFileReadEnd(aDescriptor->context.file.id, 0, nullptr, 0, result,
					aDescriptor->token);
				break;
			}

			case DataType::Service::FILE_WRITE: {
				const auto result = (aResult == CallbackResult::COMPONENT_DETACHED) ?
					Device::Result::COMPONENT_NOT_FOUND : Device::Result::FILE_TIMEOUT;

				aDescriptor->observer->onFileWriteEnd(aDescriptor->context.file.id, 0, result,
					aDescriptor->token);
				break;
			}

			default:
				break;
		}
	}

	void purgeRequestDescriptors()
	{
		while (true) {
			// Pop descriptor from the request queue

			mutex.lock();
			auto iter = requests.front();

			if (iter == nullptr) {
				mutex.unlock();
				break;
			}

			auto descriptor = **iter;

			requests.erase(iter);
			mutex.unlock();

			// Invoke callback

			invokeObserverCallback(descriptor, CallbackResult::COMPONENT_DETACHED);

			// Return descriptor to the pool

			mutex.lock();
			pool.free(descriptor);
			mutex.unlock();
		}
	}

	microseconds purgeTimedOutDescriptors(microseconds aCurrentTime)
	{
		auto nextWakeTime = microseconds::max();

		while (true) {
			// Pop descriptor from the request queue

			mutex.lock();
			auto iter = requests.front();

			if (iter == nullptr) {
				mutex.unlock();
				break;
			}

			auto descriptor = **iter;

			if (descriptor->deadline > aCurrentTime) {
				nextWakeTime = descriptor->deadline;
				mutex.unlock();
				break;
			}

			requests.erase(iter);
			mutex.unlock();

			// Invoke callback

			invokeObserverCallback(descriptor, CallbackResult::GENERIC_TIMEOUT);

			// Return descriptor to the pool

			mutex.lock();
			pool.free(descriptor);
			mutex.unlock();
		}

		return nextWakeTime;
	}

	void sendGetNodeInfo()
	{
		master->sendCanServiceRequest(info.address, DataType::Service::GET_NODE_INFO, &numbers.getNodeInfo, nullptr, 0);
	}

	void sendGetFieldInfo(Device::FieldId aField)
	{
		mutex.lock();

		auto * const request = pool.alloc(nullptr, nullptr, TimeType::microseconds() + Master::kRequestTimeout,
			static_cast<uint16_t>(DataType::Service::GET_FIELD_INFO), numbers.getFieldInfo, info.address, aField);

		if (request != nullptr) {
			const GetFieldInfoRequest payload{static_cast<uint8_t>(aField)};

			requests.pushBack(request);
			master->sendCanServiceRequest(info.address, DataType::Service::GET_FIELD_INFO,
				&numbers.getFieldInfo, &payload, sizeof(payload));
		}

		mutex.unlock();
	}

	void processMessage(const CanardRxTransfer *aTransfer)
	{
		switch (aTransfer->data_type_id) {
			case DataType::Message::NODE_STATUS: {
				processNodeStatus(aTransfer);
				break;
			}

			case DataType::Message::COMPOSITE_FIELD_VALUES: {
				processCompositeFieldValues(aTransfer);
				break;
			}

			default:
				break;
		}
	}

	void processServiceResponse(const CanardRxTransfer *aTransfer)
	{
		auto request = findRequestDescriptor(aTransfer->data_type_id, aTransfer->transfer_id);

		switch (aTransfer->data_type_id) {
			case DataType::Service::GET_NODE_INFO: {
				if (state == State::REQUEST_NODE_INFO && processGetNodeInfoResponse(aTransfer)) {

					if (info.types.size() > 0) {
						state = State::REQUEST_FIELDS;

						retries = kMaxRetries;
						sendGetFieldInfo(0);
					} else {
						state = State::IDLE;
						master->onDeviceReady(this);
					}
				}
				break;
			}

			case DataType::Service::GET_FIELD_INFO: {
				processGetFieldInfoResponse(aTransfer, request);
				break;
			}

			case DataType::Service::FIELD_READ: {
				processFieldReadResponse(aTransfer, request);
				break;
			}

			case DataType::Service::FIELD_WRITE: {
				processFieldWriteResponse(aTransfer, request);
				break;
			}

			case DataType::Service::GET_FILE_INFO: {
				processGetFileInfoResponse(aTransfer, request);
				break;
			}

			case DataType::Service::FILE_READ: {
				processFileReadResponse(aTransfer, request);
				break;
			}

			case DataType::Service::FILE_WRITE: {
				processFileWriteResponse(aTransfer, request);
				break;
			}

			default:
				break;
		}

		if (request != nullptr) {
			mutex.lock();
			requests.erase(request);
			pool.free(request);
			mutex.unlock();
		}
	}

	bool processCompositeFieldValues(const CanardRxTransfer *aTransfer)
	{
		if (state != State::IDLE) {
			return false; // Device is not ready
		}
		if (feature == nullptr) {
			return true; // Meaningless processing
		}

		size_t count{0};
		size_t position;

		// Pre-process message and compute field count
		position = 0;
		while (position < aTransfer->payload_len) {
			const auto number = CanHelpers::decodeIntegerField<uint8_t>(aTransfer, position);

			if (number < info.fields) {
				++count;
				position += 1 + Device::sizeOfFieldType(info.types[number]);
			} else {
				break; // Unknown field
			}
		}

		// Parse fields and invoke feature callback
		position = 0;
		for (size_t i = 0; i < count; ++i) {
			const auto number = static_cast<Device::FieldId>(CanHelpers::decodeIntegerField<uint8_t>(aTransfer,
				position));
			const auto size = Device::sizeOfFieldType(info.types[number]);
			uint8_t buffer[Device::kFieldMaxSize];

			CanHelpers::decodeBlobField(aTransfer, position + 1, size, buffer);
			feature->onScheduledFieldReceived(number, info.types[number], 1, buffer, i == 0, i == (count - 1));
			position += 1 + size;
		}

		return true;
	}

	bool processGetNodeInfoResponse(const CanardRxTransfer *aTransfer)
	{
		enum class PacketPart {
			STATUS,
			VERSION,
			UID,
			COA,
			NAME,
			IDLE
		} part{PacketPart::STATUS};

		size_t position = 0; // Skip result

		while (position < aTransfer->payload_len) {
			switch (part) {
				case PacketPart::STATUS: {
					const auto uptime = CanHelpers::decodeIntegerField<uint32_t>(aTransfer, position);
					info.uptime = seconds{uptime};

					position += 7;
					part = PacketPart::VERSION;
					break;
				}

				case PacketPart::VERSION: {
					info.version.sw.major = CanHelpers::decodeIntegerField<uint8_t>(aTransfer, position);
					info.version.sw.minor = CanHelpers::decodeIntegerField<uint8_t>(aTransfer, position + 1);
					info.version.sw.revision = CanHelpers::decodeIntegerField<uint32_t>(aTransfer, position + 3);
					info.version.hw.major = CanHelpers::decodeIntegerField<uint8_t>(aTransfer, position + 15);
					info.version.hw.minor = CanHelpers::decodeIntegerField<uint8_t>(aTransfer, position + 16);

					position += 17;
					part = PacketPart::UID;
					break;
				}

				case PacketPart::UID: {
					CanHelpers::decodeBlobField(aTransfer, position, kUidLength, info.uid.data());

					position += kUidLength;
					part = PacketPart::COA;
					break;
				}

				case PacketPart::COA: {
					// Skip
					const auto length = CanHelpers::decodeIntegerField<uint8_t>(aTransfer, position);

					position += 1 + length;
					part = PacketPart::NAME;
					break;
				}

				case PacketPart::NAME: {
					const size_t length = std::min(aTransfer->payload_len - position, sizeof(info.name));
					CanHelpers::decodeBlobField(aTransfer, position, length, info.name.data());

					position = aTransfer->payload_len;
					part = PacketPart::IDLE;
					break;
				}

				default:
					break;
			}
		}

		if (part == PacketPart::IDLE) {
			info.timestamp = TimeType::microseconds();
			return true;
		} else {
			return false;
		}
	}

	void processGetFieldInfoResponse(const CanardRxTransfer *aTransfer, const ProxyRequestDescriptor *aDescriptor)
	{
		using LargestResponse = GetFieldInfoResponse<Device::kFieldMaxSize, CanHelpers::kMaxUnitNameLength,
			CanHelpers::kMaxFieldNameLength>;
		using SmallestResponse = GetFieldInfoResponse<0, 0, 0>;

		if (aDescriptor == nullptr) {
			// Outdated packet or incorrect field identifier
			return;
		}

		if ((aTransfer->payload_len < sizeof(SmallestResponse) || aTransfer->payload_len > sizeof(LargestResponse))
			&& (aTransfer->payload_len != sizeof(GetFieldInfoResponseError))) {
			if (aDescriptor->observer != nullptr) {
				aDescriptor->observer->onFieldInfoRequestError(aDescriptor->context.field.id,
					Device::Result::COMMAND_ERROR, aDescriptor->token);
			}

			return;
		}

		SmallestResponse packet;
		packet.result = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::result);
		const auto result = static_cast<Device::Result>(packet.result);

		if (result == Device::Result::SUCCESS) {
			packet.type = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::type);

			if (state == State::REQUEST_FIELDS && aDescriptor->context.field.id == info.fields) {
				info.types[info.fields++] = static_cast<Device::FieldType>(packet.type);

				if (info.fields < info.types.size()) {
					retries = kMaxRetries;
					sendGetFieldInfo(info.fields);
				} else {
					// Too much fields, truncate field list and notify the hub
					state = State::IDLE;
					master->onDeviceReady(this);
				}
			}

			if (aDescriptor->observer != nullptr) {
				packet.scale = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::scale);

				// Decode min and max values
				const auto size = Device::sizeOfFieldType(static_cast<Device::FieldType>(packet.type));
				uint8_t minValue[Device::kFieldMaxSize];
				uint8_t maxValue[Device::kFieldMaxSize];

				CanHelpers::decodeBlobField(aTransfer, 4, size, minValue);
				CanHelpers::decodeBlobField(aTransfer, 5 + size, size, maxValue);

				// Decode unit string
				char unit[CanHelpers::kMaxUnitNameLength];
				uint8_t unitLength;

				unitLength = CanHelpers::decodeIntegerField<uint8_t>(aTransfer, 5 + size * 2);
				CanHelpers::decodeBlobField(aTransfer, 6 + size * 2, unitLength, unit);
				unit[unitLength] = '\0';

				// Decode name string
				const size_t nameOffset = 6 + size * 2 + unitLength;
				const size_t nameLength = aTransfer->payload_len - nameOffset;
				char name[CanHelpers::kMaxFieldNameLength];

				CanHelpers::decodeBlobField(aTransfer, nameOffset, nameLength, name);
				name[nameLength] = '\0';

				// Build callback response
				const Device::FieldInfo fieldInfo{
					aDescriptor->context.field.id,
					1,
					Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant,
					packet.scale,
					static_cast<Device::FieldType>(packet.type),
					name,
					unit,
					minValue,
					maxValue
				};

				aDescriptor->observer->onFieldInfoReceived(fieldInfo, nullptr, aDescriptor->token);
			}
		} else {
			if (state == State::REQUEST_FIELDS && result == Device::Result::FIELD_NOT_FOUND) {
				// Reached the end of the field array, notify the hub
				state = State::IDLE;
				master->onDeviceReady(this);
			}

			if (aDescriptor->observer != nullptr) {
				aDescriptor->observer->onFieldInfoRequestError(aDescriptor->context.field.id, result,
					aDescriptor->token);
			}
		}
	}

	void processFieldReadResponse(const CanardRxTransfer *aTransfer, const ProxyRequestDescriptor *aDescriptor)
	{
		using LargestResponse = FieldReadResponse<Device::kFieldMaxSize>;
		using SmallestResponse = FieldReadResponse<0>;

		if (aDescriptor == nullptr || aDescriptor->context.field.id >= info.fields) {
			// Outdated packet or incorrect field identifier
			return;
		}

		if (aTransfer->payload_len < sizeof(SmallestResponse) || aTransfer->payload_len > sizeof(LargestResponse)) {
			// Incorrect low-level packet
			if (aDescriptor->observer != nullptr) {
				aDescriptor->observer->onFieldRequestError(aDescriptor->context.field.id, Device::Result::COMMAND_ERROR,
					aDescriptor->token);
			}

			return;
		}

		if (aDescriptor->observer != nullptr) {
			// TODO Check actual length
			const auto id = static_cast<Device::FieldId>(aDescriptor->context.field.id);
			FieldReadResponse<Device::kFieldMaxSize> packet;
			packet.result = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::result);

			if (static_cast<Device::Result>(packet.result) == Device::Result::SUCCESS) {
				CanHelpers::decodeBlobField(aTransfer, 1, sizeOfFieldType(info.types[id]), packet.data);

				// Dimension is fixed to 1 in field information packet
				aDescriptor->observer->onFieldReceived(id, packet.data, info.types[id], 1, aDescriptor->token);
			} else {
				aDescriptor->observer->onFieldRequestError(id, static_cast<Device::Result>(packet.result),
					aDescriptor->token);
			}
		}
	}

	void processFieldWriteResponse(const CanardRxTransfer *aTransfer, const ProxyRequestDescriptor *aDescriptor)
	{
		if (aDescriptor == nullptr || aDescriptor->context.field.id >= info.fields) {
			// Outdated packet or incorrect field identifier
			return;
		}

		if (aTransfer->payload_len != sizeof(FieldWriteResponse)) {
			// Incorrect low-level packet
			if (aDescriptor->observer != nullptr) {
				aDescriptor->observer->onFieldRequestError(aDescriptor->context.field.id, Device::Result::COMMAND_ERROR,
					aDescriptor->token);
			}

			return;
		}

		if (aDescriptor->observer != nullptr) {
			const auto id = static_cast<Device::FieldId>(aDescriptor->context.field.id);
			FieldWriteResponse packet;
			packet.result = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::result);

			if (static_cast<Device::Result>(packet.result) == Device::Result::SUCCESS) {
				aDescriptor->observer->onFieldUpdated(id, aDescriptor->token);
			} else {
				aDescriptor->observer->onFieldRequestError(id, static_cast<Device::Result>(packet.result),
					aDescriptor->token);
			}
		}
	}

	void processGetFileInfoResponse(const CanardRxTransfer *aTransfer, const ProxyRequestDescriptor *aDescriptor)
	{
		if (aDescriptor == nullptr) {
			// Outdated packet
			return;
		}

		if (aTransfer->payload_len != sizeof(GetFileInfoResponse)
			&& aTransfer->payload_len != sizeof(GetFileInfoResponseError)) {
			// Incorrect low-level packet
			if (aDescriptor->observer != nullptr) {
				aDescriptor->observer->onFileInfoRequestError(aDescriptor->context.file.id,
					Device::Result::COMMAND_ERROR, aDescriptor->token);
			}

			return;
		}

		if (aDescriptor->observer != nullptr) {
			const auto id = static_cast<Device::FileId>(aDescriptor->context.file.id);
			GetFileInfoResponse packet;
			packet.result = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::result);

			if (static_cast<Device::Result>(packet.result) == Device::Result::SUCCESS) {
				packet.status = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::status);
				packet.size = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::size);
				packet.checksum = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::checksum);

				const Device::FileInfo fileInfo{
					id,
					packet.status,
					packet.size,
					packet.checksum
				};
				aDescriptor->observer->onFileInfoReceived(fileInfo, aDescriptor->token);
			} else {
				aDescriptor->observer->onFileInfoRequestError(id, static_cast<Device::Result>(packet.result),
					aDescriptor->token);
			}
		}
	}

	void processFileReadResponse(const CanardRxTransfer *aTransfer, const ProxyRequestDescriptor *aDescriptor)
	{
		using LargestResponse = FileReadResponse<Device::kFileMaxChunkLength>;
		using SmallestResponse = FileReadResponse<0>;

		if (aDescriptor == nullptr) {
			// Outdated packet
			return;
		}

		if (aTransfer->payload_len < sizeof(SmallestResponse) || aTransfer->payload_len > sizeof(LargestResponse)) {
			// Incorrect low-level packet
			if (aDescriptor->observer != nullptr) {
				aDescriptor->observer->onFileReadEnd(aDescriptor->context.file.id, aDescriptor->context.file.position,
					nullptr, 0, Device::Result::COMMAND_ERROR, aDescriptor->token);
			}

			return;
		}

		if (aDescriptor->observer != nullptr) {
			const auto id = static_cast<Device::FileId>(aDescriptor->context.file.id);
			FileReadResponse<Device::kFileMaxChunkLength> packet;
			packet.result = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::result);

			if (static_cast<Device::Result>(packet.result) == Device::Result::SUCCESS) {
				CanHelpers::decodeBlobField(aTransfer, 1, aTransfer->payload_len - 1, packet.data);

				aDescriptor->observer->onFileReadEnd(id, aDescriptor->context.file.position,
					packet.data, aTransfer->payload_len - 1, Device::Result::SUCCESS, aDescriptor->token);
			} else {
				aDescriptor->observer->onFileReadEnd(id, aDescriptor->context.file.position,
					nullptr, 0, static_cast<Device::Result>(packet.result), aDescriptor->token);
			}
		}
	}

	void processFileWriteResponse(const CanardRxTransfer *aTransfer, const ProxyRequestDescriptor *aDescriptor)
	{
		if (aDescriptor == nullptr) {
			// Outdated packet
			return;
		}

		if (aTransfer->payload_len != sizeof(FileWriteResponse)) {
			// Incorrect low-level packet
			if (aDescriptor->observer != nullptr) {
				aDescriptor->observer->onFileWriteEnd(aDescriptor->context.file.id, aDescriptor->context.file.position,
					Device::Result::COMMAND_ERROR, aDescriptor->token);
			}

			return;
		}

		if (aDescriptor->observer) {
			FileWriteResponse packet;
			packet.result = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::result);

			aDescriptor->observer->onFileWriteEnd(aDescriptor->context.file.id, aDescriptor->context.file.position,
				static_cast<Device::Result>(packet.result), aDescriptor->token);
		}
	}

	void processNodeStatus(const CanardRxTransfer *aTransfer)
	{
		if (aTransfer->payload_len == sizeof(NodeStatusMessage)) {
			const auto packet = CanHelpers::decodeAs<NodeStatusMessage>(aTransfer);

			info.status = Status{static_cast<Mode>(packet.mode), static_cast<Health>(packet.health),
				packet.vendor_specific_status_code};
			info.uptime = seconds{packet.uptime_sec};
			info.timestamp = TimeType::microseconds();

			if (feature != nullptr) {
				feature->onNodeStatusReceived(info.status, info.uptime);
			}
		}
	}
};

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_CANPROXY_HPP_
