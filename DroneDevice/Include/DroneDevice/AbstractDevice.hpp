//
// AbstractDevice.hpp
//
//  Created on: Oct 11, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_ABSTRACTDEVICE_HPP_
#define DRONEDEVICE_ABSTRACTDEVICE_HPP_

#include <DroneDevice/DeviceObserver.hpp>

namespace Device {

class AbstractDevice {
public:
	virtual ~AbstractDevice() = default;

	// Generic functions

	//!
	//! \brief deviceName Synchronously returns hash calculated for fields and files.
	//!
	virtual DeviceHash deviceHash() const = 0;

	//!
	//! \brief deviceName Synchronously returns symbolic device name.
	//! Device name should be cached in an object of a derived class.
	//!
	virtual const char *deviceName() const = 0;

	//!
	//! \brief deviceName Synchronously returns device version.
	//!
	virtual Version deviceVersion() const = 0;

	// Synchronous field functions

	//!
	//! \brief getFieldCount Synchronously returns number of fields
	//! Field count should be cached in an object of a derived class.
	//!
	virtual size_t getFieldCount() = 0;

	// Asynchronous field functions
	virtual void fieldRequestIndex(const char *aField, DeviceObserver *aObserver, RefCounter *aToken) = 0;
	virtual void fieldRequestInfo(FieldId aField, DeviceObserver *aObserver, RefCounter *aToken) = 0;
	virtual void fieldRead(FieldId aField, DeviceObserver *aObserver, RefCounter *aToken) = 0;
	virtual void fieldWrite(FieldId aField, const void *aBuffer, DeviceObserver *aObserver, RefCounter *aToken) = 0;

	// Asynchronous file functions
	virtual void fileRequestInfo(FileId aFile, FileFlags aFlags, DeviceObserver *aObserver,
		RefCounter *aToken) = 0;
	virtual void fileRead(FileId aFile, uint32_t aOffset, size_t aSize, DeviceObserver *aObserver,
		RefCounter *aToken) = 0;
	virtual void fileWrite(FileId aFile, uint32_t aOffset, const void *aBuffer, size_t aSize, DeviceObserver *aObserver,
		RefCounter *aToken) = 0;

	// Asynchronous template field helpers
	template<typename T>
	typename std::enable_if_t<!std::is_pointer<T>::value> fieldWrite(FieldId aField, const T &aBuffer,
		DeviceObserver *aObserver, RefCounter *aToken)
	{
		fieldWrite(aField, &aBuffer, aObserver, aToken);
	}
};

} // namespace Device

#endif // DRONEDEVICE_ABSTRACTDEVICE_HPP_
