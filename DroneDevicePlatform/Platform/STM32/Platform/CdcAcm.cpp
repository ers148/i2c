//
// CdcAcm.cpp
//
//  Created on: Aug 27, 2019
//      Author: Alexander
//

#include <libopencm3/usb/dwc/otg_fs.h>
#include "CdcAcm.hpp"

const CdcAcmFunctionalDescriptors CdcAcm::kFunctionalDescriptors{};

// clang-format off
const usb_device_descriptor CdcAcm::kDeviceDescriptor{
	USB_DT_DEVICE_SIZE, // bLength
	USB_DT_DEVICE,      // bDescriptorType
	0x0200,             // bcdUSB
	USB_CLASS_CDC,      // bDeviceClass
	0,                  // bDeviceSubClass
	0,                  // bDeviceProtocol
	64,                 // bMaxPacketSize0
	0x0483,             // idVendor
	0x5740,             // idProduct
	0x0200,             // bcdDevice
	0,                  // iManufacturer
	0,                  // iProduct
	0,                  // iSerialNumber
	1                   // bNumConfigurations
};
// clang-format on

// clang-format off
const std::array<usb_endpoint_descriptor, 1> CdcAcm::kNotificationEndpoints = {{
	{
		USB_DT_ENDPOINT_SIZE,        // bLength
		USB_DT_ENDPOINT,             // bDescriptorType
		kNotificationEp,             // bEndpointAddress
		USB_ENDPOINT_ATTR_INTERRUPT, // bmAttributes
		16,                          // wMaxPacketSize
		255,                         // bInterval
		nullptr,                     // extra
		0                            // extralen
	}
}};
// clang-format on

// clang-format off
const std::array<usb_endpoint_descriptor, 2> CdcAcm::kDataEndpoints = {{
	{
		USB_DT_ENDPOINT_SIZE,   // bLength
		USB_DT_ENDPOINT,        // bDescriptorType
		kDataRxEp,              // bEndpointAddress
		USB_ENDPOINT_ATTR_BULK, // bmAttributes
		64,                     // wMaxPacketSize
		1,                      // bInterval
		nullptr,                // extra
		0                       // extralen
	}, {
		USB_DT_ENDPOINT_SIZE,   // bLength
		USB_DT_ENDPOINT,        // bDescriptorType
		kDataTxEp,              // bEndpointAddress
		USB_ENDPOINT_ATTR_BULK, // bmAttributes
		64,                     // wMaxPacketSize
		1,                      // bInterval
		nullptr,                // extra
		0                       // extralen
	}
}};
// clang-format on

// clang-format off
const usb_interface_descriptor CdcAcm::kNotificationInterface{
	USB_DT_INTERFACE_SIZE,              // bLength
	USB_DT_INTERFACE,                   // bDescriptorType
	0,                                  // bInterfaceNumber
	0,                                  // bAlternateSetting
	kNotificationEndpoints.size(),      // bNumEndpoints
	USB_CLASS_CDC,                      // bInterfaceClass
	USB_CDC_SUBCLASS_ACM,               // bInterfaceSubClass
	USB_CDC_PROTOCOL_AT,                // bInterfaceProtocol
	0,                                  // iInterface
	kNotificationEndpoints.data(),      // endpoint
	&kFunctionalDescriptors,            // extra
	sizeof(CdcAcmFunctionalDescriptors) // extralen
};
// clang-format on

// clang-format off
const usb_interface_descriptor CdcAcm::kDataInterface{
	USB_DT_INTERFACE_SIZE, // bLength
	USB_DT_INTERFACE,      // bDescriptorType
	1,                     // bInterfaceNumber
	0,                     // bAlternateSetting
	kDataEndpoints.size(), // bNumEndpoints
	USB_CLASS_DATA,        // bInterfaceClass
	0,                     // bInterfaceSubClass
	0,                     // bInterfaceProtocol
	0,                     // iInterface
	kDataEndpoints.data(), // endpoint
	nullptr,               // extra
	0                      // extralen
};
// clang-format on

// clang-format off
const std::array<usb_interface, 2> CdcAcm::kInterfaces = {{
	{
		nullptr,                // cur_altsetting
		1,                      // num_altsetting
		nullptr,                // iface_assoc
		&kNotificationInterface // altsetting
	}, {
		nullptr,                // cur_altsetting
		1,                      // num_altsetting
		nullptr,                // iface_assoc
		&kDataInterface         // altsetting
	}
}};
// clang-format on

// clang-format off
const usb_config_descriptor CdcAcm::kConfigDescriptor{
	USB_DT_CONFIGURATION_SIZE, // bLength
	USB_DT_CONFIGURATION,      // bDescriptorType
	0,                         // wTotalLength
	kInterfaces.size(),        // bNumInterfaces
	1,                         // bConfigurationValue
	0,                         // iConfiguration
	0x80,                      // bmAttributes
	0x32,                      // bMaxPower
	kInterfaces.data()         // interface
};
// clang-format on

