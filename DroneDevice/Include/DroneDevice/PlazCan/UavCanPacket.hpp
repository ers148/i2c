//
// UavCanPacket.hpp
//
//  Created on: Dec 12, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_UAVCANPACKET_HPP_
#define DRONEDEVICE_PLAZCAN_UAVCANPACKET_HPP_

#include <array>
#include <cstring>
#include <DroneDevice/CoreTypes.hpp>

namespace PlazCan {

namespace DataType {

// clang-format off
enum Message: uint16_t {
	// Standard messages
	PLAZ_ALLOCATION                  = 0,
	ALLOCATION                       = 1,
	GLOBAL_TIME_SYNC                 = 4,
	NODE_STATUS                      = 341,

	// Vendor-specific messages
	COMPOSITE_FIELD_VALUES           = 20000
};
// clang-format on

// clang-format off
enum Service: uint8_t {
	// Standard messages
	GET_NODE_INFO        = 1,
	GET_TRANSPORT_STATS  = 4,
	RESTART_NODE         = 5,
	PARAM_GET_SET        = 11,

	// Vendor-specific services
	GET_FIELDS_SUMMARY   = 200,
	GET_FIELD_INFO       = 201,
	FIELD_READ           = 202,
	FIELD_WRITE          = 203,
	GET_FILE_INFO        = 204,
	FILE_READ            = 205,
	FILE_WRITE           = 206
};
// clang-format on

} // namespace DataType

// clang-format off
static constexpr size_t kMaxCoaLength{255};
static constexpr size_t kMaxNameLength{80};

static constexpr size_t kAllocationPart0{6};
static constexpr size_t kAllocationPart1{6};
static constexpr size_t kAllocationPart2{4};
static constexpr size_t kAllocationOffset1{kAllocationPart0};
static constexpr size_t kAllocationOffset2{kAllocationOffset1 + kAllocationPart1};

static constexpr size_t kAllocationStage0{7};
static constexpr size_t kAllocationStage1{13};
static constexpr size_t kAllocationStage2{17};
// clang-format on

static constexpr size_t kUidLength{16};
using UidType = std::array<uint8_t, kUidLength>;

static inline bool operator==(const UidType &a, const UidType &b)
{
	return memcmp(a.begin(), b.begin(), kUidLength) == 0;
}

static inline bool operator!=(const UidType &a, const UidType &b)
{
	return !(a == b);
}

template<size_t count>
struct AllocationMessage {
	// clang-format off
	uint8_t first_part_of_unique_id : 1;
	uint8_t node_id                 : 7;
	// clang-format on

	uint8_t unique_id[count];
} __attribute__((packed));

struct NodeStatusMessage {
	uint32_t uptime_sec;

	// clang-format off
	uint8_t sub_mode : 3;
	uint8_t mode     : 3;
	uint8_t health   : 2;
	// clang-format on

	uint16_t vendor_specific_status_code;
} __attribute__((packed));

// clang-format off
enum class RestartNodeMagicNumber: uint64_t {
	MAGIC_DFU     = 0xC416CC1379ULL,
	MAGIC_RESTART = 0xACCE551B1EULL
};
// clang-format on

struct RestartNodeRequest {
	uint8_t magic_number[5];

	RestartNodeRequest(RestartNodeMagicNumber aMagicNumber)
	{
		// FIXME Little-endian only
		const auto buffer{static_cast<uint64_t>(aMagicNumber)};
		memcpy(magic_number, &buffer, sizeof(magic_number));
	}
} __attribute__((packed));

struct RestartNodeResponse {
	uint8_t reserved : 7;
	uint8_t ok       : 1;
} __attribute__((packed));

template<size_t coaLength>
struct HardwareVersionType {
	uint8_t major;
	uint8_t minor;
	uint8_t unique_id[kUidLength];
	uint8_t certificate_of_authenticity_length;
	uint8_t certificate_of_authenticity[coaLength];
} __attribute__((packed));

template<>
struct HardwareVersionType<0> {
	uint8_t major;
	uint8_t minor;
	uint8_t unique_id[kUidLength];
	uint8_t certificate_of_authenticity_length;
} __attribute__((packed));

struct SoftwareVersionType {
	uint8_t major;
	uint8_t minor;
	uint8_t optional_field_flags;
	uint32_t vcs_commit;
	uint64_t image_crc;
} __attribute__((packed));

struct CANIfaceStatsType {
	uint8_t frames_tx[6]; // uint48
	uint8_t frames_rx[6]; // uint48
	uint8_t errors[6]; // uint48
} __attribute__((packed));

template<size_t coaLength, size_t nameLength>
struct GetNodeInfoResponse {
	NodeStatusMessage status;
	SoftwareVersionType software_version;
	HardwareVersionType<coaLength> hardware_version;
	uint8_t name[nameLength];
} __attribute__((packed));

template<size_t count = 1>
struct TransportStatsResponse {
	uint8_t transfers_tx[6]; // uint48
	uint8_t transfers_rx[6]; // uint48
	uint8_t transfer_errors[6]; // uint48

