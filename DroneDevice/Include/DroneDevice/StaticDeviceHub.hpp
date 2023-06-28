//
// StaticDeviceHub.hpp
//
//  Created on: Oct 20, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_STATICDEVICEHUB_HPP_
#define DRONEDEVICE_STATICDEVICEHUB_HPP_

#include <functional>
#include <tuple>
#include <DroneDevice/AbstractDevice.hpp>
#include <DroneDevice/Address.hpp>
#include <DroneDevice/DeviceObserver.hpp>

namespace Device {

template<typename... Devices>
class StaticDeviceHub {
public:
	StaticDeviceHub(Devices... args) :
		elements{std::tie(args...)}
	{
	}

	// Synchronous function

	void deviceRequestInfo(DeviceId aDevice, std::function<void (AbstractDevice *, RefCounter *)> aCallback,
		RefCounter *aToken)
	{
		DeviceIterator<0, Devices...>::deviceRequestInfoImpl(elements, aDevice, aCallback, aToken);
	}

	// Fields

	void fieldRequestInfo(DeviceId aDevice, FieldId aField, DeviceObserver *aObserver, RefCounter *aToken)
	{
		DeviceIterator<0, Devices...>::fieldRequestInfoImpl(elements, aDevice, aField, aObserver, aToken);
	}

	void fieldRead(DeviceId aDevice, FieldId aField, DeviceObserver *aObserver, RefCounter *aToken)
	{
		DeviceIterator<0, Devices...>::fieldReadImpl(elements, aDevice, aField, aObserver, aToken);
	}

	void fieldWrite(DeviceId aDevice, FieldId aField, const void *aBuffer, DeviceObserver *aObserver,
		RefCounter *aToken)
	{
		DeviceIterator<0, Devices...>::fieldWriteImpl(elements, aDevice, aField, aBuffer, aObserver, aToken);
	}

	// Files

	void fileRequestInfo(DeviceId aDevice, FileId aFile, FileFlags aFlags, DeviceObserver *aObserver,
		RefCounter *aToken)
	{
		DeviceIterator<0, Devices...>::fileRequestInfoImpl(elements, aDevice, aFile, aFlags, aObserver, aToken);
	}

	void fileRead(DeviceId aDevice, FileId aFile, uint32_t aOffset, size_t aSize, DeviceObserver *aObserver,
		RefCounter *aToken)
	{
		DeviceIterator<0, Devices...>::fileReadImpl(elements, aDevice, aFile, aOffset, aSize, aObserver, aToken);
	}

	void fileWrite(DeviceId aDevice, FileId aFile, uint32_t aOffset, const void *aBuffer, size_t aSize,
		DeviceObserver *aObserver, RefCounter *aToken)
	{
		DeviceIterator<0, Devices...>::fileWriteImpl(elements, aDevice, aFile, aOffset, aBuffer, aSize, aObserver,
			aToken);
	}

private:
	std::tuple<Devices...> elements;

	template<size_t N, typename... Ts>
	struct DeviceIterator {
		using EntryType = typename std::tuple_element<N, std::tuple<Ts...>>::type;

		static void deviceRequestInfoImpl(std::tuple<Ts...> aTuple, DeviceId aDevice,
			std::function<void (AbstractDevice *, RefCounter *)> aCallback, RefCounter *aToken)
		{
			if ((aDevice == kDeviceReservedId || aDevice == std::get<N>(aTuple).address()) && std::get<N>(aTuple).address() != kDeviceReservedId) {
				if (aToken != nullptr) {
					aToken->destination = std::get<N>(aTuple).address();
					aToken->refs = static_cast<decltype(aToken->refs)>(N);
				}

				if (aCallback != nullptr) {
					aCallback(std::get<N>(aTuple).node(), aToken);
				}
			}

			if (aDevice == kDeviceReservedId) {
				DeviceIterator<N + 1, Ts...>::deviceRequestInfoImpl(aTuple, aDevice, aCallback, aToken);
			}
		}

		static void fieldRequestInfoImpl(std::tuple<Ts...> aTuple, DeviceId aDevice, FieldId aField,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aDevice == std::get<N>(aTuple).address()) {
				std::get<N>(aTuple).node()->fieldRequestInfo(aField, aObserver, aToken);
			} else {
				DeviceIterator<N + 1, Ts...>::fieldRequestInfoImpl(aTuple, aDevice, aField, aObserver, aToken);
			}
		}

		static void fieldReadImpl(std::tuple<Ts...> aTuple, DeviceId aDevice, FieldId aField,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aDevice == std::get<N>(aTuple).address()) {
				std::get<N>(aTuple).node()->fieldRead(aField, aObserver, aToken);
			} else {
				DeviceIterator<N + 1, Ts...>::fieldReadImpl(aTuple, aDevice, aField, aObserver, aToken);
			}
		}

