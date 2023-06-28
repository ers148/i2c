//
// Main.cpp
//
//  Created on: Nov 15, 2018
//      Author: Alexander
//

#include <DroneDevice/ConfigStorage.hpp>
#include <DroneDevice/Crc32.hpp>
#include "gtest/gtest.h"
#include "DUT.hpp"
#include "MockConfig.hpp"
#include "MockMemory.hpp"

static const char kDeviceName[] = "DUT";
static constexpr Device::Version kDeviceVersion{{1, 2}, {3, 4, 0xCAFEFEED, 12345}};

using ConfigMemory = MockMemory<1024>;
using ConfigType = ConfigStorage<ConfigMemory, Crc32, MockConfig, AlignedConfig, UnalignedConfig>;

// Tests reading of default values from generic variable fields
TEST(ConfigFieldTest, StaticProperties)
{
	static_assert(sizeof(AlignedConfig) % sizeof(uint32_t) == 0, "Incorrect config");
	static_assert(sizeof(UnalignedConfig) % sizeof(uint32_t) != 0, "Incorrect config");

	ConfigType storage;

	DUT<ConfigType, AlignedConfig> dut0{"DUT", kDeviceVersion, storage};
	DUT<ConfigType, UnalignedConfig> dut1{"DUT", kDeviceVersion, storage};

	// One-time check for all config fields
	ASSERT_EQ(dut0.coeff0Field.unit(), nullptr);

	// One-time check for single-dimension fields
	ASSERT_EQ(dut0.coeff0Field.dimension(), 1);

	// One-time check for fields with default scale
	ASSERT_EQ(dut0.coeff0Field.scale(), 0);

	// Check names
	ASSERT_STREQ(dut0.coeff0Field.name(), "coeff0Field");
	ASSERT_STREQ(dut0.coeff1Field.name(), "coeff1Field");
	ASSERT_STREQ(dut0.coeff2Field.name(), "coeff2Field");
	ASSERT_STREQ(dut0.coeff3Field.name(), "coeff3Field");
	ASSERT_STREQ(dut0.arrayCoeffField.name(), "arrayCoeffField");

	// Check types of aligned config
	ASSERT_EQ(dut0.coeff0Field.type(), Device::FieldType::DOUBLE);
	ASSERT_EQ(dut0.coeff1Field.type(), Device::FieldType::UINT32);
	ASSERT_EQ(dut0.coeff2Field.type(), Device::FieldType::INT32);
	ASSERT_EQ(dut0.coeff3Field.type(), Device::FieldType::INT64);

	// Check types of unaligned config
	ASSERT_EQ(dut1.coeff0Field.type(), Device::FieldType::FLOAT);
	ASSERT_EQ(dut1.coeff1Field.type(), Device::FieldType::UINT32);
	ASSERT_EQ(dut1.coeff2Field.type(), Device::FieldType::UINT32);
	ASSERT_EQ(dut1.coeff3Field.type(), Device::FieldType::INT8);

	// Field with custom scale
	ASSERT_EQ(dut0.coeff3Field.scale(), 3);

	// Field with custom scale and dimension
	ASSERT_EQ(dut0.arrayCoeffField.dimension(), 2);
	ASSERT_EQ(dut0.arrayCoeffField.scale(), -6);
}

// Tests reading of default values using direct access
TEST(ConfigFieldTest, DefaultValues)
{
	ConfigType storage;

	// Reset memory, default values will be used
	ConfigMemory::erase();
	storage.load();

	DUT<ConfigType, AlignedConfig> dut0{"DUT", kDeviceVersion, storage};
	DUT<ConfigType, UnalignedConfig> dut1{"DUT", kDeviceVersion, storage};

	// Check default values of aligned config
	ASSERT_EQ(static_cast<double>(dut0.coeff0Field), 1024.0);
	ASSERT_EQ(static_cast<uint32_t>(dut0.coeff1Field), 131072);
	ASSERT_EQ(static_cast<int32_t>(dut0.coeff2Field), 1024);
	ASSERT_EQ(static_cast<int64_t>(dut0.coeff3Field), 64);

	const std::array<uint32_t, 2> arrayCoeffValue = dut0.arrayCoeffField;
	ASSERT_EQ(arrayCoeffValue[0], 1);
	ASSERT_EQ(arrayCoeffValue[1], 2);

	// Check default values of unaligned config
	ASSERT_EQ(static_cast<float>(dut1.coeff0Field), 512.0);
	ASSERT_EQ(static_cast<uint32_t>(dut1.coeff1Field), 131072);
	ASSERT_EQ(static_cast<uint32_t>(dut1.coeff2Field), 512);
	ASSERT_EQ(static_cast<int8_t>(dut1.coeff3Field), 32);
}

