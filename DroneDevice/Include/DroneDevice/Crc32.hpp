//
// Crc32.hpp
//
//  Created on: Apr 4, 2013
//      Author: Alexander
//

#ifndef DRONEDEVICE_CRC32_HPP_
#define DRONEDEVICE_CRC32_HPP_

#include <cstddef>
#include <cstdint>

class Crc32 {
public:
	using Type = uint32_t;
	static constexpr size_t size{sizeof(uint32_t)};

	Crc32() = delete;
	Crc32(const Crc32 &) = delete;
	Crc32 &operator=(const Crc32 &) = delete;

	//!
	//! Update checksum with buffer data.
	//! \param checksum Previous checksum value.
	//! \param buffer Pointer to buffer with at least @b length characters of data.
	//! \param length Buffer length.
	//! \return Updated checksum value.
	//!
	static uint32_t update(uint32_t aChecksum, const void *aBuffer, size_t aLength)
	{
		// x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + 1
		const uint8_t *buffer = static_cast<const uint8_t *>(aBuffer);

		aChecksum = ~aChecksum;

		while (aLength--) {
			uint8_t value = *buffer++;

			for (uint8_t bit = 0; bit < 8; ++bit) {
				if ((aChecksum ^ value) & 0x01) {
					aChecksum = static_cast<uint32_t>((aChecksum >> 1) ^ 0xEDB88320UL);
				} else {
					aChecksum = static_cast<uint32_t>(aChecksum >> 1);
				}

				value = static_cast<uint8_t>(value >> 1);
			}
		}

		return ~aChecksum;
	}
};

#endif // DRONEDEVICE_CRC32_HPP_
