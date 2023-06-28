//
// PlainText.hpp
//
//  Created on: Dec 19, 2017
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_PLAINTEXT_HPP_
#define DRONEDEVICE_STUBS_PLAINTEXT_HPP_

#include <cstddef>

namespace Device {

class PlainText {
public:
	static constexpr size_t kBlockSize = 16;

	void reset()
	{
	}

	void decrypt(void *)
	{
	}
};

} // namespace Device

#endif // DRONEDEVICE_STUBS_PLAINTEXT_HPP_
