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

#ifndef XEFIS__SUPPORT__SIMULATION__AIRFOIL_SHAPE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__AIRFOIL_SHAPE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air.h>
#include <xefis/support/math/angle_of_attack.h>
#include <xefis/support/math/space.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/simulation/airfoil_spline.h>
#include <xefis/support/simulation/atmosphere.h>
#include <xefis/utility/field.h>


namespace xf::sim {

/**
 * Uses airfoil frame of reference, that is X-Y plane where X is parallel to the airfoil's chord,
 * positive X is at the trailing edge, positive Y is at the top of the airfoil.
 */
class AirfoilShape
{
  public:
	// All fields must be defined for angle of attack range [-180°…180°].
	using LiftField						= Field<double, si::Angle, double>;
	using DragField						= Field<double, si::Angle, double>;
	using PitchingMomentField			= Field<double, si::Angle, double>;
	using CenterOfPressureOffsetField	= Field<double, si::Angle, double>;

  public:
	// Ctor
	explicit
	AirfoilShape (AirfoilSpline const&,
				  si::Length wing_length,
				  si::Length chord_length,
				  SpaceVector<si::Length, AirfoilSplineFrame> const& pivot_position,
				  LiftField const&,
				  DragField const&,
				  PitchingMomentField const&,
				  CenterOfPressureOffsetField const&);

	/**
	 * Length of the lifting surface.
	 */
	[[nodiscard]]
	si::Length
	wing_length() const noexcept
		{ return _wing_length; }

	/**
	 * Set length of the lifting surface.
	 */
	void
	set_wing_length (si::Length wing_length)
		{ _wing_length = wing_length; }

	/**
	 * Chord length (aka characteristic dimension) of the airfoil.
	 */
	[[nodiscard]]
	si::Length
	chord_length() const noexcept
		{ return _chord_length; }

	/**
	 * Set new chord length.
	 */
	void
	set_chord_length (si::Length chord_length)
		{ _chord_length = chord_length; }

	/**
	 * Pivot position relative to chord start (leading edge).
	 */
	[[nodiscard]]
	SpaceVector<si::Length, AirfoilSplineFrame> const&
	pivot_position() const noexcept
		{ return _pivot_position; }

	/**
	 * Set new pivot position.
	 */
	void
	set_pivot_position (SpaceVector<si::Length, AirfoilSplineFrame> const& pivot_position)
		{ _pivot_position = pivot_position; }

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
	 * Center of pressure offset field.
	 * Maps Reynolds number and angle of attack to a number, that multiplied by chord
	 * will give relative position of center of pressure, measured from the leading edge.
	 */
	[[nodiscard]]
	CenterOfPressureOffsetField const&
	center_of_pressure_offset_field() const noexcept
		{ return _center_of_pressure_offset; }

	/**
	 * Set new center of pressure offset field.
	 */
	void
	set_center_of_pressure_offset_field(CenterOfPressureOffsetField const& field)
		{ _center_of_pressure_offset = field; }

	/**
	 * Calculate the lift force of the airfoil.
	 */
	[[nodiscard]]
	si::Force
	lift_force (si::Angle alpha, si::Angle beta, Reynolds reynolds_number, si::Pressure dynamic_pressure, std::optional<si::Area> lifting_area) const;

	/**
	 * Calculate the drag force of the airfoil.
	 */
	[[nodiscard]]
	si::Force
	drag_force (si::Angle alpha, si::Angle beta, Reynolds reynolds_number, si::Pressure dynamic_pressure, std::optional<si::Area> dragging_area) const;

	/**
	 * Calculate the pitching moment of the airfoil.
	 */
	[[nodiscard]]
	si::Torque
	pitching_moment (si::Angle alpha, Reynolds reynolds_number, si::Pressure dynamic_pressure) const;

	/**
	 * Return a ForceTorque that is described in AirfoilShape's frame of reference.
	 * The root position of the force is at pivot point. Lift is perpendicular to the wind, drag is parallel to the wind.
	 * Resultant forces are expressed in airfoil-shape frame of reference (X axis along the chord).
	 */
	[[nodiscard]]
	ForceTorque<AirfoilSplineFrame>
	planar_aerodynamic_forces (AtmosphereState<AirfoilSplineFrame> const& atm, AngleOfAttack& aoa) const;

  private:
	/**
	 * Wrap angle to range accepted by coefficient field types (LiftField, DragField, etc).
	 */
	[[nodiscard]]
	static constexpr si::Angle
	wrap_angle_for_field (si::Angle);

	/**
	 * Return area for calculation of the lift and drag forces (wing projected in the lift direction and wing projected in the drag direction).
	 */
	[[nodiscard]]
	PlaneVector<si::Area>
	lift_drag_areas (si::Angle alpha, si::Angle beta) const;

  private:
	AirfoilSpline								_spline;
	si::Length									_wing_length					{ 0_m };
	// Chord starts in X-Y position [0, 0]:
	si::Length									_chord_length					{ 0_m };
	SpaceVector<si::Length, AirfoilSplineFrame>	_pivot_position					{ 0_m, 0_m, 0_m };
	// Map Ryenolds number → AOA → coefficients:
	LiftField									_lift_coefficient;				// Cl
	DragField									_drag_coefficient;				// Cd
	PitchingMomentField							_pitching_moment_coefficient;	// Cm
	// Positive offsets from the pivoting point to the back of the airfoil:
	CenterOfPressureOffsetField					_center_of_pressure_offset;		// XCp
};


constexpr si::Angle
AirfoilShape::wrap_angle_for_field (si::Angle angle)
{
	return floored_mod (angle, Range<si::Angle> { -180_deg, +180_deg });
}

} // namespace xf::sim

#endif

