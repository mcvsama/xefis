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
#include "simulation_machine.h"


SimulationMachine::SimulationMachine (xf::Xefis& xefis):
	Machine (xefis),
	_logger (xefis.logger())
{
	_work_performer = std::make_unique<xf::WorkPerformer> (std::thread::hardware_concurrency(), _logger);

	_navaid_storage = std::make_unique<xf::NavaidStorage> (_logger, "share/nav/nav.dat.gz", "share/nav/fix.dat.gz", "share/nav/apt.dat.gz");
	_work_performer->submit (_navaid_storage->async_loader());

	_loop.emplace (*this, "Main loop", 120_Hz, _logger.with_scope ("Main Loop 120 Hz")),
	register_processing_loop (*_loop);

	auto const main_line_width = 0.3525_mm;
	auto const main_font_height = 3.15_mm;

	xf::ScreenSpec pfd_spec { QRect { 0, 0, 1366, 768 }, 15_in, 60_Hz, main_line_width, main_font_height };
	pfd_spec.set_scale (1.25f);

	auto& pfd_screen = _screen_pfd.emplace (pfd_spec, xefis.graphics(), *_navaid_storage, *this, _logger.with_scope ("PFD screen"));
	pfd_screen->set_paint_bounding_boxes (false);
	register_screen (*_screen_pfd);

	auto const backup_line_width = 0.2_mm;
	auto const backup_font_height = 1.7_mm;

	xf::ScreenSpec backup_spec { QRect { 0, 0, 300, 300 }, 15_in / 4.5, 60_Hz, backup_line_width, backup_font_height };
	backup_spec.set_scale (1.25f);

	auto& backup_screen = _screen_backup.emplace (backup_spec, xefis.graphics(), *this, _logger.with_scope ("backup screen"));
	backup_screen->set_paint_bounding_boxes (false);
	register_screen (*_screen_backup);

	// BMP085 noise as per spec:
	auto const kBMP085PressureNoise = xf::NormalVariable { 0_Pa, 3_Pa };
	auto const kBMP085PressureResolution = 1_Pa;
	auto const kBMP085TemperatureNoise = xf::NormalVariable { 0_K, 0.1_K };
	auto const kBMP085TemperatureResolution = 0.1_K;

	auto virtual_joystick_io = std::make_unique<VirtualJoystickIO>();
	auto pressure_sensor_static_io = std::make_unique<VirtualPressureSensorIO>();
	auto pressure_sensor_total_io = std::make_unique<VirtualPressureSensorIO>();
	auto temperature_sensor_total_io = std::make_unique<VirtualTemperatureSensorIO>();
	auto sim_airplane_io = std::make_unique<SimAirplaneIO>();
	auto air_data_computer_io = std::make_unique<AirDataComputerIO>();

	pressure_sensor_static_io->update_interval								= 25_ms;
	pressure_sensor_static_io->noise										= kBMP085PressureNoise;
	pressure_sensor_static_io->resolution									= kBMP085PressureResolution;

	pressure_sensor_total_io->update_interval								= 25_ms;
	pressure_sensor_total_io->noise											= kBMP085PressureNoise;
	pressure_sensor_total_io->resolution									= kBMP085PressureResolution;

	temperature_sensor_total_io->update_interval							= 500_ms;
	temperature_sensor_total_io->noise										= kBMP085TemperatureNoise;
	temperature_sensor_total_io->resolution									= kBMP085TemperatureResolution;

	sim_airplane_io->joystick_x_axis										<< virtual_joystick_io->x_axis;
	sim_airplane_io->joystick_y_axis										<< virtual_joystick_io->y_axis;
	sim_airplane_io->joystick_throttle										<< virtual_joystick_io->throttle;
	sim_airplane_io->joystick_rudder										<< virtual_joystick_io->rudder;

	air_data_computer_io->ias_valid_minimum									= 0_kt;
	air_data_computer_io->ias_valid_maximum									= 350_kt;
	air_data_computer_io->pressure_use_std									<< xf::ConstantSource (true);
	air_data_computer_io->pressure_qnh										<< xf::ConstantSource (1013.25_hPa);
	air_data_computer_io->pressure_static									<< pressure_sensor_static_io->pressure;
	air_data_computer_io->pressure_total									<< pressure_sensor_total_io->pressure;
	air_data_computer_io->sensed_cas										<< xf::no_data_source;
	air_data_computer_io->total_air_temperature								<< temperature_sensor_total_io->temperature;

	// TODO those vars/data should come from real simulated values from FlightSimulation:
	backup_screen->adi_io->speed_ias										<< sim_airplane_io->real_cas;
	backup_screen->adi_io->speed_ground										<< sim_airplane_io->real_ground_speed;
	backup_screen->adi_io->orientation_pitch								<< sim_airplane_io->real_orientation_pitch;
	backup_screen->adi_io->orientation_roll									<< sim_airplane_io->real_orientation_roll;
	backup_screen->adi_io->orientation_heading_true							<< sim_airplane_io->real_orientation_heading_true;
	backup_screen->adi_io->track_lateral_true								<< sim_airplane_io->real_track_lateral_true;
	backup_screen->adi_io->track_vertical									<< sim_airplane_io->real_track_vertical;
	backup_screen->adi_io->orientation_heading_numbers_visible				<< xf::ConstantSource (true);
	backup_screen->adi_io->altitude_amsl									<< sim_airplane_io->real_altitude_amsl;
	backup_screen->adi_io->altitude_agl_serviceable							<< xf::ConstantSource (true);
	backup_screen->adi_io->altitude_agl										<< sim_airplane_io->real_altitude_agl;
	backup_screen->adi_io->vertical_speed									<< sim_airplane_io->real_vertical_speed;

	pfd_screen->adi_io->show_mach_above										= 0.1;
	pfd_screen->adi_io->weight_on_wheels									<< xf::ConstantSource (false);
	pfd_screen->adi_io->speed_ias											<< air_data_computer_io->speed_cas;
	pfd_screen->adi_io->speed_ias_lookahead									<< air_data_computer_io->speed_cas_lookahead;
	pfd_screen->adi_io->speed_ias_minimum									<< xf::no_data_source;
	pfd_screen->adi_io->speed_ias_minimum_maneuver							<< xf::no_data_source;
	pfd_screen->adi_io->speed_ias_maximum_maneuver							<< xf::no_data_source;
	pfd_screen->adi_io->speed_ias_maximum									<< xf::no_data_source;
	pfd_screen->adi_io->speed_mach											<< air_data_computer_io->speed_mach;
	pfd_screen->adi_io->speed_ground										<< sim_airplane_io->real_ground_speed;
	pfd_screen->adi_io->speed_v1											<< xf::no_data_source;
	pfd_screen->adi_io->speed_vr											<< xf::no_data_source;
	pfd_screen->adi_io->speed_vref											<< xf::no_data_source;
	pfd_screen->adi_io->speed_flaps_up_label								<< xf::no_data_source;
	pfd_screen->adi_io->speed_flaps_up_speed								<< xf::no_data_source;
	pfd_screen->adi_io->speed_flaps_a_label									<< xf::no_data_source;
	pfd_screen->adi_io->speed_flaps_a_speed									<< xf::no_data_source;
	pfd_screen->adi_io->speed_flaps_b_label									<< xf::no_data_source;
	pfd_screen->adi_io->speed_flaps_b_speed									<< xf::no_data_source;
	pfd_screen->adi_io->orientation_pitch									<< sim_airplane_io->real_orientation_pitch;
	pfd_screen->adi_io->orientation_roll									<< sim_airplane_io->real_orientation_roll;
	pfd_screen->adi_io->orientation_heading_magnetic						<< sim_airplane_io->real_orientation_heading_true; // TODO should be magnetic
	pfd_screen->adi_io->orientation_heading_true							<< sim_airplane_io->real_orientation_heading_true;
	pfd_screen->adi_io->orientation_heading_numbers_visible					<< xf::ConstantSource (true);
	pfd_screen->adi_io->track_lateral_magnetic								<< sim_airplane_io->real_track_lateral_true; // TODO should be magnetic
	pfd_screen->adi_io->track_lateral_true									<< sim_airplane_io->real_track_lateral_true;
	pfd_screen->adi_io->track_vertical										<< sim_airplane_io->real_track_vertical;
	pfd_screen->adi_io->fpv_visible											<< xf::ConstantSource (true);
	pfd_screen->adi_io->slip_skid											<< sim_airplane_io->real_slip_skid;
	pfd_screen->adi_io->aoa_alpha											<< sim_airplane_io->real_aoa_alpha;
	pfd_screen->adi_io->aoa_alpha_maximum									<< sim_airplane_io->real_aoa_alpha_maximum;
	pfd_screen->adi_io->aoa_alpha_visible									<< xf::ConstantSource (true);
	pfd_screen->adi_io->altitude_amsl										<< air_data_computer_io->altitude_amsl;
	pfd_screen->adi_io->altitude_amsl_lookahead								<< air_data_computer_io->altitude_amsl_lookahead;
	pfd_screen->adi_io->altitude_agl_serviceable							<< xf::ConstantSource (true);
	pfd_screen->adi_io->altitude_agl										<< sim_airplane_io->real_altitude_agl;
	pfd_screen->adi_io->decision_height_type								<< xf::no_data_source;
	pfd_screen->adi_io->decision_height_setting								<< xf::no_data_source;
	pfd_screen->adi_io->decision_height_amsl								<< xf::no_data_source;
	pfd_screen->adi_io->landing_amsl										<< xf::ConstantSource (0_ft);
	pfd_screen->adi_io->vertical_speed										<< air_data_computer_io->vertical_speed;
	pfd_screen->adi_io->vertical_speed_energy_variometer					<< xf::no_data_source;
	pfd_screen->adi_io->pressure_qnh										<< xf::ConstantSource (1013_hPa);
	pfd_screen->adi_io->pressure_display_hpa								<< xf::ConstantSource (true);
	pfd_screen->adi_io->pressure_use_std									<< xf::ConstantSource (true);
	pfd_screen->adi_io->flight_director_serviceable							<< xf::ConstantSource (true);
	pfd_screen->adi_io->flight_director_active_name							<< xf::no_data_source;
	pfd_screen->adi_io->flight_director_cmd_visible							<< xf::ConstantSource (false);
	pfd_screen->adi_io->flight_director_cmd_altitude						<< xf::no_data_source;
	pfd_screen->adi_io->flight_director_cmd_altitude_acquired				<< xf::no_data_source;
	pfd_screen->adi_io->flight_director_cmd_ias								<< xf::no_data_source;
	pfd_screen->adi_io->flight_director_cmd_mach							<< xf::no_data_source;
	pfd_screen->adi_io->flight_director_cmd_vertical_speed					<< xf::no_data_source;
	pfd_screen->adi_io->flight_director_cmd_fpa								<< xf::no_data_source;
	pfd_screen->adi_io->flight_director_guidance_visible					<< xf::ConstantSource (false);
	pfd_screen->adi_io->flight_director_guidance_pitch						<< xf::ConstantSource (0_deg);
	pfd_screen->adi_io->flight_director_guidance_roll						<< xf::ConstantSource (0_deg);
	pfd_screen->adi_io->control_surfaces_visible							<< xf::ConstantSource (true);
	pfd_screen->adi_io->control_surfaces_ailerons							<< virtual_joystick_io->x_axis;
	pfd_screen->adi_io->control_surfaces_elevator							<< virtual_joystick_io->y_axis;
	pfd_screen->adi_io->navaid_reference_visible							<< xf::no_data_source;
	pfd_screen->adi_io->navaid_course_magnetic								<< xf::no_data_source;
	pfd_screen->adi_io->navaid_type_hint									<< xf::no_data_source;
	pfd_screen->adi_io->navaid_identifier									<< xf::no_data_source;
	pfd_screen->adi_io->navaid_distance										<< xf::no_data_source;
	pfd_screen->adi_io->flight_path_deviation_lateral_serviceable			<< xf::ConstantSource (true);
	pfd_screen->adi_io->flight_path_deviation_lateral_approach				<< xf::no_data_source;
	pfd_screen->adi_io->flight_path_deviation_lateral_flight_path			<< xf::no_data_source;
	pfd_screen->adi_io->flight_path_deviation_vertical_serviceable			<< xf::ConstantSource (true);
	pfd_screen->adi_io->flight_path_deviation_vertical						<< xf::no_data_source;
	pfd_screen->adi_io->flight_path_deviation_vertical_approach				<< xf::no_data_source;
	pfd_screen->adi_io->flight_path_deviation_vertical_flight_path			<< xf::no_data_source;
	pfd_screen->adi_io->flight_path_deviation_mixed_mode					<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_hint_visible							<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_hint									<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_fma_visible								<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_fma_speed_hint							<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_fma_speed_armed_hint					<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_fma_lateral_hint						<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_fma_lateral_armed_hint					<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_fma_vertical_hint						<< xf::no_data_source;
	pfd_screen->adi_io->flight_mode_fma_vertical_armed_hint					<< xf::no_data_source;
	pfd_screen->adi_io->tcas_resolution_advisory_pitch_minimum				<< xf::no_data_source;
	pfd_screen->adi_io->tcas_resolution_advisory_pitch_maximum				<< xf::no_data_source;
	pfd_screen->adi_io->tcas_resolution_advisory_vertical_speed_minimum		<< xf::no_data_source;
	pfd_screen->adi_io->tcas_resolution_advisory_vertical_speed_maximum		<< xf::no_data_source;
	pfd_screen->adi_io->warning_novspd_flag									<< xf::no_data_source;
	pfd_screen->adi_io->warning_ldgalt_flag									<< xf::no_data_source;
	pfd_screen->adi_io->warning_pitch_disagree								<< xf::no_data_source;
	pfd_screen->adi_io->warning_roll_disagree								<< xf::no_data_source;
	pfd_screen->adi_io->warning_ias_disagree								<< xf::no_data_source;
	pfd_screen->adi_io->warning_altitude_disagree							<< xf::no_data_source;
	pfd_screen->adi_io->warning_roll										<< xf::no_data_source;
	pfd_screen->adi_io->warning_slip_skid									<< xf::no_data_source; // TODO
	pfd_screen->adi_io->style_old											<< xf::ConstantSource (false);
	pfd_screen->adi_io->style_show_metric									<< xf::ConstantSource (true);

	pfd_screen->hsi_io->display_mode										<< xf::ConstantSource (hsi::DisplayMode::Auxiliary);
	pfd_screen->hsi_io->range												<< xf::ConstantSource (60_nmi);
	pfd_screen->hsi_io->speed_gs											<< sim_airplane_io->real_ground_speed;
	pfd_screen->hsi_io->speed_tas											<< air_data_computer_io->speed_tas;
	pfd_screen->hsi_io->cmd_visible											<< xf::ConstantSource (false);
	pfd_screen->hsi_io->cmd_line_visible									<< xf::ConstantSource (false);
	pfd_screen->hsi_io->cmd_heading_magnetic								<< xf::no_data_source;
	pfd_screen->hsi_io->cmd_track_magnetic									<< xf::no_data_source;
	pfd_screen->hsi_io->cmd_use_trk											<< xf::ConstantSource (true);
	pfd_screen->hsi_io->target_altitude_reach_distance						<< xf::no_data_source;
	pfd_screen->hsi_io->orientation_heading_magnetic						<< sim_airplane_io->real_orientation_heading_true; // TODO magnetic
	pfd_screen->hsi_io->orientation_heading_true							<< sim_airplane_io->real_orientation_heading_true;
	pfd_screen->hsi_io->heading_mode										<< xf::ConstantSource (hsi::HeadingMode::Magnetic);
	pfd_screen->hsi_io->home_true_direction									<< xf::no_data_source;
	pfd_screen->hsi_io->home_track_visible									<< xf::ConstantSource (true);
	pfd_screen->hsi_io->home_distance_vlos									<< xf::no_data_source;
	pfd_screen->hsi_io->home_distance_ground								<< xf::no_data_source;
	pfd_screen->hsi_io->home_distance_vertical								<< xf::no_data_source;
	pfd_screen->hsi_io->home_position_longitude								<< xf::ConstantSource (0_deg);
	pfd_screen->hsi_io->home_position_latitude								<< xf::ConstantSource (0_deg);
	pfd_screen->hsi_io->position_longitude									<< sim_airplane_io->real_position_longitude;
	pfd_screen->hsi_io->position_latitude									<< sim_airplane_io->real_position_latitude;
	pfd_screen->hsi_io->position_source										<< xf::ConstantSource ("SIM");
	pfd_screen->hsi_io->track_visible										<< xf::ConstantSource (true);
	pfd_screen->hsi_io->track_lateral_magnetic								<< sim_airplane_io->real_track_lateral_true; // TODO magnetic
	pfd_screen->hsi_io->track_lateral_rotation								<< xf::no_data_source;
	pfd_screen->hsi_io->track_center_on_track								<< xf::ConstantSource (true);
	pfd_screen->hsi_io->course_visible										<< xf::ConstantSource (false);
	pfd_screen->hsi_io->course_setting_magnetic								<< xf::no_data_source;
	pfd_screen->hsi_io->course_deviation									<< xf::no_data_source;
	pfd_screen->hsi_io->course_to_flag										<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_selected_reference							<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_selected_identifier							<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_selected_distance							<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_selected_eta									<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_selected_course_magnetic						<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_left_type									<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_left_reference								<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_left_identifier								<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_left_distance								<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_left_initial_bearing_magnetic				<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_right_type									<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_right_reference								<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_right_identifier								<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_right_distance								<< xf::no_data_source;
	pfd_screen->hsi_io->navaid_right_initial_bearing_magnetic				<< xf::no_data_source;
	pfd_screen->hsi_io->navigation_required_performance						<< xf::no_data_source;
	pfd_screen->hsi_io->navigation_actual_performance						<< xf::no_data_source;
	pfd_screen->hsi_io->wind_from_magnetic									<< xf::no_data_source;
	pfd_screen->hsi_io->wind_speed_tas										<< xf::no_data_source;
	pfd_screen->hsi_io->localizer_id										<< xf::no_data_source;
	pfd_screen->hsi_io->tcas_on												<< xf::no_data_source;
	pfd_screen->hsi_io->tcas_range											<< xf::no_data_source;
	pfd_screen->hsi_io->features_fix										<< xf::ConstantSource (true);
	pfd_screen->hsi_io->features_vor										<< xf::ConstantSource (true);
	pfd_screen->hsi_io->features_dme										<< xf::ConstantSource (true);
	pfd_screen->hsi_io->features_ndb										<< xf::ConstantSource (true);
	pfd_screen->hsi_io->features_loc										<< xf::ConstantSource (true);
	pfd_screen->hsi_io->features_arpt										<< xf::ConstantSource (true);

	pfd_screen->engine_l_power_io->value									<< sim_airplane_io->engine_left_power;
	pfd_screen->engine_l_power_io->target									<< sim_airplane_io->requested_engine_left_power;

	pfd_screen->engine_l_speed_io->value									<< xf::no_data_source;

	pfd_screen->engine_l_thrust_io->value									<< sim_airplane_io->engine_left_thrust;
	pfd_screen->engine_l_thrust_io->reference								<< xf::ConstantSource (10_N);

	pfd_screen->engine_l_temperature_io->value								<< xf::no_data_source;

	pfd_screen->engine_l_current_io->value									<< xf::no_data_source;

	pfd_screen->engine_l_voltage_io->value									<< xf::no_data_source;

	pfd_screen->engine_l_vibration_io->value								<< xf::no_data_source;

	pfd_screen->engine_r_power_io->value									<< sim_airplane_io->engine_right_power;
	pfd_screen->engine_r_power_io->target									<< sim_airplane_io->requested_engine_right_power;

	pfd_screen->engine_r_speed_io->value									<< xf::no_data_source;

	pfd_screen->engine_r_thrust_io->value									<< sim_airplane_io->engine_right_thrust;
	pfd_screen->engine_r_thrust_io->reference								<< xf::ConstantSource (10_N);

	pfd_screen->engine_r_temperature_io->value								<< xf::no_data_source;

	pfd_screen->engine_r_current_io->value									<< xf::no_data_source;

	pfd_screen->engine_r_voltage_io->value									<< xf::no_data_source;

	pfd_screen->engine_r_vibration_io->value								<< xf::no_data_source;

	pfd_screen->gear_io->requested_down										<< xf::ConstantSource (true);
	pfd_screen->gear_io->nose_up											<< xf::ConstantSource (false);
	pfd_screen->gear_io->nose_down											<< xf::ConstantSource (true);
	pfd_screen->gear_io->left_up											<< xf::ConstantSource (false);
	pfd_screen->gear_io->left_down											<< xf::ConstantSource (true);
	pfd_screen->gear_io->right_up											<< xf::ConstantSource (false);
	pfd_screen->gear_io->right_down											<< xf::ConstantSource (true);

	pfd_screen->vertical_trim_io->trim_value								<< xf::no_data_source;
	pfd_screen->vertical_trim_io->trim_reference							<< xf::ConstantSource (0.5);
	pfd_screen->vertical_trim_io->trim_reference_minimum					<< xf::ConstantSource (0.35);
	pfd_screen->vertical_trim_io->trim_reference_maximum					<< xf::ConstantSource (0.6);

	// Other:
	_sim_airplane.emplace (std::move (sim_airplane_io), _logger.with_scope ("sim-airplane"), "sim");
	_virtual_joystick.emplace (std::move (virtual_joystick_io), "virtual-joystick");
	auto const& simulation = (*_sim_airplane)->simulation();

	auto const prandtl_location = xf::SpaceVector<si::Length, xf::BodyFrame> { 0_m, -5_cm, 0_m };
	auto const temperature_probe_location = xf::SpaceVector<si::Length, xf::BodyFrame> { 0_m, -2_cm, 0_m };

	// Sensors:
	_pressure_sensor_static.emplace (
		simulation,
		VirtualPressureSensor::Static,
		prandtl_location,
		std::move (pressure_sensor_static_io),
		_logger,
		"pressure-sensor.static"
	);
	_pressure_sensor_total.emplace (
		simulation,
		VirtualPressureSensor::Pitot,
		prandtl_location,
		std::move (pressure_sensor_total_io),
		_logger,
		"pressure-sensor.total"
	);
	_temperature_sensor_total.emplace (
		simulation,
		temperature_probe_location,
		std::move (temperature_sensor_total_io),
		_logger,
		"temperature-sensor.total"
	);

	// Systems:
	_air_data_computer.emplace (std::move (air_data_computer_io), nullptr, _logger, "air-data-computer");

	// When all modules are initialized and connected, create instruments:
	backup_screen->create_instruments();
	pfd_screen->create_instruments();

	auto& main_loop = **_loop;

	// Register all instruments in the processing loop:
	for (auto* screen: std::initializer_list<xf::Screen*> { &*pfd_screen, &*backup_screen })
		for (auto& disclosure: screen->instrument_tracker())
			main_loop.register_module (disclosure.registrant());

	// Register non-instrument modules:
	main_loop.register_module (*_loop);
	main_loop.register_module (*_sim_airplane);
	main_loop.register_module (*_virtual_joystick);
	main_loop.register_module (*_pressure_sensor_static);
	main_loop.register_module (*_pressure_sensor_total);
	main_loop.register_module (*_air_data_computer);
	main_loop.start();

	pfd_screen->show();
	backup_screen->show();
}


std::unique_ptr<xf::Machine>
xefis_machine (xf::Xefis& xefis)
{
	return std::make_unique<SimulationMachine> (xefis);
}

