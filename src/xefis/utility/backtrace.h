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

#ifndef XEFIS__UTILITY__BACKTRACE_H__INCLUDED
#define XEFIS__UTILITY__BACKTRACE_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <vector>
#include <ostream>


#define SANITY_CHECK(x) \
	if (!x) \
	{ \
		std::clog << "Error: sanity check [" #x "] failed at " << __FILE__ << ":" << __LINE__ << " in " << __func__ << "()\n"; \
		Backtrace::clog(); \
	}


namespace xf {

class Backtrace
{
  public:
	struct Symbol
	{
		explicit
		Symbol (std::string const& name, std::string const& location):
			name (name), location (location)
		{ }

		std::string name;
		std::string location;
	};

	typedef std::vector<Symbol> Symbols;

  public:
	Backtrace();

	Symbols const&
	symbols() const { return _symbols; }

	static void
	clog();

  private:
	Symbols _symbols;
};


std::ostream&
operator<< (std::ostream& os, Backtrace const& backtrace);

} // namespace xf

#endif

