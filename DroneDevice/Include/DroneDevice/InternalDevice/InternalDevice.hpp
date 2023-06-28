//
// InternalDevice.hpp
//
//  Created on: Oct 20, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_INTERNALDEVICE_HPP_
#define DRONEDEVICE_INTERNALDEVICE_INTERNALDEVICE_HPP_

#include <DroneDevice/AbstractDevice.hpp>
#include <DroneDevice/InternalDevice/AbstractField.hpp>
#include <DroneDevice/InternalDevice/AbstractFile.hpp>
#include <DroneDevice/Stubs/MockUid.hpp>
#include <DroneDevice/TypeHelpers.hpp>

namespace Device {

class InternalDevice : public AbstractDevice {
	static_assert(sizeof(DeviceHash) % 4 == 0, "Device hash size is not aligned");

public:
	InternalDevice(const InternalDevice &) = delete;
	InternalDevice &operator=(const InternalDevice &) = delete;

	InternalDevice(const char *aName = nullptr, const Version &aVersion = Version{}) :
		name{aName},
		version{aVersion},
		hash{0}
	{
	}

	// Generic functions

	DeviceHash deviceHash() const override
	{
		return hash;
	}

	const char *deviceName() const override
	{
		return name;
	}

	Version deviceVersion() const override
	{
		return version;
	}

	// Synchronous field functions

	size_t getFieldCount() override
	{
		return 0;
	}

	// Asynchronous field functions

	void fieldRequestIndex(const char *aField, DeviceObserver *aObserver, RefCounter *aToken) override
	{
		if (aObserver != nullptr) {
			const auto index = getFieldIndex(aField);

			if (index != kFieldReservedId) {
				aObserver->onFieldIndexReceived(index, aToken);
			} else {
				aObserver->onFieldIndexRequestError(Result::FIELD_NOT_FOUND, aToken);
			}
		}
	}

	void fieldRequestInfo(FieldId aField, DeviceObserver *aObserver, RefCounter *aToken) override
	{
		if (aObserver != nullptr) {
			const auto * const field = getField(aField);

			if (field != nullptr) {
				const FieldInfo info{
					aField,
					field->dimension(),
					field->flags(),
					field->scale(),
					field->type(),
					field->name(),
					field->unit(),
					field->min(),
					field->max()
				};
				uint8_t value[kFieldMaxSize];
				const auto result = field->read(value);

				if (result == Result::SUCCESS) {
					aObserver->onFieldInfoReceived(info, value, aToken);
				} else {
					aObserver->onFieldInfoRequestError(aField, result, aToken);
				}
			} else {
				aObserver->onFieldInfoRequestError(aField, Result::FIELD_NOT_FOUND, aToken);
			}
		}
	}

	void fieldRead(FieldId aField, DeviceObserver *aObserver, RefCounter *aToken) override
	{
		if (aObserver != nullptr) {
			const auto * const field = getField(aField);

			if (field != nullptr) {
				uint8_t value[kFieldMaxSize];
				const auto result = field->read(value);

				if (result == Result::SUCCESS) {
					aObserver->onFieldReceived(aField, value, field->type(), field->dimension(), aToken);
				} else {
					aObserver->onFieldRequestError(aField, result, aToken);
				}
			} else {
				aObserver->onFieldRequestError(aField, Result::FIELD_NOT_FOUND, aToken);
			}
		}
	}

	void fieldWrite(FieldId aField, const void *aBuffer, DeviceObserver *aObserver,
		RefCounter *aToken) override
	{
		auto * const field = getField(aField);
		auto result = Result::SUCCESS;

		if (field == nullptr) {
			result = Result::FIELD_NOT_FOUND;
		} else {
			result = field->write(aBuffer);
		}

		if (aObserver != nullptr) {
			if (result == Result::SUCCESS) {
				aObserver->onFieldUpdated(aField, aToken);
			} else {
				aObserver->onFieldRequestError(aField, result, aToken);
			}
		}
	}

	// Asynchronous file functions

	void fileRequestInfo(FileId aFile, FileFlags aFlags, DeviceObserver *aObserver,
		RefCounter *aToken) override
	{
		const auto * const file = getFile(aFile);

		if (file != nullptr) {
			// Invoke size and checksum functions even if observer is not defined
			const FileInfo info{
				aFile,
				file->getFlags(),
				file->getSize(),
				(aFlags & kFileReqChecksum) ? file->getChecksum() : 0
			};

			if (aObserver != nullptr) {
				aObserver->onFileInfoReceived(info, aToken);
			}
		} else {
			if (aObserver != nullptr) {
				aObserver->onFileInfoRequestError(aFile, Result::FILE_NOT_FOUND, aToken);
			}
		}
	}

	void fileRead(FileId aFile, uint32_t aOffset, size_t aChunkSize, DeviceObserver *aObserver,
		RefCounter *aToken) override
	{
		const auto * const file = getFile(aFile);
		Result result = Result::SUCCESS;

		if (file == nullptr) {
			result = Result::FILE_NOT_FOUND;
		} else if (aChunkSize > kFileMaxChunkLength) {
			result = Result::FILE_ERROR;
		} else {
			uint8_t buffer[kFileMaxChunkLength];

			if (file->readChunk(aOffset, buffer, aChunkSize)) {
				if (aObserver != nullptr) {
					aObserver->onFileReadEnd(aFile, aOffset, buffer, aChunkSize, Result::SUCCESS, aToken);
				}
			} else {
				result = Result::FILE_POSITION_ERROR;
			}
		}

		if (result != Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFileReadEnd(aFile, aOffset, nullptr, 0, result, aToken);
		}
	}

