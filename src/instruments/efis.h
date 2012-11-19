/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__INSTRUMENTS__EFIS_H__INCLUDED
#define XEFIS__INSTRUMENTS__EFIS_H__INCLUDED

// Qt:
#include <QtGui/QWidget>
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <widgets/efis_widget.h>


class EFIS: public QWidget
{
	Q_OBJECT

  public:
	EFIS (QWidget* parent);

	/**
	 * Show input alert when data is not coming for given period of time.
	 * Pass 0.f to disable. Default is 110 ms.
	 */
	void
	set_input_alert_timeout (Seconds timeout);

	/**
	 * Set path in the PropertyStorage where input data is stored.
	 */
	void
	set_path (QString const& path);

  public slots:
	/**
	 * Force EFIS to read data.
	 */
	void
	read();

  private slots:
	/**
	 * Read and apply FlightGear datagrams from UDP socket.
	 */
	void
	read_input();

	/**
	 * Show input alert (when there's no incoming
	 * data from external source).
	 */
	void
	input_timeout();

	/**
	 * Hide input alert.
	 */
	void
	input_ok();

  private:
	/**
	 * Set all input properties as invalid.
	 */
	void
	invalidate_all();

  private:
	EFISWidget*				_efis_widget				= nullptr;
	Seconds					_input_alert_timeout		= 0.0f;
	QTimer*					_input_alert_timer			= nullptr;
	QTimer*					_input_alert_hide_timer		= nullptr;
	bool					_show_input_alert			= false;
	QUdpSocket*				_input						= nullptr;
	std::string				_property_path;

	Xefis::Property<float>	_speed_kt;
	Xefis::Property<bool>	_speed_valid;
	Xefis::Property<float>	_mach;
	Xefis::Property<bool>	_mach_valid;
	Xefis::Property<float>	_pitch_deg;
	Xefis::Property<bool>	_pitch_valid;
	Xefis::Property<float>	_roll_deg;
	Xefis::Property<bool>	_roll_valid;
	Xefis::Property<float>	_heading_deg;
	Xefis::Property<bool>	_heading_valid;
	Xefis::Property<float>	_fpm_alpha_deg;
	Xefis::Property<bool>	_fpm_alpha_valid;
	Xefis::Property<float>	_fpm_beta_deg;
	Xefis::Property<bool>	_fpm_beta_valid;
	Xefis::Property<float>	_altitude_ft;
	Xefis::Property<bool>	_altitude_valid;
	Xefis::Property<float>	_pressure_inhg;
	Xefis::Property<bool>	_pressure_valid;
	Xefis::Property<float>	_cbr_fpm;
	Xefis::Property<bool>	_cbr_valid;
	Xefis::Property<float>	_autopilot_alt_setting_ft;
	Xefis::Property<bool>	_autopilot_alt_setting_valid;
	Xefis::Property<float>	_autopilot_speed_setting_kt;
	Xefis::Property<bool>	_autopilot_speed_setting_valid;
};

#endif