// clang-format off
constexpr CdcAcmFunctionalDescriptors::CdcAcmFunctionalDescriptors() :
	header{
		sizeof(usb_cdc_header_descriptor), // bFunctionLength
		CS_INTERFACE,                      // bDescriptorType
		USB_CDC_TYPE_HEADER,               // bDescriptorSubtype
		0x0110                             // bcdCDC
	},
	call_mgmt{
		sizeof(usb_cdc_call_management_descriptor), // bFunctionLength
		CS_INTERFACE,                               // bDescriptorType
		USB_CDC_TYPE_CALL_MANAGEMENT,               // bDescriptorSubtype
		0,                                          // bmCapabilities
		1                                           // bDataInterface
	},
	acm{
		sizeof(usb_cdc_acm_descriptor), // bFunctionLength
		CS_INTERFACE,                   // bDescriptorType
		USB_CDC_TYPE_ACM,               // bDescriptorSubtype
		0                               // bmCapabilities
	},
	cdc_union{
		sizeof(usb_cdc_union_descriptor), // bFunctionLength
		CS_INTERFACE,                     // bDescriptorType
		USB_CDC_TYPE_UNION,               // bDescriptorSubtype
		0,                                // bControlInterface
		1                                 // bSubordinateInterface0
	}
{
}
// clang-format on

CdcAcm::CdcAcm() :
	pending{false}
{
	rcc_periph_clock_enable(RCC_OTGFS);

	device = usbd_init(
		&otgfs_usb_driver,
		&kDeviceDescriptor,
		&kConfigDescriptor,
		nullptr, 0,
		arena, sizeof(arena));

	usbd_register_set_config_callback(device, setInterfaceConfig);
}

CdcAcm::~CdcAcm()
{
	rcc_periph_clock_disable(RCC_OTGFS);
}

void CdcAcm::update()
{
	usbd_poll(device);
}

void CdcAcm::vbusDisable()
{
	OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS;
}

usbd_request_return_codes CdcAcm::handleControlRequest(usbd_device *,
	usb_setup_data *aRequest, uint8_t **, uint16_t *aLength, void (**)(usbd_device *, usb_setup_data *))
{
	switch (aRequest->bRequest) {
		case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
			return USBD_REQ_HANDLED;
		}

		case USB_CDC_REQ_SET_LINE_CODING: {
			if (*aLength < sizeof(usb_cdc_line_coding)) {
				return USBD_REQ_NOTSUPP;
			} else {
				return USBD_REQ_HANDLED;
			}
		}

		default: {
			return USBD_REQ_NOTSUPP;
		}
	}
}

void CdcAcm::handleDataRxCallback(usbd_device *, uint8_t)
{
	auto &interface = CdcAcm::instance();

	uint8_t buffer[64];
	const auto bytesRead = usbd_ep_read_packet(interface.device, kDataRxEp, buffer, sizeof(buffer));

	interface.rxQueue.push(buffer, bytesRead);

	if (!interface.rxQueue.empty() && interface.callback != nullptr) {
		interface.callback();
	}
}

void CdcAcm::handleDataTxCallback(usbd_device *, uint8_t)
{
	auto &interface = CdcAcm::instance();

	if (!interface.txQueue.empty()) {
		uint8_t buffer[64];
		const auto bytesToWrite = interface.txQueue.pop(buffer, sizeof(buffer));

		usbd_ep_write_packet(interface.device, kDataTxEp, buffer, static_cast<uint16_t>(bytesToWrite));
		interface.pending = true;
	} else {
		interface.pending = false;
	}
}

void CdcAcm::setInterfaceConfig(usbd_device *aDevice, uint16_t)
{
	usbd_ep_setup(aDevice, kDataRxEp, USB_ENDPOINT_ATTR_BULK, 64, handleDataRxCallback);
	usbd_ep_setup(aDevice, kDataTxEp, USB_ENDPOINT_ATTR_BULK, 64, handleDataTxCallback);
	usbd_ep_setup(aDevice, kNotificationEp, USB_ENDPOINT_ATTR_INTERRUPT, 16, nullptr);

	usbd_register_control_callback(
		aDevice,
		USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
		USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
		handleControlRequest);

	CdcAcm::instance().pending = false;
}

size_t CdcAcm::read(void *aBuffer, size_t aLength)
{
	return rxQueue.pop(static_cast<uint8_t *>(aBuffer), aLength);
}

size_t CdcAcm::write(const void *aBuffer, size_t aLength)
{
	const auto bytesWritten = txQueue.push(static_cast<const uint8_t *>(aBuffer), aLength);

	if (bytesWritten && !pending) {
		handleDataTxCallback(device, kDataTxEp);
	}

	return bytesWritten;
}
