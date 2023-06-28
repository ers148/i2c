//
// ConfigStorage.hpp
//
//  Created on: Mar 2, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_CONFIGSTORAGE_HPP_
#define DRONEDEVICE_CONFIGSTORAGE_HPP_

#include <DroneDevice/TypeHash.hpp>
#include <cstddef>
#include <limits>
#include <tuple>

template<size_t index, size_t offset, typename Flash, typename Crc, typename Layout, typename... Ts>
class ConfigRwHandler {
public:
	static bool load(Layout &)
	{
		return true;
	}

	static bool store(const Layout &)
	{
		return true;
	}
};

template<typename T, size_t count>
struct AlignedConfigWrapper {
	AlignedConfigWrapper() :
		payload{},
		padding{0}
	{
	}

	// TODO Movement
	AlignedConfigWrapper(const T &aPayload) :
		payload{aPayload},
		padding{0}
	{
	}

	operator T() const
	{
		return payload;
	}

private:
	T payload;
	uint8_t padding[count];
} __attribute__((packed));

template<typename T, size_t count>
struct ConfigWrapperImpl {
	using Type = AlignedConfigWrapper<T, count>;
};

template<typename T>
struct ConfigWrapperImpl<T, 0> {
	using Type = T;
};

template<typename T, size_t alignment>
struct ConfigWrapper {
	static constexpr size_t count = sizeof(T) % alignment != 0 ? alignment - sizeof(T) % alignment : 0;
	typename ConfigWrapperImpl<T, count>::Type container;

	ConfigWrapper() :
		container{}
	{
	}

	ConfigWrapper(const T &aPayload) :
		container{aPayload}
	{
	}

	operator T() const
	{
		return static_cast<T>(container);
	}
};

template<size_t index, size_t offset, typename Flash, typename Crc, typename Layout, typename T, typename... Ts>
class ConfigRwHandler<index, offset, Flash, Crc, Layout, T, Ts...> {
	using CrcType = decltype(Crc::update(0, nullptr, 0));
	using WrapperType = ConfigWrapper<T, sizeof(CrcType)>;

	static constexpr size_t kNextOffset = offset + sizeof(WrapperType) + sizeof(CrcType);
	static constexpr TypeHash::Type kTypeHash = TypeHash::hash<T>();

	static_assert(sizeof(CrcType) == sizeof(TypeHash::Type), "Incorrect CRC type");

public:
	static bool load(Layout &values)
	{
		static const auto typeHash = kTypeHash;

		CrcType expectedChecksum;
		WrapperType valueBuffer;

		Flash::read(offset, &valueBuffer, sizeof(valueBuffer));
		Flash::read(offset + sizeof(valueBuffer), &expectedChecksum, sizeof(expectedChecksum));

		// Calculate checksum for type descriptor and data
		CrcType actualChecksum;
		actualChecksum = Crc::update(0, &typeHash, sizeof(actualChecksum));
		actualChecksum = Crc::update(actualChecksum, &valueBuffer, sizeof(valueBuffer));

		if (expectedChecksum != std::numeric_limits<CrcType>::max() && actualChecksum == expectedChecksum) {
			std::get<T>(values) = valueBuffer;
			return ConfigRwHandler<index + 1, kNextOffset, Flash, Crc, Layout, Ts...>::load(values);
		} else {
			return false;
		}
	}

	static bool store(const Layout &values)
	{
		static const auto typeHash = kTypeHash;

		const WrapperType valueBuffer{std::get<T>(values)};
		CrcType checksum;

		checksum = Crc::update(0, &typeHash, sizeof(checksum));
		checksum = Crc::update(checksum, &valueBuffer, sizeof(valueBuffer));

		if (!Flash::write(offset, &valueBuffer, sizeof(valueBuffer)))
			return false;
		if (!Flash::write(offset + sizeof(valueBuffer), &checksum, sizeof(checksum)))
			return false;

		return ConfigRwHandler<index + 1, kNextOffset, Flash, Crc, Layout, Ts...>::store(values);
	}
};

template<typename Flash, typename Crc, typename... Ts>
class ConfigStorage {
public:
	ConfigStorage(bool aLoadData = false)
	{
		if (aLoadData) {
			load();
		}
	}

	bool load()
	{
		return ConfigRwHandler<0, 0, Flash, Crc, std::tuple<Ts...>, Ts...>::load(shadow);
	}

	bool store()
	{
		bool status;

		Flash::unlock();
		if ((status = Flash::erase())) {
			status = ConfigRwHandler<0, 0, Flash, Crc, std::tuple<Ts...>, Ts...>::store(shadow);
		}
		Flash::lock();

		return status;
	}

	// TODO Make private
	std::tuple<Ts...> shadow;
};

template<typename T, typename Storage>
T &getStorageEntry(Storage &storage)
{
	return std::get<T>(storage.shadow);
}

#endif // DRONEDEVICE_CONFIGSTORAGE_HPP_
