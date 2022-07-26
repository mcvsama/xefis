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
#include "test_instruments_machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/xefis_machine.h>
#include <xefis/modules/systems/afcs_api.h>

// Standard:
#include <cstddef>
#include <thread>


TestInstrumentsMachine::TestInstrumentsMachine (xf::Xefis& xefis):
	Machine (xefis),
	_logger (xefis.logger())
{
	auto const angle_to_force = [](si::Angle const angle) {
		return angle / 1_rad * 1_N;
	};

	auto const force_to_angle = [](si::Force const force) {
		return force / 1_N * 1_rad;
	};

	_work_performer = std::make_unique<xf::WorkPerformer> (std::thread::hardware_concurrency(), _logger);

	_navaid_storage = std::make_unique<xf::NavaidStorage> (_logger, "share/nav/nav.dat.gz", "share/nav/fix.dat.gz", "share/nav/apt.dat.gz");
	_work_performer->submit (_navaid_storage->async_loader());

	auto line_width = 0.3525_mm;
	auto font_height = 3.15_mm;
	xf::ScreenSpec spec { QRect { 0, 0, 1366, 768 }, 15_in, 30_Hz, line_width, font_height };
	spec.set_scale (1.25f);

	auto& test_screen_1 = _test_screen_1.emplace (spec, xefis.graphics(), *_navaid_storage, *this, _logger.with_scope ("test screen"));
	test_screen_1->set_paint_bounding_boxes (false);

	auto& test_screen_2 = _test_screen_2.emplace (spec, xefis.graphics(), *_navaid_storage, *this, _logger.with_scope ("test screen"));
	test_screen_2->set_paint_bounding_boxes (false);

	auto& test_generator_registrant = _test_generator.emplace ("test generator");
	auto& test_generator = *test_generator_registrant;
	auto& test_generator_hsi_range											= test_generator.create_enum_socket<si::Length> ("hsi/range", { { 5_nmi, 10_s }, { 20_nmi, 10_s }, { 40_nmi, 4_s }, { 80_nmi, 2_s }, { 160_nmi, 2_s } });
	auto& test_generator_hsi_speed_gs										= test_generator.create_socket<si::Velocity> ("hsi/speed/ground-speed", 0_kt, { 0_kt, 400_kt }, 13_kt / 1_s);
	auto& test_generator_hsi_speed_tas										= test_generator.create_socket<si::Velocity> ("hsi/speed/true-airspeed", 0_kt, { 0_kt, 400_kt }, 17_kt / 1_s);
	auto  test_generator_hsi_cmd_visible									= true;
	auto  test_generator_hsi_cmd_line_visible								= true;
	auto  test_generator_hsi_cmd_heading_magnetic							= 90_deg;
	auto  test_generator_hsi_cmd_track_magnetic								= 95_deg;
	auto  test_generator_hsi_cmd_use_trk									= true;
	auto& test_generator_hsi_target_altitude_reach_distance					= test_generator.create_socket<si::Length> ("hsi/target-altitude-reach-distance", 12_nmi, { 12_nmi, 15_nmi }, 0.5_nmi / 1_s);
	auto& test_generator_hsi_orientation_heading_magnetic					= test_generator.create_socket<si::Angle> ("hsi/orientation/heading.magnetic", 0_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	auto& test_generator_hsi_orientation_heading_true						= test_generator.create_socket<si::Angle> ("hsi/orientation/heading.true", 10_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	auto  test_generator_hsi_heading_mode									= hsi::HeadingMode::Magnetic;
	auto& test_generator_hsi_home_true_direction							= test_generator.create_socket<si::Angle> ("hsi/home/true-direction", 0_deg, { 0_deg, 360_deg }, 5_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	auto  test_generator_hsi_home_track_visible								= true;
	auto& test_generator_hsi_home_distance_vlos								= test_generator.create_socket<si::Length> ("hsi/home/distance/vlos", 0_m, { 0_m, 30_km }, 150_m / 1_s);
	auto& test_generator_hsi_home_distance_ground							= test_generator.create_socket<si::Length> ("hsi/home/distance/ground", 0_m, { 0_m, 20_km }, 100_m / 1_s);
	auto& test_generator_hsi_home_distance_vertical							= test_generator.create_socket<si::Length> ("hsi/home/distance/vertical", 0_m, { 0_m, 5_km }, 25_m / 1_s);
	auto  test_generator_hsi_home_position_longitude						= 51.9_deg;
	auto  test_generator_hsi_home_position_latitude							= 19.14_deg;
	auto& test_generator_hsi_position_longitude								= test_generator.create_socket<si::Angle> ("hsi/position/longitude", 51.9_deg, { 51.9_deg, 60_deg }, 0.001_deg / 1_s);
	auto& test_generator_hsi_position_latitude								= test_generator.create_socket<si::Angle> ("hsi/position/latitude", 19.14_deg, { 19.14_deg, 20.14_deg }, 0.001_deg / 1_s);
	auto  test_generator_hsi_position_source								= "GPS";
	auto& test_generator_hsi_range_warning_longitude						= test_generator.create_socket<si::Angle> ("hsi/range/warning/longitude", 51.9_deg, { 51.9_deg, 60_deg }, 0.002_deg / 1_s);
	auto& test_generator_hsi_range_warning_latitude							= test_generator.create_socket<si::Angle> ("hsi/range/warning/latitude", 19.14_deg, { 19.14_deg, 20.14_deg }, 0.002_deg / 1_s);
	auto& test_generator_hsi_range_warning_radius							= test_generator.create_socket<si::Length> ("hsi/range/warning/radius", 10_nmi, { 0_nmi, 10_nmi }, 0.1_nmi / 1_s);
	auto& test_generator_hsi_range_critical_longitude						= test_generator.create_socket<si::Angle> ("hsi/range/critical/longitude", 51.9_deg, { 51.9_deg, 60_deg }, 0.002_deg / 1_s);
	auto& test_generator_hsi_range_critical_latitude						= test_generator.create_socket<si::Angle> ("hsi/range/critical/latitude", 19.14_deg, { 19.14_deg, 20.14_deg }, 0.002_deg / 1_s);
	auto& test_generator_hsi_range_critical_radius							= test_generator.create_socket<si::Length> ("hsi/range/critical/radius", 14_nmi, { 0_nmi, 14_nmi }, 0.1_nmi / 1_s);
	auto  test_generator_hsi_track_visible									= true;
	auto& test_generator_hsi_track_lateral_magnetic							= test_generator.create_socket<si::Angle> ("hsi/orientation/heading.magnetic", -5_deg, { -5_deg, 355_deg }, 2_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	auto  test_generator_hsi_track_lateral_rotation							= -1_deg / 1_s;
	auto  test_generator_hsi_track_center_on_track							= true;
	auto& test_generator_hsi_course_visible									= test_generator.create_enum_socket<bool> ("hsi/course/visible", { { true, 16_s }, { false, 2_s } });
	auto& test_generator_hsi_course_setting_magnetic						= test_generator.create_socket<si::Angle> ("hsi/course/setting.magnetic", 0_deg, { 0_deg, 720_deg }, 20_deg / 1_s);
	auto& test_generator_hsi_course_deviation								= test_generator.create_socket<si::Angle> ("hsi/course/deviation", 0_deg, { -10_deg, +10_deg }, 1_deg / 1_s);
	auto& test_generator_hsi_course_to_flag									= test_generator.create_enum_socket<bool> ("hsi/course/to-flag", { { true, 7_s }, { false, 3_s } });
	auto  test_generator_hsi_navaid_selected_reference						= "REF";
	auto  test_generator_hsi_navaid_selected_identifier						= "IDENT";
	auto& test_generator_hsi_navaid_selected_distance						= test_generator.create_socket<si::Length> ("hsi/navaid/selected/distance", 0_nmi, { 0_nmi, 5_nmi }, 0.15_nmi / 1_s);
	auto& test_generator_hsi_navaid_selected_eta							= test_generator.create_socket<si::Time> ("hsi/navaid/selected/eta", 300_s, { 0_s, 300_s }, 1_s / 1_s);
	auto& test_generator_hsi_navaid_selected_course_magnetic				= test_generator.create_socket<si::Angle> ("hsi/navaid/selected/course-magnetic", 27_deg, { 23_deg, 31_deg },0.5_deg / 1_s);
	auto  test_generator_hsi_navaid_left_type								= hsi::NavType::A;
	auto  test_generator_hsi_navaid_left_reference							= "LREF";
	auto  test_generator_hsi_navaid_left_identifier							= "LIDENT";
	auto& test_generator_hsi_navaid_left_distance							= test_generator.create_socket<si::Length> ("hsi/navaid/left/distance", 0_nmi, { 0_nmi, 5_nmi }, 0.1_nmi / 1_s);
	auto& test_generator_hsi_navaid_left_initial_bearing_magnetic			= test_generator.create_socket<si::Angle> ("hsi/navaid/left/initial-bearing-magnetic", 30_deg, { 28_deg, 32_deg }, 0.25_deg / 1_s);
	auto  test_generator_hsi_navaid_right_type								= hsi::NavType::B;
	auto  test_generator_hsi_navaid_right_reference							= "RREF";
	auto  test_generator_hsi_navaid_right_identifier						= "RIDENT";
	auto& test_generator_hsi_navaid_right_distance							= test_generator.create_socket<si::Length> ("hsi/navaid/right/distance", 100_nmi, { 100_nmi, 105_nmi }, 0.1_nmi / 1_s);
	auto& test_generator_hsi_navaid_right_initial_bearing_magnetic			= test_generator.create_socket<si::Angle> ("hsi/navaid/right/initial-bearing-magnetic", 80_deg, { 78_deg, 82_deg }, 0.25_deg / 1_s);
	auto  test_generator_hsi_navigation_required_performance				= 4_m;
	auto  test_generator_hsi_navigation_actual_performance					= 1.2_m;
	auto& test_generator_hsi_wind_from_magnetic								= test_generator.create_socket<si::Angle> ("hsi/wind/from-magnetic", 100_deg, { 0_deg, 360_deg }, 2_deg / 1_s);
	auto& test_generator_hsi_wind_speed_tas									= test_generator.create_socket<si::Velocity> ("hsi/wind/speed-tas", 12_kt, { 10_kt, 15_kt }, 0.1_kt / 1_s);
	auto  test_generator_hsi_localizer_id									= "LOCID";
	auto& test_generator_hsi_tcas_on										= test_generator.create_enum_socket<bool> ("hsi/tcas/on", { { true, 5_s }, { false, 3_s } });
	auto& test_generator_hsi_tcas_range										= test_generator.create_enum_socket<si::Length> ("hsi/tcas/range", { { 3_nmi, 2_s }, { 6_nmi, 2_s }, { 9_nmi, 2_s }, { 12_nmi, 2_s } });
	auto& test_generator_hsi_features_fix									= test_generator.create_enum_socket<bool> ("hsi/features/fix", { { true, 3_s }, { true, 10_s }, { false, 1_s } });
	auto& test_generator_hsi_features_vor									= test_generator.create_enum_socket<bool> ("hsi/features/vor", { { true, 5_s }, { true, 10_s }, { false, 1_s } });
	auto& test_generator_hsi_features_dme									= test_generator.create_enum_socket<bool> ("hsi/features/dme", { { true, 7_s }, { true, 10_s }, { false, 1_s } });
	auto& test_generator_hsi_features_ndb									= test_generator.create_enum_socket<bool> ("hsi/features/ndb", { { true, 9_s }, { true, 10_s }, { false, 1_s } });
	auto& test_generator_hsi_features_loc									= test_generator.create_enum_socket<bool> ("hsi/features/loc", { { true, 11_s }, { true, 10_s }, { false, 1_s } });
	auto& test_generator_hsi_features_arpt									= test_generator.create_enum_socket<bool> ("hsi/features/arpt", { { true, 13_s }, { true, 10_s }, { false, 1_s } });
	auto& test_generator_hsi_radio_range_warning							= test_generator.create_socket<si::Length> ("hsi/radio-range/range.warning", 7_mi, { 6_mi, 8_mi }, 0.1_mi / 1_s);
	auto& test_generator_hsi_radio_range_critical							= test_generator.create_socket<si::Length> ("hsi/radio-range/range.critical", 10_mi, { 8_mi, 11_mi }, 0.1_mi / 1_s);

	// IO:
	test_screen_1->adi->weight_on_wheels									<< test_generator.create_enum_socket<bool> ("adi/weight-on-wheels", { { true, 3_s }, { xf::nil, 2_s }, { false, 5_s } });
	test_screen_1->adi->speed_ias
		<< test_generator.create_socket<si::Velocity> ("adi/speed/ias", 0_kt, { 0_kt, 300_kt }, 10_kt / 1_s, TestGenerator::BorderCondition::Mirroring, { .nil = 3_s, .not_nil = 7_s });
	test_screen_1->adi->speed_ias_lookahead									<< test_generator.create_socket<si::Velocity> ("adi/speed/ias.lookahead", 25_kt, { 0_kt, 300_kt }, 8_kt / 1_s);
	test_screen_1->adi->speed_ias_minimum									<< test_generator.create_socket<si::Velocity> ("adi/speed/ias.minimum", 60_kt, { 50_kt, 70_kt }, 3_kt / 1_s);
	test_screen_1->adi->speed_ias_minimum_maneuver							<< test_generator.create_socket<si::Velocity> ("adi/speed/ias.minimum.maneuver", 65_kt, { 55_kt, 72_kt }, 3_kt / 1_s);
	test_screen_1->adi->speed_ias_maximum_maneuver							<< test_generator.create_socket<si::Velocity> ("adi/speed/ias.maximum.maneuver", 245_kt, { 238_kt, 245_kt }, 3_kt / 1_s);
	test_screen_1->adi->speed_ias_maximum									<< test_generator.create_socket<si::Velocity> ("adi/speed/ias.maximum", 250_kt, { 240_kt, 260_kt }, 3_kt / 1_s);
	test_screen_1->adi->speed_mach											<< test_generator.create_socket<double> ("adi/speed/mach", 0.0f, { 0.0f, 0.85f }, 0.025f / 1_s);
	test_screen_1->adi->speed_ground										<< test_generator.create_socket<si::Velocity> ("adi/speed/ground-speed", 0_kt, { 0_kt, 400_kt }, 13_kt / 1_s);
	test_screen_1->adi->speed_v1											<< test_generator.create_socket<si::Velocity> ("adi/speed-bugs/v1", 80_kt, { 78_kt, 82_kt }, 1_kt / 1_s);
	test_screen_1->adi->speed_vr											<< test_generator.create_socket<si::Velocity> ("adi/speed-bugs/vr", 88_kt, { 86_kt, 89_kt }, 1_kt / 1_s);
	test_screen_1->adi->speed_vref											<< test_generator.create_socket<si::Velocity> ("adi/speed-bugs/vref", 95_kt, { 94_kt, 96_kt }, 0.1_kt / 1_s);
	test_screen_1->adi->speed_flaps_up_label								<< "UP";
	test_screen_1->adi->speed_flaps_up_speed								<< 140_kt;
	test_screen_1->adi->speed_flaps_a_label									<< "1";
	test_screen_1->adi->speed_flaps_a_speed									<< 120_kt;
	test_screen_1->adi->speed_flaps_b_label									<< "5";
	test_screen_1->adi->speed_flaps_b_speed									<< 110_kt;
	test_screen_1->adi->orientation_pitch
		<< test_generator.create_socket<si::Angle> ("adi/orientation/pitch", 0_deg, { -90_deg, 90_deg }, 8_deg / 1_s, TestGenerator::BorderCondition::Mirroring, { .nil = 3_s, .not_nil = 7_s });
	test_screen_1->adi->orientation_roll
		<< test_generator.create_socket<si::Angle> ("adi/orientation/roll", 0_deg, { -180_deg, +180_deg }, 1.5_deg / 1_s, TestGenerator::BorderCondition::Periodic, { .nil = 4_s, .not_nil = 6_s });
	test_screen_1->adi->orientation_heading_magnetic
		<< test_generator.create_socket<si::Angle> ("adi/orientation/heading.magnetic", 0_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	test_screen_1->adi->orientation_heading_true
		<< test_generator.create_socket<si::Angle> ("adi/orientation/heading.true", 10_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	test_screen_1->adi->orientation_heading_numbers_visible					<< true;
	test_screen_1->adi->track_lateral_magnetic								<< test_generator.create_socket<si::Angle> ("adi/track/lateral.magnetic", 9_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	test_screen_1->adi->track_lateral_true									<< test_generator.create_socket<si::Angle> ("adi/track/lateral.true", 19_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGenerator::BorderCondition::Periodic);
	test_screen_1->adi->track_vertical										<< test_generator.create_socket<si::Angle> ("adi/track/vertical", 0_deg, { -13_deg, 13_deg }, 1_deg / 1_s);
	test_screen_1->adi->fpv_visible											<< true;
	test_screen_1->adi->slip_skid											<< test_generator.create_socket<si::Angle> ("adi/slip-skid/angle", 0_deg, { -5_deg, 5_deg }, 0.5_deg / 1_s);
	test_screen_1->adi->aoa_alpha											<< test_generator.create_socket<si::Angle> ("adi/aoa/alpha", 0_deg, { -7_deg, 15_deg }, 1_deg / 1_s);
	test_screen_1->adi->aoa_alpha_maximum									<< test_generator.create_socket<si::Angle> ("adi/aoa/alpha.maximum", 13_deg, { 13_deg, 15_deg }, 0.25_deg / 1_s);
	test_screen_1->adi->aoa_alpha_visible									<< true;
	test_screen_1->adi->altitude_amsl
		<< test_generator.create_socket<si::Length> ("adi/altitude/amsl", -200_ft, { -200_ft, 2000_ft }, 2000_ft / 1_min, TestGenerator::BorderCondition::Mirroring, { .nil = 4_s, .not_nil = 7_s });
	test_screen_1->adi->altitude_amsl_lookahead								<< test_generator.create_socket<si::Length> ("adi/altitude/amsl.lookahead", 10_ft, { 0_ft, 2000_ft }, 100_ft / 1_min);
	test_screen_1->adi->altitude_agl_serviceable							<< test_generator.create_enum_socket<bool> ("adi/altitude/agl.serviceable", { { true, 16_s }, { false, 2_s } });
	test_screen_1->adi->altitude_agl										<< test_generator.create_socket<si::Length> ("adi/altitude/agl", -4_ft, { -4_ft, 30_m }, 100_ft / 1_min);
	test_screen_1->adi->decision_height_type								<< test_generator.create_enum_socket<std::string> ("adi/decision-height/type", { { "BARO", 5_s }, { "RADIO", 4_s } });
	test_screen_1->adi->decision_height_setting								<< 300_ft;
	test_screen_1->adi->decision_height_amsl								<< 300_ft;
	test_screen_1->adi->landing_amsl										<< 140_ft;
	test_screen_1->adi->vertical_speed
		<< test_generator.create_socket<si::Velocity> ("adi/vertical-speed/speed", 0_fpm, { -6000_fpm, +6000_fpm }, 100_fpm / 1_s, TestGenerator::BorderCondition::Mirroring, { .nil = 3_s, .not_nil = 8_s });
	test_screen_1->adi->vertical_speed_energy_variometer					<< test_generator.create_socket<si::Power> ("adi/vertical-speed/energy-variometer", 0_W, { -1000_W, +1000_W }, 100_W / 1_s);
	test_screen_1->adi->pressure_qnh										<< 1013_hPa;
	test_screen_1->adi->pressure_display_hpa								<< test_generator.create_enum_socket<bool> ("adi/pressure/display-hpa", { { true, 8_s }, { false, 8_s } });
	test_screen_1->adi->pressure_use_std									<< test_generator.create_enum_socket<bool> ("adi/pressure/use-std", { { true, 4_s }, { false, 4_s } });
	test_screen_1->adi->flight_director_serviceable							<< test_generator.create_enum_socket<bool> ("adi/flight-director/serviceable", { { true, 13_s }, { false, 2_s } });
	test_screen_1->adi->flight_director_active_name							<< test_generator.create_enum_socket<std::string> ("adi/flight-director/active-name", { { "L", 3_s }, { "R", 3_s }, { "", 2_s } });
	test_screen_1->adi->flight_director_cmd_visible							<< true;
	test_screen_1->adi->flight_director_cmd_altitude						<< 1000_ft;
	test_screen_1->adi->flight_director_cmd_altitude_acquired				<< false;
	test_screen_1->adi->flight_director_cmd_ias								<< 100_kt;
	test_screen_1->adi->flight_director_cmd_mach							<< 0.34;
	test_screen_1->adi->flight_director_cmd_vertical_speed					<< 1500_fpm;
	test_screen_1->adi->flight_director_cmd_fpa								<< 5_deg;
	test_screen_1->adi->flight_director_guidance_visible					<< true;
	test_screen_1->adi->flight_director_guidance_pitch						<< 2.5_deg;
	test_screen_1->adi->flight_director_guidance_roll						<< 0_deg;
	test_screen_1->adi->control_surfaces_visible							<< true;
	test_screen_1->adi->control_surfaces_elevator							<< test_generator.create_socket<double> ("adi/control-surfaces/elevator", 0.0f, { -1.0f, +1.0f }, 0.1f / 1_s);
	test_screen_1->adi->control_surfaces_ailerons							<< test_generator.create_socket<double> ("adi/control-surfaces/ailerons", 0.0f, { -1.0f, +1.0f }, 0.3f / 1_s);
	test_screen_1->adi->navaid_reference_visible							<< true;
	test_screen_1->adi->navaid_course_magnetic								<< 150_deg;
	test_screen_1->adi->navaid_type_hint									<< "VOR";
	test_screen_1->adi->navaid_identifier									<< "WRO";
	test_screen_1->adi->navaid_distance										<< 1.5_nmi;
	test_screen_1->adi->flight_path_deviation_lateral_serviceable			<< test_generator.create_enum_socket<bool> ("adi/flight-path-deviation/lateral/serviceable", { { true, 9.5_s }, { false, 2_s } });
	test_screen_1->adi->flight_path_deviation_lateral_approach				<< test_generator.create_socket<si::Angle> ("adi/flight-path-deviation/lateral/approach", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	test_screen_1->adi->flight_path_deviation_lateral_flight_path			<< test_generator.create_socket<si::Angle> ("adi/flight-path-deviation/lateral/flight-path", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	test_screen_1->adi->flight_path_deviation_vertical_serviceable			<< test_generator.create_enum_socket<bool> ("adi/flight-path-deviation/vertical/serviceable", { { true, 13.4_s }, { false, 2_s } });
	test_screen_1->adi->flight_path_deviation_vertical						<< test_generator.create_socket<si::Angle> ("adi/flight-path-deviation/vertical/deviation", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	test_screen_1->adi->flight_path_deviation_vertical_approach				<< test_generator.create_socket<si::Angle> ("adi/flight-path-deviation/vertical/approach", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	test_screen_1->adi->flight_path_deviation_vertical_flight_path			<< test_generator.create_socket<si::Angle> ("adi/flight-path-deviation/vertical/flight-path", 0_deg, { -5_deg, 5_deg }, 3_deg / 1_s);
	test_screen_1->adi->flight_path_deviation_mixed_mode					<< true;
	test_screen_1->adi->flight_mode_hint_visible							<< true;
	test_screen_1->adi->flight_mode_hint									<< test_generator.create_enum_socket<std::string> ("adi/fma/hint", { { "F/D", 11_s }, { "CMD", 15_s } });
	test_screen_1->adi->flight_mode_fma_visible								<< true;
	test_screen_1->adi->flight_mode_fma_speed_hint
		<< test_generator.create_enum_socket<std::string> ("adi/fma/speed-hint", { { std::string (afcs::kThrustMode_TO_GA), 15_s }, { std::string (afcs::kThrustMode_Continuous), 15_s } });
	test_screen_1->adi->flight_mode_fma_speed_armed_hint
		<< test_generator.create_enum_socket<std::string> ("adi/fma/speed-armed-hint", { { std::string (afcs::kSpeedMode_Airspeed), 17_s }, { std::string (afcs::kSpeedMode_Thrust), 17_s } });
	test_screen_1->adi->flight_mode_fma_lateral_hint
		<< test_generator.create_enum_socket<std::string> (
			"adi/fma/lateral-hint",
			{
				{ std::string (afcs::kRollMode_Track), 12_s },
				{ std::string (afcs::kRollMode_WingsLevel), 12_s },
				{ std::string (afcs::kRollMode_LNAV), 15_s },
				{ std::string (afcs::kRollMode_Localizer), 12_s }
			}
		);
	test_screen_1->adi->flight_mode_fma_lateral_armed_hint
		<< test_generator.create_enum_socket<std::string> ("adi/fma/lateral-armed-hint", { { std::string (afcs::kRollMode_Track), 13_s }, { std::string (afcs::kRollMode_Heading), 13_s } });
	test_screen_1->adi->flight_mode_fma_vertical_hint
		<< test_generator.create_enum_socket<std::string> ("adi/fma/vertical-hint", { { std::string (afcs::kPitchMode_Altitude), 11_s }, { std::string (afcs::kPitchMode_TO_GA), 17_s } });
	test_screen_1->adi->flight_mode_fma_vertical_armed_hint
		<< test_generator.create_enum_socket<std::string> ("adi/fma/vertical-armed-hint", { { std::string (afcs::kPitchMode_GS), 14_s }, { std::string (afcs::kPitchMode_VNAVPath), 14_s } });
	test_screen_1->adi->tcas_resolution_advisory_pitch_minimum				<< -45_deg;
	test_screen_1->adi->tcas_resolution_advisory_pitch_maximum				<< 80_deg;
	test_screen_1->adi->tcas_resolution_advisory_vertical_speed_minimum		<< -3000_fpm;
	test_screen_1->adi->tcas_resolution_advisory_vertical_speed_maximum		<< 10000_fpm;
	test_screen_1->adi->warning_novspd_flag									<< test_generator.create_enum_socket<bool> ("adi/flags/novspd", { { false, 3_s }, { true, 2_s } });
	test_screen_1->adi->warning_ldgalt_flag									<< test_generator.create_enum_socket<bool> ("adi/flags/ldgalt", { { false, 7_s }, { true, 2_s } });
	test_screen_1->adi->warning_pitch_disagree								<< test_generator.create_enum_socket<bool> ("adi/flags/pitch-disagree", { { false, 5_s }, { true, 2_s } });
	test_screen_1->adi->warning_roll_disagree								<< test_generator.create_enum_socket<bool> ("adi/flags/roll-disagree", { { false, 4_s }, { true, 2_s } });
	test_screen_1->adi->warning_ias_disagree								<< test_generator.create_enum_socket<bool> ("adi/flags/ias-disagree", { { false, 9_s }, { true, 2_s } });
	test_screen_1->adi->warning_altitude_disagree							<< test_generator.create_enum_socket<bool> ("adi/flags/altitude-disagree", { { false, 8_s }, { true, 2_s } });
	test_screen_1->adi->warning_roll										<< test_generator.create_enum_socket<bool> ("adi/flags/roll", { { false, 11_s }, { true, 2_s } });
	test_screen_1->adi->warning_slip_skid									<< test_generator.create_enum_socket<bool> ("adi/flags/slip-skid", { { false, 7.5_s }, { true, 2_s } });
	test_screen_1->adi->style_old											<< false;
	test_screen_1->adi->style_show_metric									<< true;

	test_screen_1->hsi->display_mode										<< hsi::DisplayMode::Auxiliary;
	test_screen_1->hsi->range												<< test_generator_hsi_range;
	test_screen_1->hsi->speed_gs											<< test_generator_hsi_speed_gs;
	test_screen_1->hsi->speed_tas											<< test_generator_hsi_speed_tas;
	test_screen_1->hsi->cmd_visible											<< test_generator_hsi_cmd_visible;
	test_screen_1->hsi->cmd_line_visible									<< test_generator_hsi_cmd_line_visible;
	test_screen_1->hsi->cmd_heading_magnetic								<< test_generator_hsi_cmd_heading_magnetic;
	test_screen_1->hsi->cmd_track_magnetic									<< test_generator_hsi_cmd_track_magnetic;
	test_screen_1->hsi->cmd_use_trk											<< test_generator_hsi_cmd_use_trk;
	test_screen_1->hsi->target_altitude_reach_distance						<< test_generator_hsi_target_altitude_reach_distance;
	test_screen_1->hsi->orientation_heading_magnetic						<< test_generator_hsi_orientation_heading_magnetic;
	test_screen_1->hsi->orientation_heading_true							<< test_generator_hsi_orientation_heading_true;
	test_screen_1->hsi->heading_mode										<< test_generator_hsi_heading_mode;
	test_screen_1->hsi->home_true_direction									<< test_generator_hsi_home_true_direction;
	test_screen_1->hsi->home_track_visible									<< test_generator_hsi_home_track_visible;
	test_screen_1->hsi->home_distance_vlos									<< test_generator_hsi_home_distance_vlos;
	test_screen_1->hsi->home_distance_ground								<< test_generator_hsi_home_distance_ground;
	test_screen_1->hsi->home_distance_vertical								<< test_generator_hsi_home_distance_vertical;
	test_screen_1->hsi->home_position_longitude								<< test_generator_hsi_home_position_longitude;
	test_screen_1->hsi->home_position_latitude								<< test_generator_hsi_home_position_latitude;
	test_screen_1->hsi->position_longitude									<< test_generator_hsi_position_longitude;
	test_screen_1->hsi->position_latitude									<< test_generator_hsi_position_latitude;
	test_screen_1->hsi->position_source										<< test_generator_hsi_position_source;
	test_screen_1->hsi->flight_range_warning_longitude						<< test_generator_hsi_range_warning_longitude;
	test_screen_1->hsi->flight_range_warning_latitude						<< test_generator_hsi_range_warning_latitude;
	test_screen_1->hsi->flight_range_warning_radius							<< test_generator_hsi_range_warning_radius;
	test_screen_1->hsi->flight_range_critical_longitude						<< test_generator_hsi_range_critical_longitude;
	test_screen_1->hsi->flight_range_critical_latitude						<< test_generator_hsi_range_critical_latitude;
	test_screen_1->hsi->flight_range_critical_radius						<< test_generator_hsi_range_critical_radius;
	test_screen_1->hsi->track_visible										<< test_generator_hsi_track_visible;
	test_screen_1->hsi->track_lateral_magnetic								<< test_generator_hsi_track_lateral_magnetic;
	test_screen_1->hsi->track_lateral_rotation								<< test_generator_hsi_track_lateral_rotation;
	test_screen_1->hsi->track_center_on_track								<< test_generator_hsi_track_center_on_track;
	test_screen_1->hsi->course_visible										<< test_generator_hsi_course_visible;
	test_screen_1->hsi->course_setting_magnetic								<< test_generator_hsi_course_setting_magnetic;
	test_screen_1->hsi->course_deviation									<< test_generator_hsi_course_deviation;
	test_screen_1->hsi->course_to_flag										<< test_generator_hsi_course_to_flag;
	test_screen_1->hsi->navaid_selected_reference							<< test_generator_hsi_navaid_selected_reference;
	test_screen_1->hsi->navaid_selected_identifier							<< test_generator_hsi_navaid_selected_identifier;
	test_screen_1->hsi->navaid_selected_distance							<< test_generator_hsi_navaid_selected_distance;
	test_screen_1->hsi->navaid_selected_eta									<< test_generator_hsi_navaid_selected_eta;
	test_screen_1->hsi->navaid_selected_course_magnetic						<< test_generator_hsi_navaid_selected_course_magnetic;
	test_screen_1->hsi->navaid_left_type									<< test_generator_hsi_navaid_left_type;
	test_screen_1->hsi->navaid_left_reference								<< test_generator_hsi_navaid_left_reference;
	test_screen_1->hsi->navaid_left_identifier								<< test_generator_hsi_navaid_left_identifier;
	test_screen_1->hsi->navaid_left_distance								<< test_generator_hsi_navaid_left_distance;
	test_screen_1->hsi->navaid_left_initial_bearing_magnetic				<< test_generator_hsi_navaid_left_initial_bearing_magnetic;
	test_screen_1->hsi->navaid_right_type									<< test_generator_hsi_navaid_right_type;
	test_screen_1->hsi->navaid_right_reference								<< test_generator_hsi_navaid_right_reference;
	test_screen_1->hsi->navaid_right_identifier								<< test_generator_hsi_navaid_right_identifier;
	test_screen_1->hsi->navaid_right_distance								<< test_generator_hsi_navaid_right_distance;
	test_screen_1->hsi->navaid_right_initial_bearing_magnetic				<< test_generator_hsi_navaid_right_initial_bearing_magnetic;
	test_screen_1->hsi->navigation_required_performance						<< test_generator_hsi_navigation_required_performance;
	test_screen_1->hsi->navigation_actual_performance						<< test_generator_hsi_navigation_actual_performance;
	test_screen_1->hsi->wind_from_magnetic									<< test_generator_hsi_wind_from_magnetic;
	test_screen_1->hsi->wind_speed_tas										<< test_generator_hsi_wind_speed_tas;
	test_screen_1->hsi->localizer_id										<< test_generator_hsi_localizer_id;
	test_screen_1->hsi->tcas_on												<< test_generator_hsi_tcas_on;
	test_screen_1->hsi->tcas_range											<< test_generator_hsi_tcas_range;
	test_screen_1->hsi->features_fix										<< test_generator_hsi_features_fix;
	test_screen_1->hsi->features_vor										<< test_generator_hsi_features_vor;
	test_screen_1->hsi->features_dme										<< test_generator_hsi_features_dme;
	test_screen_1->hsi->features_ndb										<< test_generator_hsi_features_ndb;
	test_screen_1->hsi->features_loc										<< test_generator_hsi_features_loc;
	test_screen_1->hsi->features_arpt										<< test_generator_hsi_features_arpt;
	test_screen_1->hsi->radio_position_longitude							<< test_generator_hsi_home_position_longitude;
	test_screen_1->hsi->radio_position_latitude								<< test_generator_hsi_home_position_latitude;
	test_screen_1->hsi->radio_range_warning									<< test_generator_hsi_radio_range_warning;
	test_screen_1->hsi->radio_range_critical								<< test_generator_hsi_radio_range_critical;

	// Testing std::function-converters:
	test_screen_1->engine_l_thrust->value									<< std::function (angle_to_force) << std::function (force_to_angle) << test_generator.create_socket<si::Force> ("engine/left/thrust", 0_N, { -0.3_N, 4.5_N }, 0.2_N / 1_s);
	test_screen_1->engine_l_thrust->reference								<< 4.1_N;
	test_screen_1->engine_l_thrust->target									<< 3.9_N;
	test_screen_1->engine_l_thrust->automatic								<< test_generator.create_socket<si::Force> ("engine/left/thrust/automatic", 2_N, { 1.5_N, 2.5_N }, 0.1_N / 1_s);

	test_screen_1->engine_l_speed->value									<< test_generator.create_socket<si::AngularVelocity> ("engine/left/speed", 0.0_rpm, { -100_rpm, 15'000_rpm }, 1200_rpm / 1_s);

	test_screen_1->engine_l_temperature->value								<< test_generator.create_socket<si::Temperature> ("engine/left/temperature", 0_degC, { -20_degC, 75_degC }, 5_K / 1_s);

	test_screen_1->engine_l_power->value									<< test_generator.create_socket<si::Power> ("engine/left/power", 0_W, { 0_W, 295_W }, 11_W / 1_s);

	test_screen_1->engine_l_current->value									<< test_generator.create_socket<si::Current> ("engine/left/current", 0_A, { -5_A, 40_A }, 5_A / 1_s);

	test_screen_1->engine_l_voltage->value									<< test_generator.create_socket<si::Voltage> ("engine/left/voltage", 16.8_V, { 11.1_V, 16.8_V }, 0.07_V / 1_s);

	test_screen_1->engine_l_vibration->value								<< test_generator.create_socket<si::Acceleration> ("engine/left/vibration", 0.1_g, { 0.1_g, 1.2_g }, 0.025_g / 1_s);

	test_screen_1->engine_r_thrust->value									<< test_generator.create_socket<si::Force> ("engine/right/thrust", 0_N, { -0.3_N, 4.5_N }, 0.2_N / 1_s);
	test_screen_1->engine_r_thrust->reference								<< 4.1_N;
	test_screen_1->engine_r_thrust->target									<< 3.9_N;
	test_screen_1->engine_r_thrust->automatic								<< test_generator.create_socket<si::Force> ("engine/right/thrust/automatic", 2_N, { 1.5_N, 2.5_N }, 0.1_N / 1_s);

	test_screen_1->engine_r_speed->value									<< test_generator.create_socket<si::AngularVelocity> ("engine/right/speed", 0.0_rpm, { -100_rpm, 15'000_rpm }, 1200_rpm / 1_s);

	test_screen_1->engine_r_temperature->value								<< test_generator.create_socket<si::Temperature> ("engine/right/temperature", 0_degC, { -20_degC, 75_degC }, 5_K / 1_s);

	test_screen_1->engine_r_power->value									<< test_generator.create_socket<si::Power> ("engine/right/power", 0_W, { 0_W, 295_W }, 10_W / 1_s);

	test_screen_1->engine_r_current->value									<< test_generator.create_socket<si::Current> ("engine/right/current", 0_A, { -5_A, 40_A }, 5_A / 1_s);

	test_screen_1->engine_r_voltage->value									<< test_generator.create_socket<si::Voltage> ("engine/right/voltage", 16.8_V, { 11.1_V, 16.8_V }, 0.073_V / 1_s);

	test_screen_1->engine_r_vibration->value
		<< test_generator.create_socket<si::Acceleration> ("engine/right/vibration", 0.1_g, { 0.1_g, 1.2_g }, 0.025_g / 1_s, TestGenerator::BorderCondition::Mirroring, { .nil = 2.5_s, .not_nil = 6.5_s });

	test_screen_1->gear->requested_down										<< true;
	test_screen_1->gear->nose_up											<< false;
	test_screen_1->gear->nose_down											<< true;
	test_screen_1->gear->left_up											<< false;
	test_screen_1->gear->left_down											<< true;
	test_screen_1->gear->right_up											<< false;
	test_screen_1->gear->right_down											<< true;

	test_screen_1->vertical_trim->trim_value								<< test_generator.create_socket<double> ("vertical-trim", 0.0, { 0.0, 1.0 }, 0.1 / 1_s);
	test_screen_1->vertical_trim->trim_reference							<< 0.5;
	test_screen_1->vertical_trim->trim_reference_minimum					<< 0.35;
	test_screen_1->vertical_trim->trim_reference_maximum					<< 0.6;

	test_screen_1->glide_ratio->value										<< test_generator.create_socket<double> ("perf/glide-ratio", 50.0, { 15, 75 }, 3 / 1_s);
	test_screen_1->load_factor->value										<< test_generator.create_socket<double> ("perf/load-factor", 1.0, { 0.4, 3.3 }, 0.2 / 1_s);

	test_screen_2->hsi_1->display_mode										<< hsi::DisplayMode::Expanded;
	test_screen_2->hsi_1->range												<< test_generator_hsi_range;
	test_screen_2->hsi_1->speed_gs											<< test_generator_hsi_speed_gs;
	test_screen_2->hsi_1->speed_tas											<< test_generator_hsi_speed_tas;
	test_screen_2->hsi_1->cmd_visible										<< test_generator_hsi_cmd_visible;
	test_screen_2->hsi_1->cmd_line_visible									<< test_generator_hsi_cmd_line_visible;
	test_screen_2->hsi_1->cmd_heading_magnetic								<< test_generator_hsi_cmd_heading_magnetic;
	test_screen_2->hsi_1->cmd_track_magnetic								<< test_generator_hsi_cmd_track_magnetic;
	test_screen_2->hsi_1->cmd_use_trk										<< test_generator_hsi_cmd_use_trk;
	test_screen_2->hsi_1->target_altitude_reach_distance					<< test_generator_hsi_target_altitude_reach_distance;
	test_screen_2->hsi_1->orientation_heading_magnetic						<< test_generator_hsi_orientation_heading_magnetic;
	test_screen_2->hsi_1->orientation_heading_true							<< test_generator_hsi_orientation_heading_true;
	test_screen_2->hsi_1->heading_mode										<< test_generator_hsi_heading_mode;
	test_screen_2->hsi_1->home_true_direction								<< test_generator_hsi_home_true_direction;
	test_screen_2->hsi_1->home_track_visible								<< test_generator_hsi_home_track_visible;
	test_screen_2->hsi_1->home_distance_vlos								<< test_generator_hsi_home_distance_vlos;
	test_screen_2->hsi_1->home_distance_ground								<< test_generator_hsi_home_distance_ground;
	test_screen_2->hsi_1->home_distance_vertical							<< test_generator_hsi_home_distance_vertical;
	test_screen_2->hsi_1->home_position_longitude							<< test_generator_hsi_home_position_longitude;
	test_screen_2->hsi_1->home_position_latitude							<< test_generator_hsi_home_position_latitude;
	test_screen_2->hsi_1->position_longitude								<< test_generator_hsi_position_longitude;
	test_screen_2->hsi_1->position_latitude									<< test_generator_hsi_position_latitude;
	test_screen_2->hsi_1->position_source									<< test_generator_hsi_position_source;
	test_screen_2->hsi_1->flight_range_warning_longitude					<< test_generator_hsi_range_warning_longitude;
	test_screen_2->hsi_1->flight_range_warning_latitude						<< test_generator_hsi_range_warning_latitude;
	test_screen_2->hsi_1->flight_range_warning_radius						<< test_generator_hsi_range_warning_radius;
	test_screen_2->hsi_1->flight_range_critical_longitude					<< test_generator_hsi_range_critical_longitude;
	test_screen_2->hsi_1->flight_range_critical_latitude					<< test_generator_hsi_range_critical_latitude;
	test_screen_2->hsi_1->flight_range_critical_radius						<< test_generator_hsi_range_critical_radius;
	test_screen_2->hsi_1->track_visible										<< test_generator_hsi_track_visible;
	test_screen_2->hsi_1->track_lateral_magnetic							<< test_generator_hsi_track_lateral_magnetic;
	test_screen_2->hsi_1->track_lateral_rotation							<< test_generator_hsi_track_lateral_rotation;
	test_screen_2->hsi_1->track_center_on_track								<< test_generator_hsi_track_center_on_track;
	test_screen_2->hsi_1->course_visible									<< test_generator_hsi_course_visible;
	test_screen_2->hsi_1->course_setting_magnetic							<< test_generator_hsi_course_setting_magnetic;
	test_screen_2->hsi_1->course_deviation									<< test_generator_hsi_course_deviation;
	test_screen_2->hsi_1->course_to_flag									<< test_generator_hsi_course_to_flag;
	test_screen_2->hsi_1->navaid_selected_reference							<< test_generator_hsi_navaid_selected_reference;
	test_screen_2->hsi_1->navaid_selected_identifier						<< test_generator_hsi_navaid_selected_identifier;
	test_screen_2->hsi_1->navaid_selected_distance							<< test_generator_hsi_navaid_selected_distance;
	test_screen_2->hsi_1->navaid_selected_eta								<< test_generator_hsi_navaid_selected_eta;
	test_screen_2->hsi_1->navaid_selected_course_magnetic					<< test_generator_hsi_navaid_selected_course_magnetic;
	test_screen_2->hsi_1->navaid_left_type									<< test_generator_hsi_navaid_left_type;
	test_screen_2->hsi_1->navaid_left_reference								<< test_generator_hsi_navaid_left_reference;
	test_screen_2->hsi_1->navaid_left_identifier							<< test_generator_hsi_navaid_left_identifier;
	test_screen_2->hsi_1->navaid_left_distance								<< test_generator_hsi_navaid_left_distance;
	test_screen_2->hsi_1->navaid_left_initial_bearing_magnetic				<< test_generator_hsi_navaid_left_initial_bearing_magnetic;
	test_screen_2->hsi_1->navaid_right_type									<< test_generator_hsi_navaid_right_type;
	test_screen_2->hsi_1->navaid_right_reference							<< test_generator_hsi_navaid_right_reference;
	test_screen_2->hsi_1->navaid_right_identifier							<< test_generator_hsi_navaid_right_identifier;
	test_screen_2->hsi_1->navaid_right_distance								<< test_generator_hsi_navaid_right_distance;
	test_screen_2->hsi_1->navaid_right_initial_bearing_magnetic				<< test_generator_hsi_navaid_right_initial_bearing_magnetic;
	test_screen_2->hsi_1->navigation_required_performance					<< test_generator_hsi_navigation_required_performance;
	test_screen_2->hsi_1->navigation_actual_performance						<< test_generator_hsi_navigation_actual_performance;
	test_screen_2->hsi_1->wind_from_magnetic								<< test_generator_hsi_wind_from_magnetic;
	test_screen_2->hsi_1->wind_speed_tas									<< test_generator_hsi_wind_speed_tas;
	test_screen_2->hsi_1->localizer_id										<< test_generator_hsi_localizer_id;
	test_screen_2->hsi_1->tcas_on											<< test_generator_hsi_tcas_on;
	test_screen_2->hsi_1->tcas_range										<< test_generator_hsi_tcas_range;
	test_screen_2->hsi_1->features_fix										<< test_generator_hsi_features_fix;
	test_screen_2->hsi_1->features_vor										<< test_generator_hsi_features_vor;
	test_screen_2->hsi_1->features_dme										<< test_generator_hsi_features_dme;
	test_screen_2->hsi_1->features_ndb										<< test_generator_hsi_features_ndb;
	test_screen_2->hsi_1->features_loc										<< test_generator_hsi_features_loc;
	test_screen_2->hsi_1->features_arpt										<< test_generator_hsi_features_arpt;
	test_screen_2->hsi_1->radio_position_longitude							<< test_generator_hsi_home_position_longitude;
	test_screen_2->hsi_1->radio_position_latitude							<< test_generator_hsi_home_position_latitude;
	test_screen_2->hsi_1->radio_range_warning								<< test_generator_hsi_radio_range_warning;
	test_screen_2->hsi_1->radio_range_critical								<< test_generator_hsi_radio_range_critical;

	test_screen_2->hsi_2->display_mode										<< hsi::DisplayMode::Rose;
	test_screen_2->hsi_2->range												<< test_generator_hsi_range;
	test_screen_2->hsi_2->speed_gs											<< test_generator_hsi_speed_gs;
	test_screen_2->hsi_2->speed_tas											<< test_generator_hsi_speed_tas;
	test_screen_2->hsi_2->cmd_visible										<< test_generator_hsi_cmd_visible;
	test_screen_2->hsi_2->cmd_line_visible									<< test_generator_hsi_cmd_line_visible;
	test_screen_2->hsi_2->cmd_heading_magnetic								<< test_generator_hsi_cmd_heading_magnetic;
	test_screen_2->hsi_2->cmd_track_magnetic								<< test_generator_hsi_cmd_track_magnetic;
	test_screen_2->hsi_2->cmd_use_trk										<< test_generator_hsi_cmd_use_trk;
	test_screen_2->hsi_2->target_altitude_reach_distance					<< test_generator_hsi_target_altitude_reach_distance;
	test_screen_2->hsi_2->orientation_heading_magnetic						<< test_generator_hsi_orientation_heading_magnetic;
	test_screen_2->hsi_2->orientation_heading_true							<< test_generator_hsi_orientation_heading_true;
	test_screen_2->hsi_2->heading_mode										<< test_generator_hsi_heading_mode;
	test_screen_2->hsi_2->home_true_direction								<< test_generator_hsi_home_true_direction;
	test_screen_2->hsi_2->home_track_visible								<< test_generator_hsi_home_track_visible;
	test_screen_2->hsi_2->home_distance_vlos								<< test_generator_hsi_home_distance_vlos;
	test_screen_2->hsi_2->home_distance_ground								<< test_generator_hsi_home_distance_ground;
	test_screen_2->hsi_2->home_distance_vertical							<< test_generator_hsi_home_distance_vertical;
	test_screen_2->hsi_2->home_position_longitude							<< test_generator_hsi_home_position_longitude;
	test_screen_2->hsi_2->home_position_latitude							<< test_generator_hsi_home_position_latitude;
	test_screen_2->hsi_2->position_longitude								<< test_generator_hsi_position_longitude;
	test_screen_2->hsi_2->position_latitude									<< test_generator_hsi_position_latitude;
	test_screen_2->hsi_2->position_source									<< test_generator_hsi_position_source;
	test_screen_2->hsi_2->flight_range_warning_longitude					<< test_generator_hsi_range_warning_longitude;
	test_screen_2->hsi_2->flight_range_warning_latitude						<< test_generator_hsi_range_warning_latitude;
	test_screen_2->hsi_2->flight_range_warning_radius						<< test_generator_hsi_range_warning_radius;
	test_screen_2->hsi_2->flight_range_critical_longitude					<< test_generator_hsi_range_critical_longitude;
	test_screen_2->hsi_2->flight_range_critical_latitude					<< test_generator_hsi_range_critical_latitude;
	test_screen_2->hsi_2->flight_range_critical_radius						<< test_generator_hsi_range_critical_radius;
	test_screen_2->hsi_2->track_visible										<< test_generator_hsi_track_visible;
	test_screen_2->hsi_2->track_lateral_magnetic							<< test_generator_hsi_track_lateral_magnetic;
	test_screen_2->hsi_2->track_lateral_rotation							<< test_generator_hsi_track_lateral_rotation;
	test_screen_2->hsi_2->track_center_on_track								<< test_generator_hsi_track_center_on_track;
	test_screen_2->hsi_2->course_visible									<< test_generator_hsi_course_visible;
	test_screen_2->hsi_2->course_setting_magnetic							<< test_generator_hsi_course_setting_magnetic;
	test_screen_2->hsi_2->course_deviation									<< test_generator_hsi_course_deviation;
	test_screen_2->hsi_2->course_to_flag									<< test_generator_hsi_course_to_flag;
	test_screen_2->hsi_2->navaid_selected_reference							<< test_generator_hsi_navaid_selected_reference;
	test_screen_2->hsi_2->navaid_selected_identifier						<< test_generator_hsi_navaid_selected_identifier;
	test_screen_2->hsi_2->navaid_selected_distance							<< test_generator_hsi_navaid_selected_distance;
	test_screen_2->hsi_2->navaid_selected_eta								<< test_generator_hsi_navaid_selected_eta;
	test_screen_2->hsi_2->navaid_selected_course_magnetic					<< test_generator_hsi_navaid_selected_course_magnetic;
	test_screen_2->hsi_2->navaid_left_type									<< test_generator_hsi_navaid_left_type;
	test_screen_2->hsi_2->navaid_left_reference								<< test_generator_hsi_navaid_left_reference;
	test_screen_2->hsi_2->navaid_left_identifier							<< test_generator_hsi_navaid_left_identifier;
	test_screen_2->hsi_2->navaid_left_distance								<< test_generator_hsi_navaid_left_distance;
	test_screen_2->hsi_2->navaid_left_initial_bearing_magnetic				<< test_generator_hsi_navaid_left_initial_bearing_magnetic;
	test_screen_2->hsi_2->navaid_right_type									<< test_generator_hsi_navaid_right_type;
	test_screen_2->hsi_2->navaid_right_reference							<< test_generator_hsi_navaid_right_reference;
	test_screen_2->hsi_2->navaid_right_identifier							<< test_generator_hsi_navaid_right_identifier;
	test_screen_2->hsi_2->navaid_right_distance								<< test_generator_hsi_navaid_right_distance;
	test_screen_2->hsi_2->navaid_right_initial_bearing_magnetic				<< test_generator_hsi_navaid_right_initial_bearing_magnetic;
	test_screen_2->hsi_2->navigation_required_performance					<< test_generator_hsi_navigation_required_performance;
	test_screen_2->hsi_2->navigation_actual_performance						<< test_generator_hsi_navigation_actual_performance;
	test_screen_2->hsi_2->wind_from_magnetic								<< test_generator_hsi_wind_from_magnetic;
	test_screen_2->hsi_2->wind_speed_tas									<< test_generator_hsi_wind_speed_tas;
	test_screen_2->hsi_2->localizer_id										<< test_generator_hsi_localizer_id;
	test_screen_2->hsi_2->tcas_on											<< test_generator_hsi_tcas_on;
	test_screen_2->hsi_2->tcas_range										<< test_generator_hsi_tcas_range;
	test_screen_2->hsi_2->features_fix										<< test_generator_hsi_features_fix;
	test_screen_2->hsi_2->features_vor										<< test_generator_hsi_features_vor;
	test_screen_2->hsi_2->features_dme										<< test_generator_hsi_features_dme;
	test_screen_2->hsi_2->features_ndb										<< test_generator_hsi_features_ndb;
	test_screen_2->hsi_2->features_loc										<< test_generator_hsi_features_loc;
	test_screen_2->hsi_2->features_arpt										<< test_generator_hsi_features_arpt;
	test_screen_2->hsi_2->radio_position_longitude							<< test_generator_hsi_home_position_longitude;
	test_screen_2->hsi_2->radio_position_latitude							<< test_generator_hsi_home_position_latitude;
	test_screen_2->hsi_2->radio_range_warning								<< test_generator_hsi_radio_range_warning;
	test_screen_2->hsi_2->radio_range_critical								<< test_generator_hsi_radio_range_critical;

	auto& test_loop_registrant = _test_loop.emplace (*this, "Main loop", 120_Hz, _logger.with_scope ("short computations loop"));
	auto& test_loop = *test_loop_registrant;

	register_screen (test_screen_1);
	register_screen (test_screen_2);
	register_processing_loop (test_loop_registrant);

	// Register all instruments in the processing loop:
	for (auto& disclosure: test_screen_1->instrument_tracker())
		test_loop.register_module (disclosure.registrant());

	for (auto& disclosure: test_screen_2->instrument_tracker())
		test_loop.register_module (disclosure.registrant());

	// Register the rest:
	test_loop.register_module (test_generator_registrant);
	test_loop.register_module (test_loop_registrant);
	test_loop.start();

	test_screen_1->show();
	test_screen_2->show();
}


TestInstrumentsMachine::~TestInstrumentsMachine()
{
	if (_navaid_storage)
		_navaid_storage->interrupt_loading();
}


std::unique_ptr<xf::Machine>
xefis_machine (xf::Xefis& xefis)
{
	return std::make_unique<TestInstrumentsMachine> (xefis);
}

