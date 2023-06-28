//
// DUT.hpp
//
//  Created on: Nov 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_TESTS_EXTENDEDFIELDS_DUT_HPP_
#define DRONEDEVICE_TESTS_EXTENDEDFIELDS_DUT_HPP_

#include <DroneDevice/InternalDevice/ExtendedVolatileField.hpp>
#include <DroneDevice/InternalDevice/InternalDevice.hpp>

class DUT : public Device::InternalDevice {
public:
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

	DUT(const char *aAlias, const Device::Version &aVersion) :
		Device::InternalDevice{aAlias, aVersion},
		extCharField0{"extCharField0", kCharFieldDefault, "Parrots", 'a', 'z'},
		extCharField1{"extCharField1", kCharFieldDefault, "Parrots", 'a', 'z'},
		extUint8Field{"extUint8Field", kUint8FieldDefault, "Parrots", 0, kUint8FieldDefault},
		extUint16Field{"extUint16Field", kUint16FieldDefault, "Parrots", 0, kUint16FieldDefault},
		extUint32Field{"extUint32Field", kUint32FieldDefault, "Parrots", 0, kUint32FieldDefault},
		extUint64Field{"extUint64Field", kUint64FieldDefault, "Parrots", 0, kUint64FieldDefault},
		extInt8Field{"extInt8Field", kInt8FieldDefault, "Parrots", -kInt8FieldDefault, kInt8FieldDefault},
		extInt16Field{"extInt16Field", kInt16FieldDefault, "Parrots", -kInt16FieldDefault, kInt16FieldDefault},
		extInt32Field{"extInt32Field", kInt32FieldDefault, "Parrots", -kInt32FieldDefault, kInt32FieldDefault},
		extInt64Field{"extInt64Field", kInt64FieldDefault, "Parrots", -kInt64FieldDefault, kInt64FieldDefault},
		extFloatField{"extFloatField", kFloatFieldDefault, "Parrots", -kFloatFieldDefault, kFloatFieldDefault},
		extDoubleField{"extDoubleField", kDoubleFieldDefault, "Parrots", -kDoubleFieldDefault, kDoubleFieldDefault},
		fieldsArray{{
			&extCharField0,
			&extCharField1,
			&extUint8Field,
			&extUint16Field,
			&extUint32Field,
			&extUint64Field,
			&extInt8Field,
			&extInt16Field,
			&extInt32Field,
			&extInt64Field,
			&extFloatField,
			&extDoubleField
		}}
	{
	}

	size_t getFieldCount() override
	{
		return fieldsArray.size();
	}

protected:
	Device::AbstractField *getField(Device::FieldId aIndex) override
	{
		return aIndex < fieldsArray.size() ? fieldsArray[aIndex] : nullptr;
	}

public:
	// Extended volatile fields
	Device::ExtendedVolatileField<char> extCharField0;
	Device::ExtendedVolatileField<char, true> extCharField1;
	Device::ExtendedVolatileField<uint8_t> extUint8Field;
	Device::ExtendedVolatileField<uint16_t> extUint16Field;
	Device::ExtendedVolatileField<uint32_t> extUint32Field;
	Device::ExtendedVolatileField<uint64_t> extUint64Field;
	Device::ExtendedVolatileField<int8_t> extInt8Field;
	Device::ExtendedVolatileField<int16_t> extInt16Field;
	Device::ExtendedVolatileField<int32_t> extInt32Field;
	Device::ExtendedVolatileField<int64_t> extInt64Field;
	Device::ExtendedVolatileField<float> extFloatField;
	Device::ExtendedVolatileField<double> extDoubleField;

private:
	// Field container
	const std::array<Device::AbstractField *, 12> fieldsArray;
};

#endif // DRONEDEVICE_TESTS_EXTENDEDFIELDS_DUT_HPP_
