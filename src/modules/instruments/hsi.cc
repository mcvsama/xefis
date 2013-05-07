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
#include <xefis/utility/painter.h>
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
				{ "gs", _gs, false },
				{ "tas", _tas, false },
				{ "cbr", _cbr, false },
				{ "altitude", _altitude, false },
				{ "target-altitude", _target_altitude, false },
				{ "orientation-magnetic-heading", _magnetic_heading, false },
				{ "orientation-true-heading", _true_heading, false },
				{ "use-true-heading", _use_true_heading, false },
				{ "display-track", _display_track, false },
				{ "cmd-settings-visible", _cmd_settings_visible, false },
				{ "cmd-track-visible", _cmd_track_visible, false },
				{ "magnetic-track", _magnetic_track, false },
				{ "cmd-setting-heading", _cmd_heading_setting, false },
				{ "position-latitude", _position_lat, false },
				{ "position-longitude", _position_lon, false },
				{ "position-source", _positioning_hint, false },
				{ "wind-from-mag-heading", _wind_from_magnetic_heading, false },
				{ "wind-tas", _wind_tas, false },
				{ "localizer-id", _localizer_id, false },
				{ "climb-glide-ratio", _climb_glide_ratio, false }
			});
		}
	}

	_hsi_widget = new HSIWidget (this, work_performer());
	_hsi_widget->set_navaid_storage (navaid_storage());
	_hsi_widget->set_ndb_visible (true);
	_hsi_widget->set_vor_visible (true);
	_hsi_widget->set_dme_visible (true);
	_hsi_widget->set_loc_visible (true);
	_hsi_widget->set_fix_visible (true);

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

	_hsi_widget->set_range (_range.valid() ? *_range : 5_nm);

	if (_display_mode.valid())
	{
		if (*_display_mode == 0)
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Expanded);
		else if (*_display_mode == 1)
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Rose);
		else
			_hsi_widget->set_display_mode (HSIWidget::DisplayMode::Auxiliary);
	}

	_hsi_widget->set_heading_visible (_magnetic_heading.valid());
	if (_magnetic_heading.valid())
		_hsi_widget->set_magnetic_heading (*_magnetic_heading);

	_hsi_widget->set_navaids_visible (_true_heading.valid());
	if (_true_heading.valid())
		_hsi_widget->set_true_heading (*_true_heading);

	if (_use_true_heading.valid() && *_use_true_heading)
		_hsi_widget->set_heading_mode (HSIWidget::HeadingMode::True);
	else
		_hsi_widget->set_heading_mode (HSIWidget::HeadingMode::Magnetic);

	_hsi_widget->set_ap_heading_visible (_cmd_settings_visible.read (false) && _cmd_heading_setting.valid());
	if (_cmd_heading_setting.valid())
		_hsi_widget->set_ap_magnetic_heading (*_cmd_heading_setting);

	_hsi_widget->set_ap_track_visible (_cmd_track_visible.read (false));

	if (_magnetic_track.valid())
	{
		_hsi_widget->set_track_visible (true);
		_hsi_widget->set_magnetic_track (*_magnetic_track);
	}
	else
	{
		_hsi_widget->set_track_visible (false);
		_hsi_widget->set_magnetic_track (*_magnetic_heading);
	}

	_hsi_widget->set_display_track (_display_track.read (false));

	_hsi_widget->set_ground_speed_visible (_gs.valid());
	if (_gs.valid())
		_hsi_widget->set_ground_speed (*_gs);

	_hsi_widget->set_true_air_speed_visible (_tas.valid());
	if (_tas.valid())
		_hsi_widget->set_true_air_speed (*_tas);

	if (_position_lat.valid() && _position_lon.valid())
	{
		_hsi_widget->set_position (LonLat (*_position_lon, *_position_lat));
		_hsi_widget->set_position_valid (true);
	}
	else
		_hsi_widget->set_position_valid (false);

	_hsi_widget->set_positioning_hint_visible (_positioning_hint.valid());
	if (_positioning_hint.valid())
		_hsi_widget->set_positioning_hint ((*_positioning_hint).c_str());

	if (_wind_from_magnetic_heading.valid() && _wind_tas.valid())
	{
		_hsi_widget->set_wind_information_visible (true);
		_hsi_widget->set_wind_information (*_wind_from_magnetic_heading, *_wind_tas);
	}
	else
		_hsi_widget->set_wind_information_visible (false);

	if (_localizer_id.valid())
		_hsi_widget->set_highlighted_loc ((*_localizer_id).c_str());
	else
		_hsi_widget->reset_highlighted_loc();

	_hsi_widget->set_climb_glide_ratio_visible (_climb_glide_ratio.valid());
	_hsi_widget->set_climb_glide_ratio (*_climb_glide_ratio);
}


void
HSI::estimate_track()
{
	if (_position_lat.is_singular() || _position_lon.is_singular() || _trend_vector_range.is_singular())
	{
		_hsi_widget->set_trend_vector_visible (false);
		return;
	}

	LonLat current_position (*_position_lon, *_position_lat);

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
		_hsi_widget->set_trend_vector_lookahead (*_trend_vector_range);
		_hsi_widget->set_track_deviation (limit (beta_per_mile, -180.0_deg, +180.0_deg));
	}
}


void
HSI::estimate_altitude_reach_distance()
{
	if (_gs.is_singular() || _cbr.is_singular() ||
		_altitude.is_singular() || _target_altitude.is_singular())
	{
		_hsi_widget->set_altitude_reach_visible (false);
		return;
	}

	Length const alt_diff = *_target_altitude - *_altitude;
	_hsi_widget->set_altitude_reach_distance (*_gs * (alt_diff / *_cbr));
	_hsi_widget->set_altitude_reach_visible (true);
}

