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

#ifndef SI__SI_H__INCLUDED
#define SI__SI_H__INCLUDED

// Standard:
#include <type_traits>

// Local:
#include "si_config.h"


// Configuration checks:
static_assert (std::is_class<si_config::Exception>::value, "si_config::Exception class definition is required");


// Local:
#include "lonlat.h"
#include "exception.h"
#include "quantity.h"
#include "standard_literals.h"
#include "standard_quantities.h"
#include "standard_unit_traits.h"
#include "standard_units.h"
#include "unit.h"
#include "unit_traits.h"
#include "utils.h"


namespace si {

using namespace quantities;
using namespace units;
using namespace literals;

} // namespace si

#endif

