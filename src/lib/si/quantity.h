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

#ifndef SI__QUANTITY_H__INCLUDED
#define SI__QUANTITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>
#include <ratio>

// Local:
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
		static_assert (std::is_floating_point<pValue>::value, "pValue must be floating-point");

	  public:
		typedef pUnit	Unit;
		typedef pValue	Value;

	  public:
		// Ctor
		explicit constexpr
		Quantity (Value quantity = 0) noexcept:
			_quantity (quantity)
		{ }

		/**
		 * Return value of the quantity measured in Units.
		 */
		constexpr Value
		quantity() const noexcept
		{
			return _quantity;
		}

		/**
		 * Return value of the quantity measured in base units (where Scale is 1).
		 */
		constexpr Value
		base_quantity() const noexcept
		{
			return _quantity * to_fp<typename Unit::Scale>() + to_fp<typename Unit::Offset>();
		}

		/**
		 * Return quantity expressed in another unit.
		 */
		template<class OtherUnit>
			constexpr Value
			quantity() const noexcept
			{
				return Quantity<OtherUnit> (*this).quantity();
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
			_quantity *= scalar;
			return *this;
		}

		Quantity&
		operator/= (Value scalar) noexcept
		{
			_quantity /= scalar;
			return *this;
		}

		/**
		 * Convert to Unit with the same exponents vector, but different scaling value.
		 */
		template<class OtherScale, class OtherOffset>
			constexpr
			operator Quantity<si::Unit<pUnit::E0, Unit::E1, Unit::E2, Unit::E3, Unit::E4, Unit::E5, Unit::E6, Unit::E7, OtherScale, OtherOffset>, Value>() const noexcept
			{
				auto result = (base_quantity() - to_fp<OtherOffset>()) / to_fp<OtherScale>();

				return Quantity<si::Unit<Unit::E0, Unit::E1, Unit::E2, Unit::E3, Unit::E4, Unit::E5, Unit::E6, Unit::E7, OtherScale, OtherOffset>, Value> (result);
			}

	  private:
		template<class Ratio>
			static constexpr Value
			to_fp() noexcept
			{
				return static_cast<Value> (Ratio::num) / static_cast<Value> (Ratio::den);
			}

	  private:
		Value _quantity;
	};


/**
 * Meta-function returning true if parameter is a Quantity type.
 */
template<class T>
	struct is_quantity
	{
		static constexpr bool value = std::is_base_of<QuantityBase, T>::value;
	};


/**
 * Comparing quantity of the same unit, same scaling, same offset.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O, class Value>
	constexpr bool
	operator== (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> b) noexcept
	{
		return a.quantity() == b.quantity();
	}


/**
 * Comparing quantities of the same unit with different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr bool
	operator== (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (a).quantity() == NormalizedQuantity (b).quantity();
	}


/**
 * Comparing quantity of the same unit, same scaling, same offset.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O, class Value>
	constexpr bool
	operator!= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> b) noexcept
	{
		return a.quantity() != b.quantity();
	}


/**
 * Comparing quantities of the same unit with different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr bool
	operator!= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (a).quantity() != NormalizedQuantity (b).quantity();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator< (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.quantity() < b.quantity();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr bool
	operator< (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (a).quantity() < NormalizedQuantity (b).quantity();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator< (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_quantity() < b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator< (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a < b.base_quantity();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator> (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.quantity() > b.quantity();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr bool
	operator> (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (a).quantity() > NormalizedQuantity (b).quantity();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator> (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_quantity() > b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator> (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a > b.base_quantity();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator<= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.quantity() <= b.quantity();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr bool
	operator<= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (a).quantity() <= NormalizedQuantity (b).quantity();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator<= (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
				ScalarValue b) noexcept
	{
		return a.base_quantity() <= b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator<= (ScalarValue a,
				Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a <= b.base_quantity();
	}


/**
 * Comparing quantities of the same unit, same scaling.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	operator>= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return a.quantity() >= b.quantity();
	}


/**
 * Comparing quantities of the same unit, different scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr bool
	operator>= (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
				Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (a).quantity() >= NormalizedQuantity (b).quantity();
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator>= (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
				ScalarValue b) noexcept
	{
		return a.base_quantity() >= b;
	}


/**
 * Comparing dimensionless quantity with a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr bool
	operator>= (ScalarValue a,
				Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a >= b.base_quantity();
	}


/**
 * Adding quantities of the same unit, same scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator+ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (a.quantity() + b.quantity());
	}


/**
 * Adding quantities of the same unit, different scaling and offset = 0.
 * Result type has unit scaling of the first operand.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr
	Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value>
	operator+ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (NormalizedQuantity (a).quantity() + NormalizedQuantity (b).quantity());
	}


/**
 * Adding dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr Value
	operator+ (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_quantity() + b;
	}


/**
 * Adding dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue>
	constexpr Value
	operator+ (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a + b.base_quantity();
	}


/**
 * Subtracting quantities of the same unit, same scaling and offset = 0.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator- (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (a.quantity() - b.quantity());
	}


/**
 * Subtracting quantities of the same unit, different scaling and offset = 0.
 * Result type has unit scaling of the first operand.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>>
	constexpr
	Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value>
	operator- (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		return NormalizedQuantity (NormalizedQuantity (a).quantity() - NormalizedQuantity (b).quantity());
	}


/**
 * Subtracting dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue,
		 class = std::enable_if_t<!is_quantity<ScalarValue>::value>>
	constexpr Value
	operator- (Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> a,
			   ScalarValue b) noexcept
	{
		return a.base_quantity() - b;
	}


/**
 * Subtracting dimensionless quantity to a bare scalar.
 */
