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

#ifndef SI__UTILS_H__INCLUDED
#define SI__UTILS_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cmath>
#include <experimental/string_view>

// Boost:
#include <boost/endian/conversion.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

// Local:
#include "quantity.h"
#include "standard_units.h"
#include "standard_quantities.h"


namespace si {

/**
 * Returns unchanged argument for non-Quantity types and Quantity::base_quantity()
 * for Quantity types.
 */
template<class Q,
		 class = std::enable_if_t<is_quantity<Q>::value>>
	constexpr typename Q::Value
	base_quantity (Q value) noexcept;


/**
 * Overload of base_quantity() for handling non-Quantity types.
 */
template<class T,
		 class = std::enable_if_t<!is_quantity<T>::value>>
	constexpr T
	base_quantity (T value) noexcept;


/**
 * Return quantity in units U for quantity Q, if Q is a Quantity type.
 */
template<class U, class Q,
		 class = std::enable_if_t<is_quantity<Q>::value>>
	constexpr typename Q::Value
	quantity_in_units (Q value) noexcept;


/**
 * Return the argument (for non-Quantity arguments). Convenience overload.
 */
template<class U, class T,
		 class = std::enable_if_t<!is_quantity<T>::value>>
	constexpr T
	quantity_in_units (T value) noexcept;


/**
 * Return quantity in given units if Q is a Quantity type.
 *
 * \throw	UnsupportedUnit
 *			If unit_str can't be parsed correctly.
 * \throw	IncompatibleTypes
 *			If quantity can't be expressed in given units.
 */
template<class Q,
		 class = std::enable_if_t<is_quantity<Q>::value>>
	constexpr typename Q::Value
	quantity (Q value, std::string const& unit_str);


/**
 * Return value argument if T is non-Quantity type.
 * Overload of quantity() for handling non-Quantity types.
 * The string argument is ignored.
 */
template<class T,
		 class = std::enable_if_t<!is_quantity<T>::value>>
	constexpr T
	quantity (T value, std::string const&) noexcept;


/**
 * Return quantity in given units if Q is a Quantity type.
 *
 * \throw	IncompatibleTypes
 *			If quantity can't be expressed in given units.
 */
template<class Q,
		 class = std::enable_if_t<is_quantity<Q>::value>>
	constexpr typename Q::Value
	quantity (Q value, DynamicUnit const& unit);


/**
 * Return value argument if T is non-Quantity type.
 * Overload of quantity() for handling non-Quantity types.
 * The DynamicUnit argument is ignored.
 */
template<class T,
		 class = std::enable_if_t<!is_quantity<T>::value>>
	constexpr T
	quantity (T value, DynamicUnit const& unit) noexcept;


/**
 * Convert to binary blob representing base unit.
 */
template<class pUnit, class pValue>
	inline std::vector<uint8_t>
	to_blob (Quantity<pUnit, pValue> quantity);


/**
 * std::to_string implementation.
 */
template<class pUnit, class pValue>
	inline std::string
	to_string (Quantity<pUnit, pValue> quantity);


/**
 * std::to_string implementation.
 */
inline std::string
to_string (DynamicRatio const& dr);


/**
 * std::to_string implementation.
 */
inline std::string
to_string (DynamicUnit const& du);


/**
 * Return unit vector for given unit string.
 * Unit string should have form "kg^3 m-5".
 *
 * \throw	UnsupportedUnit
 *			If unit can't be parsed to any known unit.
 */
DynamicUnit
parse_unit (std::experimental::string_view const& str);


/**
 * Convert from binary blob representing base unit.
 *
 * \throw	UnparsableValue
 *			When data can't be correctly parsed.
 */
template<class pUnit, class pValue>
	inline void
	parse (std::vector<uint8_t> const& blob, Quantity<pUnit, pValue>& quantity);


/**
 * Returning version of parse (blob).
 */
template<class Q,
		 class = std::enable_if_t<is_quantity<Q>::value>>
	inline Q
	parse (std::vector<uint8_t> const& blob);


/**
 * Parse string to get quantity.
 * String should look like "1.23 kg m^3 s^-1", that is it should consist of a number
 * and a set of unit names and exponents. Short symbols (returned by UnitTraits<T>::symbol() are allowed.
 *
 * \throw	UnparsableValue
 *			If value can't be parsed correctly.
 * \throw	IncompatibleTypes
 *			If type is not convertible to given quantity argument.
 */
template<class pUnit, class pValue>
	inline void
	parse (std::experimental::string_view const& str, Quantity<pUnit, pValue>& quantity);


/**
 * Returning version of parse (string).
 */
template<class Q,
		 class = std::enable_if_t<is_quantity<Q>::value>>
	inline Q
	parse (std::experimental::string_view const& str);


/**
 * std::ostream support.
 */
template<class pUnit, class pValue>
	inline std::ostream&
	operator<< (std::ostream& out, Quantity<pUnit, pValue> quantity);


/**
 * std::abs() equivalent
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	abs (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept;


/**
 * std::isinf() equivalent.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	isinf (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept;


/**
 * std::isnan() equivalent.
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	isnan (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept;


/**
 * std::sqrt() equivalent
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value,
		 class = std::enable_if_t<E0 % 2 == 0 && E1 % 2 == 0 && E2 % 2 == 0 && E3 % 2 == 0 &&
								  E4 % 2 == 0 && E5 % 2 == 0 && E6 % 2 == 0 && E7 % 2 == 0>>
	constexpr auto
	sqrt (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept;


/**
 * std::isfinite() equivalent
 */
template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	isfinite (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept;


/**
 * Convert value 'source' expressed in 'source_unit's to 'target_unit's.
 */
template<class pValue>
	constexpr pValue
	convert (DynamicUnit const& source_unit, pValue source, DynamicUnit const& target_unit);


/*
 * Implementations
 */


template<class Q, class>
	constexpr typename Q::Value
	base_quantity (Q value) noexcept
	{
		return value.base_quantity();
	}


template<class T, class>
	constexpr T
	base_quantity (T value) noexcept
	{
		return value;
	}


/**
 * Return quantity in units U if Q is a Quantity type.
 */
template<class Q, class U = typename Q::Unit,
		 class = std::enable_if_t<is_quantity<Q>::value>>
	constexpr typename Q::Value
	quantity (Q value) noexcept
	{
		return value.template quantity<U>();
	}


/**
 * Return value argument if T is non-Quantity type.
 * Overload of quantity() for handling non-Quantity types.
 */
template<class T, class Unused = void,
		 class = std::enable_if_t<!is_quantity<T>::value>>
	constexpr T
	quantity (T value) noexcept
	{
		return value;
	}


template<class U, class Q, class>
	constexpr typename Q::Value
	quantity_in_units (Q value) noexcept
	{
		return quantity<Q, U> (value);
	}


template<class U, class T, class>
	constexpr T
	quantity_in_units (T value) noexcept
	{
		return value;
	}


template<class Q, class>
	constexpr typename Q::Value
	quantity (Q value, std::string const& unit_str)
	{
		return quantity (value, parse_unit (unit_str));
	}


template<class T, class>
	constexpr T
	quantity (T value, std::string const&) noexcept
	{
		return value;
	}


template<class Q, class>
	constexpr typename Q::Value
	quantity (Q value, DynamicUnit const& unit)
	{
		return convert (Q::Unit::dynamic_unit(), value.quantity(), unit);
	}


template<class T, class>
	constexpr T
	quantity (T value, DynamicUnit const&) noexcept
	{
		return value;
	}


template<class pUnit, class pValue>
	inline std::vector<uint8_t>
	to_blob (Quantity<pUnit, pValue> quantity)
	{
		std::vector<uint8_t> result (sizeof (pValue));
		pValue copy = quantity.base_quantity();
		boost::endian::native_to_little (copy);
		uint8_t const* begin = reinterpret_cast<uint8_t const*> (&copy);
		uint8_t const* end = begin + sizeof (pValue);
		uint8_t* destination = &*result.begin();
		std::copy (begin, end, destination);
		return result;
	}


template<class pUnit, class pValue>
	inline std::string
	to_string (Quantity<pUnit, pValue> quantity)
	{
		return std::to_string (quantity.quantity()) + " " + UnitTraits<pUnit>::symbol();
	}


inline std::string
to_string (DynamicRatio const& dr)
{
	return std::string ("DynamicRatio<") + std::to_string (dr.numerator()) + " / " + std::to_string (dr.denominator()) + ">";
}


inline std::string
to_string (DynamicUnit const& du)
{
	std::string s = "DynamicUnit<";
	for (int exp: du.exponents())
		s += std::to_string (exp) + ", ";
	s += to_string (du.scale()) + ", " + to_string (du.offset());
	return s + ">";
}


template<class pUnit, class pValue>
	inline void
	parse (std::vector<uint8_t> const& blob, Quantity<pUnit, pValue>& quantity)
	{
		if (blob.size() == sizeof (pValue))
		{
			pValue parsed;
			uint8_t const* begin = blob.data();
			uint8_t const* end = blob.data() + blob.size();
			uint8_t* destination = reinterpret_cast<uint8_t*> (&parsed);
			std::copy (begin, end, destination);
			boost::endian::little_to_native (parsed);
			quantity = Quantity<BaseUnit<pUnit>, pValue> { parsed };
		}
		else
			throw UnparsableValue ("wrong size of binary data");
	}


template<class Q, class>
	inline Q
	parse (std::vector<uint8_t> const& blob)
	{
		Q result;
		parse (blob, result);
		return result;
	}


template<class pUnit, class pValue>
	inline void
	parse (std::experimental::string_view const& str, Quantity<pUnit, pValue>& quantity)
	{
		std::size_t p = str.find_first_of (' ');

		if (p == std::experimental::string_view::npos)
			throw UnparsableValue ("error while parsing: " + str.to_string());

		try {
			// float in "units" -> Quantity
			pValue value = boost::lexical_cast<pValue> (str.substr (0, p));
			std::experimental::string_view unit_str = str.substr (p + 1);

			DynamicUnit source_unit = parse_unit (unit_str);
			DynamicUnit target_unit = pUnit::dynamic_unit();

			quantity = Quantity<pUnit, pValue> (convert (source_unit, value, target_unit));
		}
		catch (IncompatibleTypes&)
		{
			throw;
		}
		catch (...)
		{
			throw UnparsableValue ("error while parsing: " + str.to_string());
		}
	}


template<class Q, class>
	inline Q
	parse (std::experimental::string_view const& str)
	{
		Q result;
		parse (str, result);
		return result;
	}


template<class pUnit, class pValue>
	inline std::ostream&
	operator<< (std::ostream& out, Quantity<pUnit, pValue> quantity)
	{
		return out << quantity.quantity() << " " << UnitTraits<pUnit>::symbol();
	}


template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	abs (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		return Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> (q.quantity());
	}


template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	isinf (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		return std::isinf (q.quantity());
	}


template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr auto
	isnan (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		return std::isnan (q.quantity());
	}


template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value, class>
	constexpr auto
	sqrt (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		typedef Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7>, Value> NormalizedQuantity;

		auto value = std::sqrt (NormalizedQuantity (q).base_quantity());
		return Quantity<Unit<E0 / 2, E1 / 2, E2 / 2, E3 / 2, E4 / 2, E5 / 2, E6 / 2, E7 / 2, S, std::ratio<0>>, Value> (value);
	}


template<int E0, int E1, int E2, int E3, int E4, int E5, int E6, int E7, class S, class Value>
	constexpr bool
	isfinite (Quantity<Unit<E0, E1, E2, E3, E4, E5, E6, E7, S, std::ratio<0>>, Value> q) noexcept
	{
		return std::isfinite (q.quantity());
	}


template<class pValue>
	constexpr pValue
	convert (DynamicUnit const& source_unit, pValue source, DynamicUnit const& target_unit)
	{
		// Assert that units are convertible (exponents vector the same):
		if (source_unit.exponents() != target_unit.exponents())
			throw IncompatibleTypes (source_unit, target_unit);

		pValue base_value = source * source_unit.scale().to_floating_point() + source_unit.offset().to_floating_point();
		pValue result = (base_value - target_unit.offset().to_floating_point()) / target_unit.scale().to_floating_point();
		return result;
	}


template<class T>
	constexpr bool
	isfinite (T q) noexcept
	{
		return std::isfinite (q);
	}


constexpr quantities::Angle::Value
sin (quantities::Angle a)
{
	return std::sin (a.quantity<units::Radian>());
}


constexpr quantities::Angle::Value
cos (quantities::Angle a)
{
	return std::sin (a.quantity<units::Radian>());
}


constexpr quantities::Angle::Value
tan (quantities::Angle a)
{
	return std::sin (a.quantity<units::Radian>());
}


template<class Value>
	constexpr quantities::Angle
	atan2 (Value y, Value x)
	{
		return units::Radian() * std::atan2 (base_quantity (y), base_quantity (x));
	}

} // namespace si

#endif

