/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__MACHINE_MANAGER_H__INCLUDED
#define XEFIS__CORE__MACHINE_MANAGER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/xefis.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * This object is created from the main executable.
 */
class MachineManager: private Noncopyable
{
  public:
	// Ctor
	explicit
	MachineManager (Xefis&);

	// Dtor
	virtual
	~MachineManager() = default;

	/**
	 * Return main Xefis object.
	 */
	Xefis&
	xefis() const noexcept
		{ return _xefis; }

	/**
	 * Return the managed machine.
	 */
	virtual Machine&
	machine() = 0;

  private:
	Xefis& _xefis;
};


inline
MachineManager::MachineManager (Xefis& xefis):
	_xefis (xefis)
{ }

} // namespace xf

#endif

