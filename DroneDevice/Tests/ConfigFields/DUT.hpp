//
// DUT.hpp
//
//  Created on: Nov 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_TESTS_GENERICFIELDS_DUT_HPP_
#define DRONEDEVICE_TESTS_GENERICFIELDS_DUT_HPP_

#include <DroneDevice/InternalDevice/ConfigField.hpp>
#include <DroneDevice/InternalDevice/InternalDevice.hpp>

template<typename T, typename U>
class DUT : public Device::InternalDevice {
public:
	DUT(const char *aAlias, const Device::Version &aVersion, T &aStorage) :
		Device::InternalDevice{aAlias, aVersion},
		storage{aStorage},
		coeff0Field{storage, &U::coeff0, "coeff0Field"},
		coeff1Field{storage, &U::coeff1, "coeff1Field"},
		coeff2Field{storage, &U::coeff2, "coeff2Field"},
		coeff3Field{storage, &U::coeff3, "coeff3Field"},
		arrayCoeffField{storage, &U::arrayCoeff, "arrayCoeffField"},
		fieldsArray{{
			&arrayCoeffField,
			&coeff0Field,
			&coeff1Field,
			&coeff2Field,
			&coeff3Field
		}}
	{
	}

	size_t getFieldCount() override
	{
		return fieldsArray.size();
	}

protected:
	Device::AbstractField *getField(Device::FieldId aIndex) override
	{
		return aIndex < fieldsArray.size() ? fieldsArray[aIndex] : nullptr;
	}

private:
	// Config storage
	T &storage;

public:
	// Default field with disabled write through mode
	Device::ConfigField<T, decltype(&U::coeff0), false> coeff0Field;

	// Test write through mode
	Device::ConfigField<T, decltype(&U::coeff1), true> coeff1Field;

	// Test read-only
	Device::ConfigField<T, decltype(&U::coeff2), false, true> coeff2Field;

	// Test scale
	Device::ConfigField<T, decltype(&U::coeff3), false, false, 3> coeff3Field;

	// Test dimension
	Device::ConfigField<T, decltype(&U::arrayCoeff), false, false, -6, 2> arrayCoeffField;

private:
	// Field container
	const std::array<Device::AbstractField *, 5> fieldsArray;
};

#endif // DRONEDEVICE_TESTS_GENERICFIELDS_DUT_HPP_
