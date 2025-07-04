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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/executable.h>
#include <xefis/core/xefis.h>

// Standard:
#include <cstddef>


int main (int argc, char** argv, char**)
{
	return xf::setup_xefis_executable(argc, argv, [&argc, &argv] {
		auto app = std::make_unique<xf::Xefis> (argc, argv);
		app->exec();
	});
}

