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

#ifndef XEFIS__INSTRUMENTS__HSI_H__INCLUDED
#define XEFIS__INSTRUMENTS__HSI_H__INCLUDED

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
#include <widgets/hsi_widget.h>
#include <xefis/utility/one_pole_smoother.h>


class HSI: public Xefis::Instrument
{
	Q_OBJECT

  public:
	// Ctor
	HSI (QDomElement const& config, QWidget* parent);

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
	data_update() override;

  private:
	void
	estimate_track();

  private:
	HSIWidget*				_hsi_widget					= nullptr;
	std::array<LatLng, 3>	_positions;
	bool					_positions_valid			= false;
	Xefis::OnePoleSmoother	_track_estimation_smoother	= 60.0; // TODO make fps independent
	NavaidStorage			_navaid_storage;

	Xefis::PropertyFloat	_gs_kt;
	Xefis::PropertyFloat	_tas_kt;
	Xefis::PropertyFloat	_mag_heading_deg;
	Xefis::PropertyFloat	_true_heading_deg;
	Xefis::PropertyBoolean	_autopilot_visible;
	Xefis::PropertyFloat	_track_deg;
	Xefis::PropertyFloat	_autopilot_heading_setting_deg;
	Xefis::PropertyFloat	_position_lat_deg;
	Xefis::PropertyFloat	_position_lng_deg;
	Xefis::PropertyFloat	_position_sea_level_radius_ft;
};


inline void
HSI::data_update()
{
	read();
}

#endif
