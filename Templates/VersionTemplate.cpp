//
// Version.cpp
//
//  Created on: Nov 29, 2017
//      Author: Alexander
//

#include "Version.hpp"

const char Version::kName[] = "${VERSION_HW_NAME}";
const Device::Version Version::kVersion = {
	{Version::kHwMajor, Version::kHwMinor},
	{Version::kSwMajor, Version::kSwMinor, Version::kSwHash, Version::kSwRevision}
};