		static void fieldWriteImpl(std::tuple<Ts...> aTuple, DeviceId aDevice, FieldId aField, const void *aBuffer,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aDevice == std::get<N>(aTuple).address()) {
				std::get<N>(aTuple).node()->fieldWrite(aField, aBuffer, aObserver, aToken);
			} else {
				DeviceIterator<N + 1, Ts...>::fieldWriteImpl(aTuple, aDevice, aField, aBuffer, aObserver, aToken);
			}
		}

		static void fileRequestInfoImpl(std::tuple<Ts...> aTuple, DeviceId aDevice, FileId aFile, FileFlags aFlags,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aDevice == std::get<N>(aTuple).address()) {
				std::get<N>(aTuple).node()->fileRequestInfo(aFile, aFlags, aObserver, aToken);
			} else {
				DeviceIterator<N + 1, Ts...>::fileRequestInfoImpl(aTuple, aDevice, aFile, aFlags, aObserver, aToken);
			}
		}

		static void fileReadImpl(std::tuple<Ts...> aTuple, DeviceId aDevice, FileId aFile, uint32_t aOffset,
			size_t aSize, DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aDevice == std::get<N>(aTuple).address()) {
				std::get<N>(aTuple).node()->fileRead(aFile, aOffset, aSize, aObserver, aToken);
			} else {
				DeviceIterator<N + 1, Ts...>::fileReadImpl(aTuple, aDevice, aFile, aOffset, aSize, aObserver, aToken);
			}
		}

		static void fileWriteImpl(std::tuple<Ts...> aTuple, DeviceId aDevice, FileId aFile, uint32_t aOffset,
			const void *aBuffer, size_t aSize, DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aDevice == std::get<N>(aTuple).address()) {
				std::get<N>(aTuple).node()->fileWrite(aFile, aOffset, aBuffer, aSize, aObserver, aToken);
			} else {
				DeviceIterator<N + 1, Ts...>::fileWriteImpl(aTuple, aDevice, aFile, aOffset, aBuffer, aSize,
					aObserver, aToken);
			}
		}
	};

	template<typename... Ts>
	struct DeviceIterator<sizeof...(Devices), Ts...> {
		static void deviceRequestInfoImpl(std::tuple<Ts...>, DeviceId,
			std::function<void (AbstractDevice *, RefCounter *)>, RefCounter *)
		{
		}

		static void fieldRequestInfoImpl(std::tuple<Ts...>, DeviceId, FieldId aField, DeviceObserver *aObserver,
			RefCounter *aToken)
		{
			if (aObserver != nullptr) {
				aObserver->onFieldInfoRequestError(aField, Result::COMPONENT_NOT_FOUND, aToken);
			}
		}

		static void fieldReadImpl(std::tuple<Ts...>, DeviceId, FieldId aField, DeviceObserver *aObserver,
			RefCounter *aToken)
		{
			if (aObserver != nullptr) {
				aObserver->onFieldRequestError(aField, Result::COMPONENT_NOT_FOUND, aToken);
			}
		}

		static void fieldWriteImpl(std::tuple<Ts...>, DeviceId, FieldId field, const void *,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aObserver != nullptr) {
				aObserver->onFieldRequestError(field, Result::COMPONENT_NOT_FOUND, aToken);
			}
		}

		static void fileRequestInfoImpl(std::tuple<Ts...>, DeviceId, FileId aFile, FileFlags,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aObserver != nullptr) {
				aObserver->onFileInfoRequestError(aFile, Result::COMPONENT_NOT_FOUND, aToken);
			}
		}

		static void fileReadImpl(std::tuple<Ts...>, DeviceId, FileId aFile, uint32_t aOffset, size_t,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aObserver != nullptr) {
				aObserver->onFileReadEnd(aFile, aOffset, nullptr, 0, Result::COMPONENT_NOT_FOUND, aToken);
			}
		}

		static void fileWriteImpl(std::tuple<Ts...>, DeviceId, FileId aFile, uint32_t aOffset, const void *, size_t,
			DeviceObserver *aObserver, RefCounter *aToken)
		{
			if (aObserver != nullptr) {
				aObserver->onFileWriteEnd(aFile, aOffset, Result::COMPONENT_NOT_FOUND, aToken);
			}
		}
	};
};

template<typename... Ts>
constexpr auto makeStaticDeviceHub(Ts&&... aArgs)
{
	return StaticDeviceHub<std::remove_reference_t<Ts>...>(std::forward<Ts>(aArgs)...);
}

} // namespace Device

#endif // DRONEDEVICE_STATICDEVICEHUB_HPP_
