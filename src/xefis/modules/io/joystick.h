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
#include <array>
#include <set>

// Qt:
#include <QObject>
#include <QSocketNotifier>
#include <QTimer>
#include <QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>
#include <xefis/utility/range.h>


class JoystickInputIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<bool>	restart_on_failure	{ this, "restart_on_failure", true };

	// TODO PropertyOuts for axes and buttons
};


class JoystickInput:
	public QObject,
	public xf::Module<JoystickInputIO>
{
	Q_OBJECT

  private:
	static constexpr char	kLoggerScope[]	= "mod::Joystick";

	// This is defined by HID interface:
	static constexpr size_t	kMaxEventID		= 256;

	// Events from HID device are identified by such an ID:
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
		 *			Joystick event value, unchanged.
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
		explicit
		Button (QDomElement const& button_element, xf::PropertyOut<bool>&);

		xf::PropertyOut<bool>&
		property();

		void
		handle (EventType, HandlerID, int32_t value) override;

		void
		reset() override;

	  private:
		void
		set_value (float value);

	  private:
		xf::PropertyOut<bool>&	_property;
	};

	class Axis: public Handler
	{
	  public:
		// Ctor
		explicit
		Axis (QDomElement const& axis_element, xf::PropertyOut<double>&, xf::PropertyOut<si::Angle>&, xf::Range<si::Angle>& angle_range);

		/**
		 * Make Axis that is emulated by two buttons on the joystick.
		 */
		Axis (QDomElement const& axis_element, xf::PropertyOut<double>&, xf::PropertyOut<si::Angle>&, xf::Range<si::Angle>& angle_range,
			  std::optional<HandlerID> up_button_id, std::optional<HandlerID> down_button_id);

		xf::PropertyOut<double>&
		property();

		void
		handle (EventType, HandlerID id, int32_t value) override;

		void
		reset() override;

	  private:
		void
		set_value (float value);

	  private:
		xf::PropertyOut<double>&	_property;
		xf::PropertyOut<si::Angle>&	_angle_property;
		xf::Range<si::Angle>&		_angle_range;
		float						_center			= 0.f;
		float						_dead_zone		= 0.f;
		float						_reverse		= 1.f;
		float						_scale			= 1.f;
		float						_power			= 1.f;
		float						_output_minimum	= -1.f;
		float						_output_maximum	= +1.f;
		// If these are present, Axis is emulated with those two buttons,
		// that is when they're present, the property will be set to +1 or -1.
		std::optional<HandlerID>	_up_button_id;
		std::optional<HandlerID>	_down_button_id;
	};

	typedef std::array<std::vector<std::shared_ptr<Handler>>, kMaxEventID>			Handlers;
	typedef std::array<std::unique_ptr<xf::PropertyOut<bool>>, kMaxEventID>			ButtonProperties;
	typedef std::array<std::unique_ptr<xf::PropertyOut<double>>, kMaxEventID>		AxisProperties;
	typedef std::array<std::unique_ptr<xf::PropertyOut<si::Angle>>, kMaxEventID>	AngleAxisProperties;
	typedef std::array<xf::Range<si::Angle>, kMaxEventID>							AngleAxisRanges;

  public:
	// Ctor
	explicit
	JoystickInput (std::unique_ptr<JoystickInputIO>, QDomElement const& config, xf::Logger const&, std::string_view const& instance = {});

	// Module API
	void
	initialize() override;

	/**
	 * Return reference to a button property.
	 */
	xf::PropertyOut<bool>&
	button (HandlerID);

	/**
	 * Return reference to an axis property.
	 */
	xf::PropertyOut<double>&
	axis (HandlerID);

	/**
	 * Return reference to an axis property that uses si::Angle.
	 * The range is defined per-axis, and subsequent calls to the same axis with different ranges will overwrite
	 * previous ranges.
	 */
	xf::PropertyOut<si::Angle>&
	angle_axis (HandlerID, xf::Range<si::Angle>);

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
	xf::Logger							_logger;
	std::optional<std::string>			_device_path;
	int									_device				= 0;
	std::unique_ptr<QSocketNotifier>	_notifier;
	std::unique_ptr<QTimer>				_reopen_timer;
	std::set<HandlerID>					_available_buttons;
	std::set<HandlerID>					_available_axes;
	Handlers							_handlers;
	ButtonProperties					_button_properties;
	AxisProperties						_axis_properties;
	AngleAxisProperties					_angle_axis_properties;
	AngleAxisRanges						_angle_axis_ranges;
	unsigned int						_failure_count		= 0;
};


inline xf::PropertyOut<bool>&
JoystickInput::button (HandlerID id)
{
	return *_button_properties[id];
}


inline xf::PropertyOut<double>&
JoystickInput::axis (HandlerID id)
{
	return *_axis_properties[id];
}


inline xf::PropertyOut<si::Angle>&
JoystickInput::angle_axis (HandlerID id, xf::Range<si::Angle> range)
{
	_angle_axis_ranges[id] = range;
	return *_angle_axis_properties[id];
}

#endif
