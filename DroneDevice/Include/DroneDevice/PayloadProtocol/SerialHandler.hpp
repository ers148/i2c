//
// SerialHandler.hpp
//
//  Created on: Oct 23, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PAYLOADPROTOCOL_SERIALHANDLER_HPP_
#define DRONEDEVICE_PAYLOADPROTOCOL_SERIALHANDLER_HPP_

#include <DroneDevice/DeviceObserver.hpp>
#include <DroneDevice/FastCrc8.hpp>
#include <DroneDevice/PayloadProtocol/DeviceRequestDescriptor.hpp>
#include <DroneDevice/PayloadProtocol/SerialPacket.hpp>
#include <DroneDevice/PayloadProtocol/SerialParser.hpp>
#include <DroneDevice/Queue.hpp>
#include <DroneDevice/RefCounter.hpp>
#include <DroneDevice/RequestPool.hpp>

namespace PayloadProtocol {

template<size_t limit, typename Handler, typename Interface, typename Crc = FastCrc8>
class SerialHandler: public Device::DeviceObserver {
	static_assert(limit >= sizeof(DeviceInfoResponse), "Message buffer is too small");
	static_assert(limit <= 256, "Message buffer is too large");

private:
	using Parser = SerialParser<limit, Crc>;
	static constexpr size_t kMaxPendingRequests{8};

public:
	SerialHandler(Handler &aHandler, Interface &aInterface) :
		handler{aHandler},
		interface{aInterface},
		parser{},
		pool{}
	{
	}

	// Fields

	void onFieldInfoReceived(const Device::FieldInfo &aInfo, const void *, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);
		const size_t nameLength = strlen(aInfo.name);
		const size_t responseLength = sizeof(FieldInfoResponse) - sizeof(FieldInfoResponse::payload.name) + nameLength;

		FieldInfoResponse response;
		makeDefaultHeader(response, request->destination, request->number);

		response.payload.type = static_cast<uint8_t>(aInfo.type);
		response.payload.nameLength = static_cast<uint8_t>(nameLength);
		memcpy(response.payload.name, aInfo.name, nameLength);

		sendMessage(&response, responseLength);
		pool.free(request);
	}

	void onFieldInfoRequestError(Device::FieldId, Device::Result aError, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aError != Device::Result::COMPONENT_NOT_FOUND) {
			sendResult(request->number, request->destination, aError);
		}
		pool.free(request);
	}

	void onFieldReceived(Device::FieldId, const void *aData, Device::FieldType aType, Device::FieldDimension aDimension,
		Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);
		const size_t dataLength = Device::sizeOfFieldType(aType) * aDimension;
		const size_t responseLength = sizeof(FieldReadResponse) - sizeof(FieldReadResponse::payload.data) + dataLength;

		FieldReadResponse response;
		makeDefaultHeader(response, request->destination, request->number);

		response.payload.type = static_cast<uint8_t>(aType);
		memcpy(response.payload.data, aData, dataLength);

		sendMessage(&response, responseLength);
		pool.free(request);
	}

	void onFieldUpdated(Device::FieldId, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		Header response;
		makeDefaultHeader(response, request->destination, request->number);

		sendMessage(&response, sizeof(response));
		pool.free(request);
	}

	void onFieldRequestError(Device::FieldId, Device::Result aError, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aError != Device::Result::COMPONENT_NOT_FOUND) {
			sendResult(request->number, request->destination, aError);
		}
		pool.free(request);
	}

	// Files

	void onFileInfoReceived(const Device::FileInfo &aInfo, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		FileInfoResponse response;
		makeDefaultHeader(response, request->destination, request->number);

		response.payload.status = static_cast<uint8_t>(aInfo.flags);
		response.payload.size = aInfo.size;
		response.payload.crc = aInfo.checksum;

		sendMessage(&response, sizeof(response));
		pool.free(request);
	}

	void onFileInfoRequestError(Device::FileId, Device::Result aError, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aError != Device::Result::COMPONENT_NOT_FOUND) {
			sendResult(request->number, request->destination, aError);
		}
		pool.free(request);
	}

	void onFileReadEnd(Device::FileId, uint32_t, const void *aData, size_t aSize,
		Device::Result aResult, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aResult == Device::Result::SUCCESS) {
			const size_t responseLength = sizeof(FileReadResponse) - sizeof(FileReadResponse::payload.data) + aSize;

			FileReadResponse response;
			makeDefaultHeader(response, request->destination, request->number);
			memcpy(response.payload.data, aData, aSize);

			sendMessage(&response, responseLength);
		} else if (aResult != Device::Result::COMPONENT_NOT_FOUND) {
			sendResult(request->number, request->destination, aResult);
		}
		pool.free(request);
	}

	void onFileWriteEnd(Device::FileId, uint32_t, Device::Result aResult,
		Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);

		if (aResult != Device::Result::COMPONENT_NOT_FOUND) {
			sendResult(request->number, request->destination, aResult);
		}
		pool.free(request);
	}

	size_t update(const void *data, size_t aLength)
	{
		size_t left = aLength;
		size_t parsed = 0;

		while (left) {
			left -= parser.update(static_cast<const uint8_t *>(data) + (aLength - left), left);

			if (parser.state() == Parser::State::DONE) {
				++parsed;
				process(parser.data(), parser.length());
			}
		}

		return parsed;
	}

