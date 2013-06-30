/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef SI__VALUE_H__INCLUDED
#define SI__VALUE_H__INCLUDED

// Standard:
#include <cstddef>


namespace SI {

class Value
{
  public:
	/**
	 * Parse value from string (eg. 1.0 kt).
	 */
	virtual void
	parse (std::string const&) = 0;

	/**
	 * Output string with value and unit.
	 */
	virtual std::string
	stringify() const = 0;

	/**
	 * Return float value in given units.
	 */
	virtual double
	floatize (std::string unit) const = 0;
};

} // namespace std

#endif

