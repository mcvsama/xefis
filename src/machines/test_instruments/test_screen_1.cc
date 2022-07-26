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

// Local:
#include "test_screen_1.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace {

float128_t
to_n1 (si::AngularVelocity velocity)
	{ return 100.0 * velocity / 11'500_rpm; };


float128_t
to_degrees (si::Temperature temperature)
	{ return temperature.template in<si::Celsius>(); };


float128_t
to_g (si::Acceleration acceleration)
	{ return acceleration.template in<si::Gravity>(); };

} // namespace


TestScreen1::TestScreen1 (xf::ScreenSpec const& spec, xf::Graphics const& graphics, xf::NavaidStorage const& navaid_storage, xf::Machine& machine, xf::Logger const& logger):
	Screen (spec, graphics, machine, "Test Screen 1", logger.with_scope ("TestScreen1")),
	_logger (logger),
	_graphics (graphics),
	_navaid_storage (navaid_storage),
	_adi_work_performer (1, _logger.with_scope ("ADI")),
	_hsi_work_performer (1, _logger.with_scope ("HSI")),
	_others_work_performer (1, _logger.with_scope ("generic")),
	// Instruments:
	adi (_graphics, "adi"),
	hsi (_graphics, _navaid_storage, _logger, "hsi"),
	engine_l_thrust (_graphics, nullptr, "engine.l.thrust"),
	engine_l_speed (_graphics, to_n1, "engine.l.n1"),
	engine_l_power (_graphics, nullptr, "engine.l.power"),
	engine_l_current (_graphics, nullptr, "engine.l.current"),
	engine_l_temperature (_graphics, to_degrees, "engine.l.temperature"),
	engine_l_voltage (_graphics, nullptr, "engine.l.voltage"),
	engine_l_vibration (_graphics, to_g, "engine.l.vibration"),
	engine_r_thrust (_graphics, nullptr, "engine.r.thrust"),
	engine_r_speed (_graphics, to_n1, "engine.r.n1"),
	engine_r_power (_graphics, nullptr, "engine.r.power"),
	engine_r_current (_graphics, nullptr, "engine.r.current"),
	engine_r_temperature (_graphics, to_degrees, "engine.r.temperature"),
	engine_r_voltage (_graphics, nullptr, "engine.r.voltage"),
	engine_r_vibration (_graphics, to_g, "engine.r.vibration"),
	label_thrust (_graphics, "eicas.label.thrust"),
	label_pwr (_graphics, "eicas.label.amps"),
	label_n1 (_graphics, "eicas.label.thr"),
	label_amps (_graphics, "eicas.label.thrust"),
	label_temp (_graphics, "eicas.label.temp"),
	label_volts (_graphics, "eicas.label.volts"),
	label_vib (_graphics, "eicas.label.vib"),
	gear (_graphics, "gear"),
	flaps (_graphics, "flaps"),
	vertical_trim (_graphics, "eicas.trim.vertical"),
	horizontal_trim (_graphics, "eicas.trim.horizontal"),
	glide_ratio (_graphics, nullptr, "eicas.glide-ratio"),
	glide_ratio_label (_graphics, "eicas.label.glide-ratio"),
	load_factor (_graphics, nullptr, "eicas.load-factor"),
	load_factor_label (_graphics, "eicas.label.load-factor")
{
	register_instruments();
	place_instruments();
	connect_instruments();
}


void
TestScreen1::register_instruments()
{
	register_instrument (this->adi, _adi_work_performer);
	register_instrument (this->hsi, _hsi_work_performer);
	register_instrument (this->engine_l_thrust, _others_work_performer);
	register_instrument (this->engine_l_speed, _others_work_performer);
	register_instrument (this->engine_l_power, _others_work_performer);
	register_instrument (this->engine_l_current, _others_work_performer);
	register_instrument (this->engine_l_temperature, _others_work_performer);
	register_instrument (this->engine_l_voltage, _others_work_performer);
	register_instrument (this->engine_l_vibration, _others_work_performer);
	register_instrument (this->engine_r_thrust, _others_work_performer);
	register_instrument (this->engine_r_speed, _others_work_performer);
	register_instrument (this->engine_r_power, _others_work_performer);
	register_instrument (this->engine_r_current, _others_work_performer);
	register_instrument (this->engine_r_temperature, _others_work_performer);
	register_instrument (this->engine_r_voltage, _others_work_performer);
	register_instrument (this->engine_r_vibration, _others_work_performer);
	register_instrument (this->label_thrust, _others_work_performer);
	register_instrument (this->label_n1, _others_work_performer);
	register_instrument (this->label_pwr, _others_work_performer);
	register_instrument (this->label_amps, _others_work_performer);
	register_instrument (this->label_temp, _others_work_performer);
	register_instrument (this->label_volts, _others_work_performer);
	register_instrument (this->label_vib, _others_work_performer);
	register_instrument (this->gear, _others_work_performer);
	register_instrument (this->flaps, _others_work_performer);
	register_instrument (this->vertical_trim, _others_work_performer);
	register_instrument (this->horizontal_trim, _others_work_performer);
	register_instrument (this->glide_ratio, _others_work_performer);
	register_instrument (this->glide_ratio_label, _others_work_performer);
	register_instrument (this->load_factor, _others_work_performer);
	register_instrument (this->load_factor_label, _others_work_performer);
}


