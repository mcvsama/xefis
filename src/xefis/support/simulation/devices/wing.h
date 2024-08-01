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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__WING_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__WING_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/concepts.h>

// Standard:
#include <cstddef>


namespace xf::sim {

class Wing: public rigid_body::Body
{
  public:
	// Ctor
	explicit
	Wing (Airfoil const&, si::Density const material_density);

	/**
	 * Reference to the internal Airfoil object.
	 */
	Airfoil const&
	airfoil() const noexcept
		{ return _airfoil; }

	/**
	 * Return last calculated lift force vector.
	 */
	SpaceForce<rigid_body::BodyCOM>
	lift_force() const noexcept
		{ return _lift_force; }

	/**
	 * Return last calculated drag force vector.
	 */
	SpaceForce<rigid_body::BodyCOM>
	drag_force() const noexcept
		{ return _drag_force; }

	/**
	 * Return last calculated pitching moment vector (about the center of mass).
	 */
	SpaceTorque<rigid_body::BodyCOM>
	pitching_moment() const noexcept
		{ return _pitching_moment; }

	/**
	 * Return last calculated center of pressure.
	 */
	SpaceLength<rigid_body::BodyCOM>
	center_of_pressure() const noexcept
		{ return _center_of_pressure; }

	// Body API
	void
	update_external_forces (Atmosphere const*) override;

  private:
	static MassMoments<rigid_body::BodyCOM>
	calculate_body_com_mass_moments (Airfoil const&, si::Density material_density);

  private:
	Airfoil								_airfoil;
	SpaceForce<rigid_body::BodyCOM>		_lift_force;
	SpaceForce<rigid_body::BodyCOM>		_drag_force;
	SpaceTorque<rigid_body::BodyCOM>	_pitching_moment;
	SpaceLength<rigid_body::BodyCOM>	_center_of_pressure;
	SpaceLength<rigid_body::BodyCOM>	_com_to_planar_origin;
};


inline MassMoments<rigid_body::BodyCOM>
Wing::calculate_body_com_mass_moments (Airfoil const& airfoil, si::Density const material_density)
{
	// Well, let AirfoilSplineSpace and BodyCOM be actually the same, so an unit matrix:
	auto const rotate_to_body_com = RotationMatrix<rigid_body::BodyCOM, AirfoilSplineSpace> (math::unit);
	return rotate_to_body_com * calculate_mass_moments<AirfoilSplineSpace> (airfoil, material_density);
}

} // namespace xf::sim

#endif

