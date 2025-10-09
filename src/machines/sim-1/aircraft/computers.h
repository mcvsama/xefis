/* vim:ts=4
 *
 * Copyleft 2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__SIM_1__AIRCRAFT__COMPUTERS_H__INCLUDED
#define XEFIS__MACHINES__SIM_1__AIRCRAFT__COMPUTERS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/modules/systems/air_data_computer.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

/**
 * Contains modules that calculate stuff from sensor data.
 */
class Computers
{
  public:
	// Ctor
	Computers (xf::ProcessingLoop&, nu::Logger const&);

  private:
	xf::ProcessingLoop&		_loop;
	nu::Logger				_logger;

  public:
	AirDataComputer			air_data_computer	{ _loop, nullptr, _logger };
};

} // namespace sim1::aircraft

#endif

