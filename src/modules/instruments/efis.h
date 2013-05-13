/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__INSTRUMENTS__EFIS_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__EFIS_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/instrument.h>

// Local:
#include "efis_widget.h"


class EFIS: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	EFIS (Xefis::ModuleManager*, QDomElement const& config, QWidget* parent);

  public slots:
	/**
	 * Force EFIS to read data from properties.
	 */
	void
	read();

  protected:
	void
	data_updated() override;

  private:
	EFISWidget*				_efis_widget = nullptr;

	Xefis::PropertyInteger	_speed_ladder_line_every;
	Xefis::PropertyInteger	_speed_ladder_number_every;
	Xefis::PropertyInteger	_speed_ladder_extent;
	Xefis::PropertyInteger	_speed_ladder_minimum;
	Xefis::PropertyInteger	_speed_ladder_maximum;
	Xefis::PropertyInteger	_altitude_ladder_line_every;
	Xefis::PropertyInteger	_altitude_ladder_number_every;
	Xefis::PropertyInteger	_altitude_ladder_bold_every;
	Xefis::PropertyInteger	_altitude_ladder_extent;
	Xefis::PropertyBoolean	_heading_numbers_visible;
	Xefis::PropertySpeed	_ias;
	Xefis::PropertySpeed	_ias_lookahead;
	Xefis::PropertySpeed	_minimum_ias;
	Xefis::PropertySpeed	_warning_ias;
	Xefis::PropertySpeed	_maximum_ias;
	Xefis::PropertyFloat	_mach;
	Xefis::PropertyAngle	_pitch;
	Xefis::PropertyAngle	_roll;
	Xefis::PropertyAngle	_pitch_limit;
	Xefis::PropertyBoolean	_pitch_limit_visible;
	Xefis::PropertyAngle	_roll_limit;
	Xefis::PropertyAngle	_magnetic_heading;
	Xefis::PropertyAngle	_true_heading;
	Xefis::PropertyFloat	_slip_skid_g;
	Xefis::PropertyFloat	_slip_skid_limit_g;
	Xefis::PropertyBoolean	_fpm_visible;
	Xefis::PropertyAngle	_fpm_alpha;
	Xefis::PropertyAngle	_fpm_beta;
	Xefis::PropertyAngle	_magnetic_track;
	Xefis::PropertyLength	_altitude;
	Xefis::PropertyLength	_altitude_lookahead;
	Xefis::PropertyLength	_altitude_agl;
	Xefis::PropertyLength	_minimums_altitude;
	Xefis::PropertyPressure	_pressure_qnh;
	Xefis::PropertyBoolean	_pressure_display_hpa;
	Xefis::PropertyBoolean	_use_standard_pressure;
	Xefis::PropertySpeed	_cbr;
	Xefis::PropertyBoolean	_cmd_settings_visible;
	Xefis::PropertyLength	_cmd_alt_setting;
	Xefis::PropertySpeed	_cmd_speed_setting;
	Xefis::PropertySpeed	_cmd_cbr_setting;
	Xefis::PropertyBoolean	_flight_director_visible;
	Xefis::PropertyAngle	_flight_director_pitch;
	Xefis::PropertyAngle	_flight_director_roll;
	Xefis::PropertyBoolean	_control_stick_visible;
	Xefis::PropertyAngle	_control_stick_pitch;
	Xefis::PropertyAngle	_control_stick_roll;
	Xefis::PropertyBoolean	_approach_reference_visible;
	Xefis::PropertyString	_approach_type_hint;
	Xefis::PropertyAngle	_lateral_deviation;
	Xefis::PropertyAngle	_vertical_deviation;
	Xefis::PropertyLength	_dme_distance;
	Xefis::PropertyBoolean	_control_hint_visible;
	Xefis::PropertyString	_control_hint;
	Xefis::PropertyBoolean	_fma_visible;
	Xefis::PropertyString	_fma_speed_hint;
	Xefis::PropertyString	_fma_speed_small_hint;
	Xefis::PropertyString	_fma_lateral_hint;
	Xefis::PropertyString	_fma_lateral_small_hint;
	Xefis::PropertyString	_fma_vertical_hint;
	Xefis::PropertyString	_fma_vertical_small_hint;
	Xefis::PropertyString	_localizer_id;
	Xefis::PropertyBoolean	_novspd_flag;
	Xefis::PropertySpeed	_speed_v1;
	Xefis::PropertySpeed	_speed_vr;
	Xefis::PropertySpeed	_speed_ref;
};


inline void
EFIS::data_updated()
{
	read();
}

#endif
