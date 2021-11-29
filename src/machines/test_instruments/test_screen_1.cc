/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
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

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "test_screen_1.h"


TestScreen1::TestScreen1 (xf::ScreenSpec const& spec, xf::Graphics const& graphics, xf::NavaidStorage const& navaid_storage, xf::Machine& machine, xf::Logger const& logger):
	Screen (spec, graphics, machine, "Test Screen 1", logger.with_scope ("TestScreen1")),
	_logger (logger),
	_graphics (graphics),
	_navaid_storage (navaid_storage),
	_adi_work_performer (1, _logger.with_scope ("ADI")),
	_hsi_work_performer (1, _logger.with_scope ("HSI")),
	_others_work_performer (1, _logger.with_scope ("generic"))
{
	adi_io->speed_ladder_line_every								= 10;
	adi_io->speed_ladder_number_every							= 20;
	adi_io->speed_ladder_extent									= 124;
	adi_io->speed_ladder_minimum								= 20;
	adi_io->speed_ladder_maximum								= 350;
	adi_io->altitude_ladder_line_every							= 100;
	adi_io->altitude_ladder_number_every						= 200;
	adi_io->altitude_ladder_emphasis_every						= 1000;
	adi_io->altitude_ladder_bold_every							= 500;
	adi_io->altitude_ladder_extent								= 825;
	adi_io->altitude_landing_warning_hi							= 1000_ft;
	adi_io->altitude_landing_warning_lo							= 500_ft;
	adi_io->raising_runway_visibility							= 1000_ft;
	adi_io->raising_runway_threshold							= 250_ft;
	adi_io->aoa_visibility_threshold							= 17.5_deg;
	adi_io->show_mach_above										= 0.4;
	adi_io->power_eq_1000_fpm									= 1000_W;
	adi_io->focus_duration										= 1_s;
	adi_io->focus_short_duration								= 0.5_s;

	hsi_io->arpt_runways_range_threshold						= 10_nmi;
	hsi_io->arpt_map_range_threshold							= 1_nmi;
	hsi_io->arpt_runway_extension_length						= 10_nmi;

	engine_l_thrust_io->dial_scale								= 0.9;
	engine_l_thrust_io->format									= boost::format ("%5.2f");
	engine_l_thrust_io->value_minimum							= 0_N;
	engine_l_thrust_io->value_maximum_warning					= 4.5_N;
	engine_l_thrust_io->value_maximum							= 4.5_N;

	engine_l_speed_io->dial_scale								= 0.9;
	engine_l_speed_io->format									= boost::format ("%5.1f");
	engine_l_speed_io->value_minimum							= 0.0_rpm;
	engine_l_speed_io->value_maximum_warning					= 12'000_rpm;
	engine_l_speed_io->value_maximum_critical					= 13'000_rpm;
	engine_l_speed_io->value_maximum							= 13'000_rpm;

	engine_l_power_io->dial_scale								= 0.9;
	engine_l_power_io->format									= boost::format ("%3.0f");
	engine_l_power_io->value_minimum							= 0_W;
	engine_l_power_io->value_maximum_warning					= 280_W;
	engine_l_power_io->value_maximum							= 280_W;

	engine_l_current_io->format									= boost::format ("%4.1f");
	engine_l_current_io->value_minimum							= -1_A;
	engine_l_current_io->value_maximum_warning					= 28_A;
	engine_l_current_io->value_maximum_critical					= 32_A;
	engine_l_current_io->value_maximum							= 32_A;
	engine_l_current_io->mirrored_style							= false;
	engine_l_current_io->line_hidden							= true;

	engine_l_temperature_io->format								= boost::format ("%5.1f");
	engine_l_temperature_io->font_scale							= 0.75;
	engine_l_temperature_io->value_minimum						= 25_degC;
	engine_l_temperature_io->value_maximum_warning				= 60_degC;
	engine_l_temperature_io->value_maximum_critical				= 65_degC;
	engine_l_temperature_io->value_maximum						= 65_degC;
	engine_l_temperature_io->mirrored_style						= false;

	engine_l_voltage_io->format									= boost::format ("%4.1f");
	engine_l_voltage_io->font_scale								= 0.75;
	engine_l_voltage_io->value_minimum							= 12.0_V;
	engine_l_voltage_io->value_minimum_critical					= 12.0_V;
	engine_l_voltage_io->value_minimum_warning					= 13.2_V;
	engine_l_voltage_io->value_maximum							= 16.8_V;
	engine_l_voltage_io->mirrored_style							= false;

	engine_l_vibration_io->format								= boost::format ("%3.1f");
	engine_l_vibration_io->font_scale							= 0.75;
	engine_l_vibration_io->value_minimum						= 0_g;
	engine_l_vibration_io->value_maximum_warning				= 1_g;
	engine_l_vibration_io->value_maximum						= 1.25_g;
	engine_l_vibration_io->mirrored_style						= false;
	engine_l_vibration_io->note									= "N₂";

	engine_r_thrust_io->dial_scale								= 0.9;
	engine_r_thrust_io->format									= boost::format ("%5.2f");
	engine_r_thrust_io->value_minimum							= 0_N;
	engine_r_thrust_io->value_maximum_warning					= 4.5_N;
	engine_r_thrust_io->value_maximum							= 4.5_N;

	engine_r_speed_io->dial_scale								= 0.9;
	engine_r_speed_io->format									= boost::format ("%5.1f");
	engine_r_speed_io->value_minimum							= 0.0_rpm;
	engine_r_speed_io->value_maximum_warning					= 12'000_rpm;
	engine_r_speed_io->value_maximum_critical					= 13'000_rpm;
	engine_r_speed_io->value_maximum							= 13'000_rpm;

	engine_r_power_io->dial_scale								= 0.9;
	engine_r_power_io->format									= boost::format ("%3.0f");
	engine_r_power_io->value_minimum							= 0_W;
	engine_r_power_io->value_maximum_warning					= 280_W;
	engine_r_power_io->value_maximum							= 280_W;

	engine_r_current_io->format									= boost::format ("%4.1f");
	engine_r_current_io->value_minimum							= -1_A;
	engine_r_current_io->value_maximum_warning					= 28_A;
	engine_r_current_io->value_maximum_critical					= 32_A;
	engine_r_current_io->value_maximum							= 32_A;
	engine_r_current_io->mirrored_style							= true;
	engine_r_current_io->line_hidden							= true;

	engine_r_temperature_io->format								= boost::format ("%5.1f");
	engine_r_temperature_io->font_scale							= 0.75;
	engine_r_temperature_io->value_minimum						= 25_degC;
	engine_r_temperature_io->value_maximum_warning				= 60_degC;
	engine_r_temperature_io->value_maximum_critical				= 65_degC;
	engine_r_temperature_io->value_maximum						= 65_degC;
	engine_r_temperature_io->mirrored_style						= true;

	engine_r_voltage_io->format									= boost::format ("%4.1f");
	engine_r_voltage_io->font_scale								= 0.75;
	engine_r_voltage_io->value_minimum							= 12.0_V;
	engine_r_voltage_io->value_minimum_critical					= 12.0_V;
	engine_r_voltage_io->value_minimum_warning					= 13.2_V;
	engine_r_voltage_io->value_maximum							= 16.8_V;
	engine_r_voltage_io->mirrored_style							= true;

	engine_r_vibration_io->format								= boost::format ("%3.1f");
	engine_r_vibration_io->font_scale							= 0.75;
	engine_r_vibration_io->value_minimum						= 0_g;
	engine_r_vibration_io->value_maximum_warning				= 1_g;
	engine_r_vibration_io->value_maximum						= 1.25_g;
	engine_r_vibration_io->mirrored_style						= true;
	engine_r_vibration_io->note									= "N₂";

	label_pwr_io->label											= "PWR";
	label_pwr_io->color											= xf::InstrumentAids::kCyan;
	label_pwr_io->font_scale									= 1.3;

	label_n1_io->label											= "N₁";
	label_n1_io->color											= xf::InstrumentAids::kCyan;
	label_n1_io->font_scale										= 1.3;

	label_temp_io->label										= "TEMP";
	label_temp_io->color										= xf::InstrumentAids::kCyan;
	label_temp_io->font_scale									= 1.3;

	label_amps_io->label										= "AMPS";
	label_amps_io->color										= xf::InstrumentAids::kCyan;
	label_amps_io->font_scale									= 1.3;

	label_thrust_io->label										= "THRUST";
	label_thrust_io->color										= xf::InstrumentAids::kCyan;
	label_thrust_io->font_scale									= 1.3;

	label_volts_io->label										= "VOLTS";
	label_volts_io->color										= xf::InstrumentAids::kCyan;
	label_volts_io->font_scale									= 1.3;

	label_vib_io->label											= "VIB";
	label_vib_io->color											= xf::InstrumentAids::kCyan;
	label_vib_io->font_scale									= 1.3;

	flaps_io->maximum_angle										= 30_deg;
	flaps_io->hide_retracted									= false;

	horizontal_trim_io->label									= "RUDDER TRIM";

	glide_ratio_io->format										= boost::format ("%3.0f");
	glide_ratio_io->font_scale									= 0.75;
	glide_ratio_io->value_minimum								= 0.0;
	glide_ratio_io->value_maximum								= 100.0;
	glide_ratio_io->mirrored_style								= false;

	glide_ratio_label_io->label									= "G/R";
	glide_ratio_label_io->color									= xf::InstrumentAids::kCyan;
	glide_ratio_label_io->font_scale							= 1.3;

	load_factor_io->format										= boost::format ("%1.1f");
	load_factor_io->font_scale									= 0.75;
	load_factor_io->value_minimum								= -1.0;
	load_factor_io->value_maximum_warning						= +2.0;
	load_factor_io->value_maximum_critical						= +3.0;
	load_factor_io->value_maximum								= +3.0;
	load_factor_io->mirrored_style								= false;

	load_factor_label_io->label									= "L/F";
	load_factor_label_io->color									= xf::InstrumentAids::kCyan;
	load_factor_label_io->font_scale							= 1.3;
}


