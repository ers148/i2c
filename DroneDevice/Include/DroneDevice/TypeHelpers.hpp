//
// TypeHelpers.hpp
//
//  Created on: Mar 14, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_TYPEHELPERS_HPP_
#define DRONEDEVICE_TYPEHELPERS_HPP_

namespace TypeHelpers {

template<typename T, typename M> M getMemberType(M T::*);
template<typename T, typename M> T getClassType(M T::*);

}

#endif // DRONEDEVICE_TYPEHELPERS_HPP_
