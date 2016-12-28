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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <math/math.h>

// Local:
#include "imu_cff.h"


IMU_CFF::IMU_CFF (std::string const& instance):
	Module (instance)
{
	_centripetal_computer.set_callback (std::bind (&IMU_CFF::compute_centripetal, this));
	_centripetal_computer.add_depending_smoothers ({
		&_smooth_accel_x,
		&_smooth_accel_y,
		&_smooth_accel_z,
	});
	_centripetal_computer.observe ({
		&input_angular_velocity_x,
		&input_angular_velocity_y,
		&input_angular_velocity_z,
		&input_tas_x,
		&input_tas_y,
		&input_tas_z,
	});
}


void
IMU_CFF::process (x2::Cycle const& cycle)
{
	_centripetal_computer.process (cycle.update_time());
}


void
IMU_CFF::compute_centripetal()
{
	/*
	 * Turn radius:
	 *   r = v / (2 * pi * f)
	 * r ← radius
	 * f ← frequency
	 * v ← tas
	 *
	 * Also:
	 *   a = v² / r
	 * Therefore:
	 *   a = v * 2 * pi * f
	 * And:
	 *   a[y] = v[x] * (2 * pi * -f[z])
	 *   a[z] = v[x] * (2 * pi * +f[y])
	 */

	if (input_angular_velocity_x && input_angular_velocity_y && input_angular_velocity_z &&
		input_tas_x && input_tas_y && input_tas_z)
	{
		si::Time dt = _centripetal_computer.update_dt();

		math::Vector<si::Velocity, 3> vec_v {
			*input_tas_x,
			*input_tas_y,
			*input_tas_z,
		};

		math::Vector<si::AngularVelocity, 3> vec_w {
			*input_angular_velocity_x,
			*input_angular_velocity_y,
			*input_angular_velocity_z,
		};

		auto acceleration = math::cross_product (vec_v, vec_w);

		output_centripetal_acceleration_x = _smooth_accel_x (acceleration[0], dt);
		output_centripetal_acceleration_y = _smooth_accel_y (acceleration[1], dt);
		output_centripetal_acceleration_z = _smooth_accel_z (acceleration[2], dt);

		if (input_mass)
		{
			output_centripetal_force_x = *input_mass * *output_centripetal_acceleration_x;
			output_centripetal_force_y = *input_mass * *output_centripetal_acceleration_y;
			output_centripetal_force_z = *input_mass * *output_centripetal_acceleration_z;
		}
		else
		{
			output_centripetal_force_x.set_nil();
			output_centripetal_force_y.set_nil();
			output_centripetal_force_z.set_nil();
		}
	}
	else
	{
		output_centripetal_force_x.set_nil();
		output_centripetal_force_y.set_nil();
		output_centripetal_force_z.set_nil();

		output_centripetal_acceleration_x.set_nil();
		output_centripetal_acceleration_y.set_nil();
		output_centripetal_acceleration_z.set_nil();
	}
}

