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
#include "test_generator.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


void
TestGenerator::update_all (si::Time const update_dt)
{
	for (auto& generator: _generators)
		generator->update (update_dt);
}


TestGenerator::TestGenerator (std::string_view const& instance):
	Module (instance)
{ }


void
TestGenerator::process (xf::Cycle const& cycle)
{
	update_all (cycle.update_dt());
}

