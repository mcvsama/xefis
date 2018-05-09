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
#include <variant>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/strong_type.h>
#include <xefis/utility/time_helper.h>


namespace xf {

class ProcessingLoop;


class Logger
{
  public:
	using Output	= std::variant<std::ostream*, Logger const*>;
	using Parent	= xf::StrongType<Logger const&, struct LoggerType>;

  public:
	/**
	 * Ctor
	 */
	explicit
	Logger (std::ostream&);

	/**
	 * Ctor
	 * This object must NOT outlive parent logger.
	 */
	explicit
	Logger (Parent const& parent);

	/**
	 * Ctor
	 */
	explicit
	Logger (std::ostream&, ProcessingLoop const&);

	/**
	 * Copy ctor
	 */
	Logger (Logger const&) = default;

	/**
	 * Ctor
	 * This object must NOT outlive parent logger.
	 */
	explicit
	Logger (Parent const& parent, ProcessingLoop const&);

	// Copy operator
	Logger&
	operator= (Logger const&) = default;

	// Move operator
	Logger&
	operator= (Logger&&) = default;

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
		operator<< (Item&&) const;

  private:
	/**
	 * Prepare log line (add cycle number, prefix) and return
	 * std::ostream to use.
	 */
	std::ostream&
	prepare_line() const;

	void
	prepare_line (std::ostream&) const;

  private:
	std::string				_prefix;
	Output					_output;
	ProcessingLoop const*	_processing_loop	{ nullptr };
};


template<class Item>
	inline std::ostream&
	Logger::operator<< (Item&& item) const
	{
		return prepare_line() << item;
	}

} // namespace xf

#endif

