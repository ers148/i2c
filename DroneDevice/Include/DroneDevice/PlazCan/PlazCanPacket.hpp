/*
 * PlazCanPacket.hpp
 *
 *  Created on: Feb 5, 2018
 *      Author: Alexander
 */

#ifndef DRONEDEVICE_PLAZCAN_PLAZCANPACKET_HPP_
#define DRONEDEVICE_PLAZCAN_PLAZCANPACKET_HPP_

#include <DroneDevice/PlazCan/UavCanPacket.hpp>

namespace PlazCan {

struct GetFieldsSummaryResponse {
	uint8_t number;
} __attribute__((packed));

struct GetFieldInfoRequest {
	uint8_t field;
} __attribute__((packed));

struct GetFieldInfoResponseError {
	uint8_t result;
} __attribute__((packed));

template<size_t typeLength, size_t unitLength, size_t nameLength>
struct GetFieldInfoResponse {
	uint8_t result;
	uint8_t type;
	int8_t scale;

	uint8_t arena[2 * (1 + typeLength) + (1 + unitLength) + nameLength];
} __attribute__((packed));

struct FieldReadRequest {
	uint8_t field;
} __attribute__((packed));

template<size_t typeLength>
struct FieldReadResponse {
	uint8_t result;
	uint8_t data[typeLength];
} __attribute__((packed));

template<>
struct FieldReadResponse<0> {
	uint8_t result;
} __attribute__((packed));

template<size_t typeLength>
struct FieldWriteRequest {
	uint8_t field;
	uint8_t data[typeLength];

	FieldWriteRequest() :
		field{},
		data{}
	{
	}

	FieldWriteRequest(Device::FieldId aField, const void *aData, size_t aLength) :
		field{aField}
	{
		memcpy(data, aData, aLength);
	}
} __attribute__((packed));

template<>
struct FieldWriteRequest<0> {
	uint8_t field;
} __attribute__((packed));

struct FieldWriteResponse {
	uint8_t result;
} __attribute__((packed));

struct GetFileInfoRequest {
	uint8_t file;
	uint8_t flags;
} __attribute__((packed));

struct GetFileInfoResponseError {
	uint8_t result;
} __attribute__((packed));

struct GetFileInfoResponse {
	uint8_t result;
	uint8_t status;
	uint32_t size;
	uint32_t checksum;
} __attribute__((packed));

struct FileReadRequest {
	uint8_t file;
	uint32_t position;
	uint8_t length;
} __attribute__((packed));

template<size_t dataLength>
struct FileReadResponse {
	uint8_t result;
	uint8_t data[dataLength];
} __attribute__((packed));

template<>
struct FileReadResponse<0> {
	uint8_t result;
} __attribute__((packed));

template<size_t dataLength>
struct FileWriteRequest {
	uint8_t file;
	uint32_t position;
	uint8_t data[dataLength];

	FileWriteRequest(Device::FileId aFile, uint32_t aPosition, const void *aData, size_t aLength) :
		file{aFile},
		position{aPosition}
	{
		memcpy(data, aData, aLength);
	}
} __attribute__((packed));

template<>
struct FileWriteRequest<0> {
	uint8_t file;
	uint32_t position;
} __attribute__((packed));

struct FileWriteResponse {
	uint8_t result;
} __attribute__((packed));

}

#endif // DRONEDEVICE_PLAZCAN_PLAZCANPACKET_HPP_
