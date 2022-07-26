/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_CHARACTERISTICS_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_CHARACTERISTICS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil_spline.h>
#include <xefis/support/aerodynamics/angle_of_attack.h>

// Neutrino:
#include <neutrino/math/field.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * This class represents an airfoil shape combined with lift/drag/pitching-moment polars.
 *
 * Uses AirfoilSplineSpace frame of reference, that is X-Y plane where X is parallel to the airfoil's chord,
 * positive X is at the trailing edge, positive Y is at the top of the airfoil.
 */
class AirfoilCharacteristics
{
  public:
	// All fields must be defined for angle of attack range [-180°…180°].
	// They map Reynolds number and angle of attack to a result coefficient:
	using LiftField						= Field<double, si::Angle, double>;
	using DragField						= Field<double, si::Angle, double>;
	using PitchingMomentField			= Field<double, si::Angle, double>;
	using CenterOfPressurePositionField	= Field<double, si::Angle, double>;

  public:
	/**
	 * All fields must be defined for range [-180°…180°].
	 */
	explicit
	AirfoilCharacteristics (AirfoilSpline const&,
							LiftField const&,
							DragField const&,
							PitchingMomentField const&,
							CenterOfPressurePositionField const&);

	/**
	 * Return contained spline.
	 */
	[[nodiscard]]
	AirfoilSpline const&
	spline() const noexcept
		{ return _spline; }

	/**
	 * Lift coefficient field.
	 * Maps Reynolds number and angle of attack to Cl.
	 */
	[[nodiscard]]
	LiftField const&
	lift_coefficient_field() const noexcept
		{ return _lift_coefficient; }

	/**
	 * Set new lift coefficient field.
	 */
	void
	set_lift_coefficient_field (LiftField const& field)
		{ _lift_coefficient = field; }

	/**
	 * Drag coefficient field.
	 * Maps Reynolds number and angle of attack to Cd.
	 */
	[[nodiscard]]
	DragField const&
	drag_coefficient_field() const noexcept
		{ return _drag_coefficient; }

	/**
	 * Set new drag coefficient field.
	 */
	void
	set_drag_coefficient_field (DragField const& field)
		{ _drag_coefficient = field; }

	/**
	 * Pitching moment coefficient field.
	 * Maps Reynolds number and angle of attack to Cm.
	 */
	[[nodiscard]]
	PitchingMomentField const&
	pitching_moment_coefficient_field() const noexcept
		{ return _pitching_moment_coefficient; }

	/**
	 * Set new pitching moment coefficient field.
	 */
	void
	set_pitching_moment_coefficient_field (PitchingMomentField const& field)
		{ _pitching_moment_coefficient = field; }

	/**
	 * Center of pressure position field (relative to origin).
	 * Maps Reynolds number and angle of attack to a number, that multiplied by chord
	 * will give relative position of center of pressure, measured from the leading edge.
	 */
	[[nodiscard]]
	CenterOfPressurePositionField const&
	center_of_pressure_position_field() const noexcept
		{ return _center_of_pressure_position; }

	/**
	 * Set new center of pressure position field.
	 */
	void
	set_center_of_pressure_position_field (CenterOfPressurePositionField const& field)
		{ _center_of_pressure_position = field; }

	/**
	 * Return value of lift coefficient field.
	 */
	template<class ...Arg>
		[[nodiscard]]
		auto
		lift_coefficient (Arg&& ...args) const
			{ return _lift_coefficient (std::forward<Arg> (args)...); }

	/**
	 * Return value of drag coefficient field.
	 */
	template<class ...Arg>
		[[nodiscard]]
		auto
		drag_coefficient (Arg&& ...args) const
			{ return _drag_coefficient (std::forward<Arg> (args)...); }

	/**
	 * Return value of pitching moment coefficient field.
	 */
	template<class ...Arg>
		[[nodiscard]]
		auto
		pitching_moment_coefficient (Arg&& ...args) const
			{ return _pitching_moment_coefficient (std::forward<Arg> (args)...); }

	/**
	 * Return value of center of pressure field.
	 */
	template<class ...Arg>
		[[nodiscard]]
		auto
		center_of_pressure_position (Arg&& ...args) const
			{ return _center_of_pressure_position (std::forward<Arg> (args)...); }

  private:
	AirfoilSpline					_spline;
	LiftField						_lift_coefficient;				// Cl
	DragField						_drag_coefficient;				// Cd
	PitchingMomentField				_pitching_moment_coefficient;	// Cm
	CenterOfPressurePositionField	_center_of_pressure_position;	// XCp
};

} // namespace xf

#endif

