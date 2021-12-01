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

#ifndef XEFIS__SUPPORT__SIMULATION__ELECTRICAL__ELEMENT_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ELECTRICAL__ELEMENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <string_view>

// Neutrino:
#include <neutrino/noncopyable.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/electrical/node.h>


namespace xf::electrical {

class Element: public Noncopyable
{
  public:
	enum Type
	{
		VoltageSource,
		CurrentSource,
		Load,
	};

  protected:
	// Ctor
	explicit
	Element (Type type, std::string_view const& name);

  public:
	// Dtor
	virtual
	~Element() = default;

	/**
	 * Return element type.
	 */
	Type
	type() const noexcept
		{ return _type; }

	/**
	 * Return element name.
	 */
	std::string const&
	name() const noexcept
		{ return _name; }

	/**
	 * Return voltage difference between anode and cathode.
	 */
	[[nodiscard]]
	si::Voltage
	voltage() const
		{ return _voltage; }

	/**
	 * Set voltage difference between anode and cathode.
	 */
	void
	set_voltage (si::Voltage const voltage)
		{ _voltage = voltage; }

	/**
	 * Return current electrical current.
	 */
	[[nodiscard]]
	si::Current
	current() const
		{ return _current; }

	/**
	 * Set current electrical current.
	 */
	void
	set_current (si::Current const current)
		{ _current = current; }

	/**
	 * Return current electrical resistance.
	 */
	[[nodiscard]]
	si::Resistance
	resistance() const
		{ return _resistance; }

	/**
	 * Set current electrical resistance.
	 */
	void
	set_resistance (si::Resistance const resistance)
		{ _resistance = resistance; }

	/**
	 * Return current element temperature.
	 */
	[[nodiscard]]
	si::Temperature
	temperature() const noexcept
		{ return _temperature; }

	/**
	 * Set new element temperature.
	 * Defaults to 300 K after construction (~27°C).
	 */
	void
	set_temperature (si::Temperature const temperature)
		{ _temperature = temperature; }

	/**
	 * Return reference to the anode.
	 */
	[[nodiscard]]
	Node&
	anode() noexcept
		{ return _anode; }

	/**
	 * Return reference to the anode.
	 */
	[[nodiscard]]
	Node const&
	anode() const noexcept
		{ return _anode; }

	/**
	 * Return reference to the cathode.
	 */
	[[nodiscard]]
	Node&
	cathode() noexcept
		{ return _cathode; }

	/**
	 * Return reference to the cathode.
	 */
	[[nodiscard]]
	Node const&
	cathode() const noexcept
		{ return _cathode; }

	/**
	 * Return current for given voltage.
	 * Used by non-linear elements, like diodes.
	 */
	[[nodiscard]]
	virtual si::Current
	current_for_voltage (si::Voltage voltage) const = 0;

	/**
	 * Return voltage for given current.
	 * Used by non-linear elements, like diodes.
	 */
	[[nodiscard]]
	virtual si::Voltage
	voltage_for_current (si::Current current) const = 0;

	/**
	 * Flow given current through the element (from Anode to Cathode) for time dt.
	 * Called on each step of simulation.
	 */
	virtual void
	flow_current (si::Time dt) = 0;

	/**
	 * True if element has constant resistance (passive element).
	 */
	[[nodiscard]]
	bool
	has_const_resistance() const noexcept
		{ return _has_const_resistance; }

	/**
	 * True if device is broken.
	 */
	[[nodiscard]]
	bool
	broken() const noexcept
		{ return _broken; }

	/**
	 * Set device "broken" status.
	 */
	void
	set_broken (bool broken) noexcept
		{ _broken = broken; }

  protected:
	/**
	 * Declare that element has constant resistance by definition.
	 */
	void
	set_const_resistance()
		{ _has_const_resistance = true; }

  private:
	Type			_type;
	std::string		_name;
	bool			_has_const_resistance;
	si::Voltage		_voltage		{ 0_V };
	si::Current		_current		{ 0_A };
	si::Resistance	_resistance		{ 0_Ohm };
	si::Temperature	_temperature	{ 300_K };
	Node			_anode			{ *this, Node::Anode };
	Node			_cathode		{ *this, Node::Cathode };
	bool			_broken			{ false };
};


template<class T>
	concept ElementConcept = std::derived_from<T, Element>;


inline
Element::Element (Type const type, std::string_view const& name):
	_type (type),
	_name (name),
	_has_const_resistance (false)
{ }


/*
 * Global functions
 */


inline Element&
operator<< (Node& node, Element& element)
{
	node << element.cathode();
	return element;
}


inline Element&
operator<< (Element& element, Node& node)
{
	node << element.anode();
	return element;
}


inline Element&
operator>> (Node& node, Element& element)
{
	node << element.anode();
	return element;
}


inline Element&
operator>> (Element& element, Node& node)
{
	node << element.cathode();
	return element;
}

} // namespace xf::electrical

#endif

