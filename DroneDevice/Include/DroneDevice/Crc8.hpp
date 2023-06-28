//
// Crc8.hpp
//
//  Created on: Feb 13, 2015
//      Author: Alexander
//

#ifndef DRONEDEVICE_CRC8_HPP_
#define DRONEDEVICE_CRC8_HPP_

#include <cstddef>
#include <cstdint>

class Crc8 {
public:
	using Type = uint8_t;
	static constexpr size_t size{sizeof(uint8_t)};

	Crc8() = delete;
	Crc8(const Crc8 &) = delete;
	Crc8 &operator=(const Crc8 &) = delete;

	//!
	//! Update checksum with buffer data.
	//! \param checksum Previous checksum value.
	//! \param buffer Pointer to buffer with at least @b length characters of data.
	//! \param length Buffer length.
	//! \return Updated checksum value.
	//!
	static uint8_t update(uint8_t aChecksum, const void *aBuffer, size_t aLength)
	{
		const uint8_t *buffer = static_cast<const uint8_t *>(aBuffer);

		while (aLength--) {
			uint8_t value = *buffer++;

			for (uint8_t bit = 0; bit < 8; ++bit) {
				if ((aChecksum ^ value) & 0x01) {
					aChecksum = static_cast<uint8_t>(((aChecksum ^ 0x18) >> 1) | 0x80);
				} else {
					aChecksum = static_cast<uint8_t>(aChecksum >> 1);
				}

				value = static_cast<uint8_t>(value >> 1);
			}
		}

		return aChecksum;
	}
};

#endif // DRONEDEVICE_CRC8_HPP_
