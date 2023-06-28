//
// MockMemoryRegion.hpp
//
//  Created on: Mar 5, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKMEMORYREGION_HPP_
#define DRONEDEVICE_STUBS_MOCKMEMORYREGION_HPP_

#include <cstddef>

namespace Device {

class MockMemoryRegion {
public:
	class Unlocker {
	public:
		// Constructor and destructor are defined to suppress warnings
		Unlocker()
		{
		}

		~Unlocker()
		{
		}
	};

	MockMemoryRegion() = delete;
	MockMemoryRegion(const MockMemoryRegion &) = delete;
	MockMemoryRegion &operator=(const MockMemoryRegion &) = delete;

	static constexpr uintptr_t address()
	{
		return 0;
	}

	static constexpr size_t capacity()
	{
		return 0;
	}

	static constexpr size_t size()
	{
		return capacity();
	}

	static void lock()
	{
	}

	static void unlock()
	{
	}

	static bool erase()
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

#endif // DRONEDEVICE_STUBS_MOCKMEMORYREGION_HPP_
