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

#ifndef NEUTRINO__SI__UNIT_H__INCLUDED
#define NEUTRINO__SI__UNIT_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <ratio>
#include <array>
#include <tuple>

// Local:
#include "exception.h"


namespace si {

class DynamicUnit;


/**
 * Number of dimensions (distinct physical quantities) in our SI space.
 */
static constexpr std::size_t kUnitDimensions = 8;


/**
 * Convert std::ratio to desired floating-point number.
 */
template<class Ratio, class Value>
	constexpr Value
	to_floating_point() noexcept
	{
		return static_cast<Value> (Ratio::num) / static_cast<Value> (Ratio::den);
	}


/**
 * Every SI unit can be represented as a vector in a space of base dimensions.
 * Those dimensions are length, mass, time, electrical current, temperature, amount and luminous
 * intensity. Here, also angle is added, for better type-safety. The value of vector in each
 * dimension represents exponent of the unit. If vector has value 2 in length's dimension, it represents
 * meters squared (area). Value -1 for seconds represents 1 / seconds = Hertz (frequency).
 *
 * Example:
 *   Length value = 1 (where unit is meter), time value = -1 (where unit is second).
 *   Result = meter^1 * second^(-1) = meters / seconds. The unit of velocity.
 *
 * Representation of a meter unit would be done like this:
 *
 *   typedef Unit<1, 0, 0, 0, 0, 0, 0, 0> Meter;
 *
 * Representation of acceleration unit (meter / second^2):
 *
 *   typedef Unit<1, 0, -2, 0, 0, 0, 0 0> MeterPerSecondSquared;
 *
 * Additionally, Unit has Scale and Offset factors. Units with different factors are not directly
 * compatible, but when dealing with unit quantities (see Quantity), they can be automatically
 * converted.
 *
 *   typedef Unit<1, 0, 0, 0, 0, 0, 0, 0> Meter;
 *   typedef Unit<1, 0, 0, 0, 0, 0, 0, 0, std::ratio<1000>> Kilometer;
 *
 * or shorter, based on already existing typedef Meter:
 *
 *   typedef ScaledUnit<Meter, std::ratio<1000>> Kilometer;
 *
 * This class works like a type-tag that represents a unit. For representing quantitive values
 * another class is used - Quantity<Unit>, which additionally stores actual value.
 */
template<
	int pLengthExponent,
	int pMassExponent,
	int pTimeExponent,
	int pCurrentExponent,
	int pTemperatureExponent,
	int pAmountExponent,
	int pLuminousIntensityExponent,
	int pAngleExponent,
	class pScaleRatio = std::ratio<1>,
	class pOffsetRatio = std::ratio<0>
>
	class Unit
	{
	  public:
		static constexpr int LengthExponent				= pLengthExponent;
		static constexpr int MassExponent				= pMassExponent;
		static constexpr int TimeExponent				= pTimeExponent;
		static constexpr int CurrentExponent			= pCurrentExponent;
		static constexpr int TemperatureExponent		= pTemperatureExponent;
		static constexpr int AmountExponent				= pAmountExponent;
		static constexpr int LuminousIntensityExponent	= pLuminousIntensityExponent;
		// This is not a basic SI dimension, but it's useful to have:
		static constexpr int AngleExponent				= pAngleExponent;

		// Same constants, with shorter names:
		static constexpr int E0	= pLengthExponent;
		static constexpr int E1	= pMassExponent;
		static constexpr int E2	= pTimeExponent;
		static constexpr int E3	= pCurrentExponent;
		static constexpr int E4	= pTemperatureExponent;
		static constexpr int E5	= pAmountExponent;
		static constexpr int E6	= pLuminousIntensityExponent;
		static constexpr int E7	= pAngleExponent;

		// Unit scaling and offset meta-values:
		typedef pScaleRatio		Scale;
		typedef pOffsetRatio	Offset;

	  public:
		/**
		 * Return true if all exponents are 0, ie unit is dimensionless.
		 */
		static constexpr bool
		is_dimensionless();

		/**
		 * Get an instance of DynamicUnit that matches this Unit type.
		 */
		static constexpr DynamicUnit
		dynamic_unit();

