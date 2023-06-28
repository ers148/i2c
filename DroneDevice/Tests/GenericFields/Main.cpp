//
// Main.cpp
//
//  Created on: Nov 15, 2018
//      Author: Alexander
//

#include <climits>
#include <cmath>
#include "gtest/gtest.h"
#include "DUT.hpp"

static constexpr Device::Version kDeviceVersion{{1, 2}, {3, 4, 0xCAFEFEED, 12345}};

// Tests reading of default values from generic volatile fields
TEST(VolatileFieldTest, StaticProperties)
{
	DUT dut{"DUT", kDeviceVersion};

	// One-time checks for all volatile fields
	ASSERT_EQ(dut.boolField.unit(), nullptr);

	// One-time checks for single-dimension fields
	ASSERT_EQ(dut.boolField.dimension(), 1);

	// One-time check for fields with default scale
	ASSERT_EQ(dut.boolField.scale(), 0);

	// Checks for name, min, max and type values
	ASSERT_STREQ(dut.boolField.name(), "boolField");
	ASSERT_EQ(*static_cast<const bool *>(dut.boolField.max()), true);
	ASSERT_EQ(*static_cast<const bool *>(dut.boolField.min()), false);
	ASSERT_EQ(dut.boolField.type(), Device::FieldType::BOOL);

	ASSERT_STREQ(dut.charField0.name(), "charField0");
	ASSERT_EQ(*static_cast<const char *>(dut.charField0.max()), std::numeric_limits<char>::max());
	ASSERT_EQ(*static_cast<const char *>(dut.charField0.min()), std::numeric_limits<char>::min());
	ASSERT_EQ(dut.charField0.type(), Device::FieldType::CHAR);

	ASSERT_STREQ(dut.uint8Field.name(), "uint8Field");
	ASSERT_EQ(*static_cast<const uint8_t *>(dut.uint8Field.max()), std::numeric_limits<uint8_t>::max());
	ASSERT_EQ(*static_cast<const uint8_t *>(dut.uint8Field.min()), std::numeric_limits<uint8_t>::min());
	ASSERT_EQ(dut.uint8Field.type(), Device::FieldType::UINT8);

	ASSERT_STREQ(dut.uint16Field.name(), "uint16Field");
	ASSERT_EQ(*static_cast<const uint16_t *>(dut.uint16Field.max()), std::numeric_limits<uint16_t>::max());
	ASSERT_EQ(*static_cast<const uint16_t *>(dut.uint16Field.min()), std::numeric_limits<uint16_t>::min());
	ASSERT_EQ(dut.uint16Field.type(), Device::FieldType::UINT16);

	ASSERT_STREQ(dut.uint32Field.name(), "uint32Field");
	ASSERT_EQ(*static_cast<const uint32_t *>(dut.uint32Field.max()), std::numeric_limits<uint32_t>::max());
	ASSERT_EQ(*static_cast<const uint32_t *>(dut.uint32Field.min()), std::numeric_limits<uint32_t>::min());
	ASSERT_EQ(dut.uint32Field.type(), Device::FieldType::UINT32);

	ASSERT_STREQ(dut.uint64Field.name(), "uint64Field");
	ASSERT_EQ(*static_cast<const uint64_t *>(dut.uint64Field.max()), std::numeric_limits<uint64_t>::max());
	ASSERT_EQ(*static_cast<const uint64_t *>(dut.uint64Field.min()), std::numeric_limits<uint64_t>::min());
	ASSERT_EQ(dut.uint64Field.type(), Device::FieldType::UINT64);

	ASSERT_STREQ(dut.int8Field.name(), "int8Field");
	ASSERT_EQ(*static_cast<const int8_t *>(dut.int8Field.max()), std::numeric_limits<int8_t>::max());
	ASSERT_EQ(*static_cast<const int8_t *>(dut.int8Field.min()), std::numeric_limits<int8_t>::min());
	ASSERT_EQ(dut.int8Field.type(), Device::FieldType::INT8);

	ASSERT_STREQ(dut.int16Field.name(), "int16Field");
	ASSERT_EQ(*static_cast<const int16_t *>(dut.int16Field.max()), std::numeric_limits<int16_t>::max());
	ASSERT_EQ(*static_cast<const int16_t *>(dut.int16Field.min()), std::numeric_limits<int16_t>::min());
	ASSERT_EQ(dut.int16Field.type(), Device::FieldType::INT16);

	ASSERT_STREQ(dut.int32Field.name(), "int32Field");
	ASSERT_EQ(*static_cast<const int32_t *>(dut.int32Field.max()), std::numeric_limits<int32_t>::max());
	ASSERT_EQ(*static_cast<const int32_t *>(dut.int32Field.min()), std::numeric_limits<int32_t>::min());
	ASSERT_EQ(dut.int32Field.type(), Device::FieldType::INT32);

	ASSERT_STREQ(dut.int64Field.name(), "int64Field");
	ASSERT_EQ(*static_cast<const int64_t *>(dut.int64Field.max()), std::numeric_limits<int64_t>::max());
	ASSERT_EQ(*static_cast<const int64_t *>(dut.int64Field.min()), std::numeric_limits<int64_t>::min());
	ASSERT_EQ(dut.int64Field.type(), Device::FieldType::INT64);

	ASSERT_STREQ(dut.floatField.name(), "floatField");
	ASSERT_EQ(*static_cast<const float *>(dut.floatField.max()), std::numeric_limits<float>::max());
	ASSERT_EQ(*static_cast<const float *>(dut.floatField.min()), -std::numeric_limits<float>::max());
	ASSERT_EQ(dut.floatField.type(), Device::FieldType::FLOAT);

	ASSERT_STREQ(dut.doubleField.name(), "doubleField");
	ASSERT_EQ(*static_cast<const double *>(dut.doubleField.max()), std::numeric_limits<double>::max());
	ASSERT_EQ(*static_cast<const double *>(dut.doubleField.min()), -std::numeric_limits<double>::max());
	ASSERT_EQ(dut.doubleField.type(), Device::FieldType::DOUBLE);

	// Field with custom scale
	ASSERT_EQ(dut.intField.scale(), 3);
	ASSERT_EQ(dut.intField.type(), Device::typeToFieldType<int>());

	// Field with custom scale and dimension
	ASSERT_EQ(dut.intVectorField.dimension(), 3);
	ASSERT_EQ(dut.intVectorField.scale(), -6);
//	ASSERT_EQ(dut.intVectorField.type(), Device::typeToFieldType<int>()); // TODO
}

