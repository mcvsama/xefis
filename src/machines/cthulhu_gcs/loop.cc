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
#include <xefis/utility/qdom.h>

// Local:
#include "loop.h"


Loop::Loop (xf::Machine* machine, xf::Xefis*):
	ProcessingLoop (machine, 30_Hz)
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

	link_io_tx->send_frequency = 100_Hz;

	link_io_rx->reacquire_after = 150_ms;
	link_io_rx->failsafe_after = 500_ms;

	auto adc_io = std::make_unique<AirDataComputerIO>();
	adc_io->ias_valid_minimum = 30_kt;
	adc_io->ias_valid_maximum = 900_kt;

	this->joystick_input = load_module<WarthogStick> (std::move (joystick_io), joystick_config, _logger, "stick");
	this->throttle_input = load_module<JoystickInput> (std::move (throttle_io), throttle_config, _logger, "throttle");
	this->pedals_input = load_module<JoystickInput> (std::move (pedals_io), pedals_config, _logger, "pedals");

	this->link_tx = load_module<Link> (std::move (link_io_tx), std::move (link_protocol_tx), _logger, "link-tx");
	this->link_rx = load_module<Link> (std::move (link_io_rx), std::move (link_protocol_rx), _logger, "link-rx");

#if 0
	this->adc = load_module<AirDataComputer> (std::move (adc_io), _airframe.get());
#endif

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

	_pfd_screen = std::make_unique<xf::Screen> (QRect (0, 0, 400, 400), 10_Hz);

	auto some_label_io = std::make_unique<LabelIO>();
	some_label_io->label = "Nergal i Hela";
	some_label_io->font_scale = 1.0;
	some_label_io->color = Qt::white;

	auto gear_io = std::make_unique<GearIO>();
	gear_io->requested_down << xf::ConstantSource (true);
	gear_io->nose_up << xf::ConstantSource (false);
	gear_io->nose_down << xf::ConstantSource (true);
	gear_io->left_up << xf::ConstantSource (false);
	gear_io->left_down << xf::ConstantSource (true);
	gear_io->right_up << xf::ConstantSource (false);
	gear_io->right_down << xf::ConstantSource (true);

	// TODO wsparcie do rejestracji instrumentów tak, żeby nie trzeba było osobno trzymać RegistrationProof: patrz src/__registration__.cc
	// TODO wrapper na Instrumenty, który ma w środku Instrument oraz RegistrationProof i przeciążony operator->(). Tak że w przypadku skasowania
	// najpierw będzie skasowany RegistrationProof, a potem Instrument. NO CHYBA ŻE NIE OWNUJEMY INSTRUMENTU… :(
	// Przykładowy kod do tego:
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

	this->some_label = load_module<Label> (std::move (some_label_io), "some label");
	this->some_label_registration_proof = _pfd_screen->register_instrument (*this->some_label);
	_pfd_screen->set (*this->some_label, QRect (0, 0, 100, 200));

	this->gear = load_module<Gear> (std::move (gear_io), "gear");
	this->gear_registration_proof = _pfd_screen->register_instrument (*this->gear);
	_pfd_screen->set (*this->gear, QRect (100, 0, 500, 500));

	start();
}

