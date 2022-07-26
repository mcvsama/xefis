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

#ifndef XEFIS__MODULES__SYSTEMS__TRIM_CONTROL_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__TRIM_CONTROL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_observer.h>
#include <xefis/support/ui/sound_manager.h>

// Qt:
#include <QtCore/QTimer>

// Standard:
#include <cstddef>


class TrimControlIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<double>		trim_step			{ this, "trim_step", 0.01 };

	/*
	 * Input
	 */

	xf::ModuleIn<double>	trim_axis			{ this, "axis" };
	xf::ModuleIn<double>	trim_value			{ this, "value" };
	xf::ModuleIn<bool>		up_trim_button		{ this, "up-button" };
	xf::ModuleIn<bool>		down_trim_button	{ this, "down-button" };

	/*
	 * Output
	 */

	xf::ModuleOut<double>	output_trim_value	{ this, "trim-value" };

  public:
	using xf::Module::Module;
};


/**
 * Controls trim value with two buttons or axis.
 * Generates appropriate trimming sound.
 */
class TrimControl: public TrimControlIO
{
  public:
	// Ctor
	explicit
	TrimControl (xf::SoundManager*, std::string_view const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

  private:
	void
	compute_trim();

	void
	update_trim();

	void
	update_trim_without_sound();

	/**
	 * Return true if given button is 'pressed'.
	 */
	static bool
	pressed (xf::Socket<bool> const&);

	/**
	 * Return true if given axis is moved 'up'.
	 */
	static bool
	moved_up (xf::Socket<double> const&);

	/**
	 * Return true if given axis is moved 'down'.
	 */
	static bool
	moved_down (xf::Socket<double> const&);

  private:
	TrimControlIO&			_io				{ *this };
	xf::SoundManager*		_sound_manager	{ nullptr };
	double					_trim_value		{ 0.0 };
	bool					_trimming_up	{ false };
	bool					_trimming_down	{ false };
	std::unique_ptr<QTimer>	_timer;
	xf::SocketObserver		_trim_computer;
	xf::SocketObserver		_mix_computer;
};

#endif
