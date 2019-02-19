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

#ifndef NEUTRINO__BACKTRACE_H__INCLUDED
#define NEUTRINO__BACKTRACE_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>
#include <ostream>
#include <string>
#include <vector>


#define SANITY_CHECK(x) \
	if (!x) \
	{ \
		std::clog << "Error: sanity check [" #x "] failed at " << __FILE__ << ":" << __LINE__ << " in " << __func__ << "()\n"; \
		Backtrace::clog(); \
	}


namespace neutrino {

class Backtrace;


extern Backtrace
backtrace();


class Backtrace
{
	friend Backtrace backtrace();

  public:
	struct Symbol
	{
		explicit
		Symbol (std::string const& symbol, std::string const& demangled_name, std::vector<std::string> const& locations, size_t address, std::optional<size_t> offset = {}):
			symbol (symbol),
			demangled_name (demangled_name),
			locations (locations),
			address (address),
			offset (offset)
		{ }

		std::string					symbol;
		std::string					demangled_name;
		std::vector<std::string>	locations;
		size_t						address;
		std::optional<size_t>		offset;
	};

	typedef std::vector<Symbol> Symbols;

  public:
	Backtrace&
	resolve_sources();

	Symbols const&
	symbols() const { return _symbols; }

  private:
	Symbols _symbols;
};


extern std::ostream&
operator<< (std::ostream& os, Backtrace const& backtrace);

} // namespace neutrino

#endif

