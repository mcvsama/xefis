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
#include <utility>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/configurator/configurator_widget.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qutils.h>

// Local:
#include "test_loop.h"
#include "test_screen.h"


TestLoop::TestLoop (xf::Machine* machine, xf::Xefis*, xf::Logger const& logger):
	ProcessingLoop (machine, "Test Instruments", 100_Hz, logger),
	_logger (logger)
{
	xf::ScreenSpec spec { QRect { 0, 0, 1366, 768 }, 15_in, 30_Hz, 0.235_mm, 2.1_mm };
	spec.set_scale (1.5f);
	_test_screen.emplace (spec);
	_test_screen->set_paint_bounding_boxes (false);

	auto test_generator_io = std::make_unique<TestGeneratorIO>();

	// IO:
	_test_screen->adi_io->weight_on_wheels									<< xf::ConstantSource (true);
	_test_screen->adi_io->speed_ias_serviceable								<< xf::ConstantSource (true);
	_test_screen->adi_io->speed_ias											<< test_generator_io->create_property<si::Velocity> ("speed/ias", 0_kt, { 0_kt, 300_kt }, 10_kt / 1_s);
	_test_screen->adi_io->speed_ias_lookahead								<< test_generator_io->create_property<si::Velocity> ("speed/ias.lookahead", 25_kt, { 0_kt, 300_kt }, 8_kt / 1_s);
	_test_screen->adi_io->speed_ias_minimum									<< test_generator_io->create_property<si::Velocity> ("speed/ias.minimum", 60_kt, { 50_kt, 70_kt }, 3_kt / 1_s);
	_test_screen->adi_io->speed_ias_minimum_maneuver						<< test_generator_io->create_property<si::Velocity> ("speed/ias.minimum.maneuver", 65_kt, { 55_kt, 72_kt }, 3_kt / 1_s);
	_test_screen->adi_io->speed_ias_maximum_maneuver						<< test_generator_io->create_property<si::Velocity> ("speed/ias.maximum.maneuver", 245_kt, { 238_kt, 245_kt }, 3_kt / 1_s);
	_test_screen->adi_io->speed_ias_maximum									<< test_generator_io->create_property<si::Velocity> ("speed/ias.maximum", 250_kt, { 240_kt, 260_kt }, 3_kt / 1_s);
	_test_screen->adi_io->speed_mach										<< test_generator_io->create_property<double> ("speed/mach", 0.0f, { 0.0f, 0.85f }, 0.025f / 1_s);
	_test_screen->adi_io->speed_ground										<< test_generator_io->create_property<si::Velocity> ("speed/ground-speed", 0_kt, { 0_kt, 400_kt }, 13_kt / 1_s);
	_test_screen->adi_io->speed_v1											<< test_generator_io->create_property<si::Velocity> ("speed-bugs/v1", 80_kt, { 78_kt, 82_kt }, 1_kt / 1_s);
	_test_screen->adi_io->speed_vr											<< test_generator_io->create_property<si::Velocity> ("speed-bugs/vr", 88_kt, { 86_kt, 89_kt }, 1_kt / 1_s);
	_test_screen->adi_io->speed_vref										<< test_generator_io->create_property<si::Velocity> ("speed-bugs/vref", 95_kt, { 94_kt, 96_kt }, 0.1_kt / 1_s);
	_test_screen->adi_io->speed_flaps_up_label								<< xf::ConstantSource<std::string> ("UP");
	_test_screen->adi_io->speed_flaps_up_speed								<< xf::ConstantSource (140_kt);
	_test_screen->adi_io->speed_flaps_a_label								<< xf::ConstantSource<std::string> ("1"); // TODO _b_ and _a_? maybe "upper" and "lower"?
	_test_screen->adi_io->speed_flaps_a_speed								<< xf::ConstantSource (120_kt);
	_test_screen->adi_io->speed_flaps_b_label								<< xf::ConstantSource<std::string> ("5");
	_test_screen->adi_io->speed_flaps_b_speed								<< xf::ConstantSource (110_kt);
	_test_screen->adi_io->orientation_serviceable							<< xf::ConstantSource (true);
	_test_screen->adi_io->orientation_pitch									<< test_generator_io->create_property<si::Angle> ("orientation/pitch", 0_deg, { -90_deg, 90_deg }, 8_deg / 1_s);
	_test_screen->adi_io->orientation_roll									<< test_generator_io->create_property<si::Angle> ("orientation/roll", 0_deg, { -180_deg, +180_deg }, 1.5_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	_test_screen->adi_io->orientation_heading_magnetic						<< test_generator_io->create_property<si::Angle> ("orientation/heading.magnetic", 0_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	_test_screen->adi_io->orientation_heading_true							<< test_generator_io->create_property<si::Angle> ("orientation/heading.true", 10_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	_test_screen->adi_io->orientation_heading_numbers_visible				<< xf::ConstantSource (true);
	_test_screen->adi_io->track_lateral_magnetic							<< test_generator_io->create_property<si::Angle> ("track/lateral.magnetic", 9_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	_test_screen->adi_io->track_lateral_true								<< test_generator_io->create_property<si::Angle> ("track/lateral.true", 19_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	_test_screen->adi_io->track_vertical									<< test_generator_io->create_property<si::Angle> ("track/vertical", 0_deg, { -13_deg, 13_deg }, 1_deg / 1_s);
	_test_screen->adi_io->fpv_visible										<< xf::ConstantSource (true);
	_test_screen->adi_io->slip_skid											<< test_generator_io->create_property<si::Angle> ("slip-skid/angle", 0_deg, { -5_deg, 5_deg }, 0.5_deg / 1_s);
	_test_screen->adi_io->aoa_alpha											<< test_generator_io->create_property<si::Angle> ("aoa/alpha", 0_deg, { -2_deg, 15_deg }, 1_deg / 1_s);
	_test_screen->adi_io->aoa_alpha_maximum									<< test_generator_io->create_property<si::Angle> ("aoa/alpha.maximum", 13_deg, { 13_deg, 15_deg }, 0.25_deg / 1_s);
	_test_screen->adi_io->aoa_alpha_visible									<< xf::ConstantSource (true);
	_test_screen->adi_io->altitude_amsl_serviceable							<< xf::ConstantSource (true);
	_test_screen->adi_io->altitude_amsl										<< test_generator_io->create_property<si::Length> ("altitude/amsl", -200_ft, { -200_ft, 2000_ft }, 2000_ft / 1_min);
	_test_screen->adi_io->altitude_amsl_lookahead							<< test_generator_io->create_property<si::Length> ("altitude/amsl.lookahead", 10_ft, { 0_ft, 2000_ft }, 100_ft / 1_min);
	_test_screen->adi_io->altitude_agl_serviceable							<< xf::ConstantSource (true);
	_test_screen->adi_io->altitude_agl										<< test_generator_io->create_property<si::Length> ("altitude/agl", -4_ft, { -4_ft, 30_m }, 100_ft / 1_min);
	_test_screen->adi_io->altitude_minimums_type							<< xf::ConstantSource ("BARO");
	_test_screen->adi_io->altitude_minimums_setting							<< xf::ConstantSource (300_ft);
	_test_screen->adi_io->altitude_minimums_amsl							<< xf::ConstantSource (300_ft);
	_test_screen->adi_io->altitude_landing_amsl								<< xf::ConstantSource (140_ft);
	_test_screen->adi_io->vertical_speed_serviceable						<< xf::ConstantSource (true);
	_test_screen->adi_io->vertical_speed									<< test_generator_io->create_property<si::Velocity> ("vertical-speed/speed", 0_fpm, { -6000_fpm, +6000_fpm }, 100_fpm / 1_s);
	_test_screen->adi_io->vertical_speed_energy_variometer					<< test_generator_io->create_property<si::Power> ("vertical-speed/energy-variometer", 0_W, { -1000_W, +1000_W }, 100_W / 1_s);
	_test_screen->adi_io->pressure_qnh										<< xf::ConstantSource (1013_hPa);
	_test_screen->adi_io->pressure_display_hpa								<< xf::ConstantSource (true);
	_test_screen->adi_io->pressure_use_std									<< xf::ConstantSource (true);
	// TODO xf::PropertyIn<std::string>	adi_io->flight_director_active_name { ... } // "L", "R", "C"(enter)?
	_test_screen->adi_io->flight_director_serviceable						<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_director_cmd_visible						<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_director_cmd_altitude						<< xf::ConstantSource (1000_ft);
	_test_screen->adi_io->flight_director_cmd_altitude_acquired				<< xf::ConstantSource (false);
	_test_screen->adi_io->flight_director_cmd_ias							<< xf::ConstantSource (100_kt);
	_test_screen->adi_io->flight_director_cmd_mach							<< xf::ConstantSource (0.34);
	_test_screen->adi_io->flight_director_cmd_vertical_speed				<< xf::ConstantSource (1500_fpm);
	_test_screen->adi_io->flight_director_cmd_fpa							<< xf::ConstantSource (5_deg);
	_test_screen->adi_io->flight_director_guidance_visible					<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_director_guidance_pitch					<< xf::ConstantSource (2.5_deg);
	_test_screen->adi_io->flight_director_guidance_roll						<< xf::ConstantSource (0_deg);
	_test_screen->adi_io->control_stick_visible								<< xf::ConstantSource (true);
	_test_screen->adi_io->control_stick_pitch								<< xf::ConstantSource (2_deg);
	_test_screen->adi_io->control_stick_roll								<< xf::ConstantSource (2_deg);
	_test_screen->adi_io->navaid_reference_visible							<< xf::ConstantSource (true);
	_test_screen->adi_io->navaid_course_magnetic							<< xf::ConstantSource (150_deg);
	_test_screen->adi_io->navaid_type_hint									<< xf::ConstantSource ("VOR");
	_test_screen->adi_io->navaid_identifier									<< xf::ConstantSource ("WRO");
	_test_screen->adi_io->navaid_distance									<< xf::ConstantSource (1.5_nmi);
	_test_screen->adi_io->flight_path_deviation_lateral_serviceable			<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_path_deviation_lateral_approach			<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/lateral/approach", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	_test_screen->adi_io->flight_path_deviation_lateral_flight_path			<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/lateral/flight-path", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	_test_screen->adi_io->flight_path_deviation_vertical_serviceable		<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_path_deviation_vertical					<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/vertical/deviation", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	_test_screen->adi_io->flight_path_deviation_vertical_approach			<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/vertical/approach", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	_test_screen->adi_io->flight_path_deviation_vertical_flight_path		<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/vertical/flight-path", 0_deg, { -5_deg, 5_deg }, 3_deg / 1_s);
	_test_screen->adi_io->flight_path_deviation_mixed_mode					<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_mode_hint_visible							<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_mode_hint									<< xf::ConstantSource ("TEST");
	_test_screen->adi_io->flight_mode_fma_visible							<< xf::ConstantSource (true);
	_test_screen->adi_io->flight_mode_fma_speed_hint						<< xf::ConstantSource ("THR REF");
	_test_screen->adi_io->flight_mode_fma_speed_armed_hint					<< xf::ConstantSource ("SPD");
	_test_screen->adi_io->flight_mode_fma_lateral_hint						<< xf::ConstantSource ("CMD TRK");
	_test_screen->adi_io->flight_mode_fma_lateral_armed_hint				<< xf::ConstantSource ("ILS TRK");
	_test_screen->adi_io->flight_mode_fma_vertical_hint						<< xf::ConstantSource ("CMD FPA");
	_test_screen->adi_io->flight_mode_fma_vertical_armed_hint				<< xf::ConstantSource ("G/S");
	_test_screen->adi_io->tcas_resolution_advisory_pitch_minimum			<< xf::ConstantSource (-45_deg);
	_test_screen->adi_io->tcas_resolution_advisory_pitch_maximum			<< xf::ConstantSource (80_deg);
	_test_screen->adi_io->tcas_resolution_advisory_vertical_speed_minimum	<< xf::ConstantSource (-3000_fpm);
	_test_screen->adi_io->tcas_resolution_advisory_vertical_speed_maximum	<< xf::ConstantSource (10000_fpm);
#if 0
	_test_screen->adi_io->warning_novspd_flag								<< xf::ConstantSource (true); // TODO from time to time set to true
	_test_screen->adi_io->warning_ldgalt_flag								<< xf::ConstantSource (true);
	_test_screen->adi_io->warning_pitch_disagree							<< xf::ConstantSource (true);
	_test_screen->adi_io->warning_roll_disagree								<< xf::ConstantSource (true);
	_test_screen->adi_io->warning_ias_disagree								<< xf::ConstantSource (true);
	_test_screen->adi_io->warning_altitude_disagree							<< xf::ConstantSource (true);
	_test_screen->adi_io->warning_roll										<< xf::ConstantSource (true);
	_test_screen->adi_io->warning_slip_skid									<< xf::ConstantSource (true);
#endif
	_test_screen->adi_io->style_old											<< xf::ConstantSource (false);
	_test_screen->adi_io->style_show_metric									<< xf::ConstantSource (true);

	_test_screen->engine_l_thrust_io->value									<< test_generator_io->create_property<si::Force> ("engine/left/thrust", 0_N, { -0.3_N, 4.5_N }, 0.2_N / 1_s);
	_test_screen->engine_l_thrust_io->reference								<< xf::ConstantSource (4.1_N);
	_test_screen->engine_l_thrust_io->target								<< xf::ConstantSource (3.9_N);
	_test_screen->engine_l_thrust_io->automatic								<< test_generator_io->create_property<si::Force> ("engine/left/thrust/automatic", 2_N, { 1.5_N, 2.5_N }, 0.1_N / 1_s);

	_test_screen->engine_l_speed_io->value									<< test_generator_io->create_property<si::AngularVelocity> ("engine/left/speed", 0.0_rpm, { -100_rpm, 15'000_rpm }, 1200_rpm / 1_s);

	_test_screen->engine_l_temperature_io->value							<< test_generator_io->create_property<si::Temperature> ("engine/left/temperature", 0_degC, { -20_degC, 75_degC }, 5_K / 1_s);

	_test_screen->engine_l_power_io->value									<< test_generator_io->create_property<si::Power> ("engine/left/power", 0_W, { 0_W, 295_W }, 11_W / 1_s);

	_test_screen->engine_l_current_io->value								<< test_generator_io->create_property<si::Current> ("engine/left/current", 0_A, { -5_A, 40_A }, 5_A / 1_s);

	_test_screen->engine_r_thrust_io->value									<< test_generator_io->create_property<si::Force> ("engine/right/thrust", 0_N, { -0.3_N, 4.5_N }, 0.2_N / 1_s);
	_test_screen->engine_r_thrust_io->reference								<< xf::ConstantSource (4.1_N);
	_test_screen->engine_r_thrust_io->target								<< xf::ConstantSource (3.9_N);
	_test_screen->engine_r_thrust_io->automatic								<< test_generator_io->create_property<si::Force> ("engine/right/thrust/automatic", 2_N, { 1.5_N, 2.5_N }, 0.1_N / 1_s);

	_test_screen->engine_r_speed_io->value									<< test_generator_io->create_property<si::AngularVelocity> ("engine/right/speed", 0.0_rpm, { -100_rpm, 15'000_rpm }, 1200_rpm / 1_s);

	_test_screen->engine_r_temperature_io->value							<< test_generator_io->create_property<si::Temperature> ("engine/right/temperature", 0_degC, { -20_degC, 75_degC }, 5_K / 1_s);

	_test_screen->engine_r_power_io->value									<< test_generator_io->create_property<si::Power> ("engine/right/power", 0_W, { 0_W, 295_W }, 10_W / 1_s);

	_test_screen->engine_r_current_io->value								<< test_generator_io->create_property<si::Current> ("engine/right/current", 0_A, { -5_A, 40_A }, 5_A / 1_s);

	_test_screen->gear_io->requested_down									<< xf::ConstantSource (true);
	_test_screen->gear_io->nose_up											<< xf::ConstantSource (false);
	_test_screen->gear_io->nose_down										<< xf::ConstantSource (true);
	_test_screen->gear_io->left_up											<< xf::ConstantSource (false);
	_test_screen->gear_io->left_down										<< xf::ConstantSource (true);
	_test_screen->gear_io->right_up											<< xf::ConstantSource (false);
	_test_screen->gear_io->right_down										<< xf::ConstantSource (true);

	_test_generator.emplace (std::move (test_generator_io), "test generator");

	_test_screen->create_instruments();

	// Register all instruments in the processing loop:
	for (auto& disclosure: _test_screen->instrument_tracker())
		register_module (disclosure.registrant());

	// Register the rest:
	register_module (*_test_generator);

	auto configurator_widget = new xf::ConfiguratorWidget (*machine, nullptr);
	configurator_widget->show();

	start();
}

