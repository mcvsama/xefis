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

#ifndef XEFIS__UTILITY__LOGGER_H__INCLUDED
#define XEFIS__UTILITY__LOGGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <ostream>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/time_helper.h>


namespace xf {

class Logger
{
  public:
	// Ctor
	Logger();

	// Ctor
	explicit
	Logger (std::ostream& stream);

	/**
	 * Sets prefix to be written.
	 */
	void
	set_prefix (std::string const& prefix);

	/**
	 * Log function. Adds prefix to all calls.
	 */
	template<class Item>
		std::ostream&
		operator<< (Item const&) const;

  private:
	std::string		_prefix;
	std::ostream*	_stream;
};


inline
Logger::Logger():
	Logger (std::clog)
{ }


inline
Logger::Logger (std::ostream& stream):
	_stream (&stream)
{ }


inline void
Logger::set_prefix (std::string const& prefix)
{
	_prefix = prefix;
}


template<class Item>
	inline std::ostream&
	Logger::operator<< (Item const& item) const
	{
		(*_stream) << boost::format ("%08.4lf %s ") % TimeHelper::now().quantity<Second>() % _prefix;
		return (*_stream) << item;
	}

} // namespace xf

#endif