void
TestScreen1::place_instruments()
{
	set (*this->adi, { 0.0f, 0.0f, 0.5f, 0.63f });
	set (*this->hsi, { 0.0f, 0.63f, 0.5f, 1.0f - 0.63f });

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

	set_centered (*this->engine_l_thrust, { r_start_pos + r_go_left + 0 * r_go_down, ri_size });
	set_centered (*this->engine_l_speed, { r_start_pos + r_go_left + 1 * r_go_down, ri_size });
	set_centered (*this->engine_l_power, { r_start_pos + r_go_left + 2 * r_go_down, ri_size });
	set_centered (*this->engine_l_current, { l_start_pos + l_go_current + l_go_left, li_size });
	set_centered (*this->engine_l_temperature, { l_start_pos + l_go_temperature + l_go_left, li_size });
	set_centered (*this->engine_l_voltage, { l_start_pos + l_go_voltage + l_go_left, li_size });
	set_centered (*this->engine_l_vibration, { l_start_pos + l_go_vibration + l_go_left, li_size });

	// Right engine

	set_centered (*this->engine_r_thrust, { r_start_pos + r_go_right + 0 * r_go_down, ri_size });
	set_centered (*this->engine_r_speed, { r_start_pos + r_go_right + 1 * r_go_down, ri_size });
	set_centered (*this->engine_r_power, { r_start_pos + r_go_right + 2 * r_go_down, ri_size });
	set_centered (*this->engine_r_current, { l_start_pos + l_go_current + l_go_right, li_size });
	set_centered (*this->engine_r_temperature, { l_start_pos + l_go_temperature + l_go_right, li_size });
	set_centered (*this->engine_r_voltage, { l_start_pos + l_go_voltage + l_go_right, li_size });
	set_centered (*this->engine_r_vibration, { l_start_pos + l_go_vibration + l_go_right, li_size });

	// Labels

	set_centered (*this->label_thrust, { r_start_pos + 0 * r_go_down + r_go_label, label_size });
	set_centered (*this->label_n1, { r_start_pos + 1 * r_go_down + r_go_label, label_size });
	set_centered (*this->label_pwr, { r_start_pos + 2 * r_go_down + r_go_label, label_size });
	set_centered (*this->label_amps, { l_start_pos + l_go_current + l_go_label, label_size });
	set_centered (*this->label_temp, { l_start_pos + l_go_temperature + l_go_label, label_size });
	set_centered (*this->label_volts, { l_start_pos + l_go_voltage + l_go_label, label_size });
	set_centered (*this->label_vib, { l_start_pos + l_go_vibration + l_go_label, label_size });

	constexpr QPointF trims_sect_top_left (0.8f, 0.4f);

	set_centered (*this->gear, { trims_sect_top_left, QSizeF (0.1f, 0.15f) });
	set_centered (*this->flaps, { trims_sect_top_left + QPointF (0.1f, 0.0f), QSizeF (0.1f, 0.2f) });
	set_centered (*this->vertical_trim, { trims_sect_top_left + QPointF (0.0f, 0.2f), QSizeF (0.1f, 0.12f) });
	set_centered (*this->horizontal_trim, { trims_sect_top_left + QPointF (0.1f, 0.2f), QSizeF (0.08f, 0.12f) });

	constexpr QPointF perf_meters_to_left (0.75f, 0.75f);
	constexpr QSizeF perf_meter_size = li_size;
	constexpr QSizeF perf_label_size = label_size;

	set_centered (*this->glide_ratio, { perf_meters_to_left, perf_meter_size });
	set_centered (*this->glide_ratio_label, { perf_meters_to_left + QPointF (0.05f, 0.0f), perf_label_size });
	set_centered (*this->load_factor, { perf_meters_to_left + QPointF (0.0f, 0.1f), perf_meter_size });
	set_centered (*this->load_factor_label, { perf_meters_to_left + QPointF (0.05f, 0.1f), perf_label_size });

	set_paint_bounding_boxes (false);
}


