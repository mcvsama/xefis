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
#include <xefis/support/properties/has_observation_widget.h>
#include <xefis/support/simulation/antennas/antenna.h>
#include <xefis/support/simulation/devices/antenna_widget.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Standard:
#include <cstddef>


namespace xf::sim {

/**
 * Rigid-body antenna device. Keeps radio-antenna placement synchronized with
 * rigid-body center-of-mass/origin placements.
 */
class Antenna:
	public rigid_body::Body,
	public HasObservationWidget
{
  public:
    // Ctor
    explicit
    Antenna (MassMoments<BodyCOM> const&,
             xf::AntennaModel const&,
             xf::AntennaSystem&,
             xf::Antenna::SignalReceptionCallback = {});

    // Ctor
    explicit
    Antenna (MassMomentsAtArm<BodyCOM> const&,
             xf::AntennaModel const&,
             xf::AntennaSystem&,
             xf::Antenna::SignalReceptionCallback = {});

	[[nodiscard]]
	xf::Antenna&
	antenna() noexcept
		{ return _antenna; }

	[[nodiscard]]
	xf::Antenna const&
	antenna() const noexcept
		{ return _antenna; }

	[[nodiscard]]
	xf::AntennaModel const&
	model() const noexcept
		{ return _antenna.model(); }

	// HasObservationWidget API
	[[nodiscard]]
	std::unique_ptr<ObservationWidget>
	create_observation_widget() override
		{ return std::make_unique<AntennaWidget> (*this); }

	void
	emit_signal (xf::AntennaEmission const& antenna_emission)
		{ _antenna.emit_signal (antenna_emission); }

	// Body API
	void
	evolve (si::Time) override
		{ synchronize_antenna_placement(); }

  private:
	void
	synchronize_antenna_placement() noexcept;

  private:
	xf::Antenna _antenna;
};

} // namespace xf::sim

#endif