// Tests reading of default values from generic volatile fields
TEST(VolatileFieldTest, DefaultValues)
{
	DUT dut{"DUT", kDeviceVersion};

	ASSERT_EQ(static_cast<bool>(dut.boolField), DUT::kBoolFieldDefault);
	ASSERT_EQ(static_cast<char>(dut.charField0), DUT::kCharFieldDefault);
	ASSERT_EQ(static_cast<char>(dut.charField1), DUT::kCharFieldDefault);
	ASSERT_EQ(static_cast<uint8_t>(dut.uint8Field), DUT::kUint8FieldDefault);
	ASSERT_EQ(static_cast<uint16_t>(dut.uint16Field), DUT::kUint16FieldDefault);
	ASSERT_EQ(static_cast<uint32_t>(dut.uint32Field), DUT::kUint32FieldDefault);
	ASSERT_EQ(static_cast<uint64_t>(dut.uint64Field), DUT::kUint64FieldDefault);
	ASSERT_EQ(static_cast<int8_t>(dut.int8Field), DUT::kInt8FieldDefault);
	ASSERT_EQ(static_cast<int16_t>(dut.int16Field), DUT::kInt16FieldDefault);
	ASSERT_EQ(static_cast<int32_t>(dut.int32Field), DUT::kInt32FieldDefault);
	ASSERT_EQ(static_cast<int64_t>(dut.int64Field), DUT::kInt64FieldDefault);
	ASSERT_EQ(static_cast<float>(dut.floatField), DUT::kFloatFieldDefault);
	ASSERT_EQ(static_cast<double>(dut.doubleField), DUT::kDoubleFieldDefault);
	ASSERT_EQ(static_cast<int>(dut.intField), DUT::kInt32FieldDefault);

	const std::array<int, 3> intVectorValue = dut.intVectorField;
	ASSERT_TRUE(std::equal(intVectorValue.begin(), intVectorValue.end(),
		DUT::kIntVectorFieldDefault.begin()));
}

