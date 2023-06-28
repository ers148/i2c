//
// CanProxyFeature.hpp
//
//  Created on: Jan 14, 2021
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_CANPROXYFEATURE_HPP_
#define DRONEDEVICE_PLAZCAN_CANPROXYFEATURE_HPP_

#include <DroneDevice/AbstractDevice.hpp>
#include <DroneDevice/DeviceObserver.hpp>
#include <DroneDevice/PlazCan/CanDevice.hpp>
#include <DroneDevice/RefCounter.hpp>

namespace PlazCan {

class CanProxyFeature : public Device::AbstractDevice {
	using microseconds = std::chrono::microseconds;

public:
	~CanProxyFeature() = default;

	virtual bool onAcceptanceRequest(uint16_t /*aDataTypeId*/, CanardTransferType /*aTransferType*/)
	{
		return false;
	}

	virtual void onMessageReceived(const CanardRxTransfer * /*aTransfer*/)
	{
	}

	virtual void onNodeStatusReceived(const Status & /*aStatus*/, microseconds /*aUptime*/)
	{
	}

	///
	/// \brief onScheduledFieldReceived is called on each COMPOSITE_FIELD_VALUES reception
	/// \param aField field from message
	/// \param aType field type
	/// \param aDimension field dimension
	/// \param aData field value
	/// \param aFirst first field of the message
	/// \param aLast reached the end of the message
	///
	virtual void onScheduledFieldReceived(Device::FileId /*aField*/, Device::FieldType /*aType*/,
		Device::FieldDimension /*aDimension*/, const void * /*aData*/, bool /*aFirst*/, bool /*aLast*/)
	{
	}

	virtual microseconds onTimeoutOccurred()
	{
		return microseconds::max();
	}

	// Inherited from Device::AbstractDevice

	Device::DeviceHash deviceHash() const override final
	{
		return Device::DeviceHash{};
	}

	const char *deviceName() const override final
	{
		return nullptr;
	}

	Device::Version deviceVersion() const override final
	{
		static const Device::Version version{};
		return version;
	}

	size_t getFieldCount() override
	{
		return 0;
	}

	void fieldRequestIndex(const char *, Device::DeviceObserver *, Device::RefCounter *) override final
	{
		// TODO
	}

	void fieldRequestInfo(Device::FieldId, Device::DeviceObserver *, Device::RefCounter *) override
	{
	}

	void fieldRead(Device::FieldId, Device::DeviceObserver *, Device::RefCounter *) override
	{
	}

	void fieldWrite(Device::FieldId, const void *, Device::DeviceObserver *, Device::RefCounter *) override
	{
	}

	void fileRequestInfo(Device::FileId, Device::FileFlags, Device::DeviceObserver *,
		Device::RefCounter *) override final
	{
	}

	void fileRead(Device::FileId, uint32_t, size_t, Device::DeviceObserver *,
		Device::RefCounter *) override final
	{
	}

	void fileWrite(Device::FileId, uint32_t, const void *, size_t, Device::DeviceObserver *,
		Device::RefCounter *) override final
	{
	}
};

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_CANPROXYFEATURE_HPP_
