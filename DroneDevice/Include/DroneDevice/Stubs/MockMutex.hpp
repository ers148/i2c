//
// MockMutex.hpp
//
//  Created on: Mar 11, 2021
//      Author: Alexander
//

#ifndef DRONEDEVICE_STUBS_MOCKMUTEX_HPP_
#define DRONEDEVICE_STUBS_MOCKMUTEX_HPP_

#include <cstdint>

namespace Device {

struct MockMutex {
  void lock()
  {
  }

  void unlock()
  {
  }
};

} // namespace Device

#endif // DRONEDEVICE_STUBS_MOCKMUTEX_HPP_
