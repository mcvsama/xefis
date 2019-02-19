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

#undef _GNU_SOURCE
#define _GNU_SOURCE

// Standard:
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>

// System:
#include <execinfo.h>
#include <stdio.h>
#include <link.h>

// Boost:
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/process/system.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/io.hpp>
#include <boost/range/adaptor/indexed.hpp>

// Neutrino:
#include <neutrino/demangle.h>

// Local:
#include "backtrace.h"


namespace neutrino {
namespace {

/**
 * Special name "" means base address of this executable.
 */
std::map<std::string, size_t>
get_dl_entry_points()
{
	std::map<std::string, size_t> result;

    dl_iterate_phdr ([](struct dl_phdr_info *info, [[maybe_unused]] size_t size, void* void_map) -> int {
		auto& map = *reinterpret_cast<std::map<std::string, size_t>*> (void_map);
		map[info->dlpi_name] = info->dlpi_addr;
		return 0;
	}, reinterpret_cast<void*> (&result));

	return result;
}


std::optional<size_t>
base_address_for_location (std::string const& location)
{
	static auto const dl_entry_points = get_dl_entry_points();

	auto found = dl_entry_points.find (location);

	if (found == dl_entry_points.end())
		found = dl_entry_points.find ("");

	if (found != dl_entry_points.end())
		return found->second;
	else
		return std::nullopt;
}


static void* check_addr2line = []{
	auto addr2line = boost::process::search_path ("addr2line");

	if (addr2line.empty())
		std::clog << "Note: install addr2line program to get more detailed backtraces" << std::endl;

	return nullptr;
}();

} // namespace


Backtrace&
Backtrace::resolve_sources()
{
	namespace bp = ::boost::process;

	auto rm_newlines = [](std::string& s) {
		s.erase (std::remove (s.begin(), s.end(), '\n'), s.end());
	};

	for (auto& symbol: _symbols)
	{
		if (!symbol.locations.empty())
		{
			try {
				auto hex_address = (boost::format ("0x%x") % *symbol.offset).str();

				bp::ipstream child_out, child_err;
				auto addr2line = bp::search_path ("addr2line");
				auto child = bp::child (
					addr2line,
					"--inlines", "--exe=" + symbol.locations[0], hex_address,
					bp::std_out > child_out,
					bp::std_err > bp::null
				);

				std::string line;
				std::vector<std::string> detailed_locations;

				while (child.running() && std::getline (child_out, line) && !line.empty())
				{
					rm_newlines (line);
					detailed_locations.push_back (line);
				}

				// Don't bother using result of addr2line if it didn't return anything useful ("??:?" etc):
				if (detailed_locations.size() > 1 || (detailed_locations[0].find ("??") == std::string::npos && detailed_locations[0] != ":?"))
					symbol.locations = detailed_locations;

				child.wait();
			}
			catch (...)
			{
				symbol.locations[0] += " [exception when calling addr2line]";
			}
		}
	}

	return *this;
}


Backtrace
backtrace()
{
	int const MAX_DEPTH = 256;

	Backtrace result;
	void* addresses[MAX_DEPTH];
	int num_addresses = ::backtrace (addresses, MAX_DEPTH);
	std::unique_ptr<char*, decltype(&::free)> symbols (::backtrace_symbols (addresses, num_addresses), ::free);

	if (symbols)
	{
		for (int i = 0; i < num_addresses; ++i)
		{
			std::string symbol (symbols.get()[i]);
			std::string::size_type a = symbol.find ('(') + 1;
			std::string::size_type b = symbol.find ('+', a);

			if (b == std::string::npos)
				b = symbol.find (')');

			std::string const demangled_name (symbol.substr (a, b - a));
			std::string const location = symbol.substr (0, a - 1);
			std::optional<size_t> offset;
			size_t const address = reinterpret_cast<size_t> (addresses[i]);

			if (auto const base_address = base_address_for_location (location))
				offset = address - *base_address;

			result._symbols.emplace_back (symbol, demangle (demangled_name), std::vector<std::string> { location }, address, offset);
		}
	}

	return result;
}


std::ostream&
operator<< (std::ostream& os, Backtrace const& backtrace)
{
	static constexpr char kResetColor[]		= "\033[31;1;0m";
	static constexpr char kFileColor[]		= "\033[38;2;100;120;220m";
	static constexpr char kFunctionColor[]	= "\033[38;2;120;220;100m";

	for (auto const& pair: backtrace.symbols() | boost::adaptors::indexed (0))
	{
		auto const& symbol = pair.value();

		os << std::setw (4) << pair.index() << ". ";
		os << kFunctionColor << (symbol.demangled_name.empty() ? "<unknown function>" : symbol.demangled_name) << kResetColor << " ";
		os << "at 0x" << std::setw (2 * sizeof (size_t)) << std::setfill ('0') << std::hex << symbol.address << " ";

		if (symbol.offset)
			os << "(offset 0x" << std::hex << *symbol.offset << ")";

		os << std::endl << std::setfill (' ') << std::dec;

		for (std::string const& location: symbol.locations)
			os << "        in " << kFileColor << location << kResetColor << std::endl;
	}

	return os;
}

} // namespace neutrino

