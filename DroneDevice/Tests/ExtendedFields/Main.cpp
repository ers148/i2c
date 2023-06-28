//
// Main.cpp
//
//  Created on: Nov 19, 2018
//      Author: Alexander
//

#include <climits>
#include <cmath>
#include "gtest/gtest.h"
#include "DUT.hpp"

static const char kDeviceName[] = "DUT";
static constexpr Device::Version kDeviceVersion{{1, 2}, {3, 4, 0xCAFEFEED, 12345}};

// Tests reading of default values from generic volatile fields
TEST(ExtendedFieldTest, StaticProperties)
{
	DUT dut{"DUT", kDeviceVersion};

	// Check accessibility of base class functions
	ASSERT_STREQ(dut.extCharField0.name(), "extCharField0");
	ASSERT_STREQ(dut.extCharField0.unit(), "Parrots");
	ASSERT_EQ(dut.extCharField0.dimension(), 1);
	ASSERT_EQ(dut.extCharField0.scale(), 0);
	ASSERT_EQ(dut.extCharField0.type(), Device::FieldType::CHAR);
	ASSERT_EQ(static_cast<char>(dut.extCharField0), DUT::kCharFieldDefault);

	// Checks for min and max values
	ASSERT_EQ(*static_cast<const char *>(dut.extCharField0.max()), 'z');
	ASSERT_EQ(*static_cast<const char *>(dut.extCharField0.min()), 'a');

	ASSERT_EQ(*static_cast<const uint8_t *>(dut.extUint8Field.max()), DUT::kUint8FieldDefault);
	ASSERT_EQ(*static_cast<const uint8_t *>(dut.extUint8Field.min()), 0);

	ASSERT_EQ(*static_cast<const uint16_t *>(dut.extUint16Field.max()), DUT::kUint16FieldDefault);
	ASSERT_EQ(*static_cast<const uint16_t *>(dut.extUint16Field.min()), 0);

	ASSERT_EQ(*static_cast<const uint32_t *>(dut.extUint32Field.max()), DUT::kUint32FieldDefault);
	ASSERT_EQ(*static_cast<const uint32_t *>(dut.extUint32Field.min()), 0);

	ASSERT_EQ(*static_cast<const uint64_t *>(dut.extUint64Field.max()), DUT::kUint64FieldDefault);
	ASSERT_EQ(*static_cast<const uint64_t *>(dut.extUint64Field.min()), 0);

	ASSERT_EQ(*static_cast<const int8_t *>(dut.extInt8Field.max()), DUT::kInt8FieldDefault);
	ASSERT_EQ(*static_cast<const int8_t *>(dut.extInt8Field.min()), -DUT::kInt8FieldDefault);

	ASSERT_EQ(*static_cast<const int16_t *>(dut.extInt16Field.max()), DUT::kInt16FieldDefault);
	ASSERT_EQ(*static_cast<const int16_t *>(dut.extInt16Field.min()), -DUT::kInt16FieldDefault);

	ASSERT_EQ(*static_cast<const int32_t *>(dut.extInt32Field.max()), DUT::kInt32FieldDefault);
	ASSERT_EQ(*static_cast<const int32_t *>(dut.extInt32Field.min()), -DUT::kInt32FieldDefault);

	ASSERT_EQ(*static_cast<const int64_t *>(dut.extInt64Field.max()), DUT::kInt64FieldDefault);
	ASSERT_EQ(*static_cast<const int64_t *>(dut.extInt64Field.min()), -DUT::kInt64FieldDefault);

	ASSERT_EQ(*static_cast<const float *>(dut.extFloatField.max()), DUT::kFloatFieldDefault);
	ASSERT_EQ(*static_cast<const float *>(dut.extFloatField.min()), -DUT::kFloatFieldDefault);

	ASSERT_EQ(*static_cast<const double *>(dut.extDoubleField.max()), DUT::kDoubleFieldDefault);
	ASSERT_EQ(*static_cast<const double *>(dut.extDoubleField.min()), -DUT::kDoubleFieldDefault);
}

// Tests reading and writing of char fields with type-erased functions
TEST(ExtendedFieldTest, ReadWriteChar)
{
	DUT dut{"DUT", kDeviceVersion};

	static_assert(DUT::kCharFieldDefault != 'n', "Incorrect default value");
	char input;
	char output;
	Device::Result result;

	// Test writing of correct value to a writable field
	output = 'n';
	result = dut.extCharField0.write(&output);
	dut.extCharField0.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_EQ(input, output);

	// Reset to the default value
	dut.extCharField0 = DUT::kCharFieldDefault;

	// Test writing of a value that is greater than max()
	output = '~';
	result = dut.extCharField0.write(&output);
	dut.extCharField0.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_RANGE_ERROR);
	ASSERT_EQ(input, DUT::kCharFieldDefault);

	// Test writing of a value that is lower than max()
	output = '\0';
	result = dut.extCharField0.write(&output);
	dut.extCharField0.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_RANGE_ERROR);
	ASSERT_EQ(input, DUT::kCharFieldDefault);

	// Test writing to a read-only field
	output = 'n';
	result = dut.extCharField1.write(&output);
	dut.extCharField1.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_READ_ONLY);
	ASSERT_EQ(input, DUT::kCharFieldDefault);
}

// Tests reading and writing of double fields with type-erased functions
TEST(ExtendedFieldTest, ReadWriteDouble)
{
	DUT dut{"DUT", kDeviceVersion};

	double input;
	double output;
	Device::Result result;

	// Test writing of correct positive value
	output = DUT::kDoubleFieldDefault / 3.0;
	result = dut.extDoubleField.write(&output);
	dut.extDoubleField.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_EQ(input, output);

	// Test writing of correct negative value
	output = -DUT::kDoubleFieldDefault / 3.0;
	result = dut.extDoubleField.write(&output);
	dut.extDoubleField.read(&input);
	ASSERT_EQ(result, Device::Result::SUCCESS);
	ASSERT_EQ(input, output);

	// Reset to the default value
	dut.extDoubleField = DUT::kDoubleFieldDefault;

	// Test writing of incorrect positive value
	output = DUT::kDoubleFieldDefault * M_PI;
	result = dut.extDoubleField.write(&output);
	dut.extDoubleField.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_RANGE_ERROR);
	ASSERT_EQ(input, DUT::kDoubleFieldDefault);

	// Test writing of incorrect negative value
	output = -DUT::kDoubleFieldDefault * M_PI;
	result = dut.extDoubleField.write(&output);
	dut.extDoubleField.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_RANGE_ERROR);
	ASSERT_EQ(input, DUT::kDoubleFieldDefault);

	// Test writing of infinity
	output = std::numeric_limits<double>::infinity();
	result = dut.extDoubleField.write(&output);
	dut.extDoubleField.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_RANGE_ERROR);
	ASSERT_FALSE(std::isinf(input));

	// Test writing of NaN
	output = std::numeric_limits<double>::quiet_NaN();
	result = dut.extDoubleField.write(&output);
	dut.extDoubleField.read(&input);
	ASSERT_EQ(result, Device::Result::FIELD_RANGE_ERROR);
	ASSERT_FALSE(std::isnan(input));
}
