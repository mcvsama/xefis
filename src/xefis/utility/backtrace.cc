/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cxxabi.h>

// System:
#include <execinfo.h>

// Local:
#include "backtrace.h"


Backtrace::Backtrace()
{
	const int MAX = 256;
	void* buffer[MAX];
	int r = ::backtrace (buffer, MAX);
	char** symbols = ::backtrace_symbols (buffer, r);
	for (int i = 1; i < r; ++i)
	{
		std::string symbol (symbols[i]);
		std::string::size_type a = symbol.find ('(') + 1;
		std::string::size_type b = symbol.find ('+', a);
		if (b == std::string::npos)
			b = symbol.find (')');
		std::string name (symbol.substr (a, b - a));

		// Demangle name:
		int demangle_status = 0;
		std::size_t demangled_max_size = 256;
		char demangled_name[demangled_max_size];
		abi::__cxa_demangle (name.c_str(), demangled_name, &demangled_max_size, &demangle_status);
		_symbols.push_back (Symbol ((demangle_status == 0) ? demangled_name : name, symbol.substr (0, a-1)));
	}
	::free (symbols);
}


void
Backtrace::clog()
{
	std::clog << Backtrace() << std::endl;
}


std::ostream&
operator<< (std::ostream& os, Backtrace const& backtrace)
{
	for (Backtrace::Symbols::size_type i = 0, n = backtrace.symbols().size(); i < n; ++i)
		os << "  " << std::setw (4) << i << ". " << backtrace.symbols()[i].location << ": " << backtrace.symbols()[i].name << std::endl;
	return os;
}

