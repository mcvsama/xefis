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
#include <thread>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/xefis_machine.h>
#include <xefis/modules/systems/afcs_api.h>

// Local:
#include "test_instruments_machine.h"


TestInstrumentsMachine::TestInstrumentsMachine (xf::Xefis& xefis):
	Machine (xefis),
	_logger (xefis.logger())
{
	_navaid_storage = std::make_unique<xf::NavaidStorage> (_logger);
	_work_performer = std::make_unique<xf::WorkPerformer> (std::thread::hardware_concurrency(), _logger);

	_test_loop.emplace (*this, "Main loop", 120_Hz, _logger.with_scope ("TestLoop")),
	register_processing_loop (*_test_loop);

	auto line_width = 0.3525_mm;
	auto font_height = 3.15_mm;
	xf::ScreenSpec spec { QRect { 0, 0, 1366, 768 }, 15_in, 30_Hz, line_width, font_height };
	spec.set_scale (1.0f);
	auto& test_screen = _test_screen.emplace (spec, xefis.graphics(), *_navaid_storage, *this, _logger.with_scope ("test screen"));
	test_screen->set_paint_bounding_boxes (false);
	register_screen (*_test_screen);

	auto test_generator_io = std::make_unique<TestGeneratorIO>();

	// IO:
	test_screen->adi_io->weight_on_wheels									<< test_generator_io->create_enum_property<bool> ("adi/weight-on-wheels", { { true, 3_s }, { xf::nil, 2_s }, { false, 5_s } });
	test_screen->adi_io->speed_ias_serviceable								<< test_generator_io->create_enum_property<bool> ("adi/speed/ias.serviceable", { { true, 10_s }, { false, 2_s } });
	test_screen->adi_io->speed_ias											<< test_generator_io->create_property<si::Velocity> ("adi/speed/ias", 0_kt, { 0_kt, 300_kt }, 10_kt / 1_s);
	test_screen->adi_io->speed_ias_lookahead								<< test_generator_io->create_property<si::Velocity> ("adi/speed/ias.lookahead", 25_kt, { 0_kt, 300_kt }, 8_kt / 1_s);
	test_screen->adi_io->speed_ias_minimum									<< test_generator_io->create_property<si::Velocity> ("adi/speed/ias.minimum", 60_kt, { 50_kt, 70_kt }, 3_kt / 1_s);
	test_screen->adi_io->speed_ias_minimum_maneuver							<< test_generator_io->create_property<si::Velocity> ("adi/speed/ias.minimum.maneuver", 65_kt, { 55_kt, 72_kt }, 3_kt / 1_s);
	test_screen->adi_io->speed_ias_maximum_maneuver							<< test_generator_io->create_property<si::Velocity> ("adi/speed/ias.maximum.maneuver", 245_kt, { 238_kt, 245_kt }, 3_kt / 1_s);
	test_screen->adi_io->speed_ias_maximum									<< test_generator_io->create_property<si::Velocity> ("adi/speed/ias.maximum", 250_kt, { 240_kt, 260_kt }, 3_kt / 1_s);
	test_screen->adi_io->speed_mach											<< test_generator_io->create_property<double> ("adi/speed/mach", 0.0f, { 0.0f, 0.85f }, 0.025f / 1_s);
	test_screen->adi_io->speed_ground										<< test_generator_io->create_property<si::Velocity> ("adi/speed/ground-speed", 0_kt, { 0_kt, 400_kt }, 13_kt / 1_s);
	test_screen->adi_io->speed_v1											<< test_generator_io->create_property<si::Velocity> ("adi/speed-bugs/v1", 80_kt, { 78_kt, 82_kt }, 1_kt / 1_s);
	test_screen->adi_io->speed_vr											<< test_generator_io->create_property<si::Velocity> ("adi/speed-bugs/vr", 88_kt, { 86_kt, 89_kt }, 1_kt / 1_s);
	test_screen->adi_io->speed_vref											<< test_generator_io->create_property<si::Velocity> ("adi/speed-bugs/vref", 95_kt, { 94_kt, 96_kt }, 0.1_kt / 1_s);
	test_screen->adi_io->speed_flaps_up_label								<< xf::ConstantSource<std::string> ("UP");
	test_screen->adi_io->speed_flaps_up_speed								<< xf::ConstantSource (140_kt);
	test_screen->adi_io->speed_flaps_a_label								<< xf::ConstantSource<std::string> ("1");
	test_screen->adi_io->speed_flaps_a_speed								<< xf::ConstantSource (120_kt);
	test_screen->adi_io->speed_flaps_b_label								<< xf::ConstantSource<std::string> ("5");
	test_screen->adi_io->speed_flaps_b_speed								<< xf::ConstantSource (110_kt);
	test_screen->adi_io->orientation_serviceable							<< test_generator_io->create_enum_property<bool> ("adi/orientation/serviceable", { { true, 11.5_s }, { false, 2_s } });
	test_screen->adi_io->orientation_pitch									<< test_generator_io->create_property<si::Angle> ("adi/orientation/pitch", 0_deg, { -90_deg, 90_deg }, 8_deg / 1_s);
	test_screen->adi_io->orientation_roll									<< test_generator_io->create_property<si::Angle> ("adi/orientation/roll", 0_deg, { -180_deg, +180_deg }, 1.5_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->adi_io->orientation_heading_magnetic						<< test_generator_io->create_property<si::Angle> ("adi/orientation/heading.magnetic", 0_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->adi_io->orientation_heading_true							<< test_generator_io->create_property<si::Angle> ("adi/orientation/heading.true", 10_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->adi_io->orientation_heading_numbers_visible				<< xf::ConstantSource (true);
	test_screen->adi_io->track_lateral_magnetic								<< test_generator_io->create_property<si::Angle> ("adi/track/lateral.magnetic", 9_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->adi_io->track_lateral_true									<< test_generator_io->create_property<si::Angle> ("adi/track/lateral.true", 19_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->adi_io->track_vertical										<< test_generator_io->create_property<si::Angle> ("adi/track/vertical", 0_deg, { -13_deg, 13_deg }, 1_deg / 1_s);
	test_screen->adi_io->fpv_visible										<< xf::ConstantSource (true);
	test_screen->adi_io->slip_skid											<< test_generator_io->create_property<si::Angle> ("adi/slip-skid/angle", 0_deg, { -5_deg, 5_deg }, 0.5_deg / 1_s);
	test_screen->adi_io->aoa_alpha											<< test_generator_io->create_property<si::Angle> ("adi/aoa/alpha", 0_deg, { -7_deg, 15_deg }, 1_deg / 1_s);
	test_screen->adi_io->aoa_alpha_maximum									<< test_generator_io->create_property<si::Angle> ("adi/aoa/alpha.maximum", 13_deg, { 13_deg, 15_deg }, 0.25_deg / 1_s);
	test_screen->adi_io->aoa_alpha_visible									<< xf::ConstantSource (true);
	test_screen->adi_io->altitude_amsl_serviceable							<< test_generator_io->create_enum_property<bool> ("adi/altitude/amsl.serviceable", { { true, 15_s }, { false, 2_s } });
	test_screen->adi_io->altitude_amsl										<< test_generator_io->create_property<si::Length> ("adi/altitude/amsl", -200_ft, { -200_ft, 2000_ft }, 2000_ft / 1_min);
	test_screen->adi_io->altitude_amsl_lookahead							<< test_generator_io->create_property<si::Length> ("adi/altitude/amsl.lookahead", 10_ft, { 0_ft, 2000_ft }, 100_ft / 1_min);
	test_screen->adi_io->altitude_agl_serviceable							<< test_generator_io->create_enum_property<bool> ("adi/altitude/agl.serviceable", { { true, 16_s }, { false, 2_s } });
	test_screen->adi_io->altitude_agl										<< test_generator_io->create_property<si::Length> ("adi/altitude/agl", -4_ft, { -4_ft, 30_m }, 100_ft / 1_min);
	test_screen->adi_io->decision_height_type								<< test_generator_io->create_enum_property<std::string> ("adi/decision-height/type", { { "BARO", 5_s }, { "RADIO", 4_s } });
	test_screen->adi_io->decision_height_setting							<< xf::ConstantSource (300_ft);
	test_screen->adi_io->decision_height_amsl								<< xf::ConstantSource (300_ft);
	test_screen->adi_io->landing_amsl										<< xf::ConstantSource (140_ft);
	test_screen->adi_io->vertical_speed_serviceable							<< test_generator_io->create_enum_property<bool> ("adi/vertical-speed/serviceable", { { true, 8_s }, { false, 2_s } });
	test_screen->adi_io->vertical_speed										<< test_generator_io->create_property<si::Velocity> ("adi/vertical-speed/speed", 0_fpm, { -6000_fpm, +6000_fpm }, 100_fpm / 1_s);
	test_screen->adi_io->vertical_speed_energy_variometer					<< test_generator_io->create_property<si::Power> ("adi/vertical-speed/energy-variometer", 0_W, { -1000_W, +1000_W }, 100_W / 1_s);
	test_screen->adi_io->pressure_qnh										<< xf::ConstantSource (1013_hPa);
	test_screen->adi_io->pressure_display_hpa								<< test_generator_io->create_enum_property<bool> ("adi/pressure/display-hpa", { { true, 8_s }, { false, 8_s } });
	test_screen->adi_io->pressure_use_std									<< test_generator_io->create_enum_property<bool> ("adi/pressure/use-std", { { true, 4_s }, { false, 4_s } });
	test_screen->adi_io->flight_director_serviceable						<< test_generator_io->create_enum_property<bool> ("adi/flight-director/serviceable", { { true, 13_s }, { false, 2_s } });
	test_screen->adi_io->flight_director_active_name						<< test_generator_io->create_enum_property<std::string> ("adi/flight-director/active-name", { { "L", 3_s }, { "R", 3_s }, { "", 2_s } });
	test_screen->adi_io->flight_director_cmd_visible						<< xf::ConstantSource (true);
	test_screen->adi_io->flight_director_cmd_altitude						<< xf::ConstantSource (1000_ft);
	test_screen->adi_io->flight_director_cmd_altitude_acquired				<< xf::ConstantSource (false);
	test_screen->adi_io->flight_director_cmd_ias							<< xf::ConstantSource (100_kt);
	test_screen->adi_io->flight_director_cmd_mach							<< xf::ConstantSource (0.34);
	test_screen->adi_io->flight_director_cmd_vertical_speed					<< xf::ConstantSource (1500_fpm);
	test_screen->adi_io->flight_director_cmd_fpa							<< xf::ConstantSource (5_deg);
	test_screen->adi_io->flight_director_guidance_visible					<< xf::ConstantSource (true);
	test_screen->adi_io->flight_director_guidance_pitch						<< xf::ConstantSource (2.5_deg);
	test_screen->adi_io->flight_director_guidance_roll						<< xf::ConstantSource (0_deg);
	test_screen->adi_io->control_surfaces_visible							<< xf::ConstantSource (true);
	test_screen->adi_io->control_surfaces_elevator							<< test_generator_io->create_property<double> ("adi/control-surfaces/elevator", 0.0f, { -1.0f, +1.0f }, 0.1f / 1_s);
	test_screen->adi_io->control_surfaces_ailerons							<< test_generator_io->create_property<double> ("adi/control-surfaces/ailerons", 0.0f, { -1.0f, +1.0f }, 0.3f / 1_s);
	test_screen->adi_io->navaid_reference_visible							<< xf::ConstantSource (true);
	test_screen->adi_io->navaid_course_magnetic								<< xf::ConstantSource (150_deg);
	test_screen->adi_io->navaid_type_hint									<< xf::ConstantSource ("VOR");
	test_screen->adi_io->navaid_identifier									<< xf::ConstantSource ("WRO");
	test_screen->adi_io->navaid_distance									<< xf::ConstantSource (1.5_nmi);
	test_screen->adi_io->flight_path_deviation_lateral_serviceable			<< test_generator_io->create_enum_property<bool> ("adi/flight-path-deviation/lateral/serviceable", { { true, 9.5_s }, { false, 2_s } });
	test_screen->adi_io->flight_path_deviation_lateral_approach				<< test_generator_io->create_property<si::Angle> ("adi/flight-path-deviation/lateral/approach", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	test_screen->adi_io->flight_path_deviation_lateral_flight_path			<< test_generator_io->create_property<si::Angle> ("adi/flight-path-deviation/lateral/flight-path", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	test_screen->adi_io->flight_path_deviation_vertical_serviceable			<< test_generator_io->create_enum_property<bool> ("adi/flight-path-deviation/vertical/serviceable", { { true, 13.4_s }, { false, 2_s } });
	test_screen->adi_io->flight_path_deviation_vertical						<< test_generator_io->create_property<si::Angle> ("adi/flight-path-deviation/vertical/deviation", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	test_screen->adi_io->flight_path_deviation_vertical_approach			<< test_generator_io->create_property<si::Angle> ("adi/flight-path-deviation/vertical/approach", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	test_screen->adi_io->flight_path_deviation_vertical_flight_path			<< test_generator_io->create_property<si::Angle> ("adi/flight-path-deviation/vertical/flight-path", 0_deg, { -5_deg, 5_deg }, 3_deg / 1_s);
	test_screen->adi_io->flight_path_deviation_mixed_mode					<< xf::ConstantSource (true);
	test_screen->adi_io->flight_mode_hint_visible							<< xf::ConstantSource (true);
	test_screen->adi_io->flight_mode_hint									<< test_generator_io->create_enum_property<std::string> ("adi/fma/hint", { { "F/D", 11_s }, { "CMD", 15_s } });
	test_screen->adi_io->flight_mode_fma_visible							<< xf::ConstantSource (true);
	test_screen->adi_io->flight_mode_fma_speed_hint							<< test_generator_io->create_enum_property<std::string> ("adi/fma/speed-hint", { { std::string (afcs::kThrustMode_TO_GA), 15_s }, { std::string (afcs::kThrustMode_Continuous), 15_s } });
	test_screen->adi_io->flight_mode_fma_speed_armed_hint					<< test_generator_io->create_enum_property<std::string> ("adi/fma/speed-armed-hint", { { std::string (afcs::kSpeedMode_Airspeed), 17_s }, { std::string (afcs::kSpeedMode_Thrust), 17_s } });
	test_screen->adi_io->flight_mode_fma_lateral_hint						<< test_generator_io->create_enum_property<std::string> ("adi/fma/lateral-hint", { { std::string (afcs::kRollMode_Track), 12_s }, { std::string (afcs::kRollMode_WingsLevel), 12_s }, { std::string (afcs::kRollMode_LNAV), 15_s }, { std::string (afcs::kRollMode_Localizer), 12_s } });
	test_screen->adi_io->flight_mode_fma_lateral_armed_hint					<< test_generator_io->create_enum_property<std::string> ("adi/fma/lateral-armed-hint", { { std::string (afcs::kRollMode_Track), 13_s }, { std::string (afcs::kRollMode_Heading), 13_s } });
	test_screen->adi_io->flight_mode_fma_vertical_hint						<< test_generator_io->create_enum_property<std::string> ("adi/fma/vertical-hint", { { std::string (afcs::kPitchMode_Altitude), 11_s }, { std::string (afcs::kPitchMode_TO_GA), 17_s } });
	test_screen->adi_io->flight_mode_fma_vertical_armed_hint				<< test_generator_io->create_enum_property<std::string> ("adi/fma/vertical-armed-hint", { { std::string (afcs::kPitchMode_GS), 14_s }, { std::string (afcs::kPitchMode_VNAVPath), 14_s } });
	test_screen->adi_io->tcas_resolution_advisory_pitch_minimum				<< xf::ConstantSource (-45_deg);
	test_screen->adi_io->tcas_resolution_advisory_pitch_maximum				<< xf::ConstantSource (80_deg);
	test_screen->adi_io->tcas_resolution_advisory_vertical_speed_minimum	<< xf::ConstantSource (-3000_fpm);
	test_screen->adi_io->tcas_resolution_advisory_vertical_speed_maximum	<< xf::ConstantSource (10000_fpm);
	test_screen->adi_io->warning_novspd_flag								<< test_generator_io->create_enum_property<bool> ("adi/flags/novspd", { { false, 3_s }, { true, 2_s } });
	test_screen->adi_io->warning_ldgalt_flag								<< test_generator_io->create_enum_property<bool> ("adi/flags/ldgalt", { { false, 7_s }, { true, 2_s } });
	test_screen->adi_io->warning_pitch_disagree								<< test_generator_io->create_enum_property<bool> ("adi/flags/pitch-disagree", { { false, 5_s }, { true, 2_s } });
	test_screen->adi_io->warning_roll_disagree								<< test_generator_io->create_enum_property<bool> ("adi/flags/roll-disagree", { { false, 4_s }, { true, 2_s } });
	test_screen->adi_io->warning_ias_disagree								<< test_generator_io->create_enum_property<bool> ("adi/flags/ias-disagree", { { false, 9_s }, { true, 2_s } });
	test_screen->adi_io->warning_altitude_disagree							<< test_generator_io->create_enum_property<bool> ("adi/flags/altitude-disagree", { { false, 8_s }, { true, 2_s } });
	test_screen->adi_io->warning_roll										<< test_generator_io->create_enum_property<bool> ("adi/flags/roll", { { false, 11_s }, { true, 2_s } });
	test_screen->adi_io->warning_slip_skid									<< test_generator_io->create_enum_property<bool> ("adi/flags/slip-skid", { { false, 7.5_s }, { true, 2_s } });
	test_screen->adi_io->style_old											<< xf::ConstantSource (false);
	test_screen->adi_io->style_show_metric									<< xf::ConstantSource (true);

	test_screen->hsi_io->display_mode										<< xf::ConstantSource (hsi::DisplayMode::Auxiliary);
	test_screen->hsi_io->range												<< test_generator_io->create_enum_property<si::Length> ("hsi/range", { { 20_nmi, 3_s }, { 40_nmi, 3_s }, { 80_nmi, 3_s }, { 160_nmi, 3_s } });
	test_screen->hsi_io->speed_gs											<< test_generator_io->create_property<si::Velocity> ("hsi/speed/ground-speed", 0_kt, { 0_kt, 400_kt }, 13_kt / 1_s);
	test_screen->hsi_io->speed_tas											<< test_generator_io->create_property<si::Velocity> ("hsi/speed/true-airspeed", 0_kt, { 0_kt, 400_kt }, 17_kt / 1_s);
	test_screen->hsi_io->cmd_visible										<< xf::ConstantSource (true);
	test_screen->hsi_io->cmd_line_visible									<< xf::ConstantSource (true);
	test_screen->hsi_io->cmd_heading_magnetic								<< xf::ConstantSource (90_deg);
	test_screen->hsi_io->cmd_track_magnetic									<< xf::ConstantSource (95_deg);
	test_screen->hsi_io->cmd_use_trk										<< xf::ConstantSource (true);
	test_screen->hsi_io->target_altitude_reach_distance						<< test_generator_io->create_property<si::Length> ("hsi/target-altitude-reach-distance", 12_nmi, { 12_nmi, 15_nmi }, 0.5_nmi / 1_s);
	test_screen->hsi_io->orientation_heading_magnetic						<< test_generator_io->create_property<si::Angle> ("hsi/orientation/heading.magnetic", 0_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->hsi_io->orientation_heading_true							<< test_generator_io->create_property<si::Angle> ("hsi/orientation/heading.true", 10_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->hsi_io->heading_mode										<< xf::ConstantSource (hsi::HeadingMode::Magnetic);
	test_screen->hsi_io->home_true_direction								<< test_generator_io->create_property<si::Angle> ("hsi/home/true-direction", 0_deg, { 0_deg, 360_deg }, 5_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->hsi_io->home_track_visible									<< xf::ConstantSource (true);
	test_screen->hsi_io->home_distance_vlos									<< test_generator_io->create_property<si::Length> ("hsi/home/distance/vlos", 0_m, { 0_m, 30_km }, 150_m / 1_s);
	test_screen->hsi_io->home_distance_ground								<< test_generator_io->create_property<si::Length> ("hsi/home/distance/ground", 0_m, { 0_m, 20_km }, 100_m / 1_s);
	test_screen->hsi_io->home_distance_vertical								<< test_generator_io->create_property<si::Length> ("hsi/home/distance/vertical", 0_m, { 0_m, 5_km }, 25_m / 1_s);
	test_screen->hsi_io->home_position_longitude							<< xf::ConstantSource (0_deg);
	test_screen->hsi_io->home_position_latitude								<< xf::ConstantSource (0_deg);
	test_screen->hsi_io->position_longitude									<< test_generator_io->create_property<si::Angle> ("hsi/position/longitude", 51.9_deg, { 51.9_deg, 60_deg }, 0.001_deg / 1_s);
	test_screen->hsi_io->position_latitude									<< test_generator_io->create_property<si::Angle> ("hsi/position/latitude", 19.14_deg, { 19.14_deg, 20.14_deg }, 0.001_deg / 1_s);
	test_screen->hsi_io->position_source									<< xf::ConstantSource ("GPS");
	test_screen->hsi_io->track_visible										<< xf::ConstantSource (true);
	test_screen->hsi_io->track_lateral_magnetic								<< test_generator_io->create_property<si::Angle> ("hsi/orientation/heading.magnetic", -5_deg, { -5_deg, 355_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	test_screen->hsi_io->track_lateral_rotation								<< xf::ConstantSource (-1_deg / 1_s);
	test_screen->hsi_io->track_center_on_track								<< xf::ConstantSource (true);
	test_screen->hsi_io->course_visible										<< test_generator_io->create_enum_property<bool> ("hsi/course/visible", { { true, 16_s }, { false, 2_s } });
	test_screen->hsi_io->course_setting_magnetic							<< test_generator_io->create_property<si::Angle> ("hsi/course/setting.magnetic", 0_deg, { 0_deg, 720_deg }, 20_deg / 1_s);
	test_screen->hsi_io->course_deviation									<< test_generator_io->create_property<si::Angle> ("hsi/course/deviation", 0_deg, { -10_deg, +10_deg }, 1_deg / 1_s);
	test_screen->hsi_io->course_to_flag										<< test_generator_io->create_enum_property<bool> ("hsi/course/to-flag", { { true, 7_s }, { false, 3_s } });
	test_screen->hsi_io->navaid_selected_reference							<< xf::ConstantSource ("REF");
	test_screen->hsi_io->navaid_selected_identifier							<< xf::ConstantSource ("IDENT");
	test_screen->hsi_io->navaid_selected_distance							<< test_generator_io->create_property<si::Length> ("hsi/navaid/selected/distance", 0_nmi, { 0_nmi, 5_nmi }, 0.15_nmi / 1_s);
	test_screen->hsi_io->navaid_selected_eta								<< test_generator_io->create_property<si::Time> ("hsi/navaid/selected/eta", 0_s, { 300_s, 0_s }, 1_s / 1_s);
	test_screen->hsi_io->navaid_selected_course_magnetic					<< test_generator_io->create_property<si::Angle> ("hsi/navaid/selected/course-magnetic", 27_deg, { 23_deg, 31_deg },0.5_deg / 1_s);
	test_screen->hsi_io->navaid_left_type									<< xf::ConstantSource (hsi::NavType::A);
	test_screen->hsi_io->navaid_left_reference								<< xf::ConstantSource ("LREF");
	test_screen->hsi_io->navaid_left_identifier								<< xf::ConstantSource ("LIDENT");
	test_screen->hsi_io->navaid_left_distance								<< test_generator_io->create_property<si::Length> ("hsi/navaid/left/distance", 0_nmi, { 0_nmi, 5_nmi }, 0.1_nmi / 1_s);
	test_screen->hsi_io->navaid_left_initial_bearing_magnetic				<< test_generator_io->create_property<si::Angle> ("hsi/navaid/left/initial-bearing-magnetic", 30_deg, { 28_deg, 32_deg }, 0.25_deg / 1_s);
	test_screen->hsi_io->navaid_right_type									<< xf::ConstantSource (hsi::NavType::B);
	test_screen->hsi_io->navaid_right_reference								<< xf::ConstantSource ("RREF");
	test_screen->hsi_io->navaid_right_identifier							<< xf::ConstantSource ("RIDENT");
	test_screen->hsi_io->navaid_right_distance								<< test_generator_io->create_property<si::Length> ("hsi/navaid/right/distance", 100_nmi, { 100_nmi, 105_nmi }, 0.1_nmi / 1_s);
	test_screen->hsi_io->navaid_right_initial_bearing_magnetic				<< test_generator_io->create_property<si::Angle> ("hsi/navaid/right/initial-bearing-magnetic", 80_deg, { 78_deg, 82_deg }, 0.25_deg / 1_s);
	test_screen->hsi_io->navigation_required_performance					<< xf::ConstantSource (4_m);
	test_screen->hsi_io->navigation_actual_performance						<< xf::ConstantSource (1.2_m);
	test_screen->hsi_io->wind_from_magnetic									<< test_generator_io->create_property<si::Angle> ("hsi/wind/from-magnetic", 100_deg, { 0_deg, 360_deg }, 2_deg / 1_s);
	test_screen->hsi_io->wind_speed_tas										<< test_generator_io->create_property<si::Velocity> ("hsi/wind/speed-tas", 12_kt, { 10_kt, 15_kt }, 0.1_kt / 1_s);
	test_screen->hsi_io->localizer_id										<< xf::ConstantSource ("LOCID");
	test_screen->hsi_io->tcas_on											<< test_generator_io->create_enum_property<bool> ("hsi/tcas/on", { { true, 5_s }, { false, 3_s } });
	test_screen->hsi_io->tcas_range											<< test_generator_io->create_enum_property<si::Length> ("hsi/tcas/range", { { 3_nmi, 2_s }, { 6_nmi, 2_s }, { 9_nmi, 2_s }, { 12_nmi, 2_s } });
	test_screen->hsi_io->features_fix										<< test_generator_io->create_enum_property<bool> ("hsi/features/fix", { { true, 3_s }, { true, 10_s }, { false, 1_s } });
	test_screen->hsi_io->features_vor										<< test_generator_io->create_enum_property<bool> ("hsi/features/vor", { { true, 5_s }, { true, 10_s }, { false, 1_s } });
	test_screen->hsi_io->features_dme										<< test_generator_io->create_enum_property<bool> ("hsi/features/dme", { { true, 7_s }, { true, 10_s }, { false, 1_s } });
	test_screen->hsi_io->features_ndb										<< test_generator_io->create_enum_property<bool> ("hsi/features/ndb", { { true, 9_s }, { true, 10_s }, { false, 1_s } });
	test_screen->hsi_io->features_loc										<< test_generator_io->create_enum_property<bool> ("hsi/features/loc", { { true, 11_s }, { true, 10_s }, { false, 1_s } });
	test_screen->hsi_io->features_arpt										<< test_generator_io->create_enum_property<bool> ("hsi/features/arpt", { { true, 13_s }, { true, 10_s }, { false, 1_s } });

	test_screen->engine_l_thrust_io->value									<< test_generator_io->create_property<si::Force> ("engine/left/thrust", 0_N, { -0.3_N, 4.5_N }, 0.2_N / 1_s);
	test_screen->engine_l_thrust_io->reference								<< xf::ConstantSource (4.1_N);
	test_screen->engine_l_thrust_io->target									<< xf::ConstantSource (3.9_N);
	test_screen->engine_l_thrust_io->automatic								<< test_generator_io->create_property<si::Force> ("engine/left/thrust/automatic", 2_N, { 1.5_N, 2.5_N }, 0.1_N / 1_s);

	test_screen->engine_l_speed_io->value									<< test_generator_io->create_property<si::AngularVelocity> ("engine/left/speed", 0.0_rpm, { -100_rpm, 15'000_rpm }, 1200_rpm / 1_s);

	test_screen->engine_l_temperature_io->value								<< test_generator_io->create_property<si::Temperature> ("engine/left/temperature", 0_degC, { -20_degC, 75_degC }, 5_K / 1_s);

	test_screen->engine_l_power_io->value									<< test_generator_io->create_property<si::Power> ("engine/left/power", 0_W, { 0_W, 295_W }, 11_W / 1_s);

	test_screen->engine_l_current_io->value									<< test_generator_io->create_property<si::Current> ("engine/left/current", 0_A, { -5_A, 40_A }, 5_A / 1_s);

	test_screen->engine_l_voltage_io->value									<< test_generator_io->create_property<si::Voltage> ("engine/left/voltage", 16.8_V, { 11.1_V, 16.8_V }, 0.07_V / 1_s);

	test_screen->engine_l_vibration_io->value								<< test_generator_io->create_property<si::Acceleration> ("engine/left/vibration", 0.1_g, { 0.1_g, 1.2_g }, 0.025_g / 1_s);

	test_screen->engine_r_thrust_io->value									<< test_generator_io->create_property<si::Force> ("engine/right/thrust", 0_N, { -0.3_N, 4.5_N }, 0.2_N / 1_s);
	test_screen->engine_r_thrust_io->reference								<< xf::ConstantSource (4.1_N);
	test_screen->engine_r_thrust_io->target									<< xf::ConstantSource (3.9_N);
	test_screen->engine_r_thrust_io->automatic								<< test_generator_io->create_property<si::Force> ("engine/right/thrust/automatic", 2_N, { 1.5_N, 2.5_N }, 0.1_N / 1_s);

	test_screen->engine_r_speed_io->value									<< test_generator_io->create_property<si::AngularVelocity> ("engine/right/speed", 0.0_rpm, { -100_rpm, 15'000_rpm }, 1200_rpm / 1_s);

	test_screen->engine_r_temperature_io->value								<< test_generator_io->create_property<si::Temperature> ("engine/right/temperature", 0_degC, { -20_degC, 75_degC }, 5_K / 1_s);

	test_screen->engine_r_power_io->value									<< test_generator_io->create_property<si::Power> ("engine/right/power", 0_W, { 0_W, 295_W }, 10_W / 1_s);

	test_screen->engine_r_current_io->value									<< test_generator_io->create_property<si::Current> ("engine/right/current", 0_A, { -5_A, 40_A }, 5_A / 1_s);

	test_screen->engine_r_voltage_io->value									<< test_generator_io->create_property<si::Voltage> ("engine/right/voltage", 16.8_V, { 11.1_V, 16.8_V }, 0.073_V / 1_s);

	test_screen->engine_r_vibration_io->value								<< test_generator_io->create_property<si::Acceleration> ("engine/right/vibration", 0.1_g, { 0.1_g, 1.2_g }, 0.025_g / 1_s);
																			// TODO occasional nil as value in ->created_property

	test_screen->gear_io->requested_down									<< xf::ConstantSource (true);
	test_screen->gear_io->nose_up											<< xf::ConstantSource (false);
	test_screen->gear_io->nose_down											<< xf::ConstantSource (true);
	test_screen->gear_io->left_up											<< xf::ConstantSource (false);
	test_screen->gear_io->left_down											<< xf::ConstantSource (true);
	test_screen->gear_io->right_up											<< xf::ConstantSource (false);
	test_screen->gear_io->right_down										<< xf::ConstantSource (true);

	test_screen->vertical_trim_io->trim_value								<< test_generator_io->create_property<double> ("vertical-trim", 0.0, { 0.0, 1.0 }, 0.1 / 1_s);
	test_screen->vertical_trim_io->trim_reference							<< xf::ConstantSource (0.5);
	test_screen->vertical_trim_io->trim_reference_minimum					<< xf::ConstantSource (0.35);
	test_screen->vertical_trim_io->trim_reference_maximum					<< xf::ConstantSource (0.6);

	_test_generator.emplace (std::move (test_generator_io), "test generator");
	test_screen->create_instruments();

	auto& test_loop = **_test_loop;

	// Register all instruments in the processing loop:
	for (auto& disclosure: test_screen->instrument_tracker())
		test_loop.register_module (disclosure.registrant());

	// Register the rest:
	test_loop.register_module (*_test_generator);
	test_loop.start();

	test_screen->show();
}


std::unique_ptr<xf::Machine>
xefis_machine (xf::Xefis& xefis)
{
	return std::make_unique<TestInstrumentsMachine> (xefis);
}

