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
#include <thread>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/configurator/configurator_widget.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qutils.h>

// Local:
#include "loop.h"


Loop::Loop (xf::Machine* machine, xf::Xefis*):
	ProcessingLoop (machine, "Cthulhu GCS main", 30_Hz)
{
#if 0
	xf::AirframeDefinition airframe_definition {
		mydeclval<xf::Flaps>(),
		mydeclval<xf::Spoilers>(),
		mydeclval<xf::Lift>(),
		mydeclval<xf::Drag>(),
		// Wing area:
		15_m2,
		// Wing chord:
		1_m,
		// Load factor limits:
		{ -2.0, 2.0 },
		// Safe AOA correction from maximum AOA:
		-1_deg,
	};
#endif

#if 0
	_airframe = std::make_unique<xf::Airframe> (airframe_definition);
#endif
	_navaid_storage = std::make_unique<xf::NavaidStorage>();
	_work_performer = std::make_unique<xf::WorkPerformer> (std::thread::hardware_concurrency());

	QDomElement joystick_config = xf::load_xml (QFile ("machines/cthulhu_shared/xmls/joystick-hotas-stick.xml"));
	QDomElement throttle_config = xf::load_xml (QFile ("machines/cthulhu_shared/xmls/joystick-hotas-throttle.xml"));
	QDomElement pedals_config = xf::load_xml (QFile ("machines/cthulhu_shared/xmls/joystick-saitek-pedals.xml"));

	auto joystick_io = std::make_unique<JoystickInputIO>();
	auto throttle_io = std::make_unique<JoystickInputIO>();
	auto pedals_io = std::make_unique<JoystickInputIO>();

	auto link_io_tx = std::make_unique<CthulhuGCS_Tx_LinkIO>();
	auto link_protocol_tx = std::make_unique<CthulhuGCS_Tx_LinkProtocol> (*link_io_tx);

	auto link_io_rx = std::make_unique<CthulhuGCS_Rx_LinkIO>();
	auto link_protocol_rx = std::make_unique<CthulhuGCS_Rx_LinkProtocol> (*link_io_rx);

	auto test_generator_io = std::make_unique<TestGeneratorIO>();

	link_io_tx->send_frequency = 100_Hz;

	link_io_rx->reacquire_after = 150_ms;
	link_io_rx->failsafe_after = 500_ms;

	auto adc_io = std::make_unique<AirDataComputerIO>();
	adc_io->ias_valid_minimum = 30_kt;
	adc_io->ias_valid_maximum = 900_kt;

#if 0
	auto adi_io = std::make_unique<ADI_IO>();
	adi_io->speed_ias_serviceable.set_fallback (true);
	adi_io->speed_ias.set_fallback (120_kt);
	adi_io->speed_ias_lookahead.set_fallback (125_kt);
	adi_io->speed_ias_minimum.set_fallback (90_kt);
	adi_io->speed_ias_minimum_maneuver.set_fallback (94_kt);
	adi_io->speed_ias_maximum_maneuver.set_fallback (140_kt);
	adi_io->speed_ias_maximum.set_fallback (150_kt);
	adi_io->speed_mach.set_fallback (0.23);
	adi_io->speed_ground.set_fallback (140_kt);
	adi_io->speed_v1.set_fallback (91_kt);
	adi_io->speed_vr.set_fallback (99_kt);
	adi_io->speed_vref.set_fallback (124_kt);
	//xf::PropertyIn<std::string>		speed_flaps_up_label
	//xf::PropertyIn<si::Velocity>	speed_flaps_up_speed
	//xf::PropertyIn<std::string>		speed_flaps_a_label
	//xf::PropertyIn<si::Velocity>	speed_flaps_a_speed
	//xf::PropertyIn<std::string>		speed_flaps_b_label
	//xf::PropertyIn<si::Velocity>	speed_flaps_b_speed
	adi_io->orientation_serviceable.set_fallback (true);
	adi_io->orientation_pitch.set_fallback (1_deg);
	adi_io->orientation_roll.set_fallback (0_deg);
	adi_io->orientation_heading_magnetic.set_fallback (30_deg);
	adi_io->orientation_heading_true.set_fallback (32_deg);
	adi_io->orientation_heading_numbers_visible.set_fallback (true);

	this->adi = load_module<ADI> (std::move (adi_io), *_work_performer);
	//this->hsi_aux = load_module<HSI> (xefis, _navaid_storage.get(), "aux");
	//this->horizontal_trim = load_module<HorizontalTrim>();

	this->adi->resize (QSize (680, 470));
	this->adi->show();
	//this->hsi_aux->show();
	//this->horizontal_trim->show();
#endif

	auto adi_io = std::make_unique<ADI_IO>();
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
	adi_io->raising_runway_visibility							= 1000_ft; // TODO should only appear in landing mode
	adi_io->raising_runway_threshold							= 250_ft;
	adi_io->aoa_visibility_threshold							= 17.5_deg;
	adi_io->show_mach_above										= 0.4;
	adi_io->power_eq_1000_fpm									= 1000_W;
	// IO:
	adi_io->weight_on_wheels									<< xf::ConstantSource (false);
	adi_io->speed_ias_serviceable								<< xf::ConstantSource (true);
	adi_io->speed_ias											<< test_generator_io->create_property<si::Velocity> ("test/speed/ias", 0_kt, { 0_kt, 300_kt }, 10_kt / 1_s);
	adi_io->speed_ias_lookahead									<< test_generator_io->create_property<si::Velocity> ("test/speed/ias.lookahead", 25_kt, { 0_kt, 300_kt }, 8_kt / 1_s);
	adi_io->speed_ias_minimum									<< test_generator_io->create_property<si::Velocity> ("speed/ias.minimum", 60_kt, { 50_kt, 70_kt }, 3_kt / 1_s);
	adi_io->speed_ias_minimum_maneuver							<< test_generator_io->create_property<si::Velocity> ("speed/ias.minimum.maneuver", 65_kt, { 55_kt, 72_kt }, 3_kt / 1_s);
	adi_io->speed_ias_maximum_maneuver							<< test_generator_io->create_property<si::Velocity> ("speed/ias.maximum.maneuver", 245_kt, { 238_kt, 245_kt }, 3_kt / 1_s);
	adi_io->speed_ias_maximum									<< test_generator_io->create_property<si::Velocity> ("speed/ias.maximum", 250_kt, { 240_kt, 260_kt }, 3_kt / 1_s);
	adi_io->speed_mach											<< test_generator_io->create_property<double> ("speed/mach", 0.0f, { 0.0f, 0.85f }, 0.025f / 1_s);
	adi_io->speed_ground										<< test_generator_io->create_property<si::Velocity> ("speed/ground-speed", 0_kt, { 0_kt, 400_kt }, 13_kt / 1_s);
	adi_io->speed_v1											<< test_generator_io->create_property<si::Velocity> ("test/speed-bugs/v1", 80_kt, { 78_kt, 82_kt }, 1_kt / 1_s);
	adi_io->speed_vr											<< test_generator_io->create_property<si::Velocity> ("test/speed-bugs/vr", 88_kt, { 86_kt, 89_kt }, 1_kt / 1_s);
	adi_io->speed_vref											<< test_generator_io->create_property<si::Velocity> ("test/speed-bugs/vref", 95_kt, { 94_kt, 96_kt }, 0.1_kt / 1_s);
	adi_io->speed_flaps_up_label								<< xf::ConstantSource<std::string> ("UP");
	adi_io->speed_flaps_up_speed								<< xf::ConstantSource (140_kt);
	adi_io->speed_flaps_a_label									<< xf::ConstantSource<std::string> ("1"); // TODO _b_ and _a_? maybe "upper" and "lower"?
	adi_io->speed_flaps_a_speed									<< xf::ConstantSource (120_kt);
	adi_io->speed_flaps_b_label									<< xf::ConstantSource<std::string> ("5");
	adi_io->speed_flaps_b_speed									<< xf::ConstantSource (110_kt);
	adi_io->orientation_serviceable								<< xf::ConstantSource (true);
	adi_io->orientation_pitch									<< test_generator_io->create_property<si::Angle> ("test/orientation/pitch", 0_deg, { -90_deg, 90_deg }, 8_deg / 1_s);
	adi_io->orientation_roll									<< test_generator_io->create_property<si::Angle> ("test/orientation/roll", 0_deg, { -180_deg, +180_deg }, 1.5_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	adi_io->orientation_heading_magnetic						<< test_generator_io->create_property<si::Angle> ("test/orientation/heading.magnetic", 0_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	adi_io->orientation_heading_true							<< test_generator_io->create_property<si::Angle> ("test/orientation/heading.true", 10_deg, { 0_deg, 360_deg }, 2_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	adi_io->orientation_heading_numbers_visible					<< xf::ConstantSource (true);
	adi_io->track_lateral_magnetic								<< test_generator_io->create_property<si::Angle> ("track/lateral.magnetic", 9_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	adi_io->track_lateral_true									<< test_generator_io->create_property<si::Angle> ("track/lateral.true", 19_deg, { 0_deg, 360_deg }, 22_deg / 1_s, TestGeneratorIO::BorderCondition::Periodic);
	adi_io->track_vertical										<< test_generator_io->create_property<si::Angle> ("track/vertical", 0_deg, { -13_deg, 13_deg }, 1_deg / 1_s);
	adi_io->fpv_visible											<< xf::ConstantSource (true);
	adi_io->slip_skid											<< test_generator_io->create_property<si::Angle> ("slip-skid/angle", 0_deg, { -5_deg, 5_deg }, 0.5_deg / 1_s);
	adi_io->aoa_alpha											<< test_generator_io->create_property<si::Angle> ("aoa/alpha", 0_deg, { -2_deg, 15_deg }, 1_deg / 1_s);
	adi_io->aoa_alpha_maximum									<< test_generator_io->create_property<si::Angle> ("aoa/alpha.maximum", 13_deg, { 13_deg, 15_deg }, 0.25_deg / 1_s);
	adi_io->aoa_alpha_visible									<< xf::ConstantSource (true);
	adi_io->altitude_amsl_serviceable							<< xf::ConstantSource (true);
	adi_io->altitude_amsl										<< test_generator_io->create_property<si::Length> ("altitude/amsl", -200_ft, { -200_ft, 2000_ft }, 2000_ft / 1_min);
	adi_io->altitude_amsl_lookahead								<< test_generator_io->create_property<si::Length> ("altitude/amsl.lookahead", 10_ft, { 0_ft, 2000_ft }, 100_ft / 1_min);
	adi_io->altitude_agl_serviceable							<< xf::ConstantSource (true);
	adi_io->altitude_agl										<< test_generator_io->create_property<si::Length> ("altitude/agl", -4_ft, { -4_ft, 30_m }, 100_ft / 1_min);
	adi_io->altitude_minimums_type								<< xf::ConstantSource ("BARO");
	adi_io->altitude_minimums_setting							<< xf::ConstantSource (300_ft);
	adi_io->altitude_minimums_amsl								<< xf::ConstantSource (300_ft);
	adi_io->altitude_landing_amsl								<< xf::ConstantSource (140_ft);
	adi_io->vertical_speed_serviceable							<< xf::ConstantSource (true);
	adi_io->vertical_speed										<< test_generator_io->create_property<si::Velocity> ("vertical-speed/speed", 0_fpm, { -6000_fpm, +6000_fpm }, 100_fpm / 1_s);
	adi_io->vertical_speed_energy_variometer					<< test_generator_io->create_property<si::Power> ("vertical-speed/energy-variometer", 0_W, { -1000_W, +1000_W }, 100_W / 1_s);
	adi_io->pressure_qnh										<< xf::ConstantSource (1013_hPa);
	adi_io->pressure_display_hpa								<< xf::ConstantSource (true);
	adi_io->pressure_use_std									<< xf::ConstantSource (true);
	// TODO xf::PropertyIn<std::string>	adi_io->flight_director_active_name						{ ... } // "L", "R", "C"(enter)?
	adi_io->flight_director_serviceable							<< xf::ConstantSource (true);
	adi_io->flight_director_cmd_visible							<< xf::ConstantSource (true);
	adi_io->flight_director_cmd_altitude						<< xf::ConstantSource (1000_ft);
	adi_io->flight_director_cmd_altitude_acquired				<< xf::ConstantSource (false);
	adi_io->flight_director_cmd_ias								<< xf::ConstantSource (100_kt);
	adi_io->flight_director_cmd_mach							<< xf::ConstantSource (0.34);
	adi_io->flight_director_cmd_vertical_speed					<< xf::ConstantSource (1500_fpm);
	adi_io->flight_director_cmd_fpa								<< xf::ConstantSource (5_deg);
	adi_io->flight_director_guidance_visible					<< xf::ConstantSource (true);
	adi_io->flight_director_guidance_pitch						<< xf::ConstantSource (2.5_deg);
	adi_io->flight_director_guidance_roll						<< xf::ConstantSource (0_deg);
	adi_io->control_stick_visible								<< xf::ConstantSource (true);
	adi_io->control_stick_pitch									<< xf::ConstantSource (2_deg);
	adi_io->control_stick_roll									<< xf::ConstantSource (2_deg);
	adi_io->navaid_reference_visible							<< xf::ConstantSource (true);
	adi_io->navaid_course_magnetic								<< xf::ConstantSource (150_deg);
	adi_io->navaid_type_hint									<< xf::ConstantSource ("VOR");
	adi_io->navaid_identifier									<< xf::ConstantSource ("WRO");
	adi_io->navaid_distance										<< xf::ConstantSource (1.5_nmi);
	adi_io->flight_path_deviation_lateral_serviceable			<< xf::ConstantSource (true);
	adi_io->flight_path_deviation_lateral_approach				<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/lateral/approach", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	adi_io->flight_path_deviation_lateral_flight_path			<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/lateral/flight-path", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	adi_io->flight_path_deviation_vertical_serviceable			<< xf::ConstantSource (true);
	adi_io->flight_path_deviation_vertical						<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/vertical/deviation", 0_deg, { -5_deg, 5_deg }, 1_deg / 1_s);
	adi_io->flight_path_deviation_vertical_approach				<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/vertical/approach", 0_deg, { -5_deg, 5_deg }, 2_deg / 1_s);
	adi_io->flight_path_deviation_vertical_flight_path			<< test_generator_io->create_property<si::Angle> ("flight-path-deviation/vertical/flight-path", 0_deg, { -5_deg, 5_deg }, 3_deg / 1_s);
	adi_io->flight_path_deviation_mixed_mode					<< xf::ConstantSource (true);
	adi_io->flight_mode_hint_visible							<< xf::ConstantSource (true);
	adi_io->flight_mode_hint									<< xf::ConstantSource ("TEST");
	adi_io->flight_mode_fma_visible								<< xf::ConstantSource (true);
	adi_io->flight_mode_fma_speed_hint							<< xf::ConstantSource ("THR REF");
	adi_io->flight_mode_fma_speed_armed_hint					<< xf::ConstantSource ("SPD");
	adi_io->flight_mode_fma_lateral_hint						<< xf::ConstantSource ("CMD TRK");
	adi_io->flight_mode_fma_lateral_armed_hint					<< xf::ConstantSource ("ILS TRK");
	adi_io->flight_mode_fma_vertical_hint						<< xf::ConstantSource ("CMD FPA");
	adi_io->flight_mode_fma_vertical_armed_hint					<< xf::ConstantSource ("G/S");
	adi_io->tcas_resolution_advisory_pitch_minimum				<< xf::ConstantSource (-45_deg);
	adi_io->tcas_resolution_advisory_pitch_maximum				<< xf::ConstantSource (80_deg);
	adi_io->tcas_resolution_advisory_vertical_speed_minimum		<< xf::ConstantSource (-3000_fpm);
	adi_io->tcas_resolution_advisory_vertical_speed_maximum		<< xf::ConstantSource (10000_fpm);
#if 0
	adi_io->warning_novspd_flag									<< xf::ConstantSource (true);
	adi_io->warning_ldgalt_flag									<< xf::ConstantSource (true);
	adi_io->warning_pitch_disagree								<< xf::ConstantSource (true);
	adi_io->warning_roll_disagree								<< xf::ConstantSource (true);
	adi_io->warning_ias_disagree								<< xf::ConstantSource (true);
	adi_io->warning_altitude_disagree							<< xf::ConstantSource (true);
	adi_io->warning_roll										<< xf::ConstantSource (true);
	adi_io->warning_slip_skid									<< xf::ConstantSource (true);
#endif
	adi_io->style_old											<< xf::ConstantSource (false);
	adi_io->style_show_metric									<< xf::ConstantSource (true);

	auto gear_io = std::make_unique<GearIO>();
	gear_io->requested_down << xf::ConstantSource (true);
	gear_io->nose_up << xf::ConstantSource (false);
	gear_io->nose_down << xf::ConstantSource (true);
	gear_io->left_up << xf::ConstantSource (false);
	gear_io->left_down << xf::ConstantSource (true);
	gear_io->right_up << xf::ConstantSource (false);
	gear_io->right_down << xf::ConstantSource (true);

	auto some_label_io = std::make_unique<LabelIO>();
	some_label_io->label = "Nergal i Hela";
	some_label_io->font_scale = 1.0;
	some_label_io->color = Qt::white;

	// TODO wrapper na Instrumenty, który ma w środku Instrument oraz RegistrationProof i przeciążony operator->(). Tak że w przypadku skasowania
	// najpierw będzie skasowany RegistrationProof, a potem Instrument. NO CHYBA ŻE NIE OWNUJEMY INSTRUMENTU… :(
	// Przykładowy kod do tego:
	// template<class Value>
	//	class Registrable
	//	{
	//		Value								_value;
	//		xf::RegistrationProof<Value, int>	_proof;
	//	}
	//
	//struct Instrument
	//{
	//	void register_in (xf::Registry<Instrument, int> registry)
	//	{
	//		_proof = registry.register_object (*this, 25);
	//	}
	//
	//	void f()
	//	{
	//		std::cout << "Instrument[" << this << "]::f()\n";
	//	}
	//
	//	xf::RegistrationProof<Instrument, int> _proof;
	//};
	//
	//
	//struct Screen
	//{
	//	xf::Registry<Instrument, int> _registry;
	//
	//	void
	//	register_instrument (Instrument& instrument)
	//	{
	//		instrument.register_in (_registry);
	//	}
	//
	//	void
	//	sweep()
	//	{
	//		for (auto& instrument: _registry)
	//			instrument.f();
	//	}
	//};
	//
	//
	//int main()
	//{
	//	auto s1 = std::make_unique<Screen>();
	//
	//	Instrument i1, i2, i3, i4, i5;
	//
	//	s1->register_instrument (i1);
	//	s1->register_instrument (i2);
	//	s1->register_instrument (i3);
	//	s1->register_instrument (i4);
	//	s1->register_instrument (i5);
	//
	//	s1->sweep();
	//
	//	return 0;
	//}

	this->joystick_input = load_module<WarthogStick> (std::move (joystick_io), joystick_config, _logger, "stick");
	this->throttle_input = load_module<JoystickInput> (std::move (throttle_io), throttle_config, _logger, "throttle");
	this->pedals_input = load_module<JoystickInput> (std::move (pedals_io), pedals_config, _logger, "pedals");

	this->link_tx = load_module<Link> (std::move (link_io_tx), std::move (link_protocol_tx), _logger, "link-tx");
	this->link_rx = load_module<Link> (std::move (link_io_rx), std::move (link_protocol_rx), _logger, "link-rx");

#if 0
	this->adc = load_module<AirDataComputer> (std::move (adc_io), _airframe.get());
#endif

	this->test_generator = load_module<TestGenerator> (std::move (test_generator_io));

	xf::ScreenSpec spec { QRect { 0, 0, 1366, 768 }, 15_in, 30_Hz, 0.22_mm, 2.1_mm };
	spec.set_scale (1.5f);
	_pfd_screen = std::make_unique<xf::Screen> (spec);

	this->adi = load_module<ADI> (std::move (adi_io), "adi");
	this->adi_registration_proof = _pfd_screen->register_instrument (*this->adi);
	_pfd_screen->set (*this->adi, { 0.0f, 0.0f, 0.5f, 0.62f });

	this->gear = load_module<Gear> (std::move (gear_io), "gear");
	this->gear_registration_proof = _pfd_screen->register_instrument (*this->gear);
	_pfd_screen->set (*this->gear, { 0.5, 0.0f, 0.5f, 1.0f });

	auto configurator_widget = new xf::ConfiguratorWidget (*machine, nullptr);
	configurator_widget->show();

	start();
}

