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

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/v1/property.h>


namespace {

class State: public xf::Module
{
	class ConfigVariable
	{
	  public:
		explicit
		ConfigVariable (QDomElement const& config);

		ConfigVariable (ConfigVariable const&) = default;

		ConfigVariable&
		operator= (ConfigVariable const&) = default;

		ConfigVariable&
		operator= (ConfigVariable&&) = default;

		// Getter
		QString const&
		id() const noexcept;

		// Setter
		void
		set_id (QString const&);

		// Getter
		xf::PropertyType const&
		type() const noexcept;

		// Setter
		void
		set_type (xf::PropertyType const&);

		// Getter
		xf::PropertyPath const&
		path() const noexcept;

		// Setter
		void
		set_path (xf::PropertyPath const&);

		// Getter
		xf::GenericProperty&
		property() noexcept;

		// Getter
		xf::GenericProperty const&
		property() const noexcept;

		/**
		 * Return true if property is fresh.
		 */
		bool
		fresh() const noexcept;

	  private:
		QString				_id;
		xf::PropertyType	_type;
		xf::PropertyPath	_path;
		xf::GenericProperty	_property;
	};

	typedef std::map<QString, ConfigVariable> ConfigVariables;

  public:
	// Ctor
	State (xf::ModuleManager*, QDomElement const& config);

	// Dtor
	~State();

  private:
	// Module API
	void
	data_updated() override;

	/**
	 * Mark object for save in some point in the future.
	 */
	void
	try_saving();

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

	static void
	do_save_state (QString content, QString file_name);

  private:
	Unique<QTimer>		_save_delay_timer;
	QString				_file_name;
	ConfigVariables		_config_variables;
	Time				_max_save_delay		= 5_s;
	std::future<void>	_save_future;
};

} // namespace

#endif

