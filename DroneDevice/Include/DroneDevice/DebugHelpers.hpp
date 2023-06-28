//
// DebugHelpers.hpp
//
//  Created on: Oct 20, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_DEBUGHELPERS_HPP_
#define DRONEDEVICE_DEBUGHELPERS_HPP_

#include <array>
#include <cstring>
#include <iostream>
#include <DroneDevice/AbstractDevice.hpp>

namespace Device {

namespace DebugHelpers {

class FieldSerializer {
public:
	FieldSerializer(FieldType aType, const void *aValue) :
		type{aType},
		value{aValue}
	{
	}

	friend std::ostream &operator<<(std::ostream &stream, const FieldSerializer &helper)
	{
		switch (helper.type) {
			case FieldType::BOOL:
				stream << (*static_cast<const bool *>(helper.value) ? "true" : "false");
				break;

			case FieldType::CHAR:
				stream << *static_cast<const char *>(helper.value);
				break;

			case FieldType::INT8:
				FieldSerializer::print<int8_t>(stream, helper.value);
				break;

			case FieldType::INT16:
				FieldSerializer::print<int16_t>(stream, helper.value);
				break;

			case FieldType::INT32:
				FieldSerializer::print<int32_t>(stream, helper.value);
				break;

			case FieldType::INT64:
				FieldSerializer::print<int64_t>(stream, helper.value);
				break;

			case FieldType::UINT8:
				FieldSerializer::print<uint8_t>(stream, helper.value);
				break;

			case FieldType::UINT16:
				FieldSerializer::print<uint16_t>(stream, helper.value);
				break;

			case FieldType::UINT32:
				FieldSerializer::print<uint32_t>(stream, helper.value);
				break;

			case FieldType::UINT64:
				FieldSerializer::print<uint64_t>(stream, helper.value);
				break;

			case FieldType::FLOAT:
				FieldSerializer::print<float>(stream, helper.value);
				break;

			case FieldType::DOUBLE:
				FieldSerializer::print<double>(stream, helper.value);
				break;

			default:
				stream << "unknown";
				break;
		}

		return stream;
	}

private:
	const FieldType type;
	const void * const value;

	template<class T>
	static void print(std::ostream &stream, const void *pointer)
	{
		T buffer;
		memcpy(&buffer, pointer, sizeof(buffer));
		stream << buffer;
	}
};

class ResultSerializer {
public:
	ResultSerializer(Result aResult) :
		result{aResult}
	{
	}

	friend std::ostream &operator<<(std::ostream &stream, const ResultSerializer &helper)
	{
		static const std::array<const char *, 21> kStrings{
			"SUCCESS",
			"COMPONENT_NOT_FOUND",
			"FIELD_NOT_FOUND",
			"FIELD_TYPE_MISMATCH",
			"FIELD_RANGE_ERROR",
			"FIELD_ERROR",
			"FIELD_READ_ONLY",
			"FILE_NOT_FOUND",
			"FILE_ERROR",
			"FIELD_UNAVAILABLE",
			"FIELD_TIMEOUT",
			"FILE_ACCESS_ERROR",
			"FILE_POSITION_ERROR",
			"FILE_TIMEOUT",
			"QUEUE_ERROR",
			"COMMAND_UNSUPPORTED",
			"COMMAND_QUEUED",
			"COMMAND_ERROR",
			"FILE_INCORRECT",
			"FILE_OUTDATED",
			"SIGNATURE_ERROR"
		};

		if (helper.result <= Result::SIGNATURE_ERROR) {
			stream << kStrings[static_cast<size_t>(helper.result)];
		} else {
			stream << "UNDEFINED";
		}

		return stream;
	}

private:
	const Result result;
};

class TypeSerializer {
public:
	TypeSerializer(FieldType aType) :
		type{aType}
	{
	}

	friend std::ostream &operator<<(std::ostream &stream, const TypeSerializer &helper)
	{
		static const std::array<const char *, 12> kStrings{
			"bool",
			"char",
			"int8",
			"int16",
			"int32",
			"int64",
			"uint8",
			"uint16",
			"uint32",
			"uint64",
			"float",
			"double"
		};

		if (helper.type < FieldType::UNDEFINED) {
			stream << kStrings[static_cast<size_t>(helper.type)];
		} else {
			stream << "undefined";
		}

		return stream;
	}

private:
	const FieldType type;
};

} // namespace DebugHelpers

} // namespace Device

#endif // DRONEDEVICE_DEBUGHELPERS_HPP_
