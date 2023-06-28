//
// CdcAcm.hpp
//
//  Created on: Aug 27, 2019
//      Author: Alexander
//

#ifndef PLATFORM_STM32_PLATFORM_CDCACM_HPP_
#define PLATFORM_STM32_PLATFORM_CDCACM_HPP_

#include <array>
#include <functional>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <DroneDevice/Queue.hpp>

struct CdcAcmFunctionalDescriptors {
	usb_cdc_header_descriptor header;
	usb_cdc_call_management_descriptor call_mgmt;
	usb_cdc_acm_descriptor acm;
	usb_cdc_union_descriptor cdc_union;

	constexpr CdcAcmFunctionalDescriptors();
} __attribute__((packed));

class CdcAcmBase {

};

class CdcAcm: public CdcAcmBase {
public:
	~CdcAcm();

	void update();
	void vbusDisable();

	void setCallback(std::function<void ()> aCallback)
	{
		callback = aCallback;
	}

	size_t read(void *aBuffer, size_t aLength);
	size_t write(const void *aBuffer, size_t aLength);

	static CdcAcm &instance()
	{
		static CdcAcm object{};
		return object;
	}

private:
	static constexpr size_t kControlBufferSize{128};
	static constexpr size_t rxQueueSize{384};
	static constexpr size_t txQueueSize{384};

	static constexpr uint8_t kDataRxEp{0x01};
	static constexpr uint8_t kDataTxEp{0x82};
	static constexpr uint8_t kNotificationEp{0x83};

	CdcAcm();
	CdcAcm(const CdcAcm &) = delete;
	CdcAcm &operator=(const CdcAcm &) = delete;

	uint8_t arena[kControlBufferSize];
	usbd_device *device;
	bool pending;

	Queue<uint8_t, rxQueueSize> rxQueue;
	Queue<uint8_t, txQueueSize> txQueue;

	std::function<void ()> callback;

	static usbd_request_return_codes handleControlRequest(usbd_device *,
		usb_setup_data *, uint8_t **, uint16_t *, void (**)(usbd_device *, usb_setup_data *));
	static void handleDataRxCallback(usbd_device *, uint8_t);
	static void handleDataTxCallback(usbd_device *, uint8_t);
	static void setInterfaceConfig(usbd_device *, uint16_t);

	static const usb_device_descriptor kDeviceDescriptor;
	static const std::array<usb_endpoint_descriptor, 1> kNotificationEndpoints;
	static const std::array<usb_endpoint_descriptor, 2> kDataEndpoints;
	static const CdcAcmFunctionalDescriptors kFunctionalDescriptors;
	static const usb_interface_descriptor kNotificationInterface;
	static const usb_interface_descriptor kDataInterface;
	static const std::array<usb_interface, 2> kInterfaces;
	static const usb_config_descriptor kConfigDescriptor;
};

#endif // PLATFORM_STM32_PLATFORM_CDCACM_HPP_
