//
// MemoryFile.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_MEMORYFILE_HPP_
#define DRONEDEVICE_INTERNALDEVICE_MEMORYFILE_HPP_

#include <algorithm>
#include <DroneDevice/AbstractDevice.hpp>
#include <DroneDevice/FastCrc32.hpp>
#include <DroneDevice/InternalDevice/AbstractFile.hpp>

namespace Device {

template<size_t capacity, bool readonly = false, class Checksum = FastCrc32>
class MemoryFile : public AbstractFile {
public:
	MemoryFile() :
		size{0},
		finalized{true}
	{
	}

	uint32_t getChecksum() const override
	{
		return Checksum::update(kFileInitialChecksum, data, size);
	}

	FileFlags getFlags() const override
	{
		return readonly ? kFileReadable : (kFileReadable | kFileWritable);
	}

	uint32_t getSize() const override
	{
		return static_cast<uint32_t>(size);
	}

	bool readChunk(uint32_t aOffset, void *aBuffer, size_t aLength) const override
	{
		if (aOffset >= size) {
			return false;
		}

		const size_t chunkLength = std::min(size - static_cast<size_t>(aOffset), aLength);

		if (chunkLength) {
			memcpy(aBuffer, data + static_cast<size_t>(aOffset), chunkLength);
			return true;
		} else {
			return false;
		}
	}

	bool writeChunk(uint32_t aOffset, const void *aBuffer, size_t aLength) override
	{
		return writeChunkImpl<readonly>(static_cast<size_t>(aOffset), static_cast<const uint8_t *>(aBuffer), aLength);
	}

	bool restartWrite() override
	{
		return restartWriteImpl<readonly>();
	}

	bool finalizeWrite(uint32_t aTotal) override
	{
		return finalizeWriteImpl<readonly>(static_cast<size_t>(aTotal));
	}

	bool isFinalized() const override
	{
		return finalized;
	}

protected:
	size_t size;
	uint8_t data[capacity];
	bool finalized;

private:
	template<bool selector>
	typename std::enable_if_t<!selector, bool> writeChunkImpl(size_t aOffset, const uint8_t *aBuffer, size_t aLength)
	{
		if (aLength && aOffset + aLength <= capacity) {
			const size_t skipped = size - aOffset;

			if (skipped < aLength) {
				const size_t pending = aLength - skipped;

				memcpy(data + size, aBuffer + skipped, pending);
				size += pending;
			}

			return true;
		} else {
			return false;
		}
	}

	template<bool selector>
	typename std::enable_if_t<selector, bool> writeChunkImpl(size_t, const uint8_t *, size_t)
	{
		return false;
	}

	template<bool selector>
	typename std::enable_if_t<!selector, bool> restartWriteImpl()
	{
		size = 0;
		finalized = false;
		return true;
	}

	template<bool selector>
	typename std::enable_if_t<selector, bool> restartWriteImpl()
	{
		return false;
	}

	template<bool selector>
	typename std::enable_if_t<!selector, bool> finalizeWriteImpl(size_t aTotal)
	{
		if (aTotal == kFileReservedPosition) {
			aTotal = 0;
		}

		if (aTotal == size) {
			finalized = true;
			return true;
		} else {
			return false;
		}
	}

	template<bool selector>
	typename std::enable_if_t<selector, bool> finalizeWriteImpl(size_t)
	{
		return false;
	}
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_MEMORYFILE_HPP_
