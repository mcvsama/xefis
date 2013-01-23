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

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>
#include <xefis/utility/qdom.h>

// Local:
#include "hsi.h"


HSI::HSI (Xefis::ModuleManager* module_manager, QDomElement const& config, QWidget* parent):
	Instrument (module_manager, parent)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "display-mode", _display_mode, true },
				{ "range", _range, true },
				{ "trend-vector-range", _trend_vector_range, false },
				{ "gs", _gs_kt, false },
				{ "tas", _tas_kt, false },
				{ "cbr", _cbr_fpm, false },
				{ "altitude", _altitude_ft, false },
				{ "target-altitude", _target_altitude_ft, false },
				{ "orientation-magnetic-heading", _mag_heading_deg, false },
				{ "orientation-true-heading", _true_heading_deg, false },
				{ "autopilot-visible", _autopilot_visible, false },
				{ "track", _track_deg, false },
				{ "autopilot-setting-heading", _autopilot_heading_setting_deg, false },
				{ "position-latitude", _position_lat_deg, false },
				{ "position-longitude", _position_lng_deg, false },
				{ "position-sea-level-radius", _position_sea_level_radius_ft, false }
			});
		}
	}

	_hsi_widget = new HSIWidget (this);
	_hsi_widget->set_navaid_storage (navaid_storage());
	_hsi_widget->set_ndb_visible (true);
	_hsi_widget->set_vor_visible (true);
	_hsi_widget->set_dme_visible (true);
	_hsi_widget->set_loc_visible (true);
	_hsi_widget->set_fix_visible (true);
	// TODO when A/P engaged: _hsi_widget->set_ap_track_visible (true);
	// TODO when radio tuned: _hsi_widget->set_highlighted_loc (identifier);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_hsi_widget);
}


HSI::~HSI()
{
	delete _hsi_widget;
}


void
HSI::read()
{
	estimate_track();
	estimate_altitude_reach_distance();

	bool autopilot_visible = _autopilot_visible.valid() && *_autopilot_visible;

	_hsi_widget->set_range (_range.valid() ? *_range : 5.f);

	if (_display_mode.valid())
	{
		if (*_display_mode == 0)
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Expanded);
		else if (*_display_mode == 1)
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Centered);
		else
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Auxiliary);
	}

	_hsi_widget->set_heading_visible (_mag_heading_deg.valid());
	if (_mag_heading_deg.valid())
		_hsi_widget->set_magnetic_heading (*_mag_heading_deg);

	_hsi_widget->set_navaids_visible (_true_heading_deg.valid());
	if (_true_heading_deg.valid())
		_hsi_widget->set_true_heading (*_true_heading_deg);

	_hsi_widget->set_ap_heading_visible (autopilot_visible && _autopilot_heading_setting_deg.valid());
	if (_autopilot_heading_setting_deg.valid())
		_hsi_widget->set_ap_magnetic_heading (*_autopilot_heading_setting_deg);

	_hsi_widget->set_track_visible (_track_deg.valid());
	if (_track_deg.valid())
		_hsi_widget->set_track (*_track_deg);

	_hsi_widget->set_ground_speed_visible (_gs_kt.valid());
	if (_gs_kt.valid())
		_hsi_widget->set_ground_speed (*_gs_kt);

	_hsi_widget->set_true_air_speed_visible (_tas_kt.valid());
	if (_tas_kt.valid())
		_hsi_widget->set_true_air_speed (*_tas_kt);

	if (_position_lat_deg.valid() && _position_lng_deg.valid())
		_hsi_widget->set_position (LatLng (*_position_lat_deg, *_position_lng_deg));
}


void
HSI::estimate_track()
{
	if (_position_lat_deg.is_singular() || _position_lng_deg.is_singular() || _trend_vector_range.is_singular())
	{
		_hsi_widget->set_trend_vector_visible (false);
		return;
	}

	LatLng current_position (*_position_lat_deg, *_position_lng_deg);

	if (!_positions_valid)
	{
		std::fill (_positions.begin(), _positions.end(), current_position);
		_positions_valid = true;
	}

	// Estimate only if the distance between last and current positions is > 0.02nm.
	Miles epsilon = 0.02; // TODO _nm
	if (haversine_nm (_positions[0], current_position) > epsilon)
	{
		// Shift data in _positions:
		for (unsigned int i = _positions.size() - 1; i > 0; i--)
			_positions[i] = _positions[i - 1];
		_positions[0] = current_position;
	}

	double len10 = haversine_nm (_positions[1], _positions[0]);
	Degrees alpha = -180.0 + great_arcs_angle (_positions[2], _positions[1], _positions[0]);
	Degrees beta_per_mile = alpha / len10;

	if (!std::isinf (beta_per_mile) && !std::isnan (beta_per_mile))
	{
		bool visible = haversine_nm (_positions[2], _positions[0]) > 2.f * epsilon;
		if (visible)
			beta_per_mile = _trend_vector_smoother.process (beta_per_mile);

		_hsi_widget->set_trend_vector_visible (visible);
		_hsi_widget->set_trend_vector_lookahead (*_trend_vector_range);
		_hsi_widget->set_track_deviation (bound (beta_per_mile, -180.0, +180.0));
	}
}


void
HSI::estimate_altitude_reach_distance()
{
	if (_gs_kt.is_singular() || _cbr_fpm.is_singular() ||
		_altitude_ft.is_singular() || _target_altitude_ft.is_singular())
	{
		_hsi_widget->set_altitude_reach_visible (false);
		return;
	}

	float const gs = *_gs_kt;
	float const cbr = *_cbr_fpm;
	float const alt_diff = *_target_altitude_ft - *_altitude_ft;

	float const cbr_s = cbr / 60.f;
	float const t = alt_diff / cbr_s;
	float const s = gs * (t / 3600.f);

	_hsi_widget->set_altitude_reach_distance (s);
	_hsi_widget->set_altitude_reach_visible (true);
}

