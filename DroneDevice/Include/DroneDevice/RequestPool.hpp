//
// RequestPool.hpp
//
//  Created on: Feb 2, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_REQUESTPOOL_HPP_
#define DRONEDEVICE_REQUESTPOOL_HPP_

#include <DroneDevice/Queue.hpp>
#include <array>
#include <new>

namespace Device {

template<typename T, size_t size>
class RequestPool {
public:
	RequestPool(const RequestPool<T, size> &) = delete;
	RequestPool<T, size> &operator=(const RequestPool<T, size> &) = delete;

	RequestPool() :
		pool{},
		requests{}
	{
		for (auto &element : pool) {
			requests.push(&element);
		}
	}

	template<typename... Ts>
	T *alloc(Ts... aArgs)
	{
		if (!requests.empty()) {
			T * const request = requests.pop();
			new (request) T{aArgs...};
			return request;
		} else {
			return nullptr;
		}
	}

	void free(const T *aRequest)
	{
		if (!aRequest->refs) {
			requests.push(const_cast<T *>(aRequest));
		}
	}

	size_t count() const
	{
		return requests.size();
	}

private:
	std::array<T, size> pool;
	Queue<T *, size> requests;
};

} // namespace Device

#endif // DRONEDEVICE_REQUESTPOOL_HPP_
