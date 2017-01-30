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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/support/airframe/flaps.h>
#include <xefis/support/airframe/spoilers.h>
#include <xefis/support/airframe/lift.h>
#include <xefis/support/airframe/drag.h>
#include <xefis/support/airframe/types.h>


namespace xf {

class Xefis;


/**
 * Contains submodules that describe an airframe.
 */
class Airframe
{
  public:
	// Ctor
	Airframe (Xefis*, QDomElement const& config);

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
	Range<Angle> const&
	get_defined_aoa_range() const noexcept;

	/**
	 * Return total wings area.
	 */
	Area
	wings_area() const;

	/**
	 * Return chord length of the airfoil.
	 */
	Length
	wings_chord() const;

	/**
	 * Return AOA correction to apply to critical angle of attack.
	 * This + critical AOA gives the maximum safe AOA to fly at, where controls
	 * are retained.
	 */
	Angle
	safe_aoa_correction() const;

	/**
	 * Return C_L, including corrections for flaps and spoilers.
	 *
	 * \param aoa
	 *        Angle of attack as detected by AOA sensor.
	 */
	LiftCoefficient
	get_cl (Angle const& aoa, FlapsAngle const&, SpoilersAngle const&) const;

	/**
	 * Return C_D, including corrections for flaps and spoilers.
	 *
	 * \param aoa
	 *        Angle of attack as detected by AOA sensor.
	 */
	DragCoefficient
	get_cd (Angle const& aoa, FlapsAngle const&, SpoilersAngle const&) const;

	/**
	 * Return AOA for given C_L, corrected for flaps and spoilers.
	 *
	 * \param cl
	 *        Lift coefficient.
	 */
	Optional<Angle>
	get_aoa_in_normal_regime (LiftCoefficient const& cl, FlapsAngle const&, SpoilersAngle const&) const;

	/**
	 * Return critical AOA for given flaps and spoilers settings.
	 */
	Angle
	get_critical_aoa (FlapsAngle const&, SpoilersAngle const&) const;

	/**
	 * Return maximum safe AOA for given flaps and spoilers settings.
	 */
	Angle
	get_max_safe_aoa (FlapsAngle const&, SpoilersAngle const&) const;

	/**
	 * Return maximum safe load factor limits to fly at:
	 * { negative G, positive G }.
	 */
	Range<double>
	load_factor_limits() const;

  private:
	Unique<Flaps>		_flaps;
	Unique<Spoilers>	_spoilers;
	Unique<Lift>		_lift;
	Unique<Drag>		_drag;
	Range<Angle>		_defined_aoa_range;
	Area				_wings_area;
	Length				_wings_chord;
	Range<double>		_load_factor_limits;
	Angle				_safe_aoa_correction;
};


inline Flaps const&
Airframe::flaps() const
{
	if (!_flaps)
		throw BadConfiguration ("flaps submodule not configured");
	return *_flaps;
}


inline Spoilers const&
Airframe::spoilers() const
{
	if (!_spoilers)
		throw BadConfiguration ("spoilers submodule not configured");
	return *_spoilers;
}


inline Lift const&
Airframe::lift() const
{
	if (!_lift)
		throw BadConfiguration ("lift submodule not configured");
	return *_lift;
}


inline Drag const&
Airframe::drag() const
{
	if (!_drag)
		throw BadConfiguration ("drag submodule not configured");
	return *_drag;
}


inline Range<Angle> const&
Airframe::get_defined_aoa_range() const noexcept
{
	return _defined_aoa_range;
}


inline Area
Airframe::wings_area() const
{
	return _wings_area;
}


inline Length
Airframe::wings_chord() const
{
	return _wings_chord;
}


inline Angle
Airframe::safe_aoa_correction() const
{
	return _safe_aoa_correction;
}


inline Range<double>
Airframe::load_factor_limits() const
{
	return _load_factor_limits;
}

} // namespace xf

#endif

