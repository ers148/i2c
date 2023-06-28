//
// MockReloader.hpp
//
//  Created on: Feb 5, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKRELOADER_HPP_
#define DRONEDEVICE_STUBS_MOCKRELOADER_HPP_

#include <cstdint>

namespace Device {

class MockReloader {
public:
	MockReloader() = delete;
	MockReloader(const MockReloader &) = delete;
	MockReloader &operator=(const MockReloader &) = delete;

	static bool isBootRequested()
	{
		return true;
	}

	static bool isFwStartFailed()
	{
		return false;
	}

	static bool isWdtResetOccurred()
	{
		return false;
	}

	static uint32_t getRestartCount()
	{
		return 0;
	}

	static void setRestartCount(uint32_t)
	{
	}

	static void clearBootSignature()
	{
	}

	static void relocateAndStart(uint32_t, uint32_t, uint32_t)
	{
	}

	static void reset()
	{
	}

	static void resetToBootloader()
	{
	}
};

} // namespace Device

#endif // DRONEDEVICE_STUBS_MOCKRELOADER_HPP_
