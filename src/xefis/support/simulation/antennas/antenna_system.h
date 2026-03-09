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

#ifndef XEFIS__SUPPORT__SIMULATION__ANTENNAS__ANTENNA_SYSTEM_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ANTENNAS__ANTENNA_SYSTEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/simulation/antennas/antenna_emission.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/nonmovable.h>

// Standard:
#include <cstddef>
#include <set>
#include <list>


namespace xf {

class Antenna;


class AntennaSystem:
	public nu::Noncopyable,
	public nu::Nonmovable
{
  private:
	class Emission
	{
	  public:
		Antenna const&						emitter;
		AntennaEmission						antenna_emission;
		Placement<WorldSpace, BodyOrigin>	placement;
		std::set<Antenna*>					receivers;
	};

  public:
	// Ctor
	explicit
	AntennaSystem (si::Time max_ttl);

	void
	register_antenna (Antenna&);

	void
	deregister_antenna (Antenna&);

	void
	emit_signal (Antenna& emitter, AntennaEmission const&);

	/**
	 * Propagates signals and does the cleanup of old signals.
	 */
	void
	process (si::Time now);

	void
	propagate_signals (si::Time now);

	void
	cleanup_emissions (si::Time now);

  private:
	si::Time			_max_ttl	{ 1_s };
	// A list preserves order, which is needed for correct-order delivery of signals:
	std::list<Emission>	_emissions;
	std::set<Antenna*>	_antennas;
};

} // namespace xf

#endif
