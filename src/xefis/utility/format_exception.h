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

#ifndef XEFIS__UTILITY__FORMAT_EXCEPTION_H__INCLUDED
#define XEFIS__UTILITY__FORMAT_EXCEPTION_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>
#include <functional>

// Boost:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

std::optional<std::string>
handle_format_exception (std::function<void()> try_block)
{
	try {
		try_block();
	}
	catch (boost::io::too_few_args&)
	{
		return "format: too few args";
	}
	catch (boost::io::too_many_args&)
	{
		return "format: too many args";
	}
	catch (boost::io::bad_format_string const&)
	{
		return "format: ill formed";
	}
	catch (...)
	{
		return "general format error";
	}

	return std::nullopt;
}

} // namespace xf

#endif

