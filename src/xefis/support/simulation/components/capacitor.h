/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__COMPONENTS__CAPACITOR_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__COMPONENTS__CAPACITOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/components/voltage_source.h>

// Standard:
#include <cstddef>
#include <string_view>


namespace xf::electrical {

class Capacitor: public VoltageSource
{
  public:
	/**
	 * \param	name
	 *			Element identifier
	 * \param	capacitance
	 *			Capacitor capacitance
	 * \param	internal_resistance
	 *			Internal resistance. Suggest to use > 0_Ohm unless there will be a resistor in series
	 *			with the capacitor. Otherwise nan and inf values might happen.
	 */
	explicit
	Capacitor (std::string_view const& name, si::Capacitance capacitance, si::Resistance internal_resistance);

	/**
	 * Return current capacitance.
	 */
	[[nodiscard]]
	si::Capacitance
	capacitance() const noexcept
		{ return _capacitance; }

	/**
	 * Set new capacitance.
	 */
	void
	set_capacitance (si::Capacitance const capacitance) noexcept
		{ _capacitance = capacitance; }

	/**
	 * Return current charge.
	 */
	[[nodiscard]]
	si::Charge
	charge() const noexcept
		{ return _charge; }

	/**
	 * Set new charge.
	 */
	void
	set_charge (si::Charge) noexcept;

	// Element API
	void
	flow_current (si::Time dt) override;

  private:
	si::Capacitance	_capacitance;
	si::Charge		_charge { 0_C };
};


inline
Capacitor::Capacitor (std::string_view const& name, si::Capacitance const capacitance, si::Resistance const internal_resistance):
	VoltageSource (name, 0_V, internal_resistance),
	_capacitance (capacitance)
{ }


inline void
Capacitor::set_charge (si::Charge const charge) noexcept
{
	_charge = charge;
	set_source_voltage (-_charge / _capacitance);
}


inline void
Capacitor::flow_current (si::Time const dt)
{
	// Q = ∫ I dt
	set_charge (charge() + current() * dt);
}

} // namespace xf::electrical

#endif

