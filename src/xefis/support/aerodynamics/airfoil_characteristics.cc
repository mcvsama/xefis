/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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
#include "airfoil_characteristics.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

AirfoilCharacteristics::AirfoilCharacteristics (AirfoilSpline const& spline,
												LiftField const& lift_field,
												DragField const& drag_field,
												PitchingMomentField const& pitching_moment_field,
												CenterOfPressurePositionField const& center_of_pressure_offset_field):
	_spline (spline),
	_lift_coefficient (lift_field),
	_drag_coefficient (drag_field),
	_pitching_moment_coefficient (pitching_moment_field),
	_center_of_pressure_position (center_of_pressure_offset_field)
{ }

} // namespace xf

