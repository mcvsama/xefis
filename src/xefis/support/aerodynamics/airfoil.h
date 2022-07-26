/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__AIRFOIL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil_characteristics.h>
#include <xefis/support/aerodynamics/reynolds.h>
#include <xefis/support/earth/air/standard_atmosphere.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/wrench.h>

// Standard:
#include <cstddef>
#include <utility>


namespace xf {

/**
 * Used for computed values by Airfoil.
 */
template<class Space>
	class AerodynamicForces
	{
	  public:
		SpaceForce<Space>	lift;
		SpaceForce<Space>	drag;
		SpaceTorque<Space>	pitching_moment;
		SpaceLength<Space>	center_of_pressure;

	  public:
		Wrench<Space>
		wrench() const
			{ return { ForceMoments<Space> (lift + drag, pitching_moment), center_of_pressure }; }
	};


/**
 * Airfoil represents an airfoil, which is an airfoil characteristics combined with chord length and wing length.
 * This class actually allows querying for various forces generated on the wing by the moving air.
 *
 * Uses AirfoilSplineSpace frame of reference, that is X-Y plane where X is parallel to the airfoil's chord,
 * positive X is at the trailing edge, positive Y is at the top of the airfoil.
 */
class Airfoil
{
  public:
	// Ctor
	explicit
	Airfoil (AirfoilCharacteristics const& airfoil_characteristics, si::Length chord_length, si::Length wing_length);

	/**
	 * Return AirfoilCharacteristics object reference.
	 */
	[[nodiscard]]
	AirfoilCharacteristics const&
	airfoil_characteristics() const noexcept
		{ return _airfoil_characteristics; }

	/**
	 * Shortcut to get airfoil spline for this wing.
	 */
	[[nodiscard]]
	AirfoilSpline const&
	spline() const noexcept
		{ return _airfoil_characteristics.spline(); }

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
	 * Return lift/drag Wrenches relative to the origin of AirfoilSplineSpace (their force position is at center of pressure of the wing, but at 0 in Z-axis).
	 * Lift is perpendicular to the wind, drag is parallel to the wind.
	 * Also compute angle of attack and put result into @result_aoa.
	 */
	[[nodiscard]]
	AerodynamicForces<AirfoilSplineSpace>
	planar_aerodynamic_forces (AtmosphereState<AirfoilSplineSpace> const&, AngleOfAttack& result_aoa) const;

	/**
	 * Like planar_aerodynamic_forces(), except it corrects the center of pressure position (returned Wrench's position) in the Z axis, to be in the center of
	 * the airfoil (half the length).
	 */
	[[nodiscard]]
	AerodynamicForces<AirfoilSplineSpace>
	aerodynamic_forces (AtmosphereState<AirfoilSplineSpace> const&, AngleOfAttack& result_aoa) const;

  private:
	/**
	 * Return area for calculation of the lift and drag forces (wing projected in the lift direction and wing projected in the drag direction).
	 */
	[[nodiscard]]
	std::pair<si::Area, si::Area>
	lift_drag_areas (si::Angle alpha, si::Angle beta) const;

	/**
	 * Wrap angle to range accepted by coefficient field types (LiftField, DragField, etc).
	 */
	[[nodiscard]]
	static constexpr si::Angle
	wrap_angle_for_field (si::Angle);

  private:
	AirfoilCharacteristics						_airfoil_characteristics;
	// Chord starts in X-Y position [0, 0]:
	si::Length									_chord_length				{ 0_m };
	si::Length									_wing_length				{ 0_m };
};


/*
 * Global functions
 */


template<class TF, class SF>
	AerodynamicForces<TF>
	operator* (RotationMatrix<TF, SF> const& rotation, AerodynamicForces<SF> const& source)
	{
		AerodynamicForces<TF> result;
		result.lift = rotation * source.lift;
		result.drag = rotation * source.drag;
		result.pitching_moment = rotation * source.pitching_moment;
		result.center_of_pressure = rotation * source.center_of_pressure;
		return result;
	}


template<class Space>
	MassMoments<Space>
	calculate_mass_moments (Airfoil const& airfoil, si::Density const material_density)
	{
		return calculate_mass_moments<Space> (airfoil.spline(), airfoil.chord_length(), airfoil.wing_length(), material_density);
	}

} // namespace xf

#endif

