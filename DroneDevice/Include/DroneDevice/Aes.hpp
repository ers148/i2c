//
// Aes.hpp
//
//  Created on: Aug 1, 2016
//      Author: Renat
//

#ifndef DRONEDEVICE_AES_HPP_
#define DRONEDEVICE_AES_HPP_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

class Aes {
public:
	// AES-128 CBC
	static constexpr size_t kBlockSize{16};

	Aes(const char *aKey) :
		// Suppress warnings, pointers will be rewritten during reset
		prevBlockPtr{nullptr},
		tempBlockPtr{nullptr}
	{
		initSBox(sbox);
		initInvSBox(sboxInv, sbox);
		loadKey(aKey);
		reset();
	}

	void reset()
	{
		memcpy(currentKey, cachedKey, kBlockSize);
		memset(prevBlock, 0, kBlockSize);
		memset(tempBlock, 0, kBlockSize);
		prevBlockPtr = prevBlock;
		tempBlockPtr = tempBlock;
	}

	void decrypt(void *aBlock)
	{
		assert(aBlock != nullptr);

		uint8_t * const block = static_cast<uint8_t *>(aBlock);

		memcpy(tempBlockPtr, block, kBlockSize);
		decryptBlock(block, currentKey);

		for (size_t i = 0; i < kBlockSize; ++i) {
			block[i] ^= prevBlockPtr[i];
		}

		std::swap(tempBlockPtr, prevBlockPtr);
	}

private:
	static constexpr size_t kBoxSize{256};
	static constexpr size_t kRounds{10};

	uint8_t cachedKey[kBlockSize];
	uint8_t currentKey[kBlockSize];
	uint8_t sbox[kBoxSize];
	uint8_t sboxInv[kBoxSize];

	uint8_t prevBlock[kBlockSize];
	uint8_t tempBlock[kBlockSize];
	uint8_t *prevBlockPtr;
	uint8_t *tempBlockPtr;

	void decryptBlock(uint8_t *aBlock, uint8_t *aKey)
	{
		static const uint8_t roundConstants[] = {
				0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
		};

		uint8_t buf1, buf2, buf3, buf4;
		size_t round, i;

		// Compute the last key of encryption before starting the decryption
		for (round = 0; round < kRounds; ++round) {
			// Key schedule
			aKey[0] = sbox[aKey[13]] ^ aKey[0] ^ roundConstants[round];
			aKey[1] = sbox[aKey[14]] ^ aKey[1];
			aKey[2] = sbox[aKey[15]] ^ aKey[2];
			aKey[3] = sbox[aKey[12]] ^ aKey[3];

			for (i = 4; i < kBlockSize; ++i) {
				aKey[i] = aKey[i] ^ aKey[i - 4];
			}
		}

		// First add round key
		for (i = 0; i < kBlockSize; ++i){
			aBlock[i] = aBlock[i] ^ aKey[i];
		}

		// Main loop
		for (round = 0; round < kRounds; ++round){
			// Inverse key schedule
			for (i = 15; i > 3; --i) {
				aKey[i] = aKey[i] ^ aKey[i - 4];
			}

			aKey[0] = sbox[aKey[13]] ^ aKey[0] ^ roundConstants[9 - round];
			aKey[1] = sbox[aKey[14]] ^ aKey[1];
			aKey[2] = sbox[aKey[15]] ^ aKey[2];
			aKey[3] = sbox[aKey[12]] ^ aKey[3];

			// Mixcol - inv mix
			if (round > 0) {
				for (i = 0; i < 4; ++i) {
					buf4 = static_cast<uint8_t>(i << 2);
					// Precompute for decryption
					buf1 = galoisMul2(galoisMul2(aBlock[buf4] ^ aBlock[buf4 + 2]));
					buf2 = galoisMul2(galoisMul2(aBlock[buf4 + 1] ^ aBlock[buf4 + 3]));
					aBlock[buf4] ^= buf1;
					aBlock[buf4 + 1] ^= buf2;
					aBlock[buf4 + 2] ^= buf1;
					aBlock[buf4 + 3] ^= buf2;

					buf1 = aBlock[buf4] ^ aBlock[buf4 + 1] ^ aBlock[buf4 + 2] ^ aBlock[buf4 + 3];
					buf2 = aBlock[buf4];
					buf3 = aBlock[buf4] ^ aBlock[buf4 + 1];
					buf3 = galoisMul2(buf3);
					aBlock[buf4] = aBlock[buf4] ^ buf3 ^ buf1;
					buf3 = aBlock[buf4 + 1] ^ aBlock[buf4 + 2];
					buf3 = galoisMul2(buf3);
					aBlock[buf4 + 1] = aBlock[buf4 + 1] ^ buf3 ^ buf1;
					buf3 = aBlock[buf4 + 2] ^ aBlock[buf4 + 3];
					buf3 = galoisMul2(buf3);
					aBlock[buf4 + 2] = aBlock[buf4 + 2] ^ buf3 ^ buf1;
					buf3 = aBlock[buf4 + 3] ^ buf2;
					buf3 = galoisMul2(buf3);
					aBlock[buf4 + 3] = aBlock[buf4 + 3] ^ buf3 ^ buf1;
				}
			}

			// Inv shift rows

			// Row 1
			buf1   = aBlock[13];
			aBlock[13] = aBlock[9];
			aBlock[9]  = aBlock[5];
			aBlock[5]  = aBlock[1];
			aBlock[1]  = buf1;
			// Row 2
			buf1   = aBlock[10];
			buf2   = aBlock[14];
			aBlock[10] = aBlock[2];
			aBlock[14] = aBlock[6];
			aBlock[2]  = buf1;
			aBlock[6]  = buf2;
			// Row 3
			buf1   = aBlock[3];
			aBlock[3]  = aBlock[7];
			aBlock[7]  = aBlock[11];
			aBlock[11] = aBlock[15];
			aBlock[15] = buf1;

			for (i = 0; i < kBlockSize; ++i) {
				// With shift row i + 5 mod 16
				aBlock[i] = sboxInv[aBlock[i]] ^ aKey[i];
			}
		}
	}

