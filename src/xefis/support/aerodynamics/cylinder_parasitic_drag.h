/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__AERODYNAMICS__CYLINDER_PARASITIC_DRAG_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__CYLINDER_PARASITIC_DRAG_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/air.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/force_moments.h>

// Standard:
#include <cstddef>
#include <type_traits>


namespace xf {

template<math::CoordinateSystem Space>
	struct CylinderParasiticDragParameters
	{
		si::Length					radius;
		si::Length					length;
		Air<Space>					relative_air;
		SpaceVector<double, Space>	axis_direction { 1.0, 0.0, 0.0 };
		SpaceLength<Space>			center_of_pressure { 0_m, 0_m, 0_m };
		SpaceLength<Space>			reference_point { 0_m, 0_m, 0_m };
	};


[[nodiscard]]
ForceMoments<void>
cylinder_parasitic_drag_force_moments (CylinderParasiticDragParameters<void> const&);


template<math::CoordinateSystem Space>
	[[nodiscard]]
	inline ForceMoments<Space>
	cylinder_parasitic_drag_force_moments (CylinderParasiticDragParameters<Space> const& params)
		requires (!std::is_same_v<Space, void>)
	{
		auto const result = cylinder_parasitic_drag_force_moments (CylinderParasiticDragParameters<void> {
			.radius = params.radius,
			.length = params.length,
			.relative_air = {
				.density = params.relative_air.density,
				.pressure = params.relative_air.pressure,
				.temperature = params.relative_air.temperature,
				.dynamic_viscosity = params.relative_air.dynamic_viscosity,
				.speed_of_sound = params.relative_air.speed_of_sound,
				.velocity = math::coordinate_system_cast<void, void, Space, void> (params.relative_air.velocity),
			},
			.axis_direction = math::coordinate_system_cast<void, void, Space, void> (params.axis_direction),
			.center_of_pressure = math::coordinate_system_cast<void, void, Space, void> (params.center_of_pressure),
			.reference_point = math::coordinate_system_cast<void, void, Space, void> (params.reference_point),
		});

		return ForceMoments<Space> (
			math::coordinate_system_cast<Space, void, void, void> (result.force()),
			math::coordinate_system_cast<Space, void, void, void> (result.torque())
		);
	}

} // namespace xf

#endif
