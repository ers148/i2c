//
// MockMemoryInterface.hpp
//
//  Created on: Sep 28, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKMEMORYINTERFACE_HPP_
#define DRONEDEVICE_STUBS_MOCKMEMORYINTERFACE_HPP_

#include <cstdint>

namespace Device {

class MockMemoryInterface {
public:
	MockMemoryInterface() = delete;
	MockMemoryInterface(const MockMemoryInterface &) = delete;
	MockMemoryInterface &operator=(const MockMemoryInterface &) = delete;

	static bool protect()
	{
		return false;
	}

	static bool isProtected()
	{
		return true;
	}

	static void lock()
	{
	}

	static void unlock()
	{
	}

	static bool erase(uintptr_t, size_t)
	{
		return false;
	}

	static bool read(uintptr_t, void *, size_t)
	{
		return false;
	}

	static bool write(uintptr_t, const void *, size_t)
	{
		return false;
	}
};

} // namespace Device

#endif // DRONEDEVICE_STUBS_MOCKMEMORYINTERFACE_HPP_