		/**
		 * Convert value from this unit to unit with scale=1 and offset=0.
		 */
		template<class Value,
				 class = std::enable_if_t<std::is_arithmetic_v<Value>>>
			static constexpr Value
			base_value (Value value) noexcept
			{
				return value * to_floating_point<Scale, Value>() + to_floating_point<Offset, Value>();
			}
	};


/**
 * Shorthand for simple definition of scaled and offset unit, based on already existing one.
 *
 * Example:
 *
 *   typedef ScaledUnit<Meter, std::ratio<1000>> Kilometer;
 *   // It should be understood as: Meter * 1000 = Kilometer.
 *
 * Example:
 *
 *   // Define degrees Fahrenheit based on Kelvins:
 *   // [K] = [°F] * 5/9 + 229835/900
 *   typedef ScaledUnit<K, std::ratio<5, 9>, std::ratio<229835, 900>> F;
 *
 *   // Define degrees Fahrenheit based on degrees Celsius:
 *   // [C] = [°F] * 5/9 - 32 * 5/9
 *   typedef ScaledUnit<C, std::ratio<5, 9>, std::ratio<-32 * 5, 9>> F;
 */
template<class pExistingUnit, class pScale, class pOffset = std::ratio<0>>
	using ScaledUnit =
		Unit<
			pExistingUnit::E0,
			pExistingUnit::E1,
			pExistingUnit::E2,
			pExistingUnit::E3,
			pExistingUnit::E4,
			pExistingUnit::E5,
			pExistingUnit::E6,
			pExistingUnit::E7,
			std::ratio_multiply<typename pExistingUnit::Scale, pScale>,
			std::ratio_add<std::ratio_multiply<pOffset, typename pExistingUnit::Scale>, typename pExistingUnit::Offset>
		>;


/**
 * Shorthand for getting base-version of any unit (where scale = 1 and offset = 0).
 */
template<class pExistingUnit>
	using NormalizedUnit =
		Unit<
			pExistingUnit::E0,
			pExistingUnit::E1,
			pExistingUnit::E2,
			pExistingUnit::E3,
			pExistingUnit::E4,
			pExistingUnit::E5,
			pExistingUnit::E6,
			pExistingUnit::E7,
			std::ratio<1>,
			std::ratio<0>
		>;


/**
 * Dynamic ratio for use in DynamicUnit.
 */
class DynamicRatio
{
  public:
	// Ctor
	constexpr DynamicRatio (intmax_t numerator, intmax_t denominator);

	/**
	 * Equality test. Doesn't find GCD.
	 */
	constexpr bool
	operator== (DynamicRatio const&) const;

	/**
	 * Inequality test. Doesn't find GCD.
	 */
	constexpr bool
	operator!= (DynamicRatio const&) const;

	/**
	 * Ordering operator.
	 */
	constexpr bool
	operator< (DynamicRatio const&) const;

	/**
	 * Multiplication.
	 */
	constexpr DynamicRatio&
	operator*= (DynamicRatio const&);

	/**
	 * Division.
	 */
	constexpr DynamicRatio&
	operator/= (DynamicRatio const&);

	/**
	 * Return inverted ratio. n/m becomes m/n.
	 */
	constexpr DynamicRatio
	inverted() const;

	/**
	 * Return numerator.
	 */
	constexpr intmax_t
	numerator() const noexcept;

	/**
	 * Return denominator.
	 */
	constexpr intmax_t
	denominator() const noexcept;

	/**
	 * Return floating-point value for this ratio.
	 */
	constexpr long double
	to_floating_point() const;

  private:
	intmax_t	_numerator;
	intmax_t	_denominator;
};


/**
 * Dynamic unit that contains SI-space-vector, scale and offset factors
 * but as members, not type traits. Utility class.
 */
class DynamicUnit
{
	static constexpr const char* kDotProductSymbol_utf8 = "⋅";

  public:
	// Ctor
	constexpr
	DynamicUnit();

	// Ctor
	constexpr
	DynamicUnit (int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7,
				 DynamicRatio scale = { 1, 1 }, DynamicRatio offset = { 0, 1 });

	// Ctor
	constexpr
	DynamicUnit (DynamicUnit const&) = default;

	/**
	 * Copy operator.
	 */
	constexpr DynamicUnit&
	operator= (DynamicUnit const&);

	/**
	 * Equality test.
	 */
	bool
	operator== (DynamicUnit const&) const;

	/**
	 * Inequality test.
	 */
	bool
	operator!= (DynamicUnit const&) const;

	/**
	 * Ordering operator.
	 */
	bool
	operator< (DynamicUnit const&) const;

#define SI_DYNAMIC_UNIT_ACCESSOR(name, index) \
	int&					name() noexcept				{ return _exponents[index]; } \
	constexpr int const&	name() const noexcept		{ return _exponents[index]; } \
	int&					e##index() noexcept			{ return _exponents[index]; } \
	constexpr int const&	e##index() const noexcept	{ return _exponents[index]; } \

