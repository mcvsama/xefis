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

#ifndef XEFIS__SUPPORT__DEBUG__DEBUG_PERFORMANCE_H__INCLUDED
#define XEFIS__SUPPORT__DEBUG__DEBUG_PERFORMANCE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/histogram.h>
#include <neutrino/scope_exit.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

/**
 * Parameters for debug performance measurements.
 * Extends histogram configuration with sample retention cap.
 */
struct DebugPerformanceParameters
{
	si::Time				bin_width	{ 1_ms };
	std::optional<si::Time>	min_x;
	std::optional<si::Time>	max_x;
	std::size_t				max_samples { 1u };
};


/**
 * Start time measurement for a named debug histogram.
 * Returns a stopper that may be called manually and also auto-stops on destruction.
 */
[[nodiscard]]
nu::ScopeExit<>
debug_measure_performance (std::string const& name, DebugPerformanceParameters);

} // namespace xf

#endif