	void loadKey(const char *aKey)
	{
		assert(strlen(aKey) == kBlockSize * 2);

		for (size_t i = 0; i < kBlockSize; ++i) {
			cachedKey[i] = static_cast<uint8_t>((hexToBin(aKey[i * 2]) << 4) | hexToBin(aKey[i * 2 + 1]));
		}
	}

	static uint8_t galoisMul2(uint8_t value)
	{
		const int temp = static_cast<int>(static_cast<int8_t>(value));
		return static_cast<uint8_t>((value << 1) ^ ((temp >> 7) & 0x1B));
	}

	static uint8_t hexToBin(char c)
	{
		if (c >= 'A' && c <= 'F') {
			return static_cast<uint8_t>(c - 'A' + 10);
		} else {
			return static_cast<uint8_t>(c - '0');
		}
	}

	static void initSBox(uint8_t *sbox)
	{
		// Loop invariant: p * q == 1 in the Galois field
		uint8_t p = 1, q = 1;

		do {
			// Multiply p by x + 1
			p = static_cast<uint8_t>(p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0));
			// Divide q by x + 1
			q = static_cast<uint8_t>(q ^ (q << 1));
			q = static_cast<uint8_t>(q ^ (q << 2));
			q = static_cast<uint8_t>(q ^ (q << 4));
			q = static_cast<uint8_t>(q ^ (q & 0x80 ? 0x09 : 0));
			// Compute the affine transformation
			sbox[p] = 0x63 ^ q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);
		} while (p != 1);

		// 0 is a special case since it has no inverse
		sbox[0] = 0x63;
	}

	static void initInvSBox(uint8_t *sboxInv, const uint8_t *sbox)
	{
		for (size_t i = 0; i < kBoxSize; ++i)
			sboxInv[sbox[i]] = static_cast<uint8_t>(i);
	}

	static inline uint8_t ROTL8(uint8_t x, unsigned int shift)
	{
		return static_cast<uint8_t>((x << shift) | (x >> (8 - shift)));
	}
};

#endif // DRONEDEVICE_AES_HPP_
