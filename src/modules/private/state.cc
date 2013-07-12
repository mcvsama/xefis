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


XEFIS_REGISTER_MODULE_CLASS ("private/state", State);


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


State::State (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	using Xefis::limit;

	// adi:
	Xefis::PropertyBoolean ("/settings/adi/flight-path-marker-visible").write (true);
	Xefis::PropertyBoolean ("/settings/adi/pitch-limit-visible").write (true);
	Xefis::PropertyAngle ("/settings/adi/roll-warning").write (35_deg);
	Xefis::PropertyFloat ("/settings/adi/slip-skid-warning").write (1.5);
	_approach_mode.set_path ("/settings/adi/approach-reference-visible");
	_control_hint_visible.set_path ("/settings/adi/control-hint-visible");
	_control_hint_text.set_path ("/settings/adi/control-hint");
	_control_stick_visible.set_path ("/settings/adi/control-stick-visible");

	// altimeter:
	_pressure_display_hpa.set_path ("/settings/altimeter/pressure-in-hpa");
	_use_standard_pressure.set_path ("/settings/altimeter/use-std-pressure");

	// flight-director:
	Xefis::PropertyAngle ("/settings/flight-director/cmd-magnetic-heading-and-track").write (0_deg);
	Xefis::PropertySpeed ("/settings/flight-director/cmd-ias").write (0_kt);
	Xefis::PropertySpeed ("/settings/flight-director/cmd-vertical-speed").write (0_fpm);
	Xefis::PropertyLength ("/settings/flight-director/cmd-altitude").write (0_ft);
	Xefis::PropertyAngle ("/settings/flight-director/cmd-fpa").write (0_deg);
	Xefis::PropertyInteger ("/settings/flight-director/vertical-mode").write (0);
	Xefis::PropertyInteger ("/settings/flight-director/lateral-mode").write (0);
	Xefis::PropertyAngle ("/settings/flight-director/limits/pitch").write (7_deg);
	Xefis::PropertyAngle ("/settings/flight-director/limits/roll").write (25_deg);
	_flight_director_visible.set_path ("/settings/flight-director/visible");
	_flight_director_visible.write (true);
	_flight_director_guidance_visible.set_path ("/settings/flight-director/guidance-visible");
	_flight_director_guidance_visible.write (false);
	_flight_director_enabled.set_path ("/settings/flight-director/enabled");
	_flight_director_vertical_mode.set_path ("/settings/flight-director/vertical-mode");
	_flight_director_lateral_mode.set_path ("/settings/flight-director/lateral-mode");

	// fly-by-wire:
	_fly_by_wire_mode.set_path ("/settings/fly-by-wire/mode");
	_fly_by_wire_mode.write (0);

	// hsi:
	Xefis::PropertyInteger ("/settings/hsi-aux/mode").write (2);
	Xefis::PropertyLength ("/settings/hsi-aux/trend-vector-range").write (2_nm);
	Xefis::PropertyLength ("/settings/hsi-nav/trend-vector-range").write (2_nm);
	_follow_track.set_path ("/settings/hsi/follow-track");
	_use_true_heading.set_path ("/settings/hsi/use-true-heading");
	_hsi_aux_range.set_path ("/settings/hsi-aux/range");
	_hsi_aux_range.write (2_nm);
	_hsi_nav_range.set_path ("/settings/hsi-nav/range");
	_hsi_nav_range.write (20_nm);
	_hsi_nav_mode.set_path ("/settings/hsi-nav/mode");
	_hsi_nav_mode.write (0);

	// gear:
	Xefis::PropertyBoolean ("/settings/gear/lowered").write (false);
	Xefis::PropertyBoolean ("//sensors/gear/lowered").write (false);

	// lights:
	Xefis::PropertyBoolean ("/settings/lights/strobe").write (false);
	Xefis::PropertyBoolean ("/settings/lights/beacon").write (false);
	Xefis::PropertyBoolean ("/settings/lights/landing").write (false);

	// flaps:
	Xefis::PropertyAngle ("/settings/flaps/angle").write (0_deg);

	// minimums:
	Xefis::PropertyLength ("/settings/minimums/baro-altitude").write (3000_ft);

	// Input controls:
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
			_flight_director_guidance_visible.write (!*_flight_director_guidance_visible);
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

