//
// CoreTypes.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_CORETYPES_HPP_
#define DRONEDEVICE_CORETYPES_HPP_

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace Device {

static_assert(sizeof(bool) == 1, "Incorrect build settings");
static_assert(!std::is_same<bool, char>::value, "Incorrect build settings");
static_assert(!std::is_same<bool, signed char>::value, "Incorrect build settings");
static_assert(!std::is_same<bool, unsigned char>::value, "Incorrect build settings");

// Device types and constants
typedef std::array<uint8_t, 16> DeviceHash;
typedef uint8_t DeviceId;

static constexpr DeviceId kDeviceReservedId{std::numeric_limits<DeviceId>::max()};

// Field types and constants
typedef uint8_t FieldDimension;
typedef uint8_t FieldFlags;
typedef uint8_t FieldId;
typedef int8_t FieldScale;

static constexpr FieldFlags kFieldReadable{0x01};
static constexpr FieldFlags kFieldWritable{0x02};
static constexpr FieldFlags kFieldImportant{0x04};

static constexpr size_t kFieldMaxSize{8};
static constexpr FieldId kFieldReservedId{std::numeric_limits<FieldId>::max()};

// File types and constants
typedef uint8_t FileFlags;
typedef uint8_t FileId;

static constexpr size_t kFileMaxChunkLength{240};
static constexpr uint32_t kFileInitialChecksum{0};
static constexpr uint32_t kFileReservedPosition{std::numeric_limits<uint32_t>::max()};

// clang-format off
static constexpr FileFlags kFileReadable{0x01};
static constexpr FileFlags kFileWritable{0x02};
static constexpr FileFlags kFileReqFlagsSize{0x01};
static constexpr FileFlags kFileReqChecksum{0x02};
// clang-format on

struct Version {
	struct {
		uint8_t major;
		uint8_t minor;
	} hw;

	struct {
		uint8_t major;
		uint8_t minor;
		uint32_t hash;
		uint32_t revision;
	} sw;
};

// clang-format off
enum class FieldType: uint8_t {
	BOOL      = 0,
	CHAR      = 1,
	INT8      = 2,
	INT16     = 3,
	INT32     = 4,
	INT64     = 5,
	UINT8     = 6,
	UINT16    = 7,
	UINT32    = 8,
	UINT64    = 9,
	FLOAT     = 10,
	DOUBLE    = 11,
	UNDEFINED
};
// clang-format on

// clang-format off
enum class Result: uint8_t {
	SUCCESS             = 0,
	COMPONENT_NOT_FOUND = 1,
	FIELD_NOT_FOUND     = 2,
	FIELD_TYPE_MISMATCH = 3,
	FIELD_RANGE_ERROR   = 4,
	FIELD_ERROR         = 5,
	FIELD_READ_ONLY     = 6,
	FILE_NOT_FOUND      = 7,
	FILE_ERROR          = 8,
	FIELD_UNAVAILABLE   = 9,
	FIELD_TIMEOUT       = 10,
	FILE_ACCESS_ERROR   = 11,
	FILE_POSITION_ERROR = 12,
	FILE_TIMEOUT        = 13,
	QUEUE_ERROR         = 14,
	COMMAND_UNSUPPORTED = 15,
	COMMAND_QUEUED      = 16,
	COMMAND_ERROR       = 17,
	FILE_INCORRECT      = 18,
	FILE_OUTDATED       = 19,
	SIGNATURE_ERROR     = 20
};
// clang-format on

static inline bool operator==(const DeviceHash &a, const DeviceHash &b)
{
	return memcmp(a.begin(), b.begin(), sizeof(DeviceHash)) == 0;
}

static inline bool operator!=(const DeviceHash &a, const DeviceHash &b)
{
	return !(a == b);
}

static constexpr size_t sizeOfFieldType(FieldType aType)
{
	constexpr uint8_t sizes[] = {
		static_cast<uint8_t>(sizeof(bool)),     // FieldType::BOOL
		static_cast<uint8_t>(sizeof(char)),     // FieldType::CHAR
		static_cast<uint8_t>(sizeof(int8_t)),   // FieldType::INT8
		static_cast<uint8_t>(sizeof(int16_t)),  // FieldType::INT16
		static_cast<uint8_t>(sizeof(int32_t)),  // FieldType::INT32
		static_cast<uint8_t>(sizeof(int64_t)),  // FieldType::INT64
		static_cast<uint8_t>(sizeof(uint8_t)),  // FieldType::UINT8
		static_cast<uint8_t>(sizeof(uint16_t)), // FieldType::UINT16
		static_cast<uint8_t>(sizeof(uint32_t)), // FieldType::UINT32
		static_cast<uint8_t>(sizeof(uint64_t)), // FieldType::UINT64
		static_cast<uint8_t>(sizeof(float)),    // FieldType::FLOAT
		static_cast<uint8_t>(sizeof(double))    // FieldType::DOUBLE
	};

	return aType < FieldType::UNDEFINED ? static_cast<size_t>(sizes[static_cast<size_t>(aType)]) : 0;
}

template<typename T>
static constexpr FieldType typeToFieldType()
{
	// clang-format off
	return std::is_same<T, bool>::value  ? FieldType::BOOL   :
		std::is_same<T, char>::value     ? FieldType::CHAR   :
		std::is_same<T, uint8_t>::value  ? FieldType::UINT8  :
		std::is_same<T, uint16_t>::value ? FieldType::UINT16 :
		std::is_same<T, uint32_t>::value ? FieldType::UINT32 :
		std::is_same<T, uint64_t>::value ? FieldType::UINT64 :
		std::is_same<T, int8_t>::value   ? FieldType::INT8   :
		std::is_same<T, int16_t>::value  ? FieldType::INT16  :
		std::is_same<T, int32_t>::value  ? FieldType::INT32  :
		std::is_same<T, int64_t>::value  ? FieldType::INT64  :
		std::is_same<T, float>::value    ? FieldType::FLOAT  :
		std::is_same<T, double>::value   ? FieldType::DOUBLE :
			FieldType::UNDEFINED;
	// clang-format on
}

static constexpr bool isFloatType(FieldType aType)
{
	return aType == FieldType::FLOAT || aType == FieldType::DOUBLE;
}

static constexpr bool isIntegerType(FieldType aType)
{
	return aType >= FieldType::INT8 && aType <= FieldType::UINT64;
}

static constexpr bool isSignedType(FieldType aType)
{
	return aType >= FieldType::INT8 && aType <= FieldType::INT64;
}

static constexpr bool isUnsignedType(FieldType aType)
{
	return aType >= FieldType::UINT8 && aType <= FieldType::UINT64;
}

namespace Field {

	template<typename T>
	auto as(const void *aValue)
	{
		T value;
		memcpy(&value, aValue, sizeof(value));
		return value;
	}

} // namespace Field

} // namespace Device

#endif // DRONEDEVICE_CORETYPES_HPP_
