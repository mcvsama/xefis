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
#include <memory>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/utility/numeric.h>

// Local:
#include "state.h"


template<class P>
	void
	State::Observable<P>::process()
	{
		typename PropertyType::Type v = *_property;

		if (v != _prev_value)
		{
			_prev_value = v;
			if (_callback)
				_callback (_property);
		}
	}


State::State (Xefis::ModuleManager* module_manager, QDomElement const&):
	Module (module_manager)
{
	_pressure_display_hpa.set_path ("/systems/fms/settings/pressure-display-hpa");
	_use_standard_pressure.set_path ("/systems/fms/settings/use-standard-pressure");
	_follow_track.set_path ("/systems/fms/settings/follow-track");
	_use_true_heading.set_path ("/systems/fms/settings/use-true-heading");
	_approach_mode.set_path ("/instruments/efis/approach/visible");
	_flight_director_visible.set_path ("/instruments/efis/flight-director/visible");
	_flight_director_enabled.set_path ("/systems/flight-director/settings/enabled");
	_flight_director_vertical_mode.set_path ("/systems/flight-director/settings/vertical-mode");
	_flight_director_lateral_mode.set_path ("/systems/flight-director/settings/lateral-mode");
	_control_hint_visible.set_path ("/instruments/efis/control/hint-visible");
	_control_hint_text.set_path ("/instruments/efis/control/hint");
	_control_stick_visible.set_path ("/instruments/efis/control-stick/visible");
	_fly_by_wire_mode.set_path ("/systems/fly-by-wire/config/mode");
	_hsi_aux_range.set_path ("/instruments/hsi-aux/range");
	_hsi_nav_range.set_path ("/instruments/hsi-nav/range");
	_hsi_nav_mode.set_path ("/instruments/hsi-nav/display-mode");

	_saitek_a.set_path ("/input/main/button/2");
	_saitek_b.set_path ("/input/main/button/3");
	_saitek_c.set_path ("/input/main/button/4");
	_saitek_d.set_path ("/input/main/button/6");
	_saitek_shift.set_path ("/input/main/button/5");
	_saitek_t1.set_path ("/input/main/button/8");
	_saitek_t2.set_path ("/input/main/button/9");
	_saitek_t3.set_path ("/input/main/button/10");
	_saitek_t4.set_path ("/input/main/button/11");
	_saitek_t5.set_path ("/input/main/button/12");
	_saitek_t6.set_path ("/input/main/button/13");
	_saitek_mode_1.set_path ("/input/main/button/27");
	_saitek_mode_2.set_path ("/input/main/button/28");
	_saitek_mode_3.set_path ("/input/main/button/29");
	_saitek_mfd_startstop.set_path ("/input/main/button/32");
	_saitek_mfd_reset.set_path ("/input/main/button/33");
	_saitek_function_up.set_path ("/input/main/button/34");
	_saitek_function_down.set_path ("/input/main/button/35");
	_saitek_function_press.set_path ("/input/main/button/31");
	_saitek_mfd_select_up.set_path ("/input/main/button/36");
	_saitek_mfd_select_down.set_path ("/input/main/button/37");
	_saitek_mfd_select_press.set_path ("/input/main/button/38");

	_observables = {
		&_saitek_t1,
		&_saitek_t2,
		&_saitek_t3,
		&_saitek_t4,
		&_saitek_t5,
		&_saitek_t6,
		&_saitek_shift,
		&_saitek_a,
		&_saitek_b,
		&_saitek_c,
		&_saitek_d,
		&_saitek_mode_1,
		&_saitek_mode_2,
		&_saitek_mode_3,
		&_saitek_mfd_startstop,
		&_saitek_mfd_reset,
		&_saitek_function_up,
		&_saitek_function_down,
		&_saitek_function_press,
		&_saitek_mfd_select_up,
		&_saitek_mfd_select_down,
		&_saitek_mfd_select_press
	};

	_saitek_t1.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_pressure_display_hpa.write (!*_pressure_display_hpa);
	});

	_saitek_t2.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_use_standard_pressure.write (!*_use_standard_pressure);
	});

	_saitek_t3.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_follow_track.write (!*_follow_track);
	});

	_saitek_t4.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_approach_mode.write (!*_approach_mode);
	});

	_saitek_t6.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_flight_director_visible.write (!*_flight_director_visible);
	});

	_saitek_mode_1.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			_control_hint_visible.write (true);
			_control_hint_text.write ("MANUAL");
			_control_stick_visible.write (false);
			_flight_director_enabled.write (false);
			_fly_by_wire_mode.write (0);
		}
	});

	_saitek_mode_2.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			_control_hint_visible.write (false);
			_control_hint_text.write ("FBW");
			_control_stick_visible.write (true);
			_flight_director_enabled.write (false);
			_fly_by_wire_mode.write (1);
		}
	});

	_saitek_mode_3.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			_control_hint_visible.write (true);
			_control_hint_text.write ("FLT DIR");
			_control_stick_visible.write (false);
			_flight_director_enabled.write (true);
			_fly_by_wire_mode.write (2);
		}
	});

	_saitek_mfd_startstop.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_flight_director_vertical_mode.write ((*_flight_director_vertical_mode + 1) % 4);
	});

	_saitek_mfd_reset.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_flight_director_lateral_mode.write ((*_flight_director_lateral_mode + 1) % 3);
	});

	_saitek_function_up.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			Length range = *_hsi_aux_range;
			if (range <= 2_nm)
				range -= 1_nm;
			else if (range <= 20_nm)
				range -= 2_nm;
			else if (range <= 60_nm)
				range -= 10_nm;
			else if (range <= 100_nm)
				range -= 20_nm;
			else
				range -= 50_nm;
			_hsi_aux_range.write (limit (range, 1_nm, 50_nm));
		}
	});

	_saitek_function_down.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			Length range = *_hsi_aux_range;
			if (range < 2_nm)
				range += 1_nm;
			else if (range < 20_nm)
				range += 2_nm;
			else if (range < 60_nm)
				range += 10_nm;
			else if (range < 100_nm)
				range += 20_nm;
			else
				range += 50_nm;
			_hsi_aux_range.write (limit (range, 1_nm, 50_nm));
		}
	});

	_saitek_mfd_select_up.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			Length range = *_hsi_nav_range;
			if (range <= 2_nm)
				range -= 1_nm;
			else if (range <= 20_nm)
				range -= 2_nm;
			else if (range <= 60_nm)
				range -= 10_nm;
			else if (range <= 100_nm)
				range -= 20_nm;
			else
				range -= 50_nm;
			_hsi_nav_range.write (limit (range, 1_nm, 50_nm));
		}
	});

	_saitek_mfd_select_down.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
		{
			Length range = *_hsi_nav_range;
			if (range < 2_nm)
				range += 1_nm;
			else if (range < 20_nm)
				range += 2_nm;
			else if (range < 60_nm)
				range += 10_nm;
			else if (range < 100_nm)
				range += 20_nm;
			else
				range += 50_nm;
			_hsi_nav_range.write (limit (range, 1_nm, 50_nm));
		}
	});

	_saitek_mfd_select_press.set_callback ([this](Xefis::PropertyBoolean& prop) {
		if (*prop)
			_hsi_nav_mode.write ((*_hsi_nav_mode + 1) % 2);
	});

	_follow_track.write (true);
	_use_true_heading.write (false);
	_use_standard_pressure.write (false);
	_pressure_display_hpa.write (true);
}


void
State::data_updated()
{
	for (ObservableBase* o: _observables)
		o->process();
}

