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
#include <xefis/utility/navigation.h>

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
				{ "orientation-magnetic-heading", _magnetic_heading_deg, false },
				{ "orientation-true-heading", _true_heading_deg, false },
				{ "use-true-heading", _use_true_heading, false },
				{ "display-track", _display_track, false },
				{ "cmd-settings-visible", _cmd_settings_visible, false },
				{ "cmw-track-visible", _cmd_track_visible, false },
				{ "magnetic-track", _magnetic_track_deg, false },
				{ "cmd-setting-heading", _cmd_heading_setting_deg, false },
				{ "position-latitude", _position_lat_deg, false },
				{ "position-longitude", _position_lon_deg, false },
				{ "position-source", _positioning_hint, false },
				{ "wind-from-mag-heading", _wind_from_magnetic_heading_deg, false },
				{ "wind-tas", _wind_tas_kt, false },
				{ "localizer-id", _localizer_id, false }
			});
		}
	}

	_hsi_widget = new HSIWidget (this);
	_hsi_widget->set_navaid_storage (navaid_storage());
	_hsi_widget->set_ndb_visible (true);
	_hsi_widget->set_vor_visible (true);
	_hsi_widget->set_dme_visible (true);
	_hsi_widget->set_loc_visible (true);
	_hsi_widget->set_fix_visible (false);

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

	_hsi_widget->set_range (_range.valid() ? 1_nm * *_range : 5_nm);

	if (_display_mode.valid())
	{
		if (*_display_mode == 0)
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Expanded);
		else if (*_display_mode == 1)
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Rose);
		else
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Auxiliary);
	}

	_hsi_widget->set_heading_visible (_magnetic_heading_deg.valid());
	if (_magnetic_heading_deg.valid())
		_hsi_widget->set_magnetic_heading (1_deg * *_magnetic_heading_deg);

	_hsi_widget->set_navaids_visible (_true_heading_deg.valid());
	if (_true_heading_deg.valid())
		_hsi_widget->set_true_heading (1_deg * *_true_heading_deg);

	if (_use_true_heading.valid() && *_use_true_heading)
		_hsi_widget->set_heading_mode (HSIWidget::HeadingMode::True);
	else
		_hsi_widget->set_heading_mode (HSIWidget::HeadingMode::Magnetic);

	_hsi_widget->set_ap_heading_visible (_cmd_settings_visible.read (false) && _cmd_heading_setting_deg.valid());
	if (_cmd_heading_setting_deg.valid())
		_hsi_widget->set_ap_magnetic_heading (1_deg * *_cmd_heading_setting_deg);

	_hsi_widget->set_ap_track_visible (_cmd_track_visible.read (false));

	_hsi_widget->set_track_visible (_magnetic_track_deg.valid());
	if (_magnetic_track_deg.valid())
		_hsi_widget->set_magnetic_track (1_deg * *_magnetic_track_deg);

	_hsi_widget->set_display_track (_display_track.read (false));

	_hsi_widget->set_ground_speed_visible (_gs_kt.valid());
	if (_gs_kt.valid())
		_hsi_widget->set_ground_speed (*_gs_kt);

	_hsi_widget->set_true_air_speed_visible (_tas_kt.valid());
	if (_tas_kt.valid())
		_hsi_widget->set_true_air_speed (*_tas_kt);

	if (_position_lat_deg.valid() && _position_lon_deg.valid())
		_hsi_widget->set_position (LonLat (1_deg * *_position_lon_deg, 1_deg * *_position_lat_deg));

	_hsi_widget->set_positioning_hint_visible (_positioning_hint.valid());
	if (_positioning_hint.valid())
		_hsi_widget->set_positioning_hint ((*_positioning_hint).c_str());

	if (_wind_from_magnetic_heading_deg.valid() && _wind_tas_kt.valid())
	{
		_hsi_widget->set_wind_information_visible (true);
		_hsi_widget->set_wind_information (1_deg * *_wind_from_magnetic_heading_deg, *_wind_tas_kt);
	}
	else
		_hsi_widget->set_wind_information_visible (false);

	if (_localizer_id.valid())
		_hsi_widget->set_highlighted_loc ((*_localizer_id).c_str());
	else
		_hsi_widget->reset_highlighted_loc();
}


void
HSI::estimate_track()
{
	if (_position_lat_deg.is_singular() || _position_lon_deg.is_singular() || _trend_vector_range.is_singular())
	{
		_hsi_widget->set_trend_vector_visible (false);
		return;
	}

	LonLat current_position (1_deg * *_position_lon_deg, 1_deg * *_position_lat_deg);

	if (!_positions_valid)
	{
		std::fill (_positions.begin(), _positions.end(), current_position);
		_positions_valid = true;
	}

	// Estimate only if the distance between last and current positions is > 0.02nm.
	Length epsilon = 10_m;
	if (_positions[0].haversine_earth (current_position) > epsilon)
	{
		// Shift data in _positions:
		for (unsigned int i = _positions.size() - 1; i > 0; i--)
			_positions[i] = _positions[i - 1];
		_positions[0] = current_position;
	}

	Length len10 = _positions[1].haversine_earth (_positions[0]);

	Angle alpha = -180.0_deg + great_arcs_angle (_positions[2], _positions[1], _positions[0]);
	Angle beta_per_mile = alpha / len10.nm();

	if (!std::isinf (beta_per_mile.internal()) && !std::isnan (beta_per_mile.internal()))
	{
		bool visible = _positions[2].haversine_earth (_positions[0]) > 2.f * epsilon;
		if (visible)
			beta_per_mile = 1_deg * _trend_vector_smoother.process (beta_per_mile.deg());

		_hsi_widget->set_trend_vector_visible (visible);
		_hsi_widget->set_trend_vector_lookahead (1_nm * *_trend_vector_range);
		_hsi_widget->set_track_deviation (limit (beta_per_mile, -180.0_deg, +180.0_deg));
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

	_hsi_widget->set_altitude_reach_distance (1_nm * s);
	_hsi_widget->set_altitude_reach_visible (true);
}

