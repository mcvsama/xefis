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

#ifndef XEFIS__SUPPORT__SIMULATION__COMPONENTS__RESISTOR_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__COMPONENTS__RESISTOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/electrical/element.h>

// Standard:
#include <cstddef>
#include <string_view>


namespace xf::electrical {

class Resistor: public Element
{
  public:
	// Ctor
	explicit
	Resistor (std::string_view const& name, si::Resistance);

	/**
	 * Return total energy dissipated by resistor.
	 */
	[[nodiscard]]
	si::Energy
	energy_loss() const noexcept
		{ return _energy_loss; }

	// Element API
	[[nodiscard]]
	si::Current
	current_for_voltage (si::Voltage) const override;

	// Element API
	[[nodiscard]]
	si::Voltage
	voltage_for_current (si::Current) const override;

	// Element API
	void
	flow_current (si::Time const dt) override;

  private:
	si::Energy	_energy_loss	{ 0_J };
};


inline
Resistor::Resistor (std::string_view const& name, si::Resistance const resistance):
	Element (Element::Load, name)
{
	set_resistance (resistance);
	set_const_resistance();
}


inline si::Current
Resistor::current_for_voltage (si::Voltage const voltage) const
{
	return voltage / resistance();
}


inline si::Voltage
Resistor::voltage_for_current (si::Current const current) const
{
	return current * resistance();
}


inline void
Resistor::flow_current (si::Time const dt)
{
	_energy_loss += abs (voltage() * current() * dt);
}

} // namespace xf::electrical

#endif

