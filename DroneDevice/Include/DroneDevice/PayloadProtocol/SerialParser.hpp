//
// SerialParser.hpp
//
//  Created on: Oct 23, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_PAYLOADPROTOCOL_SERIALPARSER_HPP_
#define DRONEDEVICE_PAYLOADPROTOCOL_SERIALPARSER_HPP_

#include <cstring>

namespace PayloadProtocol {

template<size_t limit, typename Crc>
class SerialParser {
private:
	static constexpr uint8_t kInitialChecksum = 0x00;
	static constexpr uint8_t kStartCharacter = 0x7E;

	static constexpr size_t kPrefixLength = 2; // Start symbol (0x7E) and message length
	static constexpr size_t kSuffixLength = 1; // Checksum (CRC-8)

public:
	static constexpr size_t kBufferLength = limit + kPrefixLength + kSuffixLength;

	enum class State {
		IDLE,
		LENGTH,
		PAYLOAD,
		CHECKSUM,
		DONE,
		ERROR_CHECKSUM,
		ERROR_LENGTH
	};

	static size_t create(void *aBuffer, size_t aBufferLength, const void *aPayload, size_t aPayloadLength)
	{
		if (!aBufferLength || (aPayload != nullptr && (!aPayloadLength || aPayloadLength > aBufferLength))) {
			return 0;
		}

		uint8_t * const position = static_cast<uint8_t *>(aBuffer);

		position[0] = kStartCharacter;
		position[1] = static_cast<uint8_t>(aPayloadLength - 1);
		position[kPrefixLength + aPayloadLength] = Crc::update(kInitialChecksum, aPayload, aPayloadLength);

		if (aPayloadLength) {
			memcpy(position + kPrefixLength, aPayload, aPayloadLength);
		}

		return kPrefixLength + kSuffixLength + aPayloadLength;
	}

	SerialParser() :
		messageLength{0},
		messagePosition{0},
		parserState{State::IDLE}
	{
	}

	const void *data() const
	{
		return buffer;
	}

	size_t length() const
	{
		return messageLength;
	}

	State state() const
	{
		return parserState;
	}

	size_t update(const void *aInputData, size_t aInputLength)
	{
		if (finished()) {
			reset();
		}

		for (size_t i = 0; i < aInputLength; ++i) {
			if (finished()) {
				return i;
			}

			const uint8_t value = *(static_cast<const uint8_t *>(aInputData) + i);

			switch (parserState) {
				case State::IDLE:
					if (value == kStartCharacter) {
						parserState = State::LENGTH;
					}
					break;

				case State::LENGTH:
					messageLength = value + 1;

					if (messageLength && messageLength <= limit) {
						messagePosition = 0;
						parserState = State::PAYLOAD;
					} else {
						parserState = State::ERROR_LENGTH;
					}
					break;

				case State::PAYLOAD:
					buffer[messagePosition++] = value;

					if (messagePosition >= messageLength) {
						parserState = State::CHECKSUM;
					}
					break;

				case State::CHECKSUM:
					if (Crc::update(kInitialChecksum, buffer, messageLength) != value) {
						parserState = State::ERROR_CHECKSUM;
					} else {
						parserState = State::DONE;
					}
					break;

				case State::DONE:
				case State::ERROR_CHECKSUM:
				case State::ERROR_LENGTH:
					break;
			}
		}

		return aInputLength;
	}

private:
	uint8_t buffer[kBufferLength];
	size_t messageLength;
	size_t messagePosition;
	State parserState;

	bool finished() const
	{
		return parserState == State::DONE || parserState == State::ERROR_CHECKSUM || parserState == State::ERROR_LENGTH;
	}

	void reset()
	{
		parserState = State::IDLE;
	}
};

}

#endif // DRONEDEVICE_PAYLOADPROTOCOL_SERIALPARSER_HPP_
