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

#ifndef NEUTRINO__TEST__STDEXCEPT_H__INCLUDED
#define NEUTRINO__TEST__STDEXCEPT_H__INCLUDED

// Standard:
#include <cstddef>

// Neutrino:
#include <neutrino/backtrace.h>
#include <neutrino/exception.h>


namespace neutrino {

class TestAssertFailed: public Exception
{
	using Exception::Exception;

  public:
	// Ctor
	TestAssertFailed (std::string const& explanation, std::string const& details):
		Exception ("failed to verify that " + explanation + "; " + details)
	{
		hide_backtrace();
	}
};

} // namespace neutrino

#endif

