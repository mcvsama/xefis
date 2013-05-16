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

  private:
	HSIWidget*				_hsi_widget				= nullptr;
	std::array<LonLat, 3>	_positions;
	bool					_positions_valid		= false;
	Xefis::Smoother<double>	_trend_vector_smoother	= 50.0; // TODO make fps independent

	Xefis::PropertyInteger	_display_mode;
	Xefis::PropertyLength	_range;
	Xefis::PropertyLength	_trend_vector_range;
	Xefis::PropertySpeed	_gs;
	Xefis::PropertySpeed	_tas;
	Xefis::PropertySpeed	_cbr;
	Xefis::PropertyLength	_altitude;
	Xefis::PropertyLength	_target_altitude;
	Xefis::PropertyAngle	_magnetic_heading;
	Xefis::PropertyAngle	_true_heading;
	Xefis::PropertyBoolean	_use_true_heading;
	Xefis::PropertyBoolean	_display_track;
	Xefis::PropertyAngle	_true_home_direction;
	Xefis::PropertyLength	_home_dist_vlos;
	Xefis::PropertyLength	_home_dist_ground;
	Xefis::PropertyBoolean	_cmd_settings_visible;
	Xefis::PropertyBoolean	_cmd_track_visible;
	Xefis::PropertyAngle	_magnetic_track;
	Xefis::PropertyAngle	_cmd_heading_setting;
	Xefis::PropertyAngle	_position_lat;
	Xefis::PropertyAngle	_position_lon;
	Xefis::PropertyString	_positioning_hint;
	Xefis::PropertyAngle	_wind_from_magnetic_heading;
	Xefis::PropertySpeed	_wind_tas;
	Xefis::PropertyString	_localizer_id;
	Xefis::PropertyFloat	_climb_glide_ratio;
	Xefis::PropertyLength	_target_altitude_reach_distance;
};


inline void
HSI::data_updated()
{
	read();
}

#endif
