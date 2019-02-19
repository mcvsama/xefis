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

#ifndef NEUTRINO__SI__QUANTITY_H__INCLUDED
#define NEUTRINO__SI__QUANTITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>
#include <ratio>

// Local:
#include "conversion.h"
#include "unit.h"
#include "unit_traits.h"


namespace si {

/**
 * Common base for Quantity objects.
 */
class QuantityBase
{ };


/**
 * This represents a quantity of a unit. The type depends on given Unit,
 * and contains floating-point value that measures quantity of the unit.
 *
 * Example:
 *   typedef Quantity<Meter> Length;
 *
 * All arithmetic operators are defined only for Quantity of Units that have
 * Offset parameter equal to std::ratio<0>. This prevents misuse by eg. adding
 * 1°C + 2°C which in reality gives 274.15 K + 275.15 K = 549.3 K, which is
 * 276.15°C. Quantities of such units must be first converted to base-unit
 * Quantities, then operated on.
 */
template<class pUnit, class pValue = double>
	class Quantity: public QuantityBase
	{
		static_assert (std::is_floating_point<pValue>(), "pValue must be floating-point");

	  public:
		using Unit				= pUnit;
		using Value				= pValue;
		using NormalizedUnit	= si::NormalizedUnit<Unit>;

	  public:
		// Ctor
		explicit constexpr
		Quantity (Value value = 0) noexcept:
			_value (value)
		{ }

		/**
		 * Return the quantity of Units of measured value.
		 * TODO Rename to value() and _value
		 */
		constexpr Value
		value() const noexcept
		{
			return _value;
		}

		/**
		 * Return normalized version of this quantity (ie. Scale = 1, Offset = 0).
		 */
		constexpr auto
		normalized() const noexcept
		{
			return Quantity<NormalizedUnit, Value> (*this);
		}

		/**
		 * Return normalized value of this quantity.
		 */
		constexpr Value
		base_value() const noexcept
		{
			return Unit::base_value (_value);
		}

		/**
		 * Return quantity expressed in another unit.
		 */
		template<class OtherUnit>
			constexpr Value
			in() const noexcept
			{
				return implicit_convert_value_to<Unit, OtherUnit> (_value);
			}

		/**
		 * Convert quantity to another unit.
		 */
		template<class OtherUnit>
			constexpr Quantity<OtherUnit>
			to() const noexcept
			{
				return static_cast<Quantity<OtherUnit>> (*this);
			}

		Quantity&
		operator+= (Quantity other) noexcept
		{
			*this = *this + other;
			return *this;
		}

		Quantity&
		operator-= (Quantity other) noexcept
		{
			*this = *this - other;
			return *this;
		}

		Quantity&
		operator*= (Value scalar) noexcept
		{
			_value *= scalar;
			return *this;
		}

		Quantity&
		operator/= (Value scalar) noexcept
		{
			_value /= scalar;
			return *this;
		}

		/**
		 * Convert to Unit with the same exponents vector, but different scaling/offset.
		 */
		template<class TargetUnit,
				 class = std::enable_if_t<is_convertible<Unit, TargetUnit>()>>
			constexpr
			operator Quantity<TargetUnit, Value>() const noexcept
			{
				return Quantity<TargetUnit, Value> (implicit_convert_value_to<Unit, TargetUnit> (_value));
			}

