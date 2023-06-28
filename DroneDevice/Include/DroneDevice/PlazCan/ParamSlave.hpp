//
// ParamSlave.hpp
//
//  Created on: Feb 5, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_PARAMSLAVE_HPP_
#define DRONEDEVICE_PLAZCAN_PARAMSLAVE_HPP_

#include <DroneDevice/DeviceObserver.hpp>
#include <DroneDevice/PlazCan/CanHelpers.hpp>
#include <DroneDevice/PlazCan/ParamRequestDescriptor.hpp>
#include <DroneDevice/PlazCan/UavCanPacket.hpp>
#include <DroneDevice/RequestPool.hpp>
#include <cstring>

namespace PlazCan {

template<typename T>
class ParamSlave : public Device::DeviceObserver {
public:
	ParamSlave(T &aParent) :
		parent{aParent}
	{
	}

	bool process(const CanardRxTransfer *aTransfer)
	{
		switch (aTransfer->data_type_id) {
			case DataType::Service::PARAM_GET_SET: {
				auto * const request = pool.alloc(aTransfer->data_type_id, aTransfer->transfer_id,
					aTransfer->source_node_id, aTransfer->destination_node_id);

				if (request) {
					char nameBuffer[CanHelpers::kMaxFieldNameLength + 1] = {};
					size_t nameLength{0};
					uint16_t paramIndex{0};
					uint8_t paramTag{0};

					canardDecodeScalar(aTransfer, 0, 13, false, &paramIndex);
					canardDecodeScalar(aTransfer, 13, 3, false, &paramTag);

					const Device::FieldType paramType = fieldTypeFromParamType(paramTag);
					const size_t paramLength = paramType != Device::FieldType::UNDEFINED ?
						Device::sizeOfFieldType(paramType) : 0;

					if (aTransfer->payload_len > 3) {
						nameLength = aTransfer->payload_len - 2 - paramLength;
						CanHelpers::decodeBlobField(aTransfer, 2 + paramLength, nameLength, nameBuffer);
					}

					if (paramLength) {
						request->paramType = paramType;
						CanHelpers::decodeBlobField(aTransfer, 2, paramLength, request->desiredValue);
					}

					if (nameLength) {
						// Find field index by name
						strcpy(request->name, nameBuffer);
						parent.device.fieldRequestIndex(nameBuffer, this, request);
					} else {
						// Read or write field
						parent.device.fieldRequestInfo(static_cast<Device::FieldId>(paramIndex), this, request);
					}
				}
				return true;
			}

			default:
				return false;
		}
	}

	// Fields
	void onFieldIndexReceived(Device::FieldId aField, Device::RefCounter *aToken) override
	{
		parent.device.fieldRequestInfo(aField, this, aToken);
	}

	void onFieldIndexRequestError(Device::Result /*aError*/, Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const ParamRequestDescriptor *>(aToken);
		sendEmptyGetSetResponse(request);
		pool.free(request);
	}

	void onFieldInfoReceived(const Device::FieldInfo &aInfo, const void *aValue, Device::RefCounter *aToken) override
	{
		auto * const request = static_cast<ParamRequestDescriptor *>(aToken);

		if (request->name[0] == '\0') {
			strncpy(request->name, aInfo.name, sizeof(request->name));
		}

		CanHelpers::toUavCan(request->currentValue, aValue, aInfo.type);

		if (aInfo.min == nullptr || aInfo.max == nullptr)
			CanHelpers::fillMinMaxValues(request->min, request->max, fieldTypeToParamTag(aInfo.type));
		if (aInfo.min != nullptr)
			CanHelpers::toUavCan(request->min, aInfo.min, aInfo.type);
		if (aInfo.max != nullptr)
			CanHelpers::toUavCan(request->max, aInfo.max, aInfo.type);

		if (request->paramType == Device::FieldType::UNDEFINED) {
			// Getter
			sendGetSetResponse(request, request->currentValue);
			pool.free(request);
		} else {
			// Setter
			uint8_t value[Device::kFieldMaxSize];
			CanHelpers::fromUavCan(value, request->desiredValue, aInfo.type);
			parent.device.fieldWrite(aInfo.index, value, this, aToken);
		}
	}

	void onFieldInfoRequestError(Device::FieldId /*aField*/, Device::Result aError,
		Device::RefCounter *aToken) override
	{
		const auto * const request = static_cast<const ParamRequestDescriptor *>(aToken);

		if (aError != Device::Result::COMPONENT_NOT_FOUND) {
			sendEmptyGetSetResponse(request);
		}

		pool.free(request);
	}

	void onFieldUpdated(Device::FieldId /*aField*/, Device::RefCounter *aToken) override
	{
		auto * const request = static_cast<ParamRequestDescriptor *>(aToken);
		sendGetSetResponse(request, request->desiredValue);
		pool.free(request);
	}

	void onFieldRequestError(Device::FieldId /*aField*/, Device::Result /*aError*/,
		Device::RefCounter *aToken) override
	{
		auto * const request = static_cast<ParamRequestDescriptor *>(aToken);
		sendGetSetResponse(request, request->currentValue);
		pool.free(request);
	}

protected:
	static constexpr size_t kMaxPendingParams = 2;

	T &parent;
	Device::RequestPool<ParamRequestDescriptor, kMaxPendingParams> pool;

	void sendEmptyGetSetResponse(const ParamRequestDescriptor *aRequest)
	{
		const GetSetResponse<EmptyType, 0> response{};
		parent.handler.sendCanServiceResponse(parent.currentAddress, aRequest->source,
			static_cast<DataType::Service>(aRequest->type), aRequest->number, &response, sizeof(response));
	}

	void sendGetSetResponse(const ParamRequestDescriptor *aRequest, const void *aValue)
	{
		if (Device::isFloatType(aRequest->paramType)) {
			sendGetSetResponseImpl<float>(aRequest, aValue);
		} else if (aRequest->paramType != Device::FieldType::BOOL) {
			sendGetSetResponseImpl<int64_t>(aRequest, aValue);
		} else {
			sendGetSetResponseImpl<bool>(aRequest, aValue);
		}
	}

	template<typename U>
	void sendGetSetResponseImpl(const ParamRequestDescriptor *aRequest, const void *aValue)
	{
		const GetSetResponse<U, CanHelpers::kMaxFieldNameLength> response{
			aValue, aRequest->max, aRequest->min, aRequest->name};
		const size_t responseLength = sizeof(response) - sizeof(response.name) + strlen(aRequest->name);

		parent.handler.sendCanServiceResponse(parent.currentAddress, aRequest->source,
			static_cast<DataType::Service>(aRequest->type), aRequest->number, &response, responseLength);
	}
};

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_PARAMSLAVE_HPP_
