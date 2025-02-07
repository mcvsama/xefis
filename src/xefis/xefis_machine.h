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

#ifndef XEFIS__XEFIS_MACHINE_H__INCLUDED
#define XEFIS__XEFIS_MACHINE_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>

// Xefis:
#include <xefis/app/xefis.h>
#include <xefis/config/all.h>
#include <xefis/core/machine.h>


extern std::unique_ptr<xf::Machine>
make_xefis_machine (xf::Xefis&);

extern std::unique_ptr<xf::MachineManager>
make_xefis_machine_manager (xf::Xefis&);

#endif

