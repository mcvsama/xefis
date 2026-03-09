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

#ifndef XEFIS__SUPPORT__SIMULATION__ANTENNAS__ANTENNA_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ANTENNAS__ANTENNA_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/simulation/antennas/antenna_emission.h>
#include <xefis/support/simulation/antennas/antenna_model.h>
#include <xefis/support/simulation/antennas/antenna_system.h>

// Neutrino:
#include <neutrino/concepts.h>
#include <neutrino/noncopyable.h>
#include <neutrino/nonmovable.h>

// Standard:
#include <cstddef>
#include <functional>


namespace xf {

class Antenna:
	public nu::Noncopyable,
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
	Antenna (nu::NonTemporaryReference<AntennaModel const&> auto&& model,
			 AntennaSystem& system,
			 SignalReceptionCallback signal_reception_callback);

	// Dtor
	virtual
	~Antenna();

	/**
	 * Set antenna placement in the world. Placement must be updated before every
	 * call to AntennaSystem::propagate_signals()/process() to correctly calculate
	 * distances and polarization coupling.
	 */
	void
	set_placement (Placement<WorldSpace, BodyOrigin> const placement)
		{ _placement = placement; }

	/**
	 * Return currently stored antenna placement.
	 */
	[[nodiscard]]
	Placement<WorldSpace, BodyOrigin> const&
	placement() const noexcept
		{ return _placement; }

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
	emit_signal (AntennaEmission const&);

	/**
	 * Called when antenna receives a signal.
	 */
	void
	receive_signal (ReceivedSignal const& signal);

  private:
	AntennaModel const&					_model;
	AntennaSystem&						_system;
	Placement<WorldSpace, BodyOrigin>	_placement;
	SignalReceptionCallback				_signal_reception_callback;
};


inline
Antenna::Antenna (nu::NonTemporaryReference<AntennaModel const&> auto&& model,
				  AntennaSystem& system,
				  SignalReceptionCallback signal_reception_callback):
	_model (model),
	_system (system),
	_signal_reception_callback (std::move (signal_reception_callback))
{
	_system.register_antenna (*this);
}


inline void
Antenna::receive_signal (ReceivedSignal const& signal)
{
	if (_signal_reception_callback)
		_signal_reception_callback (signal);
}

} // namespace xf

#endif