// Tests writing using direct access
TEST(ConfigFieldTest, DirectWrite)
{
	{
		ConfigType storage;

		// Reset memory, default values will be used
		ConfigMemory::erase();
		storage.load();

		DUT<ConfigType, AlignedConfig> dut{"DUT", kDeviceVersion, storage};

		// Write values to cache
		dut.coeff0Field = 8192.0;
		dut.coeff2Field = 4096.0;
		dut.coeff3Field = 128;

		// Direct access does not lead to changes in memory
		dut.coeff1Field = 262144;
	}

	{
		ConfigType storage;
		bool result;

		// Memory is still empty, default values will be used
		result = storage.load();
		ASSERT_FALSE(result);

		DUT<ConfigType, AlignedConfig> dut{"DUT", kDeviceVersion, storage};

		// Check default values
		ASSERT_EQ(static_cast<double>(dut.coeff0Field), 1024.0);
		ASSERT_EQ(static_cast<uint32_t>(dut.coeff1Field), 131072);
		ASSERT_EQ(static_cast<int32_t>(dut.coeff2Field), 1024);
		ASSERT_EQ(static_cast<int64_t>(dut.coeff3Field), 64);
	}
}

// Tests disabled write-through mode
TEST(ConfigFieldTest, WriteToCache)
{
	{
		ConfigType storage;

		// Reset memory, default values will be used
		ConfigMemory::erase();
		storage.load();

		DUT<ConfigType, AlignedConfig> dut{"DUT", kDeviceVersion, storage};

		const double output = 8192.0;
		double input;
		Device::Result result;

		result = dut.coeff0Field.write(&output);
		dut.coeff0Field.read(&input);
		ASSERT_EQ(result, Device::Result::SUCCESS);
		ASSERT_EQ(input, output);

		// coeff1Field was not written, no changes in memory have been made
	}

	{
		ConfigType storage;
		bool result;

		// Memory is still empty, default values will be used
		result = storage.load();
		ASSERT_FALSE(result);

		DUT<ConfigType, AlignedConfig> dut{"DUT", kDeviceVersion, storage};

		// Check default value
		ASSERT_EQ(static_cast<double>(dut.coeff0Field), 1024.0);
	}
}

// Tests enabled write-through mode
TEST(ConfigFieldTest, AlignedMemoryRewrite)
{
	{
		ConfigType storage;

		// Reset memory, default values will be used
		ConfigMemory::erase();
		storage.load();

		DUT<ConfigType, AlignedConfig> dut{"DUT", kDeviceVersion, storage};

		const double ceoff0Output = 8192.0;
		const uint32_t coeff1Output = 262144;
		double coeff0Input;
		uint32_t coeff1Input;
		Device::Result result;

		result = dut.coeff0Field.write(&ceoff0Output);
		dut.coeff0Field.read(&coeff0Input);
		ASSERT_EQ(result, Device::Result::SUCCESS);
		ASSERT_EQ(coeff0Input, ceoff0Output);

		// Write coeff1Field to commit changes
		result = dut.coeff1Field.write(&coeff1Output);
		dut.coeff1Field.read(&coeff1Input);
		ASSERT_EQ(result, Device::Result::SUCCESS);
		ASSERT_EQ(coeff1Input, coeff1Output);
	}

	{
		ConfigType storage;
		bool result;

		// Memory was changed on the previous step
		result = storage.load();
		ASSERT_TRUE(result);

		DUT<ConfigType, AlignedConfig> dut{"DUT", kDeviceVersion, storage};

		// Check new values
		ASSERT_EQ(static_cast<double>(dut.coeff0Field), 8192.0);
		ASSERT_EQ(static_cast<uint32_t>(dut.coeff1Field), 262144);
	}

	{
		ConfigType storage;
		bool result;

		// Reset memory and check that default values will be used
		ConfigMemory::erase();
		result = storage.load();
		ASSERT_FALSE(result);

		DUT<ConfigType, AlignedConfig> dut{"DUT", kDeviceVersion, storage};

		// Check default values
		ASSERT_EQ(static_cast<double>(dut.coeff0Field), 1024.0);
		ASSERT_EQ(static_cast<uint32_t>(dut.coeff1Field), 131072);
	}
}

