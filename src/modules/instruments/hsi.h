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

#ifndef XEFIS__MODULES__INSTRUMENTS__HSI_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__HSI_H__INCLUDED

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
#include <xefis/utility/smoother.h>

// Local:
#include "hsi_widget.h"


class HSI: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	HSI (Xefis::ModuleManager*, QDomElement const& config, QWidget* parent);

	// Dtor
	~HSI();

  public slots:
	/**
	 * Force HSI to read data from properties.
	 */
	void
	read();

  protected:
	void
	data_updated() override;

  private:
	void
	estimate_track();

	void
	estimate_altitude_reach_distance();

  private:
	HSIWidget*				_hsi_widget				= nullptr;
	std::array<LonLat, 3>	_positions;
	bool					_positions_valid		= false;
	Xefis::Smoother<double>	_trend_vector_smoother	= 50.0; // TODO make fps independent

	Xefis::PropertyInteger	_display_mode;
	Xefis::PropertyFloat	_range;
	Xefis::PropertyFloat	_trend_vector_range;
	Xefis::PropertyFloat	_gs_kt;
	Xefis::PropertyFloat	_tas_kt;
	Xefis::PropertyFloat	_cbr_fpm;
	Xefis::PropertyFloat	_altitude_ft;
	Xefis::PropertyFloat	_target_altitude_ft;
	Xefis::PropertyFloat	_mag_heading_deg;
	Xefis::PropertyFloat	_true_heading_deg;
	Xefis::PropertyBoolean	_autopilot_visible;
	Xefis::PropertyBoolean	_autopilot_track_visible;
	Xefis::PropertyFloat	_track_deg;
	Xefis::PropertyFloat	_autopilot_heading_setting_deg;
	Xefis::PropertyFloat	_position_lat_deg;
	Xefis::PropertyFloat	_position_lon_deg;
	Xefis::PropertyFloat	_position_sea_level_radius_ft;
	Xefis::PropertyString	_positioning_hint;
	Xefis::PropertyFloat	_wind_from_mag_heading_deg;
	Xefis::PropertyFloat	_wind_tas_kt;
	Xefis::PropertyString	_localizer_id;
};


inline void
HSI::data_updated()
{
	read();
}

#endif
