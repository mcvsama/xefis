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

#ifndef XEFIS__MODULES__SYSTEMS__STATE_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__STATE_H__INCLUDED

// Standard:
#include <cstddef>
#include <future>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/utility/actions.h>


class State;


class StateIO: public xf::ModuleIO
{
	friend class State;

	struct SavedProperty
	{
		SavedProperty (xf::BasicPropertyOut& property):
			property (property)
		{ }

		xf::BasicPropertyOut&	property;
		xf::SerialChanged		changed { property };
	};

  public:
	/*
	 * Settings
	 */

	xf::Setting<si::Time>		save_period		{ this, "save_period", 5_s };
	xf::Setting<std::string>	file_name		{ this, "file_name" };

  public:
	/**
	 * Register property for serialization/deserialization.
	 */
	void
	register_property (std::string const& unique_identifier, xf::BasicPropertyOut&);

  private:
	std::map<std::string, SavedProperty>	_registered_properties;
};


class State: public xf::Module<StateIO>
{
  private:
	static constexpr char kLoggerScope[] = "mod::State";

  public:
	// Ctor
	explicit
	State (std::unique_ptr<StateIO>, xf::Logger const&, std::string_view const& instance = {});

	// Dtor
	~State();

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	/**
	 * Load data from the file.
	 */
	void
	load_state();

	/**
	 * Save data to the file.
	 * Runs saving code in a separate thread.
	 */
	void
	save_state();

	static QString
	do_load_state (QString file_name);

	static void
	do_save_state (QString content, QString file_name);

  private:
	xf::Logger				_logger;
	std::future<void>		_save_future;
};

#endif

