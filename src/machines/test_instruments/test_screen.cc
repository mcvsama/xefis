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
#include "test_screen.h"


TestScreen::TestScreen (xf::ScreenSpec const& spec):
	Screen (spec)
{
	// Settings:
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

	engine_l_temperature_io->dial_scale							= 0.9;
	engine_l_temperature_io->format								= boost::format ("%5.1f");
	engine_l_temperature_io->precision							= 1;
	engine_l_temperature_io->value_minimum						= -20_degC;
	engine_l_temperature_io->value_maximum_warning				= 60_degC;
	engine_l_temperature_io->value_maximum_critical				= 65_degC;
	engine_l_temperature_io->value_maximum						= 65_degC;

	engine_l_power_io->dial_scale								= 0.9;
	engine_l_power_io->format									= boost::format ("%3.0f");
	engine_l_power_io->precision								= 1;
	engine_l_power_io->value_minimum							= 0_W;
	engine_l_power_io->value_maximum_warning					= 280_W;
	engine_l_power_io->value_maximum							= 280_W;

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

	engine_r_temperature_io->dial_scale							= 0.9;
	engine_r_temperature_io->format								= boost::format ("%5.1f");
	engine_r_temperature_io->precision							= 1;
	engine_r_temperature_io->value_minimum						= -20_degC;
	engine_r_temperature_io->value_maximum_warning				= 60_degC;
	engine_r_temperature_io->value_maximum_critical				= 65_degC;
	engine_r_temperature_io->value_maximum						= 65_degC;

	engine_r_power_io->dial_scale								= 0.9;
	engine_r_power_io->format									= boost::format ("%3.0f");
	engine_r_power_io->precision								= 1;
	engine_r_power_io->value_minimum							= 0_W;
	engine_r_power_io->value_maximum_warning					= 280_W;
	engine_r_power_io->value_maximum							= 280_W;

	label_thr_io->label											= "THR";
	label_thr_io->color											= xf::InstrumentAids::kCyan;

	label_n1_io->label											= "N₁";
	label_n1_io->color											= xf::InstrumentAids::kCyan;

	label_temp_io->label										= "TEMP";
	label_temp_io->color										= xf::InstrumentAids::kCyan;

	label_pwr_io->label											= "PWR";
	label_pwr_io->color											= xf::InstrumentAids::kCyan;
}


void
TestScreen::create_instruments()
{
	_adi.emplace (std::move (adi_io), "adi");
	register_instrument (*_adi);
	set (**_adi, { 0.0f, 0.0f, 0.5f, 0.63f });

	auto to_degrees = [](xf::Property<si::Temperature> const& temperature) -> float128_t
	{
		if (temperature)
			return temperature->template in<si::Celsius>();
		else
			return 0.0L;
	};

	auto to_rpm = [](xf::Property<si::AngularVelocity> const& velocity) -> float128_t
	{
		if (velocity)
			return 100.0 * *velocity / 11'500_rpm;
		else
			return 0.0L;
	};

	constexpr float ri_scale = 0.9f;
	constexpr QPointF ri_margin { 0.55f, 0.1f };
	constexpr QPointF ri_delta = ri_scale * QPointF { 0.115f, 0.15f };
	constexpr QPointF ri_delta_x { ri_delta.x(), 0.f };
	constexpr QPointF ri_delta_y { 0.f, ri_delta.y() };
	constexpr QSizeF ri_size = ri_scale * QSizeF { 0.13f, 0.17f };
	constexpr QPointF label_offset = 0.3f * ri_delta_y;

	// Left engine

	_engine_l_thrust.emplace (std::move (engine_l_thrust_io), nullptr, "engine.l.thrust");
	register_instrument (*_engine_l_thrust);
	set_centered (**_engine_l_thrust, { ri_margin + 0 * ri_delta_x + 0 * ri_delta_y, ri_size });

	_engine_l_speed.emplace (std::move (engine_l_speed_io), to_rpm, "engine.l.rpm");
	register_instrument (*_engine_l_speed);
	set_centered (**_engine_l_speed, { ri_margin + 0 * ri_delta_x + 1 * ri_delta_y, ri_size });

	_engine_l_temperature.emplace (std::move (engine_l_temperature_io), to_degrees, "engine.l.temperature");
	register_instrument (*_engine_l_temperature);
	set_centered (**_engine_l_temperature, { ri_margin + 0 * ri_delta_x + 2 * ri_delta_y, ri_size });

	_engine_l_power.emplace (std::move (engine_l_power_io), nullptr, "engine.l.power");
	register_instrument (*_engine_l_power);
	set_centered (**_engine_l_power, { ri_margin + 0 * ri_delta_x + 3 * ri_delta_y, ri_size });

	// Right engine

	_engine_r_thrust.emplace (std::move (engine_r_thrust_io), nullptr, "engine.r.thrust");
	register_instrument (*_engine_r_thrust);
	set_centered (**_engine_r_thrust, { ri_margin + 1 * ri_delta_x + 0 * ri_delta_y, ri_size });

	_engine_r_speed.emplace (std::move (engine_r_speed_io), to_rpm, "engine.r.rpm");
	register_instrument (*_engine_r_speed);
	set_centered (**_engine_r_speed, { ri_margin + 1 * ri_delta_x + 1 * ri_delta_y, ri_size });

	_engine_r_temperature.emplace (std::move (engine_r_temperature_io), to_degrees, "engine.r.temperature");
	register_instrument (*_engine_r_temperature);
	set_centered (**_engine_r_temperature, { ri_margin + 1 * ri_delta_x + 2 * ri_delta_y, ri_size });

	_engine_r_power.emplace (std::move (engine_r_power_io), nullptr, "engine.r.power");
	register_instrument (*_engine_r_power);
	set_centered (**_engine_r_power, { ri_margin + 1 * ri_delta_x + 3 * ri_delta_y, ri_size });

	// TODO Engine voltages and amps shown as Linear Indicators

	// Labels

	_label_thr.emplace (std::move (label_thr_io), "eicas.label.thr");
	register_instrument (*_label_thr);
	set_centered (**_label_thr, { QPointF { ri_margin.x() + (ri_delta.x() / 2), ri_margin.y() + (0 * ri_delta.y()) } + label_offset, QSizeF { 0.1f, 0.1f } });

	_label_n1.emplace (std::move (label_n1_io), "eicas.label.thr");
	register_instrument (*_label_n1);
	set_centered (**_label_n1, { QPointF { ri_margin.x() + (ri_delta.x() / 2), ri_margin.y() + (1 * ri_delta.y()) } + label_offset, QSizeF { 0.1f, 0.1f } });

	_label_temp.emplace (std::move (label_temp_io), "eicas.label.temp");
	register_instrument (*_label_temp);
	set_centered (**_label_temp, { QPointF { ri_margin.x() + (ri_delta.x() / 2), ri_margin.y() + (2 * ri_delta.y()) } + label_offset, QSizeF { 0.1f, 0.1f } });

	_label_pwr.emplace (std::move (label_pwr_io), "eicas.label.pwr");
	register_instrument (*_label_pwr);
	set_centered (**_label_pwr, { QPointF { ri_margin.x() + (ri_delta.x() / 2), ri_margin.y() + (3 * ri_delta.y()) } + label_offset, QSizeF { 0.1f, 0.1f } });

	_gear.emplace (std::move (gear_io), "gear");
	register_instrument (*_gear);
	set (**_gear, { 0.5, 0.0f, 0.5f, 1.0f });
}


