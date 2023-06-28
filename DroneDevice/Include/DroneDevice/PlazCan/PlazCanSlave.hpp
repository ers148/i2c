//
// PlazCanSlave.hpp
//
//  Created on: Feb 5, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_PLAZCANSLAVE_HPP_
#define DRONEDEVICE_PLAZCAN_PLAZCANSLAVE_HPP_

#include <DroneDevice/DeviceObserver.hpp>
#include <DroneDevice/RequestPool.hpp>
#include <DroneDevice/PlazCan/CanHelpers.hpp>
#include <DroneDevice/PlazCan/DeviceRequestDescriptor.hpp>
#include <DroneDevice/PlazCan/PlazCanPacket.hpp>
#include <cstring>

namespace PlazCan {

template<typename T>
class PlazCanSlave : public Device::DeviceObserver {
	PlazCanSlave(const PlazCanSlave &) = delete;
	PlazCanSlave operator=(const PlazCanSlave &) = delete;

public:
	PlazCanSlave(T &aParent):
		parent{aParent},
		pool{}
	{
	}

	bool process(const CanardRxTransfer *aTransfer)
	{
		switch (aTransfer->data_type_id) {
			case DataType::Service::GET_FIELDS_SUMMARY: {
				const GetFieldsSummaryResponse response{static_cast<uint8_t>(parent.device.getFieldCount())};
				parent.handler.sendCanServiceResponse(parent.currentAddress, aTransfer->source_node_id,
					static_cast<DataType::Service>(aTransfer->data_type_id), aTransfer->transfer_id,
					&response, sizeof(response));
				return true;
			}

			case DataType::Service::FIELD_WRITE: {
				static constexpr size_t minLength = sizeof(FieldWriteRequest<1>);
				static constexpr size_t maxLength = sizeof(FieldWriteRequest<Device::kFieldMaxSize>);

				if (aTransfer->payload_len >= minLength && aTransfer->payload_len <= maxLength) {
					auto *const request = pool.alloc(aTransfer->data_type_id, aTransfer->transfer_id,
						aTransfer->source_node_id, aTransfer->destination_node_id);

					if (request != nullptr) {
						const size_t dataLength = aTransfer->payload_len - sizeof(FieldWriteRequest<0>);
						FieldWriteRequest<Device::kFieldMaxSize> packet;

						packet.field = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::field);
						CanHelpers::decodeBlobField(aTransfer, CanHelpers::packetMemberOffset(&decltype(packet)::data),
							dataLength, packet.data);

						parent.device.fieldWrite(packet.field, packet.data, this, request);
					}
				}
				return true;
			}

			case DataType::Service::FIELD_READ: {
				if (aTransfer->payload_len == sizeof(GetFieldInfoRequest)) {
					auto *const request = pool.alloc(aTransfer->data_type_id, aTransfer->transfer_id,
						aTransfer->source_node_id, aTransfer->destination_node_id);

					if (request != nullptr) {
						FieldReadRequest packet;
						packet.field = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::field);
						parent.device.fieldRead(packet.field, this, request);
					}
				}
				return true;
			}

			case DataType::Service::GET_FIELD_INFO: {
				if (aTransfer->payload_len == sizeof(GetFieldInfoRequest)) {
					auto *const request = pool.alloc(aTransfer->data_type_id, aTransfer->transfer_id,
						aTransfer->source_node_id, aTransfer->destination_node_id);

					if (request != nullptr) {
						GetFieldInfoRequest packet;
						packet.field = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::field);
						parent.device.fieldRequestInfo(packet.field, this, request);
					}
				}
				return true;
			}

