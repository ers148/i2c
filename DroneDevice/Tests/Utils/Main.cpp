//
// Main.cpp
//
//  Created on: Sep 05, 2019
//      Author: Ilya
//

#include "gtest/gtest.h"
#include <DroneDevice/Crc16.hpp>
#include <DroneDevice/Crc32.hpp>
#include <DroneDevice/FastCrc16.hpp>
#include <DroneDevice/FastCrc32.hpp>

#include <DroneDevice/RefCounter.hpp>
#include <DroneDevice/RequestPool.hpp>

// Tests reading of default values from generic volatile fields
TEST(UtilsTest, Crc)
{
	// CRC16

	// Test standart polinom and initial
	ASSERT_EQ(Crc16::update(0xFFFF, "123456789", 9), 0x29B1);
	ASSERT_EQ(FastCrc16::update(0xFFFF, "123456789", 9), 0x29B1);

	// Test standart polinom and zero value initial
	ASSERT_EQ(Crc16::update(0, "123456789", 9), 0x31C3);
	ASSERT_EQ(FastCrc16::update(0, "123456789", 9), 0x31C3);

	// CRC32

	// Test standart polinom and initial
	ASSERT_EQ(Crc32::update(0xFFFF, "123456789", 9), 0xA4F0338B);
	ASSERT_EQ(FastCrc32::update(0xFFFF, "123456789", 9), 0xA4F0338B);

	// Test standart polinom and zero value initial
	ASSERT_EQ(Crc32::update(0, "123456789", 9), 0xCBF43926);
	ASSERT_EQ(FastCrc32::update(0, "123456789", 9), 0xCBF43926);
}

// Tests reading of default values from generic volatile fields
TEST(UtilsTest, RequestPool)
{

	struct Req : Device::RefCounter {
		uint32_t val;
		Req(uint32_t aVal = 0):
			Device::RefCounter{},
			val{aVal}
		{}
	};

	Device::RequestPool<Req, 10> pool;
	ASSERT_EQ(pool.count(), 10);

	auto *const r = pool.alloc(uint32_t(1));
	ASSERT_EQ(r->val, 1);
	ASSERT_EQ(pool.count(), 9);

	auto *const r2 = pool.alloc(uint32_t(2));
	ASSERT_EQ(r2->val, 2);
	ASSERT_EQ(pool.count(), 8);

	pool.free(r2);
	ASSERT_EQ(pool.count(), 9);

	ASSERT_NE(nullptr, pool.alloc(uint32_t(2)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(3)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(4)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(5)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(6)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(7)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(8)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(9)));
	ASSERT_NE(nullptr, pool.alloc(uint32_t(10)));

	// we push more then allow
	ASSERT_EQ(nullptr, pool.alloc(uint32_t(11)));
}
