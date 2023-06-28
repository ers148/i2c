//
// SerialPacket.hpp
//
//  Created on: Oct 23, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PAYLOADPROTOCOL_SERIALPACKET_HPP_
#define DRONEDEVICE_PAYLOADPROTOCOL_SERIALPACKET_HPP_

#include <DroneDevice/AbstractDevice.hpp>

namespace PayloadProtocol {

// clang-format off
static constexpr size_t kMaxFieldLength{24};
static constexpr size_t kMaxNameLength{80};
static constexpr size_t kMaxUnitLength{16};
// clang-format on

// clang-format off
enum Command: uint8_t {
	UNDEFINED           = 0x00,
	GET_DEVICE_INFO     = 0x01,
	SET_VALUE           = 0x03,
	GET_VALUE           = 0x04,
	GET_FIELD_INFO      = 0x05,
	GET_INDEX_BY_NAME   = 0x06,
	GET_FILE_INFO       = 0x07,
	WRITE_FILE_CHUNK    = 0x08,
	READ_FILE_CHUNK     = 0x09
};
// clang-format on

// clang-format off
enum Result: uint8_t {
	SUCCESS             = 0x00,
	UNKNOWN_COMMAND     = 0x01,
	COMMAND_UNSUPPORTED = 0x02,
	FIELD_ERROR         = 0x03,
	COMMAND_FAILED      = 0x04,
	FIELD_READ_ONLY     = 0x05,
	FIELD_NOT_FOUND     = 0x06,
	FIELD_TYPE_ERROR    = 0x07,
	COMMAND_QUEUED      = 0x08,
	FIELD_UNAVAILABLE   = 0x09
};
// clang-format on

struct Header {
	uint8_t number;
	uint8_t address;
	uint8_t keyword;
} __attribute__((packed));

template<class T>
struct SerialPacket: public Header {
	T payload;
} __attribute__((packed));

struct DeviceInfoResponsePayload {
	uint8_t major;
	uint8_t minor;
	uint16_t revision;
	uint8_t count;
	uint32_t hash;
	uint8_t nameLength;
	uint8_t name[kMaxNameLength];
} __attribute__((packed));

struct FieldInfoRequestPayload {
	uint8_t field;
} __attribute__((packed));

struct FieldInfoResponsePayload {
	uint8_t type;
	uint8_t nameLength;
	uint8_t name[kMaxFieldLength];
} __attribute__((packed));

struct FieldReadRequestPayload {
	uint8_t field;
} __attribute__((packed));

struct FieldReadResponsePayload {
	uint8_t type;
	uint8_t data[Device::kFieldMaxSize];
} __attribute__((packed));

struct FieldWriteRequestPayload {
	uint8_t field;
	uint8_t type;
	uint8_t data[Device::kFieldMaxSize];
} __attribute__((packed));

struct FileInfoRequestPayload {
	uint8_t file;
	uint8_t flags;
} __attribute__((packed));

struct FileInfoResponsePayload {
	uint8_t status;
	uint32_t size;
	uint32_t crc;
} __attribute__((packed));

struct FileReadRequestPayload {
	uint8_t file;
	uint32_t offset;
	uint8_t size;
} __attribute__((packed));

struct FileReadResponsePayload {
	uint8_t data[Device::kFileMaxChunkLength];
} __attribute__((packed));

struct FileWriteRequestPayload {
	uint8_t file;
	uint32_t offset;
	uint8_t data[Device::kFileMaxChunkLength];
} __attribute__((packed));

typedef SerialPacket<DeviceInfoResponsePayload> DeviceInfoResponse;
typedef SerialPacket<FieldInfoRequestPayload> FieldInfoRequest;
typedef SerialPacket<FieldInfoResponsePayload> FieldInfoResponse;
typedef SerialPacket<FieldReadRequestPayload> FieldReadRequest;
typedef SerialPacket<FieldReadResponsePayload> FieldReadResponse;
typedef SerialPacket<FieldWriteRequestPayload> FieldWriteRequest;
typedef SerialPacket<FileInfoRequestPayload> FileInfoRequest;
typedef SerialPacket<FileInfoResponsePayload> FileInfoResponse;
typedef SerialPacket<FileReadRequestPayload> FileReadRequest;
typedef SerialPacket<FileReadResponsePayload> FileReadResponse;
typedef SerialPacket<FileWriteRequestPayload> FileWriteRequest;

}

#endif // DRONEDEVICE_PAYLOADPROTOCOL_SERIALPACKET_HPP_
