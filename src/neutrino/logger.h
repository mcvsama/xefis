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

#ifndef NEUTRINO__LOGGER_H__INCLUDED
#define NEUTRINO__LOGGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <variant>

// Neutrino:
#include <neutrino/si/si.h>
#include <neutrino/strong_type.h>
#include <neutrino/use_count.h>
#include <neutrino/time_helper.h>


namespace neutrino {

class LogBlock;
class Logger;


/**
 * Provides additional tag information to be included in the log line.
 */
class LoggerTagProvider
{
  public:
	virtual std::optional<std::string>
	logger_tag() const = 0;
};


/**
 * Represents a logger, prints stuff to some output stream.
 * Threadsafe as long as no-one writes to the given std::ostream beyond the LoggerOutput/LogBlock/Logger.
 */
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
	timestamps_enabled() const noexcept
		{ return _add_timestamps; }

	/**
	 * Enable/disable timestamps in logs.
	 */
	void
	set_timestamps_enabled (bool enabled)
		{ _add_timestamps = enabled; }

	/**
	 * Log given LogBlock.
	 */
	void
	log (LogBlock const&);

  private:
	UseCount		_use_count			{ this };
	std::mutex		_stream_mutex;
	std::ostream&	_stream;
	bool			_add_timestamps		{ true };
};


/**
 * A block of data to be logged. Often just a line.
 */
class LogBlock
{
  public:
	// Ctor
	explicit
	LogBlock (LoggerOutput*);

	// Copy ctor: delete
	LogBlock (LogBlock const&) = delete;

	// Move ctor
	LogBlock (LogBlock&&) = default;

	// Dtor
	~LogBlock();

	/**
	 * Return creation timestamp.
	 */
	si::Time
	timestamp() const noexcept
		{ return _timestamp; }

	/**
	 * Return the log data.
	 */
	std::string
	string() const
		{ return _sstream.str(); }

	/**
	 * Flush the collected data to the LoggerOutput.
	 */
	void
	flush();

	/**
	 * Log function. Collects the string to be logged.
	 */
	template<class Item>
		LogBlock&
		operator<< (Item&&);

	/**
	 * Interface for stream manipulators.
	 */
	LogBlock&
	operator<< (std::ostream& (*manipulator)(std::ostream&));

  private:
	LoggerOutput*		_output;
	std::ostringstream	_sstream;
	si::Time			_timestamp;
};


/**
 * Accessor to the LoggerOutput object that adds tags to logged lines.
 * Allows preparing LogBlocks that will be sent to LoggerOutput.
 */
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
	 * Return associated LoggerTagProvider.
	 */
	LoggerTagProvider const*
	logger_tag_provider() const noexcept;

	/**
	 * Associate a LoggerTagProvider.
	 */
	void
	set_logger_tag_provider (LoggerTagProvider const&);

	/**
	 * Log function. Adds scope to all calls.
	 */
	template<class Item>
		LogBlock
		operator<< (Item&&) const;

	/**
	 * Interface for stream manipulators.
	 */
	LogBlock
	operator<< (std::ostream& (*manipulator)(std::ostream&)) const;

  private:
	/**
	 * Compute cached string used as a scope string when logging.
	 */
	void
	compute_scope();

	/**
	 * Prepare log line (add cycle number, scope) and return as a string.
	 */
	std::string
	prepare_line() const;

  private:
	std::optional<UseToken>		_use_token;
	LoggerOutput*				_output					{ nullptr };
	std::vector<std::string>	_scopes;
	std::string					_computed_scope;
	LoggerTagProvider const*	_logger_tag_provider	{ nullptr };
};


inline
LoggerOutput::LoggerOutput (std::ostream& stream):
	_stream (stream)
{ }


inline
LogBlock::LogBlock (LoggerOutput* output):
	_output (output),
	_timestamp (TimeHelper::now())
{ }


inline
LogBlock::~LogBlock()
{
	flush();
}


inline void
LogBlock::flush()
{
	if (_output)
		_output->log (*this);

	_sstream.str ("");
}


template<class Item>
	inline LogBlock&
	LogBlock::operator<< (Item&& item)
	{
		if (_output)
		{
			using namespace exception_ops;

			_sstream << std::forward<Item> (item);
		}

		return *this;
	}


inline LogBlock&
LogBlock::operator<< (std::ostream& (*manipulator)(std::ostream&))
{
	if (_output)
		_sstream << *manipulator;

	return *this;
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


inline LoggerTagProvider const*
Logger::logger_tag_provider() const noexcept
{
	return _logger_tag_provider;
}


inline void
Logger::set_logger_tag_provider (LoggerTagProvider const& logger_tag_provider)
{
	_logger_tag_provider = &logger_tag_provider;
}


template<class Item>
	inline LogBlock
	Logger::operator<< (Item&& item) const
	{
		return std::move (LogBlock (_output) << prepare_line() << std::forward<Item> (item));
	}


inline LogBlock
Logger::operator<< (std::ostream& (*manipulator)(std::ostream&)) const
{
	return std::move (LogBlock (_output) << prepare_line() << *manipulator);
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

} // namespace neutrino

#endif

