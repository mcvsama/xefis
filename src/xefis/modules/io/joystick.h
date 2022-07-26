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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>

// Neutrino:
#include <neutrino/logger.h>
#include <neutrino/range.h>

// Qt:
#include <QObject>
#include <QSocketNotifier>
#include <QTimer>
#include <QDomElement>

// Standard:
#include <cstddef>
#include <vector>
#include <array>
#include <set>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class JoystickInputIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<bool>	restart_on_failure	{ this, "restart_on_failure", true };

	// TODO ModuleOuts for axes and buttons

  public:
	using xf::Module::Module;
};


class JoystickInput:
	public QObject,
	public JoystickInputIO
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
		// Dtor
		virtual
		~Handler() = default;

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
		Button (QDomElement const& button_element, xf::ModuleOut<bool>&);

		xf::ModuleOut<bool>&
		socket();

		void
		handle (EventType, HandlerID, int32_t value) override;

		void
		reset() override;

	  private:
		void
		set_value (float value);

	  private:
		xf::ModuleOut<bool>& _socket;
	};

	class Axis: public Handler
	{
	  public:
		// Ctor
		explicit
		Axis (QDomElement const& axis_element, xf::ModuleOut<double>&, xf::ModuleOut<si::Angle>&, xf::Range<si::Angle>& angle_range);

		/**
		 * Make Axis that is emulated by two buttons on the joystick.
		 */
		Axis (QDomElement const& axis_element, xf::ModuleOut<double>&, xf::ModuleOut<si::Angle>&, xf::Range<si::Angle>& angle_range,
			  std::optional<HandlerID> up_button_id, std::optional<HandlerID> down_button_id);

		xf::ModuleOut<double>&
		socket();

		void
		handle (EventType, HandlerID id, int32_t value) override;

		void
		reset() override;

	  private:
		void
		set_value (float value);

	  private:
		xf::ModuleOut<double>&		_socket;
		xf::ModuleOut<si::Angle>&	_angle_socket;
		xf::Range<si::Angle>&		_angle_range;
		float						_center			= 0.f;
		float						_dead_zone		= 0.f;
		float						_reverse		= 1.f;
		float						_scale			= 1.f;
		float						_power			= 1.f;
		float						_output_minimum	= -1.f;
		float						_output_maximum	= +1.f;
		// If these are present, Axis is emulated with those two buttons,
		// that is when they're present, the socket will be set to +1 or -1.
		std::optional<HandlerID>	_up_button_id;
		std::optional<HandlerID>	_down_button_id;
	};

	typedef std::array<std::vector<std::shared_ptr<Handler>>, kMaxEventID>		Handlers;
	typedef std::array<std::unique_ptr<xf::ModuleOut<bool>>, kMaxEventID>		ButtonSockets;
	typedef std::array<std::unique_ptr<xf::ModuleOut<double>>, kMaxEventID>		AxisSockets;
	typedef std::array<std::unique_ptr<xf::ModuleOut<si::Angle>>, kMaxEventID>	AngleAxisSockets;
	typedef std::array<xf::Range<si::Angle>, kMaxEventID>						AngleAxisRanges;

  public:
	// Ctor
	explicit
	JoystickInput (QDomElement const& config, xf::Logger const&, std::string_view const& instance = {});

	// Module API
	void
	initialize() override;

	/**
	 * Return reference to a button socket.
	 */
	xf::ModuleOut<bool>&
	button (HandlerID);

	/**
	 * Return reference to an axis socket.
	 */
	xf::ModuleOut<double>&
	axis (HandlerID);

	/**
	 * Return reference to an axis socket that uses si::Angle.
	 * The range is defined per-axis, and subsequent calls to the same axis with different ranges will overwrite
	 * previous ranges.
	 */
	xf::ModuleOut<si::Angle>&
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
	 * Set all sockets to nil.
	 */
	void
	reset_sockets();

  private:
	JoystickInputIO&					_io					{ *this };
	xf::Logger							_logger;
	std::optional<std::string>			_device_path;
	int									_device				{ 0 };
	std::unique_ptr<QSocketNotifier>	_notifier;
	std::unique_ptr<QTimer>				_reopen_timer;
	std::set<HandlerID>					_available_buttons;
	std::set<HandlerID>					_available_axes;
	Handlers							_handlers;
	ButtonSockets						_button_sockets;
	AxisSockets							_axis_sockets;
	AngleAxisSockets					_angle_axis_sockets;
	AngleAxisRanges						_angle_axis_ranges;
	unsigned int						_failure_count		{ 0 };
};


inline xf::ModuleOut<bool>&
JoystickInput::button (HandlerID id)
{
	return *_button_sockets[id];
}


inline xf::ModuleOut<double>&
JoystickInput::axis (HandlerID const id)
{
	return *_axis_sockets[id];
}


inline xf::ModuleOut<si::Angle>&
JoystickInput::angle_axis (HandlerID const id, xf::Range<si::Angle> const range)
{
	_angle_axis_ranges[id] = range;
	return *_angle_axis_sockets[id];
}

#endif