private:
	Handler &handler;
	Interface &interface;
	Parser parser;

	Device::RequestPool<DeviceRequestDescriptor, kMaxPendingRequests> pool;

	void onDeviceInfoCallback(Device::AbstractDevice *aDevice, Device::RefCounter *aToken)
	{
		const auto * const request = static_cast<const DeviceRequestDescriptor *>(aToken);
		const auto version = aDevice->deviceVersion();
		const size_t nameLength = std::min(strlen(aDevice->deviceName()), kMaxNameLength);
		const size_t responseLength = sizeof(DeviceInfoResponse) - sizeof(DeviceInfoResponse::payload.name)
			+ nameLength;

		DeviceInfoResponse response;
		makeDefaultHeader(response, request->destination, request->number);

		static_assert(sizeof(response.payload.hash) <= sizeof(Device::DeviceHash), "Incorrect type");

		response.payload.major = static_cast<uint8_t>(version.hw.major);
		response.payload.minor = static_cast<uint8_t>(version.hw.minor);
		response.payload.revision = static_cast<uint16_t>(version.sw.revision);
		response.payload.count = static_cast<uint8_t>(aDevice->getFieldCount());
		response.payload.nameLength = static_cast<uint8_t>(nameLength);
		memcpy(response.payload.name, aDevice->deviceName(), nameLength);
		memcpy(&response.payload.hash, aDevice->deviceHash().data(), sizeof(response.payload.hash));

		sendMessage(&response, responseLength);
		pool.free(request);
	}

	void process(const void *aMessage, size_t aLength)
	{
		const auto * const header = static_cast<const Header *>(aMessage);
		auto * const request = pool.alloc(header->number, header->address);

		if (request == nullptr) {
			return;
		}

		bool queued = false;

		switch (static_cast<Command>(header->keyword)) {
			case GET_DEVICE_INFO: {
				handler.deviceRequestInfo(header->address,
					[this](Device::AbstractDevice *aDevice, Device::RefCounter *aToken){
						onDeviceInfoCallback(aDevice, aToken);
					},
					request);
				queued = true;
				break;
			}

			case SET_VALUE: {
				static constexpr size_t minLength = sizeof(FieldWriteRequest) - sizeof(FieldWriteRequest::payload.data);
				static constexpr size_t maxLength = sizeof(FieldWriteRequest);

				if (aLength > minLength && aLength <= maxLength) {
					const auto * const packet = static_cast<const FieldWriteRequest *>(aMessage);
					handler.fieldWrite(
						static_cast<Device::DeviceId>(header->address),
						static_cast<Device::FieldId>(packet->payload.field),
						packet->payload.data,
						this,
						request
					);
					queued = true;
				}
				break;
			}

			case GET_VALUE: {
				if (aLength == sizeof(FieldReadRequest)) {
					const auto * const packet = static_cast<const FieldReadRequest *>(aMessage);
					handler.fieldRead(header->address, packet->payload.field, this, request);
					queued = true;
				}
				break;
			}

			case GET_FIELD_INFO: {
				if (aLength == sizeof(FieldInfoRequest)) {
					const auto * const packet = static_cast<const FieldInfoRequest *>(aMessage);
					handler.fieldRequestInfo(header->address, packet->payload.field, this, request);
					queued = true;
				}
				break;
			}

			case GET_FILE_INFO: {
				if (aLength == sizeof(FileInfoRequest)) {
					const auto * const packet = static_cast<const FileInfoRequest *>(aMessage);
					handler.fileRequestInfo(header->address, packet->payload.file, packet->payload.flags,
						this, request);
					queued = true;
				}
				break;
			}

			case WRITE_FILE_CHUNK: {
				static constexpr size_t baseLength = sizeof(FileWriteRequest) - sizeof(FileWriteRequest::payload.data);

				if (aLength >= baseLength) {
					const size_t dataLength = aLength - baseLength;
					const auto * const packet = static_cast<const FileWriteRequest *>(aMessage);
					uint8_t buffer[Device::kFileMaxChunkLength];

					// Copy file chunk to align buffer
					memcpy(buffer, packet->payload.data, dataLength);

					handler.fileWrite(header->address, packet->payload.file, packet->payload.offset,
						buffer, dataLength, this, request);
					queued = true;
				}
				break;
			}

			case READ_FILE_CHUNK: {
				if (aLength == sizeof(FileReadRequest)) {
					const auto * const packet = static_cast<const FileReadRequest *>(aMessage);
					handler.fileRead(header->address, packet->payload.file, packet->payload.offset,
						packet->payload.size, this, request);
					queued = true;
				}
				break;
			}

			default:
				break;
		}

		if (!queued) {
			pool.free(request);
		}
	}

	static void makeDefaultHeader(Header &header, Device::DeviceId device, uint8_t number)
	{
		header.number = number;
		header.address = static_cast<uint8_t>(device);
		header.keyword = Result::SUCCESS;
	}

	void sendResult(uint8_t number, Device::DeviceId device, Device::Result error)
	{
		Header response;

		response.number = number;
		response.address = static_cast<uint8_t>(device);
		response.keyword = convertCommandResult(error);

		sendMessage(&response, sizeof(response));
	}

	void sendMessage(const void *response, size_t responseLength)
	{
		uint8_t buffer[Parser::kBufferLength];
		const size_t aLength = Parser::create(buffer, sizeof(buffer), response, responseLength);

		if (aLength) {
			interface.write(buffer, aLength);
		}
	}

	static Result convertCommandResult(Device::Result result)
	{
		switch (result) {
			case Device::Result::SUCCESS:
				return Result::SUCCESS;

			case Device::Result::FIELD_NOT_FOUND:
				return Result::FIELD_NOT_FOUND;

			case Device::Result::FIELD_TYPE_MISMATCH:
			case Device::Result::FIELD_RANGE_ERROR:
				return Result::FIELD_TYPE_ERROR;

			case Device::Result::FIELD_ERROR:
			case Device::Result::FIELD_TIMEOUT:
				return Result::FIELD_ERROR;

			case Device::Result::FIELD_READ_ONLY:
				return Result::FIELD_READ_ONLY;

			case Device::Result::FILE_NOT_FOUND:
			case Device::Result::FILE_ERROR:
			case Device::Result::FIELD_UNAVAILABLE:
				return Result::FIELD_UNAVAILABLE;

			case Device::Result::COMMAND_UNSUPPORTED:
				return Result::COMMAND_UNSUPPORTED;

			case Device::Result::COMMAND_QUEUED:
				return Result::COMMAND_QUEUED;

			default:
				return Result::COMMAND_FAILED;
		}
	}
};

}

#endif // DRONEDEVICE_SERIAL_SERIALHANDLER_HPP_