	CANIfaceStatsType can_iface_stats[count];
} __attribute__((packed));

struct EmptyType {
};

// clang-format off
enum class ParamTag {
	TAG_EMPTY   = 0x00, // Empty
	TAG_INTEGER = 0x01, // int64
	TAG_REAL    = 0x02, // float32
	TAG_BOOLEAN = 0x03, // uint8
	TAG_STRING  = 0x04  // uint8[<=128]
};
// clang-format on

template<typename T>
static constexpr auto typeToParamTag()
{
	return std::is_same<T, int64_t>::value ? ParamTag::TAG_INTEGER :
		std::is_same<T, float>::value ? ParamTag::TAG_REAL :
		std::is_same<T, uint8_t>::value ? ParamTag::TAG_BOOLEAN :
			ParamTag::TAG_EMPTY;
}

static constexpr auto fieldTypeToParamTag(Device::FieldType type)
{
	return Device::isFloatType(type) ? ParamTag::TAG_REAL :
		type != Device::FieldType::BOOL ? ParamTag::TAG_INTEGER :
			ParamTag::TAG_BOOLEAN;
}

static constexpr Device::FieldType fieldTypeFromParamType(uint8_t tag)
{
	switch (static_cast<ParamTag>(tag)) {
		case ParamTag::TAG_INTEGER:
			return Device::FieldType::INT64;
		case ParamTag::TAG_REAL:
			return Device::FieldType::FLOAT;
		case ParamTag::TAG_BOOLEAN:
			return Device::FieldType::BOOL;
		default:
			return Device::FieldType::UNDEFINED;
	}
}

template<typename T>
struct NumericValue {
	// clang-format off
	uint8_t tag      : 2;
	uint8_t reserved : 6;
	// clang-format on

	T value;

	constexpr NumericValue():
		tag{static_cast<uint8_t>(typeToParamTag<T>()) & 0x03},
		reserved{},
		value{}
	{
	}
} __attribute__((packed));

template<>
struct NumericValue<bool> {
	// clang-format off
	uint8_t tag      : 2;
	uint8_t reserved : 6;
	// clang-format on

	constexpr NumericValue():
		tag{static_cast<uint8_t>(typeToParamTag<uint8_t>()) & 0x03},
		reserved{}
	{
	}
} __attribute__((packed));

template<>
struct NumericValue<EmptyType> {
	// clang-format off
	uint8_t tag      : 2;
	uint8_t reserved : 6;
	// clang-format on

	constexpr NumericValue():
		tag{static_cast<uint8_t>(typeToParamTag<EmptyType>()) & 0x03},
		reserved{}
	{
	}
} __attribute__((packed));

template<typename T>
struct Value {
	// clang-format off
	uint8_t tag      : 3;
	uint8_t reserved : 5;
	// clang-format on

	T value;

	constexpr Value():
		tag{static_cast<uint8_t>(typeToParamTag<T>()) & 0x07},
		reserved{},
		value{}
	{
	}
} __attribute__((packed));

template<>
struct Value<EmptyType> {
	// clang-format off
	uint8_t tag      : 3;
	uint8_t reserved : 5;
	// clang-format on

	constexpr Value():
		tag{static_cast<uint8_t>(typeToParamTag<EmptyType>()) & 0x07},
		reserved{}
	{
	}
} __attribute__((packed));

template<typename T, size_t nameLength>
struct GetSetResponse {
	Value<T> value;
	Value<EmptyType> default_value; // Default value is unused
	NumericValue<T> max_value;
	NumericValue<T> min_value;

	uint8_t name[nameLength];

	GetSetResponse(const void *aValue, const void *aMaxValue, const void *aMinValue, const char *aName):
		value{},
		default_value{},
		max_value{},
		min_value{}
	{
		memcpy(&value.value, aValue, sizeof(T));
		memcpy(&max_value.value, aMaxValue, sizeof(T));
		memcpy(&min_value.value, aMinValue, sizeof(T));
		strcpy(reinterpret_cast<char *>(name), aName);
	}
} __attribute__((packed));

template<size_t nameLength>
struct GetSetResponse<bool, nameLength> {
	Value<uint8_t> value;
	Value<EmptyType> default_value;
	NumericValue<EmptyType> max_value;
	NumericValue<EmptyType> min_value;

	uint8_t name[nameLength];

	GetSetResponse(const void *aValue, const void *, const void *, const char *aName):
		value{},
		default_value{},
		max_value{},
		min_value{}
	{
		memcpy(&value.value, aValue, sizeof(uint8_t));
		strcpy(reinterpret_cast<char *>(name), aName);
	}
} __attribute__((packed));

template<>
struct GetSetResponse<EmptyType, 0> {
	Value<EmptyType> value;
	Value<EmptyType> default_value;
	NumericValue<EmptyType> max_value;
	NumericValue<EmptyType> min_value;

	constexpr GetSetResponse():
		value{},
		default_value{},
		max_value{},
		min_value{}
	{
	}
} __attribute__((packed));

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_UAVCANPACKET_HPP_
