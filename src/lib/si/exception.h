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

#ifndef SI__EXCEPTION_H__INCLUDED
#define SI__EXCEPTION_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/exception.h>


namespace si {

class DynamicUnit;


class UnparsableValue: public xf::Exception
{
  public:
	UnparsableValue (std::string const& message):
		Exception (message)
	{ }
};


class UnsupportedUnit: public xf::Exception
{
  public:
	UnsupportedUnit (std::string const& message):
		Exception (message)
	{ }
};


class IncompatibleTypes: public xf::Exception
{
  public:
	IncompatibleTypes (DynamicUnit const& got, DynamicUnit const& expected);
};

} // namespace si

#endif

