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

#ifndef XEFIS__MODULES__TEST__IMU_CPF_H__INCLUDED
#define XEFIS__MODULES__TEST__IMU_CPF_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_observer.h>

// Standard:
#include <cstddef>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class IMU_CPF_IO: public xf::Module
{
  public:
	/*
	 * Input
	 */

	xf::ModuleIn<si::AngularVelocity>	angular_velocity_x			{ this, "angular-velocity/x" };
	xf::ModuleIn<si::AngularVelocity>	angular_velocity_y			{ this, "angular-velocity/y" };
	xf::ModuleIn<si::AngularVelocity>	angular_velocity_z			{ this, "angular-velocity/z" };
	xf::ModuleIn<si::Velocity>			tas_x						{ this, "tas/x" };
	xf::ModuleIn<si::Velocity>			tas_y						{ this, "tas/y" };
	xf::ModuleIn<si::Velocity>			tas_z						{ this, "tas/z" };
	xf::ModuleIn<si::Mass>				mass						{ this, "mass" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::Acceleration>		centripetal_acceleration_x	{ this, "centripetal/x" };
	xf::ModuleOut<si::Acceleration>		centripetal_acceleration_y	{ this, "centripetal/y" };
	xf::ModuleOut<si::Acceleration>		centripetal_acceleration_z	{ this, "centripetal/z" };
	xf::ModuleOut<si::Force>			centripetal_force_x			{ this, "force/x" };
	xf::ModuleOut<si::Force>			centripetal_force_y			{ this, "force/y" };
	xf::ModuleOut<si::Force>			centripetal_force_z			{ this, "force/z" };

  public:
	using xf::Module::Module;
};


/**
 * Compute centripetal force from IAS and gyro information.
 */
class IMU_CPF: public IMU_CPF_IO
{
  private:
	static constexpr si::Time kSmoothingTime = 1_s;

  public:
	// Ctor
	explicit
	IMU_CPF (std::string_view const& instance = {});

  protected:
	void
	process (xf::Cycle const&) override;

	void
	compute_centripetal();

  private:
	IMU_CPF_IO&						_io						{ *this };
	xf::Smoother<si::Acceleration>	_smooth_accel_x			{ kSmoothingTime };
	xf::Smoother<si::Acceleration>	_smooth_accel_y			{ kSmoothingTime };
	xf::Smoother<si::Acceleration>	_smooth_accel_z			{ kSmoothingTime };
	xf::SocketObserver				_centripetal_computer;
};

#endif
