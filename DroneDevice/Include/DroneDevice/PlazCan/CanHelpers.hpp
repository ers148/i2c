//
// CanHelpers.hpp
//
//  Created on: Dec 18, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_CANHELPERS_HPP_
#define DRONEDEVICE_PLAZCAN_CANHELPERS_HPP_

#include <cstring>
#include <DroneDevice/PlazCan/UavCanPacket.hpp>
#include <DroneDevice/PlazCan/CanardWrapper.hpp>

#if defined(__arm__)
#include <arm_fp16.h>
#endif

namespace PlazCan {

namespace CanHelpersPrivate {

template<typename A, typename B>
static inline void convertTypeUnsafely(const void *input, void *output)
{
	A a;
	memcpy(&a, input, sizeof(a));

	const B b = static_cast<B>(a);
	memcpy(output, &b, sizeof(b));
}

static inline void doubleToFloat(const void *input, void *output)
{
	double a;
	memcpy(&a, input, sizeof(a));

	float b;

	if (a < static_cast<double>(-std::numeric_limits<float>::max())) {
		b = -std::numeric_limits<float>::max();
	} else if (a > static_cast<double>(std::numeric_limits<float>::max())) {
		b = std::numeric_limits<float>::max();
	} else {
		b = static_cast<float>(a);
	}

	memcpy(output, &b, sizeof(b));
}

static inline void upconvertInteger(const void *input, void *output, Device::FieldType type)
{
	const size_t size = Device::sizeOfFieldType(type);
	uint64_t value = 0;

	memcpy(&value, input, size);

	if (size < sizeof(value) && Device::isSignedType(type)) {
		const uint64_t mask = 1ULL << (size * 8);

		if (value & mask) {
			value |= ~(mask - 1);
		}
	}

	memcpy(output, &value, sizeof(value));
}

static inline void downconvertInteger(const void *input, void *output, Device::FieldType type)
{
	const size_t size = Device::sizeOfFieldType(type);
	memcpy(output, input, size);
}

}

namespace CanHelpers {

static constexpr size_t kMaxFieldNameLength{24};
static constexpr size_t kMaxUnitNameLength{16};

template<typename T, typename U>
static inline constexpr size_t packetMemberOffset(U T::*member)
{
	return static_cast<size_t>(reinterpret_cast<const char *>(&(static_cast<const T *>(nullptr)->*member))
		- static_cast<const char *>(nullptr));
}

template<typename T>
static inline T decodeAs(const CanardRxTransfer *transfer, size_t offset = 0)
{
	T result;
	auto overlay = reinterpret_cast<uint8_t *>(&result);

	for (size_t i = offset; i < offset + sizeof(T); ++i) {
		canardDecodeScalar(transfer, static_cast<uint32_t>(i * 8), 8, false, overlay++);
	}

	return result;
}

template<typename T>
static inline void decodeBlobField(const CanardRxTransfer *transfer, size_t offset, size_t length, T *buffer)
{
	const auto maxOffset = offset + length;

	for (size_t i = offset; i < maxOffset; ++i) {
		canardDecodeScalar(transfer, static_cast<uint32_t>((8 * sizeof(T)) * i),
			static_cast<uint8_t>(8 * sizeof(T)), false, buffer++);
	}
}

template<typename T, typename U>
__attribute__((deprecated)) static inline void decodeIntegerField(const CanardRxTransfer *transfer,
	U T::*member, U *buffer)
{
	canardDecodeScalar(transfer, static_cast<uint32_t>(8 * packetMemberOffset(member)),
		static_cast<uint8_t>(8 * sizeof(U)), std::is_signed<U>::value, buffer);
}

template<typename U>
__attribute__((deprecated)) static inline void decodeIntegerField(const CanardRxTransfer *transfer,
	size_t offset, uint8_t length, U *buffer)
{
	canardDecodeScalar(transfer, static_cast<uint32_t>(offset), length, std::is_signed<U>::value, buffer);
}

template<typename T, typename U>
static inline U decodeIntegerField(const CanardRxTransfer *transfer, U T::*member)
{
	U buffer{};

	canardDecodeScalar(transfer, static_cast<uint32_t>(8 * packetMemberOffset(member)),
		static_cast<uint8_t>(8 * sizeof(U)), std::is_signed<U>::value, &buffer);
	return buffer;
}

template<typename U>
static inline U decodeIntegerField(const CanardRxTransfer *transfer, size_t byteOffset)
{
	U buffer{};

	canardDecodeScalar(transfer, static_cast<uint32_t>(8 * byteOffset), static_cast<uint8_t>(8 * sizeof(U)),
		std::is_signed<U>::value, &buffer);
	return buffer;
}

template<typename U>
static inline U decodeIntegerField(const CanardRxTransfer *transfer, size_t bitOffset, size_t bitLength)
{
	U buffer{};

	canardDecodeScalar(transfer, static_cast<uint32_t>(bitOffset), static_cast<uint8_t>(bitLength),
		std::is_signed<U>::value, &buffer);
	return buffer;
}

static inline void encodeIntegerField(void *packet, size_t bitOffset, uint8_t bitLength, const void *buffer)
{
	canardEncodeScalar(packet, static_cast<uint32_t>(bitOffset), bitLength, buffer);
}

template<typename U>
static inline void encodeIntegerField(void *packet, size_t bitOffset, uint8_t bitLength, const U &value)
{
	static_assert(std::is_arithmetic<U>::value, "Incorrect argument type");
	canardEncodeScalar(packet, static_cast<uint32_t>(bitOffset), bitLength, &value);
}

template<size_t size>
static inline size_t packFieldInfoImpl(uint8_t *buffer, const char *name, const char *unit,
	const void *min, const void *max)
{
	const size_t minValueLength = min == nullptr ? 0 : size;
	const size_t maxValueLength = max == nullptr ? 0 : size;
	const size_t unitLength = unit == nullptr ? 0 : strlen(unit);
	const size_t nameLength = name == nullptr ? 0 : strlen(name);
	size_t position = 0;

	if (unitLength > kMaxUnitNameLength || nameLength > kMaxFieldNameLength) {
		return 0;
	}

	// Copy min value
	*(buffer + position) = static_cast<uint8_t>(minValueLength);
	if (minValueLength) {
		memcpy(buffer + position + 1, min, size);
	}
	position += 1 + minValueLength;

	// Copy max value
	*(buffer + position) = static_cast<uint8_t>(maxValueLength);
	if (maxValueLength) {
		memcpy(buffer + position + 1, max, size);
	}
	position += 1 + maxValueLength;

	// Copy unit name
	*(buffer + position) = static_cast<uint8_t>(unitLength);
	if (unitLength) {
		memcpy(buffer + position + 1, unit, unitLength);
	}
	position += 1 + unitLength;

	// Copy field name
	if (nameLength) {
		memcpy(buffer + position, name, nameLength);
	}

	return 3 + minValueLength + maxValueLength + unitLength + nameLength;
}

static inline size_t packFieldInfo(void *buffer, Device::FieldType type, const char *name, const char *unit,
	const void *min, const void *max)
{
	switch (type) {
		case Device::FieldType::BOOL:
		case Device::FieldType::CHAR:
		case Device::FieldType::INT8:
		case Device::FieldType::UINT8:
			return packFieldInfoImpl<1>(static_cast<uint8_t *>(buffer), name, unit, min, max);

		case Device::FieldType::INT16:
		case Device::FieldType::UINT16:
			return packFieldInfoImpl<2>(static_cast<uint8_t *>(buffer), name, unit, min, max);

		case Device::FieldType::INT32:
		case Device::FieldType::UINT32:
		case Device::FieldType::FLOAT:
			return packFieldInfoImpl<4>(static_cast<uint8_t *>(buffer), name, unit, min, max);

		case Device::FieldType::INT64:
		case Device::FieldType::UINT64:
		case Device::FieldType::DOUBLE:
			return packFieldInfoImpl<8>(static_cast<uint8_t *>(buffer), name, unit, min, max);

		default:
			return 0;
	}
}

static inline void fillMinMaxValues(void *min, void *max, ParamTag tag)
{
	static const int64_t integerPattern[2] = {
		std::numeric_limits<int64_t>::min(),
		std::numeric_limits<int64_t>::max()
	};
	static const float floatPattern[2] = {
		-std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max()
	};

	if (tag == ParamTag::TAG_REAL) {
		memcpy(min, &floatPattern[0], sizeof(float));
		memcpy(max, &floatPattern[1], sizeof(float));
	} else if (tag == ParamTag::TAG_INTEGER) {
		memcpy(min, &integerPattern[0], sizeof(int64_t));
		memcpy(max, &integerPattern[1], sizeof(int64_t));
	}
}

static inline void fromUavCan(void *outputBuffer, const void *inputBuffer, Device::FieldType type)
{
	if (type == Device::FieldType::FLOAT) {
		memcpy(outputBuffer, inputBuffer, sizeof(float));
	} else if (type == Device::FieldType::DOUBLE) {
		CanHelpersPrivate::convertTypeUnsafely<float, double>(inputBuffer, outputBuffer);
	} else if (type == Device::FieldType::BOOL) {
		CanHelpersPrivate::convertTypeUnsafely<uint8_t, bool>(inputBuffer, outputBuffer);
	} else {
		CanHelpersPrivate::downconvertInteger(inputBuffer, outputBuffer, type);
	}
}

static inline void toUavCan(void *outputBuffer, const void *inputBuffer, Device::FieldType type)
{
	if (type == Device::FieldType::FLOAT) {
		memcpy(outputBuffer, inputBuffer, sizeof(float));
	} else if (type == Device::FieldType::DOUBLE) {
		CanHelpersPrivate::doubleToFloat(inputBuffer, outputBuffer);
	} else if (type == Device::FieldType::BOOL) {
		CanHelpersPrivate::convertTypeUnsafely<bool, uint8_t>(inputBuffer, outputBuffer);
	} else {
		CanHelpersPrivate::upconvertInteger(inputBuffer, outputBuffer, type);
	}
}

#if defined(__arm__)
static inline float float16to32(uint16_t uint_fp16)
{
	auto builtin_fp16{reinterpret_cast<__fp16 *>(&uint_fp16)};
	return static_cast<float>(*builtin_fp16);
}

static inline uint16_t float32to16(float fp32)
{
	auto builtin_fp16 = static_cast<__fp16>(fp32);
	auto *t = (reinterpret_cast<uint16_t *>(&builtin_fp16)); // avoid strict-aliasing warning
	return *t;
}
#else
static inline uint16_t float32to16(float value)
{
	union FP32 {
		uint32_t u;
		float f;
	};

	const union FP32 f32inf = {255UL << 23};
	const union FP32 f16inf = {31UL << 23};
	const union FP32 magic = {15UL << 23};
	const uint32_t sign_mask = 0x80000000U;
	const uint32_t round_mask = ~0xFFFU;

	union FP32 in;
	in.f = value;
	uint32_t sign = in.u & sign_mask;
	in.u ^= sign;

	uint16_t out = 0;

	if (in.u >= f32inf.u) {
		out = (in.u > f32inf.u) ? static_cast<uint16_t>(0x7FFFU) : static_cast<uint16_t>(0x7C00U);
	} else {
		in.u &= round_mask;
		in.f *= magic.f;
		in.u -= round_mask;
		if (in.u > f16inf.u) {
			in.u = f16inf.u;
		}
		out = static_cast<uint16_t>(in.u >> 13);
	}

	out = static_cast<uint16_t>(out | (sign >> 16));

	return out;
}

static inline float float16to32(uint16_t value)
{
	union FP32 {
		uint32_t u;
		float f;
	};

	const union FP32 magic = {(254UL - 15UL) << 23};
	const union FP32 was_inf_nan = {(127UL + 16UL) << 23};
	union FP32 out;

	out.u = (value & 0x7FFFU) << 13;
	out.f *= magic.f;
	if (out.f >= was_inf_nan.f) {
		out.u |= 255UL << 23;
	}
	out.u = static_cast<uint32_t>(out.u | ((value & 0x8000UL) << 16));

	return out.f;
}
#endif

}

}

#endif // DRONEDEVICE_PLAZCAN_CANHELPERS_HPP_
