//
// Sha256.hpp
//
//  Created on: Aug 5, 2021
//      Author: Alexander
//

#ifndef DRONEDEVICE_SHA256_HPP_
#define DRONEDEVICE_SHA256_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

class Sha256 {
public:
	Sha256()
	{
		reset();
	}

	void reset()
	{
		datalen = 0;
		bitlen = 0;
		state[0] = 0x6A09E667UL;
		state[1] = 0xBB67AE85UL;
		state[2] = 0x3C6EF372UL;
		state[3] = 0xA54FF53AUL;
		state[4] = 0x510E527FUL;
		state[5] = 0x9B05688CUL;
		state[6] = 0x1F83D9ABUL;
		state[7] = 0x5BE0CD19UL;
	}

	void update(const uint8_t *aData, size_t aLength)
	{
		uint32_t i;

		for (i = 0; i < aLength; ++i) {
			data[datalen] = aData[i];
			datalen++;

			if (datalen == 64) {
				transform(data);
				bitlen += 512;
				datalen = 0;
			}
		}
	}

	std::array<uint8_t, 32> finalize()
	{
		std::array<uint8_t, 32> hash;
		uint32_t i = datalen;

		if (datalen < 56) {
			data[i++] = 0x80;

			while (i < 56) {
				data[i++] = 0x00;
			}
		} else {
			data[i++] = 0x80;

			while (i < 64) {
				data[i++] = 0x00;
			}

			transform(data);
			memset(data, 0, 56);
		}

		bitlen += datalen * 8;
		data[63] = static_cast<uint8_t>(bitlen);
		data[62] = static_cast<uint8_t>(bitlen >> 8);
		data[61] = static_cast<uint8_t>(bitlen >> 16);
		data[60] = static_cast<uint8_t>(bitlen >> 24);
		data[59] = static_cast<uint8_t>(bitlen >> 32);
		data[58] = static_cast<uint8_t>(bitlen >> 40);
		data[57] = static_cast<uint8_t>(bitlen >> 48);
		data[56] = static_cast<uint8_t>(bitlen >> 56);
		transform(data);

		// Implementation requires little endian byte ordering
		for (i = 0; i < 4; ++i) {
			hash[i]      = (state[0] >> (24 - i * 8)) & 0xFF;
			hash[i + 4]  = (state[1] >> (24 - i * 8)) & 0xFF;
			hash[i + 8]  = (state[2] >> (24 - i * 8)) & 0xFF;
			hash[i + 12] = (state[3] >> (24 - i * 8)) & 0xFF;
			hash[i + 16] = (state[4] >> (24 - i * 8)) & 0xFF;
			hash[i + 20] = (state[5] >> (24 - i * 8)) & 0xFF;
			hash[i + 24] = (state[6] >> (24 - i * 8)) & 0xFF;
			hash[i + 28] = (state[7] >> (24 - i * 8)) & 0xFF;
		}

		return hash;
	}

private:
	uint64_t bitlen;
	uint32_t state[8];
	uint16_t datalen;
	uint8_t data[64];

	static uint32_t k(size_t i)
	{
		static const uint32_t table[64] = {
			0x428A2F98UL, 0x71374491UL, 0xB5C0FBCFUL, 0xE9B5DBA5UL, 0x3956C25BUL, 0x59F111F1UL, 0x923F82A4UL, 0xAB1C5ED5UL,
			0xD807AA98UL, 0x12835B01UL, 0x243185BEUL, 0x550C7DC3UL, 0x72BE5D74UL, 0x80DEB1FEUL, 0x9BDC06A7UL, 0xC19BF174UL,
			0xE49B69C1UL, 0xEFBE4786UL, 0x0FC19DC6UL, 0x240CA1CCUL, 0x2DE92C6FUL, 0x4A7484AAUL, 0x5CB0A9DCUL, 0x76F988DAUL,
			0x983E5152UL, 0xA831C66DUL, 0xB00327C8UL, 0xBF597FC7UL, 0xC6E00BF3UL, 0xD5A79147UL, 0x06CA6351UL, 0x14292967UL,
			0x27B70A85UL, 0x2E1B2138UL, 0x4D2C6DFCUL, 0x53380D13UL, 0x650A7354UL, 0x766A0ABBUL, 0x81C2C92EUL, 0x92722C85UL,
			0xA2BFE8A1UL, 0xA81A664BUL, 0xC24B8B70UL, 0xC76C51A3UL, 0xD192E819UL, 0xD6990624UL, 0xF40E3585UL, 0x106AA070UL,
			0x19A4C116UL, 0x1E376C08UL, 0x2748774CUL, 0x34B0BCB5UL, 0x391C0CB3UL, 0x4ED8AA4AUL, 0x5B9CCA4FUL, 0x682E6FF3UL,
			0x748F82EEUL, 0x78A5636FUL, 0x84C87814UL, 0x8CC70208UL, 0x90BEFFFAUL, 0xA4506CEBUL, 0xBEF9A3F7UL, 0xC67178F2UL
		};

		return table[i];
	}

	template<typename T>
	static T ROTLEFT(T a, unsigned int b)
	{
		return (a << b) | (a >> (32 - b));
	}

	template<typename T>
	static T ROTRIGHT(T a, unsigned int b)
	{
		return (a >> b) | (a << (32 - b));
	}

	template<typename T>
	static T CH(T x, T y, T z)
	{
		return (x & y) ^ (~x & z);
	}

	template<typename T>
	static T MAJ(T x, T y, T z)
	{
		return (x & y) ^ (x & z) ^ (y & z);
	}

	template<typename T>
	static T EP0(T x)
	{
		return ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22);
	}

	template<typename T>
	static T EP1(T x)
	{
		return ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25);
	}

	template<typename T>
	static T SIG0(T x)
	{
		return ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ (x >> 3);
	}

	template<typename T>
	static T SIG1(T x)
	{
		return ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ (x >> 10);
	}

	void transform(const uint8_t *aData)
	{
		uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

		for (i = 0, j = 0; i < 16; ++i, j += 4) {
			m[i] = (aData[j] << 24) | (aData[j + 1] << 16) | (aData[j + 2] << 8) | (aData[j + 3]);
		}

		for ( ; i < 64; ++i) {
			m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
		}

		a = state[0];
		b = state[1];
		c = state[2];
		d = state[3];
		e = state[4];
		f = state[5];
		g = state[6];
		h = state[7];

		for (i = 0; i < 64; ++i) {
			t1 = h + EP1(e) + CH(e,f,g) + k(i) + m[i];
			t2 = EP0(a) + MAJ(a,b,c);
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;
		state[4] += e;
		state[5] += f;
		state[6] += g;
		state[7] += h;
	}
};

#endif // DRONEDEVICE_SHA256_HPP_
