//
// DeviceObserver.hpp
//
//  Created on: Oct 18, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_DEVICEOBSERVER_HPP_
#define DRONEDEVICE_DEVICEOBSERVER_HPP_

#include <DroneDevice/CoreTypes.hpp>
#include <DroneDevice/FieldInfo.hpp>
#include <DroneDevice/FileInfo.hpp>

namespace Device {

struct RefCounter;

class DeviceObserver {
public:
	virtual ~DeviceObserver() = default;

	// Fields
	virtual void onFieldIndexReceived(FieldId /*aField*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFieldIndexRequestError(Result /*aError*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFieldInfoReceived(const FieldInfo &/*aInfo*/, const void */*aData*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFieldInfoRequestError(FieldId /*aField*/, Result /*aError*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFieldReceived(FieldId /*aField*/, const void */*aData*/, FieldType /*aType*/,
		FieldDimension /*aDimension*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFieldUpdated(FieldId /*aField*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFieldRequestError(FieldId /*aField*/, Result /*aError*/, RefCounter */*aToken*/)
	{
	}

	// Files
	virtual void onFileInfoReceived(const FileInfo &/*aInfo*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFileInfoRequestError(FileId /*aFile*/, Result /*aError*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFileReadEnd(FileId /*aFile*/, uint32_t /*aOffset*/, const void */*aData*/, size_t /*aSize*/,
		Result /*aResult*/, RefCounter */*aToken*/)
	{
	}
	virtual void onFileWriteEnd(FileId /*aFile*/, uint32_t /*aOffset*/, Result /*aResult*/, RefCounter */*aToken*/)
	{
	}
};

} // namespace Device

#endif // DRONEDEVICE_DEVICEOBSERVER_HPP_
