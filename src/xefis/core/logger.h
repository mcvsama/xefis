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

#ifndef XEFIS__CORE__LOGGER_H__INCLUDED
#define XEFIS__CORE__LOGGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <ostream>
#include <variant>

// Boost:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/strong_type.h>
#include <xefis/utility/use_count.h>


namespace xf {

class ProcessingLoop;
class Logger;


class LoggerOutput
{
	friend class Logger;

  public:
	static constexpr char kResetColor[]		= "\033[31;1;0m";
	static constexpr char kTimestampColor[]	= "\033[38;2;100;120;220m";
	static constexpr char kScopeColor[]		= "\033[38;2;200;240;140m";
	static constexpr char kCycleColor[]		= "\033[38;2;200;140;240m";
	static constexpr char kSpecialColor[]	= "\033[38;2;140;200;240m";

  public:
	// Ctor
	explicit
	LoggerOutput (std::ostream&);

	/**
	 * True if timestamps are enabled.
	 */
	bool
	timestamps_enabled() const noexcept;

	/**
	 * Enable/disable timestamps in logs.
	 */
	void
	set_timestamps_enabled (bool enabled);

  private:
	/**
	 * Prepare log line (add timestamp) and return std::ostream to use.
	 */
	std::ostream&
	prepare_line() const;

  private:
	UseCount		_use_count			{ this };
	std::ostream&	_stream;
	bool			_add_timestamps		{ true };
};


class Logger
{
  public:
	/**
	 * Creates a null logger, that doesn't output anything anywhere.
	 */
	explicit
	Logger();

	// Ctor
	explicit
	Logger (LoggerOutput&);

	// Ctor
	explicit
	Logger (LoggerOutput&, std::string_view const& scope);

	// Copy ctor
	Logger (Logger const&) = default;

	// Copy operator
	Logger&
	operator= (Logger const&) = default;

	/**
	 * Derive new logger that uses scope of this one and a new one.
	 */
	Logger
	with_scope (std::string_view const& additional_scope) const;

	/**
	 * Return scope currently used.
	 */
	std::vector<std::string> const&
	scopes() const noexcept;

	/**
	 * Sets scope to be written.
	 */
	void
	add_scope (std::string_view const& scope);

	/**
	 * Return associated ProcessingLoop.
	 */
	ProcessingLoop const*
	processing_loop() const noexcept;

	/**
	 * Associate a ProcessingLoop.
	 */
	void
	set_processing_loop (ProcessingLoop const&);

	/**
	 * Log function. Adds scope to all calls.
	 */
	template<class Item>
		std::ostream&
		operator<< (Item&&) const;

  private:
	/**
	 * Compute cached string used as a scope string when logging.
	 */
	void
	compute_scope();

	/**
	 * Prepare log line (add cycle number, scope) and return std::ostream to
	 * use.
	 */
	std::ostream&
	prepare_line() const;

  private:
	std::optional<UseToken>		_use_token;
	LoggerOutput*				_output				{ nullptr };
	std::vector<std::string>	_scopes;
	std::string					_computed_scope;
	ProcessingLoop const*		_processing_loop	{ nullptr };
};


inline
LoggerOutput::LoggerOutput (std::ostream& stream):
	_stream (stream)
{ }


inline bool
LoggerOutput::timestamps_enabled() const noexcept
{
	return _add_timestamps;
}


inline void
LoggerOutput::set_timestamps_enabled (bool enabled)
{
	_add_timestamps = enabled;
}


inline
Logger::Logger()
{ }


inline
Logger::Logger (LoggerOutput& output):
	_use_token (output._use_count),
	_output (&output)
{ }


inline
Logger::Logger (LoggerOutput& output, std::string_view const& scope):
	Logger (output)
{
	add_scope (scope);
}


inline std::vector<std::string> const&
Logger::scopes() const noexcept
{
	return _scopes;
}


inline void
Logger::add_scope (std::string_view const& scope)
{
	_scopes.push_back (std::string (scope));
	compute_scope();
}


inline ProcessingLoop const*
Logger::processing_loop() const noexcept
{
	return _processing_loop;
}


inline void
Logger::set_processing_loop (ProcessingLoop const& processing_loop)
{
	_processing_loop = &processing_loop;
}


template<class Item>
	inline std::ostream&
	Logger::operator<< (Item&& item) const
	{
		return prepare_line() << std::forward<Item> (item);
	}


/**
 * Return new logger that has all scopes of two given loggers.
 */
inline Logger
operator+ (Logger const& a, Logger const& b)
{
	Logger new_one (a);

	for (auto const& scope: b.scopes())
		new_one.add_scope (scope);

	return new_one;
}

} // namespace xf

#endif