			case DataType::Service::GET_FILE_INFO: {
				if (aTransfer->payload_len == sizeof(GetFileInfoRequest)) {
					auto *const request = pool.alloc(aTransfer->data_type_id, aTransfer->transfer_id,
						aTransfer->source_node_id, aTransfer->destination_node_id);

					if (request != nullptr) {
						GetFileInfoRequest packet;

						packet.file = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::file);
						packet.flags = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::flags);

						parent.device.fileRequestInfo(packet.file, packet.flags, this, request);
					}
				}
				return true;
			}

			case DataType::Service::FILE_WRITE: {
				using FileWriteRequestMin = FileWriteRequest<0>;
				using FileWriteRequestMax = FileWriteRequest<Device::kFileMaxChunkLength>;

				if (aTransfer->payload_len >= sizeof(FileWriteRequestMin)
					&& aTransfer->payload_len <= sizeof(FileWriteRequestMax)) {
					auto *const request = pool.alloc(aTransfer->data_type_id, aTransfer->transfer_id,
						aTransfer->source_node_id, aTransfer->destination_node_id);

					if (request != nullptr) {
						const size_t dataLength = aTransfer->payload_len - sizeof(FileWriteRequestMin);
						uint8_t buffer[Device::kFileMaxChunkLength];
						FileWriteRequestMin packet;

						packet.file = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::file);
						packet.position = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::position);
						CanHelpers::decodeBlobField(aTransfer,
							CanHelpers::packetMemberOffset(&FileWriteRequestMax::data), dataLength, buffer);

						parent.device.fileWrite(packet.file, packet.position, buffer, dataLength, this, request);
					}
				}
				return true;
			}

			case DataType::Service::FILE_READ: {
				if (aTransfer->payload_len == sizeof(FileReadRequest)) {
					auto *const request = pool.alloc(aTransfer->data_type_id, aTransfer->transfer_id,
						aTransfer->source_node_id, aTransfer->destination_node_id);

					if (request != nullptr) {
						FileReadRequest packet;

						packet.file = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::file);
						packet.position = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::position);
						packet.length = CanHelpers::decodeIntegerField(aTransfer, &decltype(packet)::length);

						parent.device.fileRead(packet.file, packet.position, packet.length, this, request);
					}
				}
				return true;
			}

			default:
				return false;
		}
	}

	// Fields
	void onFieldInfoReceived(const Device::FieldInfo &aInfo, const void * /*aValue*/,
		Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);
		GetFieldInfoResponse<Device::kFieldMaxSize, CanHelpers::kMaxUnitNameLength,
			CanHelpers::kMaxFieldNameLength>
			response;
		const size_t actualArenaSize = CanHelpers::packFieldInfo(response.arena, aInfo.type, aInfo.name,
			aInfo.unit, aInfo.min, aInfo.max);

		if (actualArenaSize != 0) {
			response.result = static_cast<uint8_t>(Device::Result::SUCCESS);
			response.type = static_cast<uint8_t>(aInfo.type);
			response.scale = static_cast<int8_t>(aInfo.scale);

			parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
				static_cast<DataType::Service>(request->type), request->number, &response,
				sizeof(response) - sizeof(response.arena) + actualArenaSize);
		} else {
			const GetFieldInfoResponseError error{static_cast<uint8_t>(Device::Result::FIELD_ERROR)};
			parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
				static_cast<DataType::Service>(request->type), request->number, &error, sizeof(error));
		}

		pool.free(request);
	}

	void onFieldInfoRequestError(Device::FieldId /*aField*/, Device::Result aError,
		Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aError != Device::Result::COMPONENT_NOT_FOUND) {
			if (request->type == DataType::Service::PARAM_GET_SET) {
				const GetSetResponse<EmptyType, 0> response{};
				parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
					static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));
			} else {
				const GetFieldInfoResponseError response{static_cast<uint8_t>(aError)};
				parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
					static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));
			}
		}

		pool.free(request);
	}

	void onFieldReceived(Device::FieldId /*aField*/, const void *aData, Device::FieldType aType,
		Device::FieldDimension aDimension, Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);
		FieldReadResponse<Device::kFieldMaxSize> response;
		const size_t fieldSize = Device::sizeOfFieldType(aType) * aDimension;
		const size_t responseLength = sizeof(response) - sizeof(response.data) + fieldSize;

		response.result = static_cast<uint8_t>(Device::Result::SUCCESS);
		memcpy(response.data, aData, fieldSize);

		parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
			static_cast<DataType::Service>(request->type), request->number, &response, responseLength);

		pool.free(request);
	}

	void onFieldUpdated(Device::FieldId /*aField*/, Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);
		const FieldWriteResponse response{static_cast<uint8_t>(Device::Result::SUCCESS)};
		parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
			static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));
		pool.free(request);
	}

	void onFieldRequestError(Device::FieldId /*aField*/, Device::Result aError, Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aError != Device::Result::COMPONENT_NOT_FOUND) {
			if (request->type == DataType::Service::FIELD_READ) {
				const FieldReadResponse<0> response{static_cast<uint8_t>(aError)};
				parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
					static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));
			} else if (request->type == DataType::Service::FIELD_WRITE) {
				const FieldWriteResponse response{static_cast<uint8_t>(aError)};
				parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
					static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));
			}
		}

		pool.free(request);
	}

	// Files
	void onFileInfoReceived(const Device::FileInfo &aInfo, Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		const GetFileInfoResponse response{
			static_cast<uint8_t>(Device::Result::SUCCESS),
			static_cast<uint8_t>(aInfo.flags),
			aInfo.size,
			aInfo.checksum};
		parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
			static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));

		pool.free(request);
	}

	void onFileInfoRequestError(Device::FileId /*aFile*/, Device::Result aError, Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aError != Device::Result::COMPONENT_NOT_FOUND) {
			const GetFileInfoResponseError response{static_cast<uint8_t>(aError)};
			parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
				static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));
		}

		pool.free(request);
	}

	void onFileReadEnd(Device::FileId /*aFile*/, uint32_t /*aOffset*/, const void *aData, size_t aSize,
		Device::Result aResult, Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aResult != Device::Result::COMPONENT_NOT_FOUND) {
			FileReadResponse<Device::kFileMaxChunkLength> response;
			const size_t responseLength = sizeof(response) - sizeof(response.data)
				+ (aResult == Device::Result::SUCCESS ? aSize : 0);

			response.result = static_cast<uint8_t>(aResult);
			if (aResult == Device::Result::SUCCESS)
				memcpy(response.data, aData, aSize);

			parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
				static_cast<DataType::Service>(request->type), request->number, &response, responseLength);
		}

		pool.free(request);
	}

	void onFileWriteEnd(Device::FileId /*aFile*/, uint32_t /*aOffset*/, Device::Result aResult,
		Device::RefCounter *aToken) override
	{
		const auto *const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aResult != Device::Result::COMPONENT_NOT_FOUND) {
			const FileWriteResponse response{static_cast<uint8_t>(aResult)};
			parent.handler.sendCanServiceResponse(parent.currentAddress, request->source,
				static_cast<DataType::Service>(request->type), request->number, &response, sizeof(response));
		}

		pool.free(request);
	}

private:
	static constexpr size_t kMaxPendingRequests{8};

	T &parent;
	Device::RequestPool<DeviceRequestDescriptor, kMaxPendingRequests> pool;
};

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_PLAZCANSLAVE_HPP_
