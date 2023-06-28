//
// HashFinder.hpp
//
//  Created on: Feb 15, 2022
//      Author: Aleksandr Sazhin
//

#ifndef DRONEDEVICE_PLAZCAN_HASHFINDER_HPP_
#define DRONEDEVICE_PLAZCAN_HASHFINDER_HPP_

#include <DroneDevice/PlazCan/CanardWrapper.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

template<uint16_t id, uint64_t hash>
struct HashId {
	static constexpr uint16_t kId{id};
	static constexpr uint64_t kHash{hash};
};

template<class... T>
class HashFinder {
public:
	static constexpr uint64_t findHashById(uint16_t aId)
	{
		return findImpl<T...>(aId);
	}

	static constexpr uint64_t findHashById(CanardTransferType aType, uint16_t aId)
	{
		if (aType == CanardTransferTypeBroadcast) {
			return findImpl<T...>(aId);
		}
		return 0ull;
	}

private:
	static constexpr size_t kLengthThreshold{11};

	template<class... U>
	static constexpr std::enable_if_t<sizeof...(U) >= kLengthThreshold, uint64_t> findImpl(uint16_t aId)
	{
		constexpr std::array<uint64_t, sizeof...(U)> hashArr{{U::kHash...}};
		constexpr std::array<uint16_t, sizeof...(U)> idArr{{U::kId...}};

		for (size_t i = 0; i < idArr.size(); ++i) {
			if (idArr[i] == aId) {
				return hashArr[i];
			}
		}

		return 0ull;
	}

	template<class... U>
	static constexpr std::enable_if_t<sizeof...(U) < kLengthThreshold, uint64_t> findImpl(uint16_t aId)
	{
		return helpImpl<U...>(aId);
	}

	template<typename K>
	static constexpr uint64_t helpImpl(uint16_t aId)
	{
		return K::kId == aId ? K::kHash : 0ull;
	}

	template<typename K1, typename K2, typename... Ks>
	static constexpr uint64_t helpImpl(uint16_t aId)
	{
		return aId == K1::kId ? helpImpl<K1>(aId) : helpImpl<K2, Ks...>(aId);
	}
};

#endif // DRONEDEVICE_PLAZCAN_HASHFINDER_HPP_