template<class Scale, class Value, class ScalarValue,
		 class = std::enable_if_t<!is_quantity<ScalarValue>::value>>
	constexpr Value
	operator- (ScalarValue a,
			   Quantity<Unit<0, 0, 0, 0, 0, 0, 0, 0, Scale, std::ratio<0>>, Value> b) noexcept
	{
		return a - b.base_quantity();
	}


/**
 * Multiplying quantities of same or different units, same scaling and offset = 0.
 * Result type has unit scaling of the first operand.
 */
template<int Ea0, int Ea1, int Ea2, int Ea3, int Ea4, int Ea5, int Ea6, int Ea7,
		 int Eb0, int Eb1, int Eb2, int Eb3, int Eb4, int Eb5, int Eb6, int Eb7,
		 class S, class Value,
		 class = std::enable_if_t<Ea0 != -Eb0 || Ea1 != -Eb1 || Ea2 != -Eb2 || Ea3 != -Eb3 ||
								  Ea4 != -Eb4 || Ea5 != -Eb5 || Ea6 != -Eb6 || Ea7 != -Eb7>>
	constexpr auto
	operator* (Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7, S, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<Ea0 + Eb0, Ea1 + Eb1, Ea2 + Eb2, Ea3 + Eb3, Ea4 + Eb4, Ea5 + Eb5, Ea6 + Eb6, Ea7 + Eb7, S, std::ratio<0>>, Value> (a.quantity() * b.quantity());
	}


/**
 * Multiplying quantities of same or different units, different scaling and offset = 0.
 * Result type has unit scaling of the first operand.
 */
template<int Ea0, int Ea1, int Ea2, int Ea3, int Ea4, int Ea5, int Ea6, int Ea7, class Sa,
		 int Eb0, int Eb1, int Eb2, int Eb3, int Eb4, int Eb5, int Eb6, int Eb7, class Sb,
		 class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>,
		 class = std::enable_if_t<Ea0 != -Eb0 || Ea1 != -Eb1 || Ea2 != -Eb2 || Ea3 != -Eb3 ||
								  Ea4 != -Eb4 || Ea5 != -Eb5 || Ea6 != -Eb6 || Ea7 != -Eb7>>
	constexpr auto
	operator* (Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7>, Value> NormalizedQuantity1;
		typedef Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7>, Value> NormalizedQuantity2;

		auto quantity = NormalizedQuantity1 (a).quantity() * NormalizedQuantity2 (b).quantity();
		return Quantity<Unit<Ea0 + Eb0, Ea1 + Eb1, Ea2 + Eb2, Ea3 + Eb3, Ea4 + Eb4, Ea5 + Eb5, Ea6 + Eb6, Ea7 + Eb7, Sa, std::ratio<0>>, Value> (quantity);
	}


/**
 * Multiplying mutually-inverse quantities.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value>
	constexpr Value
	operator* (Quantity<Unit< E0,  E1,  E2,  E3,  E4,  E5,  E6,  E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<-E0, -E1, -E2, -E3, -E4, -E5, -E6, -E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<+E0, +E1, +E2, +E3, +E4, +E5, +E6, +E7>, Value> NormalizedQuantity1;
		typedef Quantity<Unit<-E0, -E1, -E2, -E3, -E4, -E5, -E6, -E7>, Value> NormalizedQuantity2;

		return NormalizedQuantity1 (a).quantity() * NormalizedQuantity2 (b).quantity();
	}


/**
 * Dividing quantities of same or different units, same scaling and offset = 0.
 * Result type has unit scaling of the first operand.
 */
