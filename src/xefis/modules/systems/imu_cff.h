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

#ifndef XEFIS__MODULES__SYSTEMS__IMU_CFF_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__IMU_CFF_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>


class IMU_CFF_IO: public xf::ModuleIO
{
  public:
	/*
	 * Input
	 */

	xf::PropertyIn<si::AngularVelocity>	angular_velocity_x			{ this, "/angular-velocity/x" };
	xf::PropertyIn<si::AngularVelocity>	angular_velocity_y			{ this, "/angular-velocity/y" };
	xf::PropertyIn<si::AngularVelocity>	angular_velocity_z			{ this, "/angular-velocity/z" };
	xf::PropertyIn<si::Velocity>		tas_x						{ this, "/tas/x" };
	xf::PropertyIn<si::Velocity>		tas_y						{ this, "/tas/y" };
	xf::PropertyIn<si::Velocity>		tas_z						{ this, "/tas/z" };
	xf::PropertyIn<si::Mass>			mass						{ this, "/mass" };

	/*
	 * Output
	 */

	xf::PropertyOut<si::Acceleration>	centripetal_acceleration_x	{ this, "/centripetal/x" };
	xf::PropertyOut<si::Acceleration>	centripetal_acceleration_y	{ this, "/centripetal/y" };
	xf::PropertyOut<si::Acceleration>	centripetal_acceleration_z	{ this, "/centripetal/z" };
	xf::PropertyOut<si::Force>			centripetal_force_x			{ this, "/force/x" };
	xf::PropertyOut<si::Force>			centripetal_force_y			{ this, "/force/y" };
	xf::PropertyOut<si::Force>			centripetal_force_z			{ this, "/force/z" };
};


/**
 * Compute centripetal force from IAS and gyro information.
 * TODO rename CFF -> centripetal
 */
class IMU_CFF: public xf::Module<IMU_CFF_IO>
{
  private:
	static constexpr si::Time kSmoothingTime = 1_s;

  public:
	// Ctor
	explicit
	IMU_CFF (std::unique_ptr<IMU_CFF_IO>, std::string const& instance = {});

  protected:
	void
	process (xf::Cycle const&) override;

	void
	compute_centripetal();

  private:
	xf::Smoother<si::Acceleration>	_smooth_accel_x			{ kSmoothingTime };
	xf::Smoother<si::Acceleration>	_smooth_accel_y			{ kSmoothingTime };
	xf::Smoother<si::Acceleration>	_smooth_accel_z			{ kSmoothingTime };
	xf::PropertyObserver			_centripetal_computer;
};

#endif