	SI_DYNAMIC_UNIT_ACCESSOR (length_exponent, 0)
	SI_DYNAMIC_UNIT_ACCESSOR (mass_exponent, 1)
	SI_DYNAMIC_UNIT_ACCESSOR (time_exponent, 2)
	SI_DYNAMIC_UNIT_ACCESSOR (current_exponent, 3)
	SI_DYNAMIC_UNIT_ACCESSOR (temperature_exponent, 4)
	SI_DYNAMIC_UNIT_ACCESSOR (amount_exponent, 5)
	SI_DYNAMIC_UNIT_ACCESSOR (luminous_intensity_exponent, 6)
	SI_DYNAMIC_UNIT_ACCESSOR (angle_exponent, 7)

#undef SI_DYNAMIC_UNIT_ACCESSOR

	constexpr std::array<int, kUnitDimensions>&
	exponents() noexcept;

	constexpr std::array<int, kUnitDimensions> const&
	exponents() const noexcept;

	constexpr DynamicRatio&
	scale() noexcept;

	constexpr DynamicRatio const&
	scale() const noexcept;

	constexpr DynamicRatio&
	offset() noexcept;

	constexpr DynamicRatio const&
	offset() const noexcept;

	/**
	 * Return symbol for this unit.
	 * Equivalent to UnitTraits<Unit>::symbol().
	 */
	std::string
	symbol() const;

  private:
	static void
	add_single_unit_symbol (std::string& result, int exponent, const char* symbol);

