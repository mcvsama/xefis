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

#ifndef NEUTRINO__SI__ADDITIONAL_UNITS_H__INCLUDED
#define NEUTRINO__SI__ADDITIONAL_UNITS_H__INCLUDED

// Standard:
#include <cstddef>
#include <ratio>
#include <utility>

// Local:
#include "quantity.h"
#include "unit.h"


namespace si {
namespace units {

using Foot					= ScaledUnit<Meter, std::ratio<1'200, 3'937>>;
using Mile					= ScaledUnit<Meter, std::ratio<1'609'344, 1'000>>;
using NauticalMile			= ScaledUnit<Meter, std::ratio<1'852, 1>>;
using Inch					= ScaledUnit<Meter, std::ratio<254, 10'000>>;
using PoundMass				= ScaledUnit<Kilogram, std::ratio<45'359'237, 100'000'000>>;
using Gravity				= ScaledUnit<MeterPerSecondSquared, std::ratio<980'665, 100'000>>;
using Rankine				= ScaledUnit<Kelvin, std::ratio<5, 9>>;
using Degree				= ScaledUnit<Radian, std::ratio_divide<SmallPi, std::ratio<180>>>;
using RotationPerMinute		= ScaledUnit<RadianPerSecond, std::ratio_divide<Pi, std::ratio<30>>>;
using InchOfMercury			= ScaledUnit<Pascal, std::ratio<3'386'389, 1000>>;
using KilometerPerHour		= ScaledUnit<MeterPerSecond, std::ratio<10, 36>>;
using FootPerSecond			= ScaledUnit<MeterPerSecond, Foot::Scale>;
using FootPerMinute			= ScaledUnit<FootPerSecond, std::ratio<1, 60>>;
using Knot					= ScaledUnit<MeterPerSecond, std::ratio_multiply<NauticalMile::Scale, std::ratio<1, 3600>>>;

using DotsPerInch			= decltype (1 / std::declval<Quantity<Inch>>())::Unit;
using DotsPerMeter			= decltype (1 / std::declval<Quantity<Meter>>())::Unit;

} // namespace units
} // namespace si

#endif

