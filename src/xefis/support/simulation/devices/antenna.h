/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__ANTENNA_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__ANTENNA_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/properties/has_observation_widget.h>
#include <xefis/support/simulation/antennas/antenna_emission.h>
#include <xefis/support/simulation/antennas/antenna_model.h>
#include <xefis/support/simulation/antennas/antenna_system.h>
#include <xefis/support/simulation/devices/antenna_widget.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Neutrino:
#include <neutrino/concepts.h>
#include <neutrino/nonmovable.h>

// Standard:
#include <cstddef>
#include <functional>


namespace xf::sim {

class Antenna:
	public rigid_body::Body,
	public HasObservationWidget,
	public nu::Nonmovable
{
  public:
	struct ReceivedSignal
	{
		si::Power		signal_power;
		std::string		payload;
	};

	using SignalReceptionCallback = std::function<void (ReceivedSignal const&)>;

  public:
	// Ctor
	explicit
	Antenna (MassMoments<BodyCOM> const&,
			 nu::NonTemporaryReference<AntennaModel const&> auto&& model,
			 AntennaSystem& system,
			 SignalReceptionCallback signal_reception_callback);

	// Ctor
	explicit
	Antenna (MassMomentsAtArm<BodyCOM> const&,
			 nu::NonTemporaryReference<AntennaModel const&> auto&& model,
			 AntennaSystem& system,
			 SignalReceptionCallback signal_reception_callback);

	// Dtor
	virtual
	~Antenna()
		{ _system.deregister_antenna (*this); }

	/**
	 * Access the stored antenna model.
	 */
	[[nodiscard]]
	AntennaModel const&
	model() const noexcept
		{ return _model; }

	/**
	 * Emit signal.
	 * Due to a bug in Qt (or a really stupid decision, but I want to believe it was just a bug)
	 * there's an "emit" macro left by Qt headers. So this needs a redundant "_signal" suffix.
	 */
	void
	emit_signal (AntennaEmission const& antenna_emission)
		{ _system.emit_signal (*this, antenna_emission); }

	/**
	 * Called by the AntennaSystem when antenna receives a signal.
	 * Calls the configured SignalReceptionCallback.
	 */
	void
	receive_signal (ReceivedSignal const& signal);

	// HasObservationWidget API
	[[nodiscard]]
	std::unique_ptr<ObservationWidget>
	create_observation_widget() override
		{ return std::make_unique<sim::AntennaWidget> (*this); }

  private:
	AntennaModel const&		_model;
	AntennaSystem&			_system;
	SignalReceptionCallback	_signal_reception_callback;
};


// Has to be inline because of use of nu::NonTemporaryReference<>
inline
Antenna::Antenna (MassMoments<BodyCOM> const& mass_moments,
				  nu::NonTemporaryReference<AntennaModel const&> auto&& model,
				  AntennaSystem& system,
				  SignalReceptionCallback signal_reception_callback):
	Body (mass_moments),
	_model (model),
	_system (system),
	_signal_reception_callback (std::move (signal_reception_callback))
{
	_system.register_antenna (*this);
}


// Has to be inline because of use of nu::NonTemporaryReference<>
inline
Antenna::Antenna (MassMomentsAtArm<BodyCOM> const& mass_moments,
				  nu::NonTemporaryReference<AntennaModel const&> auto&& model,
				  AntennaSystem& system,
				  SignalReceptionCallback signal_reception_callback):
	Body (mass_moments),
	_model (model),
	_system (system),
	_signal_reception_callback (std::move (signal_reception_callback))
{
	_system.register_antenna (*this);
}


} // namespace xf::sim

#endif
