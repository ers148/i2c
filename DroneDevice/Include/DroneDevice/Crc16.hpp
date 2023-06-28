//
// Crc16.hpp
//
//  Created on: Feb 13, 2015
//      Author: Alexander
//

#ifndef DRONEDEVICE_CRC16_HPP_
#define DRONEDEVICE_CRC16_HPP_

#include <cstddef>
#include <cstdint>

class Crc16 {
public:
	using Type = uint16_t;
	static constexpr size_t size{sizeof(uint16_t)};

	Crc16() = delete;
	Crc16(const Crc16 &) = delete;
	Crc16 &operator=(const Crc16 &) = delete;

	//!
	//! Update checksum with buffer data.
	//! \param checksum Previous checksum value.
	//! \param buffer Pointer to buffer with at least @b length characters of data.
	//! \param length Buffer length.
	//! \return Updated checksum value.
	//!
	static uint16_t update(uint16_t aChecksum, const void *data, size_t aLength)
	{
		const uint8_t *buffer = static_cast<const uint8_t *>(data);

		while (aLength--) {
			aChecksum = static_cast<uint16_t>(aChecksum ^ *buffer++ << 8);

			for (uint8_t bit = 0; bit < 8; bit++)
				aChecksum = static_cast<uint16_t>(aChecksum & 0x8000 ? (aChecksum << 1) ^ 0x1021 : aChecksum << 1);
		}
		return aChecksum;
	}
};

#endif // CORE_CRC16_HPP_
