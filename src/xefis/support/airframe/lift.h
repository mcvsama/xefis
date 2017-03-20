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

#ifndef XEFIS__SUPPORT__AIRFRAME__LIFT_H__INCLUDED
#define XEFIS__SUPPORT__AIRFRAME__LIFT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/airframe/types.h>
#include <xefis/utility/datatable2d.h>


namespace xf {

class Lift
{
  public:
	// Ctor
	explicit
	Lift (QDomElement const& config);

	/**
	 * Get range of AOA for which lift is defined.
	 */
	Range<Angle>
	get_aoa_range() const noexcept;

	/**
	 * Return lift coefficient (C_L) for given angle of attack.
	 *
	 * Uses linear interpolation.
	 */
	LiftCoefficient
	get_cl (Angle const& aoa) const;

	/**
	 * Return maximum possible lift.
	 */
	LiftCoefficient
	max_cl() const noexcept;

	/**
	 * Return angle for which C_L is maximum.
	 */
	Angle
	critical_aoa() const noexcept;

	/**
	 * Return AOA in normal regime (not stalled) for given C_L.
	 */
	Optional<Angle>
	get_aoa_in_normal_regime (LiftCoefficient const& cl) const noexcept;

  private:
	Unique<Datatable2D<Angle, LiftCoefficient>>	_aoa_to_cl;
	LiftCoefficient								_max_cl;
	Angle										_critical_aoa;
};


inline Range<Angle>
Lift::get_aoa_range() const noexcept
{
	return _aoa_to_cl->domain();
}

} // namespace xf

#endif