// Tests writing of unaligned chunk to memory
TEST(ConfigFieldTest, UnalignedMemoryRewrite)
{
	{
		ConfigType storage;

		// Reset memory, default values will be used
		ConfigMemory::erase();
		storage.load();

		DUT<ConfigType, UnalignedConfig> dut{"DUT", kDeviceVersion, storage};

		const float ceoff0Output = 8192.0;
		const int32_t coeff1Output = 262144;
		float coeff0Input;
		int32_t coeff1Input;
		Device::Result result;

		result = dut.coeff0Field.write(&ceoff0Output);
		dut.coeff0Field.read(&coeff0Input);
		ASSERT_EQ(result, Device::Result::SUCCESS);
		ASSERT_EQ(coeff0Input, ceoff0Output);

		// Write coeff1Field to commit changes
		result = dut.coeff1Field.write(&coeff1Output);
		dut.coeff1Field.read(&coeff1Input);
		ASSERT_EQ(result, Device::Result::SUCCESS);
		ASSERT_EQ(coeff1Input, coeff1Output);
	}

	{
		ConfigType storage;
		bool result;

		// Memory was changed on the previous step
		result = storage.load();
		ASSERT_TRUE(result);

		DUT<ConfigType, UnalignedConfig> dut{"DUT", kDeviceVersion, storage};

		// Check new values
		ASSERT_EQ(static_cast<float>(dut.coeff0Field), 8192.0);
		ASSERT_EQ(static_cast<int32_t>(dut.coeff1Field), 262144);
	}
}

// Tests single-bit memory errors in second config memory
TEST(ConfigFieldTest, MemoryErrors)
{
	// Swap bit in data region of second config, memory structure is:
	//   24 bytes for mock config
	//    4 bytes for CRC-32
	//   32 bytes for aligned config
	//    4 bytes for CRC-32
	//   21 bytes for unaligned config
	//    3 bytes padding
	//    4 bytes for CRC-32
	static_assert(sizeof(MockConfig) == 24, "Incorrect config");
	static_assert(sizeof(AlignedConfig) == 32, "Incorrect config");
	static_assert(sizeof(UnalignedConfig) == 21, "Incorrect config");

	for (uint32_t i = 0; i < (sizeof(AlignedConfig) + sizeof(uint32_t)) * 8; ++i) {
		{
			ConfigType storage;

			// Reset memory, default values will be used
			ConfigMemory::erase();
			storage.load();

			DUT<ConfigType, MockConfig> dut0{"DUT", kDeviceVersion, storage};
			DUT<ConfigType, AlignedConfig> dut1{"DUT", kDeviceVersion, storage};
			DUT<ConfigType, UnalignedConfig> dut2{"DUT", kDeviceVersion, storage};

			const uint32_t output = i;
			uint32_t input;
			Device::Result result;

			// Write coeff1Field and commit changes to aligned config
			result = dut0.coeff1Field.write(&output);
			dut0.coeff1Field.read(&input);
			ASSERT_EQ(result, Device::Result::SUCCESS);
			ASSERT_EQ(input, output);

			// Also update unaligned config and last config
			result = dut1.coeff1Field.write(&output);
			dut1.coeff1Field.read(&input);
			ASSERT_EQ(result, Device::Result::SUCCESS);
			ASSERT_EQ(input, output);

			result = dut2.coeff1Field.write(&output);
			dut2.coeff1Field.read(&input);
			ASSERT_EQ(result, Device::Result::SUCCESS);
			ASSERT_EQ(input, output);
		}

		const size_t offset = sizeof(MockConfig) + sizeof(uint32_t) + (i >> 3);
		ConfigMemory::arena()[offset] = static_cast<uint8_t>(ConfigMemory::arena()[offset] ^ (1 << (i & 7)));

		{
			ConfigType storage;
			bool result;

			// Data memory of the second config is incorrect, so default values will be loaded.
			// The first config is placed before the second and it will be loaded correctly.
			// Load sequence stops after the second config, so the third config contains default values.
			result = storage.load();
			ASSERT_FALSE(result);

			DUT<ConfigType, MockConfig> dut0{"DUT", kDeviceVersion, storage};
			DUT<ConfigType, AlignedConfig> dut1{"DUT", kDeviceVersion, storage};
			DUT<ConfigType, UnalignedConfig> dut2{"DUT", kDeviceVersion, storage};

			// Check new value
			ASSERT_EQ(static_cast<uint32_t>(dut0.coeff1Field), i);

			// Check default value
			ASSERT_EQ(static_cast<uint32_t>(dut1.coeff1Field), 131072);

			// Check default value
			ASSERT_EQ(static_cast<uint32_t>(dut2.coeff1Field), 131072);
		}
	}
}
