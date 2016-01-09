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

#ifndef XEFIS__MODULES__IO__JOYSTICK_H__INCLUDED
#define XEFIS__MODULES__IO__JOYSTICK_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QObject>
#include <QtCore/QSocketNotifier>
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/module.h>
#include <xefis/utility/qdom.h>


class JoystickInput:
	public QObject,
	public xf::Module
{
	Q_OBJECT

	static constexpr size_t kMaxID = 256;

	typedef std::size_t HandlerID;

	enum class EventType
	{
		Unknown = 0,
		ButtonEvent,
		AxisEvent,
	};

	/**
	 * Base class for axes and buttons.
	 */
	class Handler
	{
	  public:
		/**
		 * Called when new joystick event comes.
		 * \param	value
		 * 			Joystick event value, unchanged.
		 */
		virtual void
		handle (EventType, HandlerID, int32_t value) = 0;

		virtual void
		reset() = 0;
	};

	class Button: public Handler
	{
	  public:
		// Ctor
		Button (QDomElement const& button_element);

		void
		handle (EventType, HandlerID, int32_t value) override;

		void
		reset() override;

	  private:
		void
		set_value (float value);

	  private:
		xf::PropertyBoolean	_user_defined_property;
	};

	class Axis: public Handler
	{
	  public:
		// Ctor
		Axis (QDomElement const& axis_element);

		/**
		 * Make Axis that is emulated by two buttons on the joystick.
		 */
		Axis (QDomElement const& axis_element, Optional<HandlerID> up_button_id, Optional<HandlerID> down_button_id);

		void
		handle (EventType, HandlerID id, int32_t value) override;

		void
		reset() override;

	  private:
		void
		set_value (float value);

	  private:
		xf::PropertyFloat	_user_defined_property;
		float				_center			= 0.f;
		float				_dead_zone		= 0.f;
		float				_reverse		= 1.f;
		float				_scale			= 1.f;
		float				_power			= 1.f;
		float				_output_minimum	= -1.f;
		float				_output_maximum	= +1.f;
		// If these are present, Axis is emulated with those two buttons:
		Optional<HandlerID>	_up_button_id;
		Optional<HandlerID>	_down_button_id;
	};

	typedef std::vector<xf::PropertyBoolean>	ButtonProperties;
	typedef std::vector<xf::PropertyFloat>		AxisProperties;

  public:
	// Ctor
	JoystickInput (xf::ModuleManager*, QDomElement const& config);

  private slots:
	/**
	 * Try to open input device.
	 */
	void
	open_device();

	/**
	 * Close device after failure is detected.
	 */
	void
	failure();

	/**
	 * Start reopen timer.
	 */
	void
	restart();

	/**
	 * Read event from the device.
	 */
	void
	read();

  private:
	/**
	 * Set all properties to nil.
	 */
	void
	reset_properties();

  private:
	QString					_prop_path			= "/joystick";
	QString					_device_path;
	int						_device				= 0;
	Unique<QSocketNotifier>	_notifier;
	Unique<QTimer>			_reopen_timer;
	std::vector<std::vector<Shared<Handler>>>
							_handlers;
	ButtonProperties		_button_properties;
	AxisProperties			_axis_properties;
	unsigned int			_failure_count		= 0;
	bool					_restart_on_failure	= true;
};

#endif
