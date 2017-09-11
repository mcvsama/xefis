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

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/v2/actions.h>


class State;


class StateIO: public v2::ModuleIO
{
	friend class State;

	struct SavedProperty
	{
		SavedProperty (v2::BasicProperty& property):
			property (property)
		{ }

		v2::BasicProperty&	property;
		v2::SerialChanged	changed { property };
	};

  public:
	/*
	 * Settings
	 */

	v2::Setting<si::Time>		save_period		{ this, "save_period", 5_s };
	v2::Setting<std::string>	file_name		{ this, "file_name" };

  public:
	/**
	 * Register property for serialization/deserialization.
	 */
	void
	register_property (std::string const& unique_identifier, v2::BasicProperty&);

  private:
	std::map<std::string, SavedProperty>	_registered_properties;
};


class State: public v2::Module<StateIO>
{
  public:
	// Ctor
	explicit
	State (std::unique_ptr<StateIO>, std::string const& instance = {});

	// Dtor
	~State();

  protected:
	// Module API
	void
	process (v2::Cycle const&) override;

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
	Unique<QTimer>							_save_delay_timer;
	std::future<void>						_save_future;
};

#endif

