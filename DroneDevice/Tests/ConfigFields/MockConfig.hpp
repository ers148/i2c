//
// MockConfig.hpp
//
//  Created on: Nov 19, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_TESTS_CONFIGFIELDS_MOCKCONFIG_HPP_
#define DRONEDEVICE_TESTS_CONFIGFIELDS_MOCKCONFIG_HPP_

#include <array>
#include <cstdint>

struct MockConfig {
	std::array<uint32_t, 2> arrayCoeff;
	uint32_t coeff0;
	uint32_t coeff1;
	uint32_t coeff2;
	uint32_t coeff3;

	MockConfig() :
	    arrayCoeff{{1, 2}},
	    coeff0{131072},
	    coeff1{131072},
	    coeff2{131072},
	    coeff3{131072}
	{
	}
};

struct AlignedConfig {
	std::array<uint32_t, 2> arrayCoeff;
	double coeff0;
	uint32_t coeff1;
	int32_t coeff2;
	int64_t coeff3;

	AlignedConfig() :
	    arrayCoeff{{1, 2}},
	    coeff0{1024.0},
	    coeff1{131072},
	    coeff2{1024},
	    coeff3{64}
	{
	}
};

struct UnalignedConfig {
	std::array<uint32_t, 2> arrayCoeff;
	float coeff0;
	uint32_t coeff1;
	uint32_t coeff2;
	int8_t coeff3;

	UnalignedConfig() :
	    arrayCoeff{{1, 2}},
	    coeff0{512.0f},
	    coeff1{131072},
	    coeff2{512},
	    coeff3{32}
	{
	}
} __attribute__((packed));

#endif // DRONEDEVICE_TESTS_CONFIGFIELDS_MOCKCONFIG_HPP_