	  private:
		Value _value;
	};


/**
 * Comparing quantity of the same unit, same scaling, same offset.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O, class Value>
	constexpr bool
	operator== (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> b) noexcept
	{
		return a.value() == b.value();
	}


/**
 * Comparing quantities of the same unit with different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr bool
	operator== (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.base_value() == b.base_value();
	}


/**
 * Comparing quantity of the same unit, same scaling, same offset.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O, class Value>
	constexpr bool
	operator!= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> b) noexcept
	{
		return a.value() != b.value();
	}


/**
 * Comparing quantities of the same unit with different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr bool
	operator!= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.base_value() != b.base_value();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator< (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.value() < b.value();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr bool
	operator< (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.base_value() < b.base_value();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator< (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_value() < b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator< (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a < b.base_value();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator> (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.value() > b.value();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr bool
	operator> (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.base_value() > b.base_value();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator> (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_value() > b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator> (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a > b.base_value();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator<= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.value() <= b.value();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr bool
	operator<= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.base_value() <= b.base_value();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator<= (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
				ScalarValue b) noexcept
	{
		return a.base_value() <= b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator<= (ScalarValue a,
				Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a <= b.base_value();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator>= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.value() >= b.value();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr bool
	operator>= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.base_value() >= b.base_value();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator>= (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
				ScalarValue b) noexcept
	{
		return a.base_value() >= b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator>= (ScalarValue a,
				Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a >= b.base_value();
	}


/**
 * Adding quantities of the same unit, same scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator+ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (a.value() + b.value());
	}


/**
 * Adding quantities of the same unit, different scaling and offset = 0.
 * Result type has base SI unit scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr auto
	operator+ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.normalized() + b.normalized();
	}


/**
 * Adding dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr Value
	operator+ (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_value() + b;
	}


/**
 * Adding dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr Value
	operator+ (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a + b.base_value();
	}


/**
 * Subtracting quantities of the same unit, same scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator- (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (a.value() - b.value());
	}


/**
 * Subtracting quantities of the same unit, different scaling and offset = 0.
 * Result type has base SI unit scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal_v<Sa, Sb>>>
	constexpr auto
	operator- (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return a.normalized() - b.normalized();
	}


/**
 * Subtracting dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue,
		 class = std::enable_if_t<!is_quantity_v<ScalarValue>>>
	constexpr Value
	operator- (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_value() - b;
	}


/**
 * Subtracting dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue,
		 class = std::enable_if_t<!is_quantity_v<ScalarValue>>>
	constexpr Value
	operator- (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a - b.base_value();
	}


/**
 * Multiplying quantities of same or different units, same or different scaling and offset = 0.
 */
template<int Ea0, int Ea1, int Ea2, int Ea3, int Ea4, int Ea5, int Ea6, int Ea7, class Sa,
		 int Eb0, int Eb1, int Eb2, int Eb3, int Eb4, int Eb5, int Eb6, int Eb7, class Sb,
		 class Value,
		 class = std::enable_if_t<Ea0 != -Eb0 || Ea1 != -Eb1 || Ea2 != -Eb2 || Ea3 != -Eb3 ||
								  Ea4 != -Eb4 || Ea5 != -Eb5 || Ea6 != -Eb6 || Ea7 != -Eb7>>
	constexpr auto
	operator* (Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		using NewUnit = Unit<Ea0 + Eb0, Ea1 + Eb1, Ea2 + Eb2, Ea3 + Eb3, Ea4 + Eb4, Ea5 + Eb5, Ea6 + Eb6, Ea7 + Eb7, std::ratio_multiply<Sa, Sb>, std::ratio<0>>;
		return Quantity<NewUnit, Value> (a.value() * b.value());
	}


/**
 * Multiplying mutually-inverse quantities.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value>
	constexpr Value
	operator* (Quantity<Unit< E0,  E1,  E2,  E3,  E4,  E5,  E6,  E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<-E0, -E1, -E2, -E3, -E4, -E5, -E6, -E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, std::ratio_multiply<Sa, Sb>, std::ratio<0>>, Value> (a.value() * b.value()).base_value();
	}


/**
 * Dividing quantities of same or different units, same or different scaling and offset = 0.
 */
template<int Ea0, int Ea1, int Ea2, int Ea3, int Ea4, int Ea5, int Ea6, int Ea7, class Sa,
		 int Eb0, int Eb1, int Eb2, int Eb3, int Eb4, int Eb5, int Eb6, int Eb7, class Sb,
		 class Value,
		 class = std::enable_if_t<Ea0 != Eb0 || Ea1 != Eb1 || Ea2 != Eb2 || Ea3 != Eb3 ||
								  Ea4 != Eb4 || Ea5 != Eb5 || Ea6 != Eb6 || Ea7 != Eb7>>
	constexpr auto
	operator/ (Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		using NewUnit = Unit<Ea0 - Eb0, Ea1 - Eb1, Ea2 - Eb2, Ea3 - Eb3, Ea4 - Eb4, Ea5 - Eb5, Ea6 - Eb6, Ea7 - Eb7, std::ratio_divide<Sa, Sb>, std::ratio<0>>;
		return Quantity<NewUnit, Value> (a.value() / b.value());
	}


/**
 * Dividing same-unit quantities.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value>
	constexpr Value
	operator/ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, std::ratio_divide<Sa, Sb>, std::ratio<0>>, Value> (a.value() / b.value()).base_value();
	}


/**
 * Multiplying quantity by scalar.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class ScalarValue,
		 class = std::enable_if_t<std::is_arithmetic_v<ScalarValue>>>
	constexpr auto
	operator* (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q,
			   ScalarValue scalar) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (q.value() * scalar);
	}


/**
 * Multiplying scalar by quantity.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class ScalarValue,
		 class = std::enable_if_t<std::is_arithmetic_v<ScalarValue>>>
	constexpr auto
	operator* (ScalarValue scalar,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		return q * scalar;
	}


/**
 * Dividing quantity by scalar.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class ScalarValue>
	constexpr auto
	operator/ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q,
			   ScalarValue scalar) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (q.value() / scalar);
	}


/**
 * Dividing scalar by quantity.
 * Dividing inverts the scaler.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class ScalarValue,
		 class = std::enable_if_t<std::is_arithmetic_v<ScalarValue>>>
	constexpr auto
	operator/ (ScalarValue scalar,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		return Quantity<Unit<-E0, -E1, -E2, -E3, -E4, -E5, -E6, -E7, std::ratio_divide<std::ratio<1>, S>, std::ratio<0>>, Value> (scalar / q.value());
	}


/**
 * Unary + operator.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O, class Value>
	constexpr auto
	operator+ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> q) noexcept
	{
		return q;
	}


/**
 * Unary - operator.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O, class Value>
	constexpr auto
	operator- (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> q) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> (-q.value());
	}


/**
 * Make quantity from expressions of form: 1.0 * Hertz() -> Quantity<Hertz>()
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator* (Value value, Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (value);
	}


/**
 * Make quantity from expressions of form: Hertz() * 1.0 -> Quantity<Hertz>()
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator* (Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>> unit, Value value) noexcept
	{
		return value * unit;
	}

} // namespace si


namespace std {

template<class pUnit, class pValue>
	class numeric_limits<si::Quantity<pUnit, pValue>>: public numeric_limits<typename si::Quantity<pUnit, pValue>::Value>
	{
		typedef si::Quantity<pUnit, pValue>	Quantity;
		typedef typename Quantity::Value	Value;

	  public:
		static constexpr bool is_specialized		= true;
		static constexpr auto is_signed				= numeric_limits<Value>::is_signed;
		static constexpr auto is_integer			= numeric_limits<Value>::is_integer;
		static constexpr auto is_exact				= numeric_limits<Value>::is_exact;
		static constexpr auto has_infinity			= numeric_limits<Value>::has_infinity;
		static constexpr auto has_quiet_NaN			= numeric_limits<Value>::has_quiet_NaN;
		static constexpr auto has_signaling_NaN		= numeric_limits<Value>::has_signaling_NaN;
		static constexpr auto has_denorm			= numeric_limits<Value>::has_denorm;
		static constexpr auto has_denorm_loss		= numeric_limits<Value>::has_denorm_loss;
		static constexpr auto round_style			= numeric_limits<Value>::round_style;
		static constexpr auto is_iec559				= numeric_limits<Value>::is_iec559;
		static constexpr auto is_bounded			= numeric_limits<Value>::is_bounded;
		static constexpr auto is_modulo				= numeric_limits<Value>::is_modulo;
		static constexpr auto digits				= numeric_limits<Value>::digits;
		static constexpr auto digits10				= numeric_limits<Value>::digits10;
		static constexpr auto max_digits10			= numeric_limits<Value>::max_digits10;
		static constexpr auto radix					= numeric_limits<Value>::radix;
		static constexpr auto min_exponent			= numeric_limits<Value>::min_exponent;
		static constexpr auto min_exponent10		= numeric_limits<Value>::min_exponent10;
		static constexpr auto max_exponent			= numeric_limits<Value>::max_exponent;
		static constexpr auto max_exponent10		= numeric_limits<Value>::max_exponent10;
		static constexpr auto traps					= numeric_limits<Value>::traps;
		static constexpr auto tinyness_before		= numeric_limits<Value>::tinyness_before;

	  public:
		static constexpr Quantity
		min()
		{
			return Quantity (numeric_limits<Value>::min());
		}

		static constexpr Quantity
		lowest()
		{
			return Quantity (numeric_limits<Value>::lowest());
		}

		static constexpr Quantity
		max()
		{
			return Quantity (numeric_limits<Value>::max());
		}

		static constexpr Quantity
		epsilon()
		{
			return Quantity (numeric_limits<Value>::epsilon());
		}

		static constexpr Quantity
		round_error()
		{
			return Quantity (numeric_limits<Value>::round_error());
		}

		static constexpr Quantity
		infinity()
		{
			return Quantity (numeric_limits<Value>::infinity());
		}

		static constexpr Quantity
		quiet_NaN()
		{
			return Quantity (numeric_limits<Value>::quiet_NaN());
		}

		static constexpr Quantity
		signalling_NaN()
		{
			return Quantity (numeric_limits<Value>::signalling_NaN());
		}

		static constexpr Quantity
		denorm_min()
		{
			return Quantity (numeric_limits<Value>::denorm_min());
		}
	};

} // namespace std

#endif