  private:
	std::array<int, kUnitDimensions>	_exponents;
	DynamicRatio						_scale;
	DynamicRatio						_offset;
};


/*
 * Unit methods
 */


template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O>
	constexpr bool
	Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>::is_dimensionless()
	{
		return E0 == 0 && E1 == 0 && E2 == 0 && E3 == 0
			&& E4 == 0 && E5 == 0 && E6 == 0 && E7 == 0;
	}


template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class O>
	constexpr DynamicUnit
	Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, O>::dynamic_unit()
	{
		return DynamicUnit (E0, E1, E2, E3, E4, E5, E6, E7, DynamicRatio (S::num, S::den), DynamicRatio (O::num, O::den));
	}


/*
 * DynamicRatio methods
 */


constexpr
DynamicRatio::DynamicRatio (intmax_t numerator, intmax_t denominator):
	_numerator (numerator),
	_denominator (denominator)
{ }


constexpr bool
DynamicRatio::operator== (DynamicRatio const& other) const
{
	return _numerator == other._numerator
		&& _denominator == other._denominator;
}


constexpr bool
DynamicRatio::operator!= (DynamicRatio const& other) const
{
	return !(*this == other);
}


constexpr bool
DynamicRatio::operator< (DynamicRatio const& other) const
{
	return to_floating_point() < other.to_floating_point();
}


constexpr DynamicRatio&
DynamicRatio::operator*= (DynamicRatio const& other)
{
	_numerator *= other._numerator;
	_denominator *= other._denominator;
	return *this;
}


constexpr DynamicRatio&
DynamicRatio::operator/= (DynamicRatio const& other)
{
	_numerator *= other._denominator;
	_denominator *= other._numerator;
	return *this;
}


constexpr DynamicRatio
DynamicRatio::inverted() const
{
	return DynamicRatio (_denominator, _numerator);
}


constexpr intmax_t
DynamicRatio::numerator() const noexcept
{
	return _numerator;
}


constexpr intmax_t
DynamicRatio::denominator() const noexcept
{
	return _denominator;
}


constexpr long double
DynamicRatio::to_floating_point() const
{
	return static_cast<long double> (_numerator) / static_cast<long double> (_denominator);
}


/*
 * DynamicUnit methods
 */


constexpr
DynamicUnit::DynamicUnit():
	_exponents ({ 0, 0, 0, 0, 0, 0, 0, 0 }),
	_scale (1, 1),
	_offset (0, 1)
{ }


constexpr
DynamicUnit::DynamicUnit (int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, DynamicRatio scale, DynamicRatio offset):
	_exponents ({ E0, E1, E2, E3, E4, E5, E6, E7 }),
	_scale (scale),
	_offset (offset)
{ }


constexpr DynamicUnit&
DynamicUnit::operator= (DynamicUnit const& other)
{
	_exponents = other._exponents;
	_scale = other._scale;
	_offset = other._offset;
	return *this;
}


inline bool
DynamicUnit::operator== (DynamicUnit const& other) const
{
	return _exponents == other._exponents
		&& _scale == other._scale
		&& _offset == other._offset;
}


inline bool
DynamicUnit::operator!= (DynamicUnit const& other) const
{
	return !(*this == other);
}


inline bool
DynamicUnit::operator< (DynamicUnit const& other) const
{
	return std::tie (_exponents, _scale, _offset) < std::tie (other._exponents, other._scale, other._offset);
}


constexpr std::array<int, kUnitDimensions>&
DynamicUnit::exponents() noexcept
{
	return _exponents;
}


constexpr std::array<int, kUnitDimensions> const&
DynamicUnit::exponents() const noexcept
{
	return _exponents;
}


constexpr DynamicRatio&
DynamicUnit::scale() noexcept
{
	return _scale;
}


constexpr DynamicRatio const&
DynamicUnit::scale() const noexcept
{
	return _scale;
}


constexpr DynamicRatio&
DynamicUnit::offset() noexcept
{
	return _offset;
}


constexpr DynamicRatio const&
DynamicUnit::offset() const noexcept
{
	return _offset;
}


/*
 * Global functions
 */


constexpr DynamicRatio
operator* (int x, DynamicRatio const& ratio)
{
	return DynamicRatio (x * ratio.numerator(), ratio.denominator());
}


constexpr DynamicRatio
operator* (DynamicRatio const& a, DynamicRatio const& b)
{
	return DynamicRatio (a.numerator() * b.numerator(), a.denominator() * b.denominator());
}


constexpr DynamicRatio
operator/ (int x, DynamicRatio const& ratio)
{
	return x * ratio.inverted();
}


constexpr DynamicRatio
operator/ (DynamicRatio const& ratio, int x)
{
	return ratio * DynamicRatio (1, x);
}


constexpr DynamicRatio
operator/ (DynamicRatio const& a, DynamicRatio const& b)
{
	return DynamicRatio (a.numerator() * b.denominator(), a.denominator() * b.numerator());
}


/**
 * Return true if SourceUnit is convertible to TargetUnit (exponent vector is
 * the same, only scaling and/or offset differs.
 */
template<class SourceUnit, class TargetUnit>
	constexpr bool
	is_convertible()
	{
		return SourceUnit::E0 == TargetUnit::E0
			&& SourceUnit::E1 == TargetUnit::E1
			&& SourceUnit::E2 == TargetUnit::E2
			&& SourceUnit::E3 == TargetUnit::E3
			&& SourceUnit::E4 == TargetUnit::E4
			&& SourceUnit::E5 == TargetUnit::E5
			&& SourceUnit::E6 == TargetUnit::E6
			&& SourceUnit::E7 == TargetUnit::E7;
	}


/**
 * Return true if SourceUnit is convertible to TargetUnit (exponent vector is
 * the same except for E7 aka AngleExponent).
 */
template<class SourceUnit, class TargetUnit>
	constexpr bool
	is_convertible_with_angle()
	{
		return SourceUnit::E0 == TargetUnit::E0
			&& SourceUnit::E1 == TargetUnit::E1
			&& SourceUnit::E2 == TargetUnit::E2
			&& SourceUnit::E3 == TargetUnit::E3
			&& SourceUnit::E4 == TargetUnit::E4
			&& SourceUnit::E5 == TargetUnit::E5
			&& SourceUnit::E6 == TargetUnit::E6;
	}


/**
 * Return true if source_unit is convertible to target_unit (exponent vector is
 * the same, only scaling and/or offset differs.
 */
constexpr bool
is_convertible (DynamicUnit const& source_unit, DynamicUnit const& target_unit)
{
	return source_unit.e0() == target_unit.e0()
		&& source_unit.e1() == target_unit.e1()
		&& source_unit.e2() == target_unit.e2()
		&& source_unit.e3() == target_unit.e3()
		&& source_unit.e4() == target_unit.e4()
		&& source_unit.e5() == target_unit.e5()
		&& source_unit.e6() == target_unit.e6()
		&& source_unit.e7() == target_unit.e7();
}


/**
 * Return true if source_unit is convertible to target_unit (exponent vector is
 * the same, only scaling and/or offset differs.
 */
constexpr bool
is_convertible_with_angle (DynamicUnit const& source_unit, DynamicUnit const& target_unit)
{
	return source_unit.e0() == target_unit.e0()
		&& source_unit.e1() == target_unit.e1()
		&& source_unit.e2() == target_unit.e2()
		&& source_unit.e3() == target_unit.e3()
		&& source_unit.e4() == target_unit.e4()
		&& source_unit.e5() == target_unit.e5()
		&& source_unit.e6() == target_unit.e6();
}

} // namespace si

#endif

