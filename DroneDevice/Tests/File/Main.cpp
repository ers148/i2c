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

static constexpr Device::Version kDeviceVersion{{1, 2}, {3, 4, 0xCAFEFEED, 12345}};

// Tests reading and writing file
TEST(File, ReadWriteChar)
{
	DUT dut{"DUT", kDeviceVersion};

	struct FileObserver : Device::DeviceObserver {
		void onFileInfoReceived(const Device::FileInfo &aInfo, Device::RefCounter * /*aToken*/) override
		{
			ASSERT_EQ(aInfo.index, 0);
			ASSERT_EQ(aInfo.flags, Device::kFileReadable | Device::kFileWritable);
			ASSERT_EQ(aInfo.size, 0);
			ASSERT_EQ(aInfo.checksum, 0);
			infoRecived = true;
		}

		void onFileReadEnd(Device::FileId /*aFile*/, uint32_t /*aOffset*/, const void * /*aData*/, size_t /*aSize*/,
			Device::Result /*aResult*/, Device::RefCounter * /*aToken*/) override
		{
			fileReaded = true;
		}

		void onFileWriteEnd(Device::FileId /*aFile*/, uint32_t /*aOffset*/,
			Device::Result /*aResult*/, Device::RefCounter * /*aToken*/) override
		{
			fileWrited = true;
		}

		bool infoRecived{false};
		bool fileReaded{false};
		bool fileWrited{false};
	};

	FileObserver observer;
	std::array<uint8_t, 32> data;
	data.fill(0xaa);

	dut.fileRequestInfo(0, Device::kFileReqFlagsSize, &observer, nullptr);

	dut.fileWrite(0, 0, data.data(), data.size(), &observer, nullptr);

	dut.fileRead(0, 0, 32, &observer, nullptr);

	ASSERT_EQ(observer.infoRecived, true);
	ASSERT_EQ(observer.fileReaded, true);
	ASSERT_EQ(observer.fileWrited, true);
}

// Tests reading and writing file
TEST(File, DefaultMethods)
{
	DUT dut{"DUT", kDeviceVersion};

	ASSERT_EQ("DUT", dut.deviceName());
	ASSERT_EQ(Device::DeviceHash{}, dut.deviceHash());

	auto version = dut.deviceVersion();

	ASSERT_EQ(0, memcmp(&version, &kDeviceVersion, sizeof(kDeviceVersion)));

	ASSERT_EQ(0, dut.getFieldCount());

	ASSERT_EQ(nullptr, dut.getField(0));

	ASSERT_EQ(Device::kFieldReservedId, dut.getFieldIndex("test"));

	struct FieldObserver : Device::DeviceObserver {
		void onFieldInfoRequestError(Device::FieldId /*aField*/,
			Device::Result /*aError*/, Device::RefCounter * /*aToken*/) override
		{
			infoPassed = true;
		}

		bool infoPassed{false};

		void onFieldIndexRequestError(Device::Result /*aError*/, Device::RefCounter * /*aToken*/) override
		{
			indexPassed = true;
		}

		bool indexPassed{false};

		void onFieldRequestError(Device::FieldId /*aField*/,
			Device::Result /*aError*/, Device::RefCounter * /*aToken*/) override
		{
			passed = true;
		}

		bool passed{false};
	};

	{
		// RequestInfo not exist Field
		FieldObserver o1;
		dut.fieldRequestInfo(10, &o1, nullptr);
		ASSERT_EQ(true, o1.infoPassed);
	}

	{
		// RequestIndex not exist Field
		FieldObserver o1;
		dut.fieldRequestIndex("test", &o1, nullptr);
		ASSERT_EQ(true, o1.indexPassed);
	}

	{
		// Read not exist Field
		FieldObserver o1;
		dut.fieldRead(10, &o1, nullptr);
		ASSERT_EQ(true, o1.passed);
	}

	{
		// Write not exist Field
		FieldObserver o1;
		dut.fieldWrite(10, nullptr, &o1, nullptr);
		ASSERT_EQ(true, o1.passed);
	}
}
