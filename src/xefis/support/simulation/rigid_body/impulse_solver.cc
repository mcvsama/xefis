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


EvolutionDetails
ImpulseSolver::evolve (si::Time const dt)
{
	// Reset required parts of frame cache:
	for (auto& body: _system.bodies())
	{
		body->frame_cache().gravitational_force_moments = {};
		body->frame_cache().velocity_moments = body->velocity_moments<WorldSpace>();
	}

	for (auto& frame_precalculation: _system.frame_precalculations())
		frame_precalculation->reset();

	update_mass_moments();
	update_gravitational_forces();
	update_external_forces();
	auto const details = update_constraint_forces (dt);
	update_acceleration_moments();
	update_velocity_moments (dt);
	update_locations (dt);
	orthonormalize_rotation_matrices();

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
		auto const mass_moments = body->mass_moments<BodySpace>();

		body->frame_cache().inv_M = (1.0 / mass_moments.mass()) * SpaceMatrix<double, WorldSpace> (math::unit);
		body->frame_cache().inv_I = body->location().unbound_transform_to_base (mass_moments).inversed_moment_of_inertia();
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
	auto const m1 = b1.mass_moments<BodySpace>().mass();
	auto const m2 = b2.mass_moments<BodySpace>().mass();
	auto const c1 = b1.location().position();
	auto const c2 = b2.location().position();

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
				: normalized (r_unsafe) * minimum_distance / 1_m
			: r_unsafe;
	auto const r_abs = abs (r);
	auto const gravitational_force = kGravitationalConstant * m1 * m2 * r / (r_abs * r_abs * r_abs);

	b1.frame_cache().gravitational_force_moments += ForceMoments<WorldSpace> { +gravitational_force, math::zero };
	b2.frame_cache().gravitational_force_moments += ForceMoments<WorldSpace> { -gravitational_force, math::zero };
}


void
ImpulseSolver::update_external_forces()
{
	auto const& atmosphere_model = _system.atmosphere_model();

	for (auto& body: _system.bodies())
		body->update_external_forces (atmosphere_model);

	for (auto& body: _system.bodies())
	{
		body->frame_cache().external_force_moments = body->external_force_moments<WorldSpace>();
		body->reset_applied_forces();
	}
}


EvolutionDetails
ImpulseSolver::update_constraint_forces (si::Time const dt)
{
	bool precise_enough = false;
	size_t iteration = 0;

    for (auto const& constraint: _system.constraints())
        constraint->previous_calculation_force_moments().reset();

	for (iteration = 0; iteration < _max_iterations && !precise_enough; ++iteration)
	{
		precise_enough = true;

		// Reset constraint forces:
		for (auto& body: _system.bodies())
			body->frame_cache().constraint_force_moments = ForceMoments<WorldSpace>();

		for (auto const& constraint: _system.constraints())
		{
			if (constraint->enabled() && !constraint->broken())
			{
				auto& b1 = constraint->body_1();
				auto& b2 = constraint->body_2();

				auto& fc1 = b1.frame_cache();
				auto& fc2 = b2.frame_cache();

				if (!b1.broken() && !b2.broken())
				{
					auto const total_ext_forces_1 = fc1.gravitational_force_moments + fc1.external_force_moments;
					auto const total_ext_forces_2 = fc2.gravitational_force_moments + fc2.external_force_moments;

					auto const constraint_forces = constraint->constraint_forces (fc1.velocity_moments, total_ext_forces_1,
																				  fc2.velocity_moments, total_ext_forces_2,
																				  dt);

                    if (constraint->previous_calculation_force_moments())
                    {
                        auto const& prev = *constraint->previous_calculation_force_moments();
                        auto const dF = abs (constraint_forces[0].force() - prev.force());
                        auto const dT = abs (constraint_forces[0].torque() - prev.torque());

                        if (dF > 0.001_N || dT > 0.001_Nm) // TODO configurable
                            precise_enough = false;
                    }
                    else
                        precise_enough = false;

                    constraint->previous_calculation_force_moments() = constraint_forces[0];

					fc1.constraint_force_moments += constraint_forces[0];
					fc2.constraint_force_moments += constraint_forces[1];

					// Recalculate accelerations:
					fc1.acceleration_moments = acceleration_moments (b1, fc1.all_force_moments());
					fc2.acceleration_moments = acceleration_moments (b2, fc2.all_force_moments());

					// Recalculate velocity moments:
					fc1.velocity_moments = calculate_velocity_moments (b1, fc1.acceleration_moments, dt);
					fc2.velocity_moments = calculate_velocity_moments (b2, fc2.acceleration_moments, dt);
				}
			}
		}
	}

	// Tell each constraint that we finally calculated its forces:
	for (auto& constraint: _system.constraints())
	{
		constraint->calculated_constraint_forces ({ constraint->body_1().frame_cache().constraint_force_moments,
													constraint->body_2().frame_cache().constraint_force_moments });
	}

    return {
        .iterations_run = iteration,
        .converged = precise_enough,
    };
}


AccelerationMoments<WorldSpace>
ImpulseSolver::acceleration_moments (Body const& body, ForceMoments<WorldSpace> const& force_moments)
{
	auto const fm = body.location().unbound_transform_to_body (force_moments);
	auto const mm = body.mass_moments<BodySpace>();
	auto const am = AccelerationMoments<BodySpace> (fm.force() / mm.mass(), 1_rad * mm.inversed_moment_of_inertia() * fm.torque());
	return body.location().unbound_transform_to_base (am);
}


void
ImpulseSolver::update_acceleration_moments()
{
	for (auto& body: _system.bodies())
	{
		auto fm = body->frame_cache().all_force_moments();
		apply_limits (fm);
		body->set_acceleration_moments<WorldSpace> (acceleration_moments (*body, fm));
	}
}


VelocityMoments<WorldSpace>
ImpulseSolver::calculate_velocity_moments (Body const& body, AccelerationMoments<WorldSpace> const& am, si::Time const dt)
{
	auto vm = body.velocity_moments<WorldSpace>();
	vm.set_velocity (vm.velocity() + am.acceleration() * dt);
	vm.set_angular_velocity (vm.angular_velocity() + am.angular_acceleration() * dt);
	return vm;
}


void
ImpulseSolver::update_velocity_moments (si::Time const dt)
{
	for (auto& body: _system.bodies())
	{
		auto vm = calculate_velocity_moments (*body, body->acceleration_moments<WorldSpace>(), dt);
		apply_limits (vm);
		body->set_velocity_moments<WorldSpace> (vm);
	}
}


void
ImpulseSolver::update_locations (si::Time dt)
{
	for (auto& body: _system.bodies())
	{
		auto location = body->location();
		auto const vm = body->velocity_moments<WorldSpace>();
		auto const ds = vm.velocity() * dt;
		auto const dr_vec = vm.angular_velocity() * dt;
		auto const dr = to_rotation_matrix (dr_vec);

		location.translate_frame (ds);
		location.rotate_body_frame (dr);

		body->set_location (location);
	}
}


void
ImpulseSolver::orthonormalize_rotation_matrices()
{
	// Once in a while orthonormalize rotation matrices in bodies:
	if (!_system.bodies().empty())
	{
		auto& body = _system.bodies()[_processed_frames % _system.bodies().size()];
		auto loc = body->location();
		loc.set_body_to_base_rotation (vector_normalized (orthogonalized (loc.body_to_base_rotation())));
		body->set_location (loc);
	}
}

} // namespace xf::rigid_body

