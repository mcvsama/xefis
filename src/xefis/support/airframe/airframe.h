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

#ifndef XEFIS__SUPPORT__AIRFRAME__AIRFRAME_H__INCLUDED
#define XEFIS__SUPPORT__AIRFRAME__AIRFRAME_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/airframe/flaps.h>
#include <xefis/support/airframe/spoilers.h>
#include <xefis/support/airframe/lift.h>
#include <xefis/support/airframe/drag.h>
#include <xefis/support/airframe/types.h>

// Neutrino:
#include <neutrino/stdexcept.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

class Xefis;


/**
 * Airframe definition object.
 */
struct AirframeDefinition
{
	Flaps			flaps;
	Spoilers		spoilers;
	Lift			lift;
	Drag			drag;

	si::Area		wings_area;
	si::Length		wing_chord;
	Range<double>	load_factor_limits;
	si::Angle		safe_aoa_correction;
};


/**
 * Contains submodules that describe an airframe.
 */
class Airframe
{
  public:
	// Ctor
	explicit
	Airframe (AirframeDefinition);

  public:
	Flaps const&
	flaps() const;

	Spoilers const&
	spoilers() const;

	Lift const&
	lift() const;

	Drag const&
	drag() const;

	/**
	 * Return range of useful AOA, for which computations make sense.
	 */
	Range<si::Angle> const&
	get_defined_aoa_range() const noexcept;

	/**
	 * Return total wings area.
	 */
	si::Area
	wings_area() const;

	/**
	 * Return chord length of the airfoil.
	 */
	si::Length
	wings_chord() const;

	/**
	 * Return AOA correction to apply to critical angle of attack.
	 * This + critical AOA gives the maximum safe AOA to fly at, where controls
	 * are retained.
	 */
	si::Angle
	safe_aoa_correction() const;

	/**
	 * Return C_L, including corrections for flaps and spoilers.
	 *
	 * \param aoa
	 *        Angle of attack as detected by AOA sensor.
	 */
	LiftCoefficient
	get_cl (si::Angle aoa, FlapsAngle, SpoilersAngle) const;

	/**
	 * Return C_D, including corrections for flaps and spoilers.
	 *
	 * \param aoa
	 *        Angle of attack as detected by AOA sensor.
	 */
	DragCoefficient
	get_cd (si::Angle aoa, FlapsAngle, SpoilersAngle) const;

	/**
	 * Return AOA for given C_L, corrected for flaps and spoilers.
	 *
	 * \param cl
	 *        Lift coefficient.
	 */
	std::optional<si::Angle>
	get_aoa_in_normal_regime (LiftCoefficient cl, FlapsAngle, SpoilersAngle) const;

	/**
	 * Return critical AOA for given flaps and spoilers settings.
	 */
	si::Angle
	get_critical_aoa (FlapsAngle, SpoilersAngle) const;

	/**
	 * Return maximum safe AOA for given flaps and spoilers settings.
	 */
	si::Angle
	get_max_safe_aoa (FlapsAngle, SpoilersAngle) const;

	/**
	 * Return maximum safe load factor limits to fly at:
	 * { negative G, positive G }.
	 */
	Range<double>
	load_factor_limits() const;

  private:
	AirframeDefinition	_definition;
	Range<si::Angle>	_defined_aoa_range;
};


inline Flaps const&
Airframe::flaps() const
{
	return _definition.flaps;
}


inline Spoilers const&
Airframe::spoilers() const
{
	return _definition.spoilers;
}


inline Lift const&
Airframe::lift() const
{
	return _definition.lift;
}


inline Drag const&
Airframe::drag() const
{
	return _definition.drag;
}


inline Range<si::Angle> const&
Airframe::get_defined_aoa_range() const noexcept
{
	return _defined_aoa_range;
}


inline si::Area
Airframe::wings_area() const
{
	return _definition.wings_area;
}


inline si::Length
Airframe::wings_chord() const
{
	return _definition.wing_chord;
}


inline si::Angle
Airframe::safe_aoa_correction() const
{
	return _definition.safe_aoa_correction;
}


inline Range<double>
Airframe::load_factor_limits() const
{
	return _definition.load_factor_limits;
}

} // namespace xf

#endif