void
TestScreen1::create_instruments()
{
	_adi.emplace (std::move (adi_io), _graphics, "adi");
	register_instrument (*_adi, _adi_work_performer);
	set (**_adi, { 0.0f, 0.0f, 0.5f, 0.63f });

	_hsi.emplace (std::move (hsi_io), _graphics, _navaid_storage, _logger, "hsi");
	register_instrument (*_hsi, _hsi_work_performer);
	set (**_hsi, { 0.0f, 0.63f, 0.5f, 1.0f - 0.63f });

	{
		auto to_n1 = [](si::AngularVelocity velocity) -> float128_t
		{
			return 100.0 * velocity / 11'500_rpm;
		};

		auto to_degrees = [](si::Temperature temperature) -> float128_t
		{
			return temperature.template in<si::Celsius>();
		};

		auto to_g = [](si::Acceleration acceleration) -> float128_t
		{
			return acceleration.template in<si::Gravity>();
		};

		constexpr float ri_scale = 0.9f;
		constexpr float li_scale = 1.0f;

		constexpr QPointF r_start_pos (0.602f, 0.1f);
		constexpr QPointF r_go_down = ri_scale * QPointF (0.0f, 0.15f);
		constexpr QPointF r_go_left = ri_scale * QPointF (-0.0575f, 0.0f);
		constexpr QPointF r_go_right = ri_scale * QPointF (+0.0575f, 0.0f);
		constexpr QPointF r_go_label = 0.3f * r_go_down;
		constexpr QPointF l_start_pos (r_start_pos.x(), r_start_pos.y() + 0.375f);
		constexpr QPointF l_go_left = li_scale * QPointF (-0.060f, 0.0f);
		constexpr QPointF l_go_right = li_scale * QPointF (+0.060f, 0.0f);
		constexpr QPointF l_go_label (0.0f, 0.0f);
		constexpr QPointF l_go_current (0.0f, 0.0f);
		constexpr QPointF l_go_temperature (0.0f, 0.085f);
		constexpr QPointF l_go_voltage (0.0f, 0.185f);
		constexpr QPointF l_go_vibration (0.0f, 0.285f);

		constexpr QSizeF ri_size = ri_scale * QSizeF { 0.13f, 0.17f };
		constexpr QSizeF li_size = li_scale * QSizeF { 0.09f, 0.088f };
		constexpr QSizeF label_size (0.05f, 0.05f);

		// Left engine

		_engine_l_thrust.emplace (std::move (engine_l_thrust_io), _graphics, nullptr, "engine.l.thrust");
		register_instrument (*_engine_l_thrust, _others_work_performer);
		set_centered (**_engine_l_thrust, { r_start_pos + r_go_left + 0 * r_go_down, ri_size });

		_engine_l_speed.emplace (std::move (engine_l_speed_io), _graphics, to_n1, "engine.l.n1");
		register_instrument (*_engine_l_speed, _others_work_performer);
		set_centered (**_engine_l_speed, { r_start_pos + r_go_left + 1 * r_go_down, ri_size });

		_engine_l_power.emplace (std::move (engine_l_power_io), _graphics, nullptr, "engine.l.power");
		register_instrument (*_engine_l_power, _others_work_performer);
		set_centered (**_engine_l_power, { r_start_pos + r_go_left + 2 * r_go_down, ri_size });

		_engine_l_current.emplace (std::move (engine_l_current_io), _graphics, nullptr, "engine.l.current");
		register_instrument (*_engine_l_current, _others_work_performer);
		set_centered (**_engine_l_current, { l_start_pos + l_go_current + l_go_left, li_size });

		_engine_l_temperature.emplace (std::move (engine_l_temperature_io), _graphics, to_degrees, "engine.l.temperature");
		register_instrument (*_engine_l_temperature, _others_work_performer);
		set_centered (**_engine_l_temperature, { l_start_pos + l_go_temperature + l_go_left, li_size });

		_engine_l_voltage.emplace (std::move (engine_l_voltage_io), _graphics, nullptr, "engine.l.voltage");
		register_instrument (*_engine_l_voltage, _others_work_performer);
		set_centered (**_engine_l_voltage, { l_start_pos + l_go_voltage + l_go_left, li_size });

		_engine_l_vibration.emplace (std::move (engine_l_vibration_io), _graphics, to_g, "engine.l.vibration");
		register_instrument (*_engine_l_vibration, _others_work_performer);
		set_centered (**_engine_l_vibration, { l_start_pos + l_go_vibration + l_go_left, li_size });

		// Right engine

		_engine_r_thrust.emplace (std::move (engine_r_thrust_io), _graphics, nullptr, "engine.r.thrust");
		register_instrument (*_engine_r_thrust, _others_work_performer);
		set_centered (**_engine_r_thrust, { r_start_pos + r_go_right + 0 * r_go_down, ri_size });

		_engine_r_speed.emplace (std::move (engine_r_speed_io), _graphics, to_n1, "engine.r.n1");
		register_instrument (*_engine_r_speed, _others_work_performer);
		set_centered (**_engine_r_speed, { r_start_pos + r_go_right + 1 * r_go_down, ri_size });

		_engine_r_power.emplace (std::move (engine_r_power_io), _graphics, nullptr, "engine.r.power");
		register_instrument (*_engine_r_power, _others_work_performer);
		set_centered (**_engine_r_power, { r_start_pos + r_go_right + 2 * r_go_down, ri_size });

		_engine_r_current.emplace (std::move (engine_r_current_io), _graphics, nullptr, "engine.r.current");
		register_instrument (*_engine_r_current, _others_work_performer);
		set_centered (**_engine_r_current, { l_start_pos + l_go_current + l_go_right, li_size });

		_engine_r_temperature.emplace (std::move (engine_r_temperature_io), _graphics, to_degrees, "engine.r.temperature");
		register_instrument (*_engine_r_temperature, _others_work_performer);
		set_centered (**_engine_r_temperature, { l_start_pos + l_go_temperature + l_go_right, li_size });

		_engine_r_voltage.emplace (std::move (engine_r_voltage_io), _graphics, nullptr, "engine.r.voltage");
		register_instrument (*_engine_r_voltage, _others_work_performer);
		set_centered (**_engine_r_voltage, { l_start_pos + l_go_voltage + l_go_right, li_size });

		_engine_r_vibration.emplace (std::move (engine_r_vibration_io), _graphics, to_g, "engine.r.vibration");
		register_instrument (*_engine_r_vibration, _others_work_performer);
		set_centered (**_engine_r_vibration, { l_start_pos + l_go_vibration + l_go_right, li_size });

		// Labels

		_label_thr.emplace (std::move (label_pwr_io), _graphics, "eicas.label.thr");
		register_instrument (*_label_thr, _others_work_performer);
		set_centered (**_label_thr, { r_start_pos + 0 * r_go_down + r_go_label, label_size });

		_label_n1.emplace (std::move (label_n1_io), _graphics, "eicas.label.thr");
		register_instrument (*_label_n1, _others_work_performer);
		set_centered (**_label_n1, { r_start_pos + 1 * r_go_down + r_go_label, label_size });

		_label_pwr.emplace (std::move (label_amps_io), _graphics, "eicas.label.amps");
		register_instrument (*_label_pwr, _others_work_performer);
		set_centered (**_label_pwr, { r_start_pos + 2 * r_go_down + r_go_label, label_size });

		_label_amps.emplace (std::move (label_thrust_io), _graphics, "eicas.label.thrust");
		register_instrument (*_label_amps, _others_work_performer);
		set_centered (**_label_amps, { l_start_pos + l_go_current + l_go_label, label_size });

		_label_temp.emplace (std::move (label_temp_io), _graphics, "eicas.label.temp");
		register_instrument (*_label_temp, _others_work_performer);
		set_centered (**_label_temp, { l_start_pos + l_go_temperature + l_go_label, label_size });

		_label_volts.emplace (std::move (label_volts_io), _graphics, "eicas.label.volts");
		register_instrument (*_label_volts, _others_work_performer);
		set_centered (**_label_volts, { l_start_pos + l_go_voltage + l_go_label, label_size });

		_label_vib.emplace (std::move (label_vib_io), _graphics, "eicas.label.vib");
		register_instrument (*_label_vib, _others_work_performer);
		set_centered (**_label_vib, { l_start_pos + l_go_vibration + l_go_label, label_size });

		constexpr QPointF trims_sect_top_left (0.8f, 0.4f);

		_gear.emplace (std::move (gear_io), _graphics, "gear");
		register_instrument (*_gear, _others_work_performer);
		set_centered (**_gear, { trims_sect_top_left, QSizeF (0.1f, 0.15f) });

		_flaps.emplace (std::move (flaps_io), _graphics, "flaps");
		register_instrument (*_flaps, _others_work_performer);
		set_centered (**_flaps, { trims_sect_top_left + QPointF (0.1f, 0.0f), QSizeF (0.1f, 0.2f) });

		_vertical_trim.emplace (std::move (vertical_trim_io), _graphics, "eicas.trim.vertical");
		register_instrument (*_vertical_trim, _others_work_performer);
		set_centered (**_vertical_trim, { trims_sect_top_left + QPointF (0.0f, 0.2f), QSizeF (0.1f, 0.12f) });

		_horizontal_trim.emplace (std::move (horizontal_trim_io), _graphics, "eicas.trim.horizontal");
		register_instrument (*_horizontal_trim, _others_work_performer);
		set_centered (**_horizontal_trim, { trims_sect_top_left + QPointF (0.1f, 0.2f), QSizeF (0.08f, 0.12f) });

		constexpr QPointF perf_meters_to_left (0.75f, 0.75f);
		constexpr QSizeF perf_meter_size = li_size;
		constexpr QSizeF perf_label_size = label_size;

		_glide_ratio.emplace (std::move (glide_ratio_io), _graphics, nullptr, "eicas.glide-ratio");
		register_instrument (*_glide_ratio, _others_work_performer);
		set_centered (**_glide_ratio, { perf_meters_to_left, perf_meter_size });

		_glide_ratio_label.emplace (std::move (glide_ratio_label_io), _graphics, "eicas.label.glide-ratio");
		register_instrument (*_glide_ratio_label, _others_work_performer);
		set_centered (**_glide_ratio_label, { perf_meters_to_left + QPointF (0.05f, 0.0f), perf_label_size });

		_load_factor.emplace (std::move (load_factor_io), _graphics, nullptr, "eicas.load-factor");
		register_instrument (*_load_factor, _others_work_performer);
		set_centered (**_load_factor, { perf_meters_to_left + QPointF (0.0f, 0.1f), perf_meter_size });

		_load_factor_label.emplace (std::move (load_factor_label_io), _graphics, "eicas.label.load-factor");
		register_instrument (*_load_factor_label, _others_work_performer);
		set_centered (**_load_factor_label, { perf_meters_to_left + QPointF (0.05f, 0.1f), perf_label_size });
	}

	set_paint_bounding_boxes (false);
}


