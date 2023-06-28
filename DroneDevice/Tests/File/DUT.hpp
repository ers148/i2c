//
// DUT.hpp
//
//  Created on: Nov 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_TESTS_EXTENDEDFIELDS_DUT_HPP_
#define DRONEDEVICE_TESTS_EXTENDEDFIELDS_DUT_HPP_

#include <DroneDevice/InternalDevice/MemoryFile.hpp>
#include <DroneDevice/InternalDevice/InternalDevice.hpp>

class DUT : public Device::InternalDevice {
public:
	DUT(const char *aAlias, const Device::Version &aVersion):
		Device::InternalDevice{aAlias, aVersion}
	{
	}

	Device::MemoryFile<32> file;

	Device::AbstractFile *getFile(Device::FileId) override
	{
		return &file;
	}
};

#endif // DRONEDEVICE_TESTS_EXTENDEDFIELDS_DUT_HPP_
