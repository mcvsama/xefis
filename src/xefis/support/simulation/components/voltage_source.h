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

#ifndef XEFIS__SUPPORT__SIMULATION__COMPONENTS__VOLTAGE_SOURCE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__COMPONENTS__VOLTAGE_SOURCE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/electrical/element.h>

// Standard:
#include <cstddef>
#include <string>
#include <string_view>


namespace xf::electrical {

class VoltageSource: public Element
{
  public:
	// Ctor
	explicit
	VoltageSource (std::string_view const& name, si::Voltage, si::Resistance internal_resistance);

	/**
	 * Return ideal voltage source voltage.
	 */
	[[nodiscard]]
	si::Voltage
	source_voltage() const noexcept
		{ return _source_voltage; }

	/**
	 * Set ideal voltage source voltage.
	 */
	void
	set_source_voltage (si::Voltage const voltage)
		{ _source_voltage = voltage; }

	// Element API
	[[nodiscard]]
	si::Current
	current_for_voltage (si::Voltage) const override;

	/**
	 * Return voltage for given current.
	 * Used by non-linear elements, like diodes.
	 */
	[[nodiscard]]
	si::Voltage
	voltage_for_current (si::Current) const override;

	// Element API
	void
	flow_current (si::Time) override
	{ }

  private:
	si::Voltage				_source_voltage;
};


inline
VoltageSource::VoltageSource (std::string_view const& name, si::Voltage const voltage, si::Resistance const internal_resistance):
	Element (Element::VoltageSource, name),
	_source_voltage (voltage)
{
	set_voltage (source_voltage());
	set_resistance (internal_resistance);
}


inline si::Current
VoltageSource::current_for_voltage (si::Voltage const voltage) const
{
	return (voltage + _source_voltage) / resistance();
}


inline si::Voltage
VoltageSource::voltage_for_current (si::Current const current) const
{
	return -(_source_voltage - current * resistance());
}

} // namespace xf::electrical

#endif

