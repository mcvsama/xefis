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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__CONNECTED_BODIES_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__CONNECTED_BODIES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

/**
 * A pair of bodies needed for generally all constraints.
 */
class ConnectedBodies
{
  public:
	// Ctor
	explicit
	ConnectedBodies (Body& body_1, Body& body_2);

	// Ctor
	explicit
	ConnectedBodies (ConnectedBodies const&) = default;

	/**
	 * Return reference to the first body.
	 */
	[[nodiscard]]
	Body&
	body_1() const noexcept
		{ return *_body_1; }

	/**
	 * Return reference to the second body.
	 */
	[[nodiscard]]
	Body&
	body_2() const noexcept
		{ return *_body_2; }

  private:
	Body*	_body_1;
	Body*	_body_2;
};


inline
ConnectedBodies::ConnectedBodies (Body& body_1, Body& body_2):
	_body_1 (&body_1),
	_body_2 (&body_2)
{ }

} // namespace xf::rigid_body

#endif

