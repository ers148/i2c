//
// MemoryRegion.hpp
//
//  Created on: Mar 5, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_MEMORYREGION_HPP_
#define DRONEDEVICE_MEMORYREGION_HPP_

#include <cstddef>
#include <cstdint>

namespace Device {

template<typename MemoryInterface, uintptr_t memoryAddress, size_t memorySize, bool readable, bool writable>
class MemoryRegion {
public:
	using Interface = MemoryInterface;

	class Unlocker {
		using Parent = MemoryRegion<Interface, memoryAddress, memorySize, readable, writable>;

	public:
		Unlocker()
		{
			Parent::unlock();
		}

		~Unlocker()
		{
			Parent::lock();
		}
	};

	MemoryRegion() = delete;
	MemoryRegion(const MemoryRegion &) = delete;
	MemoryRegion &operator=(const MemoryRegion &) = delete;

	static constexpr uintptr_t address()
	{
		return memoryAddress;
	}

	static constexpr size_t capacity()
	{
		return memorySize;
	}

	static constexpr size_t size()
	{
		return capacity();
	}

	static bool isReadable()
	{
		return size() > 0 && (!Interface::isProtected() || readable);
	}

	static bool isWritable()
	{
		return size() > 0 && (!Interface::isProtected() || writable);
	}

	static void lock()
	{
		Interface::lock();
	}

	static void unlock()
	{
		Interface::unlock();
	}

	static bool erase()
	{
		return isWritable() ? Interface::erase(address(), size()) : false;
	}

	static bool read(uintptr_t position, void *buffer, size_t length)
	{
		return isReadable() ? Interface::read(address() + position, buffer, length) : false;
	}

	static bool write(uintptr_t position, const void *buffer, size_t length)
	{
		return isWritable() ? Interface::write(address() + position, buffer, length) : false;
	}
};

} // namespace Device

#endif // DRONEDEVICE_MEMORYREGION_HPP_
