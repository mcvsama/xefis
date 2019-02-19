/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__SI__CONVERSION_H__INCLUDED
#define NEUTRINO__SI__CONVERSION_H__INCLUDED

// Standard:
#include <cstddef>

// Local:
#include "unit.h"


namespace si {

// Forward
class QuantityBase;

// Forward
template<class pUnit, class pValue>
	class Quantity;


/**
 * Implicit converter object. Enables explicit conversion between types with different Angle exponents.
 */
template<class SourceUnit, class Value>
	class QuantityConverter
	{
	  public:
		// Ctor
		explicit constexpr
		QuantityConverter (Quantity<SourceUnit, Value> const& quantity):
			_quantity (quantity)
		{ }

		// Conversion operator
		template<class TargetUnit,
				 class = std::enable_if_t<is_convertible<SourceUnit, TargetUnit>() || is_convertible_with_angle<SourceUnit, TargetUnit>()>>
			constexpr
			operator Quantity<TargetUnit, Value>() const noexcept
			{
				if constexpr (is_convertible<SourceUnit, TargetUnit>())
					return (SourceUnit::base_value (_quantity.value()) - to_floating_point<typename TargetUnit::Offset, Value>()) / to_floating_point<typename TargetUnit::Scale, Value>();
				else if constexpr (is_convertible_with_angle<SourceUnit, TargetUnit>())
				{
					int other_angle_exp_diff = TargetUnit::AngleExponent - SourceUnit::AngleExponent;
					// Conversion: cycles → radians (increasing AngleExponent) => return value * 2 * PI.
					// Conversion: radians → cycles (decreasing AngleExponent) => return value / (2 * PI).
					auto result = (_quantity.base_value() - to_floating_point<typename TargetUnit::Offset, Value>()) / to_floating_point<typename TargetUnit::Scale, Value>();

					// Manual powering to ensure constexprness:
					while (other_angle_exp_diff > 0)
					{
						result *= 2.0 * M_PI;
						--other_angle_exp_diff;
					}

					while (other_angle_exp_diff < 0)
					{
						result /= 2.0 * M_PI;
						++other_angle_exp_diff;
					}

					return Quantity<TargetUnit, Value> (result);
				}
			}

	  private:
		Quantity<SourceUnit, Value> _quantity;
	};


/**
 * Meta-function returning true if parameter is a Quantity type.
 * TODO use is_specialization<>
 */
template<class T>
	struct is_quantity: public std::integral_constant<bool, std::is_base_of_v<QuantityBase, T>>
	{ };


template<class T>
	constexpr bool is_quantity_v = is_quantity<T>::value;


/**
 * Convert value 'source' expressed in SourceUnits to TargetUnits.
 */
template<class SourceUnit, class TargetUnit, class Value,
		 class = std::enable_if_t<!is_quantity_v<Value> && is_convertible<SourceUnit, TargetUnit>()>>
	constexpr Value
	implicit_convert_value_to (Value source_value)
	{
		return (SourceUnit::base_value (source_value) - to_floating_point<typename TargetUnit::Offset, Value>()) / to_floating_point<typename TargetUnit::Scale, Value>();
	}


/**
 * Convert quantity 'source' to TargetUnits.
 */
template<class TargetUnit, class Value, class SourceUnit,
		 class = std::enable_if_t<!is_quantity_v<Value> && is_convertible<typename Quantity<SourceUnit, Value>::Unit, TargetUnit>()>>
	constexpr Value
	implicit_convert_to (Quantity<SourceUnit, Value> const& source_quantity)
	{
		return (base_value (source_quantity) - to_floating_point<typename TargetUnit::Offset, Value>()) / to_floating_point<typename TargetUnit::Scale, Value>();
	}


/**
 * Convert quantity 'source' to TargetUnits.
 */
template<class TargetUnit, class Value, class SourceUnit>
	constexpr Quantity<TargetUnit, Value>
	convert_to (Quantity<SourceUnit, Value> const& source_quantity)
	{
		return QuantityConverter (source_quantity);
	}


/**
 * Convert quantity 'source' to TargetUnits.
 */
template<class SourceUnit, class Value>
	constexpr QuantityConverter<typename Quantity<SourceUnit, Value>::Unit, Value>
	convert (Quantity<SourceUnit, Value> const& source_quantity)
	{
		return QuantityConverter (source_quantity);
	}


/**
 * Convert value 'source' expressed in 'source_unit's to 'target_unit's.
 */
template<class Value>
	constexpr Value
	convert (DynamicUnit const& source_unit, Value source_quantity, DynamicUnit const& target_unit)
	{
		// Assert that units are convertible (exponents vector the same):
		if (source_unit.exponents() != target_unit.exponents())
			throw IncompatibleTypes (source_unit, target_unit);

		// TODO support AngleExponent-conversion

		Value base_value = source_quantity * source_unit.scale().to_floating_point() + source_unit.offset().to_floating_point();
		Value result = (base_value - target_unit.offset().to_floating_point()) / target_unit.scale().to_floating_point();
		return result;
	}

} // namespace si

#endif

