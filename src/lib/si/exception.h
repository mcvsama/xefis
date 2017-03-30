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

// Local:
#include "si_config.h"


namespace si {

class DynamicUnit;

using Exception = si_config::Exception;


class UnparsableValue: public Exception
{
  public:
	explicit
	UnparsableValue (std::string const& message);
};


class UnsupportedUnit: public Exception
{
  public:
	explicit
	UnsupportedUnit (std::string const& message);
};


class IncompatibleTypes: public Exception
{
  public:
	explicit
	IncompatibleTypes (DynamicUnit const& got, DynamicUnit const& expected);
};

} // namespace si

#endif

