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

// Local:
#include "imu_cpf.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>


IMU_CPF::IMU_CPF (std::string_view const& instance):
	IMU_CPF_IO (instance)
{
	_centripetal_computer.set_callback (std::bind (&IMU_CPF::compute_centripetal, this));
	_centripetal_computer.add_depending_smoothers ({
		&_smooth_accel_x,
		&_smooth_accel_y,
		&_smooth_accel_z,
	});
	_centripetal_computer.observe ({
		&_io.angular_velocity_x,
		&_io.angular_velocity_y,
		&_io.angular_velocity_z,
		&_io.tas_x,
		&_io.tas_y,
		&_io.tas_z,
	});
}


void
IMU_CPF::process (xf::Cycle const& cycle)
{
	_centripetal_computer.process (cycle.update_time());
}


void
IMU_CPF::compute_centripetal()
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

	if (_io.angular_velocity_x && _io.angular_velocity_y && _io.angular_velocity_z &&
		_io.tas_x && _io.tas_y && _io.tas_z)
	{
		using namespace neutrino;

		si::Time dt = _centripetal_computer.update_dt();

		math::Vector<si::Velocity, 3> vec_v {
			*_io.tas_x,
			*_io.tas_y,
			*_io.tas_z,
		};

		math::Vector<si::AngularVelocity, 3> vec_w {
			*_io.angular_velocity_x,
			*_io.angular_velocity_y,
			*_io.angular_velocity_z,
		};

		auto acceleration = math::cross_product (vec_v, vec_w);

		_io.centripetal_acceleration_x = _smooth_accel_x (si::convert (acceleration[0]), dt);
		_io.centripetal_acceleration_y = _smooth_accel_y (si::convert (acceleration[1]), dt);
		_io.centripetal_acceleration_z = _smooth_accel_z (si::convert (acceleration[2]), dt);

		if (_io.mass)
		{
			_io.centripetal_force_x = *_io.mass * *_io.centripetal_acceleration_x;
			_io.centripetal_force_y = *_io.mass * *_io.centripetal_acceleration_y;
			_io.centripetal_force_z = *_io.mass * *_io.centripetal_acceleration_z;
		}
		else
		{
			_io.centripetal_force_x = xf::nil;
			_io.centripetal_force_y = xf::nil;
			_io.centripetal_force_z = xf::nil;
		}
	}
	else
	{
		_io.centripetal_force_x = xf::nil;
		_io.centripetal_force_y = xf::nil;
		_io.centripetal_force_z = xf::nil;

		_io.centripetal_acceleration_x = xf::nil;
		_io.centripetal_acceleration_y = xf::nil;
		_io.centripetal_acceleration_z = xf::nil;
	}
}