// Tests reading of default values from generic volatile fields
TEST(VolatileFieldTest, flags)
{
	DUT dut{"DUT", kDeviceVersion};

	ASSERT_EQ(dut.boolField.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.charField0.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.charField1.flags(), Device::kFieldReadable | Device::kFieldImportant);
	ASSERT_EQ(dut.uint8Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.uint16Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.uint32Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.uint64Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.int8Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.int16Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.int32Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.int64Field.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.floatField.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.doubleField.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.intField.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
	ASSERT_EQ(dut.intVectorField.flags(), Device::kFieldReadable | Device::kFieldWritable | Device::kFieldImportant);
}

// Tests reading of default values from generic volatile fields
TEST(VolatileFieldTest, DirectWrite)
{
	DUT dut{"DUT", kDeviceVersion};

	static_assert(DUT::kCharFieldDefault != 'z', "Incorrect default value");

	// Test one of the fields, it is redundant to test all types of standard volatile fields
	dut.charField0 = 'z';
	ASSERT_EQ(static_cast<char>(dut.charField0), 'z');

	// Test direct writing to the read-only field
	dut.charField1 = 'z';
	ASSERT_EQ(static_cast<char>(dut.charField1), 'z');
}

// Tests reading and writing of char field with type-erased functions
TEST(VolatileFieldTest, ReadWriteChar)
{
	DUT dut{"DUT", kDeviceVersion};

	static const char output = 'z';
	static_assert(DUT::kCharFieldDefault != 'z', "Incorrect default value");

	char input;
	Device::Result result;

	// Test writing to a writable field
	result = dut.charField0.write(&output);
	dut.charField0.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_EQ(input, output);

	// Test writing to a read-only field
	result = dut.charField1.write(&output);
	dut.charField1.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_READ_ONLY);
	ASSERT_EQ(input, DUT::kCharFieldDefault);
}

// Tests reading and writing of double field with type-erased functions
TEST(VolatileFieldTest, ReadWriteDouble)
{
	DUT dut{"DUT", kDeviceVersion};

	double input;
	double output;
	Device::Result result;

	// Test writing of positive value
	output = DUT::kDoubleFieldDefault / 3.0;
	result = dut.doubleField.write(&output);
	dut.doubleField.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_EQ(input, output);

	// Test writing of negative value
	output = -DUT::kDoubleFieldDefault * M_PI;
	result = dut.doubleField.write(&output);
	dut.doubleField.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_EQ(input, output);

	// Test writing of infinity
	output = std::numeric_limits<double>::infinity();
	result = dut.doubleField.write(&output);
	dut.doubleField.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_TRUE(std::isinf(input));

	// Test writing of NaN
	output = std::numeric_limits<double>::quiet_NaN();
	result = dut.doubleField.write(&output);
	dut.doubleField.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_TRUE(std::isnan(input));
}

// Tests writing of overflowing values with type-erased function
TEST(VolatileFieldTest, ReadWriteOverflow)
{
	DUT dut{"DUT", kDeviceVersion};

	static const uint64_t output = 0xFFEEDDCCBBAA9988ULL;
	static const uint64_t truncatedOutput = output & 0xFFFFFFFFULL;

	uint32_t input;
	Device::Result result;

	result = dut.uint32Field.write(&output);
	dut.uint32Field.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_EQ(input, truncatedOutput);
}

// Tests reading and writing file
TEST(VolatileFieldTest, FieldAcess)
{
	DUT dut{"DUT", kDeviceVersion};

	ASSERT_EQ(15, dut.getFieldCount());

	ASSERT_EQ(&dut.boolField, dut.getField(0));
	ASSERT_EQ(&dut.charField0, dut.getField(1));
	ASSERT_EQ(&dut.charField1, dut.getField(2));
	ASSERT_EQ(&dut.uint8Field, dut.getField(3));
	ASSERT_EQ(&dut.uint16Field, dut.getField(4));
	ASSERT_EQ(&dut.uint32Field, dut.getField(5));
	ASSERT_EQ(&dut.uint64Field, dut.getField(6));
	ASSERT_EQ(&dut.int8Field, dut.getField(7));
	ASSERT_EQ(&dut.int16Field, dut.getField(8));
	ASSERT_EQ(&dut.int32Field, dut.getField(9));
	ASSERT_EQ(&dut.int64Field, dut.getField(10));
	ASSERT_EQ(&dut.floatField, dut.getField(11));
	ASSERT_EQ(&dut.doubleField, dut.getField(12));
	ASSERT_EQ(&dut.intField, dut.getField(13));
	ASSERT_EQ(&dut.intVectorField, dut.getField(14));
}