void
TestScreen1::connect_instruments()
{
	this->adi->speed_ladder_line_every					= 10;
	this->adi->speed_ladder_number_every				= 20;
	this->adi->speed_ladder_extent						= 124;
	this->adi->speed_ladder_minimum						= 20;
	this->adi->speed_ladder_maximum						= 350;
	this->adi->altitude_ladder_line_every				= 100;
	this->adi->altitude_ladder_number_every				= 200;
	this->adi->altitude_ladder_emphasis_every			= 1000;
	this->adi->altitude_ladder_bold_every				= 500;
	this->adi->altitude_ladder_extent					= 825;
	this->adi->altitude_landing_warning_hi				= 1000_ft;
	this->adi->altitude_landing_warning_lo				= 500_ft;
	this->adi->raising_runway_visibility				= 1000_ft;
	this->adi->raising_runway_threshold					= 250_ft;
	this->adi->aoa_visibility_threshold					= 17.5_deg;
	this->adi->show_mach_above							= 0.4;
	this->adi->power_eq_1000_fpm						= 1000_W;
	this->adi->focus_duration							= 1_s;
	this->adi->focus_short_duration						= 0.5_s;

	this->hsi->arpt_runways_range_threshold				= 10_nmi;
	this->hsi->arpt_map_range_threshold					= 1_nmi;
	this->hsi->arpt_runway_extension_length				= 10_nmi;

	this->engine_l_thrust->dial_scale					= 0.9;
	this->engine_l_thrust->format						= boost::format ("%5.2f");
	this->engine_l_thrust->value_minimum				= 0_N;
	this->engine_l_thrust->value_maximum_warning		= 4.5_N;
	this->engine_l_thrust->value_maximum				= 4.5_N;

	this->engine_l_speed->dial_scale					= 0.9;
	this->engine_l_speed->format						= boost::format ("%5.1f");
	this->engine_l_speed->value_minimum					= 0.0_rpm;
	this->engine_l_speed->value_maximum_warning			= 12'000_rpm;
	this->engine_l_speed->value_maximum_critical		= 13'000_rpm;
	this->engine_l_speed->value_maximum					= 13'000_rpm;

	this->engine_l_power->dial_scale					= 0.9;
	this->engine_l_power->format						= boost::format ("%3.0f");
	this->engine_l_power->value_minimum					= 0_W;
	this->engine_l_power->value_maximum_warning			= 280_W;
	this->engine_l_power->value_maximum					= 280_W;

	this->engine_l_current->format						= boost::format ("%4.1f");
	this->engine_l_current->value_minimum				= -1_A;
	this->engine_l_current->value_maximum_warning		= 28_A;
	this->engine_l_current->value_maximum_critical		= 32_A;
	this->engine_l_current->value_maximum				= 32_A;
	this->engine_l_current->mirrored_style				= false;
	this->engine_l_current->line_hidden					= true;

	this->engine_l_temperature->format					= boost::format ("%5.1f");
	this->engine_l_temperature->font_scale				= 0.75;
	this->engine_l_temperature->value_minimum			= 25_degC;
	this->engine_l_temperature->value_maximum_warning	= 60_degC;
	this->engine_l_temperature->value_maximum_critical	= 65_degC;
	this->engine_l_temperature->value_maximum			= 65_degC;
	this->engine_l_temperature->mirrored_style			= false;

	this->engine_l_voltage->format						= boost::format ("%4.1f");
	this->engine_l_voltage->font_scale					= 0.75;
	this->engine_l_voltage->value_minimum				= 12.0_V;
	this->engine_l_voltage->value_minimum_critical		= 12.0_V;
	this->engine_l_voltage->value_minimum_warning		= 13.2_V;
	this->engine_l_voltage->value_maximum				= 16.8_V;
	this->engine_l_voltage->mirrored_style				= false;

	this->engine_l_vibration->format					= boost::format ("%3.1f");
	this->engine_l_vibration->font_scale				= 0.75;
	this->engine_l_vibration->value_minimum				= 0_g;
	this->engine_l_vibration->value_maximum_warning		= 1_g;
	this->engine_l_vibration->value_maximum				= 1.25_g;
	this->engine_l_vibration->mirrored_style				= false;
	this->engine_l_vibration->note						= "N₂";

	this->engine_r_thrust->dial_scale					= 0.9;
	this->engine_r_thrust->format						= boost::format ("%5.2f");
	this->engine_r_thrust->value_minimum				= 0_N;
	this->engine_r_thrust->value_maximum_warning		= 4.5_N;
	this->engine_r_thrust->value_maximum				= 4.5_N;

	this->engine_r_speed->dial_scale					= 0.9;
	this->engine_r_speed->format						= boost::format ("%5.1f");
	this->engine_r_speed->value_minimum					= 0.0_rpm;
	this->engine_r_speed->value_maximum_warning			= 12'000_rpm;
	this->engine_r_speed->value_maximum_critical		= 13'000_rpm;
	this->engine_r_speed->value_maximum					= 13'000_rpm;

	this->engine_r_power->dial_scale					= 0.9;
	this->engine_r_power->format						= boost::format ("%3.0f");
	this->engine_r_power->value_minimum					= 0_W;
	this->engine_r_power->value_maximum_warning			= 280_W;
	this->engine_r_power->value_maximum					= 280_W;

	this->engine_r_current->format						= boost::format ("%4.1f");
	this->engine_r_current->value_minimum				= -1_A;
	this->engine_r_current->value_maximum_warning		= 28_A;
	this->engine_r_current->value_maximum_critical		= 32_A;
	this->engine_r_current->value_maximum				= 32_A;
	this->engine_r_current->mirrored_style				= true;
	this->engine_r_current->line_hidden					= true;

	this->engine_r_temperature->format					= boost::format ("%5.1f");
	this->engine_r_temperature->font_scale				= 0.75;
	this->engine_r_temperature->value_minimum			= 25_degC;
	this->engine_r_temperature->value_maximum_warning	= 60_degC;
	this->engine_r_temperature->value_maximum_critical	= 65_degC;
	this->engine_r_temperature->value_maximum			= 65_degC;
	this->engine_r_temperature->mirrored_style			= true;

	this->engine_r_voltage->format						= boost::format ("%4.1f");
	this->engine_r_voltage->font_scale					= 0.75;
	this->engine_r_voltage->value_minimum				= 12.0_V;
	this->engine_r_voltage->value_minimum_critical		= 12.0_V;
	this->engine_r_voltage->value_minimum_warning		= 13.2_V;
	this->engine_r_voltage->value_maximum				= 16.8_V;
	this->engine_r_voltage->mirrored_style				= true;

	this->engine_r_vibration->format					= boost::format ("%3.1f");
	this->engine_r_vibration->font_scale				= 0.75;
	this->engine_r_vibration->value_minimum				= 0_g;
	this->engine_r_vibration->value_maximum_warning		= 1_g;
	this->engine_r_vibration->value_maximum				= 1.25_g;
	this->engine_r_vibration->mirrored_style			= true;
	this->engine_r_vibration->note						= "N₂";

	this->label_pwr->label								= "PWR";
	this->label_pwr->color								= xf::InstrumentAids::kCyan;
	this->label_pwr->font_scale							= 1.3;

	this->label_n1->label								= "N₁";
	this->label_n1->color								= xf::InstrumentAids::kCyan;
	this->label_n1->font_scale							= 1.3;

	this->label_temp->label								= "TEMP";
	this->label_temp->color								= xf::InstrumentAids::kCyan;
	this->label_temp->font_scale						= 1.3;

	this->label_amps->label								= "AMPS";
	this->label_amps->color								= xf::InstrumentAids::kCyan;
	this->label_amps->font_scale						= 1.3;

	this->label_thrust->label							= "THRUST";
	this->label_thrust->color							= xf::InstrumentAids::kCyan;
	this->label_thrust->font_scale						= 1.3;

	this->label_volts->label							= "VOLTS";
	this->label_volts->color							= xf::InstrumentAids::kCyan;
	this->label_volts->font_scale						= 1.3;

	this->label_vib->label								= "VIB";
	this->label_vib->color								= xf::InstrumentAids::kCyan;
	this->label_vib->font_scale							= 1.3;

	this->flaps->maximum_angle							= 30_deg;
	this->flaps->hide_retracted							= false;

	this->horizontal_trim->label						= "RUDDER TRIM";

	this->glide_ratio->format							= boost::format ("%3.0f");
	this->glide_ratio->font_scale						= 0.75;
	this->glide_ratio->value_minimum					= 0.0;
	this->glide_ratio->value_maximum					= 100.0;
	this->glide_ratio->mirrored_style					= false;

	this->glide_ratio_label->label						= "G/R";
	this->glide_ratio_label->color						= xf::InstrumentAids::kCyan;
	this->glide_ratio_label->font_scale					= 1.3;

	this->load_factor->format							= boost::format ("%1.1f");
	this->load_factor->font_scale						= 0.75;
	this->load_factor->value_minimum					= -1.0;
	this->load_factor->value_maximum_warning			= +2.0;
	this->load_factor->value_maximum_critical			= +3.0;
	this->load_factor->value_maximum					= +3.0;
	this->load_factor->mirrored_style					= false;

	this->load_factor_label->label						= "L/F";
	this->load_factor_label->color						= xf::InstrumentAids::kCyan;
	this->load_factor_label->font_scale					= 1.3;
}

