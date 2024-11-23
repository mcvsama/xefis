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

// Local:
#include "impulse_solver.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>

// Lib:
#include <boost/range/adaptors.hpp>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

ImpulseSolver::ImpulseSolver (System& system, uint32_t const max_iterations):
	_system (system),
	_max_iterations (max_iterations)
{ }


void
ImpulseSolver::set_required_precision (si::Force const force, si::Torque const torque)
{
	_required_force_torque_precision = ForceTorque {
		.force = force,
		.torque = torque,
	};
}


EvolutionDetails
ImpulseSolver::evolve (si::Time const dt)
{
	// Reset required parts of frame cache and initialize starting points:
	for (auto& body: _system.bodies())
	{
		auto& iter = body->iteration();
		iter.gravitational_force_moments = {};
		iter.velocity_moments = body->velocity_moments<WorldSpace>(); // TODO + warmer;
	}

	for (auto& frame_precalculation: _system.frame_precalculations())
		frame_precalculation->reset();

	update_mass_moments();
	update_gravitational_forces();
	update_external_forces();
	auto const details = update_constraint_forces (dt);
	update_acceleration_moments();
	update_velocity_moments();
	update_placements (dt);
	normalize_rotations();

	for (auto& body: _system.bodies())
		body->evolve (dt);

	++_processed_frames;

    return details;
}


void
ImpulseSolver::update_mass_moments()
{
	for (auto& body: _system.bodies())
	{
		auto const mass_moments = body->mass_moments();

		body->iteration().inv_M = (1.0 / mass_moments.mass()) * SpaceMatrix<double, WorldSpace> (math::unit);
		body->iteration().inv_I = body->placement().unbound_transform_to_base (mass_moments).inverse_inertia_tensor();
	}
}


void
ImpulseSolver::update_gravitational_forces()
{
	auto const& gravitating_bodies = _system.gravitating_bodies();
	auto const& non_gravitating_bodies = _system.non_gravitating_bodies();

	// Gravity interactions between gravitating bodies:
	if (gravitating_bodies.size() > 1)
		for (auto i1: gravitating_bodies | boost::adaptors::indexed())
			for (auto i2: gravitating_bodies | boost::adaptors::sliced (neutrino::to_unsigned (i1.index()) + 1u, _system.bodies().size()))
				update_gravitational_forces (*i1.value(), *i2);

	// Gravity interactions between gravitating bodies and the rest:
	for (auto& b1: gravitating_bodies)
		for (auto& b2: non_gravitating_bodies)
			update_gravitational_forces (*b1, *b2);
}


void
ImpulseSolver::update_gravitational_forces (Body& b1, Body& b2)
{
	auto const m1 = b1.mass_moments().mass();
	auto const m2 = b2.mass_moments().mass();
	auto const c1 = b1.placement().position();
	auto const c2 = b2.placement().position();

	// For very short distances simulation will be inaccurate due to quantized time, and will result
	// in one of bodies attaining unrealistically huge velocities.
	// Calculate minimum allowed distance between bodies to make simulation more realistic.
	// Those values are quite arbitrarily chosen.
	constexpr auto zero_distance = 1e-15_m;
	constexpr auto minimum_distance = 1e-9_m;

	auto const r_unsafe = c2 - c1;
	auto const r_unsafe_abs = abs (r_unsafe);
	auto const r =
		r_unsafe_abs < minimum_distance
			? r_unsafe_abs < zero_distance
				? SpaceLength<WorldSpace> { minimum_distance, 0_m, 0_m }
				: r_unsafe.normalized() * minimum_distance / 1_m
			: r_unsafe;
	auto const r_abs = abs (r);
	auto const gravitational_force = kGravitationalConstant * m1 * m2 * r / (r_abs * r_abs * r_abs);

	b1.iteration().gravitational_force_moments += ForceMoments<WorldSpace> { +gravitational_force, math::zero };
	b2.iteration().gravitational_force_moments += ForceMoments<WorldSpace> { -gravitational_force, math::zero };
}


void
ImpulseSolver::update_external_forces()
{
	auto const& atmosphere = _system.atmosphere();

	for (auto& body: _system.bodies())
		body->update_external_forces (atmosphere);

	for (auto& body: _system.bodies())
	{
		body->iteration().external_force_moments_except_gravity = body->external_force_moments<WorldSpace>();
		body->reset_applied_impulses();
	}
}


void
ImpulseSolver::calculate_constants_for_step (si::Time const dt)
{
	for (auto& body: _system.bodies())
	{
		auto& iter = body->iteration();
		iter.external_force_moments = iter.gravitational_force_moments + iter.external_force_moments_except_gravity;
		iter.external_impulses_over_mass = dt * iter.inv_M * iter.external_force_moments.force();
		iter.external_angular_impulses_over_inertia_tensor = dt * iter.inv_I * iter.external_force_moments.torque();
	}
}


