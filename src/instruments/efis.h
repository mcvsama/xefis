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

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtGui/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/instrument.h>
#include <widgets/efis_widget.h>
#include <widgets/hsi_widget.h>
#include <xefis/utility/one_pole_smoother.h>


class EFIS: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	EFIS (QDomElement const& config, QWidget* parent);

  public slots:
	/**
	 * Force EFIS to read data from properties.
	 */
	void
	read();

  private:
	void
	estimate_track();

  private:
	EFISWidget*				_efis_widget	= nullptr;
	HSIWidget*				_hsi_widget		= nullptr;
	std::array<LatLng, 3>	_positions;
	Xefis::OnePoleSmoother	_track_estimation_smoother = 60.0; // TODO make fps independent

	Xefis::PropertyInteger	_speed_ladder_line_every;
	Xefis::PropertyInteger	_speed_ladder_number_every;
	Xefis::PropertyInteger	_speed_ladder_extent;
	Xefis::PropertyInteger	_altitude_ladder_line_every;
	Xefis::PropertyInteger	_altitude_ladder_number_every;
	Xefis::PropertyInteger	_altitude_ladder_bold_every;
	Xefis::PropertyInteger	_altitude_ladder_extent;
	Xefis::PropertyBoolean	_heading_numbers_visible;
	Xefis::PropertyFloat	_ias_kt;
	Xefis::PropertyFloat	_ias_lookahead_kt;
	Xefis::PropertyFloat	_minimum_ias_kt;
	Xefis::PropertyFloat	_warning_ias_kt;
	Xefis::PropertyFloat	_maximum_ias_kt;
	Xefis::PropertyFloat	_gs_kt;
	Xefis::PropertyFloat	_tas_kt;
	Xefis::PropertyFloat	_mach;
	Xefis::PropertyFloat	_pitch_deg;
	Xefis::PropertyFloat	_roll_deg;
	Xefis::PropertyFloat	_roll_limit_deg;
	Xefis::PropertyFloat	_heading_deg;
	Xefis::PropertyFloat	_slip_skid_g;
	Xefis::PropertyFloat	_slip_skid_limit_g;
	Xefis::PropertyBoolean	_fpm_visible;
	Xefis::PropertyFloat	_fpm_alpha_deg;
	Xefis::PropertyFloat	_fpm_beta_deg;
	Xefis::PropertyFloat	_track_deg;
	Xefis::PropertyFloat	_altitude_ft;
	Xefis::PropertyFloat	_altitude_agl_ft;
	Xefis::PropertyFloat	_landing_altitude_ft;
	Xefis::PropertyFloat	_pressure_inhg;
	Xefis::PropertyFloat	_cbr_fpm;
	Xefis::PropertyBoolean	_autopilot_visible;
	Xefis::PropertyFloat	_autopilot_alt_setting_ft;
	Xefis::PropertyFloat	_autopilot_speed_setting_kt;
	Xefis::PropertyFloat	_autopilot_heading_setting_deg;
	Xefis::PropertyFloat	_autopilot_cbr_setting_fpm;
	Xefis::PropertyBoolean	_flight_director_visible;
	Xefis::PropertyFloat	_flight_director_pitch_deg;
	Xefis::PropertyFloat	_flight_director_roll_deg;
	Xefis::PropertyBoolean	_navigation_needles_visible;
	Xefis::PropertyString	_navigation_type_hint;
	Xefis::PropertyFloat	_navigation_gs_needle;
	Xefis::PropertyFloat	_navigation_hd_needle;
	Xefis::PropertyFloat	_dme_distance_nm;
	Xefis::PropertyFloat	_position_lat_deg;
	Xefis::PropertyFloat	_position_lng_deg;
	Xefis::PropertyFloat	_position_sea_level_radius_ft;
};

#endif
