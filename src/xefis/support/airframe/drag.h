/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__AIRFRAME__DRAG_H__INCLUDED
#define XEFIS__SUPPORT__AIRFRAME__DRAG_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/airframe/types.h>

// Neutrino:
#include <neutrino/math/field.h>

// Qt:
#include <QDomElement>

// Standard:
#include <cstddef>


namespace xf {

class Drag
{
  public:
	// Ctor
	explicit
	Drag (QDomElement const& config);

	/**
	 * Get range of AOA for which lift is defined.
	 */
	nu::Range<si::Angle>
	get_aoa_range() const noexcept;

	/**
	 * Return drag coefficient (C_D) for given angle of attack.
	 *
	 * Uses linear interpolation.
	 */
	DragCoefficient
	get_cd (si::Angle aoa) const;

  private:
	nu::Field<si::Angle, DragCoefficient> _aoa_to_cd {};
};


inline nu::Range<si::Angle>
Drag::get_aoa_range() const noexcept
{
	return _aoa_to_cd.domain();
}

} // namespace xf

#endif

