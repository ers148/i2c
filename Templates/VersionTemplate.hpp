//
// Version.hpp
//
//  Created on: Nov 29, 2017
//      Author: Alexander
//

#ifndef VERSION_HPP_
#define VERSION_HPP_

#include <DroneDevice/CoreTypes.hpp>

struct Version {
	Version() = delete;
	Version(const Version &) = delete;
	Version &operator=(const Version &) = delete;

	static constexpr uint8_t kHwMajor = ${VERSION_HW_MAJOR};
	static constexpr uint8_t kHwMinor = ${VERSION_HW_MINOR};

	static constexpr uint8_t kSwMajor = ${VERSION_SW_MAJOR};
	static constexpr uint8_t kSwMinor = ${VERSION_SW_MINOR};
	static constexpr uint32_t kSwHash = static_cast<uint32_t>(0x${VERSION_SW_HASH});
	static constexpr uint32_t kSwRevision = ${VERSION_SW_REVISION};

	static const char kName[];
	static const Device::Version kVersion;
};

#endif // VERSION_HPP_