	void fileWrite(FileId aFile, uint32_t aOffset, const void *aBuffer, size_t aChunkSize,
		DeviceObserver *aObserver, RefCounter *aToken) override
	{
		auto * const file = getFile(aFile);
		Result result = Result::SUCCESS;

		if (file != nullptr) {
			bool completed = false;

			if (aChunkSize == 0) {
				if (aOffset == 0) {
					completed = file->restartWrite();
				} else {
					completed = file->finalizeWrite(aOffset);
				}
			} else {
				completed = file->writeChunk(aOffset, aBuffer, aChunkSize);
			}

			if (completed) {
				if (aObserver != nullptr) {
					aObserver->onFileWriteEnd(aFile, aOffset, Result::SUCCESS, aToken);
				}
			} else {
				result = Result::FILE_ERROR;
			}
		} else {
			result = Result::FILE_NOT_FOUND;
		}

		if (result != Result::SUCCESS && aObserver != nullptr) {
			aObserver->onFileWriteEnd(aFile, aOffset, result, aToken);
		}
	}

	template<typename T, typename U = MockUid>
	void makeDeviceHash()
	{
		// Generate device hash from the Unique Identifier of the processor
		makeHashFromUid<T, U>(0);
	}

	template<typename T, typename U = MockUid>
	void makeDeviceHashFromName()
	{
		// Generate device hash from device name and UID
		typename T::Type seed = calcSeedFromName<T>(0, name);

		makeHashFromUid<T, U>(seed);
	}

	template<typename T, typename U = MockUid>
	void makeDeviceHashFromDevice()
	{
		// Generate device hash from device fields, name and UID
		typename T::Type seed = calcSeedFromName<T>(0, name);

		for (size_t i = 0; i < getFieldCount(); ++i) {
			const auto * const field = getField(static_cast<FieldId>(i));

			seed = calcSeedFromName<T>(seed, field->name());
			seed = calcSeedFromMeta<T>(seed, field->type(), field->dimension(), field->scale());
		}

		makeHashFromUid<T, U>(seed);
	}

	template<typename T>
	__attribute__((deprecated)) void updateDeviceHash()
	{
		makeDeviceHashFromDevice<T>();
	}

	virtual AbstractField *getField(FieldId /*aIndex*/)
	{
		return nullptr;
	}

	virtual AbstractFile *getFile(FileId /*aIndex*/)
	{
		return nullptr;
	}

	FieldId getFieldIndex(const char *aName)
	{
		for (FieldId i = 0; i < getFieldCount(); ++i) {
			if (strcmp(getField(i)->name(), aName) == 0) {
				return i;
			}
		}

		return kFieldReservedId;
	}

private:
	template<typename T, typename Scalar = uint16_t>
	typename T::Type calcSeedFromMeta(typename T::Type aInitialValue, FieldType aType,
		FieldDimension aDimension, FieldScale aScale)
	{
		static_assert(sizeof(DeviceHash) >= 3 * sizeof(Scalar), "Incorrect hash type");

		DeviceHash tmp;
		Scalar value;

		memset(tmp.data(), 0, sizeof(tmp));
		value = static_cast<Scalar>(aType);
		memcpy(tmp.data() + 0 * sizeof(value), &value, sizeof(value));
		value = static_cast<Scalar>(aDimension);
		memcpy(tmp.data() + 1 * sizeof(value), &value, sizeof(value));
		value = static_cast<Scalar>(aScale);
		memcpy(tmp.data() + 2 * sizeof(value), &value, sizeof(value));

		return T::update(aInitialValue, tmp.data(), sizeof(tmp));
	}

	template<typename T>
	typename T::Type calcSeedFromName(typename T::Type aInitialValue, const char *aPosition)
	{
		size_t length = strlen(aPosition);
		DeviceHash tmp;

		while (length) {
			const size_t chunk = std::min(length, sizeof(tmp));

			memset(tmp.data(), 0, sizeof(tmp));
			memcpy(tmp.data(), aPosition, chunk);

			aInitialValue = T::update(aInitialValue, tmp.data(), sizeof(tmp));

			length -= chunk;
			aPosition += chunk;
		}

		return aInitialValue;
	}

	template<typename T, typename U>
	void makeHashFromUid(typename T::Type aInitialValue)
	{
		DeviceHash tmp;

		memset(tmp.data(), 0, sizeof(tmp));
		memcpy(tmp.data(), U::data(), std::min(U::length(), sizeof(tmp)));

		for (size_t i = 0; i < tmp.size(); i += sizeof(typename T::Type)) {
			aInitialValue = T::update(aInitialValue, &tmp[i], sizeof(typename T::Type));
			memcpy(hash.data() + i, &aInitialValue, sizeof(typename T::Type));
		}
	}

protected:
	const char *name;
	const Version version;
	DeviceHash hash;
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_HPP_
