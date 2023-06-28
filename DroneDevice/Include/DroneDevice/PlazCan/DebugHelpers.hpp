//
// DebugHelpers.hpp
//
//  Created on: Jul 21, 2020
//      Author: Alexander
//

#ifndef DRONEDEVICE_PLAZCAN_DEBUGHELPERS_HPP_
#define DRONEDEVICE_PLAZCAN_DEBUGHELPERS_HPP_

#include <DroneDevice/PlazCan/UavCanPacket.hpp>

namespace PlazCan {

namespace DebugHelpers {

static std::string uidToString(PlazCan::UidType aUid)
{
	auto binToHex = [](uint8_t c){ return static_cast<uint8_t>(c + (c < 10 ? 0x30 : 0x57)); };

	std::string res;
	res.reserve(aUid.size() * 2);
	res.resize(aUid.size() * 2);

	for (size_t i = 0; i < aUid.size(); ++i) {
		res[i * 2] = binToHex(static_cast<uint8_t>(aUid[i] >> 4));
		res[i * 2 + 1] = binToHex(static_cast<uint8_t>(aUid[i] & 0x0F));
	}

	res[aUid.size() * 2] = '\0';
	return res;
}

} // namespace DebugHelpers

} // namespace PlazCan

#endif // DRONEDEVICE_PLAZCAN_DEBUGHELPERS_HPP_