template<int Ea0, int Ea1, int Ea2, int Ea3, int Ea4, int Ea5, int Ea6, int Ea7,
		 int Eb0, int Eb1, int Eb2, int Eb3, int Eb4, int Eb5, int Eb6, int Eb7,
		 class S, class Value,
		 class = std::enable_if_t<Ea0 != Eb0 || Ea1 != Eb1 || Ea2 != Eb2 || Ea3 != Eb3 ||
								  Ea4 != Eb4 || Ea5 != Eb5 || Ea6 != Eb6 || Ea7 != Eb7>>
	constexpr auto
	operator/ (Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7, S, std::ratio<0>>, Value> a,
			   Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7, S, std::ratio<0>>, Value> b) noexcept
	{
		return Quantity<Unit<Ea0 - Eb0, Ea1 - Eb1, Ea2 - Eb2, Ea3 - Eb3, Ea4 - Eb4, Ea5 - Eb5, Ea6 - Eb6, Ea7 - Eb7, S, std::ratio<0>>, Value> (a.quantity() / b.quantity());
	}


/**
 * Dividing quantities of same or different units, different scaling and offset = 0.
 * Result type has unit scaling of the first operand.
 */
template<int Ea0, int Ea1, int Ea2, int Ea3, int Ea4, int Ea5, int Ea6, int Ea7, class Sa,
		 int Eb0, int Eb1, int Eb2, int Eb3, int Eb4, int Eb5, int Eb6, int Eb7, class Sb,
		 class Value,
		 class = std::enable_if_t<std::ratio_not_equal<Sa, Sb>::value>,
		 class = std::enable_if_t<Ea0 != Eb0 || Ea1 != Eb1 || Ea2 != Eb2 || Ea3 != Eb3 ||
								  Ea4 != Eb4 || Ea5 != Eb5 || Ea6 != Eb6 || Ea7 != Eb7>>
	constexpr auto
	operator/ (Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<Ea0, Ea1, Ea2, Ea3, Ea4, Ea5, Ea6, Ea7>, Value> NormalizedQuantity1;
		typedef Quantity<Unit<Eb0, Eb1, Eb2, Eb3, Eb4, Eb5, Eb6, Eb7>, Value> NormalizedQuantity2;

		auto quantity = NormalizedQuantity1 (a).quantity() / NormalizedQuantity2 (b).quantity();
		return Quantity<Unit<Ea0 - Eb0, Ea1 - Eb1, Ea2 - Eb2, Ea3 - Eb3, Ea4 - Eb4, Ea5 - Eb5, Ea6 - Eb6, Ea7 - Eb7, Sa, std::ratio<0>>, Value> (quantity);
	}


/**
 * Dividing same-unit quantities.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class Sa, class Sb, class Value>
	constexpr Value
	operator/ (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sa, std::ratio<0>>, Value> a,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, Sb, std::ratio<0>>, Value> b) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity1;
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity2;

		return NormalizedQuantity1 (a).quantity() / NormalizedQuantity2 (b).quantity();
	}


/**
 * Multiplying quantity by scalar.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class ScalarValue>
	constexpr auto
	operator* (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q,
			   ScalarValue scalar) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (q.quantity() * scalar);
	}


/**
 * Multiplying scalar by quantity.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class ScalarValue>
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
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (q.quantity() / scalar);
	}


/**
 * Dividing scalar by quantity.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class ScalarValue>
	constexpr auto
	operator/ (ScalarValue scalar,
			   Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		return Quantity<Unit<-E0, -E1, -E2, -E3, -E4, -E5, -E6, -E7, S, std::ratio<0>>, Value> (scalar / q.quantity());
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
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>, Value> (-q.quantity());
	}


/**
 * Make quantity from expressions of form: 1.0 * Hertz() -> Quantity<Hertz>()
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator* (Value quantity, Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (quantity);
	}


/**
 * Make quantity from expressions of form: Hertz() * 1.0 -> Quantity<Hertz>()
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	operator* (Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>> unit, Value quantity) noexcept
	{
		return quantity * unit;
	}

} // namespace si


namespace std {

template<class pUnit, class pValue>
	class numeric_limits<si::Quantity<pUnit, pValue>>: public numeric_limits<typename si::Quantity<pUnit, pValue>::Value>
	{ };

} // namespace std

#endif