EvolutionDetails
ImpulseSolver::update_constraint_forces (si::Time const dt)
{
	calculate_constants_for_step (dt);

	bool precise_enough = false;
	size_t iteration = 0;

	if (!_warm_starting)
		for (auto const& constraint: _system.constraints())
			constraint->previous_calculation_force_moments().reset();

	for (auto const& constraint: _system.constraints())
		constraint->initialize_step (dt);

	for (iteration = 0; iteration < _max_iterations && !precise_enough; ++iteration)
	{
		// Reset constraint forces:
		for (auto& body: _system.bodies())
			body->iteration().all_constraints_force_moments = ForceMoments<WorldSpace>();

		precise_enough = true;

		for (auto const& constraint: _system.constraints())
			if (!update_single_constraint_forces (constraint.get(), dt))
				precise_enough = false;
	}

	// Update acceleration moments except gravity (used by eg. acceleration sensors):
	for (auto const& constraint: _system.constraints())
	{
		if (constraint->enabled() && !constraint->broken())
		{
			auto& b1 = constraint->body_1();
			auto& b2 = constraint->body_2();

			auto& iter1 = b1.iteration();
			auto& iter2 = b2.iteration();

			iter1.acceleration_moments_except_gravity = calculate_acceleration_moments (b1.placement(), b1.mass_moments(), iter1.force_moments_except_gravity());
			iter2.acceleration_moments_except_gravity = calculate_acceleration_moments (b2.placement(), b2.mass_moments(), iter2.force_moments_except_gravity());
		}
	}

	// Tell each constraint that we finally calculated its forces:
	for (auto& constraint: _system.constraints())
	{
		// TODO What, these are summed-up all_constraints_force_moments, not individual ones, should be individual though:
		constraint->calculated_constraint_forces ({ constraint->body_1().iteration().all_constraints_force_moments,
													constraint->body_2().iteration().all_constraints_force_moments }, dt);
	}

    return {
        .iterations_run = iteration,
        .converged = precise_enough,
    };
}


bool
ImpulseSolver::update_single_constraint_forces (Constraint* constraint, si::Time const dt)
{
	bool precise_enough = _required_force_torque_precision.has_value();

	if (constraint->enabled() && !constraint->broken())
	{
		auto& b1 = constraint->body_1();
		auto& b2 = constraint->body_2();

		auto& iter1 = b1.iteration();
		auto& iter2 = b2.iteration();

		if (!b1.broken() && !b2.broken())
		{
			auto const constraint_forces = constraint->constraint_forces (iter1.velocity_moments, iter2.velocity_moments, dt);

			if (_required_force_torque_precision)
			{
				if (auto prev = constraint->previous_calculation_force_moments())
				{
					auto const dF = abs (constraint_forces[0].force() - prev->force());
					auto const dT = abs (constraint_forces[0].torque() - prev->torque());

					if (dF > _required_force_torque_precision->force)
						precise_enough = false;

					if (dT > _required_force_torque_precision->torque)
						precise_enough = false;
				}
				else
					precise_enough = false;
			}

			constraint->previous_calculation_force_moments() = constraint_forces[0];

			iter1.all_constraints_force_moments += constraint_forces[0];
			iter2.all_constraints_force_moments += constraint_forces[1];

			// Recalculate accelerations:
			iter1.acceleration_moments = calculate_acceleration_moments (b1.placement(), b1.mass_moments(), iter1.all_force_moments());
			iter2.acceleration_moments = calculate_acceleration_moments (b2.placement(), b2.mass_moments(), iter2.all_force_moments());

			// Recalculate velocity moments:
			iter1.velocity_moments = calculate_velocity_moments (b1.velocity_moments<WorldSpace>(), iter1.acceleration_moments, dt);
			iter2.velocity_moments = calculate_velocity_moments (b2.velocity_moments<WorldSpace>(), iter2.acceleration_moments, dt);
		}
	}

	return precise_enough;
}


AccelerationMoments<WorldSpace>
ImpulseSolver::calculate_acceleration_moments (Placement<WorldSpace, BodyCOM> const& placement, MassMoments<BodyCOM> const& mm, ForceMoments<WorldSpace> const& force_moments)
{
	auto const fm = placement.unbound_transform_to_body (force_moments);
	auto const am = AccelerationMoments<BodyCOM> (fm.force() / mm.mass(), 1_rad * mm.inverse_inertia_tensor() * fm.torque());
	return placement.unbound_transform_to_base (am);
}


void
ImpulseSolver::update_acceleration_moments()
{
	for (auto& body: _system.bodies())
		body->set_acceleration_moments<WorldSpace> (body->iteration().acceleration_moments);
}


VelocityMoments<WorldSpace>
ImpulseSolver::calculate_velocity_moments (VelocityMoments<WorldSpace> vm, AccelerationMoments<WorldSpace> const& am, si::Time const dt)
{
	vm.set_velocity (vm.velocity() + am.acceleration() * dt);
	vm.set_angular_velocity (vm.angular_velocity() + am.angular_acceleration() * dt);
	return vm;
}


void
ImpulseSolver::update_velocity_moments()
{
	for (auto& body: _system.bodies())
	{
		auto& vm = body->iteration().velocity_moments;
		apply_limits (vm);
		body->set_velocity_moments<WorldSpace> (vm);
	}
}


Placement<WorldSpace, BodyCOM>
ImpulseSolver::calculate_placement (Placement<WorldSpace, BodyCOM> placement, VelocityMoments<WorldSpace> const& vm, si::Time const dt)
{
	auto const ds = vm.velocity() * dt;
	auto const dr_vec = vm.angular_velocity() * dt;
	auto const dr = to_rotation_quaternion (dr_vec);

	placement.translate_frame (ds);
	placement.rotate_body_frame (dr);

	return placement;
}


void
ImpulseSolver::update_placements (si::Time dt)
{
	for (auto& body: _system.bodies())
		body->set_placement (calculate_placement (body->placement(), body->velocity_moments<WorldSpace>(), dt));
}


void
ImpulseSolver::normalize_rotations()
{
	// Once in a while orthonormalize rotation matrices in bodies:
	if (!_system.bodies().empty())
	{
		auto& body = _system.bodies()[_processed_frames % _system.bodies().size()];
		auto pl = body->placement();
		pl.set_body_to_base_rotation (pl.body_to_base_rotation().normalized());
		body->set_placement (pl);
	}
}

} // namespace xf::rigid_body

