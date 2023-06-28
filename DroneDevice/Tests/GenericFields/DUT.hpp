//
// DUT.hpp
//
//  Created on: Nov 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_TESTS_GENERICFIELDS_DUT_HPP_
#define DRONEDEVICE_TESTS_GENERICFIELDS_DUT_HPP_

#include <DroneDevice/InternalDevice/InternalDevice.hpp>
#include <DroneDevice/InternalDevice/VolatileField.hpp>
#include <DroneDevice/InternalDevice/FieldList.hpp>

class DUT : public Device::InternalDevice {
public:
	static constexpr bool kBoolFieldDefault{true};
	static constexpr char kCharFieldDefault{'a'};
	static constexpr uint8_t kUint8FieldDefault{8};
	static constexpr uint16_t kUint16FieldDefault{16};
	static constexpr uint32_t kUint32FieldDefault{32};
	static constexpr uint64_t kUint64FieldDefault{64};
	static constexpr int8_t kInt8FieldDefault{-8};
	static constexpr int16_t kInt16FieldDefault{-16};
	static constexpr int32_t kInt32FieldDefault{-32};
	static constexpr int64_t kInt64FieldDefault{-64};
	static constexpr float kFloatFieldDefault{4.0f};
	static constexpr double kDoubleFieldDefault{8.0};
	static constexpr int kIntFieldDefault{-32};
	static constexpr std::array<int, 3> kIntVectorFieldDefault{1, 2, 3};

	DUT(const char *aAlias, const Device::Version &aVersion):
		Device::InternalDevice{aAlias, aVersion},
		boolField{"boolField", kBoolFieldDefault},
		charField0{"charField0", kCharFieldDefault},
		charField1{"charField1", kCharFieldDefault},
		uint8Field{"uint8Field", kUint8FieldDefault},
		uint16Field{"uint16Field", kUint16FieldDefault},
		uint32Field{"uint32Field", kUint32FieldDefault},
		uint64Field{"uint64Field", kUint64FieldDefault},
		int8Field{"int8Field", kInt8FieldDefault},
		int16Field{"int16Field", kInt16FieldDefault},
		int32Field{"int32Field", kInt32FieldDefault},
		int64Field{"int64Field", kInt64FieldDefault},
		floatField{"floatField", kFloatFieldDefault},
		doubleField{"doubleField", kDoubleFieldDefault},
		intField{"intField", kIntFieldDefault},
		intVectorField{"intVectorField", kIntVectorFieldDefault}
	{
	}

public:
	// Volatile fields
	Device::VolatileField<bool> boolField;

	Device::VolatileField<char> charField0;
	Device::VolatileField<char, true> charField1;

	Device::VolatileField<uint8_t> uint8Field;
	Device::VolatileField<uint16_t> uint16Field;
	Device::VolatileField<uint32_t> uint32Field;
	Device::VolatileField<uint64_t> uint64Field;
	Device::VolatileField<int8_t> int8Field;
	Device::VolatileField<int16_t> int16Field;
	Device::VolatileField<int32_t> int32Field;
	Device::VolatileField<int64_t> int64Field;
	Device::VolatileField<float> floatField;
	Device::VolatileField<double> doubleField;

	Device::VolatileField<int, false, 3> intField;
	Device::VolatileField<std::array<int, 3>, false, -6, 3> intVectorField;

private:
	constexpr auto list()
	{
		return makeFieldList(boolField,
			charField0,
			charField1,
			uint8Field,
			uint16Field,
			uint32Field,
			uint64Field,
			int8Field,
			int16Field,
			int32Field,
			int64Field,
			floatField,
			doubleField,
			intField,
			intVectorField);
	}

public:
	size_t getFieldCount() override
	{
		return list().count();
	}

	Device::AbstractField *getField(Device::FieldId aIndex) override
	{
		return list().get(aIndex);
	}
};

#endif // DRONEDEVICE_TESTS_GENERICFIELDS_DUT_HPP_
