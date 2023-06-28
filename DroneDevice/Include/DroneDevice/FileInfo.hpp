//
// FileInfo.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_FILEINFO_HPP_
#define DRONEDEVICE_FILEINFO_HPP_

#include <DroneDevice/CoreTypes.hpp>

namespace Device {

struct FileInfo {
	FileId index;
	FileFlags flags;
	uint32_t size;
	uint32_t checksum;
};

} // namespace Device

#endif // DRONEDEVICE_FILEINFO_HPP_
