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

#ifndef NEUTRINO__SI__UNIT_TRAITS_H__INCLUDED
#define NEUTRINO__SI__UNIT_TRAITS_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Local:
#include "exception.h"
#include "unit.h"


namespace si {

/**
 * The main unit traits class.
 */
template<class pUnit>
	class UnitTraits
	{
	  public:
		/**
		 * Return full name of the unit.
		 */
		static std::string
		name();

		/**
		 * Return short symbol.
		 */
		static std::string
		symbol();

		/**
		 * Return alternative symbols.
		 */
		static std::vector<std::string>
		alternative_symbols();
	};


/**
 * Default implementations of methods.
 */
class DefaultUnitTraits
{
  public:
	static std::vector<std::string>
	alternative_symbols()
	{
		return { };
	}
};


template<class U>
	inline std::string
	UnitTraits<U>::name()
	{
		return "unnamed";
	}


template<class U>
	inline std::string
	UnitTraits<U>::symbol()
	{
		return U::dynamic_unit().symbol();
	}


template<class U>
	inline std::vector<std::string>
	UnitTraits<U>::alternative_symbols()
	{
		return { };
	}

} // namespace si

#endif

